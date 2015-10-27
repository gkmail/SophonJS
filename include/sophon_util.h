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

#ifdef SOPHON_HAVE_STRCASECMP
	#define sophon_strcasecmp(s1, s2) strcasecmp(s1, s2)
#else
	#error no "strcasecmp" defined
#endif

#ifdef SOPHON_HAVE_STRNCMP
	#define sophon_strncmp(s1, s2) strncmp(s1, s2)
#else
	#error no "strncmp" defined
#endif

#ifdef SOPHON_HAVE_STRNCASECMP
	#define sophon_strncasecmp(s1, s2) strncasecmp(s1, s2)
#else
	#error no "strncasecmp" defined
#endif

#ifdef SOPHON_HAVE_STRSTR
	#define sophon_strstr(s, sub) strstr(s, sub)
#else
	#error no "strstr" defined
#endif

#ifdef SOPHON_HAVE_STRCHR
	#define sophon_strchr(s, c) strstr(s, c)
#else
	#error no "strchr" defined
#endif


#ifdef SOPHON_HAVE_REALLOC
	#define sophon_realloc(ptr, size) realloc(ptr, size)
#else
	#error no "realloc" defined
#endif

#ifdef SOPHON_HAVE_MATH
#define SOPHON_NAN      NAN
#define SOPHON_INFINITY INFINITY
#define sophon_isnan(n)    isnan(n)
#define sophon_isinf(n)    isinf(n)
#define sophon_fmod(a, b)  fmod(a, b)
#define sophon_abs(n)      fabs(n)
#define sophon_acos(n)     acos(n)
#define sophon_asin(n)     asin(n)
#define sophon_atan(n)     atan(n)
#define sophon_ceil(n)     ceil(n)
#define sophon_cos(n)      cos(n)
#define sophon_exp(n)      exp(n)
#define sophon_floor(n)    floor(n)
#define sophon_trunc(n)    trunc(n)
#define sophon_log(n)      log(n)
#define sophon_sin(n)      sin(n)
#define sophon_round(n)    round(n)
#define sophon_tan(n)      tan(n)
#define sophon_sqrt(n)     sqrt(n)
#define sophon_atan2(a, b) atan2(a, b)
#define sophon_pow(a, b)   pow(a, b)
#else
#error no math functions defined
#endif

#ifdef SOPHON_HAVE_DRAND48
#define sophon_random()    drand48()
#elif defined(SOPHON_HAVE_RAND)
#define sophon_random()    (rand()/(RAND_MAX+1.0))
#else
#error no random function defined
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
#define sophon_isprint(c)  (((c) >= 0x20) && ((c) <= 0x7E))

#define sophon_tolower(c) (sophon_isupper(c) ? ((c) - 'A' + 'a') : (c))
#define sophon_toupper(c) (sophon_islower(c) ? ((c) - 'a' + 'A') : (c))

#ifdef SOPHON_HAVE_TOWUPPER
	#define sophon_towupper(c) towupper(c)
#endif
#ifdef SOPHON_HAVE_TOWLOWER
	#define sophon_towlower(c) towlower(c)
#endif
#ifdef SOPHON_HAVE_ISWSPACE
	#define sophon_iswspace(c) iswspace(c)
#endif

extern void sophon_prerr(const char *fmt, ...);
extern void sophon_vprerr(const char *fmt, va_list ap);
extern void sophon_pr(const char *fmt, ...);
extern void sophon_vpr(const char *fmt, va_list ap);

extern void sophon_warning (const char *fmt, ...);
extern void sophon_error (const char *fmt, ...);
extern void sophon_fatal (const char *fmt, ...);

#define sophon_nomem() sophon_fatal("not enough memory!")

#ifdef SOPHON_8BITS_CHAR
typedef Sophon_U16 Sophon_CharRange;
#define SOPHON_MAKE_CHAR_RANGE(min, max) (((max) << 8) | (min))
#define SOPHON_EXPAND_CHAR_RANGE(v, min, max)\
	SOPHON_MACRO_BEGIN\
		(min) = (v) & 0xFF;\
		(max) = ((v) >> 8) & 0xFF;\
	SOPHON_MACRO_END
#else
typedef Sophon_U32 Sophon_CharRange;
#define SOPHON_MAKE_CHAR_RANGE(min, max) (((max) << 16) | (min))
#define SOPHON_EXPAND_CHAR_RANGE(v, min, max)\
	SOPHON_MACRO_BEGIN\
		(min) = (v) & 0xFFFF;\
		(max) = ((v) >> 16) & 0xFFFF;\
	SOPHON_MACRO_END
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

/**\brief Convert string to integer*/
#define SOPHON_BASE_INT       (0)
/**\brief Convert string to float point number*/
#define SOPHON_BASE_FLOAT     (-1)
/**\brief Convert string to integer or float point number*/
#define SOPHON_BASE_INT_FLOAT (-2)

