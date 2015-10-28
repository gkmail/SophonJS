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

#ifndef _SOPHON_GC_H_
#define _SOPHON_GC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sophon_types.h"

/*GC object types*/
#define SOPHON_GC_String        0
#define SOPHON_GC_Object        1
#define SOPHON_GC_Closure       2
#define SOPHON_GC_Array         3
#define SOPHON_GC_GlobalFrame   4
#define SOPHON_GC_DeclFrame     5
#define SOPHON_GC_NameFrame     6
#define SOPHON_GC_WithFrame     7
#define SOPHON_GC_Module        8
#define SOPHON_GC_RegExp        9

/*GC object flags*/
#define SOPHON_GC_FL_MARKED  1
#define SOPHON_GC_FL_MANAGED 2
#define SOPHON_GC_FL_INTERN  4
#define SOPHON_GC_FL_EXTENSIBLE  8
#define SOPHON_GC_FL_BIND    16
#define SOPHON_GC_FL_NATIVE  32
#define SOPHON_GC_FL_STRICT  64
#define SOPHON_GC_FL_PRIM    128

#define SOPHON_GC_CLOSURE_SIZE\
	SOPHON_MAX(sizeof(Sophon_Object), sizeof(Sophon_Closure))
#define SOPHON_GC_ARRAY_SIZE\
	SOPHON_MAX(sizeof(Sophon_Object), sizeof(Sophon_Array))

/*GC object header*/
#define SOPHON_GC_HEADER \
	Sophon_GCObject *gc_next; \
	Sophon_U16       gc_flags;\
	Sophon_U8        gc_type;

#define SOPHON_GC_HEADER_INIT(obj, type) \
	SOPHON_MACRO_BEGIN \
		(obj)->gc_type  = type; \
		(obj)->gc_flags = 0; \
	SOPHON_MACRO_END

struct Sophon_GCObject_s {
	SOPHON_GC_HEADER
};

/**\brief GC object buffer*/
typedef struct {
	Sophon_GCObject **objs;  /**< Object pointer buffer*/
	Sophon_U32        count; /**< Object pointer count in the buffer*/
	Sophon_U32        cap;   /**< Capability of the buffer(allocated size)*/
} Sophon_GCObjectBuf;

extern void sophon_gc_init (Sophon_VM *vm);
extern void sophon_gc_deinit (Sophon_VM *vm);

#define sophon_gc_get_nb_count(vm)    ((vm)->gc_nb_obuf.count)
#define sophon_gc_set_nb_count(vm, c) ((vm)->gc_nb_obuf.count = (c))

/**
 * \brief Add an object to the garbage collector
 * GC will release the object when it is not used automatically.
 * \param[in] vm The current virtual machine
 * \param[in] obj The object to be managed by GC
 */
extern void sophon_gc_add (Sophon_VM *vm, Sophon_GCObject *obj);

/**
 * \brief Make an object as the GC root object
 * GC will scan the object to mark the used object.
 * The root object is the scanning start point.
 * \param[in] vm The current virtual machine
 * \param[in] obj The root object
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern void sophon_gc_add_root (Sophon_VM *vm, Sophon_GCObject *obj);

/**
 * \brief Remove a root GC object
 * \param[in] vm The current virtual machine
 * \param[in] obj The root object
 */
extern void sophon_gc_remove_root (Sophon_VM *vm, Sophon_GCObject *obj);

/**
 * \brief Add a value to the new borned object buffer
 * \param[in] vm The current virtual machine
 * \param[in] obj The root object
 */
extern void sophon_gc_add_nb (Sophon_VM *vm, Sophon_Value v);

/**
 * \brief Run the garbage collection process
 * \param[in] vm The current virtual machine
 */
extern void          sophon_gc_run (Sophon_VM *vm);

/**
 * \brief Check current memory usage status and run GC process if needed.
 * \param[in] vm The current virtual machine
 */
extern void          sophon_gc_check (Sophon_VM *vm);


#ifdef __cplusplus
}
#endif

#endif

