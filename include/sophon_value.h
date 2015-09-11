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

#ifndef _SOPHON_VALUE_H_
#define _SOPHON_VALUE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sophon_types.h"

#ifdef SOPHON_64BITS
	#define SOPHON_VALUE_UNDEFINED 0xFFFFFFFFFFFFFFFFull
	#define SOPHON_VALUE_TRUE      0xFFFFFFFFFFFFFFFDull
	#define SOPHON_VALUE_FALSE     0xFFFFFFFFFFFFFFF9ull
#else
	#define SOPHON_VALUE_UNDEFINED 0xFFFFFFFF
	#define SOPHON_VALUE_TRUE      0xFFFFFFFD
	#define SOPHON_VALUE_FALSE     0xFFFFFFF9
	#define SOPHON_VALUE_INT_MAX   ((Sophon_Int)0x3FFFFFFF)
	#define SOPHON_VALUE_INT_MIN   ((Sophon_Int)0xE0000000)
#endif

#define SOPHON_VALUE_NULL 0

#define SOPHON_VALUE_TYPE_MASK   3
#define SOPHON_VALUE_TYPE_SHIFT  2
#define SOPHON_VALUE_TYPE_GC     0
#define SOPHON_VALUE_TYPE_CPTR   1
#define SOPHON_VALUE_TYPE_INT    2
#define SOPHON_VALUE_TYPE_DOUBLE 3
#define SOPHON_VALUE_TYPE_NULL   4
#define SOPHON_VALUE_TYPE_UNDEFINED 5
#define SOPHON_VALUE_TYPE_BOOL   6

typedef Sophon_Int Sophon_ValueType;

/**\brief Check if the value is undefined*/
#define SOPHON_VALUE_IS_UNDEFINED(v) \
	(((Sophon_Value)(v)) == SOPHON_VALUE_UNDEFINED)

/**\brief Check if the value is null*/
#define SOPHON_VALUE_IS_NULL(v) \
	(((Sophon_Value)(v)) == SOPHON_VALUE_NULL)

/**\brief Check if the value is boolean*/
#define SOPHON_VALUE_IS_BOOL(v) \
	((((Sophon_Value)(v)) == SOPHON_VALUE_TRUE) || \
			(((Sophon_Value)(v)) == SOPHON_VALUE_FALSE))

/**\brief Check if the value is a GC managed object*/
#define SOPHON_VALUE_IS_GC(v) \
	(((((Sophon_Value)(v)) & SOPHON_VALUE_TYPE_MASK) == \
			SOPHON_VALUE_TYPE_GC) && \
			!SOPHON_VALUE_IS_NULL(v))

/**\brief Check if the value is a C pointer*/
#define SOPHON_VALUE_IS_CPTR(v) \
	(((((Sophon_Value)(v)) & SOPHON_VALUE_TYPE_MASK) == \
			SOPHON_VALUE_TYPE_CPTR) && \
			!SOPHON_VALUE_IS_BOOL(v))

/**\brief Check if the value is an integer number*/
#define SOPHON_VALUE_IS_INT(v) \
	((((Sophon_Value)(v)) & SOPHON_VALUE_TYPE_MASK) == \
			SOPHON_VALUE_TYPE_INT)

/**\brief Check if the value is a double precision number*/
#define SOPHON_VALUE_IS_DOUBLE(v) \
	(((((Sophon_Value)(v)) & SOPHON_VALUE_TYPE_MASK) == \
			SOPHON_VALUE_TYPE_DOUBLE) && \
			!SOPHON_VALUE_IS_UNDEFINED(v))

/**\brief Set the value as undefined*/
#define sophon_value_set_undefined(vm, v) \
	(*(Sophon_Value*)(v) = SOPHON_VALUE_UNDEFINED)

/**\brief Set the value as null*/
#define sophon_value_set_null(vm, v) \
	(*(Sophon_Value*)(v) = SOPHON_VALUE_NULL)

/**\brief Set a boolean value*/
#define sophon_value_set_bool(vm, v, b) \
	(*(Sophon_Value*)(v) = (b) ? SOPHON_VALUE_TRUE : SOPHON_VALUE_FALSE)

/**\brief Set a GC managed object value*/
#define sophon_value_set_gc(vm, v, o) \
	(*(Sophon_Value*)(v) = (Sophon_Value)(o))

/**\brief Set a C pointer value*/
#define sophon_value_set_cptr(vm, v, p)\
	(*(Sophon_Value*)(v) = (((Sophon_Value)(p)) << \
			SOPHON_VALUE_TYPE_SHIFT) | \
			SOPHON_VALUE_TYPE_CPTR)

#ifdef SOPHON_64BITS
#define sophon_value_set_int(vm, v, i) \
	((*(Sophon_Value*)(v) = (((Sophon_Value)(i)) << \
			SOPHON_VALUE_TYPE_SHIFT) | \
			SOPHON_VALUE_TYPE_INT))
