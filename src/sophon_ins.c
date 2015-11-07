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
#include <sophon_function.h>
#include <sophon_string.h>
#include <sophon_module.h>
#include <sophon_value.h>
#include <sophon_ins.h>
#include <sophon_stack.h>
#include <sophon_frame.h>
#include <sophon_object.h>
#include <sophon_closure.h>
#include <sophon_vm.h>
#include <sophon_debug.h>

#include "sophon_parser_internal.h"
#include "sophon_ins_internal.h"

#define DUMP_INS

static const char*
ins_tags[] = {
#define DECL_INS_NAME(name, model, stk) #name,
	FOR_EACH_INS(DECL_INS_NAME)
	NULL
};

static Sophon_ParserOpModel
ins_models[] = {
#define DECL_INS_MODEL(name, model, stk) MODEL_##model,
	FOR_EACH_INS(DECL_INS_MODEL)
	-1
};

static void
dump_func_var_entry (Sophon_VM *vm, Sophon_HashEntry *ent, Sophon_Ptr data)
{
	Sophon_IntPtr id = (Sophon_IntPtr)data;
	Sophon_String *name = (Sophon_String*)ent->key;
	char *cstr;
	Sophon_U32 len;

	if (ent->value == data) {
		if (id & SOPHON_VAR_FL_ARG) {
			sophon_pr(" arg: ");
		} else {
			sophon_pr(" var: ");
		}

		if (sophon_string_new_utf8_cstr(vm, name, &cstr, &len) >= 0) {
			sophon_pr("%s", cstr);
			sophon_string_free_utf8_cstr(vm, cstr, len);
		}
	}
}

static void
dump_func_var (Sophon_VM *vm, Sophon_Function *func, Sophon_Int id)
{
	if (id >= func->varc) {
		id = (id - func->varc) | SOPHON_VAR_FL_ARG;
	}

	sophon_hash_for_each(vm, &func->var_hash,
				dump_func_var_entry, (Sophon_Ptr)(Sophon_IntPtr)id);
}

static void
dump_const (Sophon_VM *vm, Sophon_Module *mod, Sophon_Int id)
{
	Sophon_Value c;

	c = sophon_module_get_const(mod, id);
	sophon_pr(" const: ");
	sophon_value_dump(vm, c);
}

const char*
sophon_ins_get_tag (Sophon_U8 ins)
{
	if (ins < SOPHON_ARRAY_SIZE(ins_tags) - 1)
		return ins_tags[ins];
	else
		return "ILLEGAL";
}

Sophon_Int
sophon_ins_dump (Sophon_VM *vm, Sophon_Function *func,
			Sophon_U8 *pi, Sophon_U32 len)
{
	const char *tag = sophon_ins_get_tag(*pi);
	Sophon_ParserOpModel model = ins_models[*pi];
	Sophon_Int r;

	SOPHON_ASSERT((pi >= func->f.ibuf) &&
				(pi + len <= func->f.ibuf + func->ibuf_size));

	if (*pi < SOPHON_ARRAY_SIZE(ins_tags) - 1)
		model = ins_models[*pi];
	else
		model = -1;

	switch (model) {
		case MODEL_Op:
			r = 1;
			break;
		case MODEL_Const:
			r = 3;
			break;
		case MODEL_Jump:
			r = 3;
			break;
		case MODEL_Call:
			r = 2;
			break;
		case MODEL_StackOp:
			r = 2;
			break;
		default:
			r = 1;
			break;
	}

	if (r > len) {
		sophon_pr("ILLEGAL");
	} else {
		Sophon_U16 n;

		sophon_pr("%-12s", tag);

		if (r == 2) {
			n = pi[1];
			sophon_pr(" %5u", n);
		} else if (r == 3) {
			n = (pi[1] << 8) | pi[2];

			sophon_pr(" %5u", n);

			if ((*pi == I_get_var) || (*pi == I_put_var)) {
				dump_func_var(vm, func, n);
			} else if ((*pi == I_number) || (*pi == I_closure)) {
			} else if (model == MODEL_Const) {
				dump_const(vm, func->module, n);
			}
		}
	}

	return r;
}

