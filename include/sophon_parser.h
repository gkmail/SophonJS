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

#ifndef _SOPHON_PARSER_H_
#define _SOPHON_PARSER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sophon_types.h"
#include "sophon_lex.h"

/**\brief Parse eval code*/
#define SOPHON_PARSER_FL_EVAL   1
/**\brief Strict mode*/
#define SOPHON_PARSER_FL_STRICT 2
/**\brief Parser in shell mode*/
#define SOPHON_PARSER_FL_SHELL  4
/**\brief Parse a function body*/
#define SOPHON_PARSER_FL_BODY   8
/**\brief Parser has inserted a semicolon automatically*/
#define SOPHON_PARSER_FL_AUTO_SEMICOLON 16
/**\brief Input is end*/
#define SOPHON_PARSER_FL_EOF            32
/**\brief Parse error*/
#define SOPHON_PARSER_FL_ERROR          64
/**\brief Function begin*/
#define SOPHON_PARSER_FL_FUNC_BEGIN     128
/**\brief Disable warning message*/
#define SOPHON_PARSER_FL_NO_WARNING     256

#define sophon_parser_init(vm)  ((vm)->parser_data = NULL)
#define sophon_parser_deinit(vm)

/**\brief Parser information type*/
enum {
	SOPHON_PARSER_ERROR,   /**< Error*/
	SOPHON_PARSER_WARNING  /**< Warning*/
};

/**
 * \brief Check if the parser is in strict mode
 * \param[in] vm The current virtual machine
 * \return If parser is in strict mode
 */
extern Sophon_Bool  sophon_parser_strict_mode (Sophon_VM *vm);

/**
 * \brief Parse the input
 * \param[in] vm The current virtual machine
 * \param[in] mod The module to hold the generated code
 * \param enc Input character's encoding
 * \param input Character input function
 * \param data User defined argument of input function
 * \param flags Parser flags
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_parse (Sophon_VM *vm,
						Sophon_Module *mod,
						Sophon_Encoding enc,
						Sophon_IOFunc input,
						Sophon_Ptr data,
						Sophon_U32 flags);

/**
 * \brief Output parser error message
 * \param[in] vm The current virtual machine
 * \param type Information type
 * \param[in] loc Location of the error
 * \param[in] fmt Format string
 */
extern void          sophon_parser_error (Sophon_VM *vm,
						int type, Sophon_Location *loc,
						const char *fmt, ...);

/**
 * \brief Output parser error message
 * \param[in] vm The current virtual machine
 * \param type Information type
 * \param[in] loc Location of the error
 * \param[in] fmt Format string
 */
extern void          sophon_parser_errorv (Sophon_VM *vm,
						int type, Sophon_Location *loc,
						const char *fmt, va_list ap);

/**
 * \brief Parse the input and run the instructions
 * \param[in] vm The current virtual machine
 * \param enc Input character's encoding
 * \param input Character input function
 * \param data User defined argument of input function
 * \param flags Parser flags
 * \param[out] retv The return value
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_eval (Sophon_VM *vm,
						Sophon_Encoding enc,
						Sophon_IOFunc input,
						Sophon_Ptr data,
						Sophon_U32 flags,
						Sophon_Value *retv);

#ifdef __cplusplus
}
#endif

#endif

