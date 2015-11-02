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
#include <sophon_frame.h>
#include <sophon_hash.h>
#include <sophon_function.h>
#include <sophon_closure.h>
#include <sophon_value.h>
#include <sophon_object.h>
#include <sophon_module.h>
#include <sophon_arguments.h>
#include <sophon_debug.h>

Sophon_DeclFrame*
sophon_decl_global_frame_create (Sophon_VM *vm, Sophon_U8 gc_type,
			Sophon_Value callerv, Sophon_Value calleev,
			Sophon_Value thisv, Sophon_Value *argv, Sophon_U8 real_argc)
{
	Sophon_Closure *clos;
	Sophon_Function *func;
	Sophon_DeclFrame *frame;
	Sophon_U32 size, vsize;
	Sophon_U8 varc, argc;
	Sophon_Result r;

	SOPHON_ASSERT(vm);

	size = sizeof(Sophon_DeclFrame);

	if (SOPHON_VALUE_IS_UNDEFINED(calleev)) {
		func  = NULL;
		vsize = 0;
		varc  = 0;
		argc  = 0;
	} else {
		r = sophon_value_to_closure(vm, calleev, &clos);
		SOPHON_ASSERT(r == SOPHON_OK);
		SOPHON_ASSERT(!(clos->gc_flags & SOPHON_GC_FL_BIND));
		r= r;

		func  = clos->c.func.func;
		argc  = SOPHON_MAX(real_argc, func->argc);
		vsize = argc + func->varc;
		varc  = func->varc;
		size += sizeof(Sophon_Value) * (vsize - 1);
	}

	frame = (Sophon_DeclFrame*)sophon_mm_alloc_ensure(vm, size);

	SOPHON_GC_HEADER_INIT((Sophon_GCObject*)frame, gc_type);

	frame->func    = func;
	frame->callerv = callerv;
	frame->calleev = calleev;
	frame->thisv   = thisv;
	frame->argc    = real_argc;
	frame->varc    = varc;
	frame->vbuf_size = vsize;

	sophon_value_set_undefined(vm, &frame->argsv);

	sophon_hash_init(vm, &frame->var_hash);

	if (func && varc) {
		sophon_memset(frame->v, 0xFF, varc * sizeof(Sophon_Value));
	}

	if (real_argc) {
		sophon_memcpy(frame->v + varc, argv,
					real_argc * sizeof(Sophon_Value));
	}

	if (argc - real_argc) {
		sophon_memset(frame->v + varc + real_argc, 0xFF,
					(argc - real_argc) * sizeof(Sophon_Value));
	}

	return frame;
}

void
sophon_decl_frame_destroy (Sophon_VM *vm, Sophon_DeclFrame *frame)
{
	Sophon_U32 size;

	SOPHON_ASSERT(vm && frame);

	size = sizeof(Sophon_DeclFrame);

	if (frame->func) {
		size += sizeof(Sophon_Value) * (frame->vbuf_size - 1);
	}

	sophon_hash_deinit(vm, &frame->var_hash);

	sophon_mm_free(vm, frame, size);
}

Sophon_NameFrame*
sophon_name_frame_create (Sophon_VM *vm, Sophon_String *name,
			Sophon_Value v)
{
	Sophon_NameFrame *frame;

	SOPHON_ASSERT(vm);

	frame = (Sophon_NameFrame*)sophon_mm_alloc_ensure(vm,
				sizeof(Sophon_NameFrame));

	SOPHON_GC_HEADER_INIT((Sophon_GCObject*)frame, SOPHON_GC_NameFrame);

	frame->name = name;
	frame->v    = v;

	return frame;
}

void
sophon_name_frame_destroy (Sophon_VM *vm, Sophon_NameFrame *frame)
{
	SOPHON_ASSERT(vm && frame);

	sophon_mm_free(vm, frame, sizeof(Sophon_NameFrame));
}

Sophon_WithFrame*
sophon_with_frame_create (Sophon_VM *vm, Sophon_Value thisv)
{
	Sophon_WithFrame *frame;

	SOPHON_ASSERT(vm);

	frame = (Sophon_WithFrame*)sophon_mm_alloc_ensure(vm,
				sizeof(Sophon_WithFrame));

	SOPHON_GC_HEADER_INIT((Sophon_GCObject*)frame, SOPHON_GC_WithFrame);

	frame->withv = thisv;

	return frame;
}

void
sophon_with_frame_destroy (Sophon_VM *vm, Sophon_WithFrame *frame)
{
	SOPHON_ASSERT(vm && frame);

	sophon_mm_free(vm, frame, sizeof(Sophon_WithFrame));
}

void
sophon_frame_destroy (Sophon_VM *vm, Sophon_Frame *frame)
{
	SOPHON_ASSERT(vm && frame);

	switch(frame->gc_type) {
		case SOPHON_GC_GlobalFrame:
			sophon_global_frame_destroy(vm, (Sophon_GlobalFrame*)frame);
			break;
		case SOPHON_GC_DeclFrame:
			sophon_decl_frame_destroy(vm, (Sophon_DeclFrame*)frame);
			break;
		case SOPHON_GC_NameFrame:
			sophon_name_frame_destroy(vm, (Sophon_NameFrame*)frame);
			break;
		case SOPHON_GC_WithFrame:
			sophon_with_frame_destroy(vm, (Sophon_WithFrame*)frame);
			break;
	}
}

Sophon_Result
sophon_decl_frame_add_binding (Sophon_VM *vm, Sophon_DeclFrame *frame,
			Sophon_String *name)
{
	Sophon_HashEntry *ent;
	Sophon_Result r;

	SOPHON_ASSERT(vm && frame);

	r = sophon_hash_add(vm, &frame->var_hash,
				sophon_direct_key,
				sophon_direct_equal,
				(Sophon_Ptr)name,
				&ent);
	if (r == SOPHON_OK) {
		ent->value = (Sophon_Ptr)SOPHON_VALUE_UNDEFINED;
	}

	return r;
}

Sophon_Result
sophon_decl_frame_create_args (Sophon_VM *vm, Sophon_DeclFrame *frame)
{
	Sophon_Object *args;

	if (!SOPHON_VALUE_IS_UNDEFINED(frame->argsv))
		return SOPHON_OK;

	args = sophon_arguments_create(vm, frame);
	sophon_value_set_gc(vm, &frame->argsv, args);

	return SOPHON_OK;
}


