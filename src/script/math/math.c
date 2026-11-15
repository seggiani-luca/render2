#include "math.h"
#include "../../math/math.h"
#include "../lisp/execute.h"
#include "../../exception/exception.h"
#include "../lisp/lisp.h"

// matrix / vector getters / setters
extern value* vectorGet(arena* a, void* vec, int n);
extern void vectorSet(void* vec, int n, const value* val);
extern value* matrixGet(arena* a, void* vec, int n);
extern void matrixSet(void* vec, int n, const value* val);

// typedef for vector arithmetic functions
typedef void (*vecArithmFunc)(float* a, float* b, int n, float* buf);

// typedef for matrix arithmetic functions
typedef void (*matArithmFunc)(float* a, float* b, int n, float* buf);

// tries to get vector, buf must be sized to contain at least a 4D vector
// returns -1 on failure
int isVector(float* buf, value* val) {
	int dim = 0;

	for(int i = 0; i < 4; i++) {
		// check for cons
		if(val->type != VAL_CONS) break;

		// check for number
		value* num = val->cons.car;
		if(num->type != VAL_NUMBER) return -1;

		// advance
		buf[dim++] = num->number;
		val = val->cons.cdr;
	}

	if(val->type != VAL_NIL) return -1;
	if(dim < 2) return -1;

	return dim;
}

// tries to get matrix, buf must be sized to contain at least a 4x4 matrix
// writes column-major
// returns -1 on failure
int isMatrix(float* buf, value* val) {
	int dim = -1;
	int rows = 0;

	for(int i = 0; i < 4; i++) {
		// check for cons
		if(val->type != VAL_CONS) break;

		// check for row
		value* row = val->cons.car;
		float tmp[4];

		int cols = isVector(tmp, row);
		if(cols < 0) return -1;

		if(dim != -1 && cols != dim) return -1;
		dim = cols;

		// transpose into column-major storage
		for(int c = 0; c < cols; c++)
			buf[c * dim + rows] = tmp[c];

		rows++;

		// advance row
		val = val->cons.cdr;
	}

	if(val->type != VAL_NIL) return -1;
	if(dim != rows || dim < 2) return -1;

	return dim;
}

// definition of native vector/matrix arithmetic function
value* nativeVecArithm(
	arena* a,
	environment* env,
	value* args,
	arithmFunc scaFunc,
	vecArithmFunc vecFunc,
	matArithmFunc matFunc
) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Vector arithmetic op. requires first argument");
		throw;
	}

	// get arguments
	value* lhs = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Invalid lhs arguments to vector arithmetic op.");
		throw;
	}

	value* rhs = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Invalid rhs arguments to vector arithmetic op.");
		throw;
	}

	// check if scalars 
	if(lhs->type == VAL_NUMBER
	&& rhs->type == VAL_NUMBER) {
		return makeNumber(a, scaFunc(lhs->number, rhs->number));
	}

	// argument buffers
	mat4 lhsVal;
	mat4 rhsVal;
	mat4 result;
	
	// check if vectors
	int lhsN = isVector((float*)&lhsVal, lhs);
	int rhsN = isVector((float*)&rhsVal, rhs);

	if(lhsN != -1
	&& rhsN != -1
	&& lhsN == rhsN) {
		vecFunc((float*)&lhsVal, (float*)&rhsVal, lhsN, (float*)&result);
		return vectorGet(a, (float*)&result, lhsN);
	}

	// check if matrices
	lhsN = isMatrix((float*)&lhsVal, lhs);
	rhsN = isMatrix((float*)&rhsVal, rhs);
	
	if(lhsN != -1
	&& rhsN != -1
	&& lhsN == rhsN) {
		matFunc((float*)&lhsVal, (float*)&rhsVal, lhsN, (float*)&result);
		return matrixGet(a, (float*)&result, lhsN);
	}

	logEvent(ERROR, EXEC, "Invalid arguments to vector arithmetic op.");
	throw;
}