#else /*!defined(SOPHON_64BITS)*/
/**\brief Set a integer number value*/
#define sophon_value_set_int(vm, v, i) \
	SOPHON_MACRO_BEGIN \
		if ((((Sophon_Int)(i)) < SOPHON_VALUE_INT_MIN) || \
				(((Sophon_Int)(i)) > SOPHON_VALUE_INT_MAX)) \
			sophon_value_set_double_real(vm, v,\
						(Sophon_Double)(Sophon_Int)(i));\
		else \
			(*(Sophon_Value*)(v) = (((Sophon_Value)(i)) << \
					SOPHON_VALUE_TYPE_SHIFT) | \
					SOPHON_VALUE_TYPE_INT); \
	SOPHON_MACRO_END
#endif /*SOPHON_64BITS*/

/**\brief Set a double precision number value*/
#define sophon_value_set_double(vm, v, d) \
	SOPHON_MACRO_BEGIN \
		if ((d) == (Sophon_Double)(Sophon_Int)(d)) \
			sophon_value_set_int(vm, v, (Sophon_Int)(d)); \
		else \
			sophon_value_set_double_real(vm, v, d); \
	SOPHON_MACRO_END

/**\brief Get a boolean from the value*/
#define SOPHON_VALUE_GET_BOOL(v) \
	((((Sophon_Value)(v)) == SOPHON_VALUE_TRUE) ? SOPHON_TRUE : SOPHON_FALSE)

/**\brief Get a GC object fron the value*/
#define SOPHON_VALUE_GET_GC(v)   ((Sophon_GCObject*)(v))

/**\brief Get an integer number from a value*/
#define SOPHON_VALUE_GET_INT(v) \
	SOPHON_SHR(((Sophon_IntPtr)(v)), SOPHON_VALUE_TYPE_SHIFT)

/**\brief Get a double precision number from a value*/
#define SOPHON_VALUE_GET_DOUBLE(v) \
	(*(Sophon_Double*)(((Sophon_Value)(v)) & ~SOPHON_VALUE_TYPE_MASK))

#define sophon_value_mark(vm, v) \
	SOPHON_MACRO_BEGIN \
		if (SOPHON_VALUE_IS_DOUBLE(v)) \
			sophon_double_mark(vm, v); \
		else if (SOPHON_VALUE_IS_GC(v)) \
			gc_mark(vm, (Sophon_GCObject*)(v)); \
	SOPHON_MACRO_END

/**
 * \brief Get the value type
 * \param v The value
 * \return The value type
 */
static SOPHON_INLINE Sophon_ValueType
sophon_value_get_type(Sophon_Value v)
{
	if (SOPHON_VALUE_IS_UNDEFINED(v))
		return SOPHON_VALUE_TYPE_UNDEFINED;
	else if (SOPHON_VALUE_IS_NULL(v))
		return SOPHON_VALUE_TYPE_NULL;
	else if (SOPHON_VALUE_IS_BOOL(v))
		return SOPHON_VALUE_TYPE_BOOL;
	else if (SOPHON_VALUE_IS_INT(v))
		return SOPHON_VALUE_TYPE_INT;
	else if (SOPHON_VALUE_IS_DOUBLE(v))
		return SOPHON_VALUE_TYPE_DOUBLE;
	else if (SOPHON_VALUE_IS_GC(v))
		return SOPHON_VALUE_TYPE_GC;

	return SOPHON_VALUE_TYPE_UNDEFINED;
}

/**
 * \brief Call the value
 * \param[in] vm The current virtual machine
 * \param callv The value be called
 * \param thisv This value
 * \param[in] argv The arguments
 * \param argc arguments count
 * \param [out] retv The return value
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_value_call (Sophon_VM *vm, Sophon_Value callv,
						Sophon_Value thisv, Sophon_Value *argv, Sophon_Int argc,
						Sophon_Value *retv);

/**
 * \brief Get the object from the value
 * \param[in] vm The current virtual machine
 * \param v The value
 * \param [out] obj Return the object
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_value_get_object (Sophon_VM *vm, Sophon_Value v,
						Sophon_Object **obj);

/**
 * \brief Get the closure data from the value
 * \param[in] vm The current virtual machine
 * \param v The value
 * \param [out] clos Return the closure
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_value_get_closure (Sophon_VM *vm, Sophon_Value v,
						Sophon_Closure **clos);

/**
 * \brief Get the array data from the value
 * \param[in] vm The current virtual machine
 * \param v The value
 * \param [out] arr Return the array
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_value_get_array (Sophon_VM *vm, Sophon_Value v,
						Sophon_Array **arr);


#ifdef __cplusplus
}
#endif

#endif

