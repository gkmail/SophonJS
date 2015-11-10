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
#include <sophon_value.h>
#include <sophon_closure.h>
#include <sophon_arguments.h>
#include <sophon_object.h>
#include <sophon_string.h>
#include <sophon_gc.h>
#include <sophon_stack.h>
#include <sophon_function.h>
#include <sophon_ins.h>
#include <sophon_string.h>
#include <sophon_array.h>
#include <sophon_regexp.h>
#include <sophon_vm.h>
#include <sophon_debug.h>

Sophon_Result
sophon_value_to_prim (Sophon_VM *vm, Sophon_Value v, Sophon_Value *outv,
			Sophon_Int type)
{
	Sophon_Object *obj;
	Sophon_Result r;

	SOPHON_ASSERT(vm && outv);

	if (!sophon_value_is_object(v)) {
		*outv = v;
		return SOPHON_OK;
	}

	if (SOPHON_VALUE_IS_OBJECT(v)) {
		obj = SOPHON_VALUE_GET_OBJECT(v);
		if (SOPHON_VALUE_IS_BOOL(obj->primv) ||
					SOPHON_VALUE_IS_NUMBER(obj->primv) ||
					SOPHON_VALUE_IS_INT(obj->primv) ||
					SOPHON_VALUE_IS_STRING(obj->primv)) {
			*outv = obj->primv;
			return SOPHON_OK;
		}

		if (type == SOPHON_PRIM_UNDEF) {
			type = (obj->protov == vm->Date_protov) ? SOPHON_PRIM_STRING :
					SOPHON_PRIM_NUMBER;
		}
	} else if (type == SOPHON_PRIM_UNDEF) {
		type = SOPHON_PRIM_NUMBER;
	}

	if (type == SOPHON_PRIM_STRING) {
		r = sophon_value_call_prop(vm, v, SOPHON_VALUE_GC(vm->toString_str),
					NULL, 0, outv, SOPHON_FL_NONE);

		if (r == SOPHON_OK) {
			if (!sophon_value_is_object(*outv))
				return r;
		}

		r = sophon_value_call_prop(vm, v, SOPHON_VALUE_GC(vm->valueOf_str),
					NULL, 0, outv, SOPHON_FL_NONE);
		if (r == SOPHON_OK) {
			if (!sophon_value_is_object(*outv))
				return r;
		}
	} else {
		r = sophon_value_call_prop(vm, v, SOPHON_VALUE_GC(vm->valueOf_str),
					NULL, 0, outv, SOPHON_FL_NONE);

		if (r == SOPHON_OK) {
			if (!sophon_value_is_object(*outv))
				return r;
		}

		r = sophon_value_call_prop(vm, v, SOPHON_VALUE_GC(vm->toString_str),
					NULL, 0, outv, SOPHON_FL_NONE);

		if (r == SOPHON_OK) {
			if (!sophon_value_is_object(*outv))
				return r;
		}
	}

	sophon_throw(vm, vm->TypeError, "Value is not primitive");
	return SOPHON_ERR_THROW;
}

Sophon_Object*
sophon_value_get_class (Sophon_VM *vm, Sophon_Value v)
{
	Sophon_Object *obj = NULL;

	if (SOPHON_VALUE_IS_BOOL(v)) {
		obj = SOPHON_VALUE_GET_OBJECT(vm->Boolean_protov);
	} else if (SOPHON_VALUE_IS_INT(v)) {
		obj = SOPHON_VALUE_GET_OBJECT(vm->Number_protov);
	} else if (SOPHON_VALUE_IS_NUMBER(v)) {
		obj = SOPHON_VALUE_GET_OBJECT(vm->Number_protov);
	} else if (SOPHON_VALUE_IS_GC(v)) {
		Sophon_GCObject *gco = SOPHON_VALUE_GET_GC(v);

		switch (gco->gc_type) {
			case SOPHON_GC_Object:
				obj = (Sophon_Object*)gco;
				break;
			case SOPHON_GC_String:
				obj = SOPHON_VALUE_GET_OBJECT(vm->String_protov);
				break;
			case SOPHON_GC_Closure:
				obj = SOPHON_VALUE_GET_OBJECT(vm->Function_protov);
				break;
			case SOPHON_GC_Array:
				obj = SOPHON_VALUE_GET_OBJECT(vm->Array_protov);
				break;
			case SOPHON_GC_RegExp:
				obj = SOPHON_VALUE_GET_OBJECT(vm->RegExp_protov);
				break;
			default:
				break;
		}
	}

	return obj;
}

