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

#ifndef _SOPHON_JSON_H_
#define _SOPHON_JSON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sophon_types.h"
#include "sophon_lex.h"

/**
 * \brief JSON parse parameters
 */
typedef struct {
	Sophon_Value  reviver; /**< Property reviver function*/
} Sophon_JSONParseParams;

/**
 * \brief JSON stringify parameters
 */
typedef struct {
	Sophon_Array  *whitelist; /**< Properties whitelist*/
	Sophon_Value   replacer;  /**< Property replacer function*/
	Sophon_String *space;     /**< Space string*/
} Sophon_JSONStringifyParams;

/**
 * \brief Parse a JSON string
 * \param[in] vm The current virtual machine
 * \param inp Input function
 * \param[in] data Input function's parameter
 * \param[out] retv Then return value
 * \param[in] p Parameters
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_json_parse (Sophon_VM *vm, Sophon_Encoding enc,
						Sophon_IOFunc inp, Sophon_Ptr data,
						Sophon_Value *retv,
						Sophon_JSONParseParams *p);

/**
 * \brief Convert a value to JSON string
 * \param[in] vm The current virtual machine
 * \param v The value
 * \param[in] p Parameters
 * \return The JSON string
 * \retval NULL On error
 */
extern Sophon_String* sophon_json_stringify (Sophon_VM *vm, Sophon_Value v,
						Sophon_JSONStringifyParams *p);

#ifdef __cplusplus
}
#endif

#endif

