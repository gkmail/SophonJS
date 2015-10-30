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

#include <sophon_ins.h>
#include "sophon_ins_internal.h"

#ifndef SOPHON_BLOCK_STACK_SIZE
	#define SOPHON_BLOCK_STACK_SIZE 64
#endif

#define SOPHON_FRAME_STACK_SIZE SOPHON_PARSER_FRAME_STACK_SIZE

typedef struct {
	Sophon_ParserLabel *label;
	Sophon_ParserBlock *block;
} BlockStackEntry;

typedef struct {
	BlockStackEntry  stack[SOPHON_BLOCK_STACK_SIZE];
	Sophon_U32       top;
} BlockStack;

typedef struct {
	Sophon_ParserOp *stack[SOPHON_FRAME_STACK_SIZE];
	Sophon_U32       top;
} FrameStack;

static Sophon_Int
func_lookup_var (Sophon_VM *vm, Sophon_Function *func, Sophon_U16 name_id)
{
	Sophon_Value namev;
	Sophon_String *name;
	Sophon_HashEntry *ent;
	Sophon_Result r;
	Sophon_Int id = -1;

	namev = sophon_module_get_const(func->module, name_id);
	name = GET_STRING(namev);

	r = sophon_hash_lookup(vm, &func->var_hash,
				sophon_direct_key,
				sophon_direct_equal,
				(Sophon_Ptr)name,
				&ent);
	if (r == SOPHON_OK) {
		id = (Sophon_IntPtr)ent->value;
	}

	return id;
}

