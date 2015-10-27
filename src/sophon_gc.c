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
#include <sophon_gc.h>
#include <sophon_string.h>
#include <sophon_object.h>
#include <sophon_frame.h>
#include <sophon_function.h>
#include <sophon_stack.h>
#include <sophon_closure.h>
#include <sophon_module.h>
#include <sophon_array.h>
#include <sophon_arguments.h>
#include <sophon_regexp.h>
#include <sophon_debug.h>

#include "sophon_parser_internal.h"

#ifndef SOPHON_GC_START_SIZE
	#define SOPHON_GC_START_SIZE (32*1024)
#endif

/*Initialize the GC object buffer*/
static void
gc_obuf_init (Sophon_VM *vm, Sophon_GCObjectBuf *obuf)
{
	obuf->count = 0;
	obuf->cap   = 0;
	obuf->objs  = NULL;
}

/*Release the GC object buffer*/
static void
gc_obuf_deinit (Sophon_VM *vm, Sophon_GCObjectBuf *obuf)
{
	if (obuf->objs) {
		sophon_mm_free(vm, obuf->objs, sizeof(Sophon_GCObject*) * obuf->cap);
	}
}

/*Add a GC object to the buffer*/
static void
gc_obuf_add (Sophon_VM *vm, Sophon_GCObjectBuf *obuf, Sophon_GCObject *obj)
{
	if (obuf->count == obuf->cap) {
		Sophon_GCObject **nbuf;
		Sophon_U32 size;

		size = SOPHON_MAX(32, obuf->cap * 2);
		nbuf = sophon_mm_realloc_ensure(vm, obuf->objs,
				obuf->cap * sizeof(Sophon_GCObject*),
				size * sizeof(Sophon_GCObject*));

		obuf->objs = nbuf;
		obuf->cap  = size;
	}

	obuf->objs[obuf->count++] = obj;
}

/*Release a GC object*/
static void
gc_final (Sophon_VM *vm, Sophon_GCObject *obj)
{
	switch (obj->gc_type) {
		case SOPHON_GC_String:
			sophon_string_destroy(vm, (Sophon_String*)obj);
			break;
		case SOPHON_GC_Object:
			sophon_object_destroy(vm, (Sophon_Object*)obj);
			break;
		case SOPHON_GC_GlobalFrame:
			sophon_global_frame_destroy(vm, (Sophon_GlobalFrame*)obj);
			break;
		case SOPHON_GC_DeclFrame:
			sophon_decl_frame_destroy(vm, (Sophon_DeclFrame*)obj);
			break;
		case SOPHON_GC_CatchFrame:
			sophon_catch_frame_destroy(vm, (Sophon_CatchFrame*)obj);
			break;
		case SOPHON_GC_WithFrame:
			sophon_with_frame_destroy(vm, (Sophon_WithFrame*)obj);
			break;
		case SOPHON_GC_Closure:
			sophon_closure_destroy(vm, (Sophon_Closure*)obj);
			break;
		case SOPHON_GC_Module:
			sophon_module_destroy(vm, (Sophon_Module*)obj);
			break;
		case SOPHON_GC_Array:
			sophon_array_destroy(vm, (Sophon_Array*)obj);
			break;
		case SOPHON_GC_RegExp:
			sophon_regexp_destroy(vm, (Sophon_RegExp*)obj);
			break;
	}
}

/*Mark a GC object as used*/
static void
gc_mark (Sophon_VM *vm, Sophon_GCObject *obj)
{
	if (obj->gc_flags & SOPHON_GC_FL_MARKED)
		return;

	if (!(obj->gc_flags & SOPHON_GC_FL_MANAGED)) {
		obj->gc_next = vm->gc_unmanaged_list;
		vm->gc_unmanaged_list = obj;
	}

	gc_obuf_add(vm, vm->gc_curr_obuf, obj);

	obj->gc_flags |= SOPHON_GC_FL_MARKED;
}

#include "sophon_gc_object.c"
#include "sophon_gc_function.c"
#include "sophon_gc_frame.c"
#include "sophon_gc_closure.c"
#include "sophon_gc_module.c"
#include "sophon_gc_parser.c"
#include "sophon_gc_array.c"
#include "sophon_gc_vm.c"
#include "sophon_gc_regexp.c"

