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
gc_mark_value_key (Sophon_VM *vm, Sophon_HashEntry *ent, Sophon_Ptr data)
{
	Sophon_Value v = (Sophon_Value)ent->key;

	sophon_value_mark(vm, v);
}

static void
gc_scan_module (Sophon_VM *vm, Sophon_Module *mod)
{
	Sophon_Value *c, *clast;
	Sophon_Function **f, **flast;

	if (mod->name)
		gc_mark(vm, (Sophon_GCObject*)mod->name);

	if (mod->base)
		gc_mark(vm, (Sophon_GCObject*)mod->base);

	sophon_value_mark(vm, mod->globv);

	sophon_hash_for_each(vm, &mod->const_hash, gc_mark_value_key, NULL);

	c = mod->consts;
	clast = c + mod->const_count;
	while (c < clast) {
		sophon_value_mark(vm, *c);
		c++;
	}

	f = mod->funcs;
	flast = f + mod->func_count;
	while (f < flast) {
		gc_scan_func(vm, *f);
		f++;
	}
}

