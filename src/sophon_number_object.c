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

#define NUMBER_FUNC_PROP(name, argc)\
	SOPHON_FUNCTION_PROP(name, GLOBAL_ATTRS, number_##name##_func, argc)
#define NUMBER_FUNC(name)\
	SOPHON_FUNC(number_##name##_func)

static NUMBER_FUNC(call)
{
	Sophon_Value v = SOPHON_ARG(0);
	Sophon_Number d;
	Sophon_Result r;

	if (argc) {
		if ((r = sophon_value_to_number(vm, v, &d)) != SOPHON_OK)
			return r;
	}else {
		d = 0;
	}

	if (sophon_value_is_undefined(thisv)) {
		sophon_value_set_number(vm, retv, d);
	} else {
		Sophon_Object *obj;

		if ((r = sophon_value_to_object(vm, thisv, &obj)) != SOPHON_OK)
			return r;

		sophon_value_set_number(vm, &obj->primv, d);
	}

	return SOPHON_OK;
}

static NUMBER_FUNC(toString)
{
	Sophon_Value rv = SOPHON_ARG(0);
	Sophon_Number n;
	Sophon_Int radix;
	Sophon_String *str;
	Sophon_Result r;

	if (!sophon_value_is_number(thisv)) {
		sophon_throw(vm, vm->TypeError, "Value is not a number");
		return SOPHON_ERR_THROW;
	}

	if (sophon_value_is_undefined(rv)) {
		radix = 10;
	} else {
		if ((r = sophon_value_to_int(vm, rv, &radix)) != SOPHON_OK)
			return r;

		if ((radix < 2) || (radix > 36)) {
			sophon_throw(vm, vm->RangeError, "Radix should be in 2 ~ 36");
			return SOPHON_ERR_THROW;
		}
	}

	sophon_value_to_number(vm, thisv, &n);

	str = sophon_dtostr(vm, n, SOPHON_D2STR_RADIX, radix);

	sophon_value_set_string(vm, retv, str);

	return SOPHON_OK;
}

static NUMBER_FUNC(toLocaleString)
{
	return number_toString_func(vm, thisv, argv, argc, retv);
}

static NUMBER_FUNC(valueOf)
{
	Sophon_Number d;

	if (!sophon_value_is_number(thisv)) {
		sophon_throw(vm, vm->TypeError, "Value is not a number");
		return SOPHON_ERR_THROW;
	}

	sophon_value_to_number(vm, thisv, &d);
	sophon_value_set_number(vm, retv, d);

	return SOPHON_OK;
}

static NUMBER_FUNC(toFixed)
{
	Sophon_Value fv = SOPHON_ARG(0);
	Sophon_Number n;
	Sophon_Int f;
	Sophon_String *str;
	Sophon_Result r;

	if (!sophon_value_is_number(thisv)) {
		sophon_throw(vm, vm->TypeError, "Value is not a number");
		return SOPHON_ERR_THROW;
	}

	if (sophon_value_is_undefined(fv)) {
		f = 0;
	} else {
		if ((r = sophon_value_to_int(vm, fv, &f)) != SOPHON_OK)
			return r;

		if ((f < 0) || (f > 20)) {
			sophon_throw(vm, vm->RangeError, "Fraction should be in 0 ~ 20");
			return SOPHON_ERR_THROW;
		}
	}

	sophon_value_to_number(vm, thisv, &n);

	str = sophon_dtostr(vm, n, SOPHON_D2STR_FIXED, f);

	sophon_value_set_string(vm, retv, str);

	return SOPHON_OK;
}

static NUMBER_FUNC(toExponential)
{
	Sophon_Value fv = SOPHON_ARG(0);
	Sophon_Number n;
	Sophon_Int f;
	Sophon_String *str;
	Sophon_Result r;

	if (!sophon_value_is_number(thisv)) {
		sophon_throw(vm, vm->TypeError, "Value is not a number");
		return SOPHON_ERR_THROW;
	}

	if (sophon_value_is_undefined(fv)) {
		f = -1;
	} else {
		if ((r = sophon_value_to_int(vm, fv, &f)) != SOPHON_OK)
			return r;

		if ((f < 0) || (f > 20)) {
			sophon_throw(vm, vm->RangeError, "Fraction should be in 0 ~ 20");
			return SOPHON_ERR_THROW;
		}
	}

	sophon_value_to_number(vm, thisv, &n);

	str = sophon_dtostr(vm, n, SOPHON_D2STR_EXP, f);

	sophon_value_set_string(vm, retv, str);

	return SOPHON_OK;
}

static NUMBER_FUNC(toPrecision)
{
	Sophon_Value pv = SOPHON_ARG(0);
	Sophon_Number n;
	Sophon_Int p;
	Sophon_String *str;
	Sophon_Result r;

	if (!sophon_value_is_number(thisv)) {
		sophon_throw(vm, vm->TypeError, "Value is not a number");
		return SOPHON_ERR_THROW;
	}

	if (sophon_value_is_undefined(pv)) {
		return number_toString_func(vm, thisv, NULL, 0, retv);
	} else {
		if ((r = sophon_value_to_int(vm, pv, &p)) != SOPHON_OK)
			return r;

		if ((p < 1) || (p > 21)) {
			sophon_throw(vm, vm->RangeError, "Precision should be in 1 ~ 21");
			return SOPHON_ERR_THROW;
		}
	}

	sophon_value_to_number(vm, thisv, &n);

	str = sophon_dtostr(vm, n, SOPHON_D2STR_PREC, p);

	sophon_value_set_string(vm, retv, str);

	return SOPHON_OK;
}

static const Sophon_Decl
number_prototype_props[] = {
	SOPHON_STRING_PROP([Class], 0, "Number"),
	NUMBER_FUNC_PROP(toString, 0),
	NUMBER_FUNC_PROP(toLocaleString, 0),
	NUMBER_FUNC_PROP(valueOf, 0),
	NUMBER_FUNC_PROP(toFixed, 1),
	NUMBER_FUNC_PROP(toExponential, 1),
	NUMBER_FUNC_PROP(toPrecision, 1),
	{NULL}
};

static const Sophon_Decl
number_object_props[] = {
	SOPHON_OBJECT_PROP(prototype, 0, number_prototype_props),
	SOPHON_FUNCTION_PROP([Call], GLOBAL_ATTRS, number_call_func, 1),
	SOPHON_DOUBLE_PROP(MAX_VALUE, 0, 1.7976931348623157e+308),
	SOPHON_DOUBLE_PROP(MIN_VALUE, 0, 5e-324),
	SOPHON_DOUBLE_PROP(NaN, 0, SOPHON_NAN),
	SOPHON_DOUBLE_PROP(NEGATIVE_INFINITY, 0, -SOPHON_INFINITY),
	SOPHON_DOUBLE_PROP(POSITIVE_INFINITY, 0, SOPHON_INFINITY),
	{NULL}
};

