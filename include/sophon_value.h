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
	#define SOPHON_VALUE_INT_MAX   ((Sophon_Int)0x1FFFFFFF)
	#define SOPHON_VALUE_INT_MIN   ((Sophon_Int)0xE0000000)
#endif

#define SOPHON_VALUE_NULL 0

#define SOPHON_VALUE_TYPE_MASK   3
#define SOPHON_VALUE_TYPE_SHIFT  2
#define SOPHON_VALUE_TYPE_GC     0
#define SOPHON_VALUE_TYPE_CPTR   1
#define SOPHON_VALUE_TYPE_INT    2
#define SOPHON_VALUE_TYPE_NUMBER 3
#define SOPHON_VALUE_TYPE_NULL   4
#define SOPHON_VALUE_TYPE_UNDEFINED 5
#define SOPHON_VALUE_TYPE_BOOL   6

typedef Sophon_Int Sophon_ValueType;

#define SOPHON_MAX_NORM   1.797693134862315708145274237317e+308
#define SOPHON_MIN_NORM   2.2250738585072013830902327173324e-308
#define SOPHON_MAX_DENORM 2.2250738585072008890245868760859e-308
#define SOPHON_MIN_DENORM 4.9406564584124654417656879286822e-324

/**\brief Property descriptor*/
typedef struct {
	Sophon_U8    attrs; /**< Attributes*/
	Sophon_Value value; /**< Data value*/
	Sophon_Value getv;  /**< Accessor get function*/
	Sophon_Value setv;  /**< Accessor set function*/
} Sophon_PropDesc;

/**\brief Property*/
typedef struct Sophon_Property_s Sophon_Property;
/**\brief Accessor property*/
typedef struct Sophon_AccessorProperty_s Sophon_AccessorProperty;

/**\brief Object property for-each function*/
typedef Sophon_Int (*Sophon_PropForEachFunc)(Sophon_VM *vm,
			Sophon_Property *prop,
			Sophon_Ptr data);

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

/**\brief Check if the value is a string*/
#define SOPHON_VALUE_IS_STRING(v)\
	(SOPHON_VALUE_IS_GC(v) &&\
		(SOPHON_VALUE_GET_GC(v)->gc_type == SOPHON_GC_String))

/**\brief Check if the value is a closure*/
#define SOPHON_VALUE_IS_CLOSURE(v)\
	(SOPHON_VALUE_IS_GC(v) &&\
		(SOPHON_VALUE_GET_GC(v)->gc_type == SOPHON_GC_Closure))

/**\brief Check if the value is an array*/
#define SOPHON_VALUE_IS_ARRAY(v)\
	(SOPHON_VALUE_IS_GC(v) &&\
		(SOPHON_VALUE_GET_GC(v)->gc_type == SOPHON_GC_Array))

/**\brief Check if the value is an object*/
#define SOPHON_VALUE_IS_OBJECT(v)\
	(SOPHON_VALUE_IS_GC(v) &&\
		(SOPHON_VALUE_GET_GC(v)->gc_type == SOPHON_GC_Object))

/**\brief Check if the value is a C pointer*/
#define SOPHON_VALUE_IS_CPTR(v) \
	(((((Sophon_Value)(v)) & SOPHON_VALUE_TYPE_MASK) == \
			SOPHON_VALUE_TYPE_CPTR) && \
			!SOPHON_VALUE_IS_BOOL(v))

/**\brief Check if the value is an integer number*/
#define SOPHON_VALUE_IS_INT(v) \
	((((Sophon_Value)(v)) & SOPHON_VALUE_TYPE_MASK) == \
			SOPHON_VALUE_TYPE_INT)