/**
 * \brief Convert a string into double precision number
 * \param[in] str The string
 * \param[out] end If end is not NULL, return the end position of the number
 * \param base The base of the number
 * \param[out] pd Return the double precision number
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_strtod (const Sophon_Char *str,
			Sophon_Char **end, Sophon_Int base, Sophon_Number *pd);

typedef enum {
	SOPHON_D2STR_RADIX,
	SOPHON_D2STR_FIXED,
	SOPHON_D2STR_EXP,
	SOPHON_D2STR_PREC
} Sophon_D2StrMode;

/**
 * \brief Convert a double precision number into string
 * \param[in] vm The current virtual machine
 * \param d The double value
 * \param mode Convert mode
 * \param param Convert parameter
 * \return The string
 */
extern Sophon_String* sophon_dtostr (Sophon_VM *vm, Sophon_Double d,
			Sophon_D2StrMode mode, Sophon_Int param);

/**\brief Value compare function*/
typedef Sophon_Int (*Sophon_CmpFunc) (Sophon_VM *vm,
					Sophon_Value v1, Sophon_Value v2, Sophon_Ptr arg);

/**
 * \brief Sort the value array by the compare function
 * \param[in] vm The current virtual machine
 * \param[in] v The value array
 * \param n Value array length
 * \param func Compare function
 * \param[in] User defined compare function
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result  sophon_qsort (Sophon_VM *vm, Sophon_Value *v,
			Sophon_Int n, Sophon_CmpFunc cmp, Sophon_Ptr arg);

#ifdef SOPHON_8BITS_CHAR

#define sophon_ucstrlen(s)      sophon_strlen(s)
#define sophon_ucstrcmp(s1, s2) sophon_strcmp(s1, s2)
#define sophon_ucstrcasecmp(s1, s2)  sophon_strcasecmp(s1, s2)
#define sophon_ucstrncmp(s1, s2)     sophon_strncmp(s1, s2)
#define sophon_ucstrncasecmp(s1, s2) sophon_strncasecmp(s1, s2)
#define sophon_ucstrstr(str, sub)    sophon_strstr(str, sub)
#define sophon_ucstrchr(str, chr)    sophon_strchr(str, chr)

#else /*!defined(SOPHON_8BITS_CHAR)*/

/**
 * \brief Calculate unicode string's length
 * \param[in] str String
 * \return The string length
 */
extern Sophon_U32     sophon_ucstrlen (const Sophon_Char *str);

/**
 * \brief Compare 2 unicode C strings
 * \param[in] s1 String 1
 * \param[in] s2 String 2
 * \retval 0 Strings are equal
 * \retval <0 s1 < s2
 * \retval >0 s1 > s2
 */
extern Sophon_Int     sophon_ucstrcmp (const Sophon_Char *s1,
						const Sophon_Char *s2);

/**
 * \brief Compare 2 unicode C strings do not care case
 * \param[in] s1 String 1
 * \param[in] s2 String 2
 * \retval 0 Strings are equal
 * \retval <0 s1 < s2
 * \retval >0 s1 > s2
 */
extern Sophon_Int     sophon_ucstrcasecmp (const Sophon_Char *s1,
						const Sophon_Char *s2);

/**
 * \brief Compare 2 unicode C strings
 * \param[in] s1 String 1
 * \param[in] s2 String 2
 * \param len Compare length
 * \retval 0 Strings are equal
 * \retval <0 s1 < s2
 * \retval >0 s1 > s2
 */
extern Sophon_Int     sophon_ucstrncmp (const Sophon_Char *s1,
						const Sophon_Char *s2,
						Sophon_U32 len);

/**
 * \brief Compare 2 unicode C strings do not care case
 * \param[in] s1 String 1
 * \param[in] s2 String 2
 * \param len Compare length
 * \retval 0 Strings are equal
 * \retval <0 s1 < s2
 * \retval >0 s1 > s2
 */
extern Sophon_Int     sophon_ucstrncasecmp (const Sophon_Char *s1,
						const Sophon_Char *s2,
						Sophon_U32 len);

/**
 * \brief Search the substring in an unicode string
 * \param[in] str String
 * \param[in] sub String to search
 * \return The sub string position
 * \retval NULL Cannot find the substring
 */
extern const Sophon_Char* sophon_ucstrstr (const Sophon_Char *str,
						const Sophon_Char *sub);

/**
 * \brief Search a character in an unicode string
 * \param[in] str String
 * \param chr The character
 * \return The position find the character
 * \retval NULL Cannot find the character
 */
extern const Sophon_Char* sophon_ucstrchr (const Sophon_Char *str,
						Sophon_Char chr);

#endif /*SOPHON_8BITS_CHAR*/

#ifdef __cplusplus
}
#endif

#endif

