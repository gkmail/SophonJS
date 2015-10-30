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
#include <sophon_string.h>
#include <sophon_debug.h>

#include "sophon_regexp_internal.h"

#if 1
#define DEBUG(a) SOPHON_INFO(a)
#else
#define DEBUG(a)
#endif

#ifndef SOPHON_REGEXP_BRANCH_COUNT
	#define SOPHON_REGEXP_BRANCH_COUNT 64
#endif

#define CSET_WORD   0x10000
#define CSET_NWORD  0x20000
#define CSET_SPACE  0x30000
#define CSET_NSPACE 0x40000
#define CSET_DIGIT  0x50000
#define CSET_NDIGIT 0x60000

#define sophon_isword(c) (sophon_isalnum(c) || ((c) == '_'))

static Sophon_Result parse_disjunction (Sophon_VM *vm, ReInput *inp,
			ReNode **root);

static ReNode*
add_node (Sophon_VM *vm, ReInput *inp, ReNodeType type)
{
	ReNode *node;

	node = (ReNode*)sophon_mm_alloc_ensure(vm, sizeof(ReNode));

	node->type = type;
	node->prev = node;
	node->next = node;
	node->n.alter.left  = NULL;
	node->n.alter.right = NULL;

	node->alloc_next = inp->nodes;
	inp->nodes = node;

	return node;
}

static void
append_node (Sophon_VM *vm, ReNode **list, ReNode *node)
{
	if (!node)
		return;

	if (*list == NULL) {
		*list = node;
	} else {
		ReNode *h1, *h2, *t1, *t2;

		h1 = (*list);
		t1 = (*list)->prev;
		h2 = node;
		t2 = node->prev;

		h1->prev = t2;
		t1->next = h2;
		h2->prev = t1;
		t2->next = h1;
	}
}

static ReNode*
add_append_node (Sophon_VM *vm, ReInput *inp, ReNode **list, ReNodeType type)
{
	ReNode *node = add_node(vm, inp, type);

	append_node(vm, list, node);
	return node;
}

static Sophon_Result
add_range (Sophon_VM *vm, ReInput *inp, Sophon_Char min, Sophon_Char max)
{
	if (inp->cr_len >= inp->cr_cap) {
		Sophon_U32 size = SOPHON_MAX(16, inp->cr_cap * 2);
		
		inp->cr_buf = (Sophon_CharRange*)sophon_mm_realloc_ensure(vm,
					inp->cr_buf,
					sizeof(Sophon_CharRange) * inp->cr_cap,
					sizeof(Sophon_CharRange) * size);

		inp->cr_cap = size;
	}

	inp->cr_buf[inp->cr_len] = SOPHON_MAKE_CHAR_RANGE(min, max);
	inp->cr_len++;

	return SOPHON_OK;
}

static Sophon_Result
add_set (Sophon_VM *vm, ReInput *inp, Sophon_Int set)
{
	switch (set) {
		case CSET_WORD:
			add_range(vm, inp, '0', '9');
			add_range(vm, inp, 'A', 'Z');
			add_range(vm, inp, '_', '_');
			add_range(vm, inp, 'a', 'z');
			break;
		case CSET_NWORD:
			add_range(vm, inp, 0, '0'-1);
			add_range(vm, inp, '9'+1, 'A'-1);
			add_range(vm, inp, 'Z'+1, '_'-1);
			add_range(vm, inp, '_'+1, 'a'-1);
			add_range(vm, inp, 'z'+1, 0xFFFF);
			break;
		case CSET_SPACE:
			add_range(vm, inp, 0x9, 0xD);
			add_range(vm, inp, 0x20, 0x20);
			add_range(vm, inp, 0xA0, 0xA0);
			add_range(vm, inp, 0x2028, 0x2029);
			add_range(vm, inp, 0xFEFF, 0xFEFF);
			break;
		case CSET_NSPACE:
			add_range(vm, inp, 0, 0x8);
			add_range(vm, inp, 0xE, 0x1F);
			add_range(vm, inp, 0x21, 0x9F);
			add_range(vm, inp, 0xA1, 0x2027);
			add_range(vm, inp, 0x202a, 0xFEFE);
			add_range(vm, inp, 0xFF00, 0xFFFF);
			break;
		case CSET_DIGIT:
			add_range(vm, inp, '0', '9');
			break;
		case CSET_NDIGIT:
			add_range(vm, inp, 0, '0'-1);
			add_range(vm, inp, '9'+1, 0xFFFF);
			break;
		default:
			add_range(vm, inp, set, set);
			break;
	}

	return SOPHON_OK;
}

static Sophon_Int
getch (ReInput *inp)
{
	if (inp->pos >= inp->len)
		return -1;

	return inp->chars[inp->pos++];
}

static void
unget (ReInput *inp, Sophon_Int ch)
{
	if (ch < 0)
		return;

	SOPHON_ASSERT(inp->pos > 0);
	inp->pos--;
}

