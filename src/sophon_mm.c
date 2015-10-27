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

#include <sophon_mm.h>
#include <sophon_vm.h>
#include <sophon_util.h>
#include <sophon_debug.h>

#ifdef SOPHON_MM_DEBUG
static Sophon_Ptr mm_realloc
#else
Sophon_Ptr sophon_mm_realloc
#endif
(Sophon_VM *vm, Sophon_Ptr old_ptr, Sophon_U32 old_size,
		Sophon_U32 new_size)
{
	Sophon_Ptr ptr;

	SOPHON_ASSERT(vm);

	ptr = sophon_realloc(old_ptr, new_size);

	vm->mm_curr_used += new_size - old_size;

#ifdef SOPHON_DEBUG
	vm->mm_max_used = SOPHON_MAX(vm->mm_max_used, vm->mm_curr_used);
#endif

	return ptr;
}


#ifdef SOPHON_MM_DEBUG
#define MM_MAGIC 0x12345678

typedef struct MMHead_s MMHead;
struct MMHead_s {
	MMHead     *prev;
	MMHead     *next;
	const char *file;
	int         line;
	Sophon_U32  size;
	Sophon_U32  magic;
};

typedef struct {
	Sophon_U32  magic;
} MMTail;

static MMHead mm_used = {&mm_used, &mm_used};

void
sophon_mm_dump_unfreed ()
{
	MMHead *m;

	if (mm_used.next == &mm_used)
		return;

	sophon_prerr("unfreed buffers:\n");

	for (m = mm_used.next; m != &mm_used; m = m->next) {
		Sophon_U8 *buf = (Sophon_U8*)(m + 1);
		Sophon_Int i;

		sophon_prerr("\t%p \"%s\" %d size:%d ", m + 1, m->file,
					m->line, m->size);

		sophon_prerr("[");

		for (i = 0; i < SOPHON_MIN(m->size, 16); i++) {
			sophon_prerr("%02x ", buf[i]);
		}

		sophon_prerr("]\n");
	}
}

void
sophon_mm_check_all ()
{
	MMHead *m;
	Sophon_Bool err = SOPHON_FALSE;

	for (m = mm_used.next; m != &mm_used; m = m->next) {
		Sophon_U8 *buf = (Sophon_U8*)(m + 1);
		MMTail *t = (MMTail*)(buf + m->size);
		Sophon_Int i;

		if ((m->magic != MM_MAGIC) || (t->magic != MM_MAGIC)) {
			sophon_prerr("\t%p write overflow \"%s\" %d size:%d ", m + 1,
						m->file, m->line, m->size);
			sophon_prerr("[");

			for (i = 0; i < SOPHON_MIN(m->size, 16); i++) {
				sophon_prerr("%02x ", buf[i]);
			}

			sophon_prerr("]\n");
			sophon_prerr("\t\thead: %08x tail: %08x\n", m->magic, t->magic);
			err = SOPHON_TRUE;
		}
	}

	if (err)
		sophon_fatal("memory write overflow");
}

void
sophon_mm_check_ptr (Sophon_Ptr ptr, Sophon_U32 size)
{
	MMHead *head = ((MMHead*)ptr) - 1;
	MMTail *tail = (MMTail*)(((char*)ptr) + size);

	if ((head->magic != MM_MAGIC) && (tail->magic != MM_MAGIC)) {
		sophon_fatal("invalid buffer %p (head:%08x tail:%08x)",
				ptr, head->magic, tail->magic);
	}
}

Sophon_Ptr
sophon_mm_realloc_real (Sophon_VM *vm, Sophon_Ptr old_ptr, Sophon_U32 old_size,
			Sophon_U32 new_size, const char *file, int line)
{
	Sophon_Ptr ptr;
	Sophon_U32 real_new_size, real_old_size;

	real_new_size = new_size;
	real_old_size = old_size;

	if (new_size) {
		real_new_size += sizeof(MMHead) + sizeof(MMTail);
	}

	if (old_size) {
		real_old_size += sizeof(MMHead) + sizeof(MMTail);
	}

	if (old_ptr) {
		MMHead *head = ((MMHead*)old_ptr) - 1;
		MMTail *tail = (MMTail*)(((char*)old_ptr) + old_size);

		if ((head->magic != MM_MAGIC) && (tail->magic != MM_MAGIC)) {
			sophon_fatal("\"%s\" %d: free an invalid buffer %p "
					"(head:%08x tail:%08x)",
					file, line, old_ptr,
					head->magic, tail->magic);
		}

		if (old_size != head->size) {
			sophon_fatal("\"%s\" %d: %p allocated with size %d, "
						"but freed with size %d",
						file, line,
						old_ptr, head->size, old_size);
		}

		head->prev->next = head->next;
		head->next->prev = head->prev;
		head->magic = 0;
		tail->magic = 0;

		old_ptr = (Sophon_Ptr)head;
	}

	ptr = mm_realloc(vm, old_ptr, real_old_size, real_new_size);

	if (ptr && new_size) {
		MMHead *head = (MMHead*)ptr;
		MMTail *tail = (MMTail*)(((char*)(head + 1)) + new_size);

		head->prev = mm_used.prev;
		head->next = &mm_used;
		mm_used.prev->next = head;
		mm_used.prev = head;

		head->file  = file;
		head->line  = line;
		head->size  = new_size;
		head->magic = MM_MAGIC;
		tail->magic = MM_MAGIC;

		ptr = (Sophon_Ptr)(head + 1);
	}

	return ptr;
}

#endif /*SOPHON_MM_DEBUG*/

#ifndef SOPHON_MM_DEBUG
Sophon_Ptr
sophon_mm_realloc_ensure (Sophon_VM *vm, Sophon_Ptr old_ptr,
			Sophon_U32 old_size, Sophon_U32 new_size)
{
	Sophon_Ptr ptr;

	ptr = sophon_mm_realloc(vm, old_ptr, old_size, new_size);
	if (!ptr && new_size)
		sophon_nomem();

	return ptr;
}
#else /*defined(SOPHON_MM_DEBUG)*/
Sophon_Ptr
sophon_mm_realloc_ensure_real (Sophon_VM *vm, Sophon_Ptr old_ptr,
			Sophon_U32 old_size, Sophon_U32 new_size,
			const char *file, int line)
{
	Sophon_Ptr ptr;

	ptr = sophon_mm_realloc_real(vm, old_ptr, old_size, new_size,
				file, line);
	if (!ptr && new_size)
		sophon_nomem();

	return ptr;
}
#endif /*SOPHON_MM_DEBUG*/

