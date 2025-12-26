/*
  Copyright 2025 Evan Cooney
  Copyright 2015-2020 Robert Nystrom

  This file is part of Ecsi.

  Ecsi is free software: you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  Ecsi is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with
  Ecsi. If not, see <https://www.gnu.org/licenses/>.
 */

#include "table.h"

#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"

static Entry *findEntry(Entry *entries, int capacity, ObjSymbol *key);
static void adjustCapacity(Table *table, int capacity);

void initTable(Table *table) {
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void freeTable(Table *table) {
    FREE_ARRAY(Entry, table->entries, table->capacity);
    initTable(table);
}

static Entry *findEntry(Entry *entries, int capacity, ObjSymbol *key) {
    uint32_t index = key->hash & (capacity - 1);
    Entry *tombstone = NULL;

    for (;;) {
        Entry *entry = &entries[index];
        if (NULL == entry->key) {
            if (IS_NIL(entry->value)) {
                // Empty entry
                return tombstone != NULL ? tombstone : entry;
            } else {
                // We found a tombstone
                if (NULL == tombstone) tombstone = entry;
            }
        } else if (key == entry->key) {
            // We found the key
            return entry;
        }

        index = (index + 1) & (capacity - 1);
    }
}

static void adjustCapacity(Table *table, int capacity) {
    Entry *entries = ALLOCATE(Entry, capacity);
    for (int i = 0; i < capacity; i++) {
        entries[i].key = NULL;
        entries[i].value = NIL_VAL;
    }

    table->count = 0;
    for (int i = 0; i < table->capacity; i++) {
        Entry const entry = table->entries[i];
        if (NULL == entry.key) continue;

        Entry *dest = findEntry(entries, capacity, entry.key);
        dest->key = entry.key;
        dest->value = entry.value;
        table->count++;
    }

    FREE_ARRAY(Entry, table->entries, table->capacity);

    table->entries = entries;
    table->capacity = capacity;
}

bool tableGet(Table *table, ObjSymbol *key, Value *value) {
    if (0 == table->count) return false;

    Entry *entry = findEntry(table->entries, table->capacity, key);
    if (NULL == entry->key) return false;

    *value = entry->value;
    return true;
}

bool tableSet(Table *table, ObjSymbol *key, Value value) {
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
        int capacity = GROW_CAPACITY(table->capacity);
        adjustCapacity(table, capacity);
    }
    Entry *entry = findEntry(table->entries, table->capacity, key);
    bool isNewKey = NULL == entry->key;
    if (isNewKey && IS_NIL(entry->value)) table->count++;

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

bool tableDelete(Table *table, ObjSymbol *key) {
    if (0 == table->count) return false;

    // Find the entry.
    Entry *entry = findEntry(table->entries, table->capacity, key);
    if (NULL == entry->key) return false;

    // Place a tombstone in the entry.
    entry->key = NULL;
    entry->value = BOOL_VAL(true);
    return true;
}

void tableAddAll(Table *from, Table *to) {
    for (int i = 0; i < from->capacity; i++) {
        Entry *entry = &from->entries[i];
        if (entry->key != NULL) {
            tableSet(to, entry->key, entry->value);
        }
    }
}

ObjSymbol *tableFindString(Table *table, char const *chars, int length,
                           uint32_t hash) {
    if (0 == table->count) return NULL;

    uint32_t index = hash & (table->capacity - 1);
    for (;;) {
        Entry *entry = &table->entries[index];
        if (NULL == entry->key) {
            // Stop if we found an empty non-tombstone entry.
            if (IS_NIL(entry->value)) return NULL;
        } else if (length == entry->key->length && hash == entry->key->hash &&
                   0 == memcmp(entry->key->chars, chars, length)) {
            // We found it.
            return entry->key;
        }

        index = (index + 1) & (table->capacity - 1);
    }
}

void markTable(Table *table) {
    for (int i = 0; i < table->capacity; i++) {
        Entry *entry = &table->entries[i];
        markObject((Obj *)entry->key);
        markValue(entry->value);
    }
}

void tableRemoveWhite(Table *table) {
    for (int i = 0; i < table->capacity; i++) {
        Entry *entry = &table->entries[i];
        if (entry->key != NULL && !entry->key->obj.isMarked) {
            tableDelete(table, entry->key);
        }
    }
}
