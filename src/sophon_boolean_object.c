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

#define BOOLEAN_FUNC_PROP(name, argc)\
	SOPHON_FUNCTION_PROP(name, GLOBAL_ATTRS, boolean_##name##_func, argc)
#define BOOLEAN_FUNC(name)\
	SOPHON_FUNC(boolean_##name##_func)

static BOOLEAN_FUNC(call)
{
	Sophon_Value v = SOPHON_ARG(0);
	Sophon_Bool b;

	b = sophon_value_to_bool(vm, v);

	if (sophon_value_is_undefined(thisv)) {
		sophon_value_set_bool(vm, retv, b);
	} else {
		Sophon_Object *obj;

		sophon_value_to_object(vm, thisv, &obj);
		sophon_value_set_bool(vm, &obj->primv, b);
	}

	return SOPHON_OK;
}

static BOOLEAN_FUNC(toString)
{
	Sophon_Bool b;
	Sophon_String *str;

	if (!sophon_value_is_bool(thisv)) {
		sophon_throw(vm, vm->TypeError, "Value is not a boolean value");
		return SOPHON_ERR_THROW;
	}

	b = sophon_value_to_bool(vm, thisv);
	str = b ? vm->true_str : vm->false_str;

	sophon_value_set_string(vm, retv, str);

	return SOPHON_OK;
}

static BOOLEAN_FUNC(valueOf)
{
	Sophon_Bool b;

	if (!sophon_value_is_bool(thisv)) {
		sophon_throw(vm, vm->TypeError, "Value is not a boolean value");
		return SOPHON_ERR_THROW;
	}

	if (SOPHON_VALUE_IS_BOOL(thisv)) {
		b = SOPHON_VALUE_GET_BOOL(thisv);
	} else {
		Sophon_Object *obj;

		sophon_value_to_object(vm, thisv, &obj);

		b = SOPHON_VALUE_GET_BOOL(obj->primv);
	}

	sophon_value_set_bool(vm, retv, b);

	return SOPHON_OK;
}

static const Sophon_Decl
boolean_prototype_props[] = {
	SOPHON_STRING_PROP([Class], 0, "Boolean"),
	BOOLEAN_FUNC_PROP(toString, 0),
	BOOLEAN_FUNC_PROP(valueOf, 0),
	{NULL}
};

static const Sophon_Decl
boolean_object_props[] = {
	SOPHON_OBJECT_PROP(prototype, 0, boolean_prototype_props),
	SOPHON_FUNCTION_PROP([Call], GLOBAL_ATTRS, boolean_call_func, 1),
	{NULL}
};

