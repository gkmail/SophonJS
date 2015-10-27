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
#include <sophon_module.h>
#include <sophon_function.h>
#include <sophon_debug.h>

Sophon_Module*
sophon_module_create (Sophon_VM *vm)
{
	Sophon_Module *mod;

	SOPHON_ASSERT(vm);

	mod = (Sophon_Module*)sophon_mm_alloc_ensure(vm, sizeof(Sophon_Module));

	SOPHON_GC_HEADER_INIT(mod, SOPHON_GC_Module);

	mod->name   = NULL;
	mod->consts = NULL;
	mod->funcs  = NULL;
	mod->const_count = 0;
	mod->const_cap   = 0;
	mod->func_count  = 0;
	mod->func_cap    = 0;

	sophon_hash_init(vm, &mod->const_hash);

	sophon_value_set_undefined(vm, &mod->globv);

	sophon_gc_add(vm, (Sophon_GCObject*)mod);

	return mod;
}

void
sophon_module_destroy (Sophon_VM *vm, Sophon_Module *mod)
{
	SOPHON_ASSERT(vm && mod);

	if (mod->consts) {
		sophon_mm_free(vm, mod->consts,
					sizeof(Sophon_Value) * mod->const_cap);
	}

	if (mod->funcs) {
		Sophon_Function **f, **flast;
		Sophon_Function *func;

		f = mod->funcs;
		flast = f + mod->func_count;
		while (f < flast) {
			func = *f;

			if (!(func->flags & SOPHON_FUNC_FL_NATIVE) && func->f.ibuf) {
				sophon_mm_free(vm, func->f.ibuf, func->ibuf_size);
			}

#ifdef SOPHON_LINE_INFO
			if (func->lbuf) {
				sophon_mm_free(vm, func->lbuf,
							sizeof(Sophon_LineInfo) * func->lbuf_size);
			}
#endif

			sophon_hash_deinit(vm, &func->var_hash);
			sophon_mm_free(vm, func, sizeof(Sophon_Function));

			f++;
		}

		sophon_mm_free(vm, mod->funcs,
					sizeof(Sophon_Function*) * mod->func_cap);
	}

	sophon_hash_deinit(vm, &mod->const_hash);

	sophon_mm_free(vm, mod, sizeof(Sophon_Module));
}

Sophon_Int
sophon_module_add_func (Sophon_VM *vm, Sophon_Module *mod,
			Sophon_String *name, Sophon_U32 flags)
{
	Sophon_Function *func;
	Sophon_Int id;

	SOPHON_ASSERT(vm && mod);

	func = (Sophon_Function*)sophon_mm_alloc_ensure(vm,
				sizeof(Sophon_Function));

	func->container = NULL;
	func->module = mod;
	func->name   = name;
	func->flags  = flags;
	func->argc   = 0;
	func->varc   = 0;
	func->stack_size = 0;
	func->ibuf_size  = 0;
	func->f.ibuf = NULL;

#ifdef SOPHON_LINE_INFO
	func->lbuf_size = 0;
	func->lbuf   = NULL;
#endif

	sophon_hash_init(vm, &func->var_hash);

	if (mod->func_cap == mod->func_count) {
		Sophon_Function **buf;
		Sophon_U32 size;

		size = SOPHON_MAX(mod->func_cap * 2, 4);
		buf  = (Sophon_Function**)sophon_mm_realloc_ensure(vm, mod->funcs,
					sizeof(Sophon_Function*) * mod->func_cap,
					sizeof(Sophon_Function*) * size);

		mod->funcs = buf;
		mod->func_cap = size;
	}

	id = mod->func_count;
	func->id = id;
	mod->funcs[mod->func_count++] = func;

	return id;
}

Sophon_Int
sophon_module_add_const (Sophon_VM *vm, Sophon_Module *mod, Sophon_Value v)
{
	Sophon_HashEntry *ent;
	Sophon_Result r;
	Sophon_Int id;

	SOPHON_ASSERT(vm && mod);

	r = sophon_hash_add(vm, &mod->const_hash,
				sophon_value_key,
				sophon_value_equal,
				(Sophon_Ptr)v, &ent);
	if (r == SOPHON_OK) {
		id = mod->const_count;
		ent->value = (Sophon_Ptr)(Sophon_IntPtr)id;
	} else if (r == SOPHON_NONE) {
		id = (Sophon_IntPtr)ent->value;
	} else {
		id = -1;
	}

	if (id >= mod->const_count) {
		if (mod->const_count >= mod->const_cap) {
			Sophon_Value *buf;
			Sophon_U32 size = SOPHON_MAX(16, mod->const_cap *2);

			buf = sophon_mm_realloc_ensure(vm, mod->consts,
						sizeof(Sophon_Value) * mod->const_cap,
						sizeof(Sophon_Value) * size);

			mod->consts = buf;
			mod->const_cap = size;
		}

		mod->consts[id] = v;
		mod->const_count = id + 1;
	}

	return id;
}

Sophon_Result
sophon_module_call (Sophon_VM *vm, Sophon_Module *mod, Sophon_Value *argv,
			Sophon_Int argc, Sophon_Value *retv)
{
	Sophon_Value v;

	SOPHON_ASSERT(vm && mod);

	if (!retv)
		retv = &v;

	return sophon_value_call(vm, mod->globv, mod->globv, argv, argc, retv, 0);
}

Sophon_Object*
sophon_module_get_global (Sophon_VM *vm, Sophon_Module *mod)
{
	Sophon_Object *obj;
	Sophon_Result r;

	SOPHON_ASSERT(vm);

	if (!mod)
		mod = vm->glob_module;

	if ((r = sophon_value_to_object(vm, mod->globv, &obj)) != SOPHON_OK)
		return NULL;
	
	return obj;
}