static Sophon_Int
parse_num (Sophon_VM *vm, ReInput *inp)
{
	Sophon_Int n = 0;
	Sophon_Int ch;

	ch = getch(inp);
	if (!sophon_isdigit(ch))
		return SOPHON_ERR_PARSE;

	n = ch - '0';
	while (1) {
		ch = getch(inp);
		if (!sophon_isdigit(ch)) {
			unget(inp, ch);
			break;
		}

		n *= 10;
		n += ch - '0';
	}

	return n;
}

static Sophon_Int
hex_val (Sophon_Char ch)
{
	if ((ch >= '0') && (ch <= '9'))
		return ch - '0';
	else if ((ch >= 'a') && (ch <= 'f'))
		return ch - 'a' + 10;
	else
		return ch - 'A' + 10;
}

static Sophon_Int
parse_escape (Sophon_VM *vm, ReInput *inp, Sophon_Int ch)
{
	Sophon_Int n;

	switch (ch) {
		case 'd':
			return CSET_DIGIT;
		case 'D':
			return CSET_NDIGIT;
		case 'w':
			return CSET_WORD;
		case 'W':
			return CSET_NWORD;
		case 's':
			return CSET_SPACE;
		case 'S':
			return CSET_NSPACE;
		case 'f':
			return '\f';
		case 'n':
			return '\n';
		case 'r':
			return '\r';
		case 't':
			return '\t';
		case 'v':
			return '\v';
		case 'c':
			ch = getch(inp);
			if (!sophon_isalpha(ch))
				return SOPHON_ERR_PARSE;
			return ch % 32;
		case 'u':
			ch = getch(inp);
			if (!sophon_isxdigit(ch))
				return SOPHON_ERR_PARSE;
			n = (hex_val(ch) << 12);
			ch = getch(inp);
			if (!sophon_isxdigit(ch))
				return SOPHON_ERR_PARSE;
			n |= (hex_val(ch) << 8);
			ch = getch(inp);
			if (!sophon_isxdigit(ch))
				return SOPHON_ERR_PARSE;
			n |= (hex_val(ch) << 4);
			ch = getch(inp);
			if (!sophon_isxdigit(ch))
				return SOPHON_ERR_PARSE;
			n |= hex_val(ch);
			return n;
		case 'x':
			ch = getch(inp);
			if (!sophon_isxdigit(ch))
				return SOPHON_ERR_PARSE;
			n = hex_val(ch) << 4;
			ch = getch(inp);
			if (!sophon_isxdigit(ch))
				return SOPHON_ERR_PARSE;
			n |= hex_val(ch);
			return n;
		default:
			return ch;
	}

	return 0;
}

static Sophon_Result
parse_class_atom (Sophon_VM *vm, ReInput *inp, Sophon_Bool none, Sophon_Int *set)
{
	Sophon_Int ch;

	ch = getch(inp);
	switch (ch) {
		case '-':
			*set = '-';
			break;
		case ']':
			if (none) {
				unget(inp, ch);
				return SOPHON_NONE;
			}
			*set = ']';
			break;
		case '\\':
			ch = getch(inp);
			if (ch == 'b') {
				*set = 0x08;
			} else if (sophon_isdigit(ch)) {
				unget(inp, ch);
				ch = parse_num(vm, inp);
				*set = ch;
			} else {
				if ((ch = parse_escape(vm, inp, ch)) < 0)
					return ch;
				*set = ch;
			}
			break;
		default:
			if (ch == -1)
				return SOPHON_ERR_PARSE;
			*set = ch;
			break;
	}

	return SOPHON_OK;
}

static Sophon_Result
parse_class_range (Sophon_VM *vm, ReInput *inp)
{
	Sophon_Int min, max;
	Sophon_Int ch;
	Sophon_Result r;

	if ((r = parse_class_atom(vm, inp, SOPHON_TRUE, &min)) != SOPHON_OK)
		return r;

	if (min > 0xFFFF) {
		add_set(vm, inp, min);
		return SOPHON_OK;
	}

	ch = getch(inp);
	if (ch == '-') {
		if ((r = parse_class_atom(vm, inp, SOPHON_FALSE, &max)) != SOPHON_OK)
			return r;

		add_range(vm, inp, min, max);
	} else {
		unget(inp, ch);

		add_range(vm, inp, min, min);
	}

	return SOPHON_OK;
}

static Sophon_Result
parse_class (Sophon_VM *vm, ReInput *inp, ReNode **root)
{
	Sophon_Int ch;
	Sophon_Result r;
	ReNodeType type;
	ReNode *node;

	ch = getch(inp);
	if (ch == '^') {
		type = N_NCLASS;
	} else {
		unget(inp, ch);
		type = N_CLASS;
	}

	node = add_append_node(vm, inp, root, type);

	node->n.clazz.begin = inp->cr_len;

	r = parse_class_range(vm, inp);
	if (r <= 0)
		return r;

	while (1) {
		r = parse_class_range(vm, inp);
		if (r < 0) {
			return r;
		} else if (r == SOPHON_NONE) {
			r = SOPHON_OK;
			break;
		}
	}

	ch = getch(inp);
	if (ch != ']')
		return SOPHON_ERR_PARSE;

	node->n.clazz.end = inp->cr_len;

	return r;
}

