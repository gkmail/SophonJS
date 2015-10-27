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
#include <sophon_vm.h>
#include <sophon_json.h>
#include <sophon_string.h>
#include <sophon_array.h>
#include <sophon_arguments.h>
#include <sophon_debug.h>

#include "sophon_parser_internal.h"
#include "sophon_json_parser.c"

#ifndef SOPHON_JSON_PARSER_STACK_SIZE
	#define SOPHON_JSON_PARSER_STACK_SIZE 512
#endif

#if 0
#define DEBUG(a) SOPHON_INFO(a)
#else
#define DEBUG(a)
#endif

typedef struct {
	Sophon_U16        s;
	Sophon_U16        t;
	Sophon_Location   l;
	Sophon_TokenValue v;
} ParserStack;

typedef struct {
	ParserStack stack[SOPHON_JSON_PARSER_STACK_SIZE];
	ParserStack fetch;
	ParserStack reduce;
	Sophon_U32  top;
	Sophon_JSONParseParams *p;
	Sophon_Value *retv;
} ParserData;

typedef struct StrStack_s StrStack;
struct StrStack_s {
	StrStack     *bottom;
	Sophon_Value  value;
};

static Sophon_Result
parser_reduce (Sophon_VM *vm, ParserData *p, Sophon_U16 rid, Sophon_U8 pop,
			Sophon_TokenValue *v)
{
	ParserStack *top = &p->stack[p->top - 1];

#define V(n) top[(n)+1-pop].v
#define L(n) top[(n)+1-pop].l

	switch (rid) {
		case R_ACCEPT:
			*p->retv = V(0).v;
			break;
		case R_TRUE:
			sophon_value_set_bool(vm, &v->v, SOPHON_TRUE);
			break;
		case R_FALSE:
			sophon_value_set_bool(vm, &v->v, SOPHON_FALSE);
			break;
		case R_NULL:
			sophon_value_set_null(vm, &v->v);
			break;
		case R_COPY:
			*v = V(0);
			break;
		case R_COPY_1:
			*v = V(1);
			break;
		case R_EMPTY_ARRAY: {
			Sophon_Array *arr = sophon_array_create(vm);
			sophon_value_set_array(vm, v, arr);
			break;
		}
		case R_EMPTY_OBJECT: {
			Sophon_Object *obj = sophon_object_create(vm);
			sophon_value_set_object(vm, v, obj);
			break;
		}
		case R_ELEMENT: {
			Sophon_Array *arr = sophon_array_create(vm);
			sophon_array_set_item(vm, arr, 0, V(0).v);
			sophon_value_set_array(vm, v, arr);
			break;
		}
		case R_APPEND_ELEMENT: {
			Sophon_Array *arr;
			Sophon_U32 len;

			sophon_value_to_array(vm, V(0).v, &arr);
			len = sophon_array_get_length(vm, arr);
			sophon_array_set_item(vm, arr, len, V(2).v);
			sophon_value_set_array(vm, v, arr);
			break;
		}
		case R_MEMBER:
			v->member.name = V(0).v;
			v->member.v    = V(2).v;
			break;
		case R_MEMBER_OBJECT: {
			Sophon_Object *obj = sophon_object_create(vm);
			sophon_value_set_object(vm, v, obj);
			sophon_value_put(vm, SOPHON_VALUE_GC(obj), V(0).member.name,
						V(0).member.v, 0);
			break;
		}
		case R_APPEND_MEMBER: {
			Sophon_Object *obj;
			sophon_value_to_object(vm, V(0).v, &obj);
			sophon_value_set_object(vm, v, obj);
			sophon_value_put(vm, SOPHON_VALUE_GC(obj), V(2).member.name,
						V(2).member.v, 0);
			break;
		}
	}

	return SOPHON_OK;
}

#define PUSH()\
	SOPHON_MACRO_BEGIN\
		p->top++;\
		top = &p->stack[p->top - 1];\
	SOPHON_MACRO_END
#define POP()\
	SOPHON_MACRO_BEGIN\
		p->top--;\
		top = &p->stack[p->top - 1];\
	SOPHON_MACRO_END
