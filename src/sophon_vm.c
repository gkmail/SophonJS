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
#include <sophon_value.h>
#include <sophon_vm.h>
#include <sophon_debug.h>

static void
vm_init (Sophon_VM *vm)
{
	/*Initialize exception and label*/
	sophon_value_set_undefined(vm, &vm->excepv);
	sophon_value_set_undefined(vm, &vm->labelv);

	/*Get error objects*/
	sophon_value_set_undefined(vm, &vm->type_errv);
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
	sophon_double_pool_init(vm);
	sophon_string_intern_hash_init(vm);
	sophon_stack_init(vm);
	vm_init(vm);

	SOPHON_INFO(("create new VM"));
	return vm;
}

void
sophon_vm_destroy(Sophon_VM *vm)
{
	Sophon_Ptr ptr;

	SOPHON_ASSERT(vm);

	sophon_stack_deinit(vm);
	sophon_double_pool_deinit(vm);
	sophon_gc_deinit(vm);
	sophon_string_intern_hash_deinit(vm);
	sophon_mm_deinit(vm);

	ptr = sophon_realloc(vm, 0);
	ptr = ptr;

	SOPHON_INFO(("release VM"));
}