Sophon_Result
sophon_value_call (Sophon_VM *vm, Sophon_Value callv,
			Sophon_Value thisv, Sophon_Value *argv, Sophon_Int argc,
			Sophon_Value *retv, Sophon_U32 flags)
{
	Sophon_Function *func;
	Sophon_Closure *clos;
	Sophon_Result r;

	SOPHON_ASSERT(vm && retv);

	r = sophon_value_to_closure(vm, callv, &clos);
	if (r != SOPHON_OK) {
		sophon_throw(vm, vm->TypeError, "Value is not a closure");
		return r;
	}

	if (clos->gc_flags & SOPHON_GC_FL_BIND) {
		if (clos->c.bind.args) {
			argv = clos->c.bind.args->v;
			argc = clos->c.bind.args->len;
		} else {
			argv = NULL;
			argc = 0;
		}

		return sophon_value_call(vm, clos->c.bind.funcv, clos->c.bind.thisv,
					argv, argc, retv, flags);
	} else {
		func = clos->c.func.func;
		sophon_value_set_undefined(vm, retv);

		if (!(func->flags & SOPHON_FUNC_FL_STRICT)) {
			if (SOPHON_VALUE_IS_BOOL(thisv) ||
						SOPHON_VALUE_IS_INT(thisv) ||
						SOPHON_VALUE_IS_NUMBER(thisv) ||
						SOPHON_VALUE_IS_STRING(thisv)) {
				Sophon_Object *obj;

				sophon_value_to_object(vm, thisv, &obj);
				sophon_value_set_object(vm, &thisv, obj);
			}
		}

		if (func->flags & SOPHON_FUNC_FL_NATIVE) {
				r = func->f.native(vm, thisv, argv, argc, retv);
		} else {
			if (func->flags & SOPHON_FUNC_FL_GLOBAL)
				r = sophon_stack_push_global(vm, thisv, callv, argv, argc);
			else
				r = sophon_stack_push_decl(vm, thisv, callv, argv, argc,
							flags);

			if (func->flags & SOPHON_FUNC_FL_STRICT) {
				Sophon_DeclFrame *df = (Sophon_DeclFrame*)vm->stack->var_env;
				Sophon_Object *obj;

				obj = sophon_arguments_create(vm, df);
				sophon_value_set_object(vm, &df->argsv, obj);
			}

			SOPHON_ASSERT(r == SOPHON_OK);

			if (flags & SOPHON_FL_LAZY)
				return SOPHON_OK;

			r = sophon_ins_run(vm);

			if ((r == SOPHON_OK) ||
						((r == SOPHON_NONE) &&
						 (func->flags & SOPHON_FUNC_FL_EVAL)))
				*retv = vm->stack->retv;

			sophon_stack_pop(vm);

			if (r == SOPHON_NONE)
				r = SOPHON_OK;
		}
	}

	return r;
}

Sophon_Result
sophon_value_call_prop (Sophon_VM *vm, Sophon_Value thisv,
			Sophon_Value namev, Sophon_Value *argv, Sophon_Int argc,
			Sophon_Value *retv, Sophon_U32 flags)
{
	Sophon_Value callv;
	Sophon_Result r;

	r = sophon_value_get(vm, thisv, namev, &callv, flags);
	if (r != SOPHON_OK)
		return r;

	return sophon_value_call(vm, callv, thisv, argv, argc, retv, flags);
}