#define POPN(n)\
	SOPHON_MACRO_BEGIN\
		p->top -= n;\
		top = &p->stack[p->top - 1];\
	SOPHON_MACRO_END

static Sophon_Result
json_parse (Sophon_VM *vm, ParserData *p, Sophon_Value *retv)
{
	ParserStack *top;
	Sophon_U16 edge, sym;
	Sophon_U32 reduce;
	Sophon_Token tok = 0;

	PUSH();
	top->s = 0;
	top->t = 0;
	p->fetch.t = 0xFFFF;
	p->retv = retv;

next_state:
	if (p->fetch.t == 0xFFFF) {
		tok = sophon_lex(vm, &p->fetch.v, &p->fetch.l);
		DEBUG(("get token %d", tok));

		if ((tok != SOPHON_ERR_EOF) && tok < 0)
			return tok;
		if (tok == SOPHON_ERR_EOF)
			goto reduce;

		p->fetch.t = tok;
	} else {
		tok = p->fetch.t;
	}

	edge = json_state_shifts[top->s];
	while (edge != 0xFFFF) {
		sym = json_edge_symbol[edge];
		if (sym == p->fetch.t) {
			PUSH();
			p->fetch.s = json_edge_dest[edge];
			DEBUG(("shift to %d", p->fetch.s));
			*top = p->fetch;
			p->fetch.t = 0xFFFF;
			goto next_state;
		}

		edge = json_edge_next[edge];
	}

reduce:
	reduce = json_state_reduce[top->s];
	if (reduce != R_NONE) {
		Sophon_Int rid;
		Sophon_Int tid;
		Sophon_Int pop;

		rid = RV_GET_REDUCE(reduce);
		tid = RV_GET_TOKEN(reduce);
		pop = RV_GET_POP(reduce);

		DEBUG(("reduce %d %d %d", rid, tid, pop));

		if (parser_reduce(vm, p, rid, pop, &p->reduce.v) < 0)
			goto error;

		if (!RV_GET_POP_FLAG(reduce)) {
			pop = 0;
			tid = N_REDUCE;
		}

		if (pop) {
			ParserStack *stk;
			
			stk = &p->stack[p->top - pop];

			p->reduce.l.first_line   = stk->l.first_line;
			p->reduce.l.first_column = stk->l.first_column;
			p->reduce.l.last_line    = top->l.last_line;
			p->reduce.l.last_column  = top->l.last_column;
			p->reduce.t = tid;

			POPN(pop);
		} else {
			p->reduce.l = top->l;
			p->reduce.t = tid;
		}

		if ((tid == N_START) && (tok == SOPHON_ERR_EOF)) {
			goto accept;
		}

		edge = json_state_jumps[top->s];
		while (edge != 0xFFFF) {
			sym = json_edge_symbol[edge];
			if (sym == tid) {
				PUSH();
				p->reduce.s = json_edge_dest[edge];
				DEBUG(("goto %d", p->reduce.s));
				*top = p->reduce;
				goto next_state;
			}
			edge = json_edge_next[edge];
		}
	}
error:
	sophon_throw(vm, vm->SyntaxError, "Parse JSON error");
	return SOPHON_ERR_PARSE;
accept:
	DEBUG(("accept"));
	return SOPHON_OK;
}

