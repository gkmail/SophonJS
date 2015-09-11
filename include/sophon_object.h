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

#ifndef _SOPHON_OBJECT_H_
#define _SOPHON_OBJECT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sophon_types.h"
#include "sophon_gc.h"
#include "sophon_value.h"

/**\brief Property*/
typedef struct Sophon_Property_s Sophon_Property;

/**\brief The property is writable*/
#define SOPHON_PROP_ATTR_WRITABLE     1
/**\brief The property is enumerable*/
#define SOPHON_PROP_ATTR_ENUMERABLE   2
/**\brief The property is configurable*/
#define SOPHON_PROP_ATTR_CONFIGURABLE 4
/**\brief The data property's value */
#define SOPHON_PROP_ATTR_ACCESSOR     8

/**\brief Default property attributes*/
#define SOPHON_DATA_PROP_ATTR\
		(SOPHON_PROP_ATTR_WRITABLE|\
				SOPHON_PROP_ATTR_ENUMERABLE|\
				SOPHON_PROP_ATTR_CONFIGURABLE)
/**\brief Default accessor property attributes*/
#define SOPHON_ACCESSOR_PROP_ATTR\
		(SOPHON_PROP_ATTR_WRITABLE|\
				SOPHON_PROP_ATTR_ENUMERABLE|\
				SOPHON_PROP_ATTR_CONFIGURABLE|\
				SOPHON_PROP_ATTR_ACCESSOR)

/**\brief The 'writable' attribute is set*/
#define SOPHON_PROP_HAVE_WRITABLE     1
/**\brief The 'enumerable' attribute is set*/
#define SOPHON_PROP_HAVE_ENUMERABLE   2
/**\brief The 'configurable' attribute is set*/
#define SOPHON_PROP_HAVE_CONFIGURABLE 4
/**\brief The 'value' attribute is set*/
#define SOPHON_PROP_HAVE_VALUE        8
/**\brief The 'set' attribute is set*/
#define SOPHON_PROP_HAVE_GET          16
/**\brief The 'get' attribute is set*/
#define SOPHON_PROP_HAVE_SET          32

/**\brief Default property attributes flags*/
#define SOPHON_DATA_PROP_FLAGS\
		(SOPHON_PROP_HAVE_WRITABLE|\
				SOPHON_PROP_HAVE_ENUMERABLE|\
				SOPHON_PROP_HAVE_CONFIGURABLE|\
				SOPHON_PROP_HAVE_VALUE)
/**\brief Default accessor property attributes flags*/
#define SOPHON_ACCESSOR_PROP_FLAGS\
		(SOPHON_PROP_HAVE_WRITABLE|\
				SOPHON_PROP_HAVE_ENUMERABLE|\
				SOPHON_PROP_HAVE_CONFIGURABLE|\
				SOPHON_PROP_HAVE_GET|\
				SOPHON_PROP_HAVE_SET)


/**\brief Property*/
struct Sophon_Property_s {
	Sophon_Property *next;  /**< The next property in the list*/
	Sophon_String   *name;  /**< The property's name*/
	Sophon_U32       attrs; /**< The property's attributes*/
	Sophon_Value     value; /**< The data property's value*/
	Sophon_Value     getv;  /**< The get function value*/
	Sophon_Value     setv;  /**< The set function value*/
};

/**\brief Accessor Property*/
typedef struct {
	Sophon_Property *next;  /**< The next property in the list*/
	Sophon_String   *name;  /**< The property's name*/
	Sophon_U32       attrs; /**< The property's attributes*/
	Sophon_Value     getv;  /**< The get function value*/
	Sophon_Value     setv;  /**< The set function value*/
} Sophon_AccessorProperty;

/**\brief The object is extensible*/
#define SOPHON_OBJ_FL_EXTENSIBLE 1
/**\brief The object is an array*/
#define SOPHON_OBJ_FL_ARRAY      2

/**\brief Object*/
struct Sophon_Object_s {
	SOPHON_GC_HEADER
	Sophon_U16        flags;       /**< The object's flags*/
	Sophon_Property **props;       /**< The property list buffer*/
	Sophon_U16        prop_bucket; /**< The property list count*/
	Sophon_U16        prop_count;  /**< The property count*/
	Sophon_Value      protov;      /**< Prototype value*/
	Sophon_Value      primv;       /**< Primitive value*/
};

/**
 * \brief Initialize an object
 * \param[in] vm The current virtual machine
 * \param[in] obj The object
 */
extern void           sophon_object_init (Sophon_VM *vm,
						Sophon_Object *obj);

/**
 * \brief Create a new object
 * \param[in] vm The current virtual machine
 * \return The created object
 * \retval NULL On error
 */
extern Sophon_Object* sophon_object_create (Sophon_VM *vm);

/**
 * \brief Release an unused object
 * String is managed by GC.
 * You should not invoke this function directly.
 * \param[in] vm The current virtual machine
 * \param[in] obj The object to be freed
 */
extern void           sophon_object_destroy (Sophon_VM *vm,
						Sophon_Object *obj);

/**
 * \brief Get the object's property
 * \param[in] vm The current virtual machine
 * \param[in] obj The object
 * \param[in] name The property's name
 * \return The property
 * \retval NULL If the property is not exist
 */
extern Sophon_Property* sophon_object_get_prop (Sophon_VM *vm,
							Sophon_Object *obj,
							Sophon_String *name);

/**
 * \brief Add a new property to the object
 * \param[in] vm The current virtual machine
 * \param[in] obj The object
 * \param[in] name The property's name
 * \param getv The property's get function value
 * \param setv The property's set function value
 * \param attrs The property's attributes
 * \param flags Data valid flags
 * \param thr Throw exception on error
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result  sophon_object_define_prop (
							Sophon_VM *vm,
							Sophon_Object *obj,
							Sophon_String *name,
							Sophon_Value getv,
							Sophon_Value setv,
							Sophon_U8 attrs,
							Sophon_U8 flags,
							Sophon_Bool thr);

/**
 * \brief Delete an object's property
 * \param[in] vm The current virtual machine
 * \param[in] obj The object
 * \param[in] name The property's name
 * \param thr Throw exception on error
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result  sophon_object_delete_prop (Sophon_VM *vm,
							Sophon_Object *obj,
							Sophon_String *name,
							Sophon_Bool thr);

/**
 * \brief Get an object property's value
 * \param[in] vm The current virtual machine
 * \param[in] obj The object
 * \param thisv Primitive value
 * \param[in] name The property's name
 * \param[out] v The property's value
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_object_get (Sophon_VM *vm,
							Sophon_Object *obj,
							Sophon_Value thisv,
							Sophon_String *name,
							Sophon_Value *v);

/**
 * \brief Set an object property's value
 * \param[in] vm The current virtual machine
 * \param[in] obj The object
 * \param thisv Primitive value
 * \param[in] name The property's name
 * \param[in] v The property's value
 * \param thr Throw exception on error
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result  sophon_object_put(Sophon_VM *vm,
							Sophon_Object *obj,
							Sophon_Value thisv,
							Sophon_String *name,
							Sophon_Value v,
							Sophon_Bool thr);

#ifdef __cplusplus
}
#endif

#endif

