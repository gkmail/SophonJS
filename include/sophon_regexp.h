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

#ifndef _SOPHON_REGEXP_H_
#define _SOPHON_REGEXP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sophon_types.h"
#include "sophon_gc.h"
#include "sophon_util.h"

/**\brief Global flag*/
#define SOPHON_REGEXP_FL_G 1
/**\brief Ignore case flag*/
#define SOPHON_REGEXP_FL_I 2
/**\brief Multiline flag*/
#define SOPHON_REGEXP_FL_M 4

/**\brief Regular expression*/
typedef struct {
	SOPHON_GC_HEADER
	Sophon_U8          flags; /**< Flags*/
	Sophon_String     *str;   /**< String of the regular expression*/
	Sophon_Int         last;  /**< Last match index*/
	Sophon_Ptr         re;    /**< Regular expression data*/
} Sophon_RegExp;

/**\brief Match position*/
typedef struct {
	Sophon_Char       *begin; /**< Beginning of the match string*/
	Sophon_Char       *end;   /**< End of the match string*/
} Sophon_ReMatch;

/**
 * \brief Return the match buffer size needed of the regular expression
 * \param[in] re The regular expression
 * \return The match buffer size
 */
extern Sophon_Int     sophon_regexp_match_size (Sophon_RegExp *re);

/**
 * \brief Use a regular expression to match a string
 * \param[in] vm The current virtual machine
 * \param[in] re The regular expression
 * \param[in] str The string
 * \param start The match start index
 * \param[out] mbuf Match position buffer
 * \param size Match position buffer size
 * \return Match position returned
 * \retval 0 Mismatch
 */
extern Sophon_Result  sophon_regexp_match (Sophon_VM *vm,
						Sophon_RegExp *re,
						Sophon_String *str,
						Sophon_U32 start,
						Sophon_ReMatch *mbuf,
						Sophon_U32 size);

/**
 * \brief Create a new regular expression
 * \param[in] vm The current virtual machine
 * \param[in] str The regular expression string
 * \param flags Flags
 * \return The new regular expression
 */
extern Sophon_RegExp* sophon_regexp_create (Sophon_VM *vm,
						Sophon_String *str,
						Sophon_U8 flags);

/**
 * \brief Release an unused regular expression
 * Regular expression is managed by GC.
 * You should not invoke this function directly.
 * \param[in] vm The current virtual machine
 * \param[in] re The regular expression to be released
 */
extern void           sophon_regexp_destroy (Sophon_VM *vm,
						Sophon_RegExp *re);

#ifdef __cplusplus
}
#endif

#endif