static Sophon_Result
parser_gen_code (Sophon_VM *vm, Sophon_Function *func, Sophon_ParserOp **ops)
{
	Sophon_ParserOp *op;
	Sophon_U32 last_line;
	Sophon_Int ip, sp, ss, ls;
	BlockStack *bs;
	FrameStack *fs;
	Sophon_Result r = SOPHON_OK;

	SOPHON_INFO(("generate byte code"));

	/*Calculate stack and instruction buffer size*/
	bs = sophon_mm_alloc_ensure(vm, sizeof(BlockStack));
	bs->top = 0;

	fs = sophon_mm_alloc_ensure(vm, sizeof(FrameStack));
	fs->top = 0;

	last_line = 0;
	ss = 0;
	ip = 0;
	sp = 0;
	ls = 0;

	op = *ops;
	if (op) {
		do {
			switch (op->type) {
#define GET_PARAM(name) (c->name)
#define OP_CALC_Op      ip++;
#define OP_CALC_Const   ip+=3;
#define OP_CALC_Jump    c->ip = ip; ip+=3;
#define OP_CALC_Call    ip+=2;
#define OP_CALC_StackOp ip+=2;
#define OP_CALC(name, model, stk)\
				case OP_##name: {\
					Sophon_Parser##model *c = (Sophon_Parser##model*)op;\
					OP_CALC_##model;\
					sp += stk;\
					c = c;\
					break;\
				}

				FOR_EACH_INS_OP(OP_CALC)

				case OP_anchor: {
					Sophon_ParserAnchor *anchor = (Sophon_ParserAnchor*)op;
					anchor->ip = ip;
					anchor->sp = sp;
					break;
				}
				case OP_put_bind:
				case OP_get_bind: {
					Sophon_ParserBind *bind = (Sophon_ParserBind*)op;
					Sophon_Bool put = (op->type == OP_put_bind);
					Sophon_Int var_id = 0;
					Sophon_Int s;

					/*Search the binding in frame stack*/
					for (s = fs->top - 1; s >= 0; s--) {
						if (fs->stack[s]->type == OP_catch) {
							Sophon_ParserConst *cop =
								(Sophon_ParserConst*)fs->stack[s];

							if (cop->id == bind->name_id) {
								var_id = -1;
								break;
							}
						} else if (fs->stack[s]->type == OP_with) {
							var_id = -1;
							break;
						}
					}
					
					/*Search the binding in the function variant hash table*/
					if (var_id != -1)
						var_id = func_lookup_var(vm, func, bind->name_id);

					if (var_id >= 0) {
						if (var_id & SOPHON_VAR_FL_ARG) {
							var_id &= ~SOPHON_VAR_FL_ARG;
							var_id += func->varc;
						}
						if (var_id > 0x7FFF)
							sophon_fatal("too many variants");
						bind->var_id = var_id;
					} else {
						bind->var_id = 0xFFFF;
					}

					ip += 3;
					if (!put)
						sp++;
					break;
				}
				case OP_dup_ref:
				case OP_mov_ref:
				case OP_pop_ref: {
					Sophon_ParserRef *ref = (Sophon_ParserRef*)op;
					ref->sp = sp;
					ip += 2;

					if (op->type == OP_dup_ref)
						sp++;
					break;
				}
				case OP_label: {
					Sophon_ParserLabel *lab = (Sophon_ParserLabel*)op;
					Sophon_ParserOp *nop = op->next;

					lab->block = NULL;
					while (nop != *ops) {
						if (nop->type == OP_block_begin) {
							lab->block = (Sophon_ParserBlock*)nop;
							break;
						} else if (nop->type != OP_label) {
							break;
						}
						nop = nop->next;
					}

					if (lab->block) {
						if (bs->top >= SOPHON_BLOCK_STACK_SIZE)
							sophon_fatal("block stack overflow");
						bs->stack[bs->top].label = lab;
						bs->stack[bs->top].block = lab->block;
						bs->top++;
					}
					break;
				}
				case OP_continue:
				case OP_break: {
					Sophon_ParserLongJump *lj = (Sophon_ParserLongJump*)op;
					BlockStackEntry *ent;
					Sophon_Int s = bs->top - 1;
					Sophon_Bool found = SOPHON_FALSE;
					Sophon_Bool brk = (op->type == OP_break);

					lj->anchor = NULL;
					lj->ip = ip;
					lj->sp = sp;

					while (s >= 0) {
						ent = &bs->stack[s];

						if (brk ? ent->block->brk_anchor :
									ent->block->cont_anchor) {
							if (lj->label_id == 0xFFFF) {
								found = SOPHON_TRUE;
							} else if (ent->label) {
								if (ent->label->label_id == lj->label_id) {
									found = SOPHON_TRUE;
								}
							}

							if (found) {
								lj->anchor = brk ? ent->block->brk_anchor :
										ent->block->cont_anchor;
								break;
							}
						}
						s--;
					}

					if(!lj->anchor) {
						Sophon_Location loc;

						loc.first_line = op->line;
						loc.last_line  = op->line;
						loc.first_column = 0;
						loc.last_column = 0;

						sophon_parser_error(vm, SOPHON_PARSER_ERROR, &loc,
									"\"%s\" is not in a block",
									brk ? "break" : "continue");
						r = SOPHON_ERR_PARSE;
						goto calc_end;
					}

					ip += 5;
					break;
				}
				case OP_block_begin: {
					Sophon_ParserBlock *blk = (Sophon_ParserBlock*)op;

					if ((op == *ops) || (op->prev->type != OP_label)) {
						if (bs->top >= SOPHON_BLOCK_STACK_SIZE)
							sophon_fatal("block stack overflow");
						bs->stack[bs->top].label = NULL;
						bs->stack[bs->top].block = blk;
						bs->top++;
					}
					break;
				}
				case OP_block_end: {
					Sophon_ParserBlock *blk;

					SOPHON_ASSERT(bs->top);
					blk = bs->stack[bs->top - 1].block;
					SOPHON_ASSERT(blk);

					while (bs->top && (bs->stack[bs->top - 1].block == blk)) {
						bs->top--;
					}
					break;
				}
				case OP_stack: {
					Sophon_ParserStackOp *sop = (Sophon_ParserStackOp*)op;
					sp -= sop->n;
					break;
				}
				default:
					break;
			}

			/*Modify frame stack*/
			switch (op->type) {
				case OP_with:
				case OP_catch:
					if (fs->top >= SOPHON_FRAME_STACK_SIZE)
						sophon_fatal("frame stack overflow");
					fs->stack[fs->top++] = op;
					break;
				case OP_pop_frame:
					SOPHON_ASSERT(fs->top > 0);
					fs->top--;
					break;
				default:
					break;
			}

			ss = SOPHON_MAX(ss, sp);

			if (op->line != last_line) {
				last_line = op->line;
				ls++;
			}

			op = op->next;
		} while (op != *ops);
	}

	if ((ip > 0xFFFF) || (ss > 0xFFFF) || (ls > 0xFFFF))
		sophon_fatal("instructions overflow");

