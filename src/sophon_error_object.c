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

static SOPHON_FUNC(error_toString_func)
{
	Sophon_String *name;
	Sophon_String *msg;
	Sophon_String *str, *sep;
	Sophon_Value v;
	Sophon_Result r;

	sophon_value_get(vm, thisv, SOPHON_VALUE_GC(vm->name_str),
			&v, 0);
	if (sophon_value_is_undefined(v)) {
		name = vm->empty_str;
	} else {
		if ((r = sophon_value_to_string(vm, v, &name)) != SOPHON_OK)
			return r;
	}

	sophon_value_get(vm, thisv, SOPHON_VALUE_GC(vm->message_str),
			&v, 0);
	if (sophon_value_is_undefined(v)) {
		msg = vm->empty_str;
	} else {
		if ((r = sophon_value_to_string(vm, v, &msg)) != SOPHON_OK)
			return r;
	}

	if ((name == vm->empty_str) && (msg == vm->empty_str)) {
		str = vm->Error_str;
	} else if (name == vm->empty_str) {
		str = msg;
	} else if (msg == vm->empty_str) {
		str = name;
	} else {
		str = name;
		sep = vm->colons_str;
		str = sophon_string_concat(vm, str, sep);
		str = sophon_string_concat(vm, str, msg);
	}

	sophon_value_set_string(vm, retv, str);
	return SOPHON_OK;
}

static const Sophon_Decl
Error_prototype_props[] = {
	SOPHON_STRING_PROP([Class], 0, "Error"),
	SOPHON_FUNCTION_PROP(toString, GLOBAL_ATTRS, error_toString_func, 0),
	{NULL}
};

#define DECL_ERROR_OBJECT_PROPS(name)\
static SOPHON_FUNC(name##_call_func)\
{\
	Sophon_Object *obj;\
	Sophon_String *msg;\
	Sophon_Result r;\
	if (sophon_value_is_undefined(thisv)) {\
		obj = sophon_object_create(vm);\
	} else {\
		if ((r = sophon_value_to_object(vm, thisv, &obj)) != SOPHON_OK)\
			return r;\
	}\
	if (sophon_value_is_undefined(SOPHON_ARG(0))) {\
		msg = vm->empty_str;\
	} else {\
		if ((r = sophon_value_to_string(vm, SOPHON_ARG(0), &msg))\
				!= SOPHON_OK)\
			return r;\
	}\
	sophon_value_set_object(vm, &obj->protov, vm->Error);\
	sophon_value_define_prop(vm, SOPHON_VALUE_GC(obj),\
			SOPHON_VALUE_GC(vm->name_str),\
			SOPHON_VALUE_GC(vm->name##_str),\
			SOPHON_VALUE_UNDEFINED,\
			0, SOPHON_FL_HAVE_VALUE);\
	sophon_value_define_prop(vm, SOPHON_VALUE_GC(obj),\
			SOPHON_VALUE_GC(vm->message_str),\
			SOPHON_VALUE_GC(msg),\
			SOPHON_VALUE_UNDEFINED,\
			0, SOPHON_FL_HAVE_VALUE);\
	sophon_value_set_object(vm, retv, obj);\
	return SOPHON_OK;\
}\
static const Sophon_Decl \
name##_object_props[] = {\
	SOPHON_FUNCTION_PROP([Call], GLOBAL_ATTRS, name##_call_func, 1),\
	{NULL}\
};

SOPHON_FOR_EACH_ERROR(DECL_ERROR_OBJECT_PROPS)