static Sophon_Result
parse_term (Sophon_VM *vm, ReInput *inp, ReNode **root)
{
	Sophon_Int ch;
	Sophon_Bool atom = SOPHON_TRUE;
	ReNode *node = NULL;
	Sophon_Result r;

	ch = getch(inp);
	switch (ch) {
		case '^':
			node = add_node(vm, inp, N_BOL);
			atom = SOPHON_FALSE;
			break;
		case '$':
			node = add_node(vm, inp, N_EOL);
			atom = SOPHON_FALSE;
			break;
		case '\\':
			ch = getch(inp);
			switch (ch) {
				case 'b':
					node = add_node(vm, inp, N_B);
					atom = SOPHON_FALSE;
					break;
				case 'B':
					node = add_node(vm, inp, N_NB);
					atom = SOPHON_FALSE;
					break;
				case '0':
					node = add_node(vm, inp, N_NB);
					node->n.ch =0;
					break;
				default:
					if (sophon_isdigit(ch)) {
						unget(inp, ch);
						ch = parse_num(vm, inp);
						if (ch > inp->cap_end) {
							return SOPHON_ERR_PARSE;
						}

						node = add_node(vm, inp, N_REF);
						node->n.ref = ch;
					} else {
						if ((ch = parse_escape(vm, inp, ch)) < 0)
							return ch;
						if (ch < 0xFFFF) {
							node = add_node(vm, inp, N_CHAR);
							node->n.ch = ch;
						} else {
							node = add_node(vm, inp, N_CLASS);
							node->n.clazz.begin = inp->cr_len;
							add_set(vm, inp, ch);
							node->n.clazz.end   = inp->cr_len;
						}
					}
					break;
			}
			break;
		case '(': {
			ReNode **list;
			Sophon_Bool cap = SOPHON_FALSE;

			ch = getch(inp);
			if (ch == '?') {
				ch = getch(inp);
				switch (ch) {
					case '=':
						node = add_node(vm, inp, N_LA);
						list = &node->n.la;
						atom = SOPHON_FALSE;
						break;
					case '!':
						node = add_node(vm, inp, N_NLA);
						list = &node->n.la;
						atom = SOPHON_FALSE;
						break;
					case ':':
						list = &node;
						break;
					default:
						return SOPHON_ERR_PARSE;
				}
			} else {
				if (inp->cap_begin >= 255)
					sophon_fatal("too many captures");
				unget(inp, ch);
				node = add_node(vm, inp, N_CAP);
				node->n.capture.id = ++inp->cap_begin;
				list = &node->n.capture.node;
				cap = SOPHON_TRUE;
			}

			if ((r = parse_disjunction(vm, inp, list)) != SOPHON_OK)
				return r;

			ch = getch(inp);
			if (ch != ')')
				return SOPHON_ERR_PARSE;

			if (cap) {
				inp->cap_end++;
			}
			break;
		}
		case '.':
			node = add_node(vm, inp, N_ANY);
			break;
		case '[':
			if ((r = parse_class(vm, inp, &node)) != SOPHON_OK)
				return r;
			break;
		default:
			if ((ch == '|') || (ch == '+') || (ch == '*') || (ch == '?') ||
						(ch == ')') || (ch == ']') || (ch == '{') ||
						(ch == '}') || (ch == -1)) {
				unget(inp, ch);
				return SOPHON_NONE;
			}

			node = add_node(vm, inp, N_CHAR);
			node->n.ch = ch;
			break;
	}

	if (atom) {
		Sophon_Int min, max;
		ReNode *rep;
		Sophon_Bool lazy;

		ch = getch(inp);
		switch(ch) {
			case '*':
				min = -1;
				max = -1;
				break;
			case '+':
				min = 1;
				max = -1;
				break;
			case '?':
				min = 0;
				max = 1;
				break;
			case '{':
				if ((r = parse_num(vm, inp)) < 0)
					return r;
				min = r;

				ch = getch(inp);
				if (ch == ',') {
					ch = getch(inp);
					if (ch != '}') {
						unget(inp, ch);
						if ((r = parse_num(vm, inp)) < 0)
							return r;
						max = r;

						ch = getch(inp);
						if (ch != '}')
							return SOPHON_ERR_PARSE;
					} else {
						max = -1;
					}
				} else if (ch != '}') {
					return SOPHON_ERR_PARSE;
				} else {
					max = min;
				}
				break;
			default:
				unget(inp, ch);
				goto no_repeat;
		}

		ch = getch(inp);
		if (ch == '?') {
			lazy = SOPHON_TRUE;
		} else {
			unget(inp, ch);
			lazy = SOPHON_FALSE;
		}

		rep = add_append_node(vm, inp, root, N_REPEAT);

		rep->n.repeat.body = node;
		rep->n.repeat.min  = min;
		rep->n.repeat.max  = max;
		rep->n.repeat.lazy = lazy;
		rep->n.repeat.cap  = inp->cap_begin + 1;

		return SOPHON_OK;
	}

no_repeat:
	append_node(vm, root, node);

	return SOPHON_OK;
}