void
sophon_ins_dump_buffer (Sophon_VM *vm, Sophon_Function *func,
			Sophon_U8 *pi, Sophon_U32 len, Sophon_U32 flags)
{
	Sophon_Int off = 0;
	Sophon_Int left = len;
	Sophon_Int r;

	SOPHON_ASSERT((pi >= func->f.ibuf) &&
				(pi + len <= func->f.ibuf + func->ibuf_size));

	while (left > 0) {
#ifdef SOPHON_LINE_INFO
		if (func->lbuf && (flags & SOPHON_INS_FL_LINE)) {
			sophon_pr("%05d:", sophon_ins_get_line(vm, func, pi));
		}
#endif /*SOPHON_LINE_INFO*/

		if (flags & SOPHON_INS_FL_IP)
			sophon_pr("%05d: ", off);

		r = sophon_ins_dump(vm, func, pi, left);

		pi  += r;
		off += r;
		left -= r;

		sophon_pr("\n");
	}
}

#ifdef SOPHON_LINE_INFO
Sophon_U16
sophon_ins_get_line (Sophon_VM *vm, Sophon_Function *func,
			Sophon_U8 *pi)
{
	Sophon_U16 off;
	Sophon_Int min, max, mid;

	SOPHON_ASSERT((pi >= func->f.ibuf) &&
				(pi <= func->f.ibuf + func->ibuf_size));

	if (!func->lbuf_size)
		return 0;

	off = pi - func->f.ibuf;
	min = 0;
	max = func->lbuf_size;

	while (1) {
		mid = min + (max - min) / 2;

		if (func->lbuf[mid].offset == off)
			return func->lbuf[mid].line;

		if (mid == min)
			break;
		
		if (func->lbuf[mid].offset < off)
			min = mid;
		else
			max = mid;
	}

	return func->lbuf[min].line;
}

#endif /*SOPHON_LINE_INFO*/

#define THROW\
	SOPHON_MACRO_BEGIN\
		ret = SOPHON_ERR_THROW;\
		goto exception;\
	SOPHON_MACRO_END
#define TO_NUMBER(v, d)\
	SOPHON_MACRO_BEGIN\
		if ((r = sophon_value_to_number(vm, v, &d)) != SOPHON_OK)\
			THROW;\
	SOPHON_MACRO_END
#define TO_INT(v, i)\
	SOPHON_MACRO_BEGIN\
		if ((r = sophon_value_to_int(vm, v, &i)) != SOPHON_OK)\
			THROW;\
	SOPHON_MACRO_END
#define TO_S32(v, i)\
	SOPHON_MACRO_BEGIN\
		if ((r = sophon_value_to_s32(vm, v, &i)) != SOPHON_OK)\
			THROW;\
	SOPHON_MACRO_END
#define TO_U32(v, i)\
	SOPHON_MACRO_BEGIN\
		if ((r = sophon_value_to_u32(vm, v, &i)) != SOPHON_OK)\
			THROW;\
	SOPHON_MACRO_END
#define TO_STRING(v, s)\
	SOPHON_MACRO_BEGIN\
		if ((r = sophon_value_to_string(vm, v, &s)) != SOPHON_OK)\
			THROW;\
	SOPHON_MACRO_END

#define I_store_run\
	if (CURR_FUNC->flags & SOPHON_FUNC_FL_EVAL)\
		TOP->retv = STACK(0);
#define I_undef_run\
	STACK(-1) = SOPHON_VALUE_UNDEFINED;
#define I_null_run\
	STACK(-1) = SOPHON_VALUE_NULL;
#define I_true_run\
	STACK(-1) = SOPHON_VALUE_TRUE;
#define I_false_run\
	STACK(-1) = SOPHON_VALUE_FALSE;
#define I_one_run\
	sophon_value_set_int(vm, &STACK(-1), 1);
#define I_zero_run\
	sophon_value_set_int(vm, &STACK(-1), 0);

#define MATH_run(op)\
	Sophon_Number d1, d2;\
	TO_NUMBER(STACK(1), d1);\
	TO_NUMBER(STACK(0), d2);\
	sophon_value_set_number(vm, &STACK(1), op);

#define I_add_run\
	Sophon_Value pv1, pv2;\
	if ((r = sophon_value_to_prim(vm, STACK(1), &pv1, SOPHON_PRIM_UNDEF))\
				!= SOPHON_OK)\
		THROW;\
	if ((r = sophon_value_to_prim(vm, STACK(0), &pv2, SOPHON_PRIM_UNDEF))\
				!= SOPHON_OK)\
		THROW;\
	if (sophon_value_is_string(pv1) || sophon_value_is_string(pv2)) {\
		Sophon_String *s1, *s2, *nstr;\
		TO_STRING(pv1, s1);\
		TO_STRING(pv2, s2);\
		nstr = sophon_string_concat(vm, s1, s2);\
		sophon_value_set_string(vm, &STACK(1), nstr);\
	} else {\
		Sophon_Number d1, d2;\
		TO_NUMBER(pv1, d1);\
		TO_NUMBER(pv2, d2);\
		sophon_value_set_number(vm, &STACK(1), d1 + d2);\
	}