// definition of native vector add function helpers
extern float add(float a, float b);
void vecAdd(float* a, float* b, int n, float* buf) {
	switch(n) {
		case 2:
			*(float2*)buf = vecAdd2(*(float2*)a, *(float2*)b);
			break;
		case 3:
			*(float3*)buf = vecAdd3(*(float3*)a, *(float3*)b);
			break;
		case 4:
			*(float4*)buf = vecAdd4(*(float4*)a, *(float4*)b);
			break;
		default: break;
	}
}
void matAdd(float* a, float* b, int n, float* buf) {
	switch(n) {
		case 2:
			*(mat2*)buf = matAdd2(*(mat2*)a, *(mat2*)b);
			break;
		case 3:
			*(mat3*)buf = matAdd3(*(mat3*)a, *(mat3*)b);
			break;
		case 4:
			*(mat4*)buf = matAdd4(*(mat4*)a, *(mat4*)b);
			break;
		default: break;
	}
}

value* nativeVecAdd(arena* a, environment* env, value* args) {
	return nativeVecArithm(a, env, args,
		add,
		vecAdd,
		matAdd
	);
}

// definition of native vector sub function helpers
extern float sub(float a, float b);
void vecSub(float* a, float* b, int n, float* buf) {
	switch(n) {
		case 2:
			*(float2*)buf = vecSub2(*(float2*)a, *(float2*)b);
			break;
		case 3:
			*(float3*)buf = vecSub3(*(float3*)a, *(float3*)b);
			break;
		case 4:
			*(float4*)buf = vecSub4(*(float4*)a, *(float4*)b);
			break;
		default: break;
	}
}
void matSub(float* a, float* b, int n, float* buf) {
	switch(n) {
		case 2:
			*(mat2*)buf = matSub2(*(mat2*)a, *(mat2*)b);
			break;
		case 3:
			*(mat3*)buf = matSub3(*(mat3*)a, *(mat3*)b);
			break;
		case 4:
			*(mat4*)buf = matSub4(*(mat4*)a, *(mat4*)b);
			break;
		default: break;
	}
}

value* nativeVecSub(arena* a, environment* env, value* args) {
	return nativeVecArithm(a, env, args,
		sub,
		vecSub,
		matSub
	);
}

// typedef for scalar and vector arithmetic functions
typedef void (*vecScaArithmFunc)(float* a, float b, int n, float* buf);

// typedef for scalar and matrix arithmetic functions
typedef void (*matScaArithmFunc)(float* a, float b, int n, float* buf);

// definition of native scalar and vector/matrix arithmetic function
value* nativeVecScaArithm(
	arena* a,
	environment* env,
	value* args,
	arithmFunc scaFunc,
	vecScaArithmFunc vecFunc,
	matScaArithmFunc matFunc
) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Vector arithmetic op. requires first argument");
		throw;
	}

	// get arguments
	value* lhs = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Invalid lhs arguments to vector arithmetic op.");
		throw;
	}

	value* rhs = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Invalid rhs arguments to vector arithmetic op.");
		throw;
	}

	// check if scalars
	if(lhs->type == VAL_NUMBER
	&& rhs->type == VAL_NUMBER) {
		return makeNumber(a, scaFunc(lhs->number, rhs->number));
	}

	// argument buffers
	mat4 lhsVal;
	mat4 rhsVal;
	mat4 result;

	// check if vectors
	int lhsN = isVector((float*)&lhsVal, lhs);
	int rhsN = isVector((float*)&rhsVal, rhs);

	if(lhsN != -1 && rhs->type == VAL_NUMBER) {
		vecFunc((float*)&lhsVal, rhs->number, lhsN, (float*)&result);
		return vectorGet(a, (float*)&result, lhsN);
	}

	if(rhsN != -1 && lhs->type == VAL_NUMBER) {
		vecFunc((float*)&rhsVal, lhs->number, rhsN, (float*)&result);
		return vectorGet(a, (float*)&result, rhsN);
	}

	// check if matrices
	lhsN = isMatrix((float*)&lhsVal, lhs);
	rhsN = isMatrix((float*)&rhsVal, rhs);

	if(lhsN != -1 && rhs->type == VAL_NUMBER) {
		matFunc((float*)&lhsVal, rhs->number, lhsN, (float*)&result);
		return matrixGet(a, (float*)&result, lhsN);
	}

	if(rhsN != -1 && lhs->type == VAL_NUMBER) {
		matFunc((float*)&rhsVal, lhs->number, rhsN, (float*)&result);
		return matrixGet(a, (float*)&result, rhsN);
	}

	logEvent(ERROR, EXEC, "Invalid arguments to vector arithmetic op.");
	throw;
}

