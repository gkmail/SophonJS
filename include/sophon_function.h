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
/**\brief Global function*/
#define SOPHON_FUNC_FL_GLOBAL 8

/**\brief Variant type*/
typedef enum {
	SOPHON_FUNC_VAR, /**< Normal variant*/
	SOPHON_FUNC_ARG  /**< Argument*/
} Sophon_FuncVarType;

/**\brief Argument flag of variant id*/
#define SOPHON_VAR_FL_ARG   0x8000
/**\brief Get the index of the variant*/
#define SOPHON_VAR_GET_ID(v) ((v) & 0x7FFF)

/**\brief Native function*/
typedef Sophon_Result (*Sophon_NativeFunc)(Sophon_VM *vm, Sophon_Value thisv,
			Sophon_Value *argv, Sophon_Int argc, Sophon_Value *retv);

#ifdef SOPHON_LINE_INFO
/**\brief Line information*/
typedef struct {
	Sophon_U16     line;      /**< Line number*/
	Sophon_U16     offset;    /**< Byte code offset*/
} Sophon_LineInfo;
#endif

/**\brief Function*/
struct Sophon_Function_s {
	Sophon_Module *module;    /**< The module contains this function*/
	Sophon_Function *container; /**< The container function*/
	Sophon_Hash    var_hash;  /**< Variants and arguments hash table*/
	Sophon_String *name;      /**< The name of the function*/
	Sophon_U8      flags;     /**< The function's flags*/
	Sophon_U8      argc;      /**< Arguments count*/
	Sophon_U16     varc;      /**< Variants count*/
	Sophon_U16     id;        /**< The function id*/
	Sophon_U16     stack_size;/**< Stack size*/
	Sophon_U16     ibuf_size; /**< Instruction buffer size*/
#ifdef SOPHON_LINE_INFO
	Sophon_U16       lbuf_size;    /**< Line information buffer size*/
	Sophon_LineInfo *lbuf;         /**< Line information buffer*/
#endif
	union {
		Sophon_NativeFunc  native; /**< The native function pointer*/
		Sophon_U8         *ibuf;   /**< Instruction buffer*/
	}f;                            /**< Function data union*/
};

/**
 * \brief Add a variant to the function
 * \param[in] vm The current virtual machine
 * \param[in] func The function
 * \param type The variant type
 * \param name The variant name
 * \retval SOPHON_OK On success
 * \retval SOPHON_NONE The variant has already been defined
 */
extern Sophon_Result       sophon_function_add_var (Sophon_VM *vm,
							Sophon_Function *func,
							Sophon_FuncVarType type,
							Sophon_String *name);

/**
 * \brief Lookup the variant from the function by its name
 * \param[in] vm The current virtual machine
 * \param[in] func The function
 * \param name The variant name
 * \return The variant id
 * \retval SOPHON_ERR_NOTEXIST Cannot find the variant with the name
 */
extern Sophon_Int         sophon_function_lookup_var (Sophon_VM *vm,
							Sophon_Function *func,
							Sophon_String *name);

#ifdef __cplusplus
}
#endif

#endif

