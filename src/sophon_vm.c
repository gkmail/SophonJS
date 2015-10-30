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
#include <sophon_string.h>
#include <sophon_stack.h>
#include <sophon_frame.h>
#include <sophon_value.h>
#include <sophon_object.h>
#include <sophon_function.h>
#include <sophon_ins.h>
#include <sophon_parser.h>
#include <sophon_module.h>
#include <sophon_vm.h>
#include <sophon_decl.h>
#include <sophon_date.h>
#include <sophon_debug.h>

static Sophon_String*
str_create (Sophon_VM *vm, const char *cstr)
{
	return sophon_string_intern(vm,
			sophon_string_from_utf8_cstr(vm, cstr));
}

static void
vm_init (Sophon_VM *vm)
{
	/*Initialize exception and return value*/
	sophon_value_set_undefined(vm, &vm->retv);
	sophon_value_set_undefined(vm, &vm->excepv);

	/*Get error objects*/
#define CREATE_ERROR(name) sophon_value_set_null(vm, &vm->name);
	SOPHON_FOR_EACH_ERROR(CREATE_ERROR)

	/*Create strings*/
	vm->empty_str = NULL;

#define CREATE_STRING(name)\
	vm->name##_str = str_create(vm, #name);
	SOPHON_FOR_EACH_STRING(CREATE_STRING)

#define CREATE_INTERNAL_STRING(name, str)\
	vm->name##_str = str_create(vm, str);
	SOPHON_FOR_EACH_INTERNAL_STRING(CREATE_INTERNAL_STRING)

	/*Create global module and global object*/
	sophon_global_init(vm);

	/*Create the global environment*/
	sophon_stack_push_global(vm, vm->glob_module->globv,
				SOPHON_VALUE_UNDEFINED, NULL, 0);

	sophon_gc_set_nb_count(vm, 0);
}

Sophon_VM*
sophon_vm_create (void)
{
	Sophon_VM *vm;

	vm = (Sophon_VM*)sophon_realloc(NULL, sizeof(Sophon_VM));
	if (!vm) {
		SOPHON_ERROR(("allocate virtual machine failed"));
		return NULL;
	}

	sophon_mm_init(vm);
	sophon_gc_init(vm);
	sophon_number_pool_init(vm);
	sophon_string_intern_hash_init(vm);
	sophon_stack_init(vm);
	sophon_parser_init(vm);
	sophon_prop_iter_stack_init(vm);

#ifdef SOPHON_DATE
	vm->tz_offset = sophon_timezone_offset(vm);
#endif

#ifdef SOPHON_CONSOLE
	sophon_hash_init(vm, &vm->timer_hash);
#endif

	vm_init(vm);

	SOPHON_INFO(("create new VM"));
	return vm;
}

void
sophon_vm_destroy(Sophon_VM *vm)
{
	Sophon_Ptr ptr;

	SOPHON_ASSERT(vm);

#ifdef SOPHON_CONSOLE
	sophon_hash_deinit(vm, &vm->timer_hash);
#endif

#ifdef SOPHON_MM_DEBUG
	sophon_mm_check_all();
#endif

	while (vm->stack) {
		sophon_stack_pop(vm);
	}

	sophon_parser_deinit(vm);
	sophon_stack_deinit(vm);
	sophon_number_pool_deinit(vm);
	sophon_gc_deinit(vm);
	sophon_string_intern_hash_deinit(vm);
	sophon_mm_deinit(vm);

	SOPHON_INFO(("maximum memory size used: %u bytes",
				vm->mm_max_used));
	SOPHON_INFO(("unfreed memory size: %u bytes",
				vm->mm_curr_used));

#ifdef SOPHON_MM_DEBUG
	sophon_mm_dump_unfreed();
#endif

	ptr = sophon_realloc(vm, 0);
	ptr = ptr;

	SOPHON_INFO(("release VM"));
}