static Sophon_Result
json_revive (Sophon_VM *vm, Sophon_Value v, Sophon_Value reviver)
{
	Sophon_Value argv[2], item;
	Sophon_U32 id;
	Sophon_Result r;

	if (sophon_value_is_array(v)) {
		Sophon_Array *arr;
		Sophon_U32 len;

		sophon_value_to_array(vm, v, &arr);
		len = sophon_array_get_length(vm, arr);
		for (id = 0; id < len; id++) {
			Sophon_Value iv;
			Sophon_String *str;

			sophon_array_get_item(vm, arr, id, &item);

			if (!sophon_value_is_undefined(item)) {
				sophon_value_set_int(vm, &iv, id);
				sophon_value_to_string(vm, iv, &str);
				argv[0] = SOPHON_VALUE_GC(str);
				argv[1] = item;

				if ((r = sophon_value_call(vm, reviver, v, argv, 2, &item, 0))
							!= SOPHON_OK)
					return r;
			}

			if (sophon_value_is_undefined(item)) {
				sophon_array_delete_item(vm, arr, id);
			} else {
				sophon_array_set_item(vm, arr, id, item);

				if ((r = json_revive(vm, item, reviver)) != SOPHON_OK)
					return r;
			}
		}
	} else if (sophon_value_is_object(v)) {
		Sophon_Object *obj;
		Sophon_Property *prop;
		Sophon_PropIter pi;

		sophon_value_to_object(vm, v, &obj);

		pi.bottom = vm->pi_stack;
		vm->pi_stack = &pi;
		r = SOPHON_OK;

		for (id = 0; id < obj->prop_bucket; id++) {
			for (prop = obj->props[id]; prop; prop = pi.prop) {
				pi.prop = prop->next;

				if (prop->attrs & SOPHON_PROP_ATTR_ACCESSOR) {
					Sophon_AccessorProperty *aprop =
							(Sophon_AccessorProperty*)prop;
					if ((r = sophon_value_call(vm, aprop->getv, v, NULL, 0,
										&item, 0)) != SOPHON_OK)
						goto obj_end;
				} else {
					item = prop->value;
				}

				if (!sophon_value_is_undefined(item)) {
					argv[0] = SOPHON_VALUE_GC(prop->name);
					argv[1] = item;
					if ((r = sophon_value_call(vm, reviver, v, argv, 2,
								&item, 0)) != SOPHON_OK)
						goto obj_end;
				}

				if (sophon_value_is_undefined(item)) {
					sophon_value_delete_prop(vm, v,
								SOPHON_VALUE_GC(prop->name), 0);
				} else {
					sophon_value_put(vm, v,
								SOPHON_VALUE_GC(prop->name), item, 0);
					if ((r = json_revive(vm, item, reviver)) != SOPHON_OK)
						goto obj_end;
				}
			}
		}

obj_end:
		vm->pi_stack = pi.bottom;
		return r;
	}

	return SOPHON_OK;
}

Sophon_Result
sophon_json_parse (Sophon_VM *vm, Sophon_Encoding enc,
		Sophon_IOFunc inp, Sophon_Ptr data, Sophon_Value *retv,
		Sophon_JSONParseParams *p)
{
	ParserData *pd;
	Sophon_Result r;

	SOPHON_ASSERT(vm && inp && retv);

	sophon_lex_init(vm, enc, inp, data);
	pd = sophon_mm_alloc_ensure(vm, sizeof(ParserData));

	pd->top = 0;
	pd->p   = p;

	r = json_parse(vm, pd, retv);

	if ((r == SOPHON_OK) && p && sophon_value_is_closure(p->reviver) &&
				!sophon_value_is_undefined(*retv)) {
		Sophon_Object *obj;
		Sophon_Value holder, key, argv[2];

		obj = sophon_object_create(vm);
		sophon_value_set_object(vm, &holder, obj);
		sophon_value_set_string(vm, &key, vm->empty_str);
		sophon_value_define_prop(vm, holder, key, *retv,
					SOPHON_VALUE_UNDEFINED,
					SOPHON_DATA_PROP_ATTR,
					SOPHON_FL_DATA_PROP);

		argv[0] = key;
		argv[1] = *retv;

		if ((r = sophon_value_call(vm, p->reviver, holder, argv, 2,
							retv, 0)) != SOPHON_OK)
			return r;

		if (sophon_value_is_undefined(*retv))
			return r;

		r = json_revive(vm, *retv, p->reviver);
	}

	sophon_mm_free(vm, pd, sizeof(ParserData));
	sophon_lex_deinit(vm);

	return r;
}