static Sophon_Result
parse_alternative (Sophon_VM *vm, ReInput *inp, ReNode **root)
{
	Sophon_Result r;

	if ((r = parse_term(vm, inp, root)) == SOPHON_NONE)
		return SOPHON_OK;

	if (r < 0)
		return r;

	while (1) {
		if ((r = parse_term(vm, inp, root)) == SOPHON_NONE)
			break;
		if (r < 0)
			return r;
	}

	return SOPHON_OK;
}

static Sophon_Result
parse_disjunction (Sophon_VM *vm, ReInput *inp, ReNode **root)
{
	ReNode *node, *alter;
	Sophon_Int ch;
	Sophon_Result r;

	node = NULL;
	if ((r = parse_alternative(vm, inp, &node)) != SOPHON_OK)
		return r;

	while (1) {
		ch = getch(inp);
		if (ch == '|') {

			alter = add_append_node(vm, inp, root, N_ALTER);
			alter->n.alter.left = node;
			root = &alter->n.alter.right;
			node = NULL;

			if ((r = parse_alternative(vm, inp, &node))
						!= SOPHON_OK)
				return r;
		} else {
			unget(inp, ch);
			append_node(vm, root, node);
			break;
		}
	}

	return SOPHON_OK;
}

static Sophon_Result
parse (Sophon_VM *vm, ReInput *inp)
{
	Sophon_Result r;

	r = parse_disjunction(vm, inp, &inp->root);
	if (inp->pos != inp->len)
		r = SOPHON_ERR_PARSE;

	if (r != SOPHON_OK) {
		sophon_throw(vm, vm->SyntaxError, "Parse regexp error");
	}

	return r;
}

static Sophon_Int
node_calc (ReInput *inp, ReNode *node)
{
	ReNode *n = node;
	Sophon_Int r = 0, c, l;

	if (!n)
		return r;

	do {
		switch (n->type) {
			case N_ANY:
			case N_CHAR:
			case N_CLASS:
			case N_NCLASS:
			case N_BOL:
			case N_EOL:
			case N_B:
			case N_NB:
			case N_REF:
				r++;
				break;
			case N_LA:
			case N_NLA:
				r += node_calc(inp, n->n.la) + 2;
				break;
			case N_CAP:
				r += node_calc(inp, n->n.capture.node) + 2;
				break;
			case N_ALTER:
				r += node_calc(inp, n->n.alter.left);
				r += node_calc(inp, n->n.alter.right);
				r += 2;
				break;
			case N_REPEAT:
				if (inp->loop_level >= 255)
					sophon_fatal("too many repeats");

				n->n.repeat.loop = inp->cap_end + 1 + inp->loop_level++;
				inp->loop_depth  = SOPHON_MAX(inp->loop_depth,
							inp->loop_level);

				c = node_calc(inp, n->n.repeat.body) + 1;
				l = 0;

				inp->loop_level--;

				if (n->n.repeat.min > 0) {
					l += n->n.repeat.min * c;
				}
				if (n->n.repeat.max > n->n.repeat.min) {
					Sophon_Int v = n->n.repeat.max - n->n.repeat.min;
					l += v * (c + 3);
				}
				if (n->n.repeat.max < 0) {
					l += c + 4;
				}

				n->n.repeat.isize = l;
				r += l;
				break;
		}

		n = n->next;
	} while (n != node);

	return r;
}

