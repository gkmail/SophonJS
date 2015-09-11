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
#include <sophon_value.h>
#include <sophon_closure.h>
#include <sophon_array.h>
#include <sophon_object.h>
#include <sophon_vm.h>
#include <sophon_debug.h>

Sophon_Result
sophon_value_call (Sophon_VM *vm, Sophon_Value callv,
			Sophon_Value thisv, Sophon_Value *argv, Sophon_Int argc,
			Sophon_Value *retv)
{
	Sophon_Closure *clos;
	Sophon_Result r;

	if ((r = sophon_value_get_closure(vm, callv, &clos)) != SOPHON_OK)
		return r;

	return SOPHON_OK;
}

Sophon_Result
sophon_value_get_object (Sophon_VM *vm, Sophon_Value v, Sophon_Object **obj)
{
	SOPHON_ASSERT(vm && obj);

	if (SOPHON_VALUE_IS_GC(v)) {
		Sophon_GCObject *gc_obj = SOPHON_VALUE_GET_GC(v);

		switch (gc_obj->gc_type) {
			case SOPHON_GC_OBJECT:
				*obj = (Sophon_Object*)gc_obj;
				return SOPHON_OK;
			case SOPHON_GC_ARRAY:
				/*TBD*/
				break;
			case SOPHON_GC_CLOSURE:
				/*TBD*/
				break;
		}
	}

	SOPHON_ERROR(("the value is not an object"));
	return SOPHON_ERR_TYPE;
}

Sophon_Result
sophon_value_get_closure (Sophon_VM *vm, Sophon_Value v, Sophon_Closure **clos)
{
	Sophon_Object *obj;

	SOPHON_ASSERT(vm && clos);

	if (SOPHON_VALUE_IS_GC(v)) {
		Sophon_GCObject *gc_obj = SOPHON_VALUE_GET_GC(v);

		switch (gc_obj->gc_type) {
			case SOPHON_GC_CLOSURE:
				*clos = (Sophon_Closure*)gc_obj;
				return SOPHON_OK;
			case SOPHON_GC_OBJECT:
				obj = (Sophon_Object*)gc_obj;
				v = obj->primv;
				if (SOPHON_VALUE_IS_GC(v)) {
					gc_obj = SOPHON_VALUE_GET_GC(v);
					if (gc_obj->gc_type == SOPHON_GC_CLOSURE) {
						*clos = (Sophon_Closure*)gc_obj;
						return SOPHON_OK;
					}
				}
		}
	}

	SOPHON_ERROR(("the value is not a closure"));
	return SOPHON_ERR_TYPE;
}

Sophon_Result
sophon_value_get_array (Sophon_VM *vm, Sophon_Value v, Sophon_Array **arr)
{
	Sophon_Object *obj;

	SOPHON_ASSERT(vm && clos);

	if (SOPHON_VALUE_IS_GC(v)) {
		Sophon_GCObject *gc_obj = SOPHON_VALUE_GET_GC(v);

		switch (gc_obj->gc_type) {
			case SOPHON_GC_ARRAY:
				*arr = (Sophon_Array*)gc_obj;
				return SOPHON_OK;
			case SOPHON_GC_OBJECT:
				obj = (Sophon_Object*)gc_obj;
				v = obj->primv;
				if (SOPHON_VALUE_IS_GC(v)) {
					gc_obj = SOPHON_VALUE_GET_GC(v);
					if (gc_obj->gc_type == SOPHON_GC_CLOSURE) {
						*arr = (Sophon_Array*)gc_obj;
						return SOPHON_OK;
					}
				}
		}
	}

	SOPHON_ERROR(("the value is not an array"));
	return SOPHON_ERR_TYPE;
}

