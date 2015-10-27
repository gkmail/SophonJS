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

#ifndef _SOPHON_NUMBER_POOL_H_
#define _SOPHON_NUMBER_POOL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sophon_value.h"

/**\brief Number entry*/
typedef struct Sophon_NumberEntry_s Sophon_NumberEntry;

/**\brief Number entry*/
struct Sophon_NumberEntry_s {
	Sophon_NumberEntry *next; /**< The next entry in the list*/
};

/**\brief Number pool*/
typedef struct Sophon_NumberPool_s Sophon_NumberPool;

/**\brief Number pool*/
struct Sophon_NumberPool_s {
	Sophon_NumberPool  *next;       /**< The next pool in the list*/
	Sophon_U32         *used_flags; /**< Entry used flags*/
	Sophon_U32         *mark_flags; /**< Entry marked flags*/
	Sophon_NumberEntry *free_list;  /**< Free entries list*/
	Sophon_Number      *entries;    /**< Entries*/
	Sophon_U32          free_count; /**< Free entries count*/
};

extern void sophon_number_pool_init (Sophon_VM *vm);
extern void sophon_number_pool_deinit (Sophon_VM *vm);
extern void sophon_number_pool_sweep (Sophon_VM *vm);
extern void sophon_number_mark (Sophon_VM *vm,
						Sophon_Value v);
extern void sophon_value_set_number_real (Sophon_VM *vm,
						Sophon_Value *v,
						Sophon_Number d);

#ifdef __cplusplus
}
#endif

#endif

