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

static void
gc_mark_value_entry (Sophon_VM *vm, Sophon_HashEntry *ent, Sophon_Ptr data)
{
	Sophon_String *name = (Sophon_String*)ent->key;
	Sophon_Value value = (Sophon_Value)ent->value;

	gc_mark(vm, (Sophon_GCObject*)name);
	sophon_value_mark(vm, value);
}

static void
gc_scan_decl_frame (Sophon_VM *vm, Sophon_DeclFrame *frame)
{
	Sophon_Value *v, *vlast;

	sophon_hash_for_each(vm, &frame->var_hash, gc_mark_value_entry, NULL);

	if (frame->func)
		gc_scan_func(vm, frame->func);

	v = frame->v;
	vlast = v + frame->vbuf_size;

	while (v < vlast) {
		sophon_value_mark(vm, *v);
		v++;
	}

	if (frame->frame.bottom)
		gc_mark(vm, (Sophon_GCObject*)(frame->frame.bottom));

	sophon_value_mark(vm, frame->callerv);
	sophon_value_mark(vm, frame->calleev);
	sophon_value_mark(vm, frame->thisv);
	sophon_value_mark(vm, frame->argsv);
}

#define gc_scan_global_frame gc_scan_decl_frame

static void
gc_scan_catch_frame (Sophon_VM *vm, Sophon_CatchFrame *frame)
{
	if (frame->frame.bottom)
		gc_mark(vm, (Sophon_GCObject*)(frame->frame.bottom));

	gc_mark(vm, (Sophon_GCObject*)frame->name);
	sophon_value_mark(vm, frame->excepv);
}

static void
gc_scan_with_frame (Sophon_VM *vm, Sophon_WithFrame *frame)
{
	if (frame->frame.bottom)
		gc_mark(vm, (Sophon_GCObject*)(frame->frame.bottom));

	sophon_value_mark(vm, frame->withv);
}

