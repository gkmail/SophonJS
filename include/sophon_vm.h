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

#ifndef _SOPHON_VM_H_
#define _SOPHON_VM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sophon_types.h"
#include "sophon_value.h"
#include "sophon_hash.h"
#include "sophon_number_pool.h"
#include "sophon_gc.h"
#include "sophon_closure.h"

#define SOPHON_FOR_EACH_ERROR(e)\
	e(Error)\
	e(TypeError)\
	e(SyntaxError)\
	e(RangeError)\
	e(EvalError)\
	e(ReferenceError)\
	e(URIError)

#define SOPHON_FOR_EACH_INTERNAL_STRING(s)\
	s(use_strict, "use strict")\
	s(class,      "[Class]")\
	s(call,       "[Call]")\
	s(proto,      "[Proto]")\
	s(empty,      "")\
	s(comma,      ",")\
	s(lb,         "{")\
	s(rb,         "}")\
	s(ls,         "[")\
	s(rs,         "]")\
	s(quot,       "\"")\
	s(div,        "/")\
	s(colons,     ":")\
	s(space,      " ")\
	s(dollar,     "$")

#define SOPHON_FOR_EACH_STRING(s)\
	s(Object)\
	s(Boolean)\
	s(Number)\
	s(String)\
	s(Function)\
	s(Array)\
	s(Arguments)\
	s(RegExp)\
	s(Date)\
	s(arguments)\
	s(caller)\
	s(callee)\
	s(name)\
	s(length)\
	s(eval)\
	s(null)\
	s(undefined)\
	s(object)\
	s(string)\
	s(number)\
	s(function)\
	s(boolean)\
	s(true)\
	s(false)\
	s(NaN)\
	s(Infinity)\
	s(prototype)\
	s(message)\
	s(value)\
	s(writable)\
	s(get)\
	s(set)\
	s(enumerable)\
	s(configurable)\
	s(toString)\
	s(toJSON)\
	s(valueOf)\
	s(constructor)\
	s(index)\
	s(input)\
	s(exec)\
	s(JSON)\
	s(stringify)\
	SOPHON_FOR_EACH_ERROR(s)

/**\brief Virtual machine*/
struct Sophon_VM_s {
	/*Memory manager*/
	Sophon_U32 mm_curr_used;
	Sophon_U32 mm_max_used;

	/*Number pool*/
	Sophon_Hash         np_hash;
	Sophon_NumberPool  *np_free_list;
	Sophon_NumberPool  *np_full_list;

	/*Garbage collector*/
	Sophon_Bool       gc_running;
	Sophon_U32        gc_mem_size;
	Sophon_GCObject  *gc_used_list;
	Sophon_GCObject  *gc_unmanaged_list;
	Sophon_Hash       gc_root_hash;
	Sophon_GCObjectBuf  gc_obuf1;
	Sophon_GCObjectBuf  gc_obuf2;
	Sophon_GCObjectBuf  gc_nb_obuf;
	Sophon_GCObjectBuf *gc_curr_obuf;

	/*String intern hash table*/
	Sophon_Hash       str_intern_hash;

	/*Lexical analyser*/
	Sophon_Ptr        lex_data;

	/*Parser*/
	Sophon_Ptr        parser_data;

	/*Stack*/
	Sophon_Stack     *stack;

	/*Global module*/
	Sophon_Module    *glob_module;

	/*Basic object prototypes*/
	Sophon_Value      Object_protov;
	Sophon_Value      Boolean_protov;
	Sophon_Value      Number_protov;
	Sophon_Value      String_protov;
	Sophon_Value      Function_protov;
	Sophon_Value      Array_protov;
	Sophon_Value      Arguments_protov;
	Sophon_Value      RegExp_protov;
	Sophon_Value      Date_protov;

	/*Return value*/
	Sophon_Value      retv;

	/*Exception value*/
	Sophon_Value      excepv;

	/*Errors*/
#define DECL_ERROR(name) Sophon_Value name;
	SOPHON_FOR_EACH_ERROR(DECL_ERROR)

	/*Strings*/
#define DECL_INTERNAL_STRING(name, str) Sophon_String *name##_str;
	SOPHON_FOR_EACH_INTERNAL_STRING(DECL_INTERNAL_STRING)
#define DECL_STRING(name) Sophon_String *name##_str;
	SOPHON_FOR_EACH_STRING(DECL_STRING)

#ifdef SOPHON_DATE
	/*Time zone offset*/
	Sophon_Int        tz_offset;
#endif

#ifdef SOPHON_CONSOLE
	Sophon_Hash       timer_hash;
#endif

	/*Property iterator stack*/
	Sophon_PropIter  *pi_stack;
};

/**\brief Check if the virtual machine has exception*/
#define sophon_has_exception(vm) (!sophon_value_is_undefined(vm->excepv))

/**
 * \brief Create a new virtual machine
 * \return The new virtual machine
 * \retval NULL On error
 */
extern Sophon_VM* sophon_vm_create (void);

/**
 * \brief Release an unused virtual machine
 * \param[in] vm The virtual machine to be freed
 */
extern void       sophon_vm_destroy (Sophon_VM *vm);

/**
 * \brief Throw an exception and message string
 * \param[in] vm The current virtual machine
 * \param excepv The exception value
 * \param[in] msg Error message
 */
extern void       sophon_throw (Sophon_VM *vm, Sophon_Value excepv,
					const char *msg);

/**
 * \brief Throw an exception value
 * \param[in] vm The current virtual machine
 * \param excepv The exception value
 * \param[in] msg Error message
 */
extern void       sophon_throw_value (Sophon_VM *vm, Sophon_Value excepv);

/**
 * \brief Catch the exception
 * \param[in] vm The current virtual machine
 * \param[out] excepv Return the exception value
 * \retval SOPHON_TRUE The exception is catched
 * \retval SOPHON_FALSE No exception is set
 */
extern Sophon_Bool sophon_catch (Sophon_VM *vm, Sophon_Value *excepv);

/**
 * \brief Check if the virtual machine is in strict mode
 * \param[in] vm The current virtual machine
 * \retval SOPHON_TRUE Virtual machine is in strict mode
 * \retval SOPHON_TRUE Virtual machine is not in strict mode
 */
extern Sophon_Bool sophon_strict (Sophon_VM *vm);

/**
 * \brief Trace the stack
 * \param[in] vm The current virtual machine
 */
extern void sophon_trace (Sophon_VM *vm);

#ifdef __cplusplus
}
#endif

#endif