// definition of native vector mul function helper
extern float mul(float a, float b);

value* nativeVecMul(arena* a, environment* env, value* args) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Vector multiplication requires first argument");
		throw;
	}

	// get arguments
	value* lhs = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Invalid lhs arguments to vector multiplication");
		throw;
	}

	value* rhs = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Invalid rhs arguments to vector multiplication");
		throw;
	}

	// check if scalars
	if(lhs->type == VAL_NUMBER
	&& rhs->type == VAL_NUMBER) {
		return makeNumber(a, mul(lhs->number, rhs->number));
	}

	// argument buffers
	mat4 lhsVal;
	mat4 rhsVal;
	mat4 result;

	// check if vector by scalar
	int lhsN = isVector((float*)&lhsVal, lhs);

	if(lhsN != -1 && rhs->type == VAL_NUMBER) {
		switch(lhsN) {
			case 2:
				*(float2*)&result = vecMul2(*(float2*)&lhsVal, rhs->number);
				break;
			case 3:
				*(float3*)&result = vecMul3(*(float3*)&lhsVal, rhs->number);
				break;
			case 4:
				*(float4*)&result = vecMul4(*(float4*)&lhsVal, rhs->number);
				break;
			default: {
				logEvent(ERROR, EXEC, "Invalid argument size to vector multiplication");
				throw;
			}
		}

		return vectorGet(a, (float*)&result, lhsN);
	}

	// check if matrix by scalar
	lhsN = isMatrix((float*)&lhsVal, lhs);

	if(lhsN != -1 && rhs->type == VAL_NUMBER) {
		switch(lhsN) {
			case 2:
				*(mat2*)&result = matMulSca2(*(mat2*)&lhsVal, rhs->number);
				break;
			case 3:
				*(mat3*)&result = matMulSca3(*(mat3*)&lhsVal, rhs->number);
				break;
			case 4:
				*(mat4*)&result = matMulSca4(*(mat4*)&lhsVal, rhs->number);
				break;
			default: {
				logEvent(ERROR, EXEC, "Invalid argument size to matrix multiplication");
				throw;
			}
		}

		return matrixGet(a, (float*)&result, lhsN);
	}

	// check if matrix by vector
	lhsN = isMatrix((float*)&lhsVal, lhs);
	int rhsN = isVector((float*)&rhsVal, rhs);

	if(lhsN != -1
	&& rhsN != -1
	&& lhsN == rhsN) {
		switch(lhsN) {
			case 2:
				*(float2*)&result = matVec2(*(mat2*)&lhsVal, *(float2*)&rhsVal);
				break;
			case 3:
				*(float3*)&result = matVec3(*(mat3*)&lhsVal, *(float3*)&rhsVal);
				break;
			case 4:
				*(float4*)&result = matVec4(*(mat4*)&lhsVal, *(float4*)&rhsVal);
				break;
			default: {
				logEvent(ERROR, EXEC, "Invalid argument sizes to matrix vector multiplication");
				throw;
			}
		}

		return vectorGet(a, (float*)&result, lhsN);
	}

	// check if matrix by matrix
	lhsN = isMatrix((float*)&lhsVal, lhs);
	rhsN = isMatrix((float*)&rhsVal, rhs);

	if(lhsN != -1
	&& rhsN != -1
	&& lhsN == rhsN) {
		switch(lhsN) {
			case 2:
				*(mat2*)&result = matMul2(*(mat2*)&lhsVal, *(mat2*)&rhsVal);
				break;
			case 3:
				*(mat3*)&result = matMul3(*(mat3*)&lhsVal, *(mat3*)&rhsVal);
				break;
			case 4:
				*(mat4*)&result = matMul4(*(mat4*)&lhsVal, *(mat4*)&rhsVal);
				break;
			default: {
				logEvent(ERROR, EXEC, "Invalid argument sizes to matrix multiplication");
				throw;
			}
		}

		return matrixGet(a, (float*)&result, lhsN);
	}

	logEvent(ERROR, EXEC, "Invalid arguments to vector multiplication");
	throw;
}