/*Scan a GC object*/
static void
gc_scan (Sophon_VM *vm, Sophon_GCObject *obj)
{
	switch (obj->gc_type) {
		case SOPHON_GC_Object:
			gc_scan_object(vm, (Sophon_Object*)obj);
			break;
		case SOPHON_GC_GlobalFrame:
			gc_scan_global_frame(vm, (Sophon_GlobalFrame*)obj);
			break;
		case SOPHON_GC_DeclFrame:
			gc_scan_decl_frame(vm, (Sophon_DeclFrame*)obj);
			break;
		case SOPHON_GC_CatchFrame:
			gc_scan_catch_frame(vm, (Sophon_CatchFrame*)obj);
			break;
		case SOPHON_GC_WithFrame:
			gc_scan_with_frame(vm, (Sophon_WithFrame*)obj);
			break;
		case SOPHON_GC_Closure:
			gc_scan_closure(vm, (Sophon_Closure*)obj);
			break;
		case SOPHON_GC_Module:
			gc_scan_module(vm, (Sophon_Module*)obj);
			break;
		case SOPHON_GC_Array:
			gc_scan_array(vm, (Sophon_Array*)obj);
			break;
		case SOPHON_GC_RegExp:
			gc_scan_regexp(vm, (Sophon_RegExp*)obj);
			break;
	}
}

static void
gc_mark_root_entry (Sophon_VM *vm, Sophon_HashEntry *ent, Sophon_Ptr data)
{
	gc_mark(vm, (Sophon_GCObject*)ent->value);
}

static void
gc_scan_root (Sophon_VM *vm)
{
	Sophon_GCObject **obj, **olast;

	/*Mark root objects*/
	sophon_hash_for_each(vm, &vm->gc_root_hash, gc_mark_root_entry, NULL);

	/*Mark new borned objects*/
	obj   = vm->gc_nb_obuf.objs;
	olast = obj + vm->gc_nb_obuf.count;

	SOPHON_INFO(("mark new borned objects, count %d, size %d",
				vm->gc_nb_obuf.count,
				vm->gc_nb_obuf.cap));

	while (obj < olast) {
		sophon_value_mark(vm, (Sophon_Value)*obj);
		obj++;
	}

	/*Mark VM*/
	gc_mark_vm(vm);
}

void
sophon_gc_init (Sophon_VM *vm)
{
	SOPHON_ASSERT(vm);

	vm->gc_running  = SOPHON_FALSE;
	vm->gc_mem_size = 0;

	vm->gc_used_list = NULL;
	vm->gc_unmanaged_list = NULL;
	sophon_hash_init(vm, &vm->gc_root_hash);

	gc_obuf_init(vm, &vm->gc_obuf1);
	gc_obuf_init(vm, &vm->gc_obuf2);
	gc_obuf_init(vm, &vm->gc_nb_obuf);
}

void
sophon_gc_deinit (Sophon_VM *vm)
{
	Sophon_GCObject *obj, *onext;

	for (obj = vm->gc_used_list; obj; obj = onext) {
		onext = obj->gc_next;
		gc_final(vm, obj);
	}

	sophon_hash_deinit(vm, &vm->gc_root_hash);

	gc_obuf_deinit(vm, &vm->gc_obuf1);
	gc_obuf_deinit(vm, &vm->gc_obuf2);
	gc_obuf_deinit(vm, &vm->gc_nb_obuf);
}

void
sophon_gc_add (Sophon_VM *vm, Sophon_GCObject *obj)
{
	SOPHON_ASSERT(vm && obj);

	if (obj->gc_flags & SOPHON_GC_FL_MANAGED)
		return;

	gc_obuf_add(vm, &vm->gc_nb_obuf, obj);

	obj->gc_next = vm->gc_used_list;
	vm->gc_used_list = obj;

	obj->gc_flags |= SOPHON_GC_FL_MANAGED;

	sophon_gc_check(vm);
}

