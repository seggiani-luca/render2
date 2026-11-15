#include "dat/script/includes/includes.scm"

(define (update)
  (define orig-transform (get-field "Transform"))
  (define nu-transform (transform
                         (+ (get-transform-position orig-transform) (float3 0.0005 0 0))
                         (get-transform-rotation orig-transform)
                         (get-transform-scale    orig-transform)))
  (log (norm (get-transform-position orig-transform)))
  (set-field "Transform" nu-transform))
