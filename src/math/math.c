#include "math.h"
#include <math.h>
#include <stdio.h>

// -- vectors

// generic vector implementation
#define VEC_IMPL(n)                                                  \
	float##n vecNeg##n(float##n v1) {                                \
	    float##n r;                                                  \
	    for(int i = 0; i < n; i++) r.dat[i] = -v1.dat[i];            \
	    return r;                                                    \
	}                                                                \
	                                                                 \
	float##n vecAdd##n(float##n v1, float##n v2) {                   \
	    float##n r;                                                  \
	    for(int i = 0; i < n; i++) r.dat[i] = v1.dat[i] + v2.dat[i]; \
	    return r;                                                    \
	}                                                                \
	                                                                 \
	float##n vecSub##n(float##n v1, float##n v2) {                   \
	    float##n r;                                                  \
	    for(int i = 0; i < n; i++) r.dat[i] = v1.dat[i] - v2.dat[i]; \
	    return r;                                                    \
	}                                                                \
	                                                                 \
	float##n vecMul##n(float##n v,  float s) {                       \
	    float##n r;                                                  \
	    for(int i = 0; i < n; i++) r.dat[i] = v.dat[i] * s;          \
	    return r;                                                    \
	}                                                                \
	                                                                 \
	float##n vecDiv##n(float##n v,  float s) {                       \
	    float##n r;                                                  \
	    for(int i = 0; i < n; i++) r.dat[i] = v.dat[i] / s;          \
	    return r;                                                    \
	}                                                                \
	                                                                 \
	float vecDot##n(float##n v1, float##n v2) {                      \
	    float r = 0;                                                 \
	    for(int i = 0; i < n; i++) r += v1.dat[i] * v2.dat[i];       \
	    return r;                                                    \
	}                                                                \
	                                                                 \
	float##n vecHad##n(float##n v1, float##n v2) {                   \
	    float##n r;                                                  \
	    for(int i = 0; i < n; i++) r.dat[i] = v1.dat[i] * v2.dat[i]; \
	    return r;                                                    \
	}                                                                \
	                                                                 \
	float vecNormSqr##n(float##n v) {                                \
	    return vecDot##n(v, v);                                      \
	}                                                                \
	                                                                 \
	float vecNorm##n(float##n v) {                                   \
	    return sqrtf(vecNormSqr##n(v));                              \
	}                                                                \
	                                                                 \
	float##n vecNormalized##n(float##n v) {                          \
	    float len = vecNorm##n(v);                                   \
	    return len > EPSILON ? vecDiv##n(v, len) : v;                \
	}                                                                \
	                                                                 \
	void vecPrint##n(float##n v) {                                   \
	    for(int i = 0; i < n; i++) printf("%g ", v.dat[i]);          \
	}

VEC_IMPL(2)
VEC_IMPL(3)
VEC_IMPL(4)

float3 vecCross3(float3 v1, float3 v2) {
	return (float3) {
		v1.y * v2.z - v1.z * v2.y,
		v1.z * v2.x - v1.x * v2.z,
		v1.x * v2.y - v1.y * v2.x,
	};
}

// -- matrices