static Sophon_Result
json_item_replace (Sophon_VM *vm, Sophon_JSONStringifyParams *p,
			Sophon_Value holder, Sophon_Value key, Sophon_Value val,
			Sophon_Value *retv)
{
	Sophon_Result r;

	if (!p || (!p->whitelist && !sophon_value_is_closure(p->replacer))) {
		*retv = val;
		return SOPHON_OK;
	}

	if (p->whitelist) {
		Sophon_U32 id;
		Sophon_Value item;

		if (sophon_value_is_array(holder)) {
			*retv = val;
			return SOPHON_OK;
		}

		sophon_array_for_each(p->whitelist, id, item) {
			Sophon_String *s1, *s2;

			if ((r = sophon_value_to_string(vm, item, &s1)) != SOPHON_OK)
				return r;
			if ((r = sophon_value_to_string(vm, key, &s2)) != SOPHON_OK)
				return r;

			if (!sophon_string_cmp(vm, s1, s2)) {
				*retv = val;
				return SOPHON_OK;
			}
		}
	}

	if (sophon_value_is_closure(p->replacer)) {
		Sophon_Value argv[2];
		Sophon_String *str;
		Sophon_Result r;

		if ((r = sophon_value_to_string(vm, key, &str)) != SOPHON_OK)
			return r;

		sophon_value_set_string(vm, &argv[0], str);
		argv[1] = val;
		return sophon_value_call(vm, p->replacer, holder, argv, 2, retv, 0);
	}

	sophon_value_set_undefined(vm, retv);
	return SOPHON_OK;
}

