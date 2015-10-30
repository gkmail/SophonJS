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
#include <sophon_lex.h>
#include <sophon_function.h>
#include <sophon_module.h>
#include <sophon_closure.h>
#include <sophon_frame.h>
#include <sophon_stack.h>
#include <sophon_string.h>
#include <sophon_debug.h>

#include "sophon_parser_internal.h"
#include "sophon_js_parser.h"
#include "sophon_js_parser.c"

#if 0
#define DEBUG(a) SOPHON_INFO(a)
#else
#define DEBUG(a)
#endif

#define EXPR_IS_BIND(e)    (!SOPHON_VALUE_IS_UNDEFINED((e)->bind_namev))
#define EXPR_IS_REF(e)     (EXPR_IS_BIND(e) || ((e)->name_ops))
#define EXPR_INIT(e)       parser_expr_init(e)
#define EXPR_IS_EMPTY(e)   (!EXPR_IS_BIND(e) &&\
									!(e)->base_ops && !(e)->name_ops)
#define EXPR_TO_VALUE(e)   parser_expr_to_value(vm, e)
#define EXPR_MERGE(e1, e2) parser_expr_merge(e1, e2)
#define ADD_ANCHOR()       parser_anchor_create(vm)
#define ADD_CONST(v)       sophon_module_add_const(vm, p->module, v)
#define UNDEFINED(v)       sophon_value_set_undefined(vm, v)
#define GET_STRING(v)      ((Sophon_String*)SOPHON_VALUE_GET_GC(v))
#define APPEND             parser_append_ops
#define FUNC(n)            (&p->func_stack[p->func_top-1-(n)])
#define FRAME(n)           (&p->frame_stack[p->frame_top-1-(n)])

static const Sophon_U8 parser_op_model_table[] = {
#define OP_MODEL(name, model, stk) MODEL_##model,
	FOR_EACH_OP(OP_MODEL)
	MODEL_Op
};

static void parser_expr_to_value (Sophon_VM *vm, Sophon_ParserExpr *expr);

static void
token_name (Sophon_U16 tok)
{
	if (tok == SOPHON_ERR_EOF) {
		sophon_prerr("EOF");
	} else if (tok == '\'') {
		sophon_prerr("\'\'\'");
	} else if (sophon_isprint(tok)) {
		sophon_prerr("\'%c\'", tok);
	} else {
		const char *name;
		int id = tok - 256;

		SOPHON_ASSERT((id >= 0) && (id < SOPHON_ARRAY_SIZE(js_token_names)));

		name = js_token_names[id];
		sophon_prerr("\"%s\"", name);
	}
}

void
sophon_parser_error (Sophon_VM *vm, int type, Sophon_Location *loc,
			const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	sophon_parser_errorv(vm, type, loc, fmt, ap);
	va_end(ap);
}

void
sophon_parser_errorv (Sophon_VM *vm, int type, Sophon_Location *loc,
			const char *fmt, va_list ap)
{
	Sophon_ParserData *p = (Sophon_ParserData*)vm->parser_data;

	if (type == SOPHON_PARSER_ERROR) {
		p->flags |= SOPHON_PARSER_FL_ERROR;
		sophon_prerr("Error: ");
	} else {
		if (p->flags & SOPHON_PARSER_FL_NO_WARNING)
			return;

		sophon_prerr("Warning: ");
	}

	if (loc->first_line == loc->last_line) {
		if (loc->first_column == loc->last_column) {
			if (loc->first_column == 0)
				sophon_prerr("%d: ", loc->first_line);
			else
				sophon_prerr("%d.%d: ", loc->first_line, loc->first_column);
		} else {
			sophon_prerr("%d.%d-%d: ", loc->first_line, loc->first_column,
						loc->last_column);
		}
	} else {
		sophon_prerr("%d.%d-%d.%d: ", loc->first_line, loc->first_column,
					loc->last_line, loc->last_column);
	}

	if (fmt) {
		sophon_vprerr(fmt, ap);
	} else {
		Sophon_ParserStack *top = &p->stack[p->top-1];
		Sophon_U16 edge, sym;

		sophon_prerr("parse error, unexpected ");

		if (p->fetch.t != 0xFFFF)
			token_name(p->fetch.t);
		else
			sophon_prerr("token");

		edge = js_state_shifts[top->s];
		while (edge != 0xFFFF) {
			sym = js_edge_symbol[edge];
			if (sym != T_EPSILON) {
				sophon_prerr(", expect ");
				token_name(sym);
				break;
			}
			edge = js_edge_next[edge];
		}
	}

	sophon_prerr("\n");
}

/*Create a new anchor*/
static Sophon_ParserAnchor*
parser_anchor_create (Sophon_VM *vm)
{
	Sophon_ParserAnchor *a;

	a = (Sophon_ParserAnchor*)sophon_mm_alloc_ensure(vm,
				sizeof(Sophon_ParserAnchor));
	if (!a) {
		sophon_nomem();
		return NULL;
	}

	a->op.type = OP_anchor;
	return a;
}

/*Free an operation data*/
static void
parser_free_op (Sophon_VM *vm, Sophon_ParserOp *op)
{
	Sophon_ParserOpModel model;
	Sophon_U32 size = 0;

	model = sophon_parser_op_get_model(op->type);
	switch (model) {
#define GET_MODEL_SIZE(name)\
		case MODEL_##name: size = sizeof(Sophon_Parser##name); break;
		FOR_EACH_OP_MODEL(GET_MODEL_SIZE)
		default:
			break;
	}

	sophon_mm_free(vm, op, size);
}

/*Free an operation list*/
static void
parser_free_ops (Sophon_VM *vm, Sophon_ParserOp *ops)
{
	Sophon_ParserOp *o, *onext;

	o = ops;
	if (o) {
		do {
			onext = o->next;
			parser_free_op(vm, o);
			o = onext;
		} while (o != ops);
	}
}

/*Free case block list*/
static void
parser_free_cases (Sophon_VM *vm, Sophon_ParserCase *cases)
{
	Sophon_ParserCase *c, *cnext;

	c = cases;
	if (c) {
		do {
			cnext = c->next;
			parser_free_ops(vm, c->cond_ops);
			parser_free_ops(vm, c->job_ops);
			sophon_mm_free(vm, c, sizeof(Sophon_ParserCase));
			c = cnext;
		} while (c != cases);
	}
}

/*Clear expression*/
static void
parser_clear_expr (Sophon_VM *vm, Sophon_ParserExpr *expr)
{
	parser_free_ops(vm, expr->pre_ops);
	parser_free_ops(vm, expr->base_ops);
	parser_free_ops(vm, expr->name_ops);
	parser_free_ops(vm, expr->post_ops);
}

/*Clear a token value*/
static void
parser_clear_value (Sophon_VM *vm, Sophon_U16 t, Sophon_TokenValue *v)
{
	Sophon_TokenValueField field;

	if (t == 0xFFFF)
		return;

	field = sophon_token_value_get_field(t);

	switch (field) {
		case FIELD_ops:
			parser_free_ops(vm, v->ops);
			break;
		case FIELD_var:
			parser_free_ops(vm, v->var.ops);
			break;
		case FIELD_expr:
			parser_clear_expr(vm, &v->expr);
			break;
		case FIELD_cases:
			parser_free_cases(vm, v->cases);
			break;
		default:
			break;
	}
}

/*Append an operation to the list*/
static void
parser_append_op (Sophon_ParserOp **ops, Sophon_ParserOp *op)
{
	if (*ops) {
		op->prev = (*ops)->prev;
		op->next = *ops;
		(*ops)->prev->next = op;
		(*ops)->prev = op;
	} else {
		*ops = op;
		op->prev = op;
		op->next = op;
	}
}

/*Merge 2 operation lists*/
static void
parser_merge_ops (Sophon_ParserOp **ops1, Sophon_ParserOp **ops2)
{
	if (!*ops1) {
		*ops1 = *ops2;
	} else if (*ops2) {
		Sophon_ParserOp *h1, *h2, *t1, *t2;

		h1 = *ops1;
		t1 = (*ops1)->prev;
		h2 = *ops2;
		t2 = (*ops2)->prev;

		h1->prev = t2;
		t1->next = h2;
		h2->prev = t1;
		t2->next = h1;
	}

	*ops2 = NULL;
}