// definition of native vector division function helpers
extern float divs(float a, float b);
void vecDiv(float* a, float b, int n, float* buf) {
	switch(n) {
		case 2:
			*(float2*)buf = vecDiv2(*(float2*)a, b);
			break;
		case 3:
			*(float3*)buf = vecDiv3(*(float3*)a, b);
			break;
		case 4:
			*(float4*)buf = vecDiv4(*(float4*)a, b);
			break;
		default: break;
	}
}
void matDiv(float* a, float b, int n, float* buf) {
	switch(n) {
		case 2:
			*(mat2*)buf = matDivSca2(*(mat2*)a, b);
			break;
		case 3:
			*(mat3*)buf = matDivSca3(*(mat3*)a, b);
			break;
		case 4:
			*(mat4*)buf = matDivSca4(*(mat4*)a, b);
			break;
		default: break;
	}
}

value* nativeVecDiv(arena* a, environment* env, value* args) {
	return nativeVecScaArithm(a, env, args,
		divs,
		vecDiv,
		matDiv	
	);
}

value* nativeVecDot(arena* a, environment* env, value* args) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Vector dot requires first argument");
		throw;
	}

	// get arguments
	value* lhs = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Invalid lhs arguments to vector dot");
		throw;
	}

	value* rhs = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Invalid rhs arguments to vector dot");
		throw;
	}

	// argument buffers
	mat4 lhsVal;
	mat4 rhsVal;
	float result;
	
	// check if vectors
	int lhsN = isVector((float*)&lhsVal, lhs);
	int rhsN = isVector((float*)&rhsVal, rhs);

	if(lhsN != -1
	&& rhsN != -1
	&& lhsN == rhsN) {
		switch(lhsN) {
			case 2:
				result = vecDot2(*(float2*)&lhsVal, *(float2*)&rhsVal);
				break;
			case 3:
				result = vecDot3(*(float3*)&lhsVal, *(float3*)&rhsVal);
				break;
			case 4:
				result = vecDot4(*(float4*)&lhsVal, *(float4*)&rhsVal);
				break;
			default: {
				logEvent(ERROR, EXEC, "Invalid arguments sizes to vector dot");
				throw;
			}
		}

		return makeNumber(a, result);
	}

	logEvent(ERROR, EXEC, "Invalid arguments to vector dot");
	throw;
}

value* nativeVecCross(arena* a, environment* env, value* args) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Vector cross requires first argument");
		throw;
	}

	// get arguments
	value* lhs = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Invalid lhs arguments to vector cross");
		throw;
	}

	value* rhs = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Invalid rhs arguments to vector cross");
		throw;
	}

	// argument buffers
	mat4 lhsVal;
	mat4 rhsVal;
	mat4 result;
	
	// check if vectors
	int lhsN = isVector((float*)&lhsVal, lhs);
	int rhsN = isVector((float*)&rhsVal, rhs);

	if(lhsN != -1
	&& rhsN != -1
	&& lhsN == rhsN) {
		switch(lhsN) {
			case 3:
				*(float3*)&result = vecCross3(*(float3*)&lhsVal, *(float3*)&rhsVal);
				break;
			default: {
				logEvent(ERROR, EXEC, "Invalid argument sizes to vector cross");
				throw;
			}
		}

		return vectorGet(a, (float*)&result, lhsN);
	}

	logEvent(ERROR, EXEC, "Invalid arguments to vector cross");
	throw;
}

