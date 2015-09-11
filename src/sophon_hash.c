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

#include <sophon_types.h>
#include <sophon_mm.h>
#include <sophon_util.h>
#include <sophon_hash.h>
#include <sophon_value.h>
#include <sophon_gc.h>
#include <sophon_string.h>
#include <sophon_debug.h>

void
sophon_hash_init (Sophon_VM *vm, Sophon_Hash *hash)
{
	SOPHON_ASSERT(vm && hash);

	hash->count   = 0;
	hash->bucket  = 0;
	hash->entries = NULL;
}

void
sophon_hash_deinit (Sophon_VM *vm, Sophon_Hash *hash)
{
	SOPHON_ASSERT(vm && hash);

	if (hash->entries) {
		Sophon_U32 i;

		for (i = 0; i < hash->bucket; i++) {
			Sophon_HashEntry *ent, *enext;

			for (ent = hash->entries[i]; ent; ent = enext) {
				enext = ent->next;

				sophon_mm_free(vm, ent, sizeof(Sophon_HashEntry));
			}
		}

		sophon_mm_free(vm, hash->entries,
					hash->bucket * sizeof(Sophon_HashEntry*));
	}
}

Sophon_Result
sophon_hash_add (Sophon_VM *vm,
						Sophon_Hash *hash,
						Sophon_KeyFunc key_func,
						Sophon_EqualFunc equ_func,
						Sophon_Ptr key,
						Sophon_HashEntry **pent)
{
	Sophon_HashEntry *ent;
	Sophon_U32 kv, pos;

	SOPHON_ASSERT(vm && hash && key_func && equ_func && pent);

	kv = key_func(vm, key);
	if (hash->count) {
		for (ent = hash->entries[kv % hash->bucket]; ent; ent = ent->next) {
			if (equ_func(vm, ent->key, key)) {
				*pent = ent;
				return SOPHON_NONE;
			}
		}
	}

	if (hash->bucket * 3 <= hash->count) {
		Sophon_U32 size = SOPHON_MAX(8, hash->count + 1);
		Sophon_HashEntry **buf;
		Sophon_U32 i;
		Sophon_HashEntry *enext;

		buf = (Sophon_HashEntry**)sophon_mm_alloc_ensure(vm,
					sizeof(Sophon_HashEntry*) * size);

		sophon_memset(buf, 0, sizeof(Sophon_HashEntry*) * size);

		for (i = 0; i < hash->bucket; i++) {
			for (ent = hash->entries[i]; ent; ent = enext) {
				enext = ent->next;

				pos = key_func(vm, ent->key) % size;
				ent->next = buf[pos];
				buf[pos] = ent;
			}
		}

		if (hash->entries)
			sophon_mm_free(vm, hash->entries,
						hash->bucket * sizeof(Sophon_HashEntry**));

		hash->entries = buf;
		hash->bucket  = size;
	}

	pos = kv % hash->bucket;

	ent = (Sophon_HashEntry*)sophon_mm_alloc_ensure(vm,
				sizeof(Sophon_HashEntry));

	ent->key  = key;
	ent->next = hash->entries[pos];
	hash->entries[pos] = ent;

	hash->count++;

	*pent = ent;

	return SOPHON_OK;
}

Sophon_Result
sophon_hash_remove (Sophon_VM *vm,
						Sophon_Hash *hash,
						Sophon_KeyFunc key_func,
						Sophon_EqualFunc equ_func,
						Sophon_Ptr key)
{
	Sophon_HashEntry *ent, *eprev;
	Sophon_U32 pos;

	SOPHON_ASSERT(vm && hash && key_func && equ_func);

	if (!hash->count)
		return SOPHON_NONE;

	pos = key_func(vm, key) % hash->bucket;

	for (eprev = NULL, ent = hash->entries[pos]; ent; ent = ent->next) {
		if (equ_func(vm, ent->key, key)){
			if (eprev)
				eprev->next = ent->next;
			else
				hash->entries[pos] = ent->next;

			sophon_mm_free(vm, ent, sizeof(Sophon_HashEntry));
			hash->count--;

			return SOPHON_OK;
		}
	}

	return SOPHON_NONE;
}

