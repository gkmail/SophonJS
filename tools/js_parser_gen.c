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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tea/tea.h>

#include "js_keywords.h"
#include "js_puncts.h"
#include "js_terms.h"
#include "js_nonterms.h"
#include "js_parser_decl.h"

enum {
	T_BEGIN = 255,
#define DECL_KEYWORD_ENUM_ITEM(name)    T_##name,
	FOR_EACH_KEYWORD(DECL_KEYWORD_ENUM_ITEM)
	FOR_EACH_FUTURE_KEYWORD(DECL_KEYWORD_ENUM_ITEM)
#define DECL_PUNCT_ENUM_ITEM(name, str) T_##name,
	FOR_EACH_PUNCT(DECL_PUNCT_ENUM_ITEM)
#define DECL_TERM_ENUM_ITEM(name, field) T_##name,
	FOR_EACH_TERM(DECL_TERM_ENUM_ITEM)
	T_END
};

enum {
	N_BEGIN = T_PARSER_NONTERM_BEGIN - 1,
#define DECL_NONTERM_ENUM_ITEM(name, field) N_##name,
	FOR_EACH_NONTERM(DECL_NONTERM_ENUM_ITEM)
	N_END
};

enum {
	P_COMMA  = T_PARSER_PRIO(0, T_PARSER_PRIO_LEFT),
	P_ASSIGN = T_PARSER_PRIO(1, T_PARSER_PRIO_RIGHT),
	P_COND   = T_PARSER_PRIO(2, T_PARSER_PRIO_RIGHT),
	P_OR     = T_PARSER_PRIO(3, T_PARSER_PRIO_LEFT),
	P_AND    = T_PARSER_PRIO(4, T_PARSER_PRIO_LEFT),
	P_BOR    = T_PARSER_PRIO(5, T_PARSER_PRIO_LEFT),
	P_XOR    = T_PARSER_PRIO(6, T_PARSER_PRIO_LEFT),
	P_BAND   = T_PARSER_PRIO(7, T_PARSER_PRIO_LEFT),
	P_EQU    = T_PARSER_PRIO(8, T_PARSER_PRIO_LEFT),
	P_REL    = T_PARSER_PRIO(9, T_PARSER_PRIO_LEFT),
	P_SHIFT  = T_PARSER_PRIO(10, T_PARSER_PRIO_LEFT),
	P_ADD    = T_PARSER_PRIO(11, T_PARSER_PRIO_LEFT),
	P_MUL    = T_PARSER_PRIO(12, T_PARSER_PRIO_LEFT),
	P_UNARY  = T_PARSER_PRIO(13, T_PARSER_PRIO_RIGHT),
	P_MEMBER = T_PARSER_PRIO(14, T_PARSER_PRIO_LEFT),
	P_NEW    = T_PARSER_PRIO(15, T_PARSER_PRIO_LEFT)
};

enum {
	R_BEGIN = T_PARSER_REDUCE_BEGIN - 1,
#define DECL_REDUCE_ENUM_ITEM(name) R_##name,
	FOR_EACH_REDUCE(DECL_REDUCE_ENUM_ITEM)
	R_END
};

static T_ParserDecl *p_decl;

static T_Bool
js_prod_check(T_ParserDecl *decl, T_ParserClosure *clos, T_ParserProd *to)
{
	int nonterm = T_PARSER_NONTERM(to->rule_id);

	if ((nonterm == N_OBJECT_LITERAL) || (nonterm == N_FUNC_EXPR)) {
		int i;

		for (i = 0; i < clos->prods.nmem; i++) {
			T_ParserProd *prod = &clos->prods.buff[i];
			int fnt, dot;

			fnt  = T_PARSER_NONTERM(prod->rule_id);
			dot  = prod->dot;

			if ((fnt == N_EXPR_STATEMENT) && (dot == 0))
				return T_FALSE;
		}
	}

	return T_TRUE;
}

