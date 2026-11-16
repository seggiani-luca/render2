; -- base definitions

; Special forms
;   Most special forms are defined to agree with the usual Scheme syntax.
;
;   Available special forms are:
;     - +, -, *, /, redefined by the engine as a special set of vector
;                   arithmetic functions (see below)
;     - <, <=, >, >=, =, !=
;     - symbol?, number?, bool?, string?, pair?, procedure?, null?
;     - equal?
;     - define
;     - lambda
;     - begin
;     - quote
;     - list
;     - cons
;     - car
;     - cdr
;     - eval
;     - if
;     - cond
;     - when
;     - unless
;     - and, or, not
;
;   The main difference from standard Scheme is that only define exists
;   (no set!), and all definitions pollute the global namespace.
;   No closures exist (as of now).
;
;   The scripting engine is designed as a set of engine-specific extensions (in
;   src/script/*) on a base lisp execution engine (src/script/lisp*).
;
;   Check src/script/lisp.h and src/script/lisp.c for details.

; nil
;   The empty list. Used as the default return value for engine hooks.

(define nil ())

; epsilon
;   Arbitrarily small value.

(define epsilon 0.001)

; pi
;   Ratio between circumference and diameter of a circle.

(define pi 3.141519)

; length
;   Returns the length of a list.
;
;   Parameters:
;     l - A list.
;
;   Returns:
;     A scalar.

(define (length-recurse l n)
  (if (null? l)
    n
    (length-recurse (cdr l) (+ 1 n))))

(define (length l)
  (length-recurse l 0))

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


; abs
;   Takes the absolute of a scalar.
;
;   Paramters:
;     n - A scalar.
;
;   Returns:
;     A scalar.

(define (abs n)
  (if (< n 0)
    (- 0 n)
    n))


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


; translate
;   Translates the transform by a given vector
;   (local transform, local coordinates).
;   
;   Parameters:
;     vec - Translation vector.
;
;   Returns:
;     nil.

(define (translate vec)
  (define translate-transform (get-field "Transform"))
  (set-field "Transform"
             (transform
               (+ (get-transform-position translate-transform) vec)
               (get-transform-rotation    translate-transform)
               (get-transform-scale       translate-transform))))


; rotate 
;   Rotates the transform by a given quaternion 
;   (local transform, local coordinates).
;   
;   Parameters:
;     quat - Rotation quaternion.
;
;   Returns:
;     nil.

(define (rotate quat)
  (define rotate-transform (get-field "Transform"))
  (set-field "Transform"
             (transform
               (get-transform-position        rotate-transform)
               (quat* (get-transform-rotation rotate-transform) quat)
               (get-transform-scale           rotate-transform))))

; scale 
;   Scales the transform by a given vector
;   (local transform, local coordinates).
;   
;   Parameters:
;     vec - Scale vector.
;
;   Returns:
;     nil.

(define (scale vec)
  (define scale-transform (get-field "Transform"))
  (set-field "Transform"
             (transform
               (get-transform-position scale-transform)
               (get-transform-rotation scale-transform)
               (hadamard (get-transform-scale scale-transform) vec))))


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


; -- logging

; log
;   Logs an informational message.
;
;   Parameters:
;     message - The message to log.
;
;   Returns:
;     nil.

; warn
;   Logs a warning message.
;
;   Parameters:
;     message - The message to log.
;
;   Returns:
;     nil.

; error
;   Logs an error message. Note that this halts execution, which might have
;   different effects:
;   - During script execution: almost definitely results in later execution
;     errors;
;   - During engine hook execution: halts that specific hook only.
;
;   Parameters:
;     message - The message to log.
;
;   Returns:
;     nil.


; -- input

; get-key
;   Returns the current state of a keyboard key.
;
;   Parameters:
;     state - The state of the key (down, pressed, released).
;     key - The key identifier.
;
;   Available keys:
;     - space
;     - apostrophe
;     - comma
;     - minus
;     - period
;     - slash
;     - 0 through 9
;     - a through z (lowercase)
;     - semicolon
;     - equal
;     - left bracket
;     - backslash
;     - right bracket
;     - grave accent
;     - world 1
;     - world 2
;     - escape
;     - enter
;     - tab
;     - backspace
;     - insert
;     - delete
;     - right
;     - left
;     - down
;     - up
;     - page up
;     - page down
;     - home
;     - end
;     - caps lock
;     - scroll lock
;     - num lock
;     - print screen
;     - pause
;     - f1 through f25
;     - kp 0 through kp 9
;     - kp decimal
;     - kp divide
;     - kp multiply
;     - kp subtract
;     - kp add
;     - kp enter
;     - kp equal
;     - left shift
;     - left control
;     - left alt
;     - left super
;     - right shift
;     - right control
;     - right alt
;     - right super
;     - menu
;
;   Returns:
;     A boolean indicating whether the key is pressed.

; get-axis
;   Returns the current value of an input axis.
;
;   Available axes are:
;       - button 1
;       - button 2
;       - button 3
;       - button 4
;       - button 5
;       - button 6
;       - button 7
;       - button 8
;       - left mouse (alias of button 1)
;       - right mouse (alias of button 2)
;       - middle mouse (alias of button 3)
;
;   Parameters:
;     axis - The axis identifier.
;
;   Returns:
;     A scalar representing the axis value.


; -- entity handling

; find-entity
;   Finds an entity by name.
;
;   Parameters:
;     name - The entity name.
;
;   Returns:
;     The matching entity, or nil if none is found.

; get-entity-name
;   Returns the name of an entity.
;
;   Parameters:
;     entity - The entity.
;
;   Returns:
;     A string containing the entity name.

; get-entity-child
;   Returns a child entity by name.
;
;   Parameters:
;     entity - The parent entity.
;     name   - The child entity name.
;
;   Returns:
;     The matching child entity, or nil if none is found.

; get-entity-children
;   Returns the children of an entity.
;
;   Parameters:
;     entity - The parent entity.
;
;   Returns:
;     A list of child entities.

; get-entity-parent
;   Returns the parent of an entity.
;
;   Parameters:
;     entity - The entity.
;
;   Returns:
;     The parent entity, or nil if none exists.

; self
;   Alias for the entity the script runs on.

; -- field handling

; get-field
;   Retrieves the value of an entity field on the script's own entity.
;
;   Parameters:
;     name   - The field name.
;
;   Returns:
;     The field value, or nil if the field does not exist.

; set-field
;   Sets the value of an entity field on the script's own entity.
;
;   Parameters:
;     name   - The field name.
;     value  - The new field value.
;
;   Returns:
;     nil.

; require-field
;   Creates a required field on the script's own entity.
;
;   Field type must be specified, available types are:
;     - int
;     - float
;     - string
;     - float2
;     - float3
;     - float4
;     - mat2
;     - mat3
;     - mat4
;     - quat
;     - transform
;     - camera
;     - atmosphere
;     - texture
;     - mesh
;     - material
;     - script
;     - shader
;
;   Parameters:
;     type   - The field type.
;     name   - The field name.
