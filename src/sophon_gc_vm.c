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
gc_mark_vm (Sophon_VM *vm)
{
	Sophon_Stack *stk;

	sophon_value_mark(vm, vm->excepv);
	sophon_value_mark(vm, vm->Arguments_protov);

#define MARK_ERROR(name) sophon_value_mark(vm, vm->name);
	SOPHON_FOR_EACH_ERROR(MARK_ERROR)

#define MARK_STRING(name)\
	gc_mark(vm, (Sophon_GCObject*)vm->name##_str);
	SOPHON_FOR_EACH_STRING(MARK_STRING)

#define MARK_INTERNAL_STRING(name, str)\
	gc_mark(vm, (Sophon_GCObject*)vm->name##_str);
	SOPHON_FOR_EACH_INTERNAL_STRING(MARK_INTERNAL_STRING)

	stk = vm->stack;
	while (stk) {
		Sophon_Value *v, *vlast;

		v = stk->v;
		vlast = v + stk->vbuf_size;

		while (v < vlast) {
			sophon_value_mark(vm, *v);
			v++;
		}

		sophon_value_mark(vm, stk->calleev);
		sophon_value_mark(vm, stk->retv);

		if (stk->func)
			gc_mark(vm, (Sophon_GCObject*)stk->func->module);

		gc_mark(vm, (Sophon_GCObject*)stk->var_env);
		gc_mark(vm, (Sophon_GCObject*)stk->lex_env);

		stk = stk->bottom;
	}

	if (vm->parser_data)
		gc_scan_parser(vm);

	gc_mark(vm, (Sophon_GCObject*)vm->glob_module);

#ifdef SOPHON_CONSOLE
	sophon_hash_for_each(vm, &vm->timer_hash, gc_mark_value_key, NULL);
#endif
}