value* nativeVecHadamard(arena* a, environment* env, value* args) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Vector hadamard requires first argument");
		throw;
	}

	// get arguments
	value* lhs = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Invalid lhs arguments to vector hadamard");
		throw;
	}

	value* rhs = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Invalid rhs arguments to vector hadamard");
		throw;
	}

	// argument buffers
	mat4 lhsVal;
	mat4 rhsVal;
	mat4 result;
	
	// check if vectors
	int lhsN = isVector((float*)&lhsVal, lhs);
	int rhsN = isVector((float*)&rhsVal, rhs);

	if(lhsN != -1
	&& rhsN != -1
	&& lhsN == rhsN) {
		switch(lhsN) {
			case 2:
				*(float2*)&result = vecHad2(*(float2*)&lhsVal, *(float2*)&rhsVal);
				break;
			case 3:
				*(float3*)&result = vecHad3(*(float3*)&lhsVal, *(float3*)&rhsVal);
				break;
			case 4:
				*(float4*)&result = vecHad4(*(float4*)&lhsVal, *(float4*)&rhsVal);
				break;
			default: {
				logEvent(ERROR, EXEC, "Invalid argument sizes to vector hadamard");
				throw;
			}
		}

		return vectorGet(a, (float*)&result, lhsN);
	}

	logEvent(ERROR, EXEC, "Invalid arguments to vector hadamard");
	throw;
}

value* nativeVecNormSqr(arena* a, environment* env, value* args) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Vector norm squared requires first ");
		throw;
	}

	// get arguments
	value* vec = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Invalid argument to vector norm squared");
		throw;
	}

	// argument buffers
	mat4 vecVal;
	float result;
	
	// check if vector
	int vecN = isVector((float*)&vecVal, vec);

	if(vecN != -1) {
		switch(vecN) {
			case 2:
				result = vecNormSqr2(*(float2*)&vecVal);
				break;
			case 3:
				result = vecNormSqr3(*(float3*)&vecVal);
				break;
			case 4:
				result = vecNormSqr4(*(float4*)&vecVal);
				break;
			default: {
				logEvent(ERROR, EXEC, "Invalid argument size to vector norm squared");
				throw;
			}
		}

		return makeNumber(a, result);
	}

	logEvent(ERROR, EXEC, "Invalid argument to vector norm squared");
	throw;
}

value* nativeVecNorm(arena* a, environment* env, value* args) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Vector norm requires first argument");
		throw;
	}

	// get arguments
	value* vec = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Invalid argument to vector norm");
		throw;
	}

	// argument buffers
	mat4 vecVal;
	float result;
	
	// check if vector
	int vecN = isVector((float*)&vecVal, vec);

	if(vecN != -1) {
		switch(vecN) {
			case 2:
				result = vecNorm2(*(float2*)&vecVal);
				break;
			case 3:
				result = vecNorm3(*(float3*)&vecVal);
				break;
			case 4:
				result = vecNorm4(*(float4*)&vecVal);
				break;
			default: {
				logEvent(ERROR, EXEC, "Invalid argument size to vector norm");
				throw;
			}
		}

		return makeNumber(a, result);
	}

	logEvent(ERROR, EXEC, "Invalid argument to vector norm");
	throw;
}

value* nativeVecNormalized(arena* a, environment* env, value* args) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Vector normalize requires first argument");
		throw;
	}

	// get arguments
	value* vec = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Invalid argument to vector normalize");
		throw;
	}

	// argument buffers
	mat4 vecVal;
	mat4 result;
	
	// check if vector
	int vecN = isVector((float*)&vecVal, vec);

	if(vecN != -1) {
		switch(vecN) {
			case 2:
				*(float2*)&result = vecNormalized2(*(float2*)&vecVal);
				break;
			case 3:
				*(float3*)&result = vecNormalized3(*(float3*)&vecVal);
				break;
			case 4:
				*(float4*)&result = vecNormalized4(*(float4*)&vecVal);
				break;
			default: {
				logEvent(ERROR, EXEC, "Invalid arguments sizes to vector dot");
				throw;
			}
		}

		return vectorGet(a, (float*)&result, vecN);
	}

	logEvent(ERROR, EXEC, "Invalid arguments to vector dot");
	throw;
}

