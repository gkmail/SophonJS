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
#include "sophon_property.h"

/**\brief Stack entry*/
struct Sophon_Stack_s {
	Sophon_Stack   *bottom; /**< The bottom stack entry*/
	Sophon_Frame   *var_env;/**< The variant frame*/
	Sophon_Frame   *lex_env;/**< The lexical frame*/
	Sophon_Value    calleev;/**< The callee closure value*/
	Sophon_Value    retv;   /**< Return value*/
	Sophon_Function *func;  /**< The current function*/
	Sophon_PropIter *pi_bottom; /**< The bottom property iterator*/
	Sophon_U16      vbuf_size; /**< Value buffer size*/
	Sophon_U16      ip;     /**< Instruction pointer*/
	Sophon_U16      sp;     /**< Stack pointer*/
	Sophon_U16      tp;     /**< Try stack pointer*/
	Sophon_Value    v[1];   /**< The stack value buffer*/
};

#define sophon_stack_init(vm)   ((vm)->stack = NULL)
#define sophon_stack_deinit(vm)

/**
 * \brief Push a global frame into the stack
 * \param[in] vm The current virtual machine
 * \param globv The global object value
 * \param calleev The callee closure value
 * \param[in] argv Input arguments
 * \param argc The real arguments count
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_stack_push_global (Sophon_VM *vm,
						Sophon_Value globv,
						Sophon_Value calleev,
						Sophon_Value *argv,
						Sophon_U8 argc);

/**
 * \brief Push a declaration frame into the stack
 * \param[in] vm The current virtual machine
 * \param thisv This value
 * \param calleev The callee closure value
 * \param[in] argv Input arguments
 * \param argc The real arguments count
 * \param flags Invoke flags
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_stack_push_decl (Sophon_VM *vm,
						Sophon_Value thisv,
						Sophon_Value calleev,
						Sophon_Value *argv,
						Sophon_U8 argc,
						Sophon_U32 flags);

/**
 * \brief Push a named value frame into the stack
 * \param[in] vm The current virtual machine
 * \param name The name of the value
 * \param v The value
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_stack_push_name (Sophon_VM *vm,
						Sophon_String *name,
						Sophon_Value v);

/**
 * \brief Push a with frame into the stack
 * \param[in] vm The current virtual machine
 * \param thisv This value
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_stack_push_with (Sophon_VM *vm,
						Sophon_Value thisv);

/**
 * \brief Pop the top stack entry
 * \param[in] vm The current virtual machine
 */
extern void          sophon_stack_pop (Sophon_VM *vm);

/**
 * \brief Pop a frame from the stack
 * \param[in] vm The current virtual machine
 */
extern void          sophon_stack_pop_frame (Sophon_VM *vm);

/**
 * \brief Delete a binding by its name
 * \param[in] vm The current virtual machine
 * \param name The binding name
 * \param flags Invoek flags
 * \return SOPHON_OK On success
 * \retuen SOPHON_NENE The binding do not exist
 * \retval <0 On error
 */
extern Sophon_Result sophon_stack_delete_binding (Sophon_VM *vm,
						Sophon_String *name,
						Sophon_U32 flags);

/**
 * \brief Get a binding value by its name
 * \param[in] vm The current virtual machine
 * \param name The binding name
 * \param[out] getv Return the binding value
 * \param flags Invoke flags
 * \return SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_stack_get_binding (Sophon_VM *vm,
						Sophon_String *name,
						Sophon_Value *getv,
						Sophon_U32 flags);

/**
 * \brief Set a binding value by its name
 * \param[in] vm The current virtual machine
 * \param name The binding name
 * \param setv The new binding value
 * \param flags Invoke flags
 * \return SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_stack_put_binding (Sophon_VM *vm,
						Sophon_String *name,
						Sophon_Value setv,
						Sophon_U32 flags);

/**
 * \brief Get this binding value
 * \param[in] vm The current virtual machine
 * \return This binding value
 */
extern Sophon_Value sophon_stack_get_this (Sophon_VM *vm);

/**
 * \brief Get the current module
 * \param[in] vm The current virtual machine
 * \return The current module
 */
extern Sophon_Module* sophon_stack_get_module (Sophon_VM *vm);

#ifdef __cplusplus
}
#endif

#endif

