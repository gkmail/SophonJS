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
#include <sophon_number_pool.h>
#include <sophon_hash.h>
#include <sophon_gc.h>
#include <sophon_vm.h>
#include <sophon_debug.h>

#ifndef SOPHON_NUMBER_POOL_SIZE_SHIFT
	#define SOPHON_NUMBER_POOL_SIZE_SHIFT 12
#endif

#define SOPHON_NUMBER_POOL_SIZE (1 << SOPHON_NUMBER_POOL_SIZE_SHIFT)

#define SOPHON_NUMBER_ENTRY_COUNT \
	(((SOPHON_NUMBER_POOL_SIZE - sizeof(Sophon_NumberPool)) * 8 / \
			(sizeof(Sophon_Number) * 8 + 2)) & ~0x1F)

static void
np_free (Sophon_VM *vm, Sophon_NumberPool *pool)
{
	Sophon_UIntPtr key;

	key = ((Sophon_UIntPtr)pool) >> SOPHON_NUMBER_POOL_SIZE_SHIFT;
	sophon_hash_remove(vm, &vm->np_hash,
				sophon_direct_key,
				sophon_direct_equal,
				(Sophon_Ptr)key);

	sophon_mm_free(vm, pool, SOPHON_NUMBER_POOL_SIZE);

	SOPHON_INFO(("free number pool %p", pool));
}

static void
np_sweep (Sophon_VM *vm, Sophon_NumberPool *pool)
{
	Sophon_U32 *used, *mark;
	Sophon_Int i, off, b;

	used = pool->used_flags;
	mark = pool->mark_flags;
	off  = 0;

	for (i = 0; i < SOPHON_NUMBER_ENTRY_COUNT / 32; i++) {
		if (*used) {
			for (b = 0; b < 32; b++){
				if (*used & (1 << b)) {
					if (*mark & (1 << b)) {
						*mark &= ~(1 << b);
					} else {
						Sophon_NumberEntry *ent;

						ent = (Sophon_NumberEntry*)(pool->entries + off + b);
						ent->next = pool->free_list;
						pool->free_list = ent;
						pool->free_count++;

						SOPHON_ASSERT(pool->free_count <=
									SOPHON_NUMBER_ENTRY_COUNT);

						*used &= ~(1 << b);
					}
				}
			}
		}
		
		used++;
		mark++;
		off += 32;
	}
}

static Sophon_NumberPool*
np_alloc (Sophon_VM *vm)
{
	Sophon_NumberEntry **pent;
	Sophon_HashEntry *hent;
	Sophon_Int i;
	Sophon_NumberPool *pool;
	Sophon_UIntPtr key;
	Sophon_Number *pd;
	Sophon_Result r;

	pool = (Sophon_NumberPool*)sophon_mm_alloc_ensure(vm,
				SOPHON_NUMBER_POOL_SIZE);
	SOPHON_INFO(("allocate number pool %p, size %d, double count %d",
				pool,
				(int)SOPHON_NUMBER_POOL_SIZE,
				(int)SOPHON_NUMBER_ENTRY_COUNT));

	key = ((Sophon_UIntPtr)pool) >> SOPHON_NUMBER_POOL_SIZE_SHIFT;
	r = sophon_hash_add(vm, &vm->np_hash,
				sophon_direct_key,
				sophon_direct_equal,
				(Sophon_Ptr)key,
				&hent);
	if (r != SOPHON_OK)
		return NULL;

	hent->value = pool;

	pool->free_count = SOPHON_NUMBER_ENTRY_COUNT;
	pool->used_flags = (Sophon_U32*)(pool + 1);
	pool->mark_flags = pool->used_flags + (SOPHON_NUMBER_ENTRY_COUNT / 32);

	sophon_memset(pool->used_flags, 0, SOPHON_NUMBER_ENTRY_COUNT / 4);

	pool->entries    = (Sophon_Number*)
			(pool->mark_flags + (SOPHON_NUMBER_ENTRY_COUNT / 32));

	pent = &pool->free_list;
	pd   = pool->entries;

	for (i = 0; i < SOPHON_NUMBER_ENTRY_COUNT; i++) {
		*pent = (Sophon_NumberEntry*)pd;
		pent = &(((Sophon_NumberEntry*)pd)->next);
		pd++;
	}

	*pent = NULL;

	SOPHON_ASSERT((Sophon_U8*)pent < ((Sophon_U8*)pool) + SOPHON_NUMBER_POOL_SIZE);

	pool->next = vm->np_free_list;
	vm->np_free_list = pool;

	return pool;
}