calc_end:
	sophon_mm_free(vm, bs, sizeof(BlockStack));
	sophon_mm_free(vm, fs, sizeof(FrameStack));

	SOPHON_INFO(("IP:%d SS:%d LS:%d", ip, ss, ls));

	/*Generate byte code*/
	if (r == SOPHON_OK) {
		func->stack_size = ss;
		func->ibuf_size  = ip;

		func->f.ibuf = (Sophon_U8*)sophon_mm_alloc_ensure(vm, ip);

#ifdef SOPHON_LINE_INFO
		if (ls) {
			func->lbuf = (Sophon_LineInfo*)sophon_mm_alloc_ensure(vm,
						ls * sizeof(Sophon_LineInfo));
			func->lbuf_size = ls;
		}
#endif

		op = *ops;
		if (op) {
			Sophon_U8 *pi = func->f.ibuf;
#ifdef SOPHON_LINE_INFO
			Sophon_LineInfo *pl = func->lbuf;
#endif
			last_line = 0;

			do {
				switch (op->type) {
#define GEN_INS_Op
#define GEN_INS_Const   *pi++ = c->id >> 8; *pi++ = c->id & 0xFF;
#define GEN_INS_Jump\
	SOPHON_MACRO_BEGIN\
		*pi++ = c->anchor->ip >> 8;\
		*pi++ = c->anchor->ip & 0xFF;\
	SOPHON_MACRO_END
#define GEN_INS_Call    *pi++ = c->argc;
#define GEN_INS_StackOp *pi++ = c->n;
#define GEN_INS(name, model, stk)\
					case OP_##name: {\
						Sophon_Parser##model *c = (Sophon_Parser##model*)op;\
						*pi++ = I_##name;\
						GEN_INS_##model;\
						c = c;\
						break;\
					}

					FOR_EACH_INS_OP(GEN_INS)

					case OP_put_bind:
					case OP_get_bind: {
						Sophon_ParserBind *bind = (Sophon_ParserBind*)op;
						Sophon_Bool put = (op->type == OP_put_bind);

						if (bind->var_id == 0xFFFF) {
							*pi++ = put ? I_put_bind : I_get_bind;
							*pi++ = bind->name_id >> 8;
							*pi++ = bind->name_id & 0xFF;
						} else {
							*pi++ = put ? I_put_var : I_get_var;
							*pi++ = bind->var_id >> 8;
							*pi++ = bind->var_id & 0xFF;
						}
						break;
					}
					case OP_dup_ref: {
						Sophon_ParserRef *ref = (Sophon_ParserRef*)op;

						*pi++ = I_dup;
						*pi++ = ref->sp - ref->anchor->sp;
						break;
					}
					case OP_mov_ref: {
						Sophon_ParserRef *ref = (Sophon_ParserRef*)op;

						*pi++ = I_mov;
						*pi++ = ref->sp - ref->anchor->sp;
						break;
					}
					case OP_pop_ref: {
						Sophon_ParserRef *ref = (Sophon_ParserRef*)op;

						*pi++ = I_pop;
						*pi++ = ref->sp - ref->anchor->sp;
						break;
					}
					case OP_continue:
					case OP_break: {
						Sophon_ParserLongJump *lj = (Sophon_ParserLongJump*)op;
						Sophon_U16 n;

						SOPHON_ASSERT(lj->anchor);

						n = lj->anchor->ip;

						*pi++ = I_pop;
						*pi++ = lj->sp - lj->anchor->sp;
						*pi++ = (op->type == OP_break) ? I_break : I_continue;
						*pi++ = n >> 8;
						*pi++ = n & 0xFF;
						break;
					}
					default:
						break;
				}

				if (last_line != op->line) {
					last_line = op->line;
#ifdef SOPHON_LINE_INFO
					if (pl) {
						pl->line   = op->line;
						pl->offset = pi - func->f.ibuf;
						pl++;
					}
#endif
				}
				op = op->next;
			} while (op != *ops);

#ifdef SOPHON_DEBUG
			sophon_ins_dump_buffer(vm, func, func->f.ibuf, func->ibuf_size,
						SOPHON_INS_FL_IP|SOPHON_INS_FL_LINE);
#endif
			SOPHON_ASSERT(sp == 0);
		}
	}

	return r;
}

