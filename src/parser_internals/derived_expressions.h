#include "parser.h"
#include "parser_operations.h"

ParseDatumFn getDerivedExpressionParseFn(void);

ExprLogical *parseLogical(void);
ExprWhenUnless *parseWhenUnless(void);
ExprBegin *parseBegin(void);
