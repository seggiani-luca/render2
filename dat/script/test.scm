#include "dat/script/includes/includes.scm"

(define (update)
  (define orig-transform (get-field "Transform"))
  (define nu-transform (transform
                         (get-transform-position orig-transform)
                         (get-transform-rotation orig-transform)
                         (get-transform-scale    orig-transform)))
  (set-field "Transform" nu-transform))