/*Append operations to the list*/
static void
parser_append_ops (Sophon_VM *vm, Sophon_ParserOp **ops, Sophon_U32 line, ...)
{
	Sophon_ParserData *p = (Sophon_ParserData*)vm->parser_data;
	Sophon_ParserOpModel model;
	Sophon_ParserOpType type;
	Sophon_ParserOp *op = NULL;
	va_list ap;

	if (!line && *ops) {
		line = (*ops)->prev->line;
	}

	va_start(ap, line);
	while (1) {
		type = va_arg(ap, Sophon_ParserOpType);
		if (type == -1)
			break;

		if (type == OP_anchor) {
			op = (Sophon_ParserOp*)va_arg(ap, Sophon_ParserAnchor*);
			op->line = line;
			parser_append_op(ops, op);
		} else if (type == OP_merge) {
			Sophon_ParserOp **m_ops;

			m_ops = va_arg(ap, Sophon_ParserOp**);
			if (m_ops)
				parser_merge_ops(ops, m_ops);

			if (!line && *ops) {
				line = (*ops)->prev->line;
			}
		} else if (type == OP_expr) {
			Sophon_ParserExpr *expr;

			expr = va_arg(ap, Sophon_ParserExpr*);
			parser_expr_to_value(vm, expr);
			parser_merge_ops(ops, &expr->pre_ops);
			parser_merge_ops(ops, &expr->base_ops);
			parser_merge_ops(ops, &expr->post_ops);
			if (expr->pre_pushed) {
				parser_append_ops(vm, ops, line, OP_mov, expr->pre_pushed,
							OP_pop, expr->pre_pushed, -1);
			}

			if (!line && *ops) {
				line = (*ops)->prev->line;
			}
		} else if (type == OP_int) {
			Sophon_Int i;

			i = va_arg(ap, Sophon_Int);
			if (i == 0) {
				parser_append_ops(vm, ops, line, OP_zero, -1);
			} else if (i == 1) {
				parser_append_ops(vm, ops, line, OP_one, -1);
			} else if ((i >= 0) && (i <= 0xFFFF)) {
				parser_append_ops(vm, ops, line, OP_number, i, -1);
			} else {
				Sophon_Int iid;
				Sophon_Value iv;

				sophon_value_set_int(vm, &iv, i);
				iid = ADD_CONST(iv);
				parser_append_ops(vm, ops, line, OP_const, iid, -1);
			}
		} else {

			model = sophon_parser_op_get_model(type);
			switch(model) {
				case MODEL_Op: {
					op = (Sophon_ParserOp*)sophon_mm_alloc_ensure(vm,
								sizeof(Sophon_ParserOp));
					break;
				}
				case MODEL_Const: {
					Sophon_ParserConst *cop;
					Sophon_Int n;
					cop = (Sophon_ParserConst*)sophon_mm_alloc_ensure(vm,
								sizeof(Sophon_ParserConst));
					n = va_arg(ap, Sophon_Int);

					SOPHON_ASSERT(n >= 0);

					if (n > 0xFFFF)
						sophon_fatal("too many constants");

					cop->id = n;
					op = (Sophon_ParserOp*)cop;
					break;
				}
				case MODEL_Jump: {
					Sophon_ParserJump *jop;
					jop = (Sophon_ParserJump*)sophon_mm_alloc_ensure(vm,
								sizeof(Sophon_ParserJump));
					jop->anchor = va_arg(ap, Sophon_ParserAnchor*);
					op = (Sophon_ParserOp*)jop;
					break;
				}
				case MODEL_Call: {
					Sophon_ParserCall *cop;
					Sophon_Int n;
					cop = (Sophon_ParserCall*)sophon_mm_alloc_ensure(vm,
								sizeof(Sophon_ParserCall));
					n = va_arg(ap, Sophon_Int);
					SOPHON_ASSERT(n >= 0);
					if (n > 0xFF)
						sophon_fatal("too many arguments");

					cop->argc = n;
					op = (Sophon_ParserOp*)cop;
					break;
				}
				case MODEL_Ref: {
					Sophon_ParserRef *rop;
					rop = (Sophon_ParserRef*)sophon_mm_alloc_ensure(vm,
								sizeof(Sophon_ParserRef));
					rop->anchor = va_arg(ap, Sophon_ParserAnchor*);
					op = (Sophon_ParserOp*)rop;
					break;
				}
				case MODEL_StackOp: {
					Sophon_ParserStackOp *sop;
					Sophon_Int n;
					sop = (Sophon_ParserStackOp*)sophon_mm_alloc_ensure(vm,
								sizeof(Sophon_ParserStackOp));
					n = va_arg(ap, Sophon_Int);
					SOPHON_ASSERT(n >= 0);
					if (n > 0xFF)
						sophon_fatal("pop count overflow");

					sop->n = n;
					op = (Sophon_ParserOp*)sop;
					break;
				}
				case MODEL_Label: {
					Sophon_ParserLabel *lop;
					Sophon_Int id;

					lop = (Sophon_ParserLabel*)sophon_mm_alloc_ensure(vm,
								sizeof(Sophon_ParserLabel));
					id = va_arg(ap, Sophon_Int);
					if (id > 0xFFFF)
						sophon_fatal("too many labels");

					lop->label_id = id;
					op = (Sophon_ParserOp*)lop;
					break;
				}
				case MODEL_Block: {
					Sophon_ParserBlock *bop;
					bop = (Sophon_ParserBlock*)sophon_mm_alloc_ensure(vm,
								sizeof(Sophon_ParserBlock));
					bop->brk_anchor  = va_arg(ap, Sophon_ParserAnchor*);
					bop->cont_anchor = va_arg(ap, Sophon_ParserAnchor*);
					op = (Sophon_ParserOp*)bop;
					break;
				}
				case MODEL_LongJump: {
					Sophon_ParserLongJump *lop;
					Sophon_Int id;

					lop = (Sophon_ParserLongJump*)sophon_mm_alloc_ensure(vm,
								sizeof(Sophon_ParserLongJump));
					id = va_arg(ap, Sophon_Int);
					if (id > 0xFFFF)
						sophon_fatal("jump offset overflow");

					lop->label_id = id;
					op = (Sophon_ParserOp*)lop;
					break;
				}
				case MODEL_Bind: {
					Sophon_ParserBind *bind;
					Sophon_Int id;
					bind = (Sophon_ParserBind*)sophon_mm_alloc_ensure(vm,
								sizeof(Sophon_ParserBind));
					id = va_arg(ap, Sophon_Int);
					if (id > 0xFFFF)
						sophon_fatal("too many constants");

					bind->name_id = id;
					op = (Sophon_ParserOp*)bind;
					break;
				}
				default:
					SOPHON_ASSERT(0);
					break;
			}

			op->type = type;
			op->line = line;
			parser_append_op(ops, op);
		}
	}

	va_end(ap);
}

/*Insert semicolon automatically*/
static void
parser_auto_semicolon (Sophon_VM *vm)
{
	Sophon_ParserData *p = (Sophon_ParserData*)vm->parser_data;

	DEBUG(("auto insert \';\'"));
	p->flags |= SOPHON_PARSER_FL_AUTO_SEMICOLON;
	p->semicolon = p->fetch;
	p->semicolon.t = ';';
}

/*Check if the semicolon should be inserted*/
static Sophon_Bool
parser_check_auto_semicolon (Sophon_VM *vm)
{
	Sophon_ParserData *p = (Sophon_ParserData*)vm->parser_data;
	Sophon_Bool insert = SOPHON_FALSE;

	DEBUG(("check auto semicolon"));
	if ((p->semicolon.t != 0xFFFF) ||
				(p->flags & SOPHON_PARSER_FL_AUTO_SEMICOLON))
		return SOPHON_FALSE;

	if ((p->flags & SOPHON_PARSER_FL_EOF) || 
				(p->fetch.t == '}') ||
				sophon_lex_has_line_term(vm)) {
		insert = SOPHON_TRUE;
	}

	/*Do not insert in for statement*/
	if (insert) {
		Sophon_ParserStack *stk = &p->stack[p->top - 1];
		while (stk >= p->stack) {

			if (stk->t == ')')
				break;
			if (stk->t == T_for) {
				insert = SOPHON_FALSE;
				break;
			}
			stk --;
		}
	}

	/*Do not insert an empty statement*/
	if (insert) {
		Sophon_ParserStack *stk = &p->stack[p->top - 1];
		Sophon_U16 edge;
		Sophon_U16 sym;
		Sophon_U16 dest;
		Sophon_U16 reduce;

		if (stk->t != N_PROGRAM) {
			edge = js_state_shifts[stk->s];
			while (edge != 0xFFFF) {
				sym = js_edge_symbol[edge];

				if (sym == ';') {
					dest = js_edge_dest[edge];
					reduce = js_state_reduce[dest];

					if (RV_GET_REDUCE(reduce) == R_EMPTY_STATEMENT)
						insert = SOPHON_FALSE;

					break;
				}

				edge = js_edge_next[edge];
			}
		}
	}

	return insert;
}

/*File end check*/
static Sophon_Bool
parser_shell_end (Sophon_VM *vm)
{
	Sophon_ParserData *p = (Sophon_ParserData*)vm->parser_data;
	Sophon_U16 *stack = NULL;
	Sophon_LexData *ld;
	Sophon_Int top, pop;
	Sophon_U16 state;
	Sophon_U16 edge;
	Sophon_U16 sym;
	Sophon_U16 tok;
	Sophon_U32 reduce;
	Sophon_Bool semicolon = SOPHON_FALSE;
	Sophon_Bool end = SOPHON_FALSE;

	if (!(p->flags & SOPHON_PARSER_FL_SHELL))
		return SOPHON_TRUE;

	stack = sophon_mm_alloc_ensure(vm,
					sizeof(Sophon_U16) * SOPHON_PARSER_STACK_SIZE);
	for (top = 0; top < p->top; top++) {
		stack[top] = p->stack[top].s;
	}

	state = stack[top - 1];

retry:
	while (1) {
		edge = js_state_shifts[state];
		while (edge != 0xFFFF) {
			sym = js_edge_symbol[edge];
			if (sym == T_EPSILON) {
				state = js_edge_dest[edge];
				stack[top++] = state;
				goto retry;
			}
			edge = js_edge_next[edge];
		}

		reduce = js_state_reduce[state];
		if (reduce == R_NONE) {
			if (!semicolon) {
				edge = js_state_shifts[state];
				while (edge != 0xFFFF) {
					sym = js_edge_symbol[edge];
					if (sym == ';') {
						semicolon = SOPHON_TRUE;
						state = js_edge_dest[edge];
						stack[top++] = state;
						goto retry;
					}
					edge = js_edge_next[edge];
				}
			}
			break;
		}

		if (!RV_GET_POP_FLAG(reduce))
			break;

		tok = RV_GET_TOKEN(reduce);
		if (tok == N_START) {
			end = SOPHON_TRUE;
			break;
		}

		pop = RV_GET_POP(reduce);
		top -= pop;

		state = stack[top - 1];
		edge = js_state_jumps[state];
		while (edge != 0xFFFF) {
			sym = js_edge_symbol[edge];
			if (sym == tok) {
				state = js_edge_dest[edge];
				stack[top++] = state;
				goto retry;
			}

			edge = js_edge_next[edge];
		}

		break;
	}

	if (stack) {
		sophon_mm_free(vm, stack,
					sizeof(Sophon_U16) * SOPHON_PARSER_STACK_SIZE);
	}

	if (end && semicolon) {
		if (!parser_check_auto_semicolon(vm))
			end = SOPHON_FALSE;
	}

	if (!end) {
		/*Reset the input*/
		SOPHON_INFO(("input not end, reset input"));
		ld = (Sophon_LexData*)vm->lex_data;
		ld->inp_func(ld->inp_data, NULL, 0);
		ld->flags |= SOPHON_LEX_FL_SHELL_CONT;
	}

	return end;
}

/*Solve parse error*/
static Sophon_Result
parser_solve_error (Sophon_VM *vm)
{
	/*Auto insert semicolon*/
	if (parser_check_auto_semicolon(vm)) {
		parser_auto_semicolon(vm);
		return SOPHON_OK;
	}

	return SOPHON_ERR_PARSE;
}

/*Initialize an expression*/
static void
parser_expr_init (Sophon_ParserExpr *expr)
{
	expr->pre_ops   = NULL;
	expr->post_ops  = NULL;
	expr->name_ops  = NULL;
	expr->base_ops  = NULL;
	expr->pre_pushed   = 0;
	expr->array_length = 1;
	expr->bind_namev = SOPHON_VALUE_UNDEFINED;
}

/*Change expression to value operation and store it to expr->base_ops*/
static void
parser_expr_to_value (Sophon_VM *vm, Sophon_ParserExpr *expr)
{
	Sophon_ParserData *p = (Sophon_ParserData*)vm->parser_data;

	if (EXPR_IS_BIND(expr)) {
		Sophon_Int id;

		id = ADD_CONST(expr->bind_namev);
		APPEND(vm, &expr->base_ops, expr->bind_line,
					OP_get_bind, id, -1);
		UNDEFINED(&expr->bind_namev);
	} else if (expr->name_ops) {
		parser_merge_ops(&expr->base_ops, &expr->name_ops);
		APPEND(vm, &expr->base_ops, 0, OP_get, -1);
	}
}

/*Merge pre_ops and post_ops of 2 expressions*/
static void
parser_expr_merge (Sophon_ParserExpr *e1, Sophon_ParserExpr *e2)
{
	parser_merge_ops(&e1->pre_ops, &e2->pre_ops);
	parser_merge_ops(&e1->post_ops, &e2->post_ops);
	e1->pre_pushed += e2->pre_pushed;
}

/*Push an object stack entry*/
static void
parser_push_obj (Sophon_VM *vm)
{
	Sophon_ParserData *p = (Sophon_ParserData*)vm->parser_data;
	Sophon_ParserObject *obj;

	if (p->obj_top >= SOPHON_PARSER_OBJECT_STACK_SIZE)
		sophon_fatal("object nesting overflow");

	obj = &p->obj_stack[p->obj_top++];
	sophon_hash_init(vm, &obj->prop_hash);
}

/*Pop an object stack entry*/
static void
parser_pop_obj (Sophon_VM *vm)
{
	Sophon_ParserData *p = (Sophon_ParserData*)vm->parser_data;
	Sophon_ParserObject *obj;

	SOPHON_ASSERT(p->obj_top > 0);

	obj = &p->obj_stack[--p->obj_top];
	sophon_hash_deinit(vm, &obj->prop_hash);
}