#define MAT_IMPL(n)                                                     \
	mat##n matEye##n() {                                                \
	    mat##n m = {0};                                                 \
	    for(int i = 0; i < n; i++) m.col[i].dat[i] = 1;                 \
	    return m;                                                       \
	}                                                                   \
	                                                                    \
	mat##n makeMat##n(float f[n * n]) {                                 \
	    mat##n m;                                                       \
	    for(int i = 0; i < n; i++) for(int j = 0; j < n; j++)           \
	        m.col[j].dat[i] = f[i * n + j];                             \
	    return m;                                                       \
	}                                                                   \
	                                                                    \
	float##n matVec##n(mat##n m,  float##n v) {                         \
	    float##n r = (float##n){0};                                     \
	    for(int i = 0; i < n; i++) for(int j = 0; j < n; j++)           \
	        r.dat[i] += m.col[j].dat[i] * v.dat[j];                     \
	    return r;                                                       \
	}                                                                   \
	                                                                    \
	mat##n matAdd##n(mat##n m1, mat##n m2) {                            \
	    mat##n m;                                                       \
	    for(int i = 0; i < n; i++) for(int j = 0; j < n; j++)           \
	            m.col[i].dat[j] = m1.col[i].dat[j] + m2.col[i].dat[j];  \
	    return m;                                                       \
	}                                                                   \
	                                                                    \
	mat##n matSub##n(mat##n m1, mat##n m2) {                            \
	    mat##n m;                                                       \
	    for(int i = 0; i < n; i++) for(int j = 0; j < n; j++)           \
	            m.col[i].dat[j] = m1.col[i].dat[j] - m2.col[i].dat[j];  \
	    return m;                                                       \
	}                                                                   \
	                                                                    \
	mat##n matMulSca##n(mat##n m, float s) {                            \
	    mat##n r;                                                       \
	    for(int i = 0; i < n; i++) for(int j = 0; j < n; j++)           \
	            r.col[i].dat[j] = m.col[i].dat[j] * s;                  \
	    return r;                                                       \
	}                                                                   \
	                                                                    \
	mat##n matDivSca##n(mat##n m, float s) {                            \
	    mat##n r;                                                       \
	    for(int i = 0; i < n; i++) for(int j = 0; j < n; j++)           \
	            r.col[i].dat[j] = m.col[i].dat[j] / s;                  \
	    return r;                                                       \
	}                                                                   \
	                                                                    \
	mat##n matMul##n(mat##n m1, mat##n m2) {                            \
	    mat##n m = (mat##n){0};                                         \
	    for(int i = 0; i < n; i++) for(int j = 0; j < n; j++)           \
	        for(int k = 0; k < n; k++)                                  \
	            m.col[j].dat[i] += m1.col[k].dat[i] * m2.col[j].dat[k]; \
	    return m;                                                       \
	}                                                                   \
	                                                                    \
	mat##n matTran##n(mat##n m) {                                       \
	    mat##n r;                                                       \
	    for(int i = 0; i < n; i++) for(int j = 0; j < n; j++)           \
	            r.col[i].dat[j] = m.col[j].dat[i];                      \
	    return r;                                                       \
	}                                                                   \
	                                                                    \
	void matPrint##n(mat##n m) {                                        \
	    for(int i = 0; i < n; i++) {                                    \
	        for(int j = 0; j < n; j++) printf("%g ", m.col[j].dat[i]);  \
	        printf("\n");                                               \
	    }                                                               \
	}

MAT_IMPL(2)
MAT_IMPL(3)
MAT_IMPL(4)

float matDet2(mat2 m) {
	return m.a * m.d - m.b * m.c;
}

mat2 matInv2(mat2 m) {
	float det = matDet2(m);
	if(fabsf(det) < EPSILON) return (mat2){0};

	return matDivSca2(makeMat2((float[4]) {
		 m.d, -m.b,
		-m.c,  m.a
	}), det);
}

float matDet3(mat3 m) {
	return m.a * (m.e * m.i - m.f * m.h)
	     - m.b * (m.d * m.i - m.f * m.g)
	     + m.c * (m.d * m.h - m.e * m.g);
}

mat3 matInv3(mat3 m) {
	float det = matDet3(m);
	if(fabsf(det) < EPSILON) return (mat3){0};
	
	return matDivSca3(makeMat3((float[9]){
		 m.e * m.i - m.f * m.h,
		-m.b * m.i + m.c * m.h,
		 m.b * m.f - m.c * m.e,

		-m.d * m.i + m.f * m.g,
		 m.a * m.i - m.c * m.g,
		-m.a * m.f + m.c * m.d,

		 m.d * m.h - m.e * m.g,
		-m.a * m.h + m.b * m.g,
		 m.a * m.e - m.b * m.d
	}), det);
}