#define I_sub_run\
	MATH_run(d1 - d2);
#define I_mul_run\
	MATH_run(d1 * d2);
#define I_div_run\
	MATH_run(d1 / d2);
#define I_mod_run\
	MATH_run(sophon_fmod(d1, d2));

#define I_shl_run\
	Sophon_S32 n;\
	Sophon_U32 b;\
	TO_S32(STACK(1), n);\
	TO_U32(STACK(0), b);\
	sophon_value_set_int(vm, &STACK(1), n << (b & 0x1F));
#define I_shr_run\
	Sophon_S32 n;\
	Sophon_U32 b;\
	TO_S32(STACK(1), n);\
	TO_U32(STACK(0), b);\
	sophon_value_set_int(vm, &STACK(1), SOPHON_SHR(n, (b & 0x1F)));
#define I_ushr_run\
	Sophon_U32 n;\
	Sophon_U32 b;\
	TO_U32(STACK(1), n);\
	TO_U32(STACK(0), b);\
	sophon_value_set_number(vm, &STACK(1), n >> (b & 0x1F));

#define CMP_run(cmp)\
	Sophon_Int i;\
	if (sophon_value_compare(vm, STACK(1), STACK(0), &i) != SOPHON_OK) {\
		THROW;\
	}\
	if (i == -2)\
		sophon_value_set_undefined(vm, &STACK(1));\
	else if (i cmp 0)\
		sophon_value_set_bool(vm, &STACK(1), SOPHON_TRUE);\
	else\
		sophon_value_set_bool(vm, &STACK(1), SOPHON_FALSE);
#define I_lt_run\
	CMP_run(<);
#define I_le_run\
	CMP_run(<=);
#define I_gt_run\
	CMP_run(>);
#define I_ge_run\
	CMP_run(>=);

#define I_equ_run\
	Sophon_Bool b;\
	if (sophon_value_equal_compare(vm, STACK(1), STACK(0), &b) != SOPHON_OK)\
		THROW;\
	sophon_value_set_bool(vm, &STACK(1), b);

#define I_same_run\
	Sophon_Bool b = sophon_value_same(vm, STACK(1), STACK(0));\
	sophon_value_set_bool(vm, &STACK(1), b);
#define I_band_run\
	Sophon_S32 n1, n2;\
	TO_S32(STACK(1), n1);\
	TO_S32(STACK(0), n2);\
	sophon_value_set_int(vm, &STACK(1), n1 & n2);
#define I_bor_run\
	Sophon_S32 n1, n2;\
	TO_S32(STACK(1), n1);\
	TO_S32(STACK(0), n2);\
	sophon_value_set_int(vm, &STACK(1), n1 | n2);
#define I_xor_run\
	Sophon_S32 n1, n2;\
	TO_S32(STACK(1), n1);\
	TO_S32(STACK(0), n2);\
	sophon_value_set_int(vm, &STACK(1), n1 ^ n2);
#define I_in_run\
	Sophon_Bool b;\
	if (sophon_value_in(vm, STACK(1), STACK(0), &b) != SOPHON_OK) {\
		THROW;\
	}\
	sophon_value_set_bool(vm, &STACK(1), b);
#define I_instanceof_run\
	Sophon_Bool b;\
	if (sophon_value_instanceof(vm, STACK(1), STACK(0), &b) != SOPHON_OK) {\
		THROW;\
	}\
	sophon_value_set_bool(vm, &STACK(1), b);
#define I_neg_run\
	Sophon_Number d;\
	TO_NUMBER(STACK(0), d);\
	sophon_value_set_number(vm, &STACK(0), -d);
#define I_not_run\
	Sophon_Bool b = sophon_value_to_bool(vm, STACK(0));\
	sophon_value_set_bool(vm, &STACK(0), !b);
#define I_rev_run\
	Sophon_S32 i;\
	TO_S32(STACK(0), i);\
	sophon_value_set_int(vm, &STACK(0), ~i);
#define I_to_num_run\
	Sophon_Number n;\
	if ((r = sophon_value_to_number(vm, STACK(0), &n)) != SOPHON_OK)\
		THROW;\
	sophon_value_set_number(vm, &STACK(0), n);
