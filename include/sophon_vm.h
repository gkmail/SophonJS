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

#ifndef _SOPHON_VM_H_
#define _SOPHON_VM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sophon_types.h"
#include "sophon_value.h"
#include "sophon_hash.h"
#include "sophon_double_pool.h"
#include "sophon_gc.h"

/**\brief Virtual machine*/
struct Sophon_VM_s {
	/*Memory manager*/
	Sophon_U32 mm_curr_used;
	Sophon_U32 mm_max_used;

	/*Double pool*/
	Sophon_Hash         dp_hash;
	Sophon_DoublePool  *dp_free_list;
	Sophon_DoublePool  *dp_full_list;

	/*Garbage collector*/
	Sophon_Bool       gc_running;
	Sophon_U32        gc_mem_size;
	Sophon_GCObject  *gc_used_list;
	Sophon_GCObject  *gc_unmanaged_list;
	Sophon_Hash       gc_root_hash;
	Sophon_GCObjectBuf  gc_obuf1;
	Sophon_GCObjectBuf  gc_obuf2;
	Sophon_GCObjectBuf  gc_nb_obuf;
	Sophon_GCObjectBuf *gc_curr_obuf;

	/*String intern hash table*/
	Sophon_Hash       str_intern_hash;

	/*Stack*/
	Sophon_Stack     *stack;

	/*Exception*/
	Sophon_Value      excepv;

	/*Break/Continue label value*/
	Sophon_Value      labelv;

	/*Errors*/
	Sophon_Value      type_errv;
};

/**\brief Throw an exception*/
#define sophon_throw(vm, v)\
	SOPHON_MACRO_BEGIN\
		if (!SOPHON_VALUE_IS_UNDEFINED((vm)->excepv))\
			(vm)->excepv = (v);\
	SOPHON_MACRO_END

/**
 * \brief Create a new virtual machine
 * \return The new virtual machine
 * \retval NULL On error
 */
extern Sophon_VM* sophon_vm_create (void);

/**
 * \brief Release an unused virtual machine
 * \param[in] vm The virtual machine to be freed
 */
extern void       sophon_vm_destroy (Sophon_VM *vm);

#ifdef __cplusplus
}
#endif

#endif

