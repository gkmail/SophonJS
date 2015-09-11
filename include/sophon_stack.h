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

#ifndef _SOPHON_STACK_H_
#define _SOPHON_STACK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sophon_types.h"

/**\brief Stack entry*/
struct Sophon_Stack_s {
	Sophon_Stack   *bottom; /**< The bottom stack entry*/
	Sophon_Frame   *var_env;/**< The variant frame*/
	Sophon_Frame   *lex_env;/**< The lexical frame*/
	Sophon_Closure *closure;/**< The current closure*/
	Sophon_U16      ip;     /**< Instruction pointer*/
	Sophon_U16      sp;     /**< Stack pointer*/
	Sophon_U16      tp;     /**< Try stack pointer*/
	Sophon_U16      bp;     /**< Block stack pointer*/
	Sophon_Value    v[1];   /**< The stack value buffer*/
};

#define sophon_stack_init(vm)   ((vm)->stack = NULL)
#define sophon_stack_deinit(vm)

/**
 * \brief Push a stack entry
 * \param[in] vm The current virtual machine
 * \return The new stack entry
 */
extern Sophon_Stack* sophon_stack_push (Sophon_VM *vm);

#ifdef __cplusplus
}
#endif

#endif

