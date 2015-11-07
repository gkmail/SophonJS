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
#include <sophon_stack.h>
#include <sophon_frame.h>
#include <sophon_closure.h>
#include <sophon_object.h>
#include <sophon_function.h>
#include <sophon_string.h>
#include <sophon_module.h>
#include <sophon_debug.h>

static Sophon_Stack*
stack_alloc (Sophon_VM *vm, Sophon_Int vsize)
{
	Sophon_Stack *stk;
	Sophon_U32 size;

	size = sizeof(Sophon_Stack) + (vsize - 1) * sizeof(Sophon_Value);

	stk = (Sophon_Stack*)sophon_mm_alloc_ensure(vm, size);

	stk->pi_bottom = vm->pi_stack;
	stk->vbuf_size = vsize;
	stk->func = NULL;
	stk->ip = 0;
	stk->sp = 0;
	stk->tp = 0xFFFF;

	sophon_value_set_undefined(vm, &stk->retv);

	stk->bottom = vm->stack;
	vm->stack = stk;

	if (vsize)
		sophon_memset(stk->v, 0xFF, vsize * sizeof(Sophon_Value));

	return stk;
}

Sophon_Result
sophon_stack_push_global (Sophon_VM *vm, Sophon_Value globv,
			Sophon_Value calleev, Sophon_Value *argv, Sophon_U8 argc)
{
	Sophon_Stack *stk;
	Sophon_GlobalFrame *frame;
	Sophon_Closure *clos;
	Sophon_Function *func;
	Sophon_Int ssize;
	Sophon_Result r;

	SOPHON_ASSERT(vm);
	
	if (SOPHON_VALUE_IS_UNDEFINED(calleev)) {
		func  = NULL;
		ssize = 0;
	} else {
		r = sophon_value_to_closure(vm, calleev, &clos);
		SOPHON_ASSERT(r == SOPHON_OK);
		SOPHON_ASSERT(!(clos->gc_flags & SOPHON_GC_FL_BIND));
		r = r;

		func  = clos->c.func.func;
		ssize = func->stack_size;

		SOPHON_ASSERT(func->flags & SOPHON_FUNC_FL_GLOBAL);
	}

	stk = stack_alloc(vm, ssize);

	frame = sophon_global_frame_create(vm, SOPHON_VALUE_UNDEFINED,
				calleev, globv, argv, argc);
	SOPHON_ASSERT(frame);

	((Sophon_Frame*)frame)->bottom = NULL;

	stk->func    = func;
	stk->calleev = calleev;
	stk->var_env = (Sophon_Frame*)frame;
	stk->lex_env = (Sophon_Frame*)frame;

	return SOPHON_OK;
}

Sophon_Result
sophon_stack_push_decl (Sophon_VM *vm, Sophon_Value thisv,
			Sophon_Value calleev, Sophon_Value *argv, Sophon_U8 argc)
{
	Sophon_Stack *stk, *bottom;
	Sophon_Frame *var_frame, *lex_frame;
	Sophon_Closure *clos;
	Sophon_Function *func;
	Sophon_Int ssize;
	Sophon_Result r;

	SOPHON_ASSERT(vm);
	
	r = sophon_value_to_closure(vm, calleev, &clos);
	SOPHON_ASSERT(r == SOPHON_OK);
	SOPHON_ASSERT(!(clos->gc_flags & SOPHON_GC_FL_BIND));
	r = r;

	func  = clos->c.func.func;
	ssize = func->stack_size;

	SOPHON_ASSERT(!(func->flags & SOPHON_FUNC_FL_GLOBAL));

	bottom = vm->stack;
	SOPHON_ASSERT(bottom);

	stk = stack_alloc(vm, ssize);

	if ((func->flags & SOPHON_FUNC_FL_EVAL) &&
				!(func->flags & SOPHON_FUNC_FL_STRICT)) {
		var_frame = bottom->var_env;
		lex_frame = bottom->lex_env;
	} else {
		var_frame = (Sophon_Frame*)sophon_decl_frame_create(vm,
					bottom->calleev, calleev, thisv, argv, argc);
		SOPHON_ASSERT(var_frame);

		var_frame->bottom = clos->c.func.lex_env;
		lex_frame = var_frame;
	}

	stk->func    = func;
	stk->calleev = calleev;
	stk->var_env = (Sophon_Frame*)var_frame;
	stk->lex_env = (Sophon_Frame*)lex_frame;

	return SOPHON_OK;
}