/*Push a new frame into the stack*/
static void
parser_push_frame (Sophon_VM *vm)
{
	Sophon_ParserData *p = (Sophon_ParserData*)vm->parser_data;
	Sophon_ParserFrame *frame;

	if (p->frame_top >= SOPHON_PARSER_FRAME_STACK_SIZE)
		sophon_fatal("frame nesting overflow");

	frame = &p->frame_stack[p->frame_top++];
	frame->ops = NULL;
}

/*Pop a frame from the stack*/
static void
parser_pop_frame (Sophon_VM *vm)
{
	Sophon_ParserData *p = (Sophon_ParserData*)vm->parser_data;
	Sophon_ParserFrame *frame;

	SOPHON_ASSERT(p->frame_top > 0);

	frame = &p->frame_stack[p->frame_top - 1];
	parser_free_ops(vm, frame->ops);
	p->frame_top--;
}

/*Push a new function into the stack*/
static void
parser_push_func (Sophon_VM *vm, Sophon_String *name)
{
	Sophon_ParserData *p = (Sophon_ParserData*)vm->parser_data;
	Sophon_Function *func, *container;
	Sophon_ParserFunc *stk;
	Sophon_U32 flags = 0;
	Sophon_U32 func_id;

	container = p->func_top ? p->func_stack[p->func_top - 1].func : NULL;
	if (!container && vm->stack)
		container = vm->stack->func;

	if (p->flags & SOPHON_PARSER_FL_STRICT)
		flags |= SOPHON_FUNC_FL_STRICT;
	if (container && (container->flags & SOPHON_FUNC_FL_STRICT))
		flags |= SOPHON_FUNC_FL_STRICT;

	func_id = sophon_module_add_func(vm, p->module, name, flags);
	SOPHON_ASSERT(func_id >= 0);

	func = sophon_module_get_func(p->module, func_id);
	if (p->func_top >= SOPHON_PARSER_FUNC_STACK_SIZE)
		sophon_fatal("function nesting overflow");

	stk = &p->func_stack[p->func_top++];

	stk->func = func;
	stk->frame_bottom = p->frame_top;

	parser_push_frame(vm);
}

#include "sophon_parser_gen_code.c"

/*Popup a function from the stack and generate byte code*/
static Sophon_Result
parser_pop_func (Sophon_VM *vm, Sophon_ParserOp **cmd_ops)
{
	Sophon_ParserData *p = (Sophon_ParserData*)vm->parser_data;
	Sophon_ParserOp *ops = NULL;
	Sophon_ParserFunc *func;
	Sophon_ParserFrame *frame;
	Sophon_Result r = SOPHON_OK;

	SOPHON_ASSERT(p->func_top > 0 && p->frame_top > 0);

	func  = FUNC(0);
	frame = FRAME(0);

	SOPHON_ASSERT(func->frame_bottom == p->frame_top - 1);

	parser_merge_ops(&ops, &frame->ops);
	parser_merge_ops(&ops, cmd_ops);

	r = parser_gen_code(vm, func->func, &ops);

	parser_free_ops(vm, ops);

	parser_pop_frame(vm);
	p->func_top--;
	return r;
}

/*Add a variant*/
static Sophon_Result
parser_add_var (Sophon_VM *vm, Sophon_Location *loc, Sophon_String *name)
{
	Sophon_ParserData *p = (Sophon_ParserData*)vm->parser_data;
	Sophon_Function *func;
	Sophon_Result r;
#if 0
	char *cstr;
	Sophon_U32 len;
#endif

	func = FUNC(0)->func;

	if ((func->flags & SOPHON_FUNC_FL_EVAL) &&
				!(func->flags & SOPHON_FUNC_FL_STRICT)) {
		Sophon_DeclFrame *frame;

		frame = (Sophon_DeclFrame*)vm->stack->var_env;
		SOPHON_ASSERT(frame);

		if (frame->func) {
			r = sophon_function_lookup_var(vm, frame->func, name);
			if (r >= 0)
				goto redef;
		}

		r = sophon_decl_frame_add_binding(vm, (Sophon_DeclFrame*)frame, name);
		if (r == SOPHON_OK)
			return r;
	} else {
		r = sophon_function_add_var(vm, func, SOPHON_FUNC_VAR, name);
		if (r == SOPHON_OK)
			return r;
	}

redef:
#if 0
	if (sophon_string_new_utf8_cstr(vm, name, &cstr, &len) >= 0) {
		parser_error(vm, PARSER_WARNING, loc,
					"variant \"%s\" has already been defined", cstr);
		sophon_string_free_utf8_cstr(vm, cstr, len);
	}
#endif
	return SOPHON_OK;
}

static Sophon_Result
parser_pre_inc_dec (Sophon_VM *vm, Sophon_Bool dec, Sophon_Location *loc,
			Sophon_TokenValue *vin,
			Sophon_TokenValue *vout)
{
	Sophon_ParserData *p = (Sophon_ParserData*)vm->parser_data;

	if (!EXPR_IS_REF(&vin->expr)) {
		sophon_parser_error(vm, SOPHON_PARSER_ERROR, loc,
					"type is not a reference");
		return SOPHON_ERR_PARSE;
	}

	*vout = *vin;

	if (EXPR_IS_BIND(&vout->expr)) {
		Sophon_Int id;

		id = ADD_CONST(vout->expr.bind_namev);
		parser_append_ops(vm, &vout->expr.pre_ops,
					loc->first_line,
					OP_get_bind, id,
					OP_one, dec ? OP_sub : OP_add,
					OP_put_bind, id,
					OP_pop, 1, -1);
	} else {
		Sophon_ParserAnchor *anchor1, *anchor2;

		anchor1 = ADD_ANCHOR();
		anchor2 = ADD_ANCHOR();

		APPEND(vm, &vout->expr.pre_ops, loc->first_line,
					OP_merge, &vout->expr.base_ops,
					OP_anchor, anchor1,
					OP_merge, &vout->expr.name_ops,
					OP_anchor, anchor2,
					OP_dup, 1, OP_dup, 1, OP_get,
					OP_one, dec ? OP_sub : OP_add,
					OP_put, OP_pop, 1, -1);

		APPEND(vm, &vout->expr.base_ops,
					loc->first_line, OP_dup_ref, anchor1, -1);
		APPEND(vm, &vout->expr.name_ops,
					loc->first_line, OP_dup_ref, anchor2, -1);

		vout->expr.pre_pushed += 2;
	}

	return SOPHON_OK;
}

static Sophon_Result
parser_post_inc_dec (Sophon_VM *vm, Sophon_Bool dec, Sophon_Location *loc,
			Sophon_TokenValue *vin, Sophon_TokenValue *vout)
{
	Sophon_ParserData *p = (Sophon_ParserData*)vm->parser_data;

	if (!EXPR_IS_REF(&vin->expr)) {
		sophon_parser_error(vm, SOPHON_PARSER_ERROR, loc,
					"type is not a reference");
		return SOPHON_ERR_PARSE;
	}

	*vout = *vin;

	if (EXPR_IS_BIND(&vout->expr)) {
		Sophon_Int id;

		id = ADD_CONST(vout->expr.bind_namev);
		parser_append_ops(vm, &vout->expr.post_ops,
					loc->first_line,
					OP_get_bind, id,
					OP_one, dec ? OP_sub : OP_add,
					OP_put_bind, id,
					OP_pop, 1, -1);
	} else {
		Sophon_ParserAnchor *anchor1, *anchor2;

		anchor1 = parser_anchor_create(vm);
		anchor2 = parser_anchor_create(vm);

		parser_append_ops(vm, &vout->expr.pre_ops,
					loc->first_line,
					OP_merge, &vout->expr.base_ops,
					OP_anchor, anchor1,
					OP_merge, &vout->expr.name_ops,
					OP_anchor, anchor2, -1);
		parser_append_ops(vm, &vout->expr.base_ops,
					loc->first_line, OP_dup_ref, anchor1, -1);
		parser_append_ops(vm, &vout->expr.name_ops,
					loc->first_line, OP_dup_ref, anchor2, -1);
		parser_append_ops(vm, &vout->expr.post_ops,
					loc->first_line,
					OP_dup_ref, anchor1,
					OP_dup_ref, anchor2,
					OP_dup_ref, anchor1,
					OP_dup_ref, anchor2,
					OP_get,	OP_one, dec ? OP_sub : OP_add,
					OP_put, OP_pop, 3, -1);

		vout->expr.pre_pushed += 2;
	}

	return SOPHON_OK;
}

static void
parser_for_loop (Sophon_VM *vm, Sophon_Location *loc,
			Sophon_ParserOp **ops_init,
			Sophon_ParserExpr *e2,
			Sophon_ParserExpr *e3,
			Sophon_ParserOp **ops_loop,
			Sophon_TokenValue *v)
{
	Sophon_ParserAnchor *begin, *brk, *cont;

	begin = ADD_ANCHOR();
	brk   = ADD_ANCHOR();
	cont  = ADD_ANCHOR();

	v->ops = NULL;
	APPEND(vm, &v->ops, loc->first_line,
				OP_block_begin, brk, cont, OP_undef, OP_store,
				OP_merge, ops_init, OP_anchor, begin, -1);

	if (e2) {
		APPEND(vm, &v->ops, loc->first_line,
					OP_expr, e2, OP_jf, brk, -1);
	}

	APPEND(vm, &v->ops, loc->first_line, OP_merge, ops_loop,
				OP_anchor, cont, -1);

	if (e3) {
		APPEND(vm, &v->ops, loc->first_line,
					OP_expr, e3, OP_pop, 1, -1);
	}

	APPEND(vm, &v->ops, loc->first_line, OP_jmp, begin,
				OP_anchor, brk, OP_block_end, -1);
}

static Sophon_Result
parser_for_in (Sophon_VM *vm, Sophon_Location *loc,
			Sophon_ParserOp **pre_ops,
			Sophon_ParserExpr *el,
			Sophon_ParserExpr *er,
			Sophon_ParserOp **ops,
			Sophon_TokenValue *v)
{
	Sophon_ParserData *p = (Sophon_ParserData*)vm->parser_data;
	Sophon_ParserAnchor *brk, *cont;
	Sophon_Int pn;

	if (!EXPR_IS_REF(el)) {
		sophon_parser_error(vm, SOPHON_PARSER_ERROR, loc,
					"type is not a reference");
		return SOPHON_ERR_PARSE;
	}

	brk  = ADD_ANCHOR();
	cont = ADD_ANCHOR();

	v->ops = NULL;
	APPEND(vm, &v->ops, loc->first_line,
				OP_block_begin, brk, cont, OP_undef, OP_store,
				OP_merge, pre_ops, -1);

	if (!EXPR_IS_BIND(el)) {
		APPEND(vm, &v->ops, loc->first_line,
					OP_merge, el->base_ops,
					OP_merge, el->name_ops, -1);
	}

	APPEND(vm, &v->ops, loc->first_line,
				OP_expr, er, OP_for_in, brk,
				OP_anchor, cont, -1);

	if (EXPR_IS_BIND(el)) {
		Sophon_Int id;

		id = ADD_CONST(el->bind_namev);
		APPEND(vm, &v->ops, 0, OP_put_bind, id, -1);
		pn = 2;
	} else {
		APPEND(vm, &v->ops, 0, OP_dup, 3, OP_dup, 3,
					OP_dup, 2, OP_put, OP_pop, 3, -1);
		pn = 4;
	}

	APPEND(vm, &v->ops, 0, OP_merge, ops,
				OP_for_next, OP_anchor, brk, OP_pop, pn, OP_block_end, -1);

	return SOPHON_OK;
}