float matDet4(mat4 m) {
	mat3 a, b, c, d;
	a = makeMat3((float[9]){m.f,m.g,m.h, m.j,m.k,m.l, m.n,m.o,m.p});
	b = makeMat3((float[9]){m.e,m.g,m.h, m.i,m.k,m.l, m.m,m.o,m.p});
	c = makeMat3((float[9]){m.e,m.f,m.h, m.i,m.j,m.l, m.m,m.n,m.p});
	d = makeMat3((float[9]){m.e,m.f,m.g, m.i,m.j,m.k, m.m,m.n,m.o});

	return m.a * matDet3(a)
	     - m.b * matDet3(b)
	     + m.c * matDet3(c)
	     - m.d * matDet3(d);
}

mat4 matInv4(mat4 m) {
	float det = matDet4(m);
	if(fabsf(det) < EPSILON) return (mat4){0};

	mat4 r;
	for(int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) {
		mat3 minor;
		int mi = 0, mj = 0;

		for(int y = 0; y < 4; y++) {
			if(y == i) continue;
			mj = 0;
			for(int x = 0; x < 4; x++) {
				if(x == j) continue;
				minor.col[mj++].dat[mi] = m.col[x].dat[y];
			}
			mi++;
		}

		float cofactor = matDet3(minor);
		if((i + j) & 1) cofactor = -cofactor;

		r.col[i].dat[j] = cofactor / det;
	}

	return r;
}

mat4 matPersp(float fov, float near, float far, float aspect) {
	// TODO: implement this and finish render()
}

// -- quaternions

quat quatIdent() {
	return (quat) {
		.w = 1,
		.x = 0, .y = 0, .z = 0
	};
}

quat quatAdd(quat q1, quat q2) {
	return (quat){
		.vec.w = q1.w + q2.w,
		.vec.v = vecAdd3(q1.vec.v, q2.vec.v)
	};
}

quat quatSub(quat q1, quat q2) {
	return (quat){
		.vec.w = q1.w - q2.w,
		.vec.v = vecSub3(q1.vec.v, q2.vec.v)
	};
}

quat quatMulSca(quat q, float s) {
	return (quat){
		.vec.w = q.w * s,
		.vec.v = vecMul3(q.vec.v, s)
	};
}

quat quatDivSca(quat q, float s) {
	return (quat){
		.vec.w = q.w / s,
		.vec.v = vecDiv3(q.vec.v, s)
	};
}

quat quatMul(quat q1, quat q2) {
	float w1 = q1.w;
	float w2 = q2.w;
	float3 v1 = q1.vec.v;
	float3 v2 = q2.vec.v;

	return (quat){
		.vec.w = w1 * w2 - vecDot3(v1, v2),
		.vec.v = vecAdd3(vecAdd3(
			vecMul3(v2, w1),
			vecMul3(v1, w2)), 
			vecCross3(v1, v2)) 
	};
}

quat quatConj(quat q) {
	return (quat){
		.vec.w = q.w,
		.vec.v = vecNeg3(q.vec.v)
	};
}

quat quatInv(quat q) {
	return quatDivSca(
		quatConj(q), quatNormSqr(q)
	);
}

float quatNormSqr(quat q) {
	return q.w * q.w + vecDot3(q.vec.v, q.vec.v);
}

float quatNorm(quat q) {
	return sqrtf(quatNormSqr(q));
}

quat quatNormalized(quat q) {
	float len = quatNorm(q);
	return len > EPSILON ? quatDivSca(q, len) : q;
}

quat quatAngleAxis(float angle, float3 axis) {
	float hAngle = angle / 2;
	axis = vecNormalized3(axis);

	return (quat){
		.vec.w = cosf(hAngle),
		.vec.v = vecMul3(axis, sinf(hAngle))
	};
}

