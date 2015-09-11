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

#ifndef _SOPHON_DOUBLE_POOL_H_
#define _SOPHON_DOUBLE_POOL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sophon_value.h"

typedef struct Sophon_DoubleEntry_s Sophon_DoubleEntry;
struct Sophon_DoubleEntry_s {
	Sophon_DoubleEntry *next;
};

typedef struct Sophon_DoublePool_s Sophon_DoublePool;
struct Sophon_DoublePool_s {
	Sophon_DoublePool  *next;
	Sophon_U32         *used_flags;
	Sophon_U32         *mark_flags;
	Sophon_DoubleEntry *free_list;
	Sophon_Double      *entries;
	Sophon_U32          free_count;
};

extern void sophon_double_pool_init (Sophon_VM *vm);
extern void sophon_double_pool_deinit (Sophon_VM *vm);
extern void sophon_double_pool_sweep (Sophon_VM *vm);
extern void sophon_double_mark (Sophon_VM *vm,
						Sophon_Value v);
extern void sophon_value_set_double_real (Sophon_VM *vm,
						Sophon_Value *v,
						Sophon_Double d);

#ifdef __cplusplus
}
#endif

#endif