value* nativeMatTran(arena* a, environment* env, value* args) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Matrix transpose requires first argument");
		throw;
	}

	// get arguments
	value* mat = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Invalid argument to matrix transpose");
		throw;
	}

	// argument buffers
	mat4 matVal;
	mat4 result;

	// check if matrix
	int matN = isMatrix((float*)&matVal, mat);

	if(matN != -1) {
		switch(matN) {
			case 2:
				*(mat2*)&result = matTran2(*(mat2*)&matVal);
				break;
			case 3:
				*(mat3*)&result = matTran3(*(mat3*)&matVal);
				break;
			case 4:
				*(mat4*)&result = matTran4(*(mat4*)&matVal);
				break;
			default: {
				logEvent(ERROR, EXEC, "Invalid argument size to matrix transpose");
				throw;
			}
		}

		return matrixGet(a, (float*)&result, matN);
	}

	logEvent(ERROR, EXEC, "Invalid argument to matrix transpose");
	throw;
}

value* nativeMatDet(arena* a, environment* env, value* args) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Matrix determinant requires first argument");
		throw;
	}

	// get arguments
	value* mat = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Invalid argument to matrix determinant");
		throw;
	}

	// argument buffers
	mat4 matVal;
	float result;

	// check if matrix
	int matN = isMatrix((float*)&matVal, mat);

	if(matN != -1) {
		switch(matN) {
			case 2:
				result = matDet2(*(mat2*)&matVal);
				break;
			case 3:
				result = matDet3(*(mat3*)&matVal);
				break;
			case 4:
				result = matDet4(*(mat4*)&matVal);
				break;
			default: {
				logEvent(ERROR, EXEC, "Invalid argument size to matrix determinant");
				throw;
			}
		}

		return makeNumber(a, result);
	}

	logEvent(ERROR, EXEC, "Invalid argument to matrix determinant");
	throw;
}

value* nativeMatInv(arena* a, environment* env, value* args) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Matrix inverse requires first argument");
		throw;
	}

	// get arguments
	value* mat = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Invalid argument to matrix inverse");
		throw;
	}

	// argument buffers
	mat4 matVal;
	mat4 result;

	// check if matrix
	int matN = isMatrix((float*)&matVal, mat);

	if(matN != -1) {
		switch(matN) {
			case 2:
				*(mat2*)&result = matInv2(*(mat2*)&matVal);
				break;
			case 3:
				*(mat3*)&result = matInv3(*(mat3*)&matVal);
				break;
			case 4:
				*(mat4*)&result = matInv4(*(mat4*)&matVal);
				break;
			default: {
				logEvent(ERROR, EXEC, "Invalid argument size to matrix inverse");
				throw;
			}
		}

		return matrixGet(a, (float*)&result, matN);
	}

	logEvent(ERROR, EXEC, "Invalid argument to matrix inverse");
	throw;
}

value* nativeQuatAdd(arena* a, environment* env, value* args) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Quaternion add requires first argument");
		throw;
	}

	// get arguments
	value* lhs = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Invalid lhs arguments to quaternion add");
		throw;
	}

	value* rhs = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Invalid rhs arguments to quaternion add");
		throw;
	}

	// argument buffers
	quat lhsVal;
	quat rhsVal;
	quat result;

	// check if quaternions
	if(isVector((float*)&lhsVal, lhs) == 4
	&& isVector((float*)&rhsVal, rhs) == 4) {
		result = quatAdd(lhsVal, rhsVal);
		return vectorGet(a, (float*)&result, 4);
	}

	logEvent(ERROR, EXEC, "Invalid arguments to quaternion add");
	throw;
}