void
sophon_gc_add_root (Sophon_VM *vm, Sophon_GCObject *obj)
{
	Sophon_HashEntry *ent;
	Sophon_Result r;

	SOPHON_ASSERT(vm && obj);

	r = sophon_hash_add(vm, &vm->gc_root_hash,
				sophon_direct_key,
				sophon_direct_equal,
				(Sophon_Ptr)obj,
				&ent);

	if (r == SOPHON_OK) {
		ent->value = (Sophon_Ptr)1;
	} else {
		Sophon_UIntPtr v = (Sophon_UIntPtr)ent->value;
		ent->value = (Sophon_Ptr)(v + 1);
	}

	if (!(obj->gc_flags & SOPHON_GC_FL_MANAGED)) {
		sophon_gc_add(vm, obj);
	}
}

void
sophon_gc_remove_root (Sophon_VM *vm, Sophon_GCObject *obj)
{
	Sophon_HashEntry *ent;
	Sophon_Result r;
	Sophon_UIntPtr v;

	SOPHON_ASSERT(vm && obj);

	r = sophon_hash_lookup(vm, &vm->gc_root_hash,
				sophon_direct_key,
				sophon_direct_equal,
				(Sophon_Ptr)obj,
				&ent);
	if (r <= 0)
		return;

	v = (Sophon_UIntPtr)ent->value;
	v--;

	if (v == 0) {
		sophon_hash_remove(vm, &vm->gc_root_hash,
				sophon_direct_key,
				sophon_direct_equal,
				(Sophon_Ptr)obj);
	} else {
		ent->value = (Sophon_Ptr)v;
	}
}

void
sophon_gc_run (Sophon_VM *vm)
{
	Sophon_GCObject *obj, *onext, **pp;

	SOPHON_ASSERT(vm);

	if (vm->gc_running)
		return;

	vm->gc_running = SOPHON_TRUE;

	vm->gc_unmanaged_list = NULL;

	SOPHON_INFO(("gc start, total memory size %d", vm->mm_curr_used));
	vm->gc_obuf1.count = 0;
	vm->gc_curr_obuf = &vm->gc_obuf1;

	SOPHON_INFO(("gc scan root objects"));
	gc_scan_root(vm);

	SOPHON_INFO(("gc scan objects"));
	while (vm->gc_curr_obuf->count) {
		Sophon_GCObject **pobj  = vm->gc_curr_obuf->objs;
		Sophon_GCObject **plast = pobj + vm->gc_curr_obuf->count;
		
		if (vm->gc_curr_obuf == &vm->gc_obuf1) {
			vm->gc_curr_obuf = &vm->gc_obuf2;
		} else {
			vm->gc_curr_obuf = &vm->gc_obuf1;
		}

		vm->gc_curr_obuf->count = 0;

		while (pobj < plast) {
			gc_scan(vm, *pobj);

			pobj++;
		}
	}

	SOPHON_INFO(("gc sweep"));

	for (obj = vm->gc_unmanaged_list; obj; obj = obj->gc_next) {
		obj->gc_flags &= ~SOPHON_GC_FL_MARKED;
	}

	pp = &vm->gc_used_list;
	for (obj = vm->gc_used_list; obj; obj = onext) {
		onext = obj->gc_next;

		if (obj->gc_flags & SOPHON_GC_FL_MARKED) {
			obj->gc_flags &= ~SOPHON_GC_FL_MARKED;
			pp = &obj->gc_next;
		} else {
			*pp = onext;
			gc_final(vm, obj);
		}
	}

	/*Sweep number pool*/
	sophon_number_pool_sweep(vm);

	vm->gc_mem_size = vm->mm_curr_used;

	SOPHON_INFO(("gc end, memory size after GC %d", vm->mm_curr_used));

	vm->gc_running = SOPHON_FALSE;
}

void
sophon_gc_check (Sophon_VM *vm)
{
	if ((vm->mm_curr_used > SOPHON_GC_START_SIZE) &&
				(vm->gc_mem_size * 3 < vm->mm_curr_used * 2)) {
		sophon_gc_run(vm);
	}
}

void
sophon_gc_add_nb (Sophon_VM *vm, Sophon_Value v)
{
	gc_obuf_add(vm, &vm->gc_nb_obuf, (Sophon_GCObject*)v);
}