#define I_new_run\
	if (sophon_value_new(vm, STACK(argc), &STACK(argc-1), argc,\
					&STACK(argc)) != SOPHON_OK) {\
		THROW;\
	}
#define I_delete_run\
	Sophon_U32 flags = (CURR_FUNC->flags & SOPHON_FUNC_FL_STRICT) ?\
				SOPHON_FL_THROW : 0;\
	r = sophon_value_delete_prop(vm, STACK(1), STACK(0), flags);\
	if (flags && (r != SOPHON_OK) && (r != SOPHON_ERR_ACCESS)) {\
		ret = SOPHON_ERR_THROW;\
		goto exception;\
	}\
	sophon_value_set_bool(vm, &STACK(1), r == SOPHON_OK ?\
				SOPHON_TRUE : SOPHON_FALSE);
#define I_delete_bind_run\
	Sophon_String *name = SOPHON_VALUE_GET_STRING(CONST(id));\
	Sophon_Bool b;\
	if (sophon_strict(vm)) {\
		sophon_throw(vm, vm->SyntaxError,\
					"Cannot delete binding in strict mode");\
		THROW;\
	}\
	r = sophon_stack_delete_binding(vm, name, 0);\
	b = (r == SOPHON_OK) ? SOPHON_TRUE : SOPHON_FALSE;\
	sophon_value_set_bool(vm, &STACK(-1), b);
#define I_typeof_run\
	Sophon_String *str = sophon_value_typeof(vm, STACK(0));\
	sophon_value_set_gc(vm, &STACK(0), (Sophon_GCObject*)str);
#define I_this_run\
	if (sophon_value_is_null(VAR_ENV->thisv) ||\
				sophon_value_is_undefined(VAR_ENV->thisv))\
		STACK(-1) = CURR_MODULE->base->globv;\
	else\
		STACK(-1) = VAR_ENV->thisv;
#define I_const_run\
	STACK(-1) = CONST(id);
#define I_closure_run\
	Sophon_Function *func = FUNC(id);\
	Sophon_Closure *clos = sophon_closure_create(vm, func);\
	sophon_value_set_gc(vm, &STACK(-1), (Sophon_GCObject*)clos);
#define I_get_run\
	if (sophon_value_get(vm, STACK(1), STACK(0), &STACK(1), SOPHON_FL_THROW)\
				!= SOPHON_OK) {\
		THROW;\
	}
#define I_put_run\
	Sophon_U32 flags = sophon_strict(vm) ? SOPHON_FL_THROW : 0;\
	if ((r = sophon_value_put(vm, STACK(2), STACK(1), STACK(0), flags))\
				!= SOPHON_OK) {\
		THROW;\
	}
#define I_call_run\
	Sophon_Value thisv = sophon_stack_get_this(vm);\
	IP += 2;\
	SP -= argc;\
	r = sophon_value_call(vm, STACK(0), thisv, &STACK(-1),\
				argc, &STACK(0), SOPHON_FL_LAZY);\
	if (r != SOPHON_OK) {\
		THROW;\
	}\
	if (TOP != vm->stack) {\
		UPDATE();\
	}\
	break;
#define I_this_call_run\
	IP += 2;\
	SP -= argc + 1;\
	r = sophon_value_call(vm, STACK(-1), STACK(0), &STACK(-2),\
				argc, &STACK(0), SOPHON_FL_LAZY);\
	if (r != SOPHON_OK) {\
		THROW;\
	}\
	if (TOP != vm->stack) {\
		UPDATE();\
	}\
	break;
#define I_jt_run\
	Sophon_Bool b = sophon_value_to_bool(vm, STACK(0));\
	if (b) {\
		IP = pos;\
		SP--;\
		sophon_gc_set_nb_count(vm, gc_level);\
		break;\
	}
#define I_jf_run\
	Sophon_Bool b = sophon_value_to_bool(vm, STACK(0));\
	if (!b) {\
		IP = pos;\
		SP--;\
		sophon_gc_set_nb_count(vm, gc_level);\
		break;\
	}
#define I_jmp_run\
	IP = pos;\
	sophon_gc_set_nb_count(vm, gc_level);\
	break;
#define I_throw_run\
	sophon_throw_value(vm, STACK(0));\
	sophon_gc_set_nb_count(vm, gc_level);\
	THROW;
#define I_return_run\
	TOP->retv = STACK(0);\
	sophon_gc_set_nb_count(vm, gc_level);\
	ret = SOPHON_OK;\
	goto exception;
#define I_with_run\
	sophon_stack_push_with(vm, STACK(0))
