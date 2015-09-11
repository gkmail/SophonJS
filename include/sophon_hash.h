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

#ifndef _SOPHON_HASH_H_
#define _SOPHON_HASH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sophon_types.h"

/**
 * \brief Hash table entry
 */
typedef struct Sophon_HashEntry_s Sophon_HashEntry;

/**
 * \brief Hash table entry
 */
struct Sophon_HashEntry_s {
	Sophon_HashEntry  *next;  /**< The next entry in the list*/
	Sophon_Ptr         key;   /**< The hash key*/
	Sophon_Ptr         value; /**< The value*/
};

/**
 * \brief Hash table
 */
typedef struct Sophon_Hash_s {
	Sophon_U32         count;   /**< Entries count in the hash table*/
	Sophon_U32         bucket;  /**< Buckets count(Entry list count).*/
	Sophon_HashEntry **entries; /**< Entries list buffer*/
} Sophon_Hash;

/**
 * \brief Hash key equal check function
 */
typedef Sophon_Bool   (*Sophon_EqualFunc) (Sophon_VM *vm,
						Sophon_Ptr k1,
						Sophon_Ptr k2);

/**
 * \brief Hash key value calcaulate function
 */
typedef Sophon_U32    (*Sophon_KeyFunc) (Sophon_VM *vm,
						Sophon_Ptr key);

/**
 * \brief Hash entry ergodic function
 */
typedef void          (*Sophon_ForEachFunc) (Sophon_VM *vm,
						Sophon_HashEntry *ent,
						Sophon_Ptr data);

/**\brief Get the entries count in the hash table*/
#define sophon_hash_entry_count(vm, hash) ((hash)->count)

/**
 * \brief Hash table initialize
 * \param[in] vm The current virtual machine
 * \param[in] hash The hash table
 */
extern void sophon_hash_init (Sophon_VM *vm, Sophon_Hash *hash);

/**
 * \brief Clear resource used in the hash table
 * \param[in] vm The current virtual machine
 * \param[in] hash The hash table
 */
extern void sophon_hash_deinit (Sophon_VM *vm, Sophon_Hash *hash);

/**
 * \brief Add a new entry to the hash table
 * \param[in] vm The current virtual machine
 * \param[in] hash The hash table
 * \param key_func Key value calculate function
 * \param equ_func Key equal check function
 * \param key The entry's key
 * \param[out] ent The output entry pointer
 * If the key is already used in the hash table, ent return the old entry
 * with the key. If the key is not used, ent return the new created entry.
 * \retval SOPHON_NONE The key is used by another value
 * \retval SOPHON_OK A new entry is created
 * \retval <0 On error
 */
extern Sophon_Result sophon_hash_add (Sophon_VM *vm,
						Sophon_Hash *hash,
						Sophon_KeyFunc key_func,
						Sophon_EqualFunc equ_func,
						Sophon_Ptr key,
						Sophon_HashEntry **ent);

/**
 * \brief Remove an entry the key from the hash table
 * \param[in] vm The current virtual machine
 * \param[in] hash The hash table
 * \param key_func Key value calculate function
 * \param equ_func Key equal check function
 * \param key The entry's key
 * \retval SOPHON_OK The entry is removed
 * \retval SOPHON_NONE Hash table has not an entry with the key
 * \retval <0 On error
 */
extern Sophon_Result sophon_hash_remove (Sophon_VM *vm,
						Sophon_Hash *hash,
						Sophon_KeyFunc key_func,
						Sophon_EqualFunc equ_func,
						Sophon_Ptr key);

/**
 * \brief Lookup an entry in the hash table
 * \param[in] vm The current virtual machine
 * \param[in] hash The hash table
 * \param key_func Key value calculate function
 * \param equ_func Key equal check function
 * \param key The entry's key
 * \param[out] ent Return the entry with the key
 * \retval SOPHON_OK The entry with the key is found
 * \retval SOPHON_NONE Hash table has not an entry with the key
 * \retval <0 On error
 */
extern Sophon_Result sophon_hash_lookup (Sophon_VM *vm,
						Sophon_Hash *hash,
						Sophon_KeyFunc key_func,
						Sophon_EqualFunc equ_func,
						Sophon_Ptr key,
						Sophon_HashEntry **ent);

/**
 * \brief Invoke the function for each entry in the hash table
 * \param[in] vm The current virtual machine
 * \param[in] hash The hash table
 * \param func The function to be invoked for each entry
 * \param data The data will be used as the 3rd parameter of 'func'
 */
extern void          sophon_hash_for_each(Sophon_VM *vm,
						Sophon_Hash *hash,
						Sophon_ForEachFunc func,
						Sophon_Ptr data);

/**
 * \brief Integer key value calculate function
 * \param[in] vm The current virtual machine
 * \param key The hash key
 * \return The key value
 */
extern Sophon_U32    sophon_int_key (Sophon_VM *vm, Sophon_Ptr key);

/**
 * \brief Integer key equal check function
 * \param[in] vm The current virtual machine
 * \param k1 Key 1
 * \param k2 Key 2
 * \retval SOPHON_TRUE 2 keys are equal
 * \retval SOPHON_FALSE 2 keys are not equal
 */
extern Sophon_Bool   sophon_int_equal (Sophon_VM *vm, Sophon_Ptr k1,
						Sophon_Ptr k2);

/**
 * \brief String key value calculate function
 * \param[in] vm The current virtual machine
 * \param key The hash key
 * \return The key value
 */
extern Sophon_U32    sophon_string_key (Sophon_VM *vm, Sophon_Ptr key);

/**
 * \brief String key equal check function
 * \param[in] vm The current virtual machine
 * \param k1 Key 1
 * \param k2 Key 2
 * \retval SOPHON_TRUE 2 keys are equal
 * \retval SOPHON_FALSE 2 keys are not equal
 */
extern Sophon_Bool   sophon_string_equal (Sophon_VM *vm, Sophon_Ptr k1,
						Sophon_Ptr k2);

/**
 * \brief Pointer key value calculate function
 * \param[in] vm The current virtual machine
 * \param key The hash key
 * \return The key value
 */
extern Sophon_U32    sophon_direct_key (Sophon_VM *vm, Sophon_Ptr key);

/**
 * \brief String key equal check function
 * \param[in] vm The current virtual machine
 * \param k1 Key 1
 * \param k2 Key 2
 * \retval SOPHON_TRUE 2 keys are equal
 * \retval SOPHON_FALSE 2 keys are not equal
 */
extern Sophon_Bool   sophon_direct_equal (Sophon_VM *vm, Sophon_Ptr k1,
						Sophon_Ptr k2);

/**
 * \brief Value key value calculate function
 * \param[in] vm The current virtual machine
 * \param key The hash key
 * \return The key value
 */
extern Sophon_U32    sophon_value_key (Sophon_VM *vm, Sophon_Ptr key);

/**
 * \brief Value key equal check function
 * \param[in] vm The current virtual machine
 * \param k1 Key 1
 * \param k2 Key 2
 * \retval SOPHON_TRUE 2 keys are equal
 * \retval SOPHON_FALSE 2 keys are not equal
 */
extern Sophon_Bool   sophon_value_equal (Sophon_VM *vm, Sophon_Ptr k1,
						Sophon_Ptr k2);

#ifdef __cplusplus
}
#endif

#endif

