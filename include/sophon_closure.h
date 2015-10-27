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

#ifndef _SOPHON_CLOSURE_H_
#define _SOPHON_CLOSURE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sophon_types.h"
#include "sophon_gc.h"
#include "sophon_property.h"
#include "sophon_object.h"

/**\brief Closure*/
struct Sophon_Closure_s {
	SOPHON_GC_HEADER
	union {
		struct {
			Sophon_Function *func;    /**< The function*/
			Sophon_Frame    *var_env; /**< The variant frame*/
			Sophon_Frame    *lex_env; /**< The lexical frame*/
		} func;                       /**< Function*/
		struct {
			Sophon_Value     funcv;   /**< Real closure value*/
			Sophon_Value     thisv;   /**< This value*/
			Sophon_Array    *args;    /**< Arguments array*/
		} bind;                       /**< Bind*/
	} c;                              /**< Closure data union*/
};

/**
 * \brief Create a new closure
 * \param[in] vm The current virtual machine
 * \param[in] func The function of the closure
 * \return The new closure
 */
extern Sophon_Closure* sophon_closure_create (Sophon_VM *vm,
						Sophon_Function *func);

/**
 * \brief Create a closure bind to another one
 * \param[in] vm The current virtual machine
 * \param funcv The closure value binded to
 * \param thisv This argument
 * \param[in] arr Arguments array
 * \return The new closure
 */
extern Sophon_Closure* sophon_closure_bind (Sophon_VM *vm,
						Sophon_Value funcv, Sophon_Value thisv,
						Sophon_Array *arr);

/**
 * \brief Release an unused closure
 * Closure is managed by GC.
 * You should not invoke this function directly.
 * \param[in] vm The current virtual machine
 * \param[in] clos The closure to be released
 */
extern void            sophon_closure_destroy (Sophon_VM *vm,
						Sophon_Closure *clos);

#ifdef __cplusplus
}
#endif

#endif