#define I_catch_run\
	Sophon_Value namev = CONST(id);\
	Sophon_String *name = (Sophon_String*)SOPHON_VALUE_GET_GC(namev);\
	sophon_stack_push_name(vm, name, vm->excepv);\
	sophon_value_set_undefined(vm, &vm->excepv);
#define I_pop_frame_run\
	sophon_stack_pop_frame(vm)
#define I_for_in_run\
	IP += 3;\
	SP ++;\
	r = sophon_value_for_in(vm, pos);\
	if (r == SOPHON_OK)\
		break;\
	if (r < 0) {\
		ret = r;\
		goto exception;\
	}\
	if (TOP->pi_bottom != vm->pi_stack) {\
		if ((SP < vm->pi_stack->sp) ||\
					(IP == vm->pi_stack->brk_ip) ||\
					(IP == vm->pi_stack->cont_ip))\
			return r;\
	}\
	break;
#define I_for_next_run\
	return SOPHON_OK;
#define I_try_begin_run\
	sophon_value_set_int(vm, &STACK(-2), TP);\
	sophon_value_set_int(vm, &STACK(-1), pos);\
	TP = SP;
#define I_try_end_run\
	TP = SOPHON_VALUE_GET_INT(STACK(0));\
	SP -= 2;\
	IP = pos;\
	break;
#define I_prop_set_run\
	if (sophon_value_define_prop(vm, STACK(2), STACK(1),\
					SOPHON_VALUE_UNDEFINED,\
					STACK(0),\
					SOPHON_PROP_ATTR_CONFIGURABLE,\
					SOPHON_FL_HAVE_CONFIGURABLE|SOPHON_FL_HAVE_SET|\
					SOPHON_FL_THROW)\
			!= SOPHON_OK) {\
		THROW;\
	}
#define I_prop_get_run\
	if (sophon_value_define_prop(vm, STACK(2), STACK(1),\
					STACK(0),\
					SOPHON_VALUE_UNDEFINED,\
					SOPHON_PROP_ATTR_CONFIGURABLE,\
					SOPHON_FL_HAVE_CONFIGURABLE|SOPHON_FL_HAVE_GET|\
					SOPHON_FL_THROW)\
			!= SOPHON_OK) {\
		THROW;\
	}
#define I_dup_run\
	STACK(-1) = STACK(n);
#define I_mov_run\
	STACK(n) = STACK(0);
#define I_pop_run
#define I_break_run\
	IP = pos;\
	sophon_gc_set_nb_count(vm, gc_level);\
	if (TOP->pi_bottom != vm->pi_stack) {\
		if ((SP < vm->pi_stack->sp) ||\
					(IP == vm->pi_stack->brk_ip))\
			return SOPHON_BREAK;\
	}\
	break;
#define I_continue_run\
	IP = pos;\
	sophon_gc_set_nb_count(vm, gc_level);\
	if (TOP->pi_bottom != vm->pi_stack) {\
		if ((SP < vm->pi_stack->sp) ||\
					(IP == vm->pi_stack->cont_ip))\
			return SOPHON_CONTINUE;\
	}\
	break;
#define I_debugger_run
#define I_get_bind_run\
	Sophon_String *name = SOPHON_VALUE_GET_STRING(CONST(id));\
	r = sophon_stack_get_binding(vm, name, &STACK(-1), 0);\
	if (r == SOPHON_NONE) {\
		sophon_throw(vm, vm->ReferenceError, "binding has not been defined");\
		THROW;\
	} else if (r != SOPHON_OK) {\
		THROW;\
	}
#define I_get_bind_nt_run\
	Sophon_String *name = SOPHON_VALUE_GET_STRING(CONST(id));\
	r = sophon_stack_get_binding(vm, name, &STACK(-1), 0);\
	if (r == SOPHON_NONE) {\
		sophon_value_set_undefined(vm, &STACK(-1));\
	} else if (r != SOPHON_OK) {\
		THROW;\
	}
#define I_put_bind_run\
	Sophon_String *name = SOPHON_VALUE_GET_STRING(CONST(id));\
	if (sophon_stack_put_binding(vm, name, STACK(0), 0) != SOPHON_OK) {\
		THROW;\
	}
#define I_put_fbind_run\
	Sophon_String *name = SOPHON_VALUE_GET_STRING(CONST(id));\
	if (sophon_stack_put_binding(vm, name, STACK(0), SOPHON_FL_FORCE)\
			!= SOPHON_OK) {\
		THROW;\
	}
