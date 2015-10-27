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

#ifndef _SOPHON_LEX_H_
#define _SOPHON_LEX_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sophon_types.h"

/**\brief Lexical need encoding convertor*/
#define SOPHON_LEX_FL_CONV       1
/**\brief The last character is new line*/
#define SOPHON_LEX_FL_LAST_NL    2
/**\brief The last token is line terminator*/
#define SOPHON_LEX_FL_LAST_LT    4
/**\brief Comment have line terminator in it*/
#define SOPHON_LEX_FL_COMMENT_LT 8
/**\brief Shell mode continue read start*/
#define SOPHON_LEX_FL_SHELL_CONT 16

/**
 * \brief Lexical token
 */
typedef Sophon_Int Sophon_Token;

/**\brief Input location*/
typedef struct {
	Sophon_U32  first_line;   /**< The beginning line number of the token*/
	Sophon_U32  first_column; /**< The beginning column number of the token*/
	Sophon_U32  last_line;    /**< The end line number of the token*/
	Sophon_U32  last_column;  /**< The end column number of the token*/
} Sophon_Location;

/**
 * \brief Input/output function
 */
typedef Sophon_Int (*Sophon_IOFunc)(Sophon_Ptr data, Sophon_U8 *buf,
						Sophon_Int size);

/**
 * \brief Lexical analyser data
 */
typedef struct {
	Sophon_IOFunc     inp_func;  /**< Character input function*/
	Sophon_Ptr        inp_data;  /**< User defined input function argument*/
	Sophon_U8        *ibuf;      /**< Input data buffer*/
	Sophon_U32        ibegin;    /**< Next data position in the input buffer*/
	Sophon_U32        ilen;      /**< Input data length*/
	Sophon_Char      *cbuf;      /**< Character buffer*/
	Sophon_U32        cbegin;    /**< Next character position in the cbuf*/
	Sophon_U32        clen;      /**< Valid characters count in the cbuf*/
	Sophon_Char      *tbuf;      /**< Text buffer*/
	Sophon_U32        tbuf_size; /**< Text buffer length*/
	Sophon_U32        tlen;      /**< Text length*/
	Sophon_Conv       conv;      /**< Encoding convertor*/
	Sophon_U32        cond;      /**< The current lexical condition*/
	Sophon_U32        flags;     /**< Lexical flags*/
	Sophon_U32        line;      /**< The current line number*/
	Sophon_U32        column;    /**< The current column number*/
} Sophon_LexData;

/**
 * \brief Check if the last token is after a line terminator
 */
#define sophon_lex_has_line_term(vm) \
	(((Sophon_LexData*)(vm)->lex_data)->flags & SOPHON_LEX_FL_LAST_LT)

/**
 * \brief Initialize the lexical analyser
 * \param enc Input characeter's encoding
 * \param input Character input function
 * \param data The user defined data used as input function's argument
 */
extern void          sophon_lex_init (Sophon_VM *vm,
						Sophon_Encoding enc,
						Sophon_IOFunc input,
						Sophon_Ptr data);

/**
 * \brief Release the lexical analyser
 * \param[in] vm The current virtual machine
 */
extern void          sophon_lex_deinit (Sophon_VM *vm);

/**
 * \brief Get the next token from the input
 * \param[in] vm The current virtual machine
 * \param[out] val The token value
 * \param[out] loc The location of the token
 * \return The next token
 * \retval SOPHON_ERR_EOF Input is end
 * \retval <0 On error
 */
extern Sophon_Token  sophon_lex (Sophon_VM *vm,
						Sophon_TokenValue *val,
						Sophon_Location *loc);

/**
 * \brief Switch to regular expression condition
 * \param[in] vm The current virtual machine
 * \param t The beginning token of the regular expression
 */
extern void          sophon_lex_to_regexp_cond (Sophon_VM *vm,
						Sophon_Token t);

#ifdef __cplusplus
}
#endif

#endif

