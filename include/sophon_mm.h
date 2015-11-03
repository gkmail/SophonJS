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

#ifndef _SOPHON_MM_H_
#define _SOPHON_MM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sophon_types.h"

#define sophon_mm_init(vm)\
	SOPHON_MACRO_BEGIN\
		(vm)->mm_curr_used = 0;\
		(vm)->mm_max_used  = 0;\
	SOPHON_MACRO_END

#define sophon_mm_deinit(vm)

#ifdef SOPHON_MM_DEBUG
extern void       sophon_mm_dump_unfreed (Sophon_VM *vm);

extern void       sophon_mm_check_ptr (Sophon_VM *vm, Sophon_Ptr ptr,
						Sophon_U32 size);

extern void       sophon_mm_check_all (Sophon_VM *vm);

extern Sophon_Ptr sophon_mm_realloc_real (Sophon_VM *vm, Sophon_Ptr old_ptr,
						Sophon_U32 old_size, Sophon_U32 new_size,
						const char *file, int line);

extern Sophon_Ptr sophon_mm_realloc_ensure_real (Sophon_VM *vm,
						Sophon_Ptr old_ptr, Sophon_U32 old_size,
						Sophon_U32 new_size, const char *file, int line);

#define sophon_mm_realloc(vm, old_ptr, old_size, new_size) \
		sophon_mm_realloc_real(vm, old_ptr, old_size, new_size, \
					__FILE__, __LINE__)

#define sophon_mm_realloc_ensure(vm, old_ptr, old_size, new_size) \
		sophon_mm_realloc_ensure_real(vm, old_ptr, old_size, new_size, \
					__FILE__, __LINE__)

#else /*!defined(SOPHON_MM_DEBUG)*/

/**
 * \brief Change the memory block size
 * \param[in] vm The current virtual machine
 * \param[in] old_ptr The old memory block pointer
 * If old_ptr == NULL, allocate a new memory block.
 * \param old_size The old memory size in bytes
 * \param new_size The new memory size in bytes
 * If new_size == 0, free the memory block.
 * \return The new memory block's pointer
 * \retval NULL On error
 */
extern Sophon_Ptr sophon_mm_realloc (Sophon_VM *vm, Sophon_Ptr old_ptr,
						Sophon_U32 old_size, Sophon_U32 new_size);

/**
 * \brief Change the memory block size and check the result
 * If not enough memory left, invoke sophon_fatal to stop the program.
 * \param[in] vm The current virtual machine
 * \param[in] old_ptr The old memory block pointer
 * If old_ptr == NULL, allocate a new memory block.
 * \param old_size The old memory size in bytes
 * \param new_size The new memory size in bytes
 * If new_size == 0, free the memory block.
 * \return The new memory block's pointer
 * \retval NULL On error
 */
extern Sophon_Ptr sophon_mm_realloc_ensure (Sophon_VM *vm, Sophon_Ptr old_ptr,
						Sophon_U32 old_size, Sophon_U32 new_size);

#endif /*SOPHON_MM_DEBUG*/

/**
 * \brief Allocate a new memory block
 */
#define sophon_mm_alloc(vm, size)     sophon_mm_realloc(vm, NULL, 0, size)

/**
 * \brief Free a unused memory block
 */
#define sophon_mm_free(vm, ptr, size) sophon_mm_realloc(vm, ptr, size, 0)

/**
 * \brief Allocate a new memory block and check the result
 * If not enough memory left, invoke sophon_fatal to stop the program.
 */
#define sophon_mm_alloc_ensure(vm, size) \
			sophon_mm_realloc_ensure(vm, NULL, 0, size)

#ifdef __cplusplus
}
#endif

#endif