void
sophon_number_pool_init (Sophon_VM *vm)
{
	sophon_hash_init(vm, &vm->np_hash);
	vm->np_free_list = NULL;
	vm->np_full_list = NULL;
}

void
sophon_number_pool_deinit (Sophon_VM *vm)
{
	Sophon_NumberPool *p, *np;

	for (p = vm->np_free_list; p; p = np) {
		np = p->next;
		sophon_mm_free(vm, p, SOPHON_NUMBER_POOL_SIZE);
	}

	for (p = vm->np_full_list; p; p = np) {
		np = p->next;
		sophon_mm_free(vm, p, SOPHON_NUMBER_POOL_SIZE);
	}

	sophon_hash_deinit(vm, &vm->np_hash);
}

void
sophon_number_pool_sweep (Sophon_VM *vm)
{
	Sophon_NumberPool *p, *np, **pp;

	SOPHON_INFO(("sweep number pool"));

	pp = &vm->np_free_list;
	for (p = vm->np_free_list; p; p = np) {
		np = p->next;
		np_sweep(vm, p);

		if (p->free_count == SOPHON_NUMBER_ENTRY_COUNT) {
			*pp = np;
			np_free(vm, p);
		} else {
			pp = &p->next;
		}
	}

	pp = &vm->np_full_list;
	for (p = vm->np_full_list; p; p = np) {
		np = p->next;
		np_sweep(vm, p);

		if (p->free_list) {
			*pp = np;

			if (p->free_count == SOPHON_NUMBER_ENTRY_COUNT) {
				np_free(vm, p);
			} else {
				p->next = vm->np_free_list;
				vm->np_free_list = p;
			}
		} else {
			pp = &p->next;
		}
	}
}

void
sophon_number_mark (Sophon_VM *vm, Sophon_Value v)
{
	Sophon_UIntPtr key;
	Sophon_Number *ptr;
	Sophon_HashEntry *ent;
	Sophon_NumberPool *pool;
	Sophon_U32 id;
	Sophon_Result r;

	ptr = (Sophon_Number*)(((Sophon_UIntPtr)v) & ~SOPHON_VALUE_TYPE_MASK);
	key = ((Sophon_UIntPtr)ptr) >> SOPHON_NUMBER_POOL_SIZE_SHIFT;

	r = sophon_hash_lookup(vm, &vm->np_hash,
				sophon_direct_key,
				sophon_direct_equal,
				(Sophon_Ptr)key,
				&ent);

	SOPHON_ASSERT(r >= 0);

	if (r == SOPHON_NONE) {
		key--;

		r = sophon_hash_lookup(vm, &vm->np_hash,
					sophon_direct_key,
					sophon_direct_equal,
					(Sophon_Ptr)key,
					&ent);
		SOPHON_ASSERT(r == SOPHON_OK);
	}

	pool = (Sophon_NumberPool*)ent->value;
	id = ptr - pool->entries;

	SOPHON_ASSERT(ptr == (pool->entries + id));
	SOPHON_ASSERT(pool->used_flags[id >> 5] & (1 << (id & 0x1F)));

	pool->mark_flags[id >> 5] |= (1 << (id & 0x1F));
}

void
sophon_value_set_number_real (Sophon_VM *vm, Sophon_Value *v, Sophon_Number d)
{
	Sophon_NumberPool *pool;
	Sophon_Number *pd;
	Sophon_U32 id;
	Sophon_NumberEntry *ent;

	SOPHON_ASSERT(vm && v);

	pool = vm->np_free_list;

	/*Allocate a new pool*/
	if (!pool) {
		/*Try to run GC*/
		sophon_gc_check(vm);

		pool = vm->np_free_list;

		if (!pool) {
			pool = np_alloc(vm);
			if (! pool) {
				sophon_nomem();
			}
		}
	}

	/*Allocate a new double*/
	ent = pool->free_list;

	pool->free_list = ent->next;
	pool->free_count--;

	pd = (Sophon_Number*)ent;
	*pd = d;

	SOPHON_ASSERT((Sophon_U8*)pd < ((Sophon_U8*)pool) + SOPHON_NUMBER_POOL_SIZE);

	id = pd - pool->entries;
	pool->used_flags[id >> 5] |= 1 << (id & 0x1F);

	SOPHON_ASSERT(id < SOPHON_NUMBER_ENTRY_COUNT);

	*v = ((Sophon_Value)pd) | SOPHON_VALUE_TYPE_NUMBER;

	/*Add it to the new borned buffer*/
	sophon_gc_add_nb(vm, *v);

	if (! pool->free_list){
		SOPHON_ASSERT(!pool->free_count);

		vm->np_free_list = pool->next;
		pool->next = vm->np_full_list;
		vm->np_full_list = pool;
	}
}