#define I_get_var_run\
	STACK(-1) = VAR(id);
#define I_put_var_run\
	VAR(id) = STACK(0);
#define I_number_run\
	sophon_value_set_int(vm, &STACK(-1), id);

Sophon_Result
sophon_ins_run (Sophon_VM *vm)
{
	Sophon_Stack *enter_stack;
	Sophon_Stack *top_stack;
	Sophon_DeclFrame *var_env;
	Sophon_Function *curr_func;
	Sophon_Module *curr_module;
	Sophon_U16 ibuf_size;
	Sophon_U32 gc_level = sophon_gc_get_nb_count(vm);
	Sophon_Result r;

	Sophon_U16 argc;
	Sophon_U16 id;
	Sophon_U16 pos;
	Sophon_U8 n;

	Sophon_Result ret = SOPHON_NONE;

	SOPHON_INFO(("run"));

#define TOP      top_stack
#define VAR_ENV  var_env
#define CURR_FUNC     curr_func
#define CURR_MODULE   curr_module
#define IBUF_SIZE     ibuf_size

#define UPDATE()\
	SOPHON_MACRO_BEGIN\
		TOP = vm->stack;\
		VAR_ENV = (Sophon_DeclFrame*)TOP->var_env;\
		CURR_FUNC = TOP->func;\
		CURR_MODULE = CURR_FUNC->module;\
		IBUF_SIZE = CURR_FUNC->ibuf_size;\
	SOPHON_MACRO_END

#define SP (TOP->sp)
#define IP (TOP->ip)
#define TP (TOP->tp)

#define INS(n)   (CURR_FUNC->f.ibuf[IP+(n)])
#define STACK(n) (TOP->v[SP-(n)-1])
#define VAR(n)   (VAR_ENV->v[n])
#define CONST(n) (sophon_module_get_const(CURR_MODULE, (n)))
#define FUNC(n)  (sophon_module_get_func(CURR_MODULE, (n)))

	SOPHON_ASSERT(vm);

	enter_stack = vm->stack;

	UPDATE();

again:
	while (IP < IBUF_SIZE) {
		switch (INS(0)) {
#define GET_PARAM(name)    (name)
#define INS_PRE_OP_Op
#define INS_PRE_OP_Const    id = (INS(1) << 8) | INS(2);
#define INS_PRE_OP_Call     argc = INS(1);
#define INS_PRE_OP_Jump     pos = (INS(1) << 8) | INS(2);
#define INS_PRE_OP_StackOp  n = INS(1);
#define INS_POST_OP_Op      IP++;
#define INS_POST_OP_Const   IP += 3;
#define INS_POST_OP_Call    IP += 2;
#define INS_POST_OP_Jump    IP += 3;
#define INS_POST_OP_StackOp IP += 2;
#if defined(SOPHON_DEBUG) && defined(DUMP_INS)
	#define INS_DEBUG()\
		SOPHON_MACRO_BEGIN\
			sophon_pr("%03d:%05d: ", SP, IP);\
			sophon_ins_dump(vm, CURR_FUNC, &INS(0),\
						IBUF_SIZE - IP);\
			sophon_pr("\n");\
		SOPHON_MACRO_END
#else
	#define INS_DEBUG()
#endif
#define RUN_INS(name, model, stk)\
			case I_##name:\
				INS_DEBUG();\
				INS_PRE_OP_##model;\
				{I_##name##_run;}\
				INS_POST_OP_##model;\
				SP += stk;\
				break;

			FOR_EACH_INS(RUN_INS)
			default:
				SOPHON_ASSERT(0);
				break;
		}

		SOPHON_ASSERT(SP >= 0);
	}

exception:
	if (TP != 0xFFFF) {
		if ((TOP->pi_bottom != vm->pi_stack) && (vm->pi_stack->sp > TP))
			return ret;

		SP = TP;
		TP = SOPHON_VALUE_GET_INT(STACK(-2));
		IP = SOPHON_VALUE_GET_INT(STACK(-1));
		if (ret < 0)
			ret = SOPHON_NONE;
		goto again;
	}

	if (TOP->pi_bottom != vm->pi_stack) {
		return ret;
	}

	if (enter_stack != top_stack) {
		Sophon_Value rv = TOP->retv;

		sophon_stack_pop(vm);
		UPDATE();

		if (ret >= 0) {
			ret = SOPHON_NONE;
			STACK(0) = rv;
			goto again;
		} else {
			goto exception;
		}
	}

	return ret;
}

