; -- base definitions

; nil
;   The empty list. Used as the default return value for engine hooks.

(define nil ())

; -- math types 

; float2
;   Constructs a 2D vector.
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

; -- math operations

; Arithmetic operators
;   +, - operate on scalars, vectors, and matrices.
;   * supports scalar multiplication, matrix-vector multiplication,
;     and matrix-matrix multiplication.
;   / supports scalar and vector/matrix division by a scalar.
;
;   Vector and matrix operands must have matching dimensions.
;   Matrices must be square (2x2, 3x3, or 4x4).
;
;   Quaternion units are radians, euler angle units are degrees.
;
;   Examples:
;     (+ (float3 1 2 3) (float3 4 5 6))
;     (- (mat2 1 2 3 4) (mat2 4 3 2 1))
;     (* (mat3 ...) (float3 ...))
;     (* (mat4 ...) (mat4 ...))
;     (* (mat4 ...) (vec4 ...))
;     (* (float3 1 2 3) 2)
;     (/ (float3 2 4 6) 2)


; dot
;   Computes the dot product of two vectors.
;
;   Parameters:
;     a, b - Vectors of matching dimensions (2–4).
;
;   Returns:
;     A scalar.

; cross
;   Computes the cross product of two 3D vectors.
;
;   Parameters:
;     a, b - 3D vectors.
;
;   Returns:
;     A 3D vector.

; hadamard
;   Computes the component-wise product of two vectors.
;
;   Parameters:
;     a, b - Vectors of matching dimensions (2–4).
;
;   Returns:
;     A vector of the same dimension.

; norm-square
;   Computes the squared Euclidean norm of a vector.
;
;   Parameters:
;     vector - A vector (2–4 dimensions).
;
;   Returns:
;     A scalar.

; norm
;   Computes the Euclidean norm of a vector.
;
;   Parameters:
;     vector - A vector (2–4 dimensions).
;
;   Returns:
;     A scalar.

; normalize
;   Returns the normalized form of a vector.
;
;   Parameters:
;     vector - A vector (2–4 dimensions).
;
;   Returns:
;     A vector of the same dimension.

; mat2-eye
;   The identity 2x2 matrix.

(define mat2-eye
  (mat2 1 0
        0 1))


; mat3-eye
;   The identity 3x3 matrix.

(define mat3-eye
  (mat3 1 0 0
        0 1 0
        0 0 1))


; mat4-eye
;   The identity 4x4 matrix.

(define mat4-eye
  (mat4 1 0 0 0
        0 1 0 0
        0 0 1 0
        0 0 0 1))


; transpose
;   Transposes a square matrix.
;
;   Parameters:
;     matrix - A square matrix (2x2, 3x3, or 4x4).
;
;   Returns:
;     The transposed matrix.

; det
;   Computes the determinant of a square matrix.
;
;   Parameters:
;     matrix - A square matrix (2x2, 3x3, or 4x4).
;
;   Returns:
;     A scalar.

; invert
;   Computes the inverse of a square matrix.
;
;   Parameters:
;     matrix - A square matrix (2x2, 3x3, or 4x4).
;
;   Returns:
;     The inverse matrix.


; Quaternion operations
;   Quaternions are represented as 4D vectors in XYZW order.

; quat+
;   Adds two quaternions component-wise.
;
;   Parameters:
;     a, b - Quaternions.
;
;   Returns:
;     A quaternion.

; quat-
;   Subtracts two quaternions component-wise.
;
;   Parameters:
;     a, b - Quaternions.
;
;   Returns:
;     A quaternion.

; quat*
;   Multiplies a quaternion by a scalar or another quaternion.
;
;   Parameters:
;     a - A quaternion.
;     b - A scalar or quaternion.
;
;   Returns:
;     A quaternion.

; quat/
;   Divides a quaternion by a scalar.
;
;   Parameters:
;     quaternion - A quaternion.
;     scalar     - A scalar.
;
;   Returns:
;     A quaternion.

; quat-conjugate
;   Computes the conjugate of a quaternion.
;
;   Parameters:
;     quaternion - A quaternion.
;
;   Returns:
;     A quaternion.

; quat-invert
;   Computes the inverse of a quaternion.
;
;   Parameters:
;     quaternion - A quaternion.
;
;   Returns:
;     A quaternion.

; quat-norm-square
;   Computes the squared norm of a quaternion.
;
;   Parameters:
;     quaternion - A quaternion.
;
;   Returns:
;     A scalar.

; quat-norm
;   Computes the norm of a quaternion.
;
;   Parameters:
;     quaternion - A quaternion.
;
;   Returns:
;     A scalar.

; quat-normalize
;   Returns the normalized form of a quaternion.
;
;   Parameters:
;     quaternion - A quaternion.
;
;   Returns:
;     A quaternion.

; quat-angle-axis
;   Constructs a quaternion from an angle and a 3D rotation axis.
;
;   Parameters:
;     angle - Rotation angle.
;     axis  - 3D rotation axis.
;
;   Returns:
;     A quaternion.

; quat-to-euler
;   Converts a quaternion to XYZ extrinsic Euler angles (roll, pitch, yaw).
;   Returns a 3D vector containing the X, Y and Z angles.
;
;   Quaternion units are radians, euler angle units are degrees.

; euler-to-quat
;   Converts XYZ extrinsic Euler angles (roll, pitch, yaw) to a quaternion.
;   Takes a 3D vector containing the X, Y and Z angles.
;
;   Quaternion units are radians, euler angle units are degrees.

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


; -- engine hooks

; start
;   Called once when the script is initialized.
;   Defaults at nil, redefine for functionality.

(define (start) nil)


; update
;   Called during each script update.
;   Defaults at nil, redefine for functionality.

(define (update) nil)