static void
parser_try_catch (Sophon_VM *vm, Sophon_Location *loc,
			Sophon_ParserOp **try_ops,
			Sophon_ParserOp **catch_ops,
			Sophon_ParserOp **final_ops,
			Sophon_TokenValue *v)
{
	Sophon_ParserAnchor *anchor1, *anchor2;

	anchor1 = ADD_ANCHOR();
	anchor2 = ADD_ANCHOR();

	v->ops = NULL;
	APPEND(vm, &v->ops, loc->first_line,
				OP_try_begin, anchor1,
				OP_merge, try_ops,
				OP_try_end, anchor2,
				OP_anchor, anchor1,
				OP_merge, catch_ops,
				OP_anchor, anchor2,
				OP_merge, final_ops, -1);
}

static Sophon_ParserCase*
parser_case_create (Sophon_VM *vm, Sophon_Location *loc,
			Sophon_ParserExpr *expr,
			Sophon_ParserOp **ops)
{
	Sophon_ParserCase *c;

	c = (Sophon_ParserCase*)sophon_mm_alloc_ensure(vm,
				sizeof(Sophon_ParserCase));

	c->prev = c;
	c->next = c;
	c->cond_ops = NULL;
	c->job_ops  = NULL;

	if (expr)
		APPEND(vm, &c->cond_ops, loc->first_line,
					OP_expr, expr, -1);
	if (ops) {
		c->job_ops = *ops;
		*ops = NULL;
	}

	return c;
}

static void
parser_case_merge (Sophon_ParserCase *c1, Sophon_ParserCase *c2)
{
	Sophon_ParserCase *h1, *h2, *t1, *t2;

	h1 = c1;
	t1 = c1->prev;
	h2 = c2;
	t2 = c2->prev;

	h1->prev = t2;
	t1->next = h2;
	h2->prev = t1;
	t2->next = h1;
}

