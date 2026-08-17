#ifndef MATH_H
#define MATH_H

#define EPSILON 0.001f

// -- utils

#define RAD2DEG (180.0f / M_PI)
#define DEG2RAD (M_PI / 180.0f)

// -- vectors

// generic vector declaration
#define VEC_DECL(n, ...)                           \
	typedef union {                                \
	    struct { float __VA_ARGS__; };             \
	    float dat[n];                              \
	} float##n;                                    \
	                                               \
	/* vector negation */                          \
	float##n vecNeg##n(float##n v1);               \
	                                               \
	/* vector addition */                          \
	float##n vecAdd##n(float##n v1, float##n v2);  \
	                                               \
	/* vector subtraction */                       \
	float##n vecSub##n(float##n v1, float##n v2);  \
	                                               \
	/* vector multiplication by scalar */          \
	float##n vecMul##n(float##n v,  float s);      \
	                                               \
	/* vector division by scalar */                \
	float##n vecDiv##n(float##n v,  float s);      \
	                                               \
	/* vector dot product */                       \
	float vecDot##n(float##n v1, float##n v2);     \
	                                               \
	/* vector hadamard (component-wise) product */ \
	float##n vecHad##n(float##n v1, float##n v2);  \
	                                               \
	/* vector squared norm */                      \
	float vecNormSqr##n(float##n v);               \
	                                               \
	/* vector norm */                              \
	float vecNorm##n(float##n v);                  \
	                                               \
	/* normalizes a vector */                      \
	float##n vecNormalized##n(float##n v);         \
	                                               \
	/* prints a vector */                          \
	void vecPrint##n(float##n v);

// 2D vector
VEC_DECL(2, x, y)

// 3D vector
VEC_DECL(3, x, y, z)

// cross product (for 3D vectors only)
float3 vecCross3(float3 v1, float3 v2);

// 4D vector
VEC_DECL(4, x, y, z, w)

// color vector
typedef union {
	struct { float r, g, b; };
	float dat[3];
} color;

// -- matrices

#define MAT_DECL(n, ...)                          \
	typedef union {                               \
	    struct { float __VA_ARGS__; };            \
	    float##n col[n];                          \
	    float dat[n * n];                         \
	} mat##n;                                     \
	                                              \
	/* identity matrix */                         \
	mat##n matEye##n();                           \
	                                              \
	/* matrix from row major array */             \
	mat##n makeMat##n(float f[n * n]);            \
	                                              \
	/* vector multiplication by matrix (right) */ \
	float##n matVec##n (mat##n m,  float##n v);   \
	                                              \
	/* matrix addition */                         \
	mat##n matAdd##n(mat##n m1, mat##n m2);       \
	                                              \
	/* matrix subtraction */                      \
	mat##n matSub##n(mat##n m,  mat##n m2);       \
	                                              \
	/* matrix multiplication by scalar */         \
	mat##n matMulSca##n(mat##n m,  float s);      \
	                                              \
	/*  matrix division by scalar */              \
	mat##n matDivSca##n(mat##n m1, float s);      \
	                                              \
	/* matrix multiplication */                   \
	mat##n matMul##n(mat##n m1, mat##n m2);       \
	                                              \
	/* matrix transpose */                        \
	mat##n matTran##n(mat##n m);                  \
	                                              \
	/* prints a matrix */                         \
	void matPrint##n(mat##n m);

// 2x2 matrix
MAT_DECL(2, a, c, b, d)

// 2D matrix determinant 
float matDet2(mat2 m);

// 2D matrix inverse
mat2 matInv2(mat2 m);

// 3x3 matrix
MAT_DECL(3, a, d, g, b, e, h, c, f, i)

// 3D matrix determinant 
float matDet3(mat3 m);

// 3D matrix inverse
mat3 matInv3(mat3 m);

// 4x4 matrix
MAT_DECL(4, a, e, i, m, b, f, j, n, c, g, k, o, d, h, l, p)

// 4D matrix determinant 
float matDet4(mat4 m);

// 4D matrix inverse
mat4 matInv4(mat4 m);

// gets a projection matrix
mat4 matPersp(float fov, float near, float far, float aspect);

// -- quaternions

// quaternion
typedef union {
	struct { float x, y, z, w; };
	struct {
		float3 v;
		float w;
	} vec;
	float dat[4];
} quat;

// identity quaternion
quat quatIdent();

// quaternion addition 
quat quatAdd(quat q1, quat q2);

// quaternion subtraction 
quat quatSub(quat q1, quat q2);

// quaternion multiplication by scalar
quat quatMulSca(quat q, float s);

// quaternion division by scalar
quat quatDivSca(quat q, float s);

// quaternion multiplication
quat quatMul(quat q1, quat q2);

// quaternion conjugate
quat quatConj(quat q);

// quaternion inverse
quat quatInv(quat q);

// quaternion squared norm 
float quatNormSqr(quat q);

// quaternion norm
float quatNorm(quat q);

// normalizes a quaternion
quat quatNormalized(quat q);

// builds quaternion from axis and angles
quat quatAngleAxis(float angle, float3 axis);

// rotates vector by quaternion
float3 quatRotate(quat q, float3 v);

// quaternion to 3x3 matrix
mat3 quatToMat3(quat q);

// quaternion to 4x4 matrix
mat4 quatToMat4(quat q);

// quaternion to euler, XYZ intrinsic
float3 quatToEuler(quat q);

// euler to quaternion, XYZ intrinsic
quat eulerToQuat(float3 e);

// prints a quaternion
void quatPrint(quat q);

// definition of transform
typedef struct {
	// position
	float3 position;

	// rotation
	quat rotation;

	// scale
	float3 scale;
} transform;

// identity transform
transform transformIdent();

// translation to 4x4 matrix
mat4 translationToMat4(float3 v);

// scale to 4x4 matrix
mat4 scaleToMat4(float3 s);

// transform to 4x4 matrix
mat4 transformToMat4(transform t);

// prints a transform
void transformPrint(transform t);

// extracts position from a transform matrix
float* mat4ExPosition(mat4* m);

// extracts forward vector from a transform matrix
float* mat4ExForward(mat4* m);

#endif