value* nativeQuatSub(arena* a, environment* env, value* args) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Quaternion sub requires first argument");
		throw;
	}

	// get arguments
	value* lhs = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Invalid lhs arguments to quaternion sub");
		throw;
	}

	value* rhs = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Invalid rhs arguments to quaternion sub");
		throw;
	}

	// argument buffers
	quat lhsVal;
	quat rhsVal;
	quat result;

	// check if quaternions
	if(isVector((float*)&lhsVal, lhs) == 4
	&& isVector((float*)&rhsVal, rhs) == 4) {
		result = quatSub(lhsVal, rhsVal);
		return vectorGet(a, (float*)&result, 4);
	}

	logEvent(ERROR, EXEC, "Invalid arguments to quaternion sub");
	throw;
}

value* nativeQuatMul(arena* a, environment* env, value* args) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Quaternion multiplication requires first argument");
		throw;
	}

	// get arguments
	value* lhs = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Invalid lhs arguments to quaternion multiplication");
		throw;
	}

	value* rhs = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Invalid rhs arguments to quaternion multiplication");
		throw;
	}

	// argument buffers
	quat lhsVal;
	quat rhsVal;
	quat result;

	// check if quaternion by scalar
	if(rhs->type == VAL_NUMBER
	&& isVector((float*)&lhsVal, lhs) == 4) {
		result = quatMulSca(lhsVal, rhs->number);
		return vectorGet(a, (float*)&result, 4);
	}

	// check if quaternion by quaternion
	if(isVector((float*)&lhsVal, lhs) == 4
	&& isVector((float*)&rhsVal, rhs) == 4) {
		result = quatMul(lhsVal, rhsVal);
		return vectorGet(a, (float*)&result, 4);
	}

	logEvent(ERROR, EXEC, "Invalid arguments to quaternion multiplication");
	throw;
}

value* nativeQuatDivSca(arena* a, environment* env, value* args) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Quaternion scalar division requires first argument");
		throw;
	}

	// get arguments
	value* quatVal = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Invalid quaternion arguments to scalar division");
		throw;
	}

	value* sca = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Invalid scalar arguments to quaternion division");
		throw;
	}

	// check scalar
	if(sca->type != VAL_NUMBER) {
		logEvent(ERROR, EXEC, "Quaternion scalar division requires a scalar");
		throw;
	}

	// argument buffers
	quat quatBuf;
	quat result;

	// check if quaternion
	if(isVector((float*)&quatBuf, quatVal) == 4) {
		result = quatDivSca(quatBuf, sca->number);
		return vectorGet(a, (float*)&result, 4);
	}

	logEvent(ERROR, EXEC, "Invalid quaternion argument to scalar division");
	throw;
}

value* nativeQuatConj(arena* a, environment* env, value* args) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Quaternion conjugate requires first argument");
		throw;
	}

	// get argument
	value* quatVal = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Invalid argument to quaternion conjugate");
		throw;
	}

	// argument buffers
	quat quatBuf;
	quat result;

	// check if quaternion
	if(isVector((float*)&quatBuf, quatVal) == 4) {
		result = quatConj(quatBuf);
		return vectorGet(a, (float*)&result, 4);
	}

	logEvent(ERROR, EXEC, "Invalid quaternion argument to conjugate");
	throw;
}

value* nativeQuatInv(arena* a, environment* env, value* args) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Quaternion inverse requires first argument");
		throw;
	}

	// get argument
	value* quatVal = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Invalid argument to quaternion inverse");
		throw;
	}

	// argument buffers
	quat quatBuf;
	quat result;

	// check if quaternion
	if(isVector((float*)&quatBuf, quatVal) == 4) {
		result = quatInv(quatBuf);
		return vectorGet(a, (float*)&result, 4);
	}

	logEvent(ERROR, EXEC, "Invalid quaternion argument to inverse");
	throw;
}

