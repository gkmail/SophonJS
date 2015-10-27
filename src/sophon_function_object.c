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

#include <sophon_stack.h>
#include <sophon_frame.h>

#define FUNCTION_FUNC_PROP(name, argc)\
	SOPHON_FUNCTION_PROP(name, GLOBAL_ATTRS, function_##name##_func, argc)
#define FUNCTION_FUNC(name)\
	SOPHON_FUNC(function_##name##_func)

typedef struct {
	Sophon_U8   *chars;
	Sophon_U32   len;
	Sophon_U32   pos;
} BufInputData;

static Sophon_Int
buf_input_func (Sophon_Ptr data, Sophon_U8 *buf, Sophon_Int size)
{
	BufInputData *inp = (BufInputData*)data;
	Sophon_Int len = SOPHON_MIN(size, inp->len);

	sophon_memcpy(buf, inp->chars + inp->pos, len);
	inp->len -= len;
	inp->pos += len;

	return len;
}

#ifdef SOPHON_FUNCTION_CONSTRUCTOR
static FUNCTION_FUNC(constructor)
{
	Sophon_Module *mod;
	Sophon_Int func_id, aid;
	Sophon_String *str;
	Sophon_Function *func;
	Sophon_Closure *clos;
	Sophon_Result r;
	Sophon_Bool strict = sophon_strict(vm);
	BufInputData inp;

	if (argc < 1) {
		sophon_throw(vm, vm->SyntaxError, "No argument");
		return SOPHON_ERR_THROW;
	}

	mod = sophon_module_create(vm);
	func_id = sophon_module_add_func(vm, mod, NULL, 0);
	SOPHON_ASSERT(func_id == 0);
	func = sophon_module_get_func(mod, func_id);

	for (aid = 0; aid < argc - 1; aid++) {
		if ((r = sophon_value_to_string(vm, SOPHON_ARG(aid), &str))
				!= SOPHON_OK)
			return r;

		str = sophon_string_intern(vm, str);
		r = sophon_function_add_var(vm, func, SOPHON_FUNC_ARG, str);
		if ((r == SOPHON_NONE) && strict) {
			sophon_throw(vm, vm->SyntaxError,
					"Argument has already been defined");
			return SOPHON_ERR_THROW;
		}
	}

	r = sophon_value_to_string(vm, SOPHON_ARG(argc - 1), &str);
	if (r != SOPHON_OK)
		return r;

	inp.chars = (Sophon_U8*)sophon_string_chars(vm, str);
	inp.len   = sophon_string_length(vm, str) * sizeof(Sophon_Char);
	inp.pos   = 0;

	r = sophon_parse(vm, mod, SOPHON_ENC, buf_input_func, &inp,
			SOPHON_PARSER_FL_BODY);
	if (r != SOPHON_OK) {
		return SOPHON_ERR_THROW;
	}

	clos = sophon_closure_create(vm, func);
	sophon_value_set_closure(vm, retv, clos);

	return SOPHON_OK;
}
#endif /*SOPHON_FUNCTION_CONSTRUCTOR*/

static FUNCTION_FUNC(length_get)
{
	Sophon_Closure *clos;
	Sophon_Int len;
	Sophon_Result r;

	if ((r = sophon_value_to_closure(vm, thisv, &clos)) != SOPHON_OK)
		return r;

	if (clos->gc_flags & SOPHON_GC_FL_BIND) {
		len = 0;
	} else {
		len = clos->c.func.func->argc;
	}

	sophon_value_set_int(vm, retv, len);
	return SOPHON_OK;
}

static FUNCTION_FUNC(prototype_get)
{
	Sophon_Value protov;
	Sophon_Result r;

	r = sophon_value_get(vm, thisv, SOPHON_VALUE_GC(vm->proto_str),
			&protov, SOPHON_FL_NONE|SOPHON_FL_OWN);

	if (r == SOPHON_NONE) {
		Sophon_Object *obj;

		obj = sophon_object_create(vm);
		sophon_value_set_object(vm, &protov, obj);

		sophon_value_define_prop(vm, thisv,
				SOPHON_VALUE_GC(vm->proto_str),
				protov, SOPHON_VALUE_UNDEFINED,
				0,
				SOPHON_FL_HAVE_VALUE|
				SOPHON_FL_HAVE_ENUMERABLE|
				SOPHON_FL_FORCE);

		sophon_value_define_prop(vm, protov,
				SOPHON_VALUE_GC(vm->constructor_str),
				thisv, SOPHON_VALUE_UNDEFINED,
				0,
				SOPHON_FL_HAVE_VALUE|
				SOPHON_FL_HAVE_ENUMERABLE|
				SOPHON_FL_FORCE);
	}

	*retv = protov;
	return SOPHON_OK;
}

static FUNCTION_FUNC(prototype_set)
{
	Sophon_Value protov = SOPHON_ARG(0);

	sophon_value_define_prop(vm, thisv,
			SOPHON_VALUE_GC(vm->proto_str),
			protov, SOPHON_VALUE_UNDEFINED,
			0,
			SOPHON_FL_HAVE_VALUE|
			SOPHON_FL_HAVE_ENUMERABLE|
			SOPHON_FL_FORCE);

	sophon_value_define_prop(vm, protov,
			SOPHON_VALUE_GC(vm->constructor_str),
			thisv, SOPHON_VALUE_UNDEFINED,
			0,
			SOPHON_FL_HAVE_VALUE|
			SOPHON_FL_HAVE_ENUMERABLE|
			SOPHON_FL_FORCE);

	return SOPHON_OK;
}

static FUNCTION_FUNC(caller_get)
{
	Sophon_Stack *stk = vm->stack;

	if (sophon_strict(vm)) {
		sophon_throw(vm, vm->TypeError,
				"\"caller\" cannot be used in strict mode");
		return SOPHON_ERR_THROW;
	}

	while (stk) {
		Sophon_DeclFrame *df = (Sophon_DeclFrame*)stk->var_env;
		if (df->calleev == thisv) {
			*retv = df->calleev;
			return SOPHON_OK;
		}
		stk = stk->bottom;
	}

	sophon_throw(vm, vm->TypeError,
			"\"caller\" cannot be used");
	return SOPHON_ERR_THROW;
}

static FUNCTION_FUNC(arguments_get)
{
	Sophon_Stack *stk = vm->stack;

	if (sophon_strict(vm)) {
		sophon_throw(vm, vm->TypeError,
				"\"arguments\" cannot be used in strict mode");
		return SOPHON_ERR_THROW;
	}

	while (stk) {
		Sophon_DeclFrame *df = (Sophon_DeclFrame*)stk->var_env;
		if (df->calleev == thisv) {
			sophon_decl_frame_create_args(vm, df);
			*retv = df->argsv;
			return SOPHON_OK;
		}
		stk = stk->bottom;
	}

	sophon_throw(vm, vm->TypeError,
			"\"arguments\" cannot be used");
	return SOPHON_ERR_THROW;
}

static FUNCTION_FUNC(toString)
{
	Sophon_String *str, *astr;
	Sophon_Closure *clos;
	Sophon_Result r;

	if ((r = sophon_value_to_closure(vm, thisv, &clos)) != SOPHON_OK)
		return r;

	while (clos->gc_flags & SOPHON_GC_FL_BIND) {
		r = sophon_value_to_closure(vm, clos->c.bind.funcv, &clos);
		if (r != SOPHON_OK)
			return r;
	}

	str = vm->function_str;
	if (clos->c.func.func->name) {
		str  = sophon_string_concat(vm, str, vm->space_str);
		str  = sophon_string_concat(vm, str, clos->c.func.func->name);
	}

	astr = sophon_string_from_utf8_cstr(vm, "(){}");
	str  = sophon_string_concat(vm, str, astr);

	sophon_value_set_string(vm, retv, str);

	return SOPHON_OK;
}

static FUNCTION_FUNC(apply)
{
	Sophon_Value athisv = SOPHON_ARG(0);
	Sophon_Value *aargv = NULL;
	Sophon_Int aargc = 0;
	Sophon_Value argsv = SOPHON_ARG(1);
	Sophon_Array *arr;
	Sophon_Result r;

	if (!sophon_value_is_null(argsv) && !sophon_value_is_undefined(argsv)) {
		if ((r = sophon_value_to_array(vm, argsv, &arr)) != SOPHON_OK)
			return r;

		aargv = arr->v;
		aargc = arr->len;
	}

	return sophon_value_call(vm, thisv, athisv, aargv, aargc, retv,
			SOPHON_FL_THROW);
}

static FUNCTION_FUNC(call)
{
	Sophon_Value athisv = SOPHON_ARG(0);
	Sophon_Value *aargv = &argv[1];
	Sophon_Int aargc = argc - 1;

	if (aargc < 0) {
		sophon_throw(vm, vm->RangeError, "No argument");
		return SOPHON_ERR_THROW;
	}

	return sophon_value_call(vm, thisv, athisv, aargv, aargc, retv,
			SOPHON_FL_THROW);
}

static FUNCTION_FUNC(bind)
{
	Sophon_Array *arr;
	Sophon_Closure *clos;
	Sophon_Value bthisv = SOPHON_ARG(0);
	Sophon_Int bargc = argc - 1;
	Sophon_Result r;

	if (bargc < 0) {
		sophon_throw(vm, vm->RangeError, "No argument");
		return SOPHON_ERR_THROW;
	}

	arr = sophon_array_create(vm);
	if (bargc) {
		if ((r = sophon_array_set_length(vm, arr, bargc)) != SOPHON_OK) {
			return r;
		}
		sophon_memcpy(arr->v, &argv[1], bargc * sizeof(Sophon_Value));
	}

	clos = sophon_closure_bind(vm, thisv, bthisv, arr);
	sophon_value_set_closure(vm, retv, clos);

	return SOPHON_OK;
}

static const Sophon_Decl
function_prototype_props[] = {
	SOPHON_STRING_PROP([Class], 0, "Function"),
	SOPHON_ACCESSOR_PROP(length, 0, function_length_get_func, NULL),
	SOPHON_ACCESSOR_PROP(prototype, 0, function_prototype_get_func,
			function_prototype_set_func),
	SOPHON_ACCESSOR_PROP(caller, 0, function_caller_get_func, NULL),
	SOPHON_ACCESSOR_PROP(arguments, 0, function_arguments_get_func, NULL),
	FUNCTION_FUNC_PROP(toString, 0),
	FUNCTION_FUNC_PROP(apply, 2),
	FUNCTION_FUNC_PROP(call, 1),
	FUNCTION_FUNC_PROP(bind, 1),
	{NULL}
};

static const Sophon_Decl
function_object_props[] = {
	SOPHON_OBJECT_PROP(prototype, 0, function_prototype_props),
#ifdef SOPHON_FUNCTION_CONSTRUCTOR
	SOPHON_FUNCTION_PROP([Call], GLOBAL_ATTRS, function_constructor_func, 1),
#endif
	{NULL}
};

