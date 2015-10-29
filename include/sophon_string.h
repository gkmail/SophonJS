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

#ifndef _SOPHON_STRING_H_
#define _SOPHON_STRING_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sophon_types.h"
#include "sophon_gc.h"

/**\brief String*/
struct Sophon_String_s {
	SOPHON_GC_HEADER
	Sophon_U32   length;   /**< String length*/
	Sophon_Char  chars[1]; /**< Characters*/
};

#define sophon_string_intern_hash_init(vm) \
	sophon_hash_init(vm, &vm->str_intern_hash)
#define sophon_string_intern_hash_deinit(vm) \
	sophon_hash_deinit(vm, &vm->str_intern_hash)

/**
 * \brief Get the character buffer pointer of the string
 * The characters in the string is 0 termiated.
 */
#define sophon_string_chars(vm, str) \
	(((Sophon_String*)(str))->chars)

/**
 * \brief Get the string's length
 * Length is equal to the unicode count in the string.
 */
#define sophon_string_length(vm, str) \
	(((Sophon_String*)(str))->length)

#ifdef SOPHON_8BITS_CHAR
	#define SOPHON_UTF8_BUF_SIZE(len) ((len)+1)
#else
	#define SOPHON_UTF8_BUF_SIZE(len) ((len)*3+1)
#endif

/**
 * \brief Create a new string from an 0 terminated unicode C string
 * \param[in] vm The current virtual machine
 * \param[in] cstr The 0 terminated unicode C string
 * \return The new string
 * \retval NULL On error
 */
extern Sophon_String* sophon_string_create (Sophon_VM *vm,
						const Sophon_Char *cstr);

/**
 * \brief Create a new string from unicode characters
 * \param[in] vm The current virtual machine
 * \param[in] chars The unicode character buffer
 * \param len The characters count of the string
 * \return The new string
 * \retval NULL On error
 */
extern Sophon_String* sophon_string_from_chars (Sophon_VM *vm,
						const Sophon_Char *chars,
						Sophon_U32 len);

#ifdef SOPHON_8BITS_CHAR
#define sophon_string_from_utf8_cstr(vm, cstr)\
			sophon_string_create(vm, cstr)
#else
/**
 * \brief Create a new string from an 0 terminated UTF8 C string
 * \param[in] vm The current virtual machine
 * \param[in] cstr The 0 terminated UTF8 C string
 * \return The new string
 * \retval NULL On error
 */
extern Sophon_String* sophon_string_from_utf8_cstr (Sophon_VM *vm,
						const char *cstr);
#endif

#ifdef SOPHON_8BITS_CHAR
#define sophon_string_from_utf8_chars(vm, chars, len)\
			sophon_string_from_chars(vm, chars, len)
#else
/**
 * \brief Create a new string from UTF8 characters
 * \param[in] vm The current virtual machine
 * \param[in] chars The UTF8 character buffer
 * \param len Ths UTF8 character length in bytes
 * \return The new string
 * \retval NULL On error
 */
extern Sophon_String* sophon_string_from_utf8_chars (Sophon_VM *vm,
						const char *chars,
						Sophon_U32 len);
#endif

/**
 * \brief Convert the string to a UTF 0 terminated C string
 * \param[in] vm The current virtual machine
 * \param[in] str The string
 * \param[out] buf The output buffer to store the UTF8
 * \param[in,out] The output buffer size in bytes
 * Return the real string length in bytes.
 * The terminated 0 is not calculated.
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result  sophon_string_to_utf8_cstr (Sophon_VM *vm,
						Sophon_String *str,
						char *buf,
						Sophon_U32 *len);

/**
 * \brief Allocate buffer and convert the string to a UTF 0 terminated C string
 * Use sophon_free(vm, buf, len) to free the buffer.
 * \param[in] vm The current virtual machine
 * \param[in] str The string
 * \param[out] buf The output buffer to store the UTF8
 * \param[out] The output buffer size in bytes
 * Return the real string length in bytes.
 * The terminated 0 is not calculated.
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result  sophon_string_new_utf8_cstr (Sophon_VM *vm,
						Sophon_String *str,
						char **buf,
						Sophon_U32 *len);

#ifdef SOPHON_8BITS_CHAR
#define sophon_string_free_utf8_cstr(vm, buf, len)
#else
/**\brief Free the UTF8 buffer allocated by sophon_string_new_utf8_cstr*/
#define sophon_string_free_utf8_cstr(vm, buf, len)\
			sophon_mm_free(vm, buf, len)
#endif

/**
 * \brief Release an unused string
 * String is managed by GC.
 * You should not invoke this function directly.
 * \param[in] vm The current virtual machine
 * \param[in] str The string to be released
 */
extern void           sophon_string_destroy (Sophon_VM *vm,
						Sophon_String *str);

/**
 * \brief Make the string to be a unique one
 * Virtual machine has an intern string hash table.
 * This function check the string value in the hash table,
 * and return the unique string in the hash table with the same value.
 * \param[in] vm The current virtual machine
 * \param[in] str The string to be released
 * \return The unique string
 * \retval NULL On error
 */
extern Sophon_String* sophon_string_intern_real (Sophon_VM *vm,
						Sophon_String *str);

/**
 * \brief Make the string to be a unique one
 * Virtual machine has an intern string hash table.
 * This function check the string value in the hash table,
 * and return the unique string in the hash table with the same value.
 */
#define sophon_string_intern(vm, str)\
	(((str)->gc_flags & SOPHON_GC_FL_INTERN) ? (str) :\
				sophon_string_intern_real(vm, str))

/**
 * \brief Compare 2 strings
 * \param[in] vm The current virtual machine
 * \param[in] s1 String 1
 * \param[in] s2 STring 2
 * \retval 0 2 strings are equal
 * \retval >0 s1 > s2
 * \retval <0 s1 < s2
 */
extern Sophon_Int     sophon_string_cmp (Sophon_VM *vm,
						Sophon_String *s1,
						Sophon_String *s2);

/**
 * \brief Compare 2 strings do not care the case
 * \param[in] vm The current virtual machine
 * \param[in] s1 String 1
 * \param[in] s2 STring 2
 * \retval 0 2 strings are equal
 * \retval >0 s1 > s2
 * \retval <0 s1 < s2
 */
extern Sophon_Int     sophon_string_casecmp (Sophon_VM *vm,
						Sophon_String *s1,
						Sophon_String *s2);

/**
 * \brief Concatenate 2 strings
 * \param[in] vm The current virtual machine
 * \param[in] s1 String 1
 * \param[in] s2 STring 2
 * \return The new srting
 */
extern Sophon_String* sophon_string_concat (Sophon_VM *vm,
						Sophon_String *s1,
						Sophon_String *s2);

/**
 * \brief Convert the characters to escape characters
 * \param[in] vm The current virtual machine
 * \param[in] str The input string
 * \return The new string
 */
extern Sophon_String* sophon_string_escape (Sophon_VM *vm,
						Sophon_String *str);

/**
 * \brief Unescape the characters in the string
 * \param[in] vm The current virtual machine
 * \param[in] str The input string
 * \return The new string
 */
extern Sophon_String* sophon_string_unescape (Sophon_VM *vm,
						Sophon_String *str);

/**
 * \brief Get the character item in the string
 * \param[in] vm The current virtual machine
 * \param[in] str The input string
 * \param id The item's index
 * \param[out] retv The return value
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result  sophon_string_get_item (Sophon_VM *vm,
						Sophon_String *str,
						Sophon_U32 id,
						Sophon_Value *retv);

#ifdef __cplusplus
}
#endif

#endif