static ReIns*
node_gen_ins (ReInput *inp, ReNode *node, ReIns *ins)
{
	ReNode *n = node;
	ReIns *pi = ins;
	ReIns *pi2, *pi3;
	Sophon_Int i;

	if (!n)
		return pi;

	do {
		switch (n->type) {
			case N_ANY:
			case N_BOL:
			case N_EOL:
			case N_B:
			case N_NB:
				pi->type = n->type;
				pi++;
				break;
			case N_CHAR:
				pi->type = I_CHAR;
				pi->i.ch = n->n.ch;
				pi++;
				break;
			case N_CLASS:
			case N_NCLASS:
				pi->type = (n->type == N_CLASS) ? I_CLASS : I_NCLASS;
				pi->i.clazz.begin = n->n.clazz.begin;
				pi->i.clazz.end   = n->n.clazz.end;
				pi++;
				break;
			case N_REF:
				pi->type  = I_REF;
				pi->i.ref = n->n.ref;
				pi++;
				break;
			case N_LA:
			case N_NLA:
				pi->type = (n->type == N_LA) ? I_LA : I_NLA;
				pi2 = node_gen_ins(inp, n->n.la, pi + 1);
				pi2->type = I_END;
				pi2++;
				pi->i.jmp = pi2;
				pi = pi2;
				break;
			case N_CAP:
				pi->type  = I_CAPB;
				pi->i.cap = n->n.capture.id;
				pi++;
				pi = node_gen_ins(inp, n->n.capture.node, pi);
				pi->type  = I_CAPE;
				pi->i.cap = n->n.capture.id;
				pi++;
				break;
			case N_ALTER:
				pi->type  = I_BRANCH;
				pi->i.branch.b1 = pi + 1;
				pi2 = node_gen_ins(inp, n->n.alter.left, pi + 1);
				pi2->type = I_JMP;
				pi->i.branch.b2 = pi2 + 1;
				pi3 = node_gen_ins(inp, n->n.alter.right, pi2 + 1);
				pi2->i.jmp = pi3;
				pi = pi3;
				break;
			case N_REPEAT:
				pi2 = pi + n->n.repeat.isize;

				SOPHON_INFO(("repeat size %d", n->n.repeat.isize));

				if (n->n.repeat.min > 0) {
					for (i = 0; i < n->n.repeat.min; i++) {
						pi->type  = I_CLEAR;
						pi->i.cap = n->n.repeat.cap;
						pi++;

						pi = node_gen_ins(inp, n->n.repeat.body, pi);
					}
				}
				if (n->n.repeat.max > n->n.repeat.min) {
					i = n->n.repeat.max - n->n.repeat.min;
					while (i--) {
						pi->type  = I_BRANCH;
						if (n->n.repeat.lazy) {
							pi->i.branch.b1 = pi2;
							pi->i.branch.b2 = pi + 1;
						} else {
							pi->i.branch.b2 = pi2;
							pi->i.branch.b1 = pi + 1;
						}
						pi++;

						pi->type  = I_CLEAR;
						pi->i.cap = n->n.repeat.cap;
						pi++;

						pi->type  = I_CAPB;
						pi->i.cap = n->n.repeat.loop;
						pi++;

						pi = node_gen_ins(inp, n->n.repeat.body, pi);

						pi->type  = I_EMPTY;
						pi->i.empty.cap = n->n.repeat.cap;
						pi->i.empty.jmp = pi2;
						pi++;
					}
				}
				if (n->n.repeat.max < 0) {
					ReIns *istart = pi;

					pi->type = I_BRANCH;
					if (n->n.repeat.lazy) {
						pi->i.branch.b1 = pi2;
						pi->i.branch.b2 = pi + 1;
					} else {
						pi->i.branch.b2 = pi2;
						pi->i.branch.b1 = pi + 1;
					}
					pi++;

					pi->type  = I_CLEAR;
					pi->i.cap = n->n.repeat.cap;
					pi++;

					pi->type  = I_CAPB;
					pi->i.cap = n->n.repeat.loop;
					pi++;

					pi = node_gen_ins(inp, n->n.repeat.body, pi);

					pi->type  = I_EMPTY;
					pi->i.empty.cap = n->n.repeat.cap;
					pi->i.empty.jmp = pi2;
					pi++;
				
					pi->type  = I_JMP;
					pi->i.jmp = istart;
					pi++;
				}

				SOPHON_ASSERT(pi == pi2);
				pi = pi2;
				break;
			default:
				SOPHON_ASSERT(0);
		}

		n = n->next;
	} while (n != node);

	return pi;
}

/*#define SOPHON_DEBUG*/
#ifdef SOPHON_DEBUG

static void
dump_class (Sophon_CharRange *cr_buf, Sophon_U32 begin, Sophon_U32 end)
{
	Sophon_CharRange *cr = cr_buf + begin;
	Sophon_CharRange *cr_end = cr_buf + end;

	while (cr < cr_end) {
		Sophon_Char min, max;

		SOPHON_EXPAND_CHAR_RANGE(*cr, min, max);
		sophon_pr("%04x-%04x ", min, max);
		cr++;
	}
}

static void
dump_ins (ReIns *ins, Sophon_U32 size, Sophon_CharRange *cr_buf)
{
	ReIns *pi = ins;
	ReIns *pi_end = pi + size;

	sophon_pr("regexp:\n");
	while (pi < pi_end) {
		sophon_pr("%05d: ", pi - ins);
		switch (pi->type) {
			case I_LA:
				sophon_pr("la     %d\n", pi->i.jmp - ins);
				break;
			case I_NLA:
				sophon_pr("nla    %d\n", pi->i.jmp - ins);
				break;
			case I_ANY:
				sophon_pr("any\n");
				break;
			case I_CHAR:
				sophon_pr("char   %04x\n", pi->i.ch);
				break;
			case I_CLASS:
				sophon_pr("class  ");
				dump_class(cr_buf, pi->i.clazz.begin, pi->i.clazz.end);
				sophon_pr("\n");
				break;
			case I_NCLASS:
				sophon_pr("nclass ");
				dump_class(cr_buf, pi->i.clazz.begin, pi->i.clazz.end);
				sophon_pr("\n");
				break;
			case I_BOL:
				sophon_pr("bol\n");
				break;
			case I_EOL:
				sophon_pr("eol\n");
				break;
			case I_B:
				sophon_pr("b\n");
				break;
			case I_NB:
				sophon_pr("nb\n");
				break;
			case I_REF:
				sophon_pr("ref    %d\n", pi->i.ref);
				break;
			case I_CAPB:
				sophon_pr("capb   %d\n", pi->i.cap);
				break;
			case I_CAPE:
				sophon_pr("cape   %d\n", pi->i.cap);
				break;
			case I_BRANCH:
				sophon_pr("branch %d %d\n",
						pi->i.branch.b1 - ins,
						pi->i.branch.b2 - ins);
				break;
			case I_JMP:
				sophon_pr("jmp    %d\n", pi->i.jmp - ins);
				break;
			case I_CLEAR:
				sophon_pr("clear  %d\n", pi->i.cap);
				break;
			case I_EMPTY:
				sophon_pr("empty  %d %d\n", pi->i.empty.cap,
							pi->i.empty.jmp - ins);
				break;
			case I_END:
				sophon_pr("end\n");
				break;
			default:
				SOPHON_ASSERT(0);
		}
		pi++;
	}
}