static Sophon_Result
parser_reduce (Sophon_VM *vm, Sophon_U16 rid, Sophon_Int pop,
			Sophon_TokenValue *v)
{
	Sophon_ParserData *p = (Sophon_ParserData*)vm->parser_data;
	Sophon_ParserStack *top = &p->stack[p->top - 1];
	Sophon_Result r;

#define T(n) top[(n)+1-pop].t
#define V(n) top[(n)+1-pop].v
#define L(n) top[(n)+1-pop].l

	switch((ParserReduceType)rid) {
		case R_COPY_0:
			*v = V(0);
			break;
		case R_COPY_1:
			*v = V(1);
			break;
		case R_NULL:
			sophon_value_set_null(vm, &v->v);
			break;
		case R_KEY_TO_ID: {
			Sophon_String *str = NULL;

#define KEY_TO_ID(name) case T_##name: str = vm->name##_str; break;
			switch (T(0)) {
				SOPHON_FOR_EACH_KEYWORD(KEY_TO_ID)
				default:
					SOPHON_ASSERT(0);
			}

			sophon_value_set_string(vm, &v->v, str);
			break;
		}
		case R_EXPR_CONST: {
			EXPR_INIT(&v->expr);

			if (SOPHON_VALUE_IS_INT(V(0).v)) {
				Sophon_Int i = SOPHON_VALUE_GET_INT(V(0).v);
				APPEND(vm, &v->expr.base_ops, L(0).first_line,
						OP_int, i, -1);
			} else {
				Sophon_Int id = ADD_CONST(V(0).v);
				APPEND(vm, &v->expr.base_ops, L(0).first_line,
						OP_const, id, -1);
			}

			break;
		}
		case R_EXPR_TRUE: {
			EXPR_INIT(&v->expr);
			APPEND(vm, &v->expr.base_ops, L(0).first_line,
						OP_true, -1);
			break;
		}
		case R_EXPR_FALSE: {
			EXPR_INIT(&v->expr);
			APPEND(vm, &v->expr.base_ops, L(0).first_line,
						OP_false, -1);
			break;
		}
		case R_EXPR_NULL: {
			EXPR_INIT(&v->expr);
			APPEND(vm, &v->expr.base_ops, L(0).first_line,
						OP_null, -1);
			break;
		}
		case R_EXPR_ID: {
			EXPR_INIT(&v->expr);
			v->expr.bind_namev = V(0).v;
			v->expr.bind_line  = L(0).first_line;
			break;
		}
		case R_EXPR_REGEXP: {
			Sophon_Int re, body, flags = -1;

			EXPR_INIT(&v->expr);
			re   = ADD_CONST(SOPHON_VALUE_GC(vm->RegExp_str));
			body = ADD_CONST(V(0).re.bodyv);

			if (!SOPHON_VALUE_IS_UNDEFINED(V(0).re.flagsv)) {
				flags = ADD_CONST(V(0).re.flagsv);
			}

			APPEND(vm, &v->expr.base_ops, L(0).first_line,
						OP_get_bind, re, OP_const, body, -1);
			if (flags != -1) {
				APPEND(vm, &v->expr.base_ops, 0,
							OP_const, flags, -1);
			}
			APPEND(vm, &v->expr.base_ops, 0, OP_new,
						(flags == -1) ? 1 : 2, -1);
			break;
		}
		case R_EXPR_THIS: {
			EXPR_INIT(&v->expr);
			APPEND(vm, &v->expr.base_ops, L(0).first_line,
						OP_this, -1);
			break;
		}
		case R_GET_PROP: {
			EXPR_TO_VALUE(&V(0).expr);
			EXPR_TO_VALUE(&V(2).expr);
			EXPR_MERGE(&V(0).expr, &V(2).expr);
			APPEND(vm, &V(0).expr.name_ops, L(0).first_line,
						OP_merge, &V(2).expr.base_ops, -1);
			*v = V(0);
			break;
		}
		case R_MEMBER: {
			Sophon_Int id;

			id = ADD_CONST(V(2).v);
			EXPR_TO_VALUE(&V(0).expr);
			APPEND(vm, &V(0).expr.name_ops, L(2).first_line,
						OP_const, id, -1);
			*v = V(0);
			break;
		}
		case R_NEW: {
			EXPR_TO_VALUE(&V(1).expr);
			EXPR_MERGE(&V(1).expr, &V(2).expr);
			APPEND(vm, &V(1).expr.base_ops, L(0).first_line,
						OP_merge, &V(2).expr.base_ops,
						OP_new, V(2).expr.array_length, -1);
			*v = V(1);
			break;
		}
		case R_CALL: {
			Sophon_ParserOpType type;

			if (EXPR_IS_BIND(&V(0).expr) || !V(0).expr.name_ops) {
				EXPR_TO_VALUE(&V(0).expr);
				type = OP_call;
			} else {
				APPEND(vm, &V(0).expr.base_ops, 0, OP_dup, 0,
							OP_merge, &V(0).expr.name_ops, OP_get, -1);
				type = OP_this_call;
			}

			EXPR_TO_VALUE(&V(1).expr);
			EXPR_MERGE(&V(0).expr, &V(1).expr);
			APPEND(vm, &V(0).expr.base_ops, L(0).first_line,
						OP_merge, &V(1).expr.base_ops,
						type, V(1).expr.array_length, -1);
			*v = V(0);
			break;
		}
		case R_PRE_INC: {
			r = parser_pre_inc_dec(vm, SOPHON_FALSE, &L(1), &V(1), v);
			if (r != SOPHON_OK)
				return r;
			break;
		}
		case R_PRE_DEC: {
			r = parser_pre_inc_dec(vm, SOPHON_TRUE, &L(1), &V(1), v);
			if (r != SOPHON_OK)
				return r;
			break;
		}
		case R_POST_INC: {
			r = parser_post_inc_dec(vm, SOPHON_FALSE, &L(0), &V(0), v);
			if (r != SOPHON_OK)
				return r;
			break;
		}
		case R_POST_DEC: {
			r = parser_post_inc_dec(vm, SOPHON_TRUE, &L(0), &V(0), v);
			if (r != SOPHON_OK)
				return r;
			break;
		}
		case R_AND: {
			Sophon_ParserAnchor *anchor;

			anchor = ADD_ANCHOR();
			EXPR_TO_VALUE(&V(0).expr);
			EXPR_TO_VALUE(&V(2).expr);
			EXPR_MERGE(&V(0).expr, &V(2).expr);
			APPEND(vm, &V(0).expr.base_ops, L(1).first_line,
						OP_dup, 0, OP_jf, anchor, OP_pop, 1,
						OP_merge, &V(2).expr.base_ops,
						OP_anchor, anchor, -1);
			*v = V(0);
			break;
		}
		case R_OR: {
			Sophon_ParserAnchor *anchor;

			anchor = ADD_ANCHOR();
			EXPR_TO_VALUE(&V(0).expr);
			EXPR_TO_VALUE(&V(2).expr);
			EXPR_MERGE(&V(0).expr, &V(2).expr);
			APPEND(vm, &V(0).expr.base_ops, L(1).first_line,
						OP_dup, 0, OP_jt, anchor, OP_pop, 1,
						OP_merge, &V(2).expr.base_ops,
						OP_anchor, anchor, -1);
			*v = V(0);
			break;
		}
		case R_COND: {
			Sophon_ParserAnchor *anchor1;
			Sophon_ParserAnchor *anchor2;

			anchor1 = ADD_ANCHOR();
			anchor2 = ADD_ANCHOR();

			EXPR_TO_VALUE(&V(0).expr);
			EXPR_TO_VALUE(&V(2).expr);
			EXPR_TO_VALUE(&V(4).expr);
			EXPR_MERGE(&V(0).expr, &V(2).expr);
			EXPR_MERGE(&V(0).expr, &V(4).expr);
			APPEND(vm, &V(0).expr.base_ops,
						L(1).first_line,
						OP_jf, anchor1,
						OP_merge, &V(2).expr.base_ops,
						OP_jmp, anchor2,
						OP_stack, 1,
						OP_anchor, anchor1,
						OP_merge, &V(4).expr.base_ops,
						OP_anchor, anchor2, -1);
			*v = V(0);
			break;
		}
		case R_ASSIGN_EXPR: {
			Sophon_ParserOpType type = V(1).i;

			if (!EXPR_IS_REF(&V(0).expr)) {
				sophon_parser_error(vm, SOPHON_PARSER_ERROR, &L(0),
							"type is not a reference");
				return SOPHON_ERR_PARSE;
			}

			EXPR_TO_VALUE(&V(2).expr);
			EXPR_MERGE(&V(0).expr, &V(2).expr);

			if (EXPR_IS_BIND(&V(0).expr)) {
				Sophon_Int id;

				id = ADD_CONST(V(0).expr.bind_namev);

				if (type == -1) {
					APPEND(vm, &V(0).expr.base_ops,
								L(1).first_line,
								OP_merge, &V(2).expr.base_ops,
								OP_put_bind, id, -1);
				} else {
					APPEND(vm, &V(0).expr.base_ops,
								L(1).first_line,
								OP_get_bind, id,
								OP_merge, &V(2).expr.base_ops, type,
								OP_put_bind, id, -1);
				}

				UNDEFINED(&V(0).expr.bind_namev);
			} else {
				if (type == -1) {
					APPEND(vm, &V(0).expr.base_ops,
								L(1).first_line,
								OP_merge, &V(0).expr.name_ops,
								OP_merge, &V(2).expr.base_ops,
								OP_put, OP_mov, 2, OP_pop, 2, -1);
				} else {
					APPEND(vm, &V(0).expr.base_ops,
								L(1).first_line,
								OP_merge, &V(0).expr.name_ops,
								OP_dup, 1, OP_dup, 1, OP_get,
								OP_merge, &V(2).expr.base_ops, type,
								OP_put, OP_mov, 2, OP_pop, 2, -1);
				}
			}

			*v = V(0);
			break;
		}
		case R_COMMA_EXPR: {
			EXPR_TO_VALUE(&V(0).expr);
			EXPR_TO_VALUE(&V(2).expr);
			EXPR_MERGE(&V(0).expr, &V(2).expr);
			APPEND(vm, &V(0).expr.base_ops, L(1).first_line,
						OP_pop, 1,
						OP_merge, &V(2).expr.base_ops, -1);
			*v = V(0);
			break;
		}
		case R_NO_ARGUMENT: {
			EXPR_INIT(&v->expr);
			v->expr.array_length = 0;
			break;
		}
		case R_FIRST_ARGUMENT: {
			EXPR_TO_VALUE(&V(0).expr);
			*v = V(0);
			v->expr.array_length = 1;
			break;
		}
		case R_APPEND_ARGUMENT: {
			EXPR_TO_VALUE(&V(0).expr);
			EXPR_TO_VALUE(&V(2).expr);
			EXPR_MERGE(&V(0).expr, &V(2).expr);
			APPEND(vm, &V(0).expr.base_ops, 0,
						OP_merge, &V(2).expr.base_ops, -1);
			V(0).expr.array_length++;

			if (V(0).expr.array_length > 0xFF)
				sophon_fatal("too many arguments");

			*v = V(0);
			break;
		}
		case R_ZERO: {
			v->i = 0;
			break;
		}
		case R_ONE: {
			v->i = 1;
			break;
		}
		case R_EMPTY_EXPR: {
			EXPR_INIT(&v->expr);
			break;
		}
		case R_ARRAY_INC_INDEX: {
			v->i = V(0).i + 1;
			break;
		}
		case R_ARRAY_SET_LENGTH: {
			Sophon_Int aid;
			Sophon_Int n;

			aid = ADD_CONST(SOPHON_VALUE_GC(vm->Array_str));
			n = V(1).i;

			EXPR_INIT(&v->expr);
			APPEND(vm, &v->expr.base_ops, L(0).first_line,
						OP_get_bind, aid,
						OP_new, 0, -1);

			if (n > 0) {
				Sophon_Int lid;

				lid = ADD_CONST(SOPHON_VALUE_GC(vm->length_str));

				APPEND(vm, &v->expr.base_ops, L(1).first_line,
							OP_const, lid, OP_int, n,
							OP_put, OP_pop, 2, -1);
			}
			break;
		}
		case R_ARRAY_RESIZE: {
			Sophon_Int lid;
			Sophon_Int n;

			if (V(3).i > 0) {
				n = V(3).i + V(1).expr.array_length;

				lid = ADD_CONST(SOPHON_VALUE_GC(vm->length_str));

				APPEND(vm, &V(1).expr.base_ops,
							L(3).first_line,
							OP_const, lid, OP_int, n,
							OP_put, OP_pop, 2, -1);
			}

			*v = V(1);
			break;
		}
		case R_ARRAY_FIRST_INDEX: {
			Sophon_Int aid;
			Sophon_Int n;

			n = V(0).i;

			aid = ADD_CONST(SOPHON_VALUE_GC(vm->Array_str));

			EXPR_INIT(&v->expr);
			EXPR_TO_VALUE(&V(1).expr);
			EXPR_MERGE(&v->expr, &V(1).expr);
			APPEND(vm, &v->expr.base_ops, L(1).first_line,
						OP_get_bind, aid,
						OP_new, 0,
						OP_int, n, OP_merge, &V(1).expr.base_ops,
						OP_put, OP_pop, 2, -1);

			if ((n + 1) > 0xFFFF)
				sophon_fatal("too many array elements");

			v->expr.array_length = n + 1;
			break;
		}
		case R_ARRAY_SET_ELEMENT: {
			Sophon_Int n;

			n = V(0).expr.array_length;
			n += V(2).i;

			EXPR_TO_VALUE(&V(3).expr);
			EXPR_MERGE(&v->expr, &V(3).expr);
			APPEND(vm, &V(0).expr.base_ops, L(3).first_line,
						OP_int, n, OP_merge, &V(3).expr.base_ops,
						OP_put, OP_pop, 2, -1);

			V(0).expr.array_length = n + 1;
			*v = V(0);
			break;
		}
		case R_OBJECT_BEGIN: {
			parser_push_obj(vm);
			break;
		}
		case R_OBJECT_NEW: {
			Sophon_Int id;

			id = ADD_CONST(SOPHON_VALUE_GC(vm->Object_str));
			EXPR_INIT(&v->expr);

			if (!EXPR_IS_EMPTY(&V(2).expr)) {
				EXPR_TO_VALUE(&V(2).expr);
				EXPR_MERGE(&v->expr, &V(2).expr);
			}

			APPEND(vm, &v->expr.base_ops, L(0).first_line,
						OP_get_bind, id, OP_new, 0,
						OP_merge, &V(2).expr.base_ops, -1);
			parser_pop_obj(vm);
			break;
		}
		case R_PROP: {
			Sophon_Int id;
			Sophon_ParserObject *obj = p->obj_stack;
			Sophon_HashEntry *ent;
			Sophon_Value namev;

			if (T(0) == T_get) {
				namev = SOPHON_VALUE_GC(vm->get_str);
			} else if (T(0) == T_set) {
				namev = SOPHON_VALUE_GC(vm->set_str);
			} else {
				namev = V(0).v;
			}

			r = sophon_hash_add(vm, &obj->prop_hash,
						sophon_value_key,
						sophon_value_equal,
						(Sophon_Ptr)namev,
						&ent);
			if (r == SOPHON_OK) {
				ent->value = (Sophon_Ptr)PROP_DEFINE_DATA;
			} else if (r == SOPHON_NONE) {
				Sophon_UIntPtr old = (Sophon_UIntPtr)ent->value;
				Sophon_Bool strict = FUNC(0)->func->flags &
						SOPHON_FUNC_FL_STRICT;
				Sophon_Bool ok = SOPHON_TRUE;

				if (old & (PROP_DEFINE_SET|PROP_DEFINE_GET)) {
					ok = SOPHON_FALSE;
				} else if (strict && (old & PROP_DEFINE_DATA)) {
					ok = SOPHON_FALSE;
				}

				if (!ok) {
					sophon_parser_error(vm, SOPHON_PARSER_ERROR, &L(0),
								"property has already been defined");
					return SOPHON_ERR_PARSE;
				}

				ent->value = (Sophon_Ptr)(PROP_DEFINE_DATA|old);
			}

			id = ADD_CONST(namev);

			EXPR_TO_VALUE(&V(2).expr);
			EXPR_INIT(&v->expr);
			EXPR_MERGE(&v->expr, &V(2).expr);
			APPEND(vm, &v->expr.base_ops, L(0).first_line,
						OP_const, id, OP_merge, &V(2).expr.base_ops,
						OP_put, OP_pop, 2, -1);
			break;
		}
		case R_PROP_GET_BEGIN: {
			parser_push_func(vm, NULL);
			p->flags |= SOPHON_PARSER_FL_FUNC_BEGIN;
			break;
		}
		case R_PROP_GET: {
			Sophon_Int pid;
			Sophon_Function *func = FUNC(0)->func;
			Sophon_ParserObject *obj = p->obj_stack;
			Sophon_HashEntry *ent;

			r = sophon_hash_add(vm, &obj->prop_hash,
						sophon_value_key,
						sophon_value_equal,
						(Sophon_Ptr)V(1).v,
						&ent);
			if (r == SOPHON_OK) {
				ent->value = (Sophon_Ptr)PROP_DEFINE_GET;
			} else if (r == SOPHON_NONE) {
				Sophon_UIntPtr old = (Sophon_UIntPtr)ent->value;
				Sophon_Bool ok = SOPHON_TRUE;

				if (old & PROP_DEFINE_GET) {
					ok = SOPHON_FALSE;
				}

				if (!ok) {
					sophon_parser_error(vm, SOPHON_PARSER_ERROR, &L(1),
								"property has already been defined");
					return SOPHON_ERR_PARSE;
				}

				ent->value = (Sophon_Ptr)(PROP_DEFINE_GET|old);
			}

			pid = ADD_CONST(V(1).v);

			EXPR_INIT(&v->expr);
			APPEND(vm, &v->expr.base_ops, L(1).first_line,
						OP_const, pid, OP_closure, func->id,
						OP_prop_get, -1);
			if ((r = parser_pop_func(vm, &V(6).ops)) < 0)
				return r;
			break;
		}
		case R_PROP_SET_BEGIN: {
			Sophon_Function *func;
			Sophon_String *str;

			parser_push_func(vm, NULL);

			func = FUNC(0)->func;
			str = (Sophon_String*)SOPHON_VALUE_GET_GC(V(3).v);
			sophon_function_add_var(vm, func, SOPHON_FUNC_ARG, str);
			p->flags |= SOPHON_PARSER_FL_FUNC_BEGIN;
			break;
		}
		case R_PROP_SET: {
			Sophon_Int pid;
			Sophon_Function *func = FUNC(0)->func;
			Sophon_ParserObject *obj = p->obj_stack;
			Sophon_HashEntry *ent;

			r = sophon_hash_add(vm, &obj->prop_hash,
						sophon_value_key,
						sophon_value_equal,
						(Sophon_Ptr)V(1).v,
						&ent);
			if (r == SOPHON_OK) {
				ent->value = (Sophon_Ptr)PROP_DEFINE_SET;
			} else if (r == SOPHON_NONE) {
				Sophon_UIntPtr old = (Sophon_UIntPtr)ent->value;
				Sophon_Bool ok = SOPHON_TRUE;

				if (old & PROP_DEFINE_SET) {
					ok = SOPHON_FALSE;
				}

				if (!ok) {
					sophon_parser_error(vm, SOPHON_PARSER_ERROR, &L(1),
								"property has already been defined");
					return SOPHON_ERR_PARSE;
				}

				ent->value = (Sophon_Ptr)(PROP_DEFINE_SET|old);
			}

			pid = ADD_CONST(V(1).v);

			EXPR_INIT(&v->expr);
			APPEND(vm, &v->expr.base_ops, L(1).first_line,
						OP_const, pid, OP_closure, func->id,
						OP_prop_set, -1);
			if ((r = parser_pop_func(vm, &V(7).ops)) < 0)
				return r;
			break;
		}
		case R_EMPTY_STATEMENT:
		case R_EMPTY_OPS: {
			v->ops = NULL;
			APPEND(vm, &v->ops, L(0).first_line, OP_undef, OP_store, -1);
			break;
		}
		case R_MERGE_OPS: {
			parser_merge_ops(&V(0).ops, &V(1).ops);
			*v = V(0);
			break;
		}
		case R_MERGE_OPS_2: {
			parser_merge_ops(&V(0).ops, &V(2).ops);
			*v = V(0);
			break;
		}
		case R_CONTINUE: {
			v->ops = NULL;
			APPEND(vm, &v->ops, L(0).first_line,
						OP_continue, 0xFFFF, -1);
			break;
		}
		case R_CONTINUE_LABEL: {
			Sophon_Int id;

			v->ops = NULL;
			id = ADD_CONST(V(1).v);
			APPEND(vm, &v->ops, L(0).first_line,
						OP_continue, id, -1);
			break;
		}
		case R_BREAK: {
			v->ops = NULL;
			APPEND(vm, &v->ops, L(0).first_line,
						OP_break, 0xFFFF, -1);
			break;
		}
		case R_BREAK_LABEL: {
			Sophon_Int id;

			v->ops = NULL;
			id = ADD_CONST(V(1).v);
			APPEND(vm, &v->ops, L(0).first_line,
						OP_break, id, -1);
			break;
		}
		case R_RETURN: {
			if (FUNC(0)->func->flags & SOPHON_FUNC_FL_EVAL) {
				sophon_parser_error(vm, SOPHON_PARSER_ERROR, &L(0),
						"\"return\" is not in a function");
				return SOPHON_ERR_PARSE;
			}

			v->ops = NULL;
			APPEND(vm, &v->ops, L(0).first_line,
						OP_undef, OP_return, -1);
			break;
		}
		case R_RETURN_VALUE: {
			if (FUNC(0)->func->flags & SOPHON_FUNC_FL_EVAL) {
				sophon_parser_error(vm, SOPHON_PARSER_ERROR, &L(0),
						"\"return\" is not in a function");
				return SOPHON_ERR_PARSE;
			}

			v->ops = NULL;
			APPEND(vm, &v->ops, L(0).first_line,
						OP_expr, &V(1).expr, OP_return, -1);
			break;
		}
		case R_VAR_INIT:
		case R_VAR: {
			Sophon_String *name = GET_STRING(V(0).v);
			Sophon_Bool strict = FUNC(0)->func->flags & SOPHON_FUNC_FL_STRICT;
			Sophon_Int id;

			if (strict &&
					((name == vm->arguments_str) || (name == vm->eval_str))) {
				char *buf;
				Sophon_U32 len;

				if (sophon_string_new_utf8_cstr(vm, name, &buf, &len) >= 0) {
					sophon_parser_error(vm, SOPHON_PARSER_ERROR, &L(0),
								"\"%s\" cannot be a variant name",
								buf);
					sophon_string_free_utf8_cstr(vm, buf, len);
				}
				return SOPHON_ERR_PARSE;
			}

			parser_add_var(vm, &L(0), name);
			v->var.ops   = NULL;
			v->var.namev = V(0).v;

			if (rid == R_VAR_INIT) {
				id = ADD_CONST(SOPHON_VALUE_GC(name));
				APPEND(vm, &v->var.ops, L(0).first_line,
							OP_expr, &V(2).expr,
							OP_put_bind, id,
							OP_pop, 1, -1);
			}
			break;
		}
		case R_VAR_STATEMENT: {
			*v = V(1);
			APPEND(vm, &v->ops, L(0).first_line, OP_undef, OP_store, -1);
			break;
		}
		case R_EXPR_STATEMENT: {
			v->ops = NULL;
			APPEND(vm, &v->ops, L(0).first_line,
						OP_expr, &V(0).expr,
						OP_store, -1);
			break;
		}
		case R_IF_STATEMENT: {
			Sophon_ParserAnchor *anchor;

			anchor = ADD_ANCHOR();
			v->ops = NULL;
			APPEND(vm, &v->ops, L(0).first_line,
						OP_expr, &V(2).expr,
						OP_jf, anchor, OP_merge, &V(4).ops,
						OP_anchor, anchor, -1);
			break;
		}
		case R_IF_ELSE_STATEMENT: {
			Sophon_ParserAnchor *anchor1, *anchor2;

			anchor1 = ADD_ANCHOR();
			anchor2 = ADD_ANCHOR();

			v->ops = NULL;
			APPEND(vm, &v->ops, L(0).first_line,
						OP_expr, &V(2).expr,
						OP_jf, anchor1, OP_merge, &V(4).ops, OP_jmp, anchor2,
						OP_anchor, anchor1, OP_merge, &V(6).expr,
						OP_anchor, anchor2, -1);
			break;
		}
		case R_DO_STATEMENT: {
			Sophon_ParserAnchor *begin, *brk, *cont;

			begin = ADD_ANCHOR();
			brk   = ADD_ANCHOR();
			cont  = ADD_ANCHOR();

			v->ops = NULL;
			APPEND(vm, &v->ops, L(0).first_line,
						OP_block_begin, brk, cont, OP_undef, OP_store,
						OP_anchor, begin,
						OP_merge, &V(1).ops,
						OP_anchor, cont,
						OP_expr, &V(4).expr, OP_jt, begin,
						OP_anchor, brk,
						OP_block_end, -1);
			break;
		}
		case R_WHILE_STATEMENT: {
			Sophon_ParserAnchor *brk, *cont;

			brk   = ADD_ANCHOR();
			cont  = ADD_ANCHOR();

			v->ops = NULL;
			APPEND(vm, &v->ops, L(0).first_line,
						OP_block_begin, brk, cont, OP_undef, OP_store,
						OP_anchor, cont,
						OP_expr, &V(2).expr, OP_jf, brk,
						OP_merge, &V(4).ops,
						OP_jmp, cont,
						OP_anchor, brk,
						OP_block_end, -1);
			break;
		}
		case R_FOR_LOOP_2: {
			Sophon_ParserExpr *e1 = NULL, *e2 = NULL;

			if (!EXPR_IS_EMPTY(&V(3).expr))
				e1 = &V(3).expr;
			if (!EXPR_IS_EMPTY(&V(5).expr))
				e2 = &V(5).expr;

			parser_for_loop(vm, &L(0), NULL, e1, e2, &V(7).ops, v);
			break;
		}
		case R_FOR_LOOP_3: {
			Sophon_ParserOp *ops = NULL;
			Sophon_ParserExpr *e2 = NULL, *e3 = NULL;

			parser_append_ops(vm, &ops, L(2).first_line,
						OP_expr, &V(2).expr, OP_pop, 1, -1);
			if (!EXPR_IS_EMPTY(&V(4).expr))
				e2 = &V(4).expr;
			if (!EXPR_IS_EMPTY(&V(6).expr))
				e3 = &V(6).expr;

			parser_for_loop(vm, &L(0), &ops, e2, e3, &V(8).ops, v);
			break;
		}
		case R_FOR_LOOP_VAR: {
			Sophon_ParserExpr *e2 = NULL, *e3 = NULL;

			if (!EXPR_IS_EMPTY(&V(5).expr))
				e2 = &V(5).expr;
			if (!EXPR_IS_EMPTY(&V(7).expr))
				e3 = &V(7).expr;

			parser_for_loop(vm, &L(0), &V(3).ops, e2, e3, &V(9).ops, v);
			break;
		}
		case R_FOR_IN: {
			r = parser_for_in(vm, &L(2), NULL, &V(2).expr, &V(4).expr,
						&V(6).ops, v);
			if (r != SOPHON_OK)
				return r;
			break;
		}
		case R_FOR_IN_VAR: {
			Sophon_ParserExpr expr;

			EXPR_INIT(&expr);
			expr.bind_namev = V(3).var.namev;
			expr.bind_line  = L(3).first_line;

			r = parser_for_in(vm, &L(3), &V(3).var.ops, &expr, &V(5).expr,
						&V(7).ops, v);
			if (r != SOPHON_OK)
				return r;
			break;
		}
		case R_THROW: {
			v->ops = NULL;
			APPEND(vm, &v->ops, L(0).first_line,
						OP_expr, &V(1).expr, OP_throw, -1);
			break;
		}
		case R_FRAME_BEGIN: {
			parser_push_frame(vm);
			break;
		}
		case R_CATCH: {
			Sophon_Int id;
			Sophon_ParserFrame *frame = FRAME(0);

			v->ops = NULL;
			id = ADD_CONST(V(2).v);
			APPEND(vm, &v->ops, L(4).first_line,
						OP_catch, id, OP_merge, &frame->ops,
						OP_merge, &V(5).ops,
						OP_pop_frame, -1);

			parser_pop_frame(vm);
			break;
		}
		case R_TRY_CATCH: {
			parser_try_catch(vm, &L(0), &V(1).ops, &V(2).ops, NULL, v);
			break;
		}
		case R_TRY_FINALLY: {
			parser_try_catch(vm, &L(0), &V(1).ops, NULL, &V(2).ops, v);
			break;
		}
		case R_TRY_CATCH_FINALLY: {
			parser_try_catch(vm, &L(0), &V(1).ops, &V(2).ops, &V(3).ops, v);
			break;
		}
		case R_WITH: {
			Sophon_ParserFrame *frame = FRAME(0);

			if (FUNC(0)->func->flags & SOPHON_FUNC_FL_STRICT) {
				sophon_parser_error(vm, SOPHON_PARSER_ERROR, &L(0),
							"\"with\" cannot be used in strict mode");
				return SOPHON_ERR_PARSE;
			}

			v->ops = NULL;
			APPEND(vm, &v->ops, L(0).first_line,
						OP_expr, &V(2).expr, OP_with,
						OP_merge, &frame->ops,
						OP_merge, &V(5).ops,
						OP_pop_frame, -1);

			parser_pop_frame(vm);
			break;
		}
		case R_CASE_CLAUSE: {
			v->cases = parser_case_create(vm, &L(0), &V(1).expr, &V(3).ops);
			break;
		}
		case R_CASE_CLAUSE_NOOP: {
			v->cases = parser_case_create(vm, &L(0), &V(1).expr, NULL);
			break;
		}
		case R_DEFAULT_CLAUSE: {
			v->cases = parser_case_create(vm, &L(0), NULL, &V(2).ops);
			break;
		}
		case R_DEFAULT_CLAUSE_NOOP: {
			v->cases = parser_case_create(vm, &L(0), NULL, NULL);
			break;
		}
		case R_CASE_CLAUSES: {
			parser_case_merge(V(0).cases, V(1).cases);
			*v = V(0);
			break;
		}
		case R_CASE_EMPTY: {
			v->cases = NULL;
			break;
		}
		case R_CASE_DEFAULT: {
			parser_case_merge(V(1).cases, V(2).cases);
			*v = V(1);
			break;
		}
		case R_CASE_DEFAULT_CASE: {
			parser_case_merge(V(1).cases, V(2).cases);
			parser_case_merge(V(1).cases, V(3).cases);
			*v = V(1);
			break;
		}
		case R_SWITCH: {
			Sophon_ParserCase *conds = V(4).cases;
			Sophon_ParserCase *c;
			Sophon_ParserOp *cond_ops = NULL, *job_ops = NULL;
			Sophon_ParserAnchor *anchor, *def_anchor, *brk;

			brk = ADD_ANCHOR();

			c = conds;
			if (c) {
				Sophon_Bool has_default = SOPHON_FALSE;

				def_anchor = ADD_ANCHOR();

				do {
					if (c->cond_ops) {
						anchor = ADD_ANCHOR();
						APPEND(vm, &cond_ops, L(4).first_line, OP_dup, 0,
									OP_merge, &c->cond_ops,
									OP_equ, OP_jt, anchor, -1);
					} else {
						anchor = def_anchor;
						has_default = SOPHON_TRUE;
					}

					APPEND(vm, &job_ops, L(4).first_line, OP_anchor, anchor,
								OP_merge, &c->job_ops, -1);

					c = c->next;
				} while (c != conds);

				APPEND(vm, &cond_ops, L(4).first_line, OP_jmp, def_anchor, -1);

				if (!has_default) {
					APPEND(vm, &job_ops, L(4).first_line,
								OP_anchor, def_anchor, -1);
				}
			}

			v->ops = NULL;
			APPEND(vm, &v->ops, L(0).first_line,
						OP_block_begin, brk, NULL,
						OP_undef, OP_store,
						OP_expr, &V(2).expr,
						OP_merge, &cond_ops, OP_merge, &job_ops,
						OP_pop, 1,
						OP_anchor, brk, -1);

			parser_clear_value(vm, N_CASE_BLOCK, &V(4));
			break;
		}
		case R_DELETE: {
			if (!EXPR_IS_REF(&V(1).expr)) {
				EXPR_TO_VALUE(&V(1).expr);
				APPEND(vm, &V(1).expr.base_ops, L(0).first_line,
							OP_pop, 1, OP_true);
				*v = V(1);
				return SOPHON_OK;
			}

			if (EXPR_IS_BIND(&V(1).expr)) {
				Sophon_Int id;

				id = ADD_CONST(V(1).expr.bind_namev);
				APPEND(vm, &V(1).expr.base_ops, L(0).first_line,
							OP_delete_bind, id, -1);
				UNDEFINED(&V(1).expr.bind_namev);
			} else {
				APPEND(vm, &V(1).expr.base_ops, L(0).first_line,
							OP_merge, &V(1).expr.name_ops,
							OP_delete, -1);
			}

			*v = V(1);
			break;
		}
		case R_LABEL: {
			Sophon_Int id;

			v->ops = NULL;
			id = ADD_CONST(V(0).v);
			APPEND(vm, &v->ops, L(0).first_line,
						OP_label, id,
						OP_merge, &V(2).ops, -1);
			break;
		}
		case R_PARAM_2:
		case R_PARAM: {
			Sophon_Function *func = FUNC(0)->func;
			Sophon_String *name;
			Sophon_Int r;
			Sophon_Bool strict = func->flags & SOPHON_FUNC_FL_STRICT;
			char *buf;
			Sophon_U32 len;

			SOPHON_ASSERT(func);

			if (rid == R_PARAM)
				name = GET_STRING(V(0).v);
			else
				name = GET_STRING(V(2).v);
			
			if (strict &&
					((name == vm->arguments_str) || (name == vm->eval_str))) {
				if (sophon_string_new_utf8_cstr(vm, name, &buf, &len) >= 0) {
					sophon_parser_error(vm, SOPHON_PARSER_ERROR, &L(0),
								"\"%s\" cannot be an argument name",
								buf);
					sophon_string_free_utf8_cstr(vm, buf, len);
				}
				return SOPHON_ERR_PARSE;
			}

			r = sophon_function_add_var(vm, func, SOPHON_FUNC_ARG, name);
			if (r == SOPHON_NONE) {		
				if (sophon_string_new_utf8_cstr(vm, name, &buf, &len) >= 0) {
					sophon_parser_error(vm,
								strict ? SOPHON_PARSER_ERROR :
								SOPHON_PARSER_WARNING,
								&L(0),
								"argument \"%s\" has already been defined",
								buf);
					sophon_string_free_utf8_cstr(vm, buf, len);
				}

				if (strict) {
					return SOPHON_ERR_PARSE;
				}
			}
			break;
		}
		case R_FUNC_BEGIN: {
			Sophon_Bool strict = FUNC(0)->func->flags & SOPHON_FUNC_FL_STRICT;
			Sophon_String *name = GET_STRING(V(1).v);

			if (strict &&
					((name == vm->arguments_str) || (name == vm->eval_str))) {
				char *buf;
				Sophon_U32 len;

				if (sophon_string_new_utf8_cstr(vm, name, &buf, &len) >= 0) {
					sophon_parser_error(vm, SOPHON_PARSER_ERROR, &L(0),
								"\"%s\" cannot be a function name",
								buf);
					sophon_string_free_utf8_cstr(vm, buf, len);
				}
				return SOPHON_ERR_PARSE;
			}

			parser_push_func(vm, name);
			break;
		}
		case R_FUNC_BODY: {
			p->flags |= SOPHON_PARSER_FL_FUNC_BEGIN;
			break;
		}
		case R_FUNC_DECL: {
			Sophon_ParserFunc *func;
			Sophon_ParserFrame *frame;
			Sophon_Int id;

			func  = FUNC(0);
			frame = FRAME(1);

			id = ADD_CONST(SOPHON_VALUE_GC(func->func->name));
			v->ops = NULL;

			APPEND(vm, &frame->ops, L(0).first_line, OP_closure,
						func->func->id, OP_put_bind, id, OP_pop, 1, -1);
			r = parser_pop_func(vm, &V(8).ops);
			if (r != SOPHON_OK)
				return r;
			break;
		}
		case R_FUNC_EXPR: {
			Sophon_Function *func = FUNC(0)->func;

			EXPR_INIT(&v->expr);
			APPEND(vm, &v->expr.base_ops, L(0).first_line,
						OP_closure, func->id, -1);
			r = parser_pop_func(vm, &V(8).ops);
			if (r != SOPHON_OK)
				return r;
			break;
		}
		case R_DEBUGGER: {
			v->ops = NULL;
			APPEND(vm, &v->ops, L(0).first_line, OP_debugger,
						OP_undef, OP_store, -1);
			break;
		}
		case R_VOID: {
			*v = V(1);
			EXPR_TO_VALUE(&v->expr);
			APPEND(vm, &v->expr.base_ops, L(0).first_line,
						OP_pop, 1, OP_undef, -1);
			break;
		}
		case R_NSAME: {
			EXPR_TO_VALUE(&V(0).expr);
			EXPR_TO_VALUE(&V(2).expr);
			EXPR_MERGE(&V(0).expr, &V(2).expr);
			APPEND(vm, &V(0).expr.base_ops, L(1).first_line,
						OP_merge,
						&V(2).expr.base_ops, OP_same, OP_not, -1);
			*v = V(0);
			break;
		}
		case R_NEQ: {
			EXPR_TO_VALUE(&V(0).expr);
			EXPR_TO_VALUE(&V(2).expr);
			EXPR_MERGE(&V(0).expr, &V(2).expr);
			APPEND(vm, &V(0).expr.base_ops, L(1).first_line,
						OP_merge,
						&V(2).expr.base_ops, OP_equ, OP_not, -1);
			*v = V(0);
			break;
		}
#define UNARY_REDUCE_OP(name, op)\
		case R_##name: {\
			EXPR_TO_VALUE(&V(1).expr);\
			APPEND(vm, &V(1).expr.base_ops, L(0).first_line,\
						OP_##op, -1);\
			*v = V(1);\
			break;\
		}
		UNARY_REDUCE_OP(TYPEOF, typeof)
		UNARY_REDUCE_OP(POSITIVE, to_num)
		UNARY_REDUCE_OP(NEG, neg)
		UNARY_REDUCE_OP(NOT, not)
		UNARY_REDUCE_OP(REV, rev)
#define BIN_REDUCE_OP(name, op)\
		case R_##name: {\
			EXPR_TO_VALUE(&V(0).expr);\
			EXPR_TO_VALUE(&V(2).expr);\
			EXPR_MERGE(&V(0).expr, &V(2).expr);\
			APPEND(vm, &V(0).expr.base_ops, L(1).first_line,\
						OP_merge,\
						&V(2).expr.base_ops, OP_##op, -1);\
			*v = V(0);\
			break;\
		}
		BIN_REDUCE_OP(ADD, add)
		BIN_REDUCE_OP(SUB, sub)
		BIN_REDUCE_OP(MUL, mul)
		BIN_REDUCE_OP(DIV, div)
		BIN_REDUCE_OP(MOD, mod)
		BIN_REDUCE_OP(SHL, shl)
		BIN_REDUCE_OP(SHR, shr)
		BIN_REDUCE_OP(USHR, ushr)
		BIN_REDUCE_OP(LT, lt)
		BIN_REDUCE_OP(GT, gt)
		BIN_REDUCE_OP(LE, le)
		BIN_REDUCE_OP(GE, ge)
		BIN_REDUCE_OP(EQU, equ)
		BIN_REDUCE_OP(SAME, same)
		BIN_REDUCE_OP(INSTANCEOF, instanceof)
		BIN_REDUCE_OP(BAND, band)
		BIN_REDUCE_OP(BOR, bor)
		BIN_REDUCE_OP(XOR, xor)
		BIN_REDUCE_OP(IN, in)
		case R_ASSIGN:
			v->i = -1;
			break;
#define ASSIGN_REDUCE_OP(name, op)\
		case R_##name##_ASSIGN: v->i = OP_##op; break;
		ASSIGN_REDUCE_OP(ADD, add)
		ASSIGN_REDUCE_OP(SUB, sub)
		ASSIGN_REDUCE_OP(MUL, mul)
		ASSIGN_REDUCE_OP(DIV, div)
		ASSIGN_REDUCE_OP(MOD, mod)
		ASSIGN_REDUCE_OP(SHL, shl)
		ASSIGN_REDUCE_OP(SHR, shr)
		ASSIGN_REDUCE_OP(USHR, ushr)
		ASSIGN_REDUCE_OP(BAND, band)
		ASSIGN_REDUCE_OP(BOR, bor)
		ASSIGN_REDUCE_OP(XOR, xor)
		case R_ACCEPT: {
			if ((r = parser_pop_func(vm, &V(0).ops)) < 0)
				return r;
			break;
		}
		case R_BEGIN:
		case R_END:
			break;
	}

	return SOPHON_OK;
}