Sophon_Result
sophon_stack_push_name (Sophon_VM *vm, Sophon_String *name,
			Sophon_Value v)
{
	Sophon_Frame *frame;
	Sophon_Stack *top;

	SOPHON_ASSERT(vm);

	top = vm->stack;
	SOPHON_ASSERT(top);

	frame = (Sophon_Frame*)sophon_name_frame_create(vm, name, v);
	frame->bottom = top->lex_env;
	top->lex_env = frame;

	return SOPHON_OK;
}

Sophon_Result
sophon_stack_push_with (Sophon_VM *vm, Sophon_Value thisv)
{
	Sophon_Frame *frame;
	Sophon_Stack *top;

	SOPHON_ASSERT(vm);

	top = vm->stack;
	SOPHON_ASSERT(top);

	frame = (Sophon_Frame*)sophon_with_frame_create(vm, thisv);
	frame->bottom = top->lex_env;
	top->lex_env = frame;

	return SOPHON_OK;
}

void
sophon_stack_pop (Sophon_VM *vm)
{
	Sophon_Frame *frame, *bframe;
	Sophon_Stack *top, *bottom;
	Sophon_U32 size;

	SOPHON_ASSERT(vm);

	top = vm->stack;
	SOPHON_ASSERT(top);

	bottom = top->bottom;

	vm->stack = bottom;

	frame = top->lex_env;
	SOPHON_ASSERT(frame && top->var_env);

	while (1) {
		bframe = frame->bottom;

		if (!(frame->gc_flags & SOPHON_GC_FL_MANAGED)) {
			sophon_frame_destroy(vm, frame);
		}

		if (frame == top->var_env)
			break;

		frame = bframe;
	}

	size = sizeof(Sophon_Stack);
	size += (top->vbuf_size - 1) * sizeof(Sophon_Value);

	sophon_mm_free(vm, top, size);
}

void
sophon_stack_pop_frame (Sophon_VM *vm)
{
	Sophon_Frame *frame, *bottom;

	SOPHON_ASSERT(vm && vm->stack);

	frame = vm->stack->lex_env;
	SOPHON_ASSERT(frame);

	bottom = frame->bottom;
	SOPHON_ASSERT(bottom);

	vm->stack->lex_env = bottom;

	if (!(frame->gc_flags & SOPHON_GC_FL_MANAGED))
		sophon_frame_destroy(vm, frame);
}

Sophon_Result
sophon_stack_delete_binding (Sophon_VM *vm, Sophon_String *name,
		Sophon_U32 flags)
{
	Sophon_Stack *stack;
	Sophon_Frame *frame;
	Sophon_Result r;

	SOPHON_ASSERT(vm && name);

	stack = vm->stack;
	frame = stack->lex_env;

	while (frame) {
		switch (frame->gc_type) {
			case SOPHON_GC_GlobalFrame:
			case SOPHON_GC_DeclFrame: {
				Sophon_DeclFrame *df = (Sophon_DeclFrame*)frame;
				Sophon_HashEntry *ent;

				if (df->func) {
					r = sophon_hash_lookup(vm, &df->func->var_hash,
								sophon_direct_key,
								sophon_direct_equal,
								(Sophon_Ptr)name,
								&ent);
					if (r == SOPHON_OK)
						return SOPHON_ERR_ACCESS;

					if (name == df->func->name)
						return SOPHON_ERR_ACCESS;
				}

				if (name == vm->arguments_str)
					return SOPHON_ERR_ACCESS;

				if (frame->gc_type == SOPHON_GC_GlobalFrame) {
					Sophon_Module *mod;

					if (df->func) {
						mod = df->func->module->base;
						if (mod != vm->glob_module) {
							if ((r = sophon_value_delete_prop(vm, mod->globv,
											SOPHON_VALUE_GC(name),
											SOPHON_FL_NONE|SOPHON_FL_THROW))
									!= SOPHON_NONE)
								return r;
						}
					}

					mod = vm->glob_module;
					return sophon_value_delete_prop(vm, mod->globv,
							SOPHON_VALUE_GC(name),
							SOPHON_FL_NONE|SOPHON_FL_THROW);
				} else {
					return sophon_hash_remove(vm, &df->var_hash,
							sophon_direct_key,
							sophon_direct_equal,
							(Sophon_Ptr)name);
				}
				break;
			}
			case SOPHON_GC_NameFrame: {
				Sophon_NameFrame *nf = (Sophon_NameFrame*)frame;

				if (nf->name == name)
					return SOPHON_ERR_ACCESS;
				break;
			}
			case SOPHON_GC_WithFrame: {
				Sophon_WithFrame *wf = (Sophon_WithFrame*)frame;
				Sophon_Value namev;

				sophon_value_set_gc(vm, &namev, (Sophon_GCObject*)name);
				return sophon_value_delete_prop(vm, wf->withv, namev,
							SOPHON_FL_NONE);
			}
			default:
				SOPHON_ASSERT(0);
		}

		frame = frame->bottom;
	}

	return SOPHON_NONE;
}

