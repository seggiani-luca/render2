#ifndef SCRIPT_MATH_H
#define SCRIPT_MATH_H

#include "../lisp/lisp.h"

// definition of native vector add function
value* nativeVecAdd(arena* a, environment* env, value* args);

// definition of native vector sub function
value* nativeVecSub(arena* a, environment* env, value* args);

// definition of native vector multiplication function
value* nativeVecMul(arena* a, environment* env, value* args);

// definition of native vector division function
value* nativeVecDiv(arena* a, environment* env, value* args);

// definition of native vector dot product function
value* nativeVecDot(arena* a, environment* env, value* args);

// definition of native vector cross product function
value* nativeVecCross(arena* a, environment* env, value* args);

// definition of native vector hadamard product function
value* nativeVecHadamard(arena* a, environment* env, value* args);

// definition of native vector norm squared function
value* nativeVecNormSqr(arena* a, environment* env, value* args);

// definition of native vector norm function
value* nativeVecNorm(arena* a, environment* env, value* args);

// definition of native vector normalization function
value* nativeVecNormalized(arena* a, environment* env, value* args);

// definition of native matrix transpose function
value* nativeMatTran(arena* a, environment* env, value* args);

// definition of native matrix determinant function
value* nativeMatDet(arena* a, environment* env, value* args);

// definition of native matrix inverse function
value* nativeMatInv(arena* a, environment* env, value* args);

// definition of native quaternion add function
value* nativeQuatAdd(arena* a, environment* env, value* args);

// definition of native quaternion sub function
value* nativeQuatSub(arena* a, environment* env, value* args);

// definition of native quaternion multiplication function
value* nativeQuatMul(arena* a, environment* env, value* args);

// definition of native quaternion division function
value* nativeQuatDivSca(arena* a, environment* env, value* args);

// definition of native quaternion conjugate function
value* nativeQuatConj(arena* a, environment* env, value* args);

// definition of native quaternion inverse function
value* nativeQuatInv(arena* a, environment* env, value* args);

// definition of native quaternion norm squared function
value* nativeQuatNormSqr(arena* a, environment* env, value* args);

// definition of native quaternion norm function
value* nativeQuatNorm(arena* a, environment* env, value* args);

// definition of native quaternion normalization function
value* nativeQuatNormalized(arena* a, environment* env, value* args);

// definition of native quaternion from angle / axis function
value* nativeQuatAngleAxis(arena* a, environment* env, value* args);

// native quaternion to euler function
value* nativeQuatToEuler(arena* a, environment* env, value* args);

// native euler to quaternion function
value* nativeEulerToQuat(arena* a, environment* env, value* args);

#endif
