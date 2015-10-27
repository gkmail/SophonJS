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

#ifndef _SOPHON_MODULE_H_
#define _SOPHON_MODULE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sophon_types.h"
#include "sophon_gc.h"
#include "sophon_hash.h"

/**\brief Module*/
struct Sophon_Module_s {
	SOPHON_GC_HEADER
	Sophon_String     *name;       /**< File name of the module*/
	Sophon_Value      *consts;     /**< Constant value buffer*/
	Sophon_Function **funcs;       /**< Functions buffer*/
	Sophon_Hash       const_hash;  /**< Constant hash table*/
	Sophon_U16        const_count; /**< Constants count*/
	Sophon_U16        const_cap;   /**< Constant buffer size*/
	Sophon_U16        func_count;  /**< Functions count*/
	Sophon_U16        func_cap;    /**< Function buffer size*/
	Sophon_Value      globv;       /**< Global object value*/
};

/**\brief Get a function from the module*/
#define sophon_module_get_func(mod, id)  ((mod)->funcs[id])

/**\brief Get a constant from the module*/
#define sophon_module_get_const(mod, id) ((mod)->consts[id])

/**\brief Set the module name*/
#define sophon_module_set_name(mod, name) ((mod)->name = (name))

/**
 * \brief Create a new module
 * \param[in] vm The current virtual machine
 * \return The new module
 */
extern Sophon_Module* sophon_module_create (Sophon_VM *vm);

/**
 * \brief Release an unused module
 * Module is managed by GC.
 * You should not invoke this function directly.
 * \param[in] vm The current virtual machine
 * \param[in] mod The module to be released
 */
extern void           sophon_module_destroy (Sophon_VM *vm,
						Sophon_Module *mod);

/**
 * \brief Create a function and add it to the module
 * \param[in] vm The current virtual machine
 * \param[in] mod The module
 * \param name The name of the function
 * \param flags The function's flags
 * \return The function's ID
 */
extern Sophon_Int     sophon_module_add_func (Sophon_VM *vm,
						Sophon_Module *mod,
						Sophon_String *name,
						Sophon_U32 flags);

/**
 * \brief Add a constant to the module
 * \param[in] vm The current virtual machine
 * \param[in] mod The module
 * \param v The constant value
 * \return The constant's ID
 */
extern Sophon_Int     sophon_module_add_const (Sophon_VM *vm,
						Sophon_Module *mod,
						Sophon_Value v);

/**
 * \brief Call the global object of the module
 * \param[in] vm The current virtual machine
 * \param[in] mod The module
 * \param[in] argv The arguments
 * \param argc arguments count
 * \param [out] retv The return value
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result  sophon_module_call (Sophon_VM *vm,
						Sophon_Module *mod,
						Sophon_Value *argv,
						Sophon_Int argc,
						Sophon_Value *retv);

/**
 * \brief Get the global object of the module
 * \param[in] vm The current virtual machine
 * \param[in] mod The module contains the object
 * If mod == NULL, return the global object of the global module.
 * \return The global object
 */
extern Sophon_Object* sophon_module_get_global (Sophon_VM *vm,
						Sophon_Module *mod);

#ifdef __cplusplus
}
#endif

#endif

