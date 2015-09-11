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

#ifndef _SOPHON_FUNCTION_H_
#define _SOPHON_FUNCTION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sophon_types.h"
#include "sophon_hash.h"

/**\brief The function is a native function*/
#define SOPHON_FUNC_FL_NATIVE 1
/**\brief The function contains 'eval' code*/
#define SOPHON_FUNC_FL_EVAL   2
/**\brief The function should be run in strict mode*/
#define SOPHON_FUNC_FL_STRICT 4

/**\brief Native function*/
typedef Sophon_Result (*Sophon_NativeFunc)(Sophon_VM *vm);

/**\brief Function*/
struct Sophon_Function_s {
	Sophon_Module *module;    /**< The module contains this function*/
	Sophon_Hash    var_hash;  /**< Variants and arguments hash table*/
	Sophon_U8      flags;     /**< The function's flags*/
	Sophon_U8      argc;      /**< Arguments count*/
	Sophon_U16     varc;      /**< Variants count*/
	Sophon_U16     cbuf_size; /**< Byte code buffer size*/
	union {
		Sophon_NativeFunc  native; /**< The native function pointer*/
		Sophon_U8         *cbuf;   /**< Byte code buffer*/
	}c;
};

#ifdef __cplusplus
}
#endif

#endif

