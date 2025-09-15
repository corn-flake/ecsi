(lambda (x y . z)
  (define (sum-list lst accum)
    (if (null? lst)
	0
	(+ (car lst) (sum-lst (cdr lst) accum))))
  (+ x y (sum-list z 0)))
