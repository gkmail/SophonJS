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

#ifndef _SOPHON_PARSER_INTERNAL_H_
#define _SOPHON_PARSER_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sophon_parser.h>

#define FOR_EACH_INS_OP(o)\
	o(store, Op, -1)\
	o(undef, Op, 1)\
	o(null,  Op, 1)\
	o(true,  Op, 1)\
	o(false, Op, 1)\
	o(one,   Op, 1)\
	o(zero,  Op, 1)\
	o(add,   Op, -1)\
	o(sub,   Op, -1)\
	o(mul,   Op, -1)\
	o(div,   Op, -1)/*10*/\
	o(mod,   Op, -1)\
	o(shl,   Op, -1)\
	o(shr,   Op, -1)\
	o(ushr,  Op, -1)\
	o(lt,    Op, -1)\
	o(gt,    Op, -1)\
	o(le,    Op, -1)\
	o(ge,    Op, -1)\
	o(equ,   Op, -1)\
	o(same,  Op, -1)/*20*/\
	o(band,  Op, -1)\
	o(bor,   Op, -1)\
	o(xor,   Op, -1)\
	o(in,    Op, -1)\
	o(instanceof, Op, -1)\
	o(neg,   Op, 0)\
	o(not,   Op, 0)\
	o(rev,   Op, 0)\
	o(to_num, Op, 0)\
	o(new,   Call, -GET_PARAM(argc))/*30*/\
	o(delete,Op, -1)\
	o(delete_bind, Const, 1)\
	o(typeof,Op, 0)\
	o(this,  Op, 1)\
	o(const, Const, 1)\
	o(number, Const, 1)\
	o(closure, Const, 1)\
	o(get,   Op, -1)\
	o(put,   Op, 0)\
	o(call,  Call, -GET_PARAM(argc))/*40*/\
	o(this_call,  Call, -GET_PARAM(argc)-1)\
	o(jt,    Jump, -1)\
	o(jf,    Jump, -1)\
	o(jmp,   Jump, 0)\
	o(throw,      Op,  -1)\
	o(return,     Op,  -1)\
	o(with,       Op,  -1)\
	o(catch,      Const, 0)\
	o(pop_frame,  Op,    0)\
	o(for_in,     Jump,  1)/*50*/\
	o(for_next,   Op,    0)\
	o(try_begin,  Jump,  3)\
	o(try_end,    Jump, -3)\
	o(prop_set,   Op,  -2)\
	o(prop_get,   Op,  -2)\
	o(dup,        StackOp,  1)\
	o(mov,        StackOp,  0)\
	o(pop,        StackOp,  -GET_PARAM(n))\
	o(debugger,   Op,   0)

#define OP_ABSTRACT_BEGIN OP_get_bind

#define FOR_EACH_ABSTRACT_OP(o)\
	o(get_bind,   Bind, 1)/*60*/\
	o(put_bind,   Bind, 0)\
	o(put_fbind,  Bind, 0)\
	o(dup_ref, Ref, 1)\
	o(mov_ref, Ref, 0)\
	o(pop_ref, Ref, 0)\
	o(label,   Label, 0)\
	o(break,   LongJump, 0)\
	o(continue,LongJump, 0)\
	o(block_begin, Block, 0)\
	o(block_end, Op, 0)\
	o(anchor,  Anchor, 0)/*70*/\
	o(stack,   StackOp, 0)

#define FOR_EACH_OP(o) \
	FOR_EACH_INS_OP(o) \
	FOR_EACH_ABSTRACT_OP(o)

#define FOR_EACH_OP_MODEL(m)\
	m(Op)\
	m(Const)\
	m(Anchor)\
	m(Jump)\
	m(Call)\
	m(Ref)\
	m(StackOp)\
	m(Label)\
	m(Block)\
	m(LongJump)\
	m(Bind)

#define FOR_EACH_VALUE_FIELD(f)\
	f(v)\
	f(re)\
	f(i)\
	f(ops)\
	f(expr)\
	f(var)\
	f(cases)

typedef enum {
#define DECL_OP_ENUM_ITEM(name, model, stk) OP_##name,
	FOR_EACH_OP(DECL_OP_ENUM_ITEM)
	OP_merge,
	OP_expr,
	OP_int,
	OP_COUNT
} Sophon_ParserOpType;

typedef enum {
#define DECL_OP_MODEL_ENUM_ITEM(name) MODEL_##name,
	FOR_EACH_OP_MODEL(DECL_OP_MODEL_ENUM_ITEM)
	MODEL_COUNT
} Sophon_ParserOpModel;

typedef enum {
#define DECL_VALUE_FIELD_ENUM_ITEM(name) FIELD_##name,
	FOR_EACH_VALUE_FIELD(DECL_VALUE_FIELD_ENUM_ITEM)
	FIELD_COUNT
} Sophon_TokenValueField;

#define DECL_OP_MODEL_STRUCT(name) \
	typedef struct Sophon_Parser##name##_s Sophon_Parser##name;
FOR_EACH_OP_MODEL(DECL_OP_MODEL_STRUCT)

typedef struct Sophon_ParserExpr_s   Sophon_ParserExpr;
typedef struct Sophon_ParserCase_s   Sophon_ParserCase;
typedef struct Sophon_ParserObject_s Sophon_ParserObject;
typedef struct Sophon_ParserFunc_s   Sophon_ParserFunc;
typedef struct Sophon_ParserFrame_s  Sophon_ParserFrame;