static void
build_js_parser (void)
{
#define add_rule(n...) t_parser_decl_add_rule(p_decl, n, -1)

#include "js_parser_rules.c"

	p_decl->prod_check = js_prod_check;
	t_parser_decl_build(p_decl);
}

static void
dump_js_parser_h (void)
{
	int i = 255;

	printf("typedef enum {\n");
	printf("\tT_BEGIN = %d,\n", i++);

#define DUMP_KEYWORD(name)    printf("\tT_%s,\t/* %d */\n", #name, i++);
	FOR_EACH_KEYWORD(DUMP_KEYWORD)
	FOR_EACH_FUTURE_KEYWORD(DUMP_KEYWORD)

#define DUMP_PUNCT(name, str) printf("\tT_%s,\t/* %d */\n", #name, i++);
	FOR_EACH_PUNCT(DUMP_PUNCT)

#define DUMP_TERM(name, field) printf("\tT_%s,\t/* %d */\n", #name, i++);
	FOR_EACH_TERM(DUMP_TERM)

	printf("\tN_START = %d,\n", i++);

#define DUMP_NONTERM(name, field) printf("\tN_%s,\t/* %d */\n", #name, i++);
	FOR_EACH_NONTERM(DUMP_NONTERM)

	printf("\tT_END\n");
	printf("} ParserTokenType;\n");

	i = 1;
	printf("typedef enum {\n");
	printf("\tR_BEGIN,\n");
#define DUMP_REDUCE(name) printf("\tR_%s,\t/* %d */\n", #name, i++);
	FOR_EACH_REDUCE(DUMP_REDUCE)
	printf("\tR_END\n");
	printf("} ParserReduceType;\n");

	printf("#define N_ERROR    0xFFF\n");
	printf("#define N_REDUCE   0xFFE\n");
	printf("#define T_EPSILON  0xFFFF\n");
	printf("#define R_NONE     0xFFFFFFFF\n");

	printf("#define RV_GET_REDUCE(r)     ((r) & 0xFFF)\n");
	printf("#define RV_GET_TOKEN(r)      (((r) >> 12) & 0xFFF)\n");
	printf("#define RV_GET_POP(r)        (((r) >> 24) & 0x7F)\n");
	printf("#define RV_GET_POP_FLAG(r)   ((r) & 0x80000000)\n");

	printf("#define TOKEN_FIELD_TABLE_BEGIN T_NUMBER\n");
}