#define POP()\
	SOPHON_MACRO_BEGIN \
		p->top--; \
		top = &p->stack[p->top - 1]; \
	SOPHON_MACRO_END

#define POPN(n)\
	SOPHON_MACRO_BEGIN \
		p->top -= (n); \
		top = &p->stack[p->top - 1]; \
	SOPHON_MACRO_END

#define PUSH()\
	SOPHON_MACRO_BEGIN \
		p->top++; \
		if (p->top > SOPHON_PARSER_STACK_SIZE)\
			sophon_fatal("parser stack overflow");\
		top = &p->stack[p->top - 1]; \
	SOPHON_MACRO_END

/*Main parse function*/
static Sophon_Result
parse (Sophon_VM *vm)
{
	Sophon_ParserData *p = (Sophon_ParserData*)vm->parser_data;
	Sophon_ParserStack *top;
	Sophon_ParserStack *curr;
	Sophon_Token tok = 0;
	Sophon_U16 sym;
	Sophon_U16 edge;
	Sophon_U16 eps;
	Sophon_U32 reduce;
	Sophon_Result r;
	Sophon_Bool error = SOPHON_FALSE;
	Sophon_Bool fatal = SOPHON_FALSE;

	p->top = 0;
	p->obj_top  = 0;
	p->fetch.t  = 0xFFFF;
	p->reduce.t = 0xFFFF;
	p->semicolon.t = 0xFFFF;

	PUSH();
	top->t = 0;
	top->s = 0;
	top->l.first_line   = 0;
	top->l.first_column = 0;
	top->l.last_line    = 0;
	top->l.last_column  = 0;

next_state:
	if (p->semicolon.t != 0xFFFF) {
		curr = &p->semicolon;
	} else if (p->fetch.t == 0xFFFF) {

		if (p->flags & SOPHON_PARSER_FL_EOF) {
			tok = SOPHON_ERR_EOF;
		} else {
			tok = sophon_lex(vm, &p->fetch.v, &p->fetch.l);
		}

		curr = &p->fetch;

		if (tok == SOPHON_ERR_EOF) {
			if (!parser_shell_end(vm))
				goto next_state;

			p->flags |= SOPHON_PARSER_FL_EOF;
			goto reduce;
		}

		if (tok == SOPHON_ERR_LEX)
			goto real_error;

		if (tok < 0) {
			error = SOPHON_TRUE;
			fatal = SOPHON_TRUE;
			goto accept;
		}

		p->flags &= ~SOPHON_PARSER_FL_AUTO_SEMICOLON;

		/*Clear new borned GC objects*/
		sophon_gc_set_nb_count(vm, p->gc_level);

		/*Auto semicolon check*/
		if (sophon_lex_has_line_term(vm)) {
			if ((top->t == T_break) || (top->t == T_continue) ||
						(top->t == T_return)) {
				parser_auto_semicolon(vm);
				goto next_state;
			}
		}

		curr->t = tok;
		DEBUG(("input token %d", curr->t));
	} else {
		curr = &p->fetch;
	}

	/*Strict mode flag check*/
	if (p->flags & SOPHON_PARSER_FL_FUNC_BEGIN) {
		if (!(FUNC(0)->func->flags & SOPHON_FUNC_FL_STRICT)) {
			if (curr->t == T_STRING) {
				Sophon_String *str = GET_STRING(curr->v.v);

				if (!sophon_string_cmp(vm, str, vm->use_strict_str)) {
					FUNC(0)->func->flags |= SOPHON_FUNC_FL_STRICT;
				}
			}
		}
		p->flags &= ~SOPHON_PARSER_FL_FUNC_BEGIN;
	}

	/*State change*/
	edge = js_state_shifts[top->s];
	eps  = 0xFFFF;

	while (edge != 0xFFFF) {
		sym = js_edge_symbol[edge];

		if (sym == curr->t) {
			PUSH();

			curr->s = js_edge_dest[edge];
			*top = *curr;

			curr->t = 0xFFFF;

			DEBUG(("shift to state %d", curr->s));
			goto next_state;
		} else if (sym == T_EPSILON) {
			eps = js_edge_dest[edge];
		} else if ((sym == T_REGEXP) &&
				((curr->t == '/') || (curr->t == T_DIV_ASSIGN))) {
			Sophon_Token t;

			SOPHON_INFO(("switch to regexp"));
			t = curr->t;
			curr->t = 0xFFFF;
			sophon_lex_to_regexp_cond(vm, t);
			goto next_state;
		}

		edge = js_edge_next[edge];
	}

	if (eps != 0xFFFF) {
		PUSH();

		curr->s = eps;
		*top = *curr;

		DEBUG(("shift epsilon to state %d", curr->s));
		goto next_state;
	}

reduce:
	reduce = js_state_reduce[top->s];
	if (reduce != R_NONE) {

		Sophon_Int rid;
		Sophon_Int tid;
		Sophon_Int pop;

		rid = RV_GET_REDUCE(reduce);
		tid = RV_GET_TOKEN(reduce);
		pop = RV_GET_POP(reduce);

		DEBUG(("reduce rid:%d tid:%d pop:%d flag:%d",
						rid, tid, pop,
						RV_GET_POP_FLAG(reduce) ? 1 : 0));

		SOPHON_ASSERT(pop < p->top);

		if ((tid == N_EXPR_OR_EMPTY) && (p->flags & SOPHON_PARSER_FL_EOF))
			goto error;

		if (!(p->flags & SOPHON_PARSER_FL_ERROR)) {
			r = parser_reduce(vm, rid, pop, &p->reduce.v);
			if (r < 0)
				goto error;
		} else {
			Sophon_Int i;

			for (i = 0; i < pop; i++) {
				Sophon_ParserStack *stk = &p->stack[p->top - pop + i];
				parser_clear_value(vm, stk->t, &stk->v);
			}

			sophon_memset(&p->reduce.v, 0, sizeof(p->reduce.v));
		}

		if (!RV_GET_POP_FLAG(reduce)) {
			pop = 0;
			tid = N_REDUCE;
		}

		if (pop) {
			Sophon_ParserStack *stk;
			
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

		edge = js_state_jumps[top->s];
		while (edge != 0xFFFF) {
			sym = js_edge_symbol[edge];
			if (sym == tid) {
				PUSH();

				p->reduce.s = js_edge_dest[edge];
				*top = p->reduce;
				p->reduce.t = 0xFFFF;

				/*Auto semicolon check*/
				if (sophon_lex_has_line_term(vm)) {
					if ((top->t == N_MATH_EXPR) &&
								((curr->t == T_INC) ||
								 (curr->t == T_DEC))) {
						parser_auto_semicolon(vm);
						goto next_state;
					}
				}

				DEBUG(("goto state %d", top->s));
				goto next_state;
			}

			edge = js_edge_next[edge];
		}

		parser_clear_value(vm, p->reduce.t, &p->reduce.v);
		p->reduce.t = 0xFFFF;
	}

error:
	/*Insert semicolon*/
	r = parser_solve_error(vm);
	if (r == SOPHON_OK) {
		goto next_state;
	}

real_error:
	/*Error*/
	if (!error) {
		error = SOPHON_TRUE;
		p->flags |= SOPHON_PARSER_FL_ERROR;
	}

	sophon_parser_error(vm, SOPHON_PARSER_ERROR, &curr->l, NULL);

	if (tok != SOPHON_ERR_EOF) {
		parser_clear_value(vm, curr->t, &curr->v);
		curr->t = 0xFFFF;

		while (p->top > 1) {
			edge = js_state_jumps[top->s];

			while (edge != 0xFFFF) {
				sym = js_edge_symbol[edge];

				if (sym == N_ERROR) {
					PUSH();

					top->t = N_ERROR;
					top->s = js_edge_dest[edge];
					top->l = curr->l;
					goto next_state;
				}

				edge = js_edge_next[edge];
			}

			parser_clear_value(vm, top->t, &top->v);
			POP();
		}
	}

accept:
	if (!error) {
		SOPHON_INFO(("accept"));
	}

	/*Clear parser data*/
	parser_clear_value(vm, p->fetch.t, &p->fetch.v);
	parser_clear_value(vm, p->reduce.t, &p->reduce.v);

	for (; top >= p->stack; top--) {
		parser_clear_value(vm, top->t, &top->v);
	}

	while (p->obj_top) {
		parser_pop_obj(vm);
	}

	while (p->frame_top) {
		parser_pop_frame(vm);
	}

	return error ? (fatal ? tok : SOPHON_ERR_PARSE) : SOPHON_OK;
}

Sophon_Bool
sophon_parser_strict_mode (Sophon_VM *vm)
{
	Sophon_ParserData *p;
	
	SOPHON_ASSERT(vm);

	p = (Sophon_ParserData*)vm->parser_data;

	return p->flags & SOPHON_PARSER_FL_STRICT;
}

Sophon_Result
sophon_parse (Sophon_VM *vm, Sophon_Module *mod, Sophon_Encoding enc,
			Sophon_IOFunc input, Sophon_Ptr data, Sophon_U32 flags)
{
	Sophon_ParserData *p;
	Sophon_Int func_id;
	Sophon_U32 func_flags;
	Sophon_Function *func;
	Sophon_Closure *clos;
	Sophon_Result r;

	SOPHON_ASSERT(vm && input && mod);

	/*Allocate parser data*/
	p = sophon_mm_alloc_ensure(vm, sizeof(Sophon_ParserData));

	/*Store GC level*/
	p->gc_level = sophon_gc_get_nb_count(vm);

	sophon_lex_init(vm, enc, input, data);

	p->module = mod;
	p->flags = flags & (SOPHON_PARSER_FL_EVAL|SOPHON_PARSER_FL_STRICT|\
				SOPHON_PARSER_FL_SHELL|SOPHON_PARSER_FL_NO_WARNING);
	p->flags |= SOPHON_PARSER_FL_FUNC_BEGIN;

	/*Add a new function*/
	if (flags & SOPHON_PARSER_FL_BODY) {
		func_id = 0;
	} else {
		func_flags = 0;
		if (flags & SOPHON_PARSER_FL_EVAL) {
			func_flags |= SOPHON_FUNC_FL_EVAL;

			if (vm->stack && vm->stack->func) {
				if (vm->stack->func->flags & SOPHON_FUNC_FL_STRICT)
					func_flags |= SOPHON_FUNC_FL_STRICT;
			}
		} else {
			func_flags |= SOPHON_FUNC_FL_GLOBAL;
		}

		if (flags & SOPHON_PARSER_FL_STRICT)
			func_flags |= SOPHON_FUNC_FL_STRICT;
		
		func_id = sophon_module_add_func(vm, mod, NULL,	func_flags);
		SOPHON_ASSERT(func_id >= 0);
	}

	func = sophon_module_get_func(mod, func_id);

	p->func_stack[0].func = func;
	p->func_stack[0].frame_bottom = 0;
	p->func_top  = 1;
	p->frame_top = 0;

	/*Add a new closure*/
	clos = sophon_closure_create(vm, func);
	SOPHON_ASSERT(clos);
	sophon_value_set_gc(vm, &mod->globv, (Sophon_GCObject*)clos);

	vm->parser_data = p;

	parser_push_frame(vm);

	/*Parse*/
	r = parse(vm);
	if (r == SOPHON_ERR_PARSE) {
		sophon_throw(vm, vm->SyntaxError, "Syntax error");
	}

	/*Release data*/
	vm->parser_data = NULL;
	sophon_gc_set_nb_count(vm, p->gc_level);
	sophon_lex_deinit(vm);
	sophon_mm_free(vm, p, sizeof(Sophon_ParserData));

	return r;
}

Sophon_ParserOpModel
sophon_parser_op_get_model (Sophon_ParserOpType op)
{

	if ((op < 0) ||
				(op >= SOPHON_ARRAY_SIZE(parser_op_model_table) - 1))
		return MODEL_COUNT;

	return parser_op_model_table[op];
}

Sophon_TokenValueField
sophon_token_value_get_field(Sophon_U16 tok)
{
	if (tok < TOKEN_FIELD_TABLE_BEGIN)
		return FIELD_COUNT;

	tok -= TOKEN_FIELD_TABLE_BEGIN;

	if (tok >= SOPHON_ARRAY_SIZE(js_token_field_table) - 1)
		return FIELD_COUNT;

	return js_token_field_table[tok];
}

Sophon_Result
sophon_eval (Sophon_VM *vm, Sophon_Encoding enc, Sophon_IOFunc input,
			Sophon_Ptr data, Sophon_U32 flags, Sophon_Value *retv)
{
	Sophon_Module *mod;
	Sophon_U32 gc_level;
	Sophon_Result r;

	SOPHON_ASSERT(vm);

	gc_level = sophon_gc_get_nb_count(vm);

	mod = sophon_module_create(vm);

	r = sophon_parse(vm, mod, enc, input, data, flags|SOPHON_PARSER_FL_EVAL);

	if (r == SOPHON_OK) {
		Sophon_Value thisv;

		thisv = sophon_stack_get_this(vm);
		r = sophon_value_call(vm, mod->globv, thisv, NULL, 0, retv, 0);
	}

	sophon_gc_set_nb_count(vm, gc_level);

	return r;
}