#endif /*SOPHON_DEBUG*/

static ReData*
gen_re_data (Sophon_VM *vm, ReInput *inp)
{
	Sophon_U32 isize = 1;
	ReData *rd;
	ReIns *pi;

	isize += node_calc(inp, inp->root);
	DEBUG(("ins buffer size %d", isize));

	if (inp->loop_depth + inp->cap_end + 1 >= 256)
		sophon_fatal("too many captures");

	rd = (ReData*)sophon_mm_alloc_ensure(vm, sizeof(ReData));

	rd->ibuf_size  = isize;
	rd->cr_size    = inp->cr_len;
	rd->cap_count  = inp->cap_end + 1;
	rd->loop_count = inp->loop_depth;

	if (rd->ibuf_size)
		rd->ibuf = (ReIns*)sophon_mm_alloc_ensure(vm,
					sizeof(ReIns) * rd->ibuf_size);
	else
		rd->ibuf = NULL;

	if (rd->cr_size)
		rd->cr_buf = (Sophon_CharRange*)sophon_mm_alloc_ensure(vm,
					sizeof(Sophon_CharRange) * rd->cr_size);
	else
		rd->cr_buf = NULL;

	sophon_memcpy(rd->cr_buf, inp->cr_buf,
				sizeof(Sophon_CharRange) * rd->cr_size);

	pi = node_gen_ins(inp, inp->root, rd->ibuf);

	pi->type = I_END;
	pi++;

	SOPHON_ASSERT(pi == (rd->ibuf + rd->ibuf_size));

#ifdef SOPHON_DEBUG
	dump_ins(rd->ibuf, rd->ibuf_size, rd->cr_buf);
#endif
	return rd;
}

Sophon_RegExp*
sophon_regexp_create (Sophon_VM *vm, Sophon_String *str,
		Sophon_U8 flags)
{
	Sophon_RegExp *re;
	ReData *rd = NULL;
	ReNode *n, *nn;
	ReInput inp;
	Sophon_Result r;

	SOPHON_ASSERT(vm && str);

	inp.chars = sophon_string_chars(vm, str);
	inp.len   = sophon_string_length(vm, str);
	inp.pos   = 0;
	inp.root  = NULL;
	inp.nodes = NULL;
	inp.cr_buf = NULL;
	inp.cr_len = 0;
	inp.cr_cap = 0;
	inp.cap_begin  = 0;
	inp.cap_end    = 0;
	inp.loop_level = 0;
	inp.loop_depth = 0;

	/*Parse regular expression source*/
	DEBUG(("parse"));
	r = parse(vm, &inp);
	if (r == SOPHON_OK) {
		DEBUG(("gen ins"));
		if (!(rd = gen_re_data(vm, &inp)))
			r = SOPHON_ERR_PARSE;
	}

	/*Free input*/
	for (n = inp.nodes; n; n = nn) {
		nn = n->alloc_next;
		sophon_mm_free(vm, n, sizeof(ReNode));
	}

	if (inp.cr_buf) {
		sophon_mm_free(vm, inp.cr_buf,
					sizeof(Sophon_CharRange) * inp.cr_cap);
	}

	if (r != SOPHON_OK) {
		sophon_throw(vm, vm->SyntaxError,
				"Regular expression parse error");
		return NULL;
	}

	SOPHON_PRIM_OBJ_ALLOC(vm, re, RegExp);

	re->str   = str;
	re->flags = flags;
	re->last  = 0;
	re->re    = (Sophon_Ptr)rd;

	sophon_gc_add(vm, (Sophon_GCObject*)re);

	return re;
}

void
sophon_regexp_destroy (Sophon_VM *vm, Sophon_RegExp *re)
{
	ReData *rd;

	SOPHON_ASSERT(vm && re);

	rd = (ReData*)re->re;

	if (rd) {
		if (rd->cr_buf)
			sophon_mm_free(vm, rd->cr_buf,
					sizeof(Sophon_CharRange) * rd->cr_size);

		if (rd->ibuf)
			sophon_mm_free(vm, rd->ibuf,
					sizeof(ReIns) * rd->ibuf_size);

		sophon_mm_free(vm, rd, sizeof(ReData));
	}

	SOPHON_PRIM_OBJ_FREE(vm, re, RegExp);
}

Sophon_Int
sophon_regexp_match_size (Sophon_RegExp *re)
{
	ReData *rd;

	SOPHON_ASSERT(re);

	rd = (ReData*)re->re;

	return rd->cap_count;
}

static Sophon_Bool
class_check (Sophon_CharRange *buf, Sophon_U32 begin,
			Sophon_U32 end, Sophon_Char chr)
{
	Sophon_CharRange *cr, *cr_end;
	Sophon_Char min, max;
	
	cr = buf + begin;
	cr_end = buf + end;

	while (cr < cr_end) {
		SOPHON_EXPAND_CHAR_RANGE(*cr, min, max);

		if ((min <= chr) && (max >= chr))
			return SOPHON_TRUE;

		cr++;
	}

	return SOPHON_FALSE;
}

