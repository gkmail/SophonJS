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
#include <sophon_array.h>
#include <sophon_object.h>
#include <sophon_gc.h>
#include <sophon_vm.h>
#include <sophon_debug.h>

#ifndef SOPHON_ARRAY_EXPAND_SIZE
	#define SOPHON_ARRAY_EXPAND_SIZE 16
#endif

static Sophon_Result
array_resize (Sophon_VM *vm, Sophon_Array *arr, Sophon_U32 len)
{
	Sophon_Value *v, *vend;

	if (arr->cap < len) {
		Sophon_Value *buf;
		Sophon_U32 size;

		if (arr->ref) {
			sophon_throw(vm, vm->TypeError, "Cannot reset an arguments object");
			return SOPHON_ERR_THROW;
		}
		
		size = SOPHON_MAX(arr->cap + SOPHON_ARRAY_EXPAND_SIZE, len);

		buf = (Sophon_Value*)sophon_mm_realloc(vm, arr->v,
					arr->len * sizeof(Sophon_Value),
					size * sizeof(Sophon_Value));
		if (!buf) {
			sophon_throw(vm, vm->RangeError, "Array size is too big");
			return SOPHON_ERR_NOMEM;
		}

		arr->v   = buf;
		arr->cap = size;
	}

	/*Set the unsed value*/
	v    = arr->v + arr->len;
	vend = arr->v + len;
	while (v < vend) {
		*v = SOPHON_ARRAY_UNUSED;
		v++;
	}

	arr->len = len;

	return SOPHON_OK;
}

Sophon_Array*
sophon_array_create (Sophon_VM *vm)
{
	Sophon_Array *arr;

	SOPHON_ASSERT(vm);

	SOPHON_PRIM_OBJ_ALLOC(vm, arr, Array);

	arr->v   = NULL;
	arr->len = 0;
	arr->cap = 0;
	arr->ref = NULL;

	sophon_gc_add(vm, (Sophon_GCObject*)arr);

	return arr;
}

void
sophon_array_destroy (Sophon_VM *vm, Sophon_Array *arr)
{
	SOPHON_ASSERT(vm && arr);

	if (arr->v && !arr->ref)
		sophon_mm_free(vm, arr->v, sizeof(Sophon_Value) * arr->cap);

	SOPHON_PRIM_OBJ_FREE(vm, arr, Array);
}

Sophon_Result
sophon_array_set_length (Sophon_VM *vm, Sophon_Array *arr, Sophon_U32 len)
{
	Sophon_Result r;

	SOPHON_ASSERT(vm && arr);

	if (len < arr->len) {
		arr->len = len;
	} else if (len > arr->len) {
		if ((r = array_resize(vm, arr, len)) != SOPHON_OK)
			return r;
	}

	return SOPHON_OK;
}

Sophon_Result
sophon_array_get_item (Sophon_VM *vm, Sophon_Array *arr, Sophon_U32 off,
			Sophon_Value *v)
{
	SOPHON_ASSERT(vm && arr);

	if (off >= arr->len)
		sophon_value_set_undefined(vm, v);
	else if (arr->v[off] == SOPHON_ARRAY_UNUSED)
		*v = SOPHON_VALUE_UNDEFINED;
	else
		*v = arr->v[off];

	return SOPHON_OK;
}

Sophon_Result
sophon_array_set_item (Sophon_VM *vm, Sophon_Array *arr, Sophon_U32 off,
			Sophon_Value v)
{
	Sophon_Result r;

	SOPHON_ASSERT(vm && arr);

	if (off >= arr->len) {
		if ((r = array_resize(vm, arr, off + 1)) != SOPHON_OK)
			return r;
	}

	arr->v[off] = v;
	
	return SOPHON_OK;
}

Sophon_Result
sophon_array_sort (Sophon_VM *vm, Sophon_Array *arr,
		Sophon_CmpFunc cmp, Sophon_Ptr arg)
{
	SOPHON_ASSERT(vm && arr && cmp);

	return sophon_qsort(vm, arr->v, arr->len, cmp, arg);
}
