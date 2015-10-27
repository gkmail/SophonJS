/******************************************************************************
 *                SophonJS: An embedded javascript engine                     *
 *                                                                            *
 * Copyright (c) 2015 Gong Ke                                                 *
 * All rights reserved.                                                       *
 *                                                                            *
 * Redistribution and use in source and binary forms, with or without         *
 * modification, are permitted provided that the following conditions are     *
 * met:                                                                       *
 * 1.Redistributions of source code must retain the above copyright notice,   *
 * this list of conditions and the following disclaimer.                      *
 * 2.Redistributions in binary form must reproduce the above copyright        *
 * notice, this list of conditions and the following disclaimer in the        *
 * documentation and/or other materials provided with the distribution.       *
 * 3.Neither the name of the Gong Ke; nor the names of its contributors may   *
 * be used to endorse or promote products derived from this software without  *
 * specific prior written permission.                                         *
 *                                                                            *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS    *
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,  *
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR     *
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR           *
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,      *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,        *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR         *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF     *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING       *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               *
 *****************************************************************************/

#define MATH_FUNC_PROP(name, argc)\
	SOPHON_FUNCTION_PROP(name, GLOBAL_ATTRS, math_##name##_func, argc)
#define MATH_FUNC(name)\
	SOPHON_FUNC(math_##name##_func)


#define MATH_UNARY_FUNC(name) \
static MATH_FUNC(name) \
{ \
	Sophon_Number n;\
	Sophon_Result r;\
	if ((r = sophon_value_to_number(vm, SOPHON_ARG(0), &n)) != SOPHON_OK)\
		return r;\
	n = sophon_##name(n);\
	sophon_value_set_number(vm, retv, n);\
	return SOPHON_OK;\
}

#define MATH_BIN_FUNC(name) \
static MATH_FUNC(name) \
{ \
	Sophon_Number n1, n2, nr;\
	Sophon_Result r;\
	if ((r = sophon_value_to_number(vm, SOPHON_ARG(0), &n1)) != SOPHON_OK)\
		return r;\
	if ((r = sophon_value_to_number(vm, SOPHON_ARG(1), &n2)) != SOPHON_OK)\
		return r;\
	nr = sophon_##name(n1, n2);\
	sophon_value_set_number(vm, retv, nr);\
	return SOPHON_OK;\
}

MATH_UNARY_FUNC(abs)
MATH_UNARY_FUNC(acos)
MATH_UNARY_FUNC(asin)
MATH_UNARY_FUNC(atan)
MATH_UNARY_FUNC(ceil)
MATH_UNARY_FUNC(cos)
MATH_UNARY_FUNC(exp)
MATH_UNARY_FUNC(floor)
MATH_UNARY_FUNC(log)
MATH_UNARY_FUNC(sin)
MATH_UNARY_FUNC(round)
MATH_UNARY_FUNC(tan)
MATH_UNARY_FUNC(sqrt)

MATH_BIN_FUNC(atan2)
MATH_BIN_FUNC(pow)

static MATH_FUNC(random)
{
	Sophon_Number r = sophon_random();
	sophon_value_set_number(vm, retv, r);
	return SOPHON_OK;
}

static MATH_FUNC(max)
{
	Sophon_Double max;

	if (argc) {
		Sophon_Int id;
		Sophon_Number d;
		Sophon_Result r;

		if ((r = sophon_value_to_number(vm, SOPHON_ARG(0), &max))
					!= SOPHON_OK)
			return r;
		for (id = 1; id < argc; id++) {
			if ((r = sophon_value_to_number(vm, SOPHON_ARG(id), &d))
						!= SOPHON_OK)
				return r;
			max = SOPHON_MAX(max, d);
		}
	} else {
		max = -SOPHON_INFINITY;
	}

	sophon_value_set_number(vm, retv, max);
	return SOPHON_OK;
}

static MATH_FUNC(min)
{
	Sophon_Number min;

	if (argc) {
		Sophon_Int id;
		Sophon_Number d;
		Sophon_Result r;

		if ((r = sophon_value_to_number(vm, SOPHON_ARG(0), &min))
					!= SOPHON_OK)
			return r;
		for (id = 1; id < argc; id++) {
			if ((r = sophon_value_to_number(vm, SOPHON_ARG(id), &d))
						!= SOPHON_OK)
				return r;

			min = SOPHON_MIN(min, d);
		}
	} else {
		min = SOPHON_INFINITY;
	}

	sophon_value_set_number(vm, retv, min);
	return SOPHON_OK;
}

static const Sophon_Decl
math_object_props[] = {
	SOPHON_DOUBLE_PROP(E,      0, 2.7182818284590452354),
	SOPHON_DOUBLE_PROP(LN10,   0, 2.30258509299404568402),
	SOPHON_DOUBLE_PROP(LN2,    0, 0.69314718055994530942),
	SOPHON_DOUBLE_PROP(LOG2E,  0, 1.4426950408889634074),
	SOPHON_DOUBLE_PROP(LOG10E, 0, 0.43429448190325182765),
	SOPHON_DOUBLE_PROP(PI,     0, 3.14159265358979323846),
	SOPHON_DOUBLE_PROP(SQRT1_2,0, 0.70710678118654752440),
	SOPHON_DOUBLE_PROP(SQRT2,  0, 1.41421356237309504880),
	MATH_FUNC_PROP(abs, 1),
	MATH_FUNC_PROP(acos, 1),
	MATH_FUNC_PROP(asin, 1),
	MATH_FUNC_PROP(atan, 1),
	MATH_FUNC_PROP(atan2, 2),
	MATH_FUNC_PROP(ceil, 1),
	MATH_FUNC_PROP(cos, 1),
	MATH_FUNC_PROP(exp, 1),
	MATH_FUNC_PROP(floor, 1),
	MATH_FUNC_PROP(log, 1),
	MATH_FUNC_PROP(max, 2),
	MATH_FUNC_PROP(min, 2),
	MATH_FUNC_PROP(pow, 2),
	MATH_FUNC_PROP(random, 0),
	MATH_FUNC_PROP(round, 1),
	MATH_FUNC_PROP(sin, 1),
	MATH_FUNC_PROP(sqrt, 1),
	MATH_FUNC_PROP(tan, 1),
	{NULL}
};