Sophon_Result
sophon_stack_get_binding (Sophon_VM *vm, Sophon_String *name,
			Sophon_Value *getv, Sophon_U32 flags)
{
	Sophon_Stack *stack;
	Sophon_Frame *frame;
	Sophon_Result r;

	SOPHON_ASSERT(vm && name);

	stack = vm->stack;
	frame = stack->lex_env;

	while (frame) {
		switch (frame->gc_type) {
			case SOPHON_GC_GlobalFrame:
			case SOPHON_GC_DeclFrame: {
				Sophon_DeclFrame *df = (Sophon_DeclFrame*)frame;
				Sophon_HashEntry *ent;

				if (df->func) {
					r = sophon_hash_lookup(vm, &df->func->var_hash,
								sophon_direct_key,
								sophon_direct_equal,
								(Sophon_Ptr)name,
								&ent);
					if (r == SOPHON_OK) {
						Sophon_Int id = (Sophon_IntPtr)ent->value;

						if (id & SOPHON_VAR_FL_ARG) {
							id = (id & ~SOPHON_VAR_FL_ARG) + df->func->varc;
						}

						*getv = df->v[id];
						return SOPHON_OK;
					}

					if (name == df->func->name) {
						*getv = df->calleev;
						return SOPHON_OK;
					}
				}

				if (name == vm->arguments_str) {
					sophon_decl_frame_create_args(vm, df);
					*getv = df->argsv;
					return SOPHON_OK;
				}

				if (frame->gc_type == SOPHON_GC_GlobalFrame) {
					Sophon_Module *mod;

					if (df->func) {
						mod = df->func->module->base;
						if (mod != vm->glob_module) {
							if ((r = sophon_value_get(vm, mod->globv,
										SOPHON_VALUE_GC(name),
										getv,
										SOPHON_FL_THROW|SOPHON_FL_NONE)) !=
										SOPHON_NONE)
								return r;
						}
					}

					mod = vm->glob_module;
					return sophon_value_get(vm, mod->globv,
							SOPHON_VALUE_GC(name),
							getv,
							SOPHON_FL_THROW|SOPHON_FL_NONE);
				} else {
					r = sophon_hash_lookup(vm, &df->var_hash,
							sophon_direct_key,
							sophon_direct_equal,
							(Sophon_Ptr)name,
							&ent);
					if (r == SOPHON_OK) {
						*getv = (Sophon_Value)ent->value;
						return r;
					}
				}
				break;
			}
			case SOPHON_GC_NameFrame: {
				Sophon_NameFrame *nf = (Sophon_NameFrame*)frame;

				if (nf->name == name) {
					*getv = nf->v;
					return SOPHON_OK;
				}
				break;
			}
			case SOPHON_GC_WithFrame: {
				Sophon_WithFrame *wf = (Sophon_WithFrame*)frame;

				if ((r = sophon_value_get(vm, wf->withv,
							SOPHON_VALUE_GC(name),
							getv, SOPHON_FL_NONE)) != SOPHON_NONE)
					return r;
				break;
			}
			default:
				SOPHON_ASSERT(0);
		}

		frame = frame->bottom;
	}

	return SOPHON_NONE;
}

