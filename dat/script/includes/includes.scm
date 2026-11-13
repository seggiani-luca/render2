; -- base definitions

; nil
;   The empty list. Used as the default return value for engine hooks.

(define nil ())

; -- math types 

; float2
;   Constructs a "d" vector.
;
;   Parameters:
;     x, y - Vector components.
;
;   Returns:
;     A list containing (x y).

(define (float2 x y)
  (list x y))


; float3
;   Constructs a 3D vector.
;
;   Parameters:
;     x, y, z - Vector components.
;
;   Returns:
;     A list containing (x y z).

(define (float3 x y z)
  (list x y z))


; float4
;   Constructs a 4D vector.
;
;   Parameters:
;     x, y, z, w - Vector components.
;
;   Returns:
;     A list containing (x y z w).

(define (float4 x y z w)
  (list x y z w))


; mat2
;   Constructs a 2x2 matrix.
;
;   Parameters:
;     a-d - Matrix elements in row-major order.
;
;   Returns:
;     A nested list representing the matrix:
;       ((a b)
;        (c d))

(define (mat2 a b c d)
  (list (list a b)
        (list c d)))


; mat3
;   Constructs a 3x3 matrix.
;
;   Parameters:
;     a-i - Matrix elements in row-major order.
;
;   Returns:
;     A nested list representing the matrix:
;       ((a b c)
;        (d e f)
;        (g h i))

(define (mat3 a b c d e f g h i)
  (list (list a b c)
        (list d e f)
        (list g h i)))


; mat4
;   Constructs a 4x4 matrix.
;
;   Parameters:
;     a-p - Matrix elements in row-major order.
;
;   Returns:
;     A nested list representing the matrix.

(define (mat4 a b c d e f g h i j k l m n o p)
  (list (list a b c d)
        (list e f g h)
        (list i j k l)
        (list m n o p)))


; quat
;   Constructs a quaternion.
;
;   Parameters:
;     x, y, z - Vector components.
;     w       - Scalar component.
;
;   Returns:
;     A 4D vector in XYZW order.

(define (quat x y z w)
  (float4 x y z w))

; TODO math

; -- engine types

; transform
;   Constructs a transform descriptor.
;
;   Parameters:
;     position - Translation vector (float3).
;     rotation - Rotation quaternion (quat).
;     scale    - Scale vector (float3).
;
;   Returns:
;     A list containing (position rotation scale).

(define (transform position rotation scale)
  (list position rotation scale))


; Accessors for transform descriptors.

(define (get-transform-position transform)
  (car transform))

(define (get-transform-rotation transform)
  (car (cdr transform)))

(define (get-transform-scale transform)
  (car (cdr (cdr transform))))


; camera
;   Constructs a camera descriptor.
;
;   Parameters:
;     far-plane  - Far clipping plane.
;     near-plane - Near clipping plane.
;     fov        - Vertical field of view.
;
;   Returns:
;     A list containing (far-plane near-plane fov).

(define (camera far-plane near-plane fov)
  (list far-plane near-plane fov))


; Accessors for camera descriptors.

(define (get-camera-far-plane camera)
  (car camera))

(define (get-camera-near-plane camera)
  (car (cdr camera)))

(define (get-camera-fov camera)
  (car (cdr (cdr camera))))


; atmosphere
;   Constructs an atmosphere descriptor.
;
;   Parameters:
;     ambient    - Ambient lighting color (float3).
;     sun        - Sun lighting color (float3).
;     background - Background color (float3).
;
;   Returns:
;     A list containing (ambient sun background).

(define (atmosphere ambient sun background)
  (list ambient sun background))


; Accessors for atmosphere descriptors.

(define (get-atmosphere-ambient atmosphere)
  (car atmosphere))

(define (get-atmosphere-sun atmosphere)
  (car (cdr atmosphere)))

(define (get-atmosphere-background atmosphere)
  (car (cdr (cdr atmosphere))))


; -- Engine hooks

; start
;   Called once when the script is initialized.
;   Defaults at nil, redefine for functionality.

(define (start) nil)


; update
;   Called during each script update.
;   Defaults at nil, redefine for functionality.

(define (update) nil)
