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
#include <sophon_arguments.h>
#include <sophon_function.h>
#include <sophon_array.h>
#include <sophon_debug.h>

Sophon_Object*
sophon_arguments_create (Sophon_VM *vm, Sophon_DeclFrame *frame)
{
	Sophon_Array *arr;
	Sophon_Object *obj;
	Sophon_Value argsv;
	Sophon_U32 cap, len;

	arr = sophon_array_create(vm);

	cap = frame->argc;
	if (frame->func)
		cap = SOPHON_MAX(cap, frame->func->argc);

	len = frame->argc;

	if (sophon_strict(vm)) {
		if (frame->argc) {
			sophon_array_set_length(vm, arr, len);
			sophon_memcpy(arr->v, frame->v + frame->varc,
						len * sizeof (Sophon_Value));
		}
	} else {
		arr->v   = frame->v + frame->varc;
		arr->len = len;
		arr->cap = cap;
		arr->ref = (Sophon_GCObject*)frame;
	}

	sophon_value_set_array(vm, &argsv, arr);
	sophon_value_to_object(vm, argsv, &obj);

	if (!sophon_strict(vm)) {
		sophon_value_define_prop(vm, argsv, SOPHON_VALUE_GC(vm->caller_str),
					frame->callerv, SOPHON_VALUE_UNDEFINED,
					0, SOPHON_FL_HAVE_VALUE);
		sophon_value_define_prop(vm, argsv, SOPHON_VALUE_GC(vm->callee_str),
					frame->calleev, SOPHON_VALUE_UNDEFINED,
					0, SOPHON_FL_HAVE_VALUE);
	}

	obj->protov = vm->Arguments_protov;

	return obj;
}