Sophon_Result
sophon_hash_lookup (Sophon_VM *vm,
						Sophon_Hash *hash,
						Sophon_KeyFunc key_func,
						Sophon_EqualFunc equ_func,
						Sophon_Ptr key,
						Sophon_HashEntry **pent)
{
	Sophon_HashEntry *ent;
	Sophon_U32 pos;

	SOPHON_ASSERT(vm && hash && key_func && equ_func);

	if (!hash->count)
		return SOPHON_NONE;

	pos = key_func(vm, key) % hash->bucket;

	for (ent = hash->entries[pos]; ent; ent = ent->next) {
		if (equ_func(vm, ent->key, key)){
			if (pent)
				*pent = ent;
			return SOPHON_OK;
		}
	}

	return SOPHON_NONE;
}

void
sophon_hash_for_each(Sophon_VM *vm,
						Sophon_Hash *hash,
						Sophon_ForEachFunc func,
						Sophon_Ptr data)
{
	Sophon_U32 i;
	Sophon_HashEntry *ent;

	SOPHON_ASSERT(vm && hash);

	for (i = 0; i < hash->bucket; i++) {
		for (ent = hash->entries[i]; ent; ent = ent->next) {
			func(vm, ent, data);
		}
	}
}

Sophon_U32
sophon_int_key (Sophon_VM *vm, Sophon_Ptr key)
{
	return (Sophon_U32)(Sophon_UIntPtr)key;
}

Sophon_Bool
sophon_int_equal (Sophon_VM *vm, Sophon_Ptr k1, Sophon_Ptr k2)
{
	return ((Sophon_IntPtr)k1) == ((Sophon_IntPtr)k2);
}

Sophon_U32
sophon_string_key (Sophon_VM *vm, Sophon_Ptr key)
{
	Sophon_String *str = (Sophon_String*)key;
	Sophon_Char *p;
	Sophon_U32 v = 1978;

	SOPHON_ASSERT(str);

	p = sophon_string_chars(vm, str);

	while (*p) {
		v = (v << 5) + v + *p;
		p++;
	}

	return v;
}

Sophon_Bool
sophon_string_equal (Sophon_VM *vm, Sophon_Ptr k1, Sophon_Ptr k2)
{
	Sophon_String *s1 = (Sophon_String*)k1;
	Sophon_String *s2 = (Sophon_String*)k2;

	return sophon_string_cmp(vm, s1, s2) ? SOPHON_FALSE : SOPHON_TRUE;
}

Sophon_U32
sophon_direct_key (Sophon_VM *vm, Sophon_Ptr key)
{
	return (Sophon_U32)(Sophon_UIntPtr)key;
}

Sophon_Bool
sophon_direct_equal (Sophon_VM *vm, Sophon_Ptr k1, Sophon_Ptr k2)
{
	return k1 == k2;
}

Sophon_U32
sophon_value_key (Sophon_VM *vm, Sophon_Ptr key)
{
	Sophon_Value v = (Sophon_Value)key;
	Sophon_U32 kv;

	if (SOPHON_VALUE_IS_DOUBLE(v)) {
		kv = (Sophon_U32)SOPHON_VALUE_GET_DOUBLE(v);
	} else if (SOPHON_VALUE_IS_GC(v)) {
		Sophon_GCObject *obj = SOPHON_VALUE_GET_GC(v);

		if (obj->gc_type == SOPHON_GC_STRING) {
			kv = sophon_string_key(vm, obj);
		} else {
			kv = (Sophon_U32)v;
		}
	} else {
		kv = (Sophon_U32)v;
	}

	return kv;
}

Sophon_Bool
sophon_value_equal (Sophon_VM *vm, Sophon_Ptr k1, Sophon_Ptr k2)
{
	Sophon_Value v1 = (Sophon_Value)k1;
	Sophon_Value v2 = (Sophon_Value)k2;
	Sophon_Bool r;

	if (SOPHON_VALUE_IS_DOUBLE(v1) && SOPHON_VALUE_IS_DOUBLE(v2)) {
		r = SOPHON_VALUE_GET_DOUBLE(v1) == SOPHON_VALUE_GET_DOUBLE(v2);
	} else if (SOPHON_VALUE_IS_GC(v1) && SOPHON_VALUE_IS_GC(v2)) {
		Sophon_GCObject *o1 = SOPHON_VALUE_GET_GC(v1);
		Sophon_GCObject *o2 = SOPHON_VALUE_GET_GC(v2);

		if ((o1->gc_type == SOPHON_GC_STRING) &&
					(o2->gc_type == SOPHON_GC_STRING)) {
			r = sophon_string_equal(vm, o1, o2);
		} else {
			r = v1 == v2;
		}
	} else {
		r = v1 == v2;
	}

	return r;
}