void
sophon_trace (Sophon_VM *vm)
{
	Sophon_DeclFrame *decl;
	Sophon_Module *mod;
	Sophon_Stack *stk;
	Sophon_U32 level;
	char *buf;
	Sophon_U32 len;
	Sophon_U16 line;
	Sophon_Bool first = SOPHON_TRUE;

	SOPHON_ASSERT(vm);

	level = 0;
	stk = vm->stack;
	while (stk) {
		decl = (Sophon_DeclFrame*)stk->var_env;

		if (decl->func) {
			if (first) {
				sophon_prerr("stack:\n");
				first = SOPHON_FALSE;
			}

			sophon_prerr("\t%d: ", level);

			mod = decl->func->module;
			if (mod->name) {
				if (sophon_string_new_utf8_cstr(vm, mod->name,
								&buf, &len) >= 0) {
					sophon_prerr("\"%s\" ", buf);
					sophon_string_free_utf8_cstr(vm, buf, len);
				}
			}

			if (decl->func->name) {
				if (sophon_string_new_utf8_cstr(vm, decl->func->name,
								&buf, &len) >= 0) {
					sophon_prerr("%s ", buf);
					sophon_string_free_utf8_cstr(vm, buf, len);
				}
			} else if (decl->func->flags & SOPHON_FUNC_FL_EVAL) {
				sophon_prerr("[eval] ");
			}

#ifdef SOPHON_LINE_INFO
			line = sophon_ins_get_line(vm, decl->func,
						decl->func->f.ibuf + stk->ip);
			sophon_prerr("line: %d", line);
#endif

			sophon_prerr("\n");

			level++;
		}

		if (stk->var_env->gc_type == SOPHON_GC_GlobalFrame)
			break;

		stk = stk->bottom;
	}
}

void
sophon_throw_value (Sophon_VM *vm, Sophon_Value excepv)
{
	Sophon_String *str;
	char *buf;
	Sophon_U32 len;

	SOPHON_ASSERT(vm);

	if (!SOPHON_VALUE_IS_UNDEFINED(vm->excepv))
		return;

	vm->excepv = excepv;

	sophon_prerr("exception ");

	if (sophon_value_to_string(vm, vm->excepv, &str) == SOPHON_OK) {
		if (sophon_string_new_utf8_cstr(vm, str, &buf, &len) >= 0) {
			sophon_prerr("%s", buf);
			sophon_string_free_utf8_cstr(vm, buf, len);
		}
	}
	sophon_prerr("\n");

	sophon_trace(vm);
}

void
sophon_throw (Sophon_VM *vm, Sophon_Value excepv, const char *msg_info)
{
	Sophon_String *str;
	Sophon_Value msgv, ev;

	if (!SOPHON_VALUE_IS_UNDEFINED(vm->excepv))
		return;

	if (msg_info) {
		str = sophon_string_from_utf8_cstr(vm, msg_info);
		sophon_value_set_string(vm, &msgv, str);
	} else {
		sophon_value_set_undefined(vm, &msgv);
	}

	sophon_value_new(vm, excepv, &msgv, 1, &ev);

	sophon_throw_value(vm, ev);
}

Sophon_Bool
sophon_catch (Sophon_VM *vm, Sophon_Value *excepv)
{
	SOPHON_ASSERT(vm);

	if (SOPHON_VALUE_IS_UNDEFINED(vm->excepv))
		return SOPHON_FALSE;

	if (excepv)
		*excepv = vm->excepv;

	sophon_value_set_undefined(vm, &vm->excepv);
	return SOPHON_TRUE;
}

Sophon_Bool
sophon_strict (Sophon_VM *vm)
{
	Sophon_Stack *stk;

	SOPHON_ASSERT(vm);

	stk = vm->stack;
	while (stk) {
		if (stk->func) {
			return (stk->func->flags & SOPHON_FUNC_FL_STRICT) ?
						SOPHON_TRUE : SOPHON_FALSE;
		}
		stk = stk->bottom;
	}

	return SOPHON_FALSE;
}