static void
dump_js_parser_c (void)
{
	int i;

	printf("static const char* js_token_names[] = {\n");
#define DUMP_KEYWORD_NAME(name)    printf("\t\"%s\",\n", #name);
	FOR_EACH_KEYWORD(DUMP_KEYWORD_NAME)
	FOR_EACH_FUTURE_KEYWORD(DUMP_KEYWORD_NAME)

#define DUMP_PUNCT_NAME(name, str) printf("\t\"%s\",\n", #str);
	FOR_EACH_PUNCT(DUMP_PUNCT_NAME)

#define DUMP_TERM_NAME(name, field) printf("\t\"%s\",\n", #name);
	FOR_EACH_TERM(DUMP_TERM_NAME)
	printf("\tNULL\n");
	printf("};\n");


	printf("static const Sophon_U16 js_state_shifts[] = {");

	for (i = 0; i < p_decl->states.nmem; i++) {
		unsigned int v;

		v = p_decl->states.buff[i].shifts;
		assert((v < 0xFFFE) || (v == -1));

		if (v == -1)
			v = 0xFFFF;

		if ((i % 16) == 0)
			printf("\n\t");

		printf("0x%04x", v);
		if (i != p_decl->states.nmem - 1)
			printf(", ");
	}

	printf("\n};\n");


	printf("static const Sophon_U32 js_state_reduce[] = {");

	for (i = 0; i < p_decl->states.nmem; i++) {
		unsigned int r;
		unsigned int v;
		unsigned int rid, tid, pop, flag;

		r = p_decl->states.buff[i].reduce;

		if (r == T_PARSER_REDUCE_NONE) {
			v = 0xFFFFFFFF;
		} else {
			rid  = r & 0xFFF;
			tid  = (r >> 12) & 0xFFF;
			pop  = (r >> 24) & 0x7F;
			flag = r & 0x80000000;

			tid = tid + T_END;
			assert(tid < 0xFFE);

			v = flag | (pop << 24) | (tid << 12) | rid;
		}

		if ((i % 8) == 0)
			printf("\n\t");

		printf("0x%08x", v);
		if (i != p_decl->states.nmem - 1)
			printf(", ");
	}

	printf("\n};\n");


	printf("static const Sophon_U16 js_state_jumps[] = {");

	for (i = 0; i < p_decl->states.nmem; i++) {
		unsigned int v;

		v = p_decl->states.buff[i].jumps;
		assert((v < 0xFFFF) || (v == -1));

		if (v == -1)
			v = 0xFFFF;

		if ((i % 16) == 0)
			printf("\n\t");

		printf("0x%04x", v);
		if (i != p_decl->states.nmem - 1)
			printf(", ");
	}

	printf("\n};\n");

	printf("static const Sophon_U16 js_edge_next[] = {");

	for (i = 0; i < p_decl->edges.nmem; i++) {
		unsigned int v;

		v = p_decl->edges.buff[i].next;
		assert((v < 0xFFFF) || (v == -1));

		if (v == -1)
			v = 0xFFFF;

		if ((i % 16) == 0)
			printf("\n\t");

		printf("0x%04x", v);
		if (i != p_decl->edges.nmem - 1)
			printf(", ");
	}

	printf("\n};\n");


	printf("static const Sophon_U16 js_edge_symbol[] = {");

	for (i = 0; i < p_decl->edges.nmem; i++) {
		int s;
		unsigned int v;

		s = p_decl->edges.buff[i].symbol;

		if (s == T_PARSER_EPSILON) {
			v = 0xFFFF;
		} else if (T_PARSER_IS_TERM(s)) {
			v = s;
		} else if (s == T_PARSER_NONTERM_ERR) {
			v = 0xFFF;
		} else if (s == T_PARSER_NONTERM_REDUCE) {
			v = 0xFFE;
		} else if (T_PARSER_IS_NONTERM(s)) {
			v = T_PARSER_NONTERM_ID(s) + T_END;
			assert(v < 0xFFE);
		} else {
			assert(0);
		}

		if ((i % 16) == 0)
			printf("\n\t");

		printf("0x%04x", v);
		if (i != p_decl->edges.nmem - 1)
			printf(", ");
	}

	printf("\n};\n");


	printf("static const Sophon_U16 js_edge_dest[] = {");

	for (i = 0; i < p_decl->edges.nmem; i++) {
		unsigned int v;

		v = p_decl->edges.buff[i].dest;
		assert(v <= 0xFFFF);

		if ((i % 16) == 0)
			printf("\n\t");

		printf("0x%04x", v);
		if (i != p_decl->edges.nmem - 1)
			printf(", ");
	}

	printf("\n};\n");

	printf("static const Sophon_U8 js_token_field_table[] = {\n");
#define DUMP_TOKEN_FIELD(name, field) printf("\tFIELD_%s,\n", #field);
	FOR_EACH_TERM(DUMP_TOKEN_FIELD)
	printf("\tFIELD_i,\n");
	FOR_EACH_NONTERM(DUMP_TOKEN_FIELD)
	printf("\t0xFF");
	printf("\n};\n");
}

int
main (int argc, char **argv)
{
	p_decl = t_parser_decl_create();

	build_js_parser();

	if ((argc > 1) && !strcmp(argv[1], "h")) {
		dump_js_parser_h();
	} else {
		dump_js_parser_c();
	}

	t_parser_decl_destroy(p_decl);

	return 0;
}

