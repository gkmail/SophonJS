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
#include "sophon_property.h"

/**\brief Object*/
struct Sophon_Object_s {
	SOPHON_GC_HEADER
	Sophon_Property **props;       /**< The property list buffer*/
	Sophon_U16        prop_bucket; /**< The property list count*/
	Sophon_U16        prop_count;  /**< The property count*/
	Sophon_Value      protov;      /**< Prototype value*/
	Sophon_Value      primv;       /**< Primitive value*/
};

/*Primitive object type*/
#define SOPHON_FOR_EACH_PRIM_OBJ(o)\
	o(Closure)\
	o(Array)\
	o(RegExp)

enum {
#define SOPHON_PRIM_OBJ_ENUM_ITEM(o) SOPHON_PRIM_OBJ_##o,
	SOPHON_FOR_EACH_PRIM_OBJ(SOPHON_PRIM_OBJ_ENUM_ITEM)
	SOPHON_PRIM_OBJ_COUNT
};

/*Primitive object's GC flag*/
enum {
#define SOPHON_PRIM_OBJ_GC_FLAGS(o)\
	SOPHON_GC_FL_##o = SOPHON_GC_FL_PRIM << SOPHON_PRIM_OBJ_##o,
	SOPHON_FOR_EACH_PRIM_OBJ(SOPHON_PRIM_OBJ_GC_FLAGS)
	SOPHON_GC_FL_PRIM_END
};

#define SOPHON_PRIM_OBJ_ALLOC(vm, ptr, type)\
	SOPHON_MACRO_BEGIN\
		Sophon_U32 size;\
		size = SOPHON_MAX(sizeof(Sophon_Object), sizeof(Sophon_##type));\
		ptr = (Sophon_##type*)sophon_mm_alloc_ensure(vm, size);\
		((Sophon_GCObject*)(ptr))->gc_type  = SOPHON_GC_##type;\
		((Sophon_GCObject*)(ptr))->gc_flags = \
					SOPHON_GC_FL_##type | SOPHON_GC_FL_EXTENSIBLE;\
	SOPHON_MACRO_END

#define SOPHON_PRIM_OBJ_FREE(vm, ptr, type)\
	SOPHON_MACRO_BEGIN\
		Sophon_U32 size;\
		if (((Sophon_GCObject*)(ptr))->gc_flags & SOPHON_GC_FL_##type)\
			size = SOPHON_MAX(sizeof(Sophon_Object), sizeof(Sophon_##type));\
		else\
			size = sizeof(Sophon_##type);\
		sophon_mm_free(vm, ptr, size);\
	SOPHON_MACRO_END

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

#ifdef __cplusplus
}
#endif

#endif

