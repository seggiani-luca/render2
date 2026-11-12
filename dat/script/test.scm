#include "dat/script/includes/includes.scm"

(define (log-children children)
  (unless (null? children) (begin
	(log (get-entity-name (car children)))
	(log-children (cdr children)))))

(define (start)
  (define children (get-entity-children (find-entity "root")))
  (log-children children))
