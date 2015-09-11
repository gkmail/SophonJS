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

#ifndef _SOPHON_UTIL_H_
#define _SOPHON_UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sophon_types.h"

#ifdef SOPHON_HAVE_MEMSET
	#define sophon_memset(ptr, v, size) memset(ptr, v, size)
#else
	#error no "memset" defined
#endif

#ifdef SOPHON_HAVE_MEMCPY
	#define sophon_memcpy(dst, src, size) memcpy(dst, src, size)
#else
	#error no "memcpy" defined
#endif

#ifdef SOPHON_HAVE_MEMMOVE
	#define sophon_memmove(dst, src, size) memmove(dst, src, size)
#else
	#error no "memmove" defined
#endif

#ifdef SOPHON_HAVE_STRLEN
	#define sophon_strlen(str) strlen((char*)(str))
#else
	#error no "strlen" defined
#endif

#ifdef SOPHON_HAVE_STRCMP
	#define sophon_strcmp(s1, s2) strcmp(s1, s2)
#else
	#error no "strcmp" defined
#endif

#ifdef SOPHON_HAVE_REALLOC
	#define sophon_realloc(ptr, size) realloc(ptr, size)
#else
	#error no "realloc" defined
#endif

#define sophon_isascii(c)  (!((c) & 0xFFFFFF80))
#define sophon_islower(c)  (((c) >= 'a') && ((c) <= 'z'))
#define sophon_isupper(c)  (((c) >= 'A') && ((c) <= 'Z'))
#define sophon_isdigit(c)  (((c) >= '0') && ((c) <= '9'))
#define sophon_isxdigit(c) (sophon_isdigit(c) || \
			(((c) >= 'a') && ((c) <= 'f')) || \
			(((c) >= 'A') && ((c) <= 'F')))
#define sophon_isblank(c)  ((c) == ' ') || ((c) == '\t')
#define sophon_isspace(c)  (sophon_isblank(c) || \
			((c) == '\n') || ((c) == '\r') || ((c) == '\f') || ((c) == '\v'))
#define sophon_isalpha(c)  (sophon_islower(c) || sophon_isupper(c))
#define sophon_isalnum(c)  (sophon_isalpha(c) || sophon_isdigit(c))

extern void sophon_prerr(const char *fmt, ...);
extern void sophon_vprerr(const char *fmt, va_list ap);

extern void sophon_warning (const char *fmt, ...);
extern void sophon_error (const char *fmt, ...);
extern void sophon_fatal (const char *fmt, ...);

#define sophon_nomem() sophon_fatal("not enough memory!")

#ifdef SOPHON_UTF8_CHAR
typedef Sophon_U16 Sophon_CharRange;
#else
typedef Sophon_U32 Sophon_CharRange;
#endif

/**
 * \brief Check if the character is in the code table
 * Table is an array of 32bits unsigned integer.
 * Every integer defined an unicode range.
 * The low 16 bits is the minimum code in the range.
 * The high 16 bits is the maximum code in the range.
 * \param ch The character
 * \param[in] tab The range table
 * \param size The range table size
 * \retval SOPHON_TRUE The character is in the table
 * \retval SOPHON_FALSE The character is not in the table
 */
extern Sophon_Bool sophon_char_table_search (Sophon_Char ch,
					const Sophon_CharRange *tab,
					Sophon_U32 size);

/**
 * \brief Convert a string into integer number
 * \param[in] str The string
 * \param[out] end If end is not NULL, return the end position of the number
 * \param base The base of the number
 * \param[out] i Return the integer number
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_strtoi (const Sophon_Char *nptr,
			Sophon_Char **endptr, Sophon_Int base, Sophon_Int *pi);

/**
 * \brief Convert a string into double precision number
 * \param[in] str The string
 * \param[out] end If end is not NULL, return the end position of the number
 * \param[out] d Return the double precision number
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_strtod (const Sophon_Char *str,
			Sophon_Char **end, Sophon_Double *pd);

#ifdef __cplusplus
}
#endif

#endif

