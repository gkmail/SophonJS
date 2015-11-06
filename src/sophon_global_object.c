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

#include <sophon_types.h>
#include <sophon_mm.h>
#include <sophon_util.h>
#include <sophon_vm.h>
#include <sophon_decl.h>
#include <sophon_object.h>
#include <sophon_array.h>
#include <sophon_value.h>
#include <sophon_util.h>
#include <sophon_string.h>
#include <sophon_module.h>
#include <sophon_parser.h>
#include <sophon_date.h>
#include <sophon_regexp.h>
#include <sophon_conv.h>
#include <sophon_debug.h>

#define GLOBAL_ATTRS SOPHON_PROP_ATTR_WRITABLE|SOPHON_PROP_ATTR_CONFIGURABLE
#define GLOBAL_FUNC_PROP(name, argc)\
	SOPHON_FUNCTION_PROP(name, GLOBAL_ATTRS, name##_func, argc)
#define GLOBAL_FUNC(name)\
	SOPHON_FUNC(name##_func)

typedef struct {
	Sophon_Char *chars;
	Sophon_U32   len;
	Sophon_U32   off;
} StrInputParams;

static Sophon_Int
str_input (Sophon_Ptr data, Sophon_U8 *buf, Sophon_Int size)
{
	StrInputParams *p = (StrInputParams*)data;
	Sophon_U32 cnt;

	if (p->len <= 0)
		return 0;

	cnt = SOPHON_MIN(p->len, size / sizeof(Sophon_Char));

	sophon_memcpy(buf, p->chars + p->off, cnt * sizeof(Sophon_Char));

	p->len -= cnt;
	p->off += cnt;

	return cnt * sizeof(Sophon_Char);
}

#ifdef SOPHON_EVAL_FUNC
static GLOBAL_FUNC(eval)
{
	StrInputParams params;
	Sophon_Value v = SOPHON_ARG(0);
	Sophon_String *str;

	if (!sophon_value_is_string(v)) {
		*retv = v;
		return SOPHON_OK;
	}

	sophon_value_to_string(vm, v, &str);

	params.chars = sophon_string_chars(vm, str);
	params.len   = sophon_string_length(vm, str);
	params.off   = 0;

	return sophon_eval(vm, SOPHON_ENC, str_input, &params, 0, retv);
}
#endif

static GLOBAL_FUNC(parseInt)
{
	Sophon_String *str;
	Sophon_Char *cstr;
	Sophon_Int radix;
	Sophon_Number n;
	Sophon_Result r;

	if ((r = sophon_value_to_string(vm, SOPHON_ARG(0), &str)) != SOPHON_OK)
		return r;

	if (sophon_value_is_undefined(SOPHON_ARG(1))) {
		radix = SOPHON_BASE_INT;
	} else {
		if ((r = sophon_value_to_int(vm, SOPHON_ARG(1), &radix)) != SOPHON_OK)
			return r;
	}

	cstr = sophon_string_chars(vm, str);

	r = sophon_strtod(cstr, NULL, radix, &n);
	if (r == SOPHON_ERR_LEX) {
		n = SOPHON_NAN;
	} else if (r != SOPHON_OK) {
		sophon_throw(vm, vm->RangeError,
				"String cannot be converted to an integer number");
		return SOPHON_ERR_THROW;
	}

	sophon_value_set_number(vm, retv, n);
	return SOPHON_OK;
}

static GLOBAL_FUNC(parseFloat)
{
	Sophon_String *str;
	Sophon_Char *cstr;
	Sophon_Number n;
	Sophon_Result r;

	if ((r = sophon_value_to_string(vm, SOPHON_ARG(0), &str)) != SOPHON_OK)
		return r;

	cstr = sophon_string_chars(vm, str);

	r = sophon_strtod(cstr, NULL, SOPHON_BASE_FLOAT, &n);
	if (r == SOPHON_ERR_LEX) {
		n = SOPHON_NAN;
	} else if (r != SOPHON_OK) {
		sophon_throw(vm, vm->RangeError,
				"String cannot be converted to a float point number");
		return SOPHON_ERR_THROW;
	}

	sophon_value_set_number(vm, retv, n);
	return SOPHON_OK;
}

static GLOBAL_FUNC(isNaN)
{
	Sophon_Number d;
	Sophon_Bool b;
	Sophon_Result r;

	if ((r = sophon_value_to_number(vm, SOPHON_ARG(0), &d)) != SOPHON_OK)
		return r;

	b = sophon_isnan(d);

	sophon_value_set_bool(vm, retv, b);
	return SOPHON_OK;
}

static GLOBAL_FUNC(isFinite)
{
	Sophon_Number d;
	Sophon_Bool b;
	Sophon_Result r;

	if ((r = sophon_value_to_number(vm, SOPHON_ARG(0), &d)) != SOPHON_OK)
		return r;

	b = !sophon_isinf(d);

	sophon_value_set_bool(vm, retv, b);
	return SOPHON_OK;
}

#ifdef SOPHON_ESCAPE_FUNC
static GLOBAL_FUNC(escape)
{
	Sophon_String *str;
	Sophon_Result r;

	if ((r = sophon_value_to_string(vm, SOPHON_ARG(0), &str)) != SOPHON_OK)
		return r;

	if (!(str = sophon_string_escape(vm, str)))
		return SOPHON_ERR_ARG;

	sophon_value_set_string(vm, retv, str);
	return SOPHON_OK;
}
#endif

#ifdef SOPHON_UNESCAPE_FUNC
static GLOBAL_FUNC(unescape)
{
	Sophon_String *str;
	Sophon_Result r;

	if ((r = sophon_value_to_string(vm, SOPHON_ARG(0), &str)) != SOPHON_OK)
		return r;

	if (!(str = sophon_string_unescape(vm, str)))
		return SOPHON_ERR_LEX;

	sophon_value_set_string(vm, retv, str);
	return SOPHON_OK;
}
#endif


#include "sophon_object_object.c"
#include "sophon_boolean_object.c"
#include "sophon_number_object.c"
#include "sophon_string_object.c"
#include "sophon_function_object.c"
#include "sophon_array_object.c"
#include "sophon_arguments_object.c"
#include "sophon_error_object.c"

#ifdef SOPHON_MATH
#include "sophon_math_object.c"
#endif

#ifdef SOPHON_DATE
#include "sophon_date_object.c"
#endif

#ifdef SOPHON_URI
#include "sophon_uri_functions.c"
#endif

#ifdef SOPHON_JSON
#include "sophon_json_object.c"
#endif

#ifdef SOPHON_REGEXP
#include "sophon_regexp_object.c"
#endif

#ifdef SOPHON_CONSOLE
#include "sophon_console_object.c"
#endif

static const Sophon_Decl
global_object_props[] = {
	SOPHON_OBJECT_PROP(Boolean,  GLOBAL_ATTRS, boolean_object_props),
	SOPHON_OBJECT_PROP(Number,   GLOBAL_ATTRS, number_object_props),
	SOPHON_OBJECT_PROP(String,   GLOBAL_ATTRS, string_object_props),
	SOPHON_OBJECT_PROP(Array,    GLOBAL_ATTRS, array_object_props),
#define ERROR_OBJECT_PROP(name)\
	SOPHON_OBJECT_PROP(name,     GLOBAL_ATTRS, name##_object_props),
	SOPHON_FOR_EACH_ERROR(ERROR_OBJECT_PROP)
	SOPHON_DOUBLE_PROP(NaN,      0, SOPHON_NAN),
	SOPHON_DOUBLE_PROP(Infinity, 0, SOPHON_INFINITY),
	SOPHON_UNDEFINED_PROP(undefined, 0),
#ifdef SOPHON_EVAL_FUNC
	GLOBAL_FUNC_PROP(eval, 1),
#endif
	GLOBAL_FUNC_PROP(parseInt, 1),
	GLOBAL_FUNC_PROP(parseFloat, 1),
	GLOBAL_FUNC_PROP(isNaN, 1),
	GLOBAL_FUNC_PROP(isFinite, 1),
#ifdef SOPHON_ESCAPE_FUNC
	GLOBAL_FUNC_PROP(escape, 1),
#endif
#ifdef SOPHON_UNESCAPE_FUNC
	GLOBAL_FUNC_PROP(unescape, 1),
#endif
#ifdef SOPHON_URI
	GLOBAL_FUNC_PROP(decodeURI, 1),
	GLOBAL_FUNC_PROP(decodeURIComponent, 1),
	GLOBAL_FUNC_PROP(encodeURI, 1),
	GLOBAL_FUNC_PROP(encodeURIComponent, 1),
#endif
#ifdef SOPHON_MATH
	SOPHON_OBJECT_PROP(Math, GLOBAL_ATTRS, math_object_props),
#endif
#ifdef SOPHON_DATE
	SOPHON_OBJECT_PROP(Date, GLOBAL_ATTRS, date_object_props),
#endif
#ifdef SOPHON_JSON
	SOPHON_OBJECT_PROP(JSON, GLOBAL_ATTRS, json_object_props),
#endif
#ifdef SOPHON_REGEXP
	SOPHON_OBJECT_PROP(RegExp, GLOBAL_ATTRS, regexp_object_props),
#endif
#ifdef SOPHON_CONSOLE
	SOPHON_OBJECT_PROP(console, GLOBAL_ATTRS, console_object_props),
#endif
	{NULL}
};

static void
load_prototype (Sophon_VM *vm, Sophon_Module *mod, Sophon_String *name,
		Sophon_Value *protov)
{
	Sophon_Value objv;

	sophon_value_get(vm, mod->globv, SOPHON_VALUE_GC(name),
				&objv, 0);
	sophon_value_get(vm, objv, SOPHON_VALUE_GC(vm->prototype_str),
				protov, 0);
}

static void
load_error (Sophon_VM *vm, Sophon_Module *mod, Sophon_String *name,
		Sophon_Value *errv)
{
	Sophon_Value v;

	sophon_value_get(vm, mod->globv, SOPHON_VALUE_GC(name),
			&v, 0);
	if (sophon_value_is_undefined(vm->Error)) {
		sophon_decl_load(vm, mod, SOPHON_VALUE_GET_OBJECT(v),
				Error_prototype_props);
		vm->Error = v;
	}

	sophon_value_define_prop(vm, v,
				SOPHON_VALUE_GC(vm->prototype_str),
				vm->Error,
				SOPHON_VALUE_UNDEFINED,
				GLOBAL_ATTRS,
				SOPHON_FL_DATA_PROP);

	*errv = v;
}

Sophon_Result
sophon_global_init (Sophon_VM *vm)
{
	Sophon_Module *mod;
	Sophon_Object *obj, *glob, *fobj;
	Sophon_Result r;

	/*Disable GC*/
	vm->gc_running = SOPHON_TRUE;

	sophon_value_set_null(vm, &vm->Object_protov);
	sophon_value_set_null(vm, &vm->Function_protov);
	sophon_value_set_undefined(vm, &vm->Error);

	mod  = sophon_module_create(vm);
	obj  = sophon_object_create(vm);

	/*Load object object*/
	r = sophon_decl_load(vm, mod, obj, object_object_props);
	if (r != SOPHON_OK)
		return r;

	sophon_value_get(vm, SOPHON_VALUE_GC(obj),
				SOPHON_VALUE_GC(vm->prototype_str),
				&vm->Object_protov, 0);
	sophon_value_define_prop(vm, vm->Object_protov,
				SOPHON_VALUE_GC(vm->constructor_str),
				SOPHON_VALUE_GC(obj),
				SOPHON_VALUE_UNDEFINED,
				GLOBAL_ATTRS,
				SOPHON_FL_DATA_PROP);

	/*Load function object*/
	fobj  = sophon_object_create(vm);

	r = sophon_decl_load(vm, mod, fobj, function_object_props);
	if (r != SOPHON_OK)
		return r;

	sophon_value_get(vm, SOPHON_VALUE_GC(fobj),
				SOPHON_VALUE_GC(vm->prototype_str),
				&vm->Function_protov, 0);

	obj->protov  = vm->Function_protov;
	fobj->protov = vm->Function_protov;

	/*Load global object*/
	glob = sophon_object_create(vm);
	r = sophon_decl_load(vm, mod, glob, global_object_props);
	if (r != SOPHON_OK)
		return r;

	sophon_value_set_object(vm, &mod->globv, glob);
	sophon_value_define_prop(vm, mod->globv,
				SOPHON_VALUE_GC(vm->Object_str),
				SOPHON_VALUE_GC(obj),
				SOPHON_VALUE_UNDEFINED,
				GLOBAL_ATTRS,
				SOPHON_FL_DATA_PROP);
	sophon_value_define_prop(vm, mod->globv,
				SOPHON_VALUE_GC(vm->Function_str),
				SOPHON_VALUE_GC(fobj),
				SOPHON_VALUE_UNDEFINED,
				GLOBAL_ATTRS,
				SOPHON_FL_DATA_PROP);

	/*Load prototypes*/
	load_prototype(vm, mod, vm->Boolean_str, &vm->Boolean_protov);
	load_prototype(vm, mod, vm->Number_str, &vm->Number_protov);
	load_prototype(vm, mod, vm->String_str, &vm->String_protov);
	load_prototype(vm, mod, vm->Array_str, &vm->Array_protov);
	load_prototype(vm, mod, vm->RegExp_str, &vm->RegExp_protov);
	load_prototype(vm, mod, vm->Date_str, &vm->Date_protov);

	/*Load arguments prototypes*/
	obj  = sophon_object_create(vm);
	r = sophon_decl_load(vm, mod, obj, arguments_prototype_props);
	if (r != SOPHON_OK)
		return r;
	sophon_value_set_object(vm, &vm->Arguments_protov, obj);
	obj->protov = vm->Array_protov;

	/*Load errors*/
#define LOAD_ERROR_OBJECT(name)\
	load_error(vm, mod, vm->name##_str, &vm->name);

	SOPHON_FOR_EACH_ERROR(LOAD_ERROR_OBJECT)

	vm->glob_module = mod;

	/*Enable GC*/
	vm->gc_running = SOPHON_FALSE;

	return SOPHON_OK;
}