/**\brief Check if the value is a JS number*/
#define SOPHON_VALUE_IS_NUMBER(v) \
	(((((Sophon_Value)(v)) & SOPHON_VALUE_TYPE_MASK) == \
			SOPHON_VALUE_TYPE_NUMBER) && \
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
/**\brief Convert boolean to value*/
#define SOPHON_VALUE_BOOL(b) ((b) ? SOPHON_VALUE_TRUE : SOPHON_VALUE_FALSE)

/**\brief Set a GC managed object value*/
#define sophon_value_set_gc(vm, v, o) \
	(*(Sophon_Value*)(v) = (Sophon_Value)(o))
/**\brief Convert a GC object to value*/
#define SOPHON_VALUE_GC(o) ((Sophon_Value)(o))

/**\brief Set a C pointer value*/
#define sophon_value_set_cptr(vm, v, p)\
	(*(Sophon_Value*)(v) = (((Sophon_Value)(p)) | \
			SOPHON_VALUE_TYPE_CPTR))
/**\brief Convert a C pointer object to value*/
#define SOPHON_VALUE_CPTR(p) (((Sophon_Value)(p)) | SOPHON_VALUE_TYPE_CPTR)

#ifdef SOPHON_64BITS
#define sophon_value_set_int(vm, v, i) \
	(*(Sophon_Value*)(v) = (((Sophon_Value)(i)) << \
			SOPHON_VALUE_TYPE_SHIFT) | \
			SOPHON_VALUE_TYPE_INT)
#else /*!defined(SOPHON_64BITS)*/
/**\brief Set a integer number value*/
#define sophon_value_set_int(vm, v, i) \
	SOPHON_MACRO_BEGIN \
		if ((((Sophon_Int)(i)) < SOPHON_VALUE_INT_MIN) || \
				(((Sophon_Int)(i)) > SOPHON_VALUE_INT_MAX))\
			sophon_value_set_number_real(vm, v,\
						(Sophon_Number)(Sophon_Int)(i));\
		else\
			(*(Sophon_Value*)(v) = (((Sophon_Value)(i)) << \
					SOPHON_VALUE_TYPE_SHIFT) | \
					SOPHON_VALUE_TYPE_INT); \
	SOPHON_MACRO_END
#endif /*SOPHON_64BITS*/

/**\brief Set a JS number value*/
#define sophon_value_set_number(vm, v, d) \
	SOPHON_MACRO_BEGIN \
		if (SOPHON_ABS(d) < SOPHON_MIN_DENORM)\
			sophon_value_set_int(vm, v, 0); \
		else if ((d) == (Sophon_Number)(Sophon_Int)(d)) \
			sophon_value_set_int(vm, v, (Sophon_Int)(d)); \
		else \
			sophon_value_set_number_real(vm, v, d); \
	SOPHON_MACRO_END

/**\brief Get a boolean from the value*/
#define SOPHON_VALUE_GET_BOOL(v) \
	((((Sophon_Value)(v)) == SOPHON_VALUE_TRUE) ? SOPHON_TRUE : SOPHON_FALSE)

/**\brief Get a GC object from the value*/
#define SOPHON_VALUE_GET_GC(v)   ((Sophon_GCObject*)(v))
/**\brief Get a string from the value*/
#define SOPHON_VALUE_GET_STRING(v) ((Sophon_String*)(v))
/**\brief Get an object from the value*/
#define SOPHON_VALUE_GET_OBJECT(v) ((Sophon_Object*)(v))

/**\brief Get an integer number from a value*/
#define SOPHON_VALUE_GET_INT(v) \
	SOPHON_SHR(((Sophon_IntPtr)(v)), SOPHON_VALUE_TYPE_SHIFT)

/**\brief Get a JS number from a value*/
#define SOPHON_VALUE_GET_NUMBER(v) \
	(*(Sophon_Number*)(((Sophon_Value)(v)) & ~SOPHON_VALUE_TYPE_MASK))

/**\brief Get a c pointer from a value*/
#define SOPHON_VALUE_GET_CPTR(v) \
	((Sophon_Ptr)(((Sophon_Value)(v)) & ~SOPHON_VALUE_TYPE_MASK))

#define sophon_value_mark(vm, v) \
	SOPHON_MACRO_BEGIN \
		if (SOPHON_VALUE_IS_NUMBER(v)) \
			sophon_number_mark(vm, v); \
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
	else if (SOPHON_VALUE_IS_NUMBER(v))
		return SOPHON_VALUE_TYPE_NUMBER;
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
 * \param flags Invoke flags
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_value_call (Sophon_VM *vm, Sophon_Value callv,
						Sophon_Value thisv, Sophon_Value *argv, Sophon_Int argc,
						Sophon_Value *retv, Sophon_U32 flags);

/**
 * \brief Call the value's property
 * \param[in] vm The current virtual machine
 * \param thisv This value
 * \param namev The property name
 * \param[in] argv The arguments
 * \param argc arguments count
 * \param [out] retv The return value
 * \param flags Invoke flags
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_value_call_prop (Sophon_VM *vm, Sophon_Value thisv,
						Sophon_Value namev, Sophon_Value *argv, Sophon_Int argc,
						Sophon_Value *retv, Sophon_U32 flags);

/**
 * \brief Get the class object of the value
 * \param[in] vm The current virtual machine
 * \param v The value
 * \return The class object
 * \retval NULL If v is null or undefined
 */
extern Sophon_Object* sophon_value_get_class (Sophon_VM *vm, Sophon_Value v);

/**
 * \brief Get the object from the value
 * \param[in] vm The current virtual machine
 * \param v The value
 * \param [out] obj Return the object
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_value_to_object (Sophon_VM *vm, Sophon_Value v,
						Sophon_Object **obj);

/**
 * \brief Get primitive GC object from the object
 * \param[in] vm The current virtual machine
 * \param v The value
 * \param gc_type The primitive type's GC type
 * \param[out] gc_obj Return the primitive data
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_value_to_gc (Sophon_VM *vm, Sophon_Value v,
						Sophon_U8 gc_type, Sophon_GCObject **gc_obj);

/**\brief Get closure data from a value*/
#define sophon_value_to_closure(vm, v, clos)\
	sophon_value_to_gc(vm, v, SOPHON_GC_Closure, (Sophon_GCObject**)(clos))

/**\brief Get array data from a value*/
#define sophon_value_to_array(vm, v, arr)\
	sophon_value_to_gc(vm, v, SOPHON_GC_Array, (Sophon_GCObject**)(arr))

/**\brief Get regular expression data from a value*/
#define sophon_value_to_regexp(vm, v, re)\
	sophon_value_to_gc(vm, v, SOPHON_GC_RegExp, (Sophon_GCObject**)(re))

/**
 * \brief Get the value own property descriptor
 * \param[in] vm The current virtual machine
 * \param v The value
 * \param namev The property's name
 * \param[out] desc The property's descriptor
 * \retval SOPHON_OK On success
 * \retval SOPHON_NONE The property do not have this property
 */
extern Sophon_Result sophon_value_prop_desc (Sophon_VM *vm,
						Sophon_Value v,
						Sophon_Value namev,
						Sophon_PropDesc *desc);

/**
 * \brief Add a new property to the value
 * \param[in] vm The current virtual machine
 * \param v The value
 * \param namev The property's name
 * \param getv The get or value attribute
 * \param setv The set attribute
 * \param attrs Property's attributes
 * \param flags Invoke flags
 * \retval SOPHON_OK On success
 * \retval SOPHON_NONE The property do not have this property
 */
extern Sophon_Result sophon_value_define_prop (Sophon_VM *vm,
						Sophon_Value v,
						Sophon_Value namev,
						Sophon_Value getv,
						Sophon_Value setv,
						Sophon_U8 attrs,
						Sophon_U32 flags);

/**
 * \brief Delete a property from the value
 * \param[in] vm The current virtual machine
 * \param v The value
 * \param namev The property's name
 * \param flags Invoke flags
 */
extern Sophon_Result  sophon_value_delete_prop (Sophon_VM *vm,
						Sophon_Value v,
						Sophon_Value namev,
						Sophon_U32 flags);

/**
 * \brief For each properties of the value
 * \param[in] vm The current virtual machine
 * \param v The value
 * \param func For each function
 * \param data User defined function argument
 * \return The return value of the function
 */
extern Sophon_Int     sophon_value_for_each_prop(Sophon_VM *vm,
						Sophon_Value v,
						Sophon_PropForEachFunc func,
						Sophon_Ptr data);

/**
 * \brief Get a value's property value
 * \param[in] vm The current virtual machine
 * \param v The value
 * \param namev The property's name
 * \param[out] getv Return the property's value
 * \param flags Invoke flags
 * \retval SOPHON_OK On success
 * \retval SOPHON_NONE The value has not the property
 * \retval <0 On error
 */
extern Sophon_Result sophon_value_get (Sophon_VM *vm, Sophon_Value v,
						Sophon_Value namev, Sophon_Value *getv,
						Sophon_U32 flags);

/**
 * \brief Set a value's property value
 * \param[in] vm The current virtual machine
 * \param v The value
 * \param namev The property's name
 * \param setv The property's new value
 * \param flags Invoke flags
 * \retval SOPHON_OK On success
 * \retval SOPHON_NONE The value has not the property
 * \retval <0 On error
 */
extern Sophon_Result sophon_value_put (Sophon_VM *vm, Sophon_Value v,
						Sophon_Value namev, Sophon_Value setv,
						Sophon_U32 flags);

/**
 * \brief Convert a value to boolean
 * \param[in] vm The current virtual machine
 * \param v The value
 * \return The boolean value
 */
extern Sophon_Bool   sophon_value_to_bool (Sophon_VM *vm, Sophon_Value v);

/**
 * \brief Convert a value to a JS number
 * \param[in] vm The current virtual machine
 * \param v The value
 * \param[out] pd Return the JS number
 * \retval SOPHON_SUCCESS On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_value_to_number (Sophon_VM *vm, Sophon_Value v,
						Sophon_Number *pd);

/**
 * \brief Convert a value to string
 * \param[in] vm The current virtual machine
 * \param v The value
 * \param[out] pstr Return the string
 * \retval SOPHON_SUCCESS On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_value_to_string (Sophon_VM *vm, Sophon_Value v,
						Sophon_String **pstr);

/**
 * \brief Convert a value to string
 * \param[in] vm The current virtual machine
 * \param v The value
 * \param quot Add quotation marks around string
 * \param[out] pstr Return the string
 * \retval SOPHON_SUCCESS On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_value_to_string_ex (Sophon_VM *vm, Sophon_Value v,
						Sophon_Bool quot, Sophon_String **pstr);


#define SOPHON_PRIM_NUMBER 0
#define SOPHON_PRIM_STRING 1
#define SOPHON_PRIM_UNDEF  2

/**
 * \brief Convert a value to primitive type
 * \param[in] vm The current virtual machine
 * \param v The value
 * \param[out] retv Return the primitive type value
 * \param hint Hint value type
 * \retval SOPHON_SUCCESS On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_value_to_prim (Sophon_VM *vm, Sophon_Value v,
						Sophon_Value *retv, Sophon_Int hint);

/**
 * \brief Convert a value to integer number
 * \param[in] vm The current virtual machine
 * \param v The value
 * \param[out] pi Return the integer number
 * \retval SOPHON_SUCCESS On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_value_to_int (Sophon_VM *vm, Sophon_Value v,
						Sophon_Int *pi);

/**
 * \brief Convert a value to 32bits signed integer number
 * \param[in] vm The current virtual machine
 * \param v The value
 * \param[out] pi Return the integer number
 * \retval SOPHON_SUCCESS On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_value_to_s32 (Sophon_VM *vm, Sophon_Value v,
						Sophon_S32 *pi);

/**
 * \brief Convert a value to 32bits unsigned integer number
 * \param[in] vm The current virtual machine
 * \param v The value
 * \param[out] pi Return the integer number
 * \retval SOPHON_SUCCESS On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_value_to_u32 (Sophon_VM *vm, Sophon_Value v,
						Sophon_U32 *pi);

/**
 * \brief Convert a value to 16bits unsigned integer number
 * \param[in] vm The current virtual machine
 * \param v The value
 * \param[out] pi Return the integer number
 * \retval SOPHON_SUCCESS On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_value_to_u16 (Sophon_VM *vm, Sophon_Value v,
						Sophon_U16 *pi);

/**
 * \brief Compare 2 value
 * \param[in] vm The current virtual machine
 * \param lv The left value
 * \param rv The right value
 * \param[out] ri The compare result
 * If r == -2, undefined.
 * If r == -1, means lv < rv.
 * If r == 1, means lv > rv.
 * If r == 0, means lv == rv.
 * \retval SOPHON_OK On success
 * \retval <0 On error
 * When lv or rv is NaN, throw TypeError and return SOPHON_ERR_THROW
 */
extern Sophon_Result sophon_value_compare (Sophon_VM *vm, Sophon_Value lv,
						Sophon_Value rv, Sophon_Int *ri);

/**
 * \brief Check if 2 value are equal
 * \param[in] vm The current virtual machine
 * \param lv The left value
 * \param rv The right value
 * \param[out] pb Return if 2 value are equal
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_value_equal_compare (Sophon_VM *vm, Sophon_Value lv,
						Sophon_Value rv, Sophon_Bool *pb);

/**
 * \brief Check if 2 value are same
 * \param[in] vm The current virtual machine
 * \param lv The left value
 * \param rv The right value
 * \retval SOPHON_TRUE lv and rv are same
 * \retval SOPHON_FALSE lv and rv are not same
 */
extern Sophon_Bool   sophon_value_same (Sophon_VM *vm, Sophon_Value lv,
						Sophon_Value rv);

/**
 * \brief Check if lv is one of rv properties' name
 * \param[in] vm The current virtual machine
 * \param lv The left value
 * \param rv The right value
 * \param[out] pb The check result
 * SOPHON_TRUE when lv is a rv's property name
 * SOPHON_FALSE lv when is not a rv's property name
 * \retval SOPHON_OK On success
 * \retval <0 On error
 * If rv is not an object, throw TypeError and return SOPHON_ERR_THROW
 */
extern Sophon_Result sophon_value_in (Sophon_VM *vm, Sophon_Value lv,
						Sophon_Value rv, Sophon_Bool *pb);

/**
 * \brief Check if lv is an instance of rv
 * \param[in] vm The current virtual machine
 * \param lv The left value
 * \param rv The right value
 * \param[out] pb The check result
 * SOPHON_TRUE when lv is an instance of rv
 * SOPHON_FALSE lv when is an instance of rv
 * \retval SOPHON_OK On success
 * \retval <0 On error
 * If rv is not a constructor, throw TypeError and return SOPHON_ERR_THROW
 */
extern Sophon_Result sophon_value_instanceof (Sophon_VM *vm, Sophon_Value lv,
						Sophon_Value rv, Sophon_Bool *pb);

/**
 * \brief Invoke a constructot to create a new object
 * \param[in] vm The current virtual machine
 * \param cv The constructor value
 * \param[in] argv The arguments
 * \param argc The arguments count
 * \param[out] objv Return the new object
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_value_new (Sophon_VM *vm, Sophon_Value cv,
						Sophon_Value *argv, Sophon_Int argc,
						Sophon_Value *objv);

/**
 * \brief Get the value's type name string
 * \param[in] vm The current virtual machine
 * \param v The value
 * \return The type name
 */
extern Sophon_String* sophon_value_typeof (Sophon_VM *vm, Sophon_Value v);

/**
 * \brief Begin a for..in... loop
 * \param[in] vm The current virtual machine
 * \param end_ip Loop end instruction pointer
 * \retval SOPHON_OK On success
 * \retval SOPHON_NONE For loop end because break/continue/return
 * \retval <0 On error
 */
extern Sophon_Result  sophon_value_for_in (Sophon_VM *vm, Sophon_U16 end_ip);

/**\brief Check if the value is undefined*/
#define sophon_value_is_undefined(v)\
	SOPHON_VALUE_IS_UNDEFINED(v)

/**\brief Check if the value is null*/
#define sophon_value_is_null(v)\
	SOPHON_VALUE_IS_NULL(v)

/**
 * \brief Check if a value is a boolean value
 * \param v The value
 * \retval SOPHON_TRUE The value is a string
 * \retval SOPHON_FALSE The value is not a string
 */
extern Sophon_Bool   sophon_value_is_bool (Sophon_Value v);

/**
 * \brief Check if a value is a number value
 * \param v The value
 * \retval SOPHON_TRUE The value is a string
 * \retval SOPHON_FALSE The value is not a string
 */
extern Sophon_Bool   sophon_value_is_number (Sophon_Value v);

/**
 * \brief Check if a value is an GC object
 * \param v The value
 * \param type GC object type
 * \retval SOPHON_TRUE The value is a string
 * \retval SOPHON_FALSE The value is not a string
 */
extern Sophon_Bool   sophon_value_is_prim (Sophon_Value v, Sophon_U8 type);

/**\brief Check if the value is a string*/
#define sophon_value_is_string(v)\
	sophon_value_is_prim(v, SOPHON_GC_String)
/**\brief Check if the value is a closure*/
#define sophon_value_is_closure(v)\
	sophon_value_is_prim(v, SOPHON_GC_Closure)
/**\brief Check if the value is an array*/
#define sophon_value_is_array(v)\
	sophon_value_is_prim(v, SOPHON_GC_Array)
/**\brief Check if the value is a regular expression object*/
#define sophon_value_is_regexp(v)\
	sophon_value_is_prim(v, SOPHON_GC_RegExp)

/**
 * \brief Check if a value is an object
 * \param v The value
 * \retval SOPHON_TRUE The value is an object
 * \retval SOPHON_FALSE The value is not an object
 */
extern Sophon_Bool   sophon_value_is_object (Sophon_Value v);

/**\brief Set a value as string*/
#define sophon_value_set_string(vm, v, str)\
	sophon_value_set_gc(vm, v, (Sophon_GCObject*)(str))

/**\brief Set a value as object*/
#define sophon_value_set_object(vm, v, obj)\
	sophon_value_set_gc(vm, v, (Sophon_GCObject*)(obj))

/**\brief Set a value as closure*/
#define sophon_value_set_closure(vm, v, c)\
	sophon_value_set_gc(vm, v, (Sophon_GCObject*)(c))

/**\brief Set a value as array*/
#define sophon_value_set_array(vm, v, arr)\
	sophon_value_set_gc(vm, v, (Sophon_GCObject*)(arr))

/**\brief Set a value as regular expression*/
#define sophon_value_set_regexp(vm, v, re)\
	sophon_value_set_gc(vm, v, (Sophon_GCObject*)(re))


/**
 * \brief Dump a value to the output
 * \param[in] vm The current virtual machine
 * \param v The value
 */
extern void          sophon_value_dump (Sophon_VM *vm, Sophon_Value v);

#ifdef __cplusplus
}
#endif

#endif

