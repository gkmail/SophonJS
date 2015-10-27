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

#include "sophon_js_parser.h"

static void
gc_mark_parser_stack (Sophon_VM *vm, Sophon_ParserStack *stk)
{
	Sophon_TokenValueField field;
	Sophon_TokenValue *v = &stk->v;

	field = sophon_token_value_get_field(stk->t);
	switch (field) {
		case FIELD_v:
			sophon_value_mark(vm, v->v);
			break;
		case FIELD_re:
			sophon_value_mark(vm, v->re.bodyv);
			sophon_value_mark(vm, v->re.flagsv);
			break;
		case FIELD_i:
			break;
		case FIELD_ops:
			break;
		case FIELD_expr:
			sophon_value_mark(vm, v->expr.bind_namev);
			break;
		case FIELD_var:
			sophon_value_mark(vm, v->var.namev);
			break;
		case FIELD_cases:
			break;
		case FIELD_COUNT:
			break;
	}
}

static void
gc_scan_parser (Sophon_VM *vm)
{
	Sophon_ParserData *p = (Sophon_ParserData*)vm->parser_data;
	Sophon_ParserObject *obj, *olast;
	Sophon_ParserStack *stk, *slast;

	gc_mark(vm, (Sophon_GCObject*)p->module);

	obj = p->obj_stack;
	olast = obj + p->obj_top;
	while (obj < olast) {
		sophon_hash_for_each(vm, &obj->prop_hash, gc_mark_value_key, NULL);
		obj++;
	}

	gc_mark_parser_stack(vm, &p->fetch);
	gc_mark_parser_stack(vm, &p->reduce);
	gc_mark_parser_stack(vm, &p->semicolon);

	stk = p->stack;
	slast = stk + p->top;
	while (stk < slast) {
		gc_mark_parser_stack(vm, stk);
		stk++;
	}
}

