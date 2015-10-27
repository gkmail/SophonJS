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

#ifndef _SOPHON_DECL_H_
#define _SOPHON_DECL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sophon_types.h"
#include "sophon_function.h"

/**\brief Property declaration*/
typedef struct Sophon_Decl_s       Sophon_Decl;

/**\brief Delcaration value type*/
typedef enum {
	SOPHON_DECL_UNDEFINED,  /**< undefined*/
	SOPHON_DECL_NULL,       /**< null*/
	SOPHON_DECL_BOOL,       /**< Boolean value*/
	SOPHON_DECL_INT,        /**< Integer number*/
	SOPHON_DECL_DOUBLE,     /**< Double precision number*/
	SOPHON_DECL_STRING,     /**< String*/
	SOPHON_DECL_FUNCTION,   /**< Function*/
	SOPHON_DECL_ACCESSOR,   /**< Accessor*/
	SOPHON_DECL_OBJECT      /**< Object*/
} Sophon_DeclType;

/**\brief Property declaration*/
struct Sophon_Decl_s {
	const char       *name;  /**< Property name*/
	Sophon_DeclType   type;  /**< Value type*/
	Sophon_U32        attrs; /**< Property attributes*/
#ifdef SOPHON_HAVE_GNUC_EXT
	union {
		Sophon_Bool   b;     /**< Boolean value*/
		Sophon_Int    i;     /**< Integer number*/
		Sophon_Double d;     /**< Double precision number*/
		const char   *str;   /**< String*/
		struct {
			Sophon_NativeFunc func; /**< Function*/
			Sophon_U8         argc; /**< Arguments count*/
		} func;                     /**< Function*/
		struct {
			Sophon_NativeFunc  get; /**< Accessor get function*/
			Sophon_NativeFunc  set; /**< Accessor set function*/
		} accessor;                 /**< Accessor*/
		const Sophon_Decl     *obj; /**< Object properties*/
	}p;
#else
	Sophon_UIntPtr    vp1;   /**< Integer parameter 1*/
	Sophon_UIntPtr    vp2;   /**< Integer parameter 2*/
	Sophon_Double     vd;    /**< Double precision number parameter*/
#endif
};