float3 quatRotate(quat q, float3 v) {
	quat vP = (quat){
		.vec.w = 0,
		.vec.v = v
	};

	quat r = quatMul(quatMul(
		q, 
		vP),
		quatConj(q));

	return r.vec.v;
}

mat3 quatToMat3(quat q) {
	return makeMat3((float[9]){
		1 - 2 * (q.y * q.y + q.z * q.z), 2 * (q.x * q.y - q.w * q.z),     2 * (q.x * q.z + q.w * q.y), 
		2 * (q.x * q.y + q.w * q.z),     1 - 2 * (q.x * q.x + q.z * q.z), 2 * (q.y * q.z - q.w * q.x),
		2 * (q.x * q.z - q.w * q.y),     2 * (q.y * q.z + q.w * q.x),     1 - 2 * (q.x * q.x + q.y * q.y)
	});
}

mat4 quatToMat4(quat q) {
	mat3 r3 = quatToMat3(q);

	mat4 r4 = {0};
	for(int i = 0; i < 3; i++) for(int j = 0; j < 3; j++)
		r4.col[i].dat[j] = r3.col[i].dat[j];
	r4.p = 1;

	return r4;
}

float3 quatToEuler(quat q) {
	float3 e;

	// pitch (X)
	float sinp = 2 * (q.w * q.x + q.y * q.z);
	float cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
	e.x = atan2f(sinp, cosp);

	// yaw (Y)
	float siny = 2 * (q.w * q.y - q.z * q.x);
	if (fabsf(siny) >= 1) e.y = copysignf(M_PI / 2, siny); // gimbal lock
	else e.y = asinf(siny);

	// roll (Z)
	float sinr = 2 * (q.w * q.z + q.x * q.y);
	float cosr = 1 - 2 * (q.y * q.y + q.z * q.z);
	e.z = atan2f(sinr, cosr);

	// convert to degrees 
	e.x *= RAD2DEG;
	e.y *= RAD2DEG;
	e.z *= RAD2DEG;

	return e;
}

quat eulerToQuat(float3 e) {
	// convert to radians
	e.x *= DEG2RAD;
	e.y *= DEG2RAD;
	e.z *= DEG2RAD;

	float cx = cosf(e.x * 0.5f); float sx = sinf(e.x * 0.5f);
	float cy = cosf(e.y * 0.5f); float sy = sinf(e.y * 0.5f);
	float cz = cosf(e.z * 0.5f); float sz = sinf(e.z * 0.5f);

	return (quat){
		.w = cx * cy * cz + sx * sy * sz,
		.x = sx * cy * cz - cx * sy * sz,
		.y = cx * sy * cz + sx * cy * sz,
		.z = cx * cy * sz - sx * sy * cz
	};
}

void quatPrint(quat q) {
	printf("%g (%g %g %g)", q.w, q.x, q.y, q.z);
}

transform transformIdent() {
	transform r = {0};
	r.scale = (float3){1.0f, 1.0f, 1.0f};

	return r;
}

mat4 translationToMat4(float3 v) {
	mat4 r = matEye4();
	r.d = v.x;
	r.h = v.y;
	r.l = v.z;

	return r;
}

mat4 scaleToMat4(float3 s) {
	mat4 r = {0};
	r.a = s.x;
	r.f = s.y;
	r.k = s.z;
	r.p = 1.0f;

	return r;
}

mat4 transformToMat4(transform t) {
	mat4 tMat = translationToMat4(t.position);
	mat4 rMat = quatToMat4(t.rotation);
	mat4 sMat = scaleToMat4(t.scale);

	return matMul4(matMul4(
		tMat,
		rMat),
		sMat);
}

void transformPrint(transform t) {
	float3 euler = quatToEuler(t.rotation);
	printf("Position: %f, %f, %f, Rotation: %f, %f, %f, Scale: %f, %f, %f",
		t.position.x, t.position.y, t.position.z,
		euler.x, euler.y, euler.z,
		t.scale.x, t.scale.y, t.scale.z);
}