static Sophon_Bool
class_case_check (Sophon_CharRange *buf, Sophon_U32 begin,
			Sophon_U32 end, Sophon_Char chr)
{
	Sophon_CharRange *cr, *cr_end;
	Sophon_Char min, max;
	Sophon_Char c1, c2;

	c1 = chr;
	c2 = sophon_tolower(c1);
	
	cr = buf + begin;
	cr_end = buf + end;

	while (cr < cr_end) {
		SOPHON_EXPAND_CHAR_RANGE(*cr, min, max);

		if ((min <= c1) && (max >= c1))
			return SOPHON_TRUE;
		if ((min <= c2) && (max >= c2))
			return SOPHON_TRUE;

		cr++;
	}

	return SOPHON_FALSE;
}

#define RE_BRANCH_SIZE(ctxt)\
	(((sizeof(ReBranch) + ((ctxt)->cap_count + (ctxt)->loop_count - 1)\
		* sizeof(Sophon_ReMatch)) + sizeof(Sophon_Ptr) - 1) &\
		~(sizeof(Sophon_Ptr) - 1))
#define RE_GET_BRANCH(ctxt, n)\
	((ReBranch*)(((Sophon_U8*)(ctxt)->branch) + RE_BRANCH_SIZE(ctxt) * (n)))

static Sophon_Result
push_branch (Sophon_VM *vm, ReCtxt *ctxt, ReIns *ins, Sophon_Char *ch,
			Sophon_ReMatch *match)
{
	ReBranch *br;

	if (ctxt->br_count >= ctxt->br_cap) {
		Sophon_U32 cap = SOPHON_MAX(ctxt->br_cap * 2, 32);
		ReBranch *buf;

		buf = (ReBranch*)sophon_mm_realloc(vm, ctxt->branch,
					RE_BRANCH_SIZE(ctxt) * ctxt->br_cap,
					RE_BRANCH_SIZE(ctxt) * cap);
		if (!buf)
			return SOPHON_ERR_NOMEM;

		ctxt->branch = buf;
		ctxt->br_cap = cap;
	}


	br = RE_GET_BRANCH(ctxt, ctxt->br_count);

	br->ins = ins;
	br->ch  = ch;

	if (match) {
		sophon_memcpy(br->match, match,
					ctxt->cap_count * sizeof(Sophon_ReMatch));
	} else {
		sophon_memset(br->match, 0,
					ctxt->cap_count * sizeof(Sophon_ReMatch));
	}

	ctxt->br_count++;

	return SOPHON_OK;
}

