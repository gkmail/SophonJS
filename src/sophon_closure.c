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
#include <sophon_object.h>
#include <sophon_closure.h>
#include <sophon_stack.h>
#include <sophon_frame.h>
#include <sophon_gc.h>
#include <sophon_function.h>
#include <sophon_array.h>
#include <sophon_debug.h>

Sophon_Closure*
sophon_closure_create (Sophon_VM *vm, Sophon_Function *func)
{
	Sophon_Closure *clos;
	Sophon_Frame *frame;

	SOPHON_ASSERT(vm && func);

	SOPHON_PRIM_OBJ_ALLOC(vm, clos, Closure);

	clos->c.func.func = func;

	if (func->flags & SOPHON_FUNC_FL_NATIVE)
		clos->gc_flags |= SOPHON_GC_FL_NATIVE;

	if (func->flags & SOPHON_FUNC_FL_GLOBAL) {
		clos->c.func.var_env = NULL;
		clos->c.func.lex_env = NULL;
	} else {
		if (vm->stack) {
			clos->c.func.var_env = vm->stack->var_env;
			clos->c.func.lex_env = vm->stack->lex_env;
		} else {
			clos->c.func.var_env = NULL;
			clos->c.func.lex_env = NULL;
		}

		/*Add the frames referened to GC*/
		frame = clos->c.func.lex_env;
		while (frame && !(frame->gc_flags & SOPHON_GC_FL_MANAGED)) {
			sophon_gc_add(vm, (Sophon_GCObject*)frame);
			frame = frame->bottom;
		}
	}

	sophon_gc_add(vm, (Sophon_GCObject*)clos);

	return clos;
}

Sophon_Closure*
sophon_closure_bind (Sophon_VM *vm, Sophon_Value funcv, Sophon_Value thisv,
		Sophon_Array *arr)
{
	Sophon_Closure *clos;

	SOPHON_ASSERT(vm);

	SOPHON_PRIM_OBJ_ALLOC(vm, clos, Closure);

	clos->gc_flags |= SOPHON_GC_FL_BIND;

	clos->c.bind.funcv = funcv;
	clos->c.bind.thisv = thisv;
	clos->c.bind.args  = arr;

	sophon_gc_add(vm, (Sophon_GCObject*)clos);

	return clos;
}

void
sophon_closure_destroy (Sophon_VM *vm, Sophon_Closure *clos)
{
	SOPHON_ASSERT(vm && clos);

	SOPHON_PRIM_OBJ_FREE(vm, clos, Closure);
}