Sophon_Result
sophon_stack_put_binding (Sophon_VM *vm, Sophon_String *name,
			Sophon_Value setv, Sophon_U32 flags)
{
	Sophon_Stack *stack;
	Sophon_Frame *frame;
	Sophon_Result r;
	Sophon_Bool strict;

	SOPHON_ASSERT(vm && name);

	stack = vm->stack;
	frame = stack->lex_env;

	if (stack->func) {
		strict = stack->func->flags & SOPHON_FUNC_FL_STRICT;
	} else {
		strict = SOPHON_TRUE;
	}

	while (frame) {
		switch (frame->gc_type) {
			case SOPHON_GC_GlobalFrame:
			case SOPHON_GC_DeclFrame: {
				Sophon_DeclFrame *df = (Sophon_DeclFrame*)frame;
				Sophon_HashEntry *ent;

				if (df->func) {
					r = sophon_hash_lookup(vm, &df->func->var_hash,
								sophon_direct_key,
								sophon_direct_equal,
								(Sophon_Ptr)name,
								&ent);
					if (r == SOPHON_OK) {
						Sophon_Int id = (Sophon_IntPtr)ent->value;

						if (id & SOPHON_VAR_FL_ARG) {
							id = (id & ~SOPHON_VAR_FL_ARG) + df->func->varc;
						}

						df->v[id] = setv;
						return SOPHON_OK;
					}

					if (name == df->func->name) {
						sophon_throw(vm, vm->TypeError,
								"Cannot reset the function name");
						return SOPHON_ERR_THROW;
					}
				}

				if (name == vm->arguments_str) {
					if (strict) {
						sophon_throw(vm, vm->TypeError,
								"Cannot reset \"arguments\" in strict mode");
						return SOPHON_ERR_THROW;
					} else {
						df->argsv = setv;
						return SOPHON_OK;
					}
				}

				if (frame->gc_type == SOPHON_GC_GlobalFrame) {
					Sophon_Module *mod;
					Sophon_U32 fl;
					
					if (sophon_strict(vm) && !(flags & SOPHON_FL_FORCE))
						fl = SOPHON_FL_NONE;
					else
						fl = 0;

					if (df->func) {
						mod = df->func->module->base;
						if (mod != vm->glob_module) {
							r = sophon_value_put(vm, mod->globv,
									SOPHON_VALUE_GC(name),
									setv, fl);
							if (r == SOPHON_NONE) {
								sophon_throw(vm, vm->ReferenceError,
										"Unresolved reference");
								return SOPHON_ERR_THROW;
							}

							return r;
						}
					}

					mod = vm->glob_module;
					r = sophon_value_put(vm, mod->globv,
							SOPHON_VALUE_GC(name),
							setv, fl);
					if (r == SOPHON_NONE) {
						sophon_throw(vm, vm->ReferenceError,
								"Unresolved reference");
						return SOPHON_ERR_THROW;
					}

					return r;
				} else {
					r = sophon_hash_lookup(vm, &df->var_hash,
							sophon_direct_key,
							sophon_direct_equal,
							(Sophon_Ptr)name,
							&ent);
					if (r == SOPHON_OK) {
						ent->value = (Sophon_Ptr)setv;
						return r;
					}
				}
				break;
			}
			case SOPHON_GC_NameFrame: {
				Sophon_NameFrame *nf = (Sophon_NameFrame*)frame;

				if (nf->name == name) {
					nf->v = setv;
					return SOPHON_OK;
				}
				break;
			}
			case SOPHON_GC_WithFrame: {
				Sophon_WithFrame *wf = (Sophon_WithFrame*)frame;

				if ((r = sophon_value_put(vm, wf->withv,
							SOPHON_VALUE_GC(name),
							setv, SOPHON_FL_NONE)) != SOPHON_NONE)
					return r;
				break;
			}
			default:
				SOPHON_ASSERT(0);
		}

		frame = frame->bottom;
	}

	return SOPHON_NONE;
}

Sophon_Value
sophon_stack_get_this (Sophon_VM *vm)
{
	Sophon_Frame *frame;

	SOPHON_ASSERT(vm && vm->stack);

	frame = vm->stack->lex_env;

	do {
		switch (frame->gc_type) {
			case SOPHON_GC_GlobalFrame: {
				return SOPHON_VALUE_UNDEFINED;
			}
			case SOPHON_GC_DeclFrame: {
				Sophon_DeclFrame *df = (Sophon_DeclFrame*)frame;
				return df->thisv;
			}
			case SOPHON_GC_WithFrame: {
				Sophon_WithFrame *wf = (Sophon_WithFrame*)frame;
				return wf->withv;
			}
		}
		frame = frame->bottom;
	} while (frame);

	return SOPHON_VALUE_UNDEFINED;
}

Sophon_Module*
sophon_stack_get_module (Sophon_VM *vm)
{
	Sophon_Frame *frame;

	SOPHON_ASSERT(vm && vm->stack);

	frame = vm->stack->var_env;

	do {
		switch (frame->gc_type) {
			case SOPHON_GC_GlobalFrame:
			case SOPHON_GC_DeclFrame: {
				Sophon_DeclFrame *df = (Sophon_DeclFrame*)frame;
				if (df->func)
					return df->func->module->base;
			}
		}
		frame = frame->bottom;
	} while (frame);

	return vm->glob_module;
}

