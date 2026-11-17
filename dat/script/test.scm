#include "dat/script/includes/includes.scm"

(define (start)
  (require-field "float" "Rot. Speed")
  (require-field "float3" "Rot. Axis"))

(define (update)
  (rotate (quat-angle-axis
            (* delta-time (get-field "Rot. Speed"))
            (get-field "Rot. Axis"))))
