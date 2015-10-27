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
#include "js_lex_decl.h"

enum {
#define DECL_COND_ENUM_ITEM(name) C_##name,
	FOR_EACH_COND(DECL_COND_ENUM_ITEM)
	C_COUNT
};

enum {
	A_NONE,
#define DECL_KEYWORD_ENUM_ITEM(name)    A_##name,
	FOR_EACH_KEYWORD(DECL_KEYWORD_ENUM_ITEM)
#define DECL_PUNCT_ENUM_ITEM(name, str) A_##name,
	FOR_EACH_PUNCT(DECL_PUNCT_ENUM_ITEM)
#define DECL_ACTION_ENUM_ITEM(name)       A_##name,
	FOR_EACH_ACTION(DECL_ACTION_ENUM_ITEM)
	A_COUNT
};

static T_LexDecl *l_decl;

static void
add_rule (T_LexCond cond, const char *rule, T_LexToken tok)
{
	t_lex_decl_add_rule(l_decl, cond, rule, -1, tok, 0, NULL);
}

static void
build_js_lex (void)
{
#define KEYWORD_RULE(t)       add_rule(C_BEGIN, "\"" #t "\"", A_##t);

	FOR_EACH_KEYWORD(KEYWORD_RULE)

#define PUNCT_RULE(name, str) add_rule(C_BEGIN, "\"" #str "\"", A_##name);

	FOR_EACH_PUNCT(PUNCT_RULE)

#include "js_lex_rules.c"

	t_lex_decl_build(l_decl);
}

static void
dump_conds (void)
{
	printf("typedef enum {\n");

#define DUMP_COND_NAME(name) \
	printf("\tC_%s,\n", #name);

	FOR_EACH_COND(DUMP_COND_NAME)

	printf("\tC_COUNT\n");
	printf("} LexConditionType;\n");
}

static void
dump_terms (void)
{
	int i = 0;
	int kw_end;

	printf("typedef enum {\n");

	printf("\tA_NONE, /* %d */\n", i++);

#define DUMP_KEYWORD_NAME(name) \
	printf("\tA_%s, /* %d */\n", #name, i++);\

	FOR_EACH_KEYWORD(DUMP_KEYWORD_NAME)
	kw_end = i;

#define DUMP_PUNCT_NAME(name, str) \
	printf("\tA_%s, /* %d */\n", #name, i++);

	FOR_EACH_PUNCT(DUMP_PUNCT_NAME)

#define DUMP_ACTION_NAME(name) \
	printf("\tA_%s, /* %d */\n", #name, i++);

	FOR_EACH_ACTION(DUMP_ACTION_NAME)

	printf("\tA_COUNT /* %d */\n", i++);
	printf("} LexActionType;\n");

	printf("#define A_KEYWORD_END %d\n", kw_end);
}

static void
dump_dfa(int id, T_Auto *dfa)
{
	int v;
	int i;

	printf("static const Sophon_U16 js_dfa%d_state_edges[] = {", id);
	for (i = 0; i < dfa->states.nmem; i++) {
		v = dfa->states.buff[i].edges;
		assert(v <= 0xFFFF);

		if((i % 8) == 0) {
			printf("\n\t");
		}

		printf("0x%04x", (v & 0xFFFF));

		if (i != dfa->states.nmem - 1)
			printf(", ");
	}
	printf("\n};\n");

	printf("static const Sophon_U8 js_dfa%d_state_action[] = {", id);
	for (i = 0; i < dfa->states.nmem; i++) {
		v = dfa->states.buff[i].data;
		assert(v <= 0xFF);

		if((i % 8) == 0) {
			printf("\n\t");
		}

		printf("0x%02x", (v & 0xFF));

		if (i != dfa->states.nmem - 1)
			printf(", ");
	}
	printf("\n};\n");

	printf("static const Sophon_U16 js_dfa%d_edge_dest[] = {", id);
	for (i = 0; i < dfa->edges.nmem; i++) {
		v = dfa->edges.buff[i].dest;
		assert(v <= 0xFFFF);

		if((i % 8) == 0) {
			printf("\n\t");
		}

		printf("0x%04x", (v & 0xFFFF));

		if (i != dfa->edges.nmem - 1)
			printf(", ");
	}
	printf("\n};\n");

	printf("static const Sophon_U16 js_dfa%d_edge_next[] = {", id);
	for (i = 0; i < dfa->edges.nmem; i++) {
		v = dfa->edges.buff[i].next;
		assert(v <= 0xFFFF);

		if((i % 8) == 0) {
			printf("\n\t");
		}

		printf("0x%04x", (v & 0xFFFF));

		if (i != dfa->edges.nmem - 1)
			printf(", ");
	}
	printf("\n};\n");

	printf("static const Sophon_U32 js_dfa%d_edge_symbol[] = {", id);
	for (i = 0; i < dfa->edges.nmem; i++) {
		v = dfa->edges.buff[i].symbol;

		if((i % 8) == 0) {
			printf("\n\t");
		}

		printf("0x%08x", v);

		if (i != dfa->edges.nmem - 1)
			printf(", ");
	}
	printf("\n};\n");
}

static void
dump_dfa_array (void)
{
	int c;

	printf("typedef struct {\n");
	printf("\tconst Sophon_U16 *state_edges;\n");
	printf("\tconst Sophon_U8  *state_action;\n");
	printf("\tconst Sophon_U16 *edge_dest;\n");
	printf("\tconst Sophon_U16 *edge_next;\n");
	printf("\tconst Sophon_U32 *edge_symbol;\n");
	printf("} LexCond;\n");

	for (c = 0; c < l_decl->conds.nmem; c++){
		T_Auto *dfa = &l_decl->conds.buff[c];
		dump_dfa(c, dfa);
	}

	printf("static const LexCond js_lex_conds[] = {\n");
	for (c = 0; c < l_decl->conds.nmem; c++){
		printf("\t{\n");
		printf("\t\tjs_dfa%d_state_edges,\n", c);
		printf("\t\tjs_dfa%d_state_action,\n", c);
		printf("\t\tjs_dfa%d_edge_dest,\n", c);
		printf("\t\tjs_dfa%d_edge_next,\n", c);
		printf("\t\tjs_dfa%d_edge_symbol\n", c);
		printf("\t}");

		if (c != l_decl->conds.nmem - 1)
			printf(",");

		printf("\n");
	}
	printf("};\n");
}

static void
dump_js_lex (void)
{
	dump_conds();
	dump_terms();
	dump_dfa_array();
}

int
main (int argc, char **argv)
{
	l_decl = t_lex_decl_create();

	build_js_lex();

	dump_js_lex();

	t_lex_decl_destroy(l_decl);
	return 0;
}