static Sophon_Result
match (Sophon_VM *vm, ReCtxt *ctxt, Sophon_ReMatch *rbuf, Sophon_U32 rsize)
{
	ReBranch *br;
	ReIns *pi;
	Sophon_Char *ch;
	Sophon_Int left;
	Sophon_U32 br_count = ctxt->br_count;
	Sophon_U32 i;
	Sophon_ReMatch mbuf[ctxt->cap_count + ctxt->loop_count];
	Sophon_Result r;

retry:
	br = RE_GET_BRANCH(ctxt, ctxt->br_count - 1);
	pi = br->ins;
	ch = br->ch;
	sophon_memcpy(mbuf, br->match, sizeof(mbuf));
	left = ctxt->len - (ch - ctxt->chars);
	ctxt->br_count--;

	while (1) {
		switch (pi->type) {
			case I_LA:
				if ((r = push_branch(vm, ctxt, pi + 1, ch, mbuf))
							!= SOPHON_OK)
					return r;
				r = match(vm, ctxt, mbuf, ctxt->cap_count);
				if (r < 0)
					return r;
				if (r == SOPHON_NONE)
					goto mismatch;

				pi = pi->i.jmp;
				continue;
			case I_NLA:
				if ((r = push_branch(vm, ctxt, pi + 1, ch, mbuf))
							!= SOPHON_OK)
					return r;
				r = match(vm, ctxt, mbuf, ctxt->cap_count);
				if (r < 0)
					return r;
				if (r == SOPHON_OK)
					goto mismatch;

				pi = pi->i.jmp;
				continue;
			case I_ANY:
				if (left <= 0)
					goto mismatch;
				ch++;
				left--;
				break;
			case I_CHAR:
				if (left <= 0)
					goto mismatch;
				if (ctxt->flags & SOPHON_REGEXP_FL_I) {
					if (sophon_tolower(*ch) != sophon_tolower(pi->i.ch))
						goto mismatch;
				} else {
					if (*ch != pi->i.ch)
						goto mismatch;
				}
				ch++;
				left--;
				break;
			case I_CLASS:
				if (left <= 0)
					goto mismatch;
				if (ctxt->flags & SOPHON_REGEXP_FL_I) {
					if (!class_case_check(ctxt->cr_buf, pi->i.clazz.begin,
									pi->i.clazz.end, *ch))
						goto mismatch;
				} else {
					if (!class_check(ctxt->cr_buf, pi->i.clazz.begin,
									pi->i.clazz.end, *ch))
						goto mismatch;
				}
				ch++;
				left--;
				break;
			case I_NCLASS:
				if (left <= 0)
					goto mismatch;
				if (ctxt->flags & SOPHON_REGEXP_FL_I) {
					if (class_case_check(ctxt->cr_buf, pi->i.clazz.begin,
									pi->i.clazz.end, *ch))
						goto mismatch;
				} else {
					if (class_check(ctxt->cr_buf, pi->i.clazz.begin,
									pi->i.clazz.end, *ch))
						goto mismatch;
				}
				ch++;
				left--;
				break;
			case I_BOL:
				if (left != ctxt->len) {
					if (ctxt->flags & SOPHON_REGEXP_FL_M) {
						if (ch[-1] != 10)
							goto mismatch;
					} else {
						goto mismatch;
					}
				}
				break;
			case I_EOL:
				if (left != 0) {
					if (ctxt->flags & SOPHON_REGEXP_FL_M) {
						if (*ch != 10)
							goto mismatch;
					} else {
						goto mismatch;
					}
				}
				break;
			case I_B: {
				Sophon_Bool b;

				b = (left == ctxt->len) || !sophon_isword(ch[-1]);
				b ^= !left || !sophon_isword(*ch);
				if (!b)
					goto mismatch;
				break;
			}
			case I_NB: {
				Sophon_Bool b;

				b = (left == ctxt->len) || !sophon_isword(ch[-1]);
				b ^= !left || !sophon_isword(*ch);
				if (b)
					goto mismatch;
				break;
			}
			case I_REF: {
				Sophon_Char *ref;
				Sophon_U32 len;

				ref = mbuf[pi->i.ref].begin;
				len = ref ? mbuf[pi->i.ref].end - ref : 0;

				if (len) {
					if (ctxt->flags & SOPHON_REGEXP_FL_I) {
						if (sophon_ucstrncasecmp(ref, ch, len))
							goto mismatch;
					} else {
						if (sophon_ucstrncmp(ref, ch, len))
							goto mismatch;
					}
				}

				ch += len;
				break;
			}
			case I_CAPB:
				mbuf[pi->i.cap].begin = ch;
				break;
			case I_CAPE:
				mbuf[pi->i.cap].end = ch;
				break;
			case I_BRANCH:
				if ((r = push_branch(vm, ctxt, pi->i.branch.b2, ch, mbuf))
							!= SOPHON_OK)
					return r;
				pi = pi->i.branch.b1;
				continue;
			case I_JMP:
				pi = pi->i.jmp;
				continue;
			case I_CLEAR:
				for (i = pi->i.cap; i < ctxt->cap_count; i++) {
					mbuf[i].begin = NULL;
					mbuf[i].end = NULL;
				}
				break;
			case I_EMPTY:
				if (mbuf[pi->i.empty.cap].begin == ch) {
					pi = pi->i.empty.jmp;
					continue;
				}
				break;
			case I_END:
				mbuf[0].begin = ctxt->chars + ctxt->start;
				mbuf[0].end   = ch;
				rsize = SOPHON_MIN(rsize, ctxt->cap_count);
				if (rsize) {
					sophon_memcpy(rbuf, mbuf, rsize * sizeof(Sophon_ReMatch));
				}
				ctxt->br_count = br_count - 1;
				return SOPHON_OK;
		}

		pi++;
	}

mismatch:
	if (br_count <= ctxt->br_count) {
		goto retry;
	}

	return SOPHON_NONE;
}

Sophon_Result
sophon_regexp_match (Sophon_VM *vm, Sophon_RegExp *re, Sophon_String *str,
			Sophon_U32 start, Sophon_ReMatch *mbuf,	Sophon_U32 mbuf_size)
{
	ReData *rd;
	ReCtxt ctxt;
	Sophon_Int r = 0;

	SOPHON_ASSERT(vm && re && str);

	rd = (ReData*)re->re;

	/*Initialize context*/
	ctxt.chars = sophon_string_chars(vm, str);
	ctxt.len   = sophon_string_length(vm, str);
	ctxt.start  = start;
	ctxt.flags  = re->flags;
	ctxt.cr_buf = rd->cr_buf;
	ctxt.cap_count  = rd->cap_count;
	ctxt.loop_count = rd->loop_count;

	ctxt.br_cap   = 0;
	ctxt.br_count = 0;
	ctxt.branch   = NULL;

	/*Match process*/
	while (ctxt.start <= ctxt.len) {
		ctxt.br_count = 0;

		if ((r = push_branch(vm, &ctxt, rd->ibuf,
							ctxt.chars + ctxt.start, NULL)) != SOPHON_OK)
			break;

		if ((r = match(vm, &ctxt, mbuf, mbuf_size)) != 0)
			break;

		ctxt.start++;
	}

	/*Free branches data*/
	if (ctxt.branch) {
		sophon_mm_free(vm, ctxt.branch,
					RE_BRANCH_SIZE(&ctxt) * ctxt.br_cap);
	}

	return r;
}

