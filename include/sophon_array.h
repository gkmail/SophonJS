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

#ifndef _SOPHON_ARRAY_H_
#define _SOPHON_ARRAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sophon_types.h"
#include "sophon_gc.h"

/**\brief Array*/
struct Sophon_Array_s {
	SOPHON_GC_HEADER
	Sophon_U32    cap; /**< Value buffer size*/
	Sophon_U32    len; /**< Real value count in the buffer*/
	Sophon_Value *v;   /**< Value buffer*/
	Sophon_GCObject *ref; /**< Referenced GC object*/
};

#define SOPHON_ARRAY_UNUSED SOPHON_VALUE_CPTR(NULL)

/**\brief Get the array length*/
#define sophon_array_get_length(vm, arr) ((arr)->len)

/**\brief Check if the item is defined in the array*/
#define sophon_array_has_item(vm, arr, id)\
	(((id) >= (arr)->len) ? SOPHON_FALSE :\
			((arr)->v[id] != SOPHON_ARRAY_UNUSED))

/**\brief Delete an item of the array*/
#define sophon_array_delete_item(vm, arr, id)\
	SOPHON_MACRO_BEGIN\
		if ((id) < (arr)->len)\
			(arr)->v[id] = SOPHON_ARRAY_UNUSED;\
	SOPHON_MACRO_END

/**\brief For each item in the array*/
#define sophon_array_for_each(arr, id, val)\
	for ((id) = 0; (id) < (arr)->len; (id)++)\
		if (((val) = arr->v[id]) != SOPHON_ARRAY_UNUSED)

/**\brief For each item in the array reversely*/
#define sophon_array_for_each_r(arr, id, val)\
	for ((id) = (arr)->len - 1; (id) >= 0; (id)--)\
		if (((val) = arr->v[id]) != SOPHON_ARRAY_UNUSED)

/**
 * \brief Create a new array
 * \param[in] vm The current virtual machine
 * \return The new array
 */
extern Sophon_Array* sophon_array_create (Sophon_VM *vm);

/**
 * \brief Release an unused array
 * Array is managed by GC.
 * You should not invoke this function directly.
 * \param[in] vm The current virtual machine
 * \param[in] arr The array to be released
 */
extern void          sophon_array_destroy (Sophon_VM *vm,
						Sophon_Array *arr);

/**
 * \brief Reset the array length
 * \param[in] vm The current virtual machine
 * \param[in] arr The array
 * \param len The array length
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_array_set_length (Sophon_VM *vm,
						Sophon_Array *arr, Sophon_U32 len);

/**
 * \brief Get an item in the array
 * \param[in] vm The current virtual machine
 * \param[in] arr The array
 * \param off The item's index
 * \param[out] v Return the item value
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_array_get_item (Sophon_VM *vm,
						Sophon_Array *arr, Sophon_U32 off,
						Sophon_Value *v);

/**
 * \brief Set an item in the array
 * \param[in] vm The current virtual machine
 * \param[in] arr The array
 * \param off The item's index
 * \param v Return item's new value
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_array_set_item (Sophon_VM *vm,
						Sophon_Array *arr, Sophon_U32 off,
						Sophon_Value v);

/**
 * \brief Sort the array item by the compare function
 * \param[in] vm The current virtual machine
 * \param[in] arr The array
 * \param func Compare function
 * \param[in] User defined compare function
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_array_sort (Sophon_VM *vm,
						Sophon_Array *arr,
						Sophon_CmpFunc func,
						Sophon_Ptr data);


#ifdef __cplusplus
}
#endif

#endif

