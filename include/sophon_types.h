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

#ifndef _SOPHON_TYPES_H_
#define _SOPHON_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sophon_config.h"

/**\brief Integer number*/
typedef int            Sophon_Int;
/**\brief Unsigned integer number*/
typedef unsigned int   Sophon_UInt;
/**\brief Float point number*/
typedef float          Sophon_Float;
/**\brief Double precision number*/
typedef double         Sophon_Double;
/**\brief Unsigned 8bits number*/
typedef unsigned char  Sophon_U8;
/**\brief Signed 8bits number*/
typedef char           Sophon_S8;
/**\brief Unsigned 16bits number*/
typedef unsigned short Sophon_U16;
/**\brief Signed 16bits number*/
typedef short          Sophon_S16;
/**\brief Unsigned 32bits number*/
typedef unsigned int   Sophon_U32;
/**\brief Signed 32bits number*/
typedef int            Sophon_S32;
/**\brief Unsigned 64bits number*/
typedef unsigned long long Sophon_U64;
/**\brief Signed 64bits number*/
typedef long long          Sophon_S64;
/**\brief Pointer*/
typedef void*          Sophon_Ptr;
/**\brief Boolean*/
typedef unsigned char  Sophon_Bool;

/**\brief Number*/
typedef Sophon_Double  Sophon_Number;

#ifdef SOPHON_8BITS_CHAR
typedef Sophon_S8      Sophon_Char;
#else
/**\brief Unicode character*/
typedef Sophon_U16     Sophon_Char;
#endif

#define SOPHON_MAX_U8        0xFF
#define SOPHON_MAX_U16       0xFFFF
#define SOPHON_MAX_U32       0xFFFFFFFF

#define SOPHON_MAX_INT       2147483647
#define SOPHON_MIN_INT       (-SOPHON_MAX_INT - 1)

#define SOPHON_MAX_S64       9223372036854775807LL
#define SOPHON_MIN_S64       (-SOPHON_MAX_S64 - 1LL)

#ifdef SOPHON_64BITS
typedef long long            Sophon_IntPtr;
typedef unsigned long long   Sophon_UIntPtr;
#else
/**\brief Signed integer with the pointer size*/
typedef int                  Sophon_IntPtr;
/**\brief Unsigned integer with the pointer size*/
typedef unsigned int         Sophon_UIntPtr;
#endif

/**\brief Function result code*/
typedef int                  Sophon_Result;

/**\brief Break*/
#define SOPHON_BREAK         3
/**\brief Continue*/
#define SOPHON_CONTINUE      2
/**\brief No error*/
#define SOPHON_OK            1
/**\brief No error, but the function do nothing*/
#define SOPHON_NONE          0
/**\brief File end*/
#define SOPHON_ERR_EOF       -1
/**\brief Not enough memory*/
#define SOPHON_ERR_NOMEM     -2
/**\brief Not supported*/
#define SOPHON_ERR_NOTSUPP   -3
/**\brief The data is too large*/
#define SOPHON_ERR_2BIG      -4
/**\brief Illegal code*/
#define SOPHON_ERR_ILLCODE   -5
/**\brief Lexical error*/
#define SOPHON_ERR_LEX       -6
/**\brief Parse error*/
#define SOPHON_ERR_PARSE     -7
/**\brief Do not exist*/
#define SOPHON_ERR_NOTEXIST  -8
/**\brief Access deny*/
#define SOPHON_ERR_ACCESS    -9
/**\brief Read only*/
#define SOPHON_ERR_RDONLY    -10
/**\brief Type mismatch*/
#define SOPHON_ERR_TYPE      -11
/**\brief Exception*/
#define SOPHON_ERR_THROW     -12
/**\brief Illegal argument*/
#define SOPHON_ERR_ARG       -13

/**\brief Character encoding*/
typedef enum {
	SOPHON_UTF16LE, /**< UTF16 little endian*/
	SOPHON_UTF16BE, /**< UTF16 big endian*/
	SOPHON_UTF8     /**< UTF8*/
} Sophon_Encoding;

#ifdef SOPHON_LITTLE_ENDIAN
	#define SOPHON_UTF16 SOPHON_UTF16LE
#else
	#define SOPHON_UTF16 SOPHON_UTF16BE
#endif

#ifdef SOPHON_8BITS_CHAR
	#define SOPHON_ENC   SOPHON_UTF8
#else
	#define SOPHON_ENC   SOPHON_UTF16
#endif

/**\brief Character encoding convertor*/
typedef Sophon_Ptr         Sophon_Conv;
/**\brief Value*/
typedef Sophon_UIntPtr     Sophon_Value;
/**\brief GC managed object*/
typedef struct Sophon_GCObject_s Sophon_GCObject;
/**\brief Virtual machine*/
typedef struct Sophon_VM_s Sophon_VM;
/**\brief String*/
typedef struct Sophon_String_s Sophon_String;
/**\brief Object*/
typedef struct Sophon_Object_s Sophon_Object;
/**\brief Stack frame*/
typedef struct Sophon_Frame_s  Sophon_Frame;
/**\brief Function*/
typedef struct Sophon_Function_s Sophon_Function;
/**\brief Module*/
typedef struct Sophon_Module_s   Sophon_Module;
/**\brief Closure*/
typedef struct Sophon_Closure_s  Sophon_Closure;
/**\brief Stack entry*/
typedef struct Sophon_Stack_s    Sophon_Stack;
/**\brief Array*/
typedef struct Sophon_Array_s    Sophon_Array;
/**\brief Lexical token value*/
typedef union Sophon_TokenValue_u Sophon_TokenValue;
/**\brief Property iterator*/
typedef struct Sophon_PropIter_s  Sophon_PropIter;

#ifndef NULL
#define NULL ((void*)0)
#endif

/**\brief Boolean value true*/
#define SOPHON_TRUE  1
/**\brief Boolean value false*/
#define SOPHON_FALSE 0

#define SOPHON_ARRAY_SIZE(a)  (sizeof(a) / sizeof((a)[0]))

#define SOPHON_MAX(a, b) ((a)>(b)?(a):(b))
#define SOPHON_MIN(a, b) ((a)<(b)?(a):(b))
#define SOPHON_ABS(a)    ((a)<0?-(a):(a))

#define SOPHON_MACRO_BEGIN do {
#define SOPHON_MACRO_END   } while(0)

/*Signed integer right shift bits*/
#ifdef SOPHON_SINT_LOGIC_SHR
	#define SOPHON_SHR(n, b) \
		(((n)<0) ? ~((~(n))>>(b)) : ((n)>>(b)))
#else
	#define SOPHON_SHR(n, b) ((n) >> (b))
#endif

#ifdef __cplusplus
}
#endif

#endif