#define SOPHON_UNDEFINED_PROP(name, attrs)\
	{#name, SOPHON_DECL_UNDEFINED, attrs}
#define SOPHON_NULL_PROP(name, attrs)\
	{#name, SOPHON_DECL_NULL, attrs}

#ifdef SOPHON_HAVE_GNUC_EXT
#define SOPHON_BOOL_PROP(name, attrs, v)\
	{#name, SOPHON_DECL_BOOL, attrs, {b: v}}
#define SOPHON_INT_PROP(name, attrs, v)\
	{#name, SOPHON_DECL_INT, attrs, {i: v}}
#define SOPHON_DOUBLE_PROP(name, attrs, v)\
	{#name, SOPHON_DECL_DOUBLE, attrs, {d: v}}
#define SOPHON_STRING_PROP(name, attrs, v)\
	{#name, SOPHON_DECL_STRING, attrs, {str: v}}
#define SOPHON_FUNCTION_PROP(name, attrs, v, a)\
	{#name, SOPHON_DECL_FUNCTION, attrs, {func: {func:v, argc:a}}}
#define SOPHON_ACCESSOR_PROP(name, attrs, getv, setv)\
	{#name, SOPHON_DECL_ACCESSOR, attrs, {accessor:{getv, setv}}}
#define SOPHON_OBJECT_PROP(name, attrs, v)\
	{#name, SOPHON_DECL_OBJECT, attrs, {obj: v}}

#define SOPHON_DECL_GET_BOOL(decl)     ((decl)->p.b)
#define SOPHON_DECL_GET_INT(decl)      ((decl)->p.i)
#define SOPHON_DECL_GET_DOUBLE(decl)   ((decl)->p.d)
#define SOPHON_DECL_GET_STRING(decl)   ((decl)->p.str)
#define SOPHON_DECL_GET_FUNCTION(decl) ((decl)->p.func.func)
#define SOPHON_DECL_GET_ARGC(decl)     ((decl)->p.func.argc)
#define SOPHON_DECL_GET_OBJECT(decl)   ((decl)->p.obj)
#define SOPHON_DECL_GET_GET(decl)      ((decl)->p.accessor.get)
#define SOPHON_DECL_GET_SET(decl)      ((decl)->p.accessor.set)

#else /*!defined(SOPHON_HAVE_GNUC_EXT)*/
#define SOPHON_BOOL_PROP(name, attrs, v)\
	{#name, SOPHON_DECL_BOOL, attrs, (Sophon_UIntPtr)(v)}
#define SOPHON_INT_PROP(name, attrs, v)\
	{#name, SOPHON_DECL_INT, attrs, (Sophon_UIntPtr)(v)}
#define SOPHON_DOUBLE_PROP(name, attrs, v)\
	{#name, SOPHON_DECL_DOUBLE, attrs, 0, 0, (Sophon_Double)(v)}
#define SOPHON_STRING_PROP(name, attrs, v)\
	{#name, SOPHON_DECL_STRING, attrs, (Sophon_UIntPtr)(v)}
#define SOPHON_FUNCTION_PROP(name, attrs, v, a)\
	{#name, SOPHON_DECL_FUNCTION, attrs, (Sophon_UIntPtr)(v),\
		(Sophon_UIntPtr)(a)}
#define SOPHON_ACCESSOR_PROP(name, attrs, getv, setv)\
	{#name, SOPHON_DECL_ACCESSOR, attrs, (Sophon_UIntPtr)(getv),\
		(Sophon_UIntPtr)(setv)}
#define SOPHON_OBJECT_PROP(name, attrs, v)\
	{#name, SOPHON_DECL_OBJECT, attrs, (Sophon_UIntPtr)(v)}

#define SOPHON_DECL_GET_BOOL(decl)     ((Sophon_Bool)((decl)->vp1))
#define SOPHON_DECL_GET_INT(decl)      ((Sophon_Int)((decl)->vp1))
#define SOPHON_DECL_GET_DOUBLE(decl)   ((Sophon_Double)((decl)->vp1))
#define SOPHON_DECL_GET_STRING(decl)   ((const char*)((decl)->vp1))
#define SOPHON_DECL_GET_FUNCTION(decl) ((Sophon_NativeFunc)((decl)->vp1))
#define SOPHON_DECL_GET_ARGC(decl)     ((Sophon_U8)((decl)->vp2))
#define SOPHON_DECL_GET_OBJECT(decl)   ((const Sophon_Decl*)((decl)->vp1))
#define SOPHON_DECL_GET_GET(decl)      ((Sophon_NativeFunc)((decl)->vp1))
#define SOPHON_DECL_GET_SET(decl)      ((Sophon_NativeFunc)((decl)->vp2))

#endif /*SOPHON_HAVE_GNUC_EXT*/

#define SOPHON_FUNC(name)\
Sophon_Result name (Sophon_VM *vm, Sophon_Value thisv, Sophon_Value *argv,\
			Sophon_Int argc, Sophon_Value *retv)

#define SOPHON_ARG(id) (((id) >= argc) ? SOPHON_VALUE_UNDEFINED : argv[id])

/**
 * \brief Load property declarations to the object
 * \param[in] vm The current virtual machine
 * \param[in] mod The module contains the object
 * \param[in] obj The object to load the properties
 * \param[in] decls Declarations
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_decl_load (Sophon_VM *vm, Sophon_Module *mod,
						Sophon_Object *obj,
						const Sophon_Decl *decls);

/**
 * \brief Load a native function and create an closure
 * \param[in] vm The current virtual machine
 * \param[in] mod The module contains the function
 * \param[in] name The name of the function
 * \param func The native function
 * \param argc Arguments count
 * \return The new closure
 */
extern Sophon_Closure* sophon_closure_load (Sophon_VM *vm, Sophon_Module *mod,
						Sophon_String *name, Sophon_NativeFunc func,
						Sophon_U8 argc);

/**
 * \brief Initialize the global object
 * \param[in] vm The current virtual machine
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_global_init (Sophon_VM *vm);

#ifdef __cplusplus
}
#endif

#endif