struct Sophon_ParserCase_s {
	Sophon_ParserCase  *prev;
	Sophon_ParserCase  *next;
	Sophon_ParserOp    *cond_ops;
	Sophon_ParserOp    *job_ops;
};

struct Sophon_ParserExpr_s {
	Sophon_ParserOp    *pre_ops;
	Sophon_U32          bind_line;
	Sophon_Value        bind_namev;
	Sophon_ParserOp    *base_ops;
	Sophon_ParserOp    *name_ops;
	Sophon_ParserOp    *post_ops;
	Sophon_U8           pre_pushed;
	Sophon_U16          array_length;
};

struct Sophon_ParserOp_s {
	Sophon_ParserOp    *prev;
	Sophon_ParserOp    *next;
	Sophon_ParserOpType type;
	Sophon_U32          line;
};

struct Sophon_ParserConst_s {
	Sophon_ParserOp     op;
	Sophon_U16          id;
};

struct Sophon_ParserAnchor_s {
	Sophon_ParserOp     op;
	Sophon_U16          ip;
	Sophon_U16          sp;
};

struct Sophon_ParserJump_s {
	Sophon_ParserOp      op;
	Sophon_ParserAnchor *anchor;
	Sophon_U16           ip;
};

struct Sophon_ParserRef_s {
	Sophon_ParserOp      op;
	Sophon_ParserAnchor *anchor;
	Sophon_U8            sp;
};

struct Sophon_ParserCall_s {
	Sophon_ParserOp      op;
	Sophon_U8            argc;
};

struct Sophon_ParserStackOp_s {
	Sophon_ParserOp      op;
	Sophon_U8            n;
};

struct Sophon_ParserLabel_s {
	Sophon_ParserOp      op;
	Sophon_ParserBlock  *block;
	Sophon_U16           label_id;
};

struct Sophon_ParserBlock_s {
	Sophon_ParserOp      op;
	Sophon_ParserAnchor *brk_anchor;
	Sophon_ParserAnchor *cont_anchor;
};

struct Sophon_ParserLongJump_s {
	Sophon_ParserOp      op;
	Sophon_ParserAnchor *anchor;
	Sophon_U16           label_id;
	Sophon_U16           ip;
	Sophon_U16           sp;
};

#define SOPHON_PARSER_BIND_NOT_THROW 1

struct Sophon_ParserBind_s {
	Sophon_ParserOp      op;
	Sophon_U16           name_id;
	Sophon_U16           var_id;
	Sophon_U16           flags;
};

#define PROP_DEFINE_DATA 1
#define PROP_DEFINE_SET  2
#define PROP_DEFINE_GET  4

struct Sophon_ParserObject_s {
	Sophon_Hash          prop_hash;
};

struct Sophon_ParserFunc_s {
	Sophon_Function     *func;
	Sophon_U32           frame_bottom;
};

struct Sophon_ParserFrame_s {
	Sophon_ParserOp     *ops;
};

union Sophon_TokenValue_u {
	Sophon_Value   v;
	struct {
		Sophon_Value bodyv;
		Sophon_Value flagsv;
	} re;
	struct {
		Sophon_Value     v;
		Sophon_Value     name;
	} member;
	Sophon_ParserOp     *ops;
	Sophon_ParserExpr    expr;
	Sophon_Int           i;
	struct {
		Sophon_ParserOp *ops;
		Sophon_Value     namev;
	} var;
	Sophon_ParserCase   *cases;
};

typedef struct {
	Sophon_TokenValue v;
	Sophon_Location   l;
	Sophon_U16        t;
	Sophon_U16        s;
} Sophon_ParserStack;

#ifndef SOPHON_PARSER_STACK_SIZE
	#define SOPHON_PARSER_STACK_SIZE 256
#endif

#ifndef SOPHON_PARSER_OBJECT_STACK_SIZE
	#define SOPHON_PARSER_OBJECT_STACK_SIZE 64
#endif

#ifndef SOPHON_PARSER_FUNC_STACK_SIZE
	#define SOPHON_PARSER_FUNC_STACK_SIZE 64
#endif

#ifndef SOPHON_PARSER_FRAME_STACK_SIZE
	#define SOPHON_PARSER_FRAME_STACK_SIZE 64
#endif

typedef struct {
	Sophon_ParserStack  stack[SOPHON_PARSER_STACK_SIZE];
	Sophon_ParserStack  fetch;
	Sophon_ParserStack  reduce;
	Sophon_ParserStack  semicolon;
	Sophon_U32          top;
	Sophon_U32          flags;
	Sophon_U32          gc_level;
	Sophon_Module      *module;
	Sophon_ParserObject obj_stack[SOPHON_PARSER_OBJECT_STACK_SIZE];
	Sophon_ParserFunc   func_stack[SOPHON_PARSER_FUNC_STACK_SIZE];
	Sophon_ParserFrame  frame_stack[SOPHON_PARSER_FRAME_STACK_SIZE];
	Sophon_U32          obj_top;
	Sophon_U32          func_top;
	Sophon_U32          frame_top;
} Sophon_ParserData;

extern Sophon_ParserOpModel   sophon_parser_op_get_model (
								Sophon_ParserOpType op);

extern Sophon_TokenValueField sophon_token_value_get_field(
								Sophon_U16 tok);

#ifdef __cplusplus
}
#endif

#endif