static Sophon_String*
json_stringify (Sophon_VM *vm, Sophon_Value v,
		Sophon_JSONStringifyParams *p, StrStack *ss_bot)
{
	Sophon_String *str = NULL;
	Sophon_Result r;

	if (sophon_value_is_bool(v)) {
		Sophon_Bool b;

		b = sophon_value_to_bool(vm, v);
		str = b ? vm->true_str : vm->false_str;
	} else if (sophon_value_is_number(v)) {
		Sophon_Number n;

		if ((r = sophon_value_to_number(vm, v, &n)) != SOPHON_OK)
			return NULL;

		if (sophon_isinf(n) || sophon_isnan(n)) {
			str = vm->null_str;
		} else {
			str = sophon_dtostr(vm, n, SOPHON_D2STR_RADIX, 0);
		}
	} else if (sophon_value_is_string(v)) {
		if ((r = sophon_value_to_string(vm, v, &str)) != SOPHON_OK)
			return NULL;

		str = sophon_string_escape(vm, str);
		str = sophon_string_concat(vm, vm->quot_str, str);
		str = sophon_string_concat(vm, str, vm->quot_str);
	} else if (sophon_value_is_array(v)) {
		Sophon_Array *arr;
		Sophon_U32 id, len;
		Sophon_Value item, iv;
		Sophon_Bool first = SOPHON_TRUE;
		Sophon_String *item_str;
		StrStack *pss, ss;

		if ((r = sophon_value_to_array(vm, v, &arr)) != SOPHON_OK)
			return NULL;

		pss = ss_bot;
		while (pss) {
			if (pss->value == SOPHON_VALUE_GC(arr)) {
				sophon_throw(vm, vm->TypeError, "Structure is cyclical");
				return NULL;
			}
			pss = pss->bottom;
		}

		ss.bottom = ss_bot;
		sophon_value_set_array(vm, &ss.value, arr);

		str = vm->ls_str;

		len = sophon_array_get_length(vm, arr);
		for (id = 0; id < len; id++) {
			item = arr->v[id];

			if ((item == SOPHON_ARRAY_UNUSED) ||
						sophon_value_is_undefined(item))
				continue;

			sophon_value_set_int(vm, &iv, id);
			if ((r = json_item_replace(vm, p, v, iv, item, &item))
						!= SOPHON_OK)
				return NULL;

			if (sophon_value_is_undefined(item))
				continue;

			if (!first) {
				str = sophon_string_concat(vm, str, vm->comma_str);
				if (p->space)
					str = sophon_string_concat(vm, str, p->space);
			} else {
				first = SOPHON_FALSE;
			}

			if (!(item_str = json_stringify(vm, item, p, &ss)))
				return NULL;

			str = sophon_string_concat(vm, str, item_str);
		}

		str = sophon_string_concat(vm, str, vm->rs_str);
	} else if (sophon_value_is_object(v)) {
		Sophon_Value ov, item;
		Sophon_Int id;
		Sophon_PropIter pi;
		Sophon_Property *prop;
		Sophon_Object *obj;
		Sophon_String *item_str;
		Sophon_Bool first = SOPHON_TRUE;
		StrStack *pss, ss;

		r = sophon_value_call_prop(vm, v, SOPHON_VALUE_GC(vm->toJSON_str),
					NULL, 0, &ov, SOPHON_FL_NONE);

		if (r == SOPHON_NONE) {
			ov = v;
		} else if (r == SOPHON_OK) {
			str = json_stringify(vm, ov, p, ss_bot);
			goto end;
		} else {
			return NULL;
		}

		if ((r = sophon_value_to_object(vm, ov, &obj)) != SOPHON_OK)
			return NULL;

		pss = ss_bot;
		while (pss) {
			if (pss->value == SOPHON_VALUE_GC(obj)) {
				sophon_throw(vm, vm->TypeError, "Structure is cyclical");
				return NULL;
			}
			pss = pss->bottom;
		}

		ss.bottom = ss_bot;
		sophon_value_set_object(vm, &ss.value, obj);

		str = vm->lb_str;

		pi.bottom = vm->pi_stack;
		vm->pi_stack = &pi;

		for (id = 0; id < obj->prop_bucket; id++) {
			for (prop = obj->props[id]; prop; prop = pi.prop) {
				pi.prop = prop->next;

				if (prop->attrs & SOPHON_PROP_ATTR_ACCESSOR) {
					Sophon_AccessorProperty *aprop =
							(Sophon_AccessorProperty*)prop;
					if ((r = sophon_value_call(vm, aprop->getv, ov, NULL, 0,
										&item, 0)) != SOPHON_OK) {
						str = NULL;
						goto obj_end;
					}
				} else {
					item = prop->value;
				}

				if (sophon_value_is_undefined(item))
					continue;

				if ((r = json_item_replace(vm, p, ov,
									SOPHON_VALUE_GC(prop->name),
									item, &item)) != SOPHON_OK) {
					str = NULL;
					goto obj_end;
				}

				if (sophon_value_is_undefined(item))
					continue;

				if (!first) {
					str = sophon_string_concat(vm, str, vm->comma_str);
					if (p->space)
						str = sophon_string_concat(vm, str, p->space);
				} else {
					first = SOPHON_FALSE;
				}

				str = sophon_string_concat(vm, str, vm->quot_str);
				str = sophon_string_concat(vm, str,
							sophon_string_escape(vm, prop->name));
				str = sophon_string_concat(vm, str, vm->quot_str);
				str = sophon_string_concat(vm, str, vm->colons_str);

				if (!(item_str = json_stringify(vm, item, p, &ss))) {
					str = NULL;
					goto obj_end;
				}

				str = sophon_string_concat(vm, str, item_str);
			}
		}

		str = sophon_string_concat(vm, str, vm->rb_str);
obj_end:
		vm->pi_stack = pi.bottom;
	} else {
		str = vm->null_str;
	}
end:
	return str;
}

Sophon_String*
sophon_json_stringify (Sophon_VM *vm, Sophon_Value v,
		Sophon_JSONStringifyParams *p)
{
	Sophon_String *str;
	Sophon_Result r;

	SOPHON_ASSERT(vm);

	if (p && sophon_value_is_closure(p->replacer)) {
		Sophon_Object *obj;
		Sophon_Value key, holder;

		obj = sophon_object_create(vm);
		sophon_value_set_object(vm, &holder, obj);
		sophon_value_set_string(vm, &key, vm->empty_str);
		sophon_value_define_prop(vm, holder, key, v,
					SOPHON_VALUE_UNDEFINED,
					SOPHON_DATA_PROP_ATTR,
					SOPHON_FL_DATA_PROP);

		if ((r = json_item_replace(vm, p, holder, key, v, &v))
					!= SOPHON_OK)
			return NULL;
	}

	str = json_stringify(vm, v, p, NULL);

	return str;
}