Sophon_Result
sophon_value_to_object (Sophon_VM *vm, Sophon_Value v, Sophon_Object **pobj)
{
	Sophon_Object *proto;
	Sophon_Object *obj;
	Sophon_Object *new_obj;

	SOPHON_ASSERT(vm && pobj);

	if (SOPHON_VALUE_IS_GC(v)) {
		Sophon_GCObject *gco = SOPHON_VALUE_GET_GC(v);

		switch (gco->gc_type) {
			case SOPHON_GC_Object:
				*pobj = (Sophon_Object*)gco;
				return SOPHON_OK;
			case SOPHON_GC_String:
				obj = sophon_object_create(vm);
				obj->protov = vm->String_protov;
				obj->primv = v;
				*pobj = obj;
				return SOPHON_OK;
#define PRIM_TO_OBJ(o)\
			case SOPHON_GC_##o:\
				obj = (Sophon_Object*)gco;\
				proto = sophon_value_get_class(vm, v);\
				new_obj = (Sophon_Object*)sophon_mm_alloc_ensure(vm,\
							sizeof(Sophon_##o));\
				sophon_memcpy(new_obj, obj, sizeof(Sophon_##o));\
				new_obj->gc_flags &= ~(SOPHON_GC_FL_##o|SOPHON_GC_FL_MANAGED);\
				sophon_gc_add(vm, (Sophon_GCObject*)new_obj);\
				sophon_object_init(vm, obj);\
				obj->gc_flags |= SOPHON_GC_FL_##o;\
				sophon_value_set_gc(vm, &obj->primv, new_obj);\
				sophon_value_set_object(vm, &obj->protov, proto);\
				*pobj = obj;\
				return SOPHON_OK;

			SOPHON_FOR_EACH_PRIM_OBJ(PRIM_TO_OBJ)
		}
	} else if (SOPHON_VALUE_IS_INT(v) || SOPHON_VALUE_IS_NUMBER(v) ||
				SOPHON_VALUE_IS_BOOL(v)) {
		obj = sophon_object_create(vm);
		obj->protov = SOPHON_VALUE_IS_BOOL(v) ? vm->Boolean_protov :
					vm->Number_protov;

		obj->primv = v;
		*pobj = obj;
		return SOPHON_OK;
	}

	sophon_throw(vm, vm->TypeError, "Value is not an object");
	SOPHON_ERROR(("the value is not an object"));
	return SOPHON_ERR_TYPE;
}

Sophon_Result
sophon_value_to_gc (Sophon_VM *vm, Sophon_Value v, Sophon_U8 gc_type,
			Sophon_GCObject **pgco)
{
	Sophon_GCObject *gco;
	const char *msg;

	SOPHON_ASSERT(vm && pgco);

	if (SOPHON_VALUE_IS_GC(v)) {
		gco = SOPHON_VALUE_GET_GC(v);
		if (gco->gc_type == gc_type) {
			*pgco = gco;
			return SOPHON_OK;
		}

		if (gco->gc_type == SOPHON_GC_Object) {
			Sophon_Object *obj = (Sophon_Object*)gco;

			if (SOPHON_VALUE_IS_GC(obj->primv)) {
				gco = SOPHON_VALUE_GET_GC(obj->primv);
				if (gco->gc_type == gc_type) {
					*pgco = gco;
					return SOPHON_OK;
				}
			}
		}
	}

	if (gc_type == SOPHON_GC_Closure)
		msg = "Value is not an clousre";
	else if (gc_type == SOPHON_GC_Array)
		msg = "Value is not an array";
	else
		msg = "Value type mismatch";

	sophon_throw(vm, vm->TypeError, msg);
	SOPHON_ERROR(("the value is not in type %d", gc_type));
	return SOPHON_ERR_TYPE;
}

Sophon_Bool
sophon_value_to_bool (Sophon_VM *vm, Sophon_Value v)
{
	if (SOPHON_VALUE_IS_UNDEFINED(v)) {
		return SOPHON_FALSE;
	} else if (SOPHON_VALUE_IS_NULL(v)) {
		return SOPHON_FALSE;
	} else if (SOPHON_VALUE_IS_BOOL(v)) {
		return SOPHON_VALUE_GET_BOOL(v);
	} else if (SOPHON_VALUE_IS_INT(v)) {
		return SOPHON_VALUE_GET_INT(v) ? SOPHON_TRUE : SOPHON_FALSE;
	} else if (SOPHON_VALUE_IS_NUMBER(v)) {
		Sophon_Number d = SOPHON_VALUE_GET_NUMBER(v);

		if ((d == 0) || sophon_isnan(d))
			return SOPHON_FALSE;
	} else if (SOPHON_VALUE_IS_GC(v)) {
		Sophon_GCObject *gco = (Sophon_GCObject*)SOPHON_VALUE_GET_GC(v);

		if (gco->gc_type == SOPHON_GC_String) {
			Sophon_String *str = (Sophon_String*)gco;

			if (!sophon_string_length(vm, str))
				return SOPHON_FALSE;
		}
	}

	return SOPHON_TRUE;
}

Sophon_Result
sophon_value_to_number (Sophon_VM *vm, Sophon_Value v, Sophon_Number *pd)
{
	Sophon_Number d = SOPHON_NAN;

	SOPHON_ASSERT(pd);

	if (SOPHON_VALUE_IS_NULL(v)) {
		d = 0.;
	} else if (SOPHON_VALUE_IS_BOOL(v)) {
		d = SOPHON_VALUE_GET_BOOL(v) ? 1. : 0.;
	} else if (SOPHON_VALUE_IS_INT(v)) {
		d = (Sophon_Double)SOPHON_VALUE_GET_INT(v);
	} else if (SOPHON_VALUE_IS_NUMBER(v)) {
		d = SOPHON_VALUE_GET_NUMBER(v);
	} else if (SOPHON_VALUE_IS_GC(v)) {
		Sophon_GCObject *gco = (Sophon_GCObject*)SOPHON_VALUE_GET_GC(v);
		Sophon_Result r;

		if (gco->gc_type == SOPHON_GC_String) {
			Sophon_String *str = (Sophon_String*)gco;
			Sophon_Char *cstr, *end;
			Sophon_U32 len;
		
			if (sophon_string_length(vm, str) == 0) {
				d = 0;
			} else if (!sophon_string_casecmp(vm, str, vm->NaN_str)) {
				d = SOPHON_NAN;
			} else if (!sophon_string_casecmp(vm, str, vm->Infinity_str)) {
				d = SOPHON_INFINITY;
			} else {
				cstr = sophon_string_chars(vm, str);
				len  = sophon_string_length(vm, str);
				r = sophon_strtod(cstr, &end, SOPHON_BASE_INT_FLOAT, &d);
				if ((r == SOPHON_OK) && (end != cstr + len)) {
					while (end < cstr + len) {
						if (!sophon_isspace(*end)) {
							r = SOPHON_ERR_LEX;
							break;
						}
					}
				}

				if (r != SOPHON_OK) {
					d = SOPHON_NAN;
				}
			}
		} else if (gco->gc_type == SOPHON_GC_Object) {
			Sophon_Value primv;

			if ((r = sophon_value_to_prim(vm, v, &primv, SOPHON_PRIM_NUMBER))
						!= SOPHON_OK)
				return r;

			return sophon_value_to_number(vm, primv, pd);
		}
	}

	*pd = d;
	return SOPHON_OK;
}

Sophon_Result
sophon_value_to_int (Sophon_VM *vm, Sophon_Value v, Sophon_Int *pi)
{
	Sophon_Number d;
	Sophon_Int i;
	Sophon_Result r;

	SOPHON_ASSERT(pi);

	if (SOPHON_VALUE_IS_INT(v)) {
		i = SOPHON_VALUE_GET_INT(v);
		*pi = i;
		return SOPHON_OK;
	}

	if ((r = sophon_value_to_number(vm, v, &d)) != SOPHON_OK)
		return r;

	if (sophon_isnan(d)) {
		i = 0;
	} else {
		i = d;
	}

	*pi = i;
	return SOPHON_OK;
}

#define NUM_2_32 4294967296.0
#define NUM_2_31 2147483648.0
#define NUM_2_16 65536.0

Sophon_Result
sophon_value_to_s32 (Sophon_VM *vm, Sophon_Value v, Sophon_S32 *pi)
{
	Sophon_Number d;
	Sophon_S32 i;
	Sophon_Result r;

	SOPHON_ASSERT(pi);

	if (SOPHON_VALUE_IS_INT(v)) {
		i = SOPHON_VALUE_GET_INT(v);
		*pi = i;
		return SOPHON_OK;
	}

	if ((r = sophon_value_to_number(vm, v, &d)) != SOPHON_OK)
		return r;

	if ((d == 0) || sophon_isinf(d) || sophon_isnan(d)) {
		i = 0;
	} else {
		d = sophon_fmod(d, NUM_2_32);
		d = (d >= 0) ? sophon_floor(d) : sophon_ceil(d) + NUM_2_32;
		i = (d >= NUM_2_31) ? d - NUM_2_32 : d;
	}

	*pi = i;

	return SOPHON_OK;
}

Sophon_Result
sophon_value_to_u32 (Sophon_VM *vm, Sophon_Value v, Sophon_U32 *pi)
{
	Sophon_Number d;
	Sophon_U32 i;
	Sophon_Result r;

	SOPHON_ASSERT(pi);

	if (SOPHON_VALUE_IS_INT(v)) {
		i = SOPHON_VALUE_GET_INT(v);
		*pi = i;
		return SOPHON_OK;
	}

	if ((r = sophon_value_to_number(vm, v, &d)) != SOPHON_OK)
		return r;

	if ((d == 0) || sophon_isinf(d) || sophon_isnan(d)) {
		i = 0;
	} else {
		d = sophon_fmod(d, NUM_2_32);
		i = (d >= 0) ? sophon_floor(d) : sophon_ceil(d) + NUM_2_32;
	}

	*pi = i;

	return SOPHON_OK;
}

Sophon_Result
sophon_value_to_u16 (Sophon_VM *vm, Sophon_Value v, Sophon_U16 *pi)
{
	Sophon_Number d;
	Sophon_U16 i;
	Sophon_Result r;

	SOPHON_ASSERT(pi);

	if (SOPHON_VALUE_IS_INT(v)) {
		i = SOPHON_VALUE_GET_INT(v);
		*pi = i;
		return SOPHON_OK;
	}

	if ((r = sophon_value_to_number(vm, v, &d)) != SOPHON_OK)
		return r;

	if ((d == 0) || sophon_isinf(d)) {
		i = 0;
	} else {
		d = sophon_fmod(sophon_trunc(d), NUM_2_16);
		i = (d >= 0) ? sophon_floor(d) : sophon_ceil(d) + NUM_2_16;
	}

	*pi = i;

	return SOPHON_OK;
}

static Sophon_String*
number_to_string (Sophon_VM *vm, Sophon_Number d)
{
	return sophon_dtostr(vm, d, SOPHON_D2STR_RADIX, 10);
}

Sophon_Result
sophon_value_to_string_ex (Sophon_VM *vm, Sophon_Value v, Sophon_Bool quot,
			Sophon_String **pstr)
{
	Sophon_String *str;

	SOPHON_ASSERT(pstr);

	if (SOPHON_VALUE_IS_NULL(v)) {
		str = vm->null_str;
	} else if (SOPHON_VALUE_IS_BOOL(v)) {
		str = SOPHON_VALUE_GET_BOOL(v) ? vm->true_str : vm->false_str;
	} else if (SOPHON_VALUE_IS_INT(v)) {
		Sophon_Int i = SOPHON_VALUE_GET_INT(v);

		str = number_to_string(vm, (Sophon_Number)i);
	} else if (SOPHON_VALUE_IS_NUMBER(v)) {
		Sophon_Number d = SOPHON_VALUE_GET_NUMBER(v);

		str = number_to_string(vm, d);
	} else if (SOPHON_VALUE_IS_GC(v)) {
		Sophon_GCObject *gco = (Sophon_GCObject*)SOPHON_VALUE_GET_GC(v);

		if (gco->gc_type == SOPHON_GC_String) {
			if (quot) {
				str = vm->quot_str;
				str = sophon_string_concat(vm, str, (Sophon_String*)gco);
				str = sophon_string_concat(vm, str, vm->quot_str);
			} else {
				str = (Sophon_String*)gco;
			}
		} else {
			Sophon_Value primv;
			Sophon_Result r;

			if ((r = sophon_value_to_prim(vm, v, &primv, SOPHON_PRIM_STRING))
						!= SOPHON_OK)
				return r;

			return sophon_value_to_string_ex(vm, primv, quot, pstr);
		}
	} else {
		str = vm->undefined_str;
	}

	*pstr = str;
	return SOPHON_OK;
}

Sophon_Result
sophon_value_to_string (Sophon_VM *vm, Sophon_Value v, Sophon_String **pstr)
{
	return sophon_value_to_string_ex(vm, v, SOPHON_FALSE, pstr);
}

Sophon_Result
sophon_value_compare (Sophon_VM *vm, Sophon_Value lv,
			Sophon_Value rv, Sophon_Int *ri)
{
	Sophon_Result r;
	Sophon_Int i;
	Sophon_Value p1, p2;

	SOPHON_ASSERT(vm && ri);

	if ((r = sophon_value_to_prim(vm, lv, &p1, SOPHON_PRIM_NUMBER))
				!= SOPHON_OK)
		return r;
	if ((r = sophon_value_to_prim(vm, rv, &p2, SOPHON_PRIM_NUMBER))
				!= SOPHON_OK)
		return r;

	if (sophon_value_is_string(p1) &&
				sophon_value_is_string(p2)) {
		Sophon_String *s1, *s2;

		sophon_value_to_string(vm, p1, &s1);
		sophon_value_to_string(vm, p2, &s2);

		i = sophon_string_cmp(vm, s1, s2);
		if (i < 0)
			i = -1;
		else if (i > 0)
			i = 1;
		else
			i = 0;

	} else {
		Sophon_Number d1, d2;

		sophon_value_to_number(vm, p1, &d1);
		sophon_value_to_number(vm, p2, &d2);

		if (sophon_isnan(d1) || sophon_isnan(d2)) {
			i = -2;
		} else {
			if (d1 < d2)
				i = -1;
			else if (d1 > d2)
				i = 1;
			else
				i = 0;
		}
	}

	*ri = i;

	return SOPHON_OK;
}

Sophon_Result
sophon_value_equal_compare (Sophon_VM *vm, Sophon_Value lv, Sophon_Value rv,
			Sophon_Bool *pb)
{
	Sophon_Result r;
	Sophon_Number n1, n2;
	Sophon_Bool lnul, rnul;

	SOPHON_ASSERT(vm && pb);

	if (lv == rv) {
		if (SOPHON_VALUE_IS_NUMBER(lv)) {
			Sophon_Number n = SOPHON_VALUE_GET_NUMBER(lv);

			if (sophon_isnan(n)) {
				*pb = SOPHON_FALSE;
				return SOPHON_OK;
			}
		}

		*pb = SOPHON_TRUE;
		return SOPHON_OK;
	}

	lnul = (SOPHON_VALUE_IS_UNDEFINED(lv) || SOPHON_VALUE_IS_NULL(lv));
	rnul = (SOPHON_VALUE_IS_UNDEFINED(rv) || SOPHON_VALUE_IS_NULL(rv));

	if (lnul || rnul) {
		if (lnul == rnul) {
			*pb = SOPHON_TRUE;
		} else {
			*pb = SOPHON_FALSE;
		}

		return SOPHON_OK;
	}

	if (sophon_value_is_string(lv) && sophon_value_is_string(rv)) {
		Sophon_String *s1, *s2;

		sophon_value_to_string(vm, lv, &s1);
		sophon_value_to_string(vm, rv, &s2);

		*pb = (sophon_string_cmp(vm, s1, s2) == 0);
		return SOPHON_OK;
	}

	if (sophon_value_is_object(lv) && sophon_value_is_object(rv)) {
		*pb = SOPHON_FALSE;
		return SOPHON_OK;
	}

	if ((r = sophon_value_to_number(vm, lv, &n1)) != SOPHON_OK)
		return r;

	if ((r = sophon_value_to_number(vm, rv, &n2)) != SOPHON_OK)
		return r;

	if (sophon_isnan(n1) || sophon_isnan(n2)) {
		*pb = SOPHON_FALSE;
	} else {
		*pb = (n1 == n2);
	}

	return SOPHON_OK;
}

Sophon_Bool
sophon_value_same (Sophon_VM *vm, Sophon_Value lv, Sophon_Value rv)
{
	if (lv == rv) {
		if (SOPHON_VALUE_IS_NUMBER(lv)) {
			Sophon_Double d = SOPHON_VALUE_GET_NUMBER(lv);

			if (sophon_isnan(d))
				return SOPHON_FALSE;
		}

		return SOPHON_TRUE;
	}

	if (SOPHON_VALUE_IS_STRING(lv) && SOPHON_VALUE_IS_STRING(rv)) {
		Sophon_String *s1, *s2;

		s1 = SOPHON_VALUE_GET_STRING(lv);
		s2 = SOPHON_VALUE_GET_STRING(rv);

		return sophon_string_cmp(vm, s1, s2) ? SOPHON_FALSE : SOPHON_TRUE;
	}

	if (SOPHON_VALUE_IS_NUMBER(lv) && SOPHON_VALUE_IS_NUMBER(rv)) {
		Sophon_Number d1, d2;

		d1 = SOPHON_VALUE_GET_NUMBER(lv);
		d2 = SOPHON_VALUE_GET_NUMBER(rv);

		if (sophon_isnan(d1) || sophon_isnan(d2))
			return SOPHON_FALSE;

		return (d1 == d2) ? SOPHON_TRUE : SOPHON_FALSE;
	}

	return SOPHON_FALSE;
}

Sophon_Result
sophon_value_in (Sophon_VM *vm, Sophon_Value lv, Sophon_Value rv,
			Sophon_Bool *pb)
{
	Sophon_Object *obj;
	Sophon_Result r;

	SOPHON_ASSERT(vm && pb);

	while (1) {
		if ((r = sophon_value_prop_desc(vm, rv, lv, NULL)) < 0)
			return r;

		if (r == SOPHON_OK) {
			*pb = SOPHON_TRUE;
			return SOPHON_OK;
		}

		if (!sophon_value_is_object(rv))
			break;

		if ((r = sophon_value_to_object(vm, rv, &obj)) != SOPHON_OK)
			return r;

		if (sophon_value_is_null(obj->protov))
			break;

		rv = obj->protov;
	}

	*pb = SOPHON_FALSE;
	return SOPHON_OK;
}

Sophon_Result
sophon_value_instanceof (Sophon_VM *vm, Sophon_Value lv, Sophon_Value rv,
			Sophon_Bool *pb)
{
	Sophon_Object *lobj;
	Sophon_Value protov;
	Sophon_Result r;

	SOPHON_ASSERT(vm && pb);

	r = sophon_value_get(vm, rv, SOPHON_VALUE_GC(vm->prototype_str), &protov,
				SOPHON_FL_NONE);

	if ((r == SOPHON_NONE) || !sophon_value_is_object(protov)) {
		sophon_throw(vm, vm->TypeError, "Value is not an object");
		return SOPHON_ERR_THROW;
	}

	while (sophon_value_is_object(lv)) {
		sophon_value_to_object(vm, lv, &lobj);

		if (protov == lobj->protov) {
			*pb = SOPHON_TRUE;
			return SOPHON_OK;
		} else if (!sophon_value_is_null(lobj->protov)) {
			lv = lobj->protov;
		} else {
			break;
		}
	}

	*pb = SOPHON_FALSE;
	return SOPHON_OK;
}

Sophon_Result
sophon_value_new (Sophon_VM *vm, Sophon_Value cv, Sophon_Value *argv,
			Sophon_Int argc, Sophon_Value *objv)
{
	Sophon_Object *obj;
	Sophon_Value protov;
	Sophon_Value retv;
	Sophon_Result r;

	SOPHON_ASSERT(objv);

	r = sophon_value_get(vm, cv, SOPHON_VALUE_GC(vm->prototype_str), &protov,
				SOPHON_FL_NONE);
	if (r < 0) {
		return r;
	} else if (r == SOPHON_NONE) {
		sophon_value_set_undefined(vm, &protov);
	}

	obj = sophon_object_create(vm);
	obj->protov = protov;

	sophon_value_set_object(vm, objv, obj);

	r = sophon_value_call(vm, cv, *objv, argv, argc, &retv, 0);
	if (r != SOPHON_OK)
		return r;

	if (!sophon_value_is_undefined(retv)) {
		*objv = retv;
	}

	return SOPHON_OK;
}

Sophon_String*
sophon_value_typeof (Sophon_VM *vm, Sophon_Value v)
{
	if (SOPHON_VALUE_IS_UNDEFINED(v))
		return vm->undefined_str;
	if (SOPHON_VALUE_IS_NULL(v))
		return vm->object_str;
	if (SOPHON_VALUE_IS_BOOL(v))
		return vm->boolean_str;
	if (SOPHON_VALUE_IS_INT(v) || SOPHON_VALUE_IS_NUMBER(v))
		return vm->number_str;
	if (SOPHON_VALUE_IS_STRING(v))
		return vm->string_str;
	if (SOPHON_VALUE_IS_GC(v)) {
		Sophon_GCObject *gco = SOPHON_VALUE_GET_GC(v);

		if (gco->gc_type == SOPHON_GC_Closure)
			return vm->function_str;

		if (gco->gc_type == SOPHON_GC_Object) {
			Sophon_Object *obj;

			obj = SOPHON_VALUE_GET_OBJECT(v);
			if (SOPHON_VALUE_IS_CLOSURE(obj->primv))
				return vm->function_str;
		}
	}

	return vm->object_str;
}

Sophon_Result
sophon_value_for_in (Sophon_VM *vm, Sophon_U16 end_ip)
{
	Sophon_PropIter iter;
	Sophon_Stack *stk;
	Sophon_Value v;
	Sophon_Result r = SOPHON_OK;
	Sophon_Object *obj;
	Sophon_Int i;

	SOPHON_ASSERT(vm);

	stk = vm->stack;

	iter.prop    = NULL;
	iter.sp      = stk->sp;
	iter.brk_ip  = end_ip;
	iter.cont_ip = stk->ip;
	iter.bottom  = vm->pi_stack;
	vm->pi_stack = &iter;

	v = stk->v[stk->sp - 2];
	if (sophon_value_is_array(v)) {
		Sophon_Array *arr;
		Sophon_Value item;
		Sophon_U32 id;

		if ((r = sophon_value_to_array(vm, v, &arr)) != SOPHON_OK)
			goto end;

		sophon_array_for_each(arr, id, item) {
			sophon_value_set_int(vm, &stk->v[stk->sp - 1], id);

			stk->ip = iter.cont_ip;
			r = sophon_ins_run(vm);

			if ((r == SOPHON_CONTINUE) && (stk->ip == iter.cont_ip))
				continue;

			if ((r == SOPHON_BREAK) && (stk->ip == iter.brk_ip))
				goto break_end;

			if (r != SOPHON_NONE)
				goto end;
		}
	}

	if ((r = sophon_value_to_object(vm, v, &obj)) != SOPHON_OK)
		goto end;

	for (i = 0; i < obj->prop_bucket; i++) {
		Sophon_Property *prop;

		for (prop = obj->props[i]; prop; prop = iter.prop) {
			iter.prop = prop->next;

			if (!(prop->attrs & SOPHON_PROP_ATTR_ENUMERABLE))
				continue;

			sophon_value_set_string(vm, &stk->v[stk->sp - 1], prop->name);

			stk->ip = iter.cont_ip;
			r = sophon_ins_run(vm);

			if ((r == SOPHON_CONTINUE) && (stk->ip == iter.cont_ip))
				continue;

			if ((r == SOPHON_BREAK) && (stk->ip == iter.brk_ip))
				goto break_end;

			if (r != SOPHON_NONE)
				goto end;
		}
	}

break_end:
	stk->sp = iter.sp;
	stk->ip = end_ip;
	r = SOPHON_OK;
end:
	vm->pi_stack = iter.bottom;

	return r;
}

void
sophon_value_dump (Sophon_VM *vm, Sophon_Value v)
{
	Sophon_String *str;
	char *cstr;
	Sophon_U32 len;
	Sophon_Result r;

	if ((r = sophon_value_to_string_ex(vm, v, SOPHON_TRUE, &str))
				!= SOPHON_OK) {
		return;
	}

	if (sophon_string_new_utf8_cstr(vm, str, &cstr, &len) >= 0){
		sophon_pr("%s", cstr);
		sophon_string_free_utf8_cstr(vm, cstr, len);
	}
}

Sophon_Bool
sophon_value_is_bool (Sophon_Value v)
{
	Sophon_Object *obj;

	if (SOPHON_VALUE_IS_BOOL(v))
		return SOPHON_TRUE;

	if (!SOPHON_VALUE_IS_OBJECT(v))
		return SOPHON_FALSE;

	obj = SOPHON_VALUE_GET_OBJECT(v);

	return SOPHON_VALUE_IS_BOOL(obj->primv);
}

Sophon_Bool
sophon_value_is_number (Sophon_Value v)
{
	Sophon_Object *obj;

	if (SOPHON_VALUE_IS_INT(v))
		return SOPHON_TRUE;

	if (SOPHON_VALUE_IS_NUMBER(v))
		return SOPHON_TRUE;

	if (!SOPHON_VALUE_IS_OBJECT(v))
		return SOPHON_FALSE;

	obj = SOPHON_VALUE_GET_OBJECT(v);

	if (SOPHON_VALUE_IS_INT(obj->primv))
		return SOPHON_TRUE;

	return SOPHON_VALUE_IS_NUMBER(obj->primv);
}

Sophon_Bool
sophon_value_is_prim (Sophon_Value v, Sophon_U8 type)
{
	Sophon_GCObject *gco;
	Sophon_Object *obj;

	if (!SOPHON_VALUE_IS_GC(v))
		return SOPHON_FALSE;

	gco = SOPHON_VALUE_GET_GC(v);
	if (gco->gc_type == type)
		return SOPHON_TRUE;

	if (gco->gc_type != SOPHON_GC_Object)
		return SOPHON_FALSE;

	obj = (Sophon_Object*)gco;
	if (!SOPHON_VALUE_IS_GC(obj->primv))
		return SOPHON_FALSE;

	gco = SOPHON_VALUE_GET_GC(obj->primv);

	return gco->gc_type == type;
}

Sophon_Bool
sophon_value_is_object (Sophon_Value v)
{
	Sophon_GCObject *gco;

	if (!SOPHON_VALUE_IS_GC(v))
		return SOPHON_FALSE;

	gco = SOPHON_VALUE_GET_GC(v);
	if (gco->gc_type == SOPHON_GC_String)
		return SOPHON_FALSE;

	return SOPHON_TRUE;
}