value* nativeQuatNormSqr(arena* a, environment* env, value* args) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Quaternion norm squared requires first argument");
		throw;
	}

	// get argument
	value* quatVal = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Invalid argument to quaternion norm squared");
		throw;
	}

	// argument buffers
	quat quatBuf;
	float result;

	// check if quaternion
	if(isVector((float*)&quatBuf, quatVal) == 4) {
		result = quatNormSqr(quatBuf);
		return makeNumber(a, result);
	}

	logEvent(ERROR, EXEC, "Invalid quaternion argument to norm squared");
	throw;
}

value* nativeQuatNorm(arena* a, environment* env, value* args) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Quaternion norm requires first argument");
		throw;
	}

	// get argument
	value* quatVal = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Invalid argument to quaternion norm");
		throw;
	}

	// argument buffers
	quat quatBuf;
	float result;

	// check if quaternion
	if(isVector((float*)&quatBuf, quatVal) == 4) {
		result = quatNorm(quatBuf);
		return makeNumber(a, result);
	}

	logEvent(ERROR, EXEC, "Invalid quaternion argument to norm");
	throw;
}

value* nativeQuatNormalized(arena* a, environment* env, value* args) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Quaternion normalize requires first argument");
		throw;
	}

	// get argument
	value* quatVal = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Invalid argument to quaternion normalize");
		throw;
	}

	// argument buffers
	quat quatBuf;
	quat result;

	// check if quaternion
	if(isVector((float*)&quatBuf, quatVal) == 4) {
		result = quatNormalized(quatBuf);
		return vectorGet(a, (float*)&result, 4);
	}

	logEvent(ERROR, EXEC, "Invalid quaternion argument to normalize");
	throw;
}

value* nativeQuatAngleAxis(arena* a, environment* env, value* args) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Quaternion angle-axis requires first argument");
		throw;
	}

	// get arguments
	value* angle = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Invalid angle arguments to quaternion angle-axis");
		throw;
	}

	value* axis = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Invalid axis arguments to quaternion angle-axis");
		throw;
	}

	// check angle
	if(angle->type != VAL_NUMBER) {
		logEvent(ERROR, EXEC, "Quaternion angle-axis requires a scalar angle");
		throw;
	}

	// argument buffers
	float3 axisVal;
	quat result;

	// check if axis is a 3D vector
	if(isVector((float*)&axisVal, axis) == 3) {
		result = quatAngleAxis(angle->number, axisVal);
		return vectorGet(a, (float*)&result, 4);
	}

	logEvent(ERROR, EXEC, "Invalid axis argument to quaternion angle-axis");
	throw;
}

value* nativeQuatToEuler(arena* a, environment* env, value* args) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Quaternion to euler requires first argument");
		throw;
	}

	// get argument
	value* quatVal = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Invalid argument to quaternion to euler");
		throw;
	}

	// argument buffers
	quat quatBuf;
	float3 result;

	// check if quaternion
	if(isVector((float*)&quatBuf, quatVal) == 4) {
		result = quatToEuler(quatBuf);
		return vectorGet(a, (float*)&result, 3);
	}

	logEvent(ERROR, EXEC, "Invalid quaternion argument to euler conversion");
	throw;
}

value* nativeEulerToQuat(arena* a, environment* env, value* args) {
	if(args->type != VAL_CONS) {
		logEvent(ERROR, EXEC, "Euler to quaternion requires first argument");
		throw;
	}

	// get argument
	value* eulerVal = evaluateValue(a, env, args->cons.car);
	args = args->cons.cdr;

	if(args->type != VAL_NIL) {
		logEvent(ERROR, EXEC, "Invalid argument to euler to quaternion");
		throw;
	}

	// argument buffers
	float3 eulerBuf;
	quat result;

	// check if euler angles
	if(isVector((float*)&eulerBuf, eulerVal) == 3) {
		result = eulerToQuat(eulerBuf);
		return vectorGet(a, (float*)&result, 4);
	}

	logEvent(ERROR, EXEC, "Invalid euler argument to quaternion conversion");
	throw;
}
