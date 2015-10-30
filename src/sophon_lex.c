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
#include <sophon_conv.h>
#include <sophon_string.h>
#include <sophon_debug.h>

#include "sophon_parser_internal.h"

#include "sophon_js_parser.h"
#include "sophon_js_lex.c"

#if 0
#define DEBUG(a) SOPHON_INFO(a)
#else
#define DEBUG(a)
#endif

#ifndef SOPHON_LEX_INPUT_BUF_SIZE
	#define SOPHON_LEX_INPUT_BUF_SIZE 4096
#endif

#define LEX_CBUF_PADDING 16

/*Output error message*/
static void
lex_error (Sophon_VM *vm, Sophon_Int type, const char *fmt, ...)
{
	Sophon_LexData *l = (Sophon_LexData*)vm->lex_data;
	Sophon_Location loc;
	va_list ap;

	loc.first_line = l->line;
	loc.last_line  = l->line;
	loc.first_column = l->column;
	loc.last_column  = l->column;

	va_start(ap, fmt);
	sophon_parser_errorv(vm, type, &loc, fmt, ap);
	va_end(ap);
}

/*Append a character into the text buffer*/
static Sophon_Int
lex_tbuf_append (Sophon_VM *vm, Sophon_Char ch)
{
	Sophon_LexData *l = (Sophon_LexData*)vm->lex_data;

	if (l->tlen >= l->tbuf_size) {
		Sophon_Char *buf;
		Sophon_U32 size = SOPHON_MAX(64, l->tbuf_size * 2);

		buf = (Sophon_Char*)sophon_mm_realloc_ensure(vm, l->tbuf,
					sizeof(Sophon_Char) * l->tbuf_size,
					sizeof(Sophon_Char) * size);

		l->tbuf = buf;
		l->tbuf_size = size;
	}

	l->tbuf[l->tlen++] = ch;

	return ch;
}

/*Convert input data to UTF16*/
static Sophon_Result
lex_conv (Sophon_VM *vm)
{
	Sophon_LexData *l = (Sophon_LexData*)vm->lex_data;
	Sophon_Result r;
	Sophon_U32 in_size = l->ilen;
	Sophon_U8 *out = (Sophon_U8*)(l->cbuf + LEX_CBUF_PADDING);
	Sophon_U32 out_size = (SOPHON_LEX_INPUT_BUF_SIZE - LEX_CBUF_PADDING) *
				sizeof(Sophon_Char);

	/*Convert the characters*/
	r = sophon_conv(l->conv, l->ibuf + l->ibegin, &in_size,
				out, &out_size);
	if (r < 0)
		return r;

	l->cbegin = LEX_CBUF_PADDING;
	l->clen   = out_size / sizeof(Sophon_Char);

	if (out_size == 0) {
		/*Left some untermiated code, move it to the head of the input buffer
		 * and try to read mote data*/
		Sophon_U32 pos  = l->ibegin + in_size;
		Sophon_U32 left = l->ilen - in_size;

		if (left && pos)
			memmove(l->ibuf, l->ibuf + pos, left);

		l->ibegin = 0;
	} else {
		l->ibegin += in_size;
	}

	l->ilen -= in_size;

	return out_size;
}

/*Get a character from the input*/
static Sophon_Int
lex_getc (Sophon_VM *vm)
{
	Sophon_LexData *l = (Sophon_LexData*)vm->lex_data;
	Sophon_Int ch;
	Sophon_Result r;

	if (l->clen == 0) {
		Sophon_U8 *buf;
		Sophon_Int size, cnt;

		if (l->flags & SOPHON_LEX_FL_CONV) {
			if (l->ilen) {
				r = lex_conv(vm);
				if (r < 0)
					return r;

				if (r > 0)
					goto char_ok;
			} else {
				l->ibegin = 0;
			}

			buf  = l->ibuf + l->ilen;
			size = SOPHON_LEX_INPUT_BUF_SIZE - l->ilen;

			cnt = l->inp_func(l->inp_data, buf, size);

			if (cnt < 0)
				return cnt;
			else if (cnt == 0)
				return l->ilen ? SOPHON_ERR_ILLCODE : SOPHON_ERR_EOF;

			l->ilen += cnt;

			r = lex_conv(vm);
			if (r < 0)
				return r;

			if (r == 0)
				return SOPHON_ERR_ILLCODE;
		} else {
			buf  = (Sophon_U8*)(l->cbuf + LEX_CBUF_PADDING);
			size = (SOPHON_LEX_INPUT_BUF_SIZE - LEX_CBUF_PADDING) *
						sizeof(Sophon_Char);

			cnt = l->inp_func(l->inp_data, buf, size);

			if (cnt < 0)
				return cnt;
			else if (cnt == 0)
				return SOPHON_ERR_EOF;

			l->cbegin = LEX_CBUF_PADDING;
			l->clen   = cnt / sizeof(Sophon_Char);
		}
	}

char_ok:
	ch = l->cbuf[l->cbegin++];
	l->clen--;

	if (l->flags & SOPHON_LEX_FL_LAST_NL) {
		l->flags &= ~SOPHON_LEX_FL_LAST_NL;
		l->line++;
		l->column = 0;
	}

	if (ch == '\n')
		l->flags |= SOPHON_LEX_FL_LAST_NL;

	l->column++;

	ch = lex_tbuf_append(vm, ch);

	return ch;
}

/*Unget a character*/
static void
lex_unget (Sophon_VM *vm, Sophon_Int ch)
{
	Sophon_LexData *l = (Sophon_LexData*)vm->lex_data;

	SOPHON_ASSERT(l->tlen > 0);

	if (ch < 0)
		return;

	if (ch == '\n')
		l->flags &= ~SOPHON_LEX_FL_LAST_NL;
	
	l->column--;

	l->tlen--;

	SOPHON_ASSERT(l->cbegin > 0);
	l->cbuf[--l->cbegin] = ch;
	l->clen++;
}

/*Unget characters*/
static void
lex_unget_n (Sophon_VM *vm, Sophon_Int n)
{
	Sophon_LexData *l = (Sophon_LexData*)vm->lex_data;

	SOPHON_ASSERT(l->tlen >= n);

	while (n--)
		lex_unget(vm, l->tbuf[l->tlen - 1]);
}

/*Get the text string*/
static Sophon_Char*
lex_text (Sophon_VM *vm)
{
	Sophon_LexData *l = (Sophon_LexData*)vm->lex_data;

	if (l->tlen && l->tbuf[l->tlen - 1]) {
		lex_tbuf_append(vm, 0);
	}

	return l->tbuf;
}

/*Convert character to number*/
static Sophon_Int
lex_char_to_hex (Sophon_VM *vm, Sophon_Char ch)
{
	if (sophon_isdigit(ch))
		return ch - '0';
	else if (((ch) >= 'a') && ((ch) <= 'f'))
		return ch - 'a' + 10;
	else if (((ch) >= 'A') && ((ch) <= 'F'))
		return ch - 'A' + 10;

	lex_error(vm, SOPHON_PARSER_ERROR,
				"expect a hexadecimal number character");
	return SOPHON_ERR_LEX;
}

/*Get an escape unicode value*/
static Sophon_Int
lex_get_escape_uc (Sophon_VM *vm)
{
	Sophon_Int c, bytes;
	Sophon_Int code, v;
	Sophon_Bool br, br_end;

	if ((c = lex_getc(vm)) < 0)
		return c;

	code  = 0;
	bytes = 4;

	if (c == '{') {
		br = SOPHON_TRUE;
		br_end = SOPHON_FALSE;
	} else {
		br = SOPHON_FALSE;
		if ((v = lex_char_to_hex(vm, c)) < 0) {
			return v;
		}

		code = (code << 4) | v;
		bytes--;
	}

	while (bytes--) {
		if ((c = lex_getc(vm)) < 0)
			return c;

		if (br && (c == '}')) {
			br_end = SOPHON_TRUE;
			break;
		}

		if ((v = lex_char_to_hex(vm, c)) < 0)
			return v;

		code = (code << 4) | v;
	}

	if (br && !br_end) {
		if ((c = lex_getc(vm)) < 0)
			return c;

		if (c != '}') {
			lex_error(vm, SOPHON_PARSER_ERROR,
						"unterminated unicode escape sequence");
			return SOPHON_ERR_LEX;
		}
	}

	return code;
}

/*Get a string*/
static Sophon_Result
lex_get_str (Sophon_VM *vm, Sophon_TokenValue *val)
{
	Sophon_LexData *l = (Sophon_LexData*)vm->lex_data;
	Sophon_Char end = l->tbuf[0];
	Sophon_Char code;
	Sophon_String *str;
	Sophon_Int c, v;

	l->tlen--;

	while (1) {
		c = lex_getc(vm);
		if (c < 0)
			return c;

		if (c == '\\') {
			Sophon_U32 len = l->tlen;

			c = lex_getc(vm);
			if (c < 0)
				return c;

			switch(c) {
				case '0':
					l->tlen = len;
					l->tbuf[len - 1] = 0;
					break;
				case 'b':
					l->tlen = len;
					l->tbuf[len - 1] = '\b';
					break;
				case 'f':
					l->tlen = len;
					l->tbuf[len - 1] = '\f';
					break;
				case 'n':
					l->tlen = len;
					l->tbuf[len - 1] = '\n';
					break;
				case 'r':
					l->tlen = len;
					l->tbuf[len - 1] = '\r';
					break;
				case 't':
					l->tlen = len;
					l->tbuf[len - 1] = '\t';
					break;
				case 'v':
					l->tlen = len;
					l->tbuf[len - 1] = '\v';
					break;
				case 'x':
					if ((c = lex_getc(vm)) < 0)
						return c;
					if ((v = lex_char_to_hex(vm, c)) < 0)
						return v;
					code = v;
					if ((c = lex_getc(vm)) < 0)
						return c;
					if ((v = lex_char_to_hex(vm, c)) < 0)
						return v;
					code = (code << 4) | v;

					l->tlen = len;
					l->tbuf[len - 1] = code;
					break;
				case 'u':
					if ((code = lex_get_escape_uc(vm)) < 0)
						return code;

#ifdef SOPHON_8BITS_CHAR
					if (code > 0xFF) {
						lex_error(vm, SOPHON_PARSER_ERROR,
									"illegal unicode value");
						return SOPHON_ERR_LEX;
					}
#endif
					l->tlen = len;
					l->tbuf[len - 1] = code;
					break;
				case '\r':
					if ((c = lex_getc(vm)) < 0)
						return c;
					if (c != '\n')
						lex_unget(vm, c);
				case '\n':
#ifndef SOPHON_8BITS_CHAR
				case 0x2028:
				case 0x2029:
#endif
					l->tlen = len - 1;
					break;
				default:
					l->tlen = len;
					l->tbuf[len - 1] = c;
					break;
			}
		} else if (c == end) {
			l->tlen--;
			break;
		} else if ((c == '\n') || (c == '\r')
#ifndef SOPHON_8BITS_CHAR
					|| (c == 0x2028) || (c == 0x2029)
#endif
				  ){
			lex_error(vm, SOPHON_PARSER_ERROR,
						"unterminated string");
			return SOPHON_ERR_LEX;
		}
	}

	str = sophon_string_from_chars(vm, l->tbuf, l->tlen);
	if (!str)
		return SOPHON_ERR_NOMEM;

	sophon_value_set_gc(vm, &val->v, str);

#ifdef SOPHON_DEBUG
	{
		Sophon_U32 len;
		char *cstr;

		if (sophon_string_new_utf8_cstr(vm, str, &cstr, &len) >= 0) {
			DEBUG(("string: \"%s\"", cstr));
			sophon_string_free_utf8_cstr(vm, cstr, len);
		}
	}
#endif

	return SOPHON_OK;
}

#ifndef SOPHON_8BITS_CHAR
#include "uc_id_start_table.c"
#include "uc_id_cont_table.c"
#endif

static Sophon_Bool
lex_is_id_start (Sophon_Int ch)
{
	switch (ch) {
		case '_':
		case '$':
			return SOPHON_TRUE;
		default:
#ifndef SOPHON_8BITS_CHAR
			return sophon_char_table_search(ch, uc_id_start_table,
						SOPHON_ARRAY_SIZE(uc_id_start_table));
#else
			return sophon_isalpha(ch);
#endif
	}

	return SOPHON_FALSE;
}

static Sophon_Bool
lex_is_id_cont (Sophon_Int ch)
{
	switch (ch) {
		case '_':
		case '$':
#ifndef SOPHON_8BITS_CHAR
		case 0x200C:
		case 0x200D:
#endif
			return SOPHON_TRUE;
		default:
			if (sophon_isdigit(ch))
				return SOPHON_TRUE;
#ifndef SOPHON_8BITS_CHAR
			return sophon_char_table_search(ch, uc_id_start_table,
						SOPHON_ARRAY_SIZE(uc_id_start_table));
#else
			return sophon_isalpha(ch);
#endif
	}

	return SOPHON_FALSE;
}

/*Get identifier start character*/
static Sophon_Bool
lex_get_id_start (Sophon_VM *vm, Sophon_Int ch)
{
	Sophon_LexData *l = (Sophon_LexData*)vm->lex_data;
	Sophon_Bool r;

	if (ch == '\\') {
		ch = lex_getc(vm);
		if (ch == 'u') {
			Sophon_U32 len = l->tlen - 1;
			Sophon_Int uc;

			if ((uc = lex_get_escape_uc(vm)) < 0) {
				lex_unget_n(vm, l->tlen - len);
				r = SOPHON_FALSE;
			} else {
				r = lex_is_id_start(uc);
				if (r) {
					l->tbuf[len - 1] = uc;
					l->tlen = len;
				}
			}
		} else {
			lex_unget(vm, ch);
			r = SOPHON_FALSE;
		}
	} else {
		r = lex_is_id_start(ch);
	}

	return r;
}

/*Get identifier continue character*/
static Sophon_Bool
lex_get_id_cont (Sophon_VM *vm, Sophon_Int ch)
{
	Sophon_LexData *l = (Sophon_LexData*)vm->lex_data;
	Sophon_Bool r;

	if (ch == '\\') {
		ch = lex_getc(vm);
		if (ch == 'u') {
			Sophon_U32 len = l->tlen - 1;
			Sophon_Int uc;

			if ((uc = lex_get_escape_uc(vm)) < 0) {
				lex_unget_n(vm, l->tlen - len);
				r = SOPHON_FALSE;
			} else {
				r = lex_is_id_cont(uc);
				if (r) {
					l->tbuf[len - 1] = uc;
					l->tlen = len;
				}
			}
		} else {
			lex_unget(vm, ch);
			r = SOPHON_FALSE;
		}
	} else {
		r = lex_is_id_cont(ch);
	}

	return r;
}

/*Get an identifier*/
static Sophon_Token
lex_get_id (Sophon_VM *vm, Sophon_TokenValue *val, Sophon_Bool keyword)
{
	Sophon_LexData *l = (Sophon_LexData*)vm->lex_data;
	Sophon_Int ch = l->tbuf[l->tlen - 1];

	if (keyword) {
		ch = lex_getc(vm);
		if (!lex_get_id_cont(vm, ch)) {
			lex_unget(vm, ch);
			return 0;
		}
	}

	if ((l->tlen > 1) || lex_get_id_start(vm, ch)) {
		Sophon_String *str;

		while (1) {
			ch = lex_getc(vm);
			if ((ch < 0) && (ch != SOPHON_ERR_EOF))
				return ch;

			if (ch == SOPHON_ERR_EOF)
				break;

			if (!lex_get_id_cont(vm, ch)) {
				lex_unget(vm, ch);
				break;
			}
		}

		if (!(str = sophon_string_from_chars(vm, l->tbuf, l->tlen)))
			return SOPHON_ERR_NOMEM;

		if (!(str = sophon_string_intern(vm, str)))
			return SOPHON_ERR_NOMEM;

		sophon_value_set_gc(vm, &val->v, str);

#ifdef SOPHON_DEBUG
		{
			char *cstr;
			Sophon_U32 len;

			if (sophon_string_new_utf8_cstr(vm, str, &cstr, &len) >= 0) {
				DEBUG(("id: \"%s\"", cstr));
				sophon_string_free_utf8_cstr(vm, cstr, len);
			}
		}
#endif
		return T_IDENTIFIER;
	} else {
		DEBUG(("character: \'%c\'", ch));
		return ch;
	}
}

static Sophon_Result
lex_get_num (Sophon_VM *vm, const Sophon_Char *text, Sophon_Int base,
			Sophon_Value *v)
{
	Sophon_Result r;
	Sophon_Number d;

	r = sophon_strtod(text, NULL, base, &d);
	if (r == SOPHON_ERR_2BIG) {
		d = SOPHON_INFINITY;
	} else if (r < 0) {
		lex_error(vm, SOPHON_PARSER_ERROR,
					"illegal character in number");
		return r;
	}

	DEBUG(("double: %e", d));

	sophon_value_set_number(vm, v, d);
	return SOPHON_OK;
}

/*Solve the action*/
static Sophon_Token
lex_action (Sophon_VM *vm, Sophon_U16 action, Sophon_TokenValue *val)
{
	Sophon_LexData *l = (Sophon_LexData*)vm->lex_data;
	Sophon_Char *text;
	Sophon_Result r;
	Sophon_Token t;

	switch ((LexActionType)action) {
		case A_COMMENT_LINE_TERM:
			l->flags |= SOPHON_LEX_FL_COMMENT_LT;
			break;
		case A_COMMENT_BLOCK_END:
			if (!(l->flags & SOPHON_LEX_FL_COMMENT_LT)) {
				l->cond = C_BEGIN;
				break;
			}
		case A_COMMENT_LINE_END:
			l->cond = C_BEGIN;
		case A_LINE_TERM:
			l->flags |= SOPHON_LEX_FL_LAST_LT;
			DEBUG(("line terminator"));
			break;
		case A_OCT_NUM:
			text = lex_text(vm);
			if (!text)
				return SOPHON_ERR_NOMEM;

			r = lex_get_num(vm, text, 8, &val->v);
			if (r < 0)
				return r;

			return T_NUMBER;
		case A_HEX_NUM:
			text = lex_text(vm);
			if (!text)
				return SOPHON_ERR_NOMEM;

			r = lex_get_num(vm, text + 2, 16, &val->v);
			if (r < 0)
				return r;

			return T_NUMBER;
		case A_DEC_NUM:
			text = lex_text(vm);
			if (!text)
				return SOPHON_ERR_NOMEM;

			r = lex_get_num(vm, text, 10, &val->v);
			if (r < 0)
				return r;

			return T_NUMBER;
		case A_DOUBLE:
			text = lex_text(vm);
			if (!text)
				return SOPHON_ERR_NOMEM;

			r = lex_get_num(vm, text, SOPHON_BASE_FLOAT, &val->v);
			if (r < 0)
				return r;

			return T_NUMBER;
		case A_COMMENT_LINE_BEGIN:
			l->cond = C_COMMENT_LINE;
			DEBUG(("comment line"));
			break;
		case A_COMMENT_BLOCK_BEGIN:
			l->cond = C_COMMENT_BLOCK;
			DEBUG(("comment block"));
			break;
		case A_TO_BEGIN:
			l->cond = C_BEGIN;
			DEBUG(("comment end"));
			break;
		case A_STRING:
			r = lex_get_str(vm, val);
			if (r < 0)
				return r;
			return T_STRING;
		case A_CHAR:
			return lex_get_id(vm, val, SOPHON_FALSE);
		case A_NONE:
			break;
		default:
			SOPHON_ASSERT((action > A_NONE) && (action < A_COUNT));

			/*Check if the token is an indentifier*/
			if ((action > A_NONE) && (action < A_KEYWORD_END)) {
				t = lex_get_id(vm, val, SOPHON_TRUE);
				if (t != 0)
					return t;
			}

			/*Real keyword*/
			t = T_BEGIN + action - A_NONE;
			DEBUG(("keyword: %d", t));
			return t;
	}

	return 0;
}

static Sophon_Int
lex_re_getc (Sophon_VM *vm)
{
	Sophon_Int c = lex_getc(vm);
	
	if ((c == '\n') || (c == '\r')
#ifndef SOPHON_8BITS_CHAR
				|| (c == 0x2028) || (c == 0x2029)
#endif
				) {
		lex_error(vm, SOPHON_PARSER_ERROR,
					"unterminated regular expression");
		return SOPHON_ERR_LEX;
	}

	return c;
}

static Sophon_Token
lex_get_re (Sophon_VM *vm, Sophon_TokenValue *val)
{
	Sophon_LexData *l = (Sophon_LexData*)vm->lex_data;
	Sophon_Int c;
	Sophon_String *str;

	/*Get body*/
	if ((c = lex_re_getc(vm)) < 0)
		return c;

	if (l->tlen == 1) {
		if (c == '*') {
			lex_error(vm, SOPHON_PARSER_ERROR,
						"illegal character in the regular expression");
			return SOPHON_ERR_LEX;
		}
	}

	while (1) {
		if (c == '[') {
			while (1) {
				if ((c = lex_re_getc(vm)) < 0)
					return c;

				if (c == ']') {
					break;
				} else if (c == '\\') {
					if ((c = lex_re_getc(vm)) < 0)
						return c;
				}
			}
		} else if (c == '\\') {
			if ((c = lex_re_getc(vm)) < 0)
				return c;
		} else if (c == '/') {
			/*Store body value*/
			str = sophon_string_from_chars(vm, l->tbuf, l->tlen - 1);
			if (!str)
				return SOPHON_ERR_NOMEM;

			sophon_value_set_gc(vm, &val->re.bodyv, str);
			l->tlen = 0;
			break;
		}

		if ((c = lex_re_getc(vm)) < 0)
			return c;
	}

	/*Get flags*/
	c = lex_getc(vm);
	if ((c < 0) && (c != SOPHON_ERR_EOF))
		return c;

	if ((c != SOPHON_ERR_EOF) && lex_get_id_start(vm, c)) {
		while (1) {
			c = lex_getc(vm);
			if ((c < 0) && (c != SOPHON_ERR_EOF))
				return c;

			if (!lex_get_id_cont(vm, c)) {
				lex_unget(vm, c);
				break;
			}
		}

		/*Store flags value*/
		str = sophon_string_from_chars(vm, l->tbuf, l->tlen);
		if (!str)
			return SOPHON_ERR_NOMEM;

		sophon_value_set_gc(vm, &val->re.flagsv, str);
	} else {
		if (c >= 0)
			lex_unget(vm, c);

		sophon_value_set_undefined(vm, &val->re.flagsv);
	}

#ifdef SOPHON_DEBUG
	{
		Sophon_String *str;
		char *cstr;
		Sophon_U32 len;

		str = (Sophon_String*)SOPHON_VALUE_GET_GC(val->re.bodyv);
		if (sophon_string_new_utf8_cstr(vm, str, &cstr, &len) >= 0) {
			DEBUG(("regex: \"%s\"", str));
			sophon_string_free_utf8_cstr(vm, cstr, len);
		}

		if (SOPHON_VALUE_IS_GC(val->re.flagsv)) {
			str = (Sophon_String*)SOPHON_VALUE_GET_GC(val->re.flagsv);
			if (sophon_string_new_utf8_cstr(vm, str, &cstr, &len) >= 0) {
				DEBUG(("flags: \"%s\"", str));
				sophon_string_free_utf8_cstr(vm, cstr, len);
			}
		}
	}
#endif

	return T_REGEXP;
}

void
sophon_lex_init (Sophon_VM *vm, Sophon_Encoding enc, Sophon_IOFunc input,
			Sophon_Ptr data)
{
	Sophon_LexData *l;
	Sophon_Encoding lenc;

	SOPHON_ASSERT(vm && input);

	l = (Sophon_LexData*)sophon_mm_alloc_ensure(vm, sizeof(Sophon_LexData));

	l->inp_func = input;
	l->inp_data = data;
	l->flags    = 0;

	/*Allocate character buffer*/
	l->cbuf = (Sophon_Char*)sophon_mm_alloc_ensure(vm,
				sizeof(Sophon_Char) * SOPHON_LEX_INPUT_BUF_SIZE);

	lenc = SOPHON_ENC;

	if (lenc != enc) {
		Sophon_Result r;

		/*Create character converter*/
		r = sophon_conv_create(&l->conv, enc, lenc);
		SOPHON_ASSERT(r == SOPHON_OK);
		r = r;

		l->flags |= SOPHON_LEX_FL_CONV;
		l->ibuf = (Sophon_U8*)sophon_mm_alloc_ensure(vm,
					sizeof(Sophon_U8) * SOPHON_LEX_INPUT_BUF_SIZE);
	} else {
		l->ibuf = NULL;
	}

	/*Initialize the status*/
	l->tlen  = 0;
	l->ilen  = 0;
	l->clen  = 0;
	l->cond  = C_BEGIN;
	l->ibegin = 0;
	l->cbegin = 0;
	l->line   = 0;
	l->column = 0;
	l->tbuf   = NULL;
	l->tbuf_size = 0;

	l->flags |= SOPHON_LEX_FL_LAST_NL;

	vm->lex_data = l;
}

void
sophon_lex_deinit (Sophon_VM *vm)
{
	Sophon_LexData *l;

	SOPHON_ASSERT(vm);

	l = (Sophon_LexData*)vm->lex_data;
	SOPHON_ASSERT(l);

	if (l->flags & SOPHON_LEX_FL_CONV) {
		sophon_conv_destroy(l->conv);
	}

	if (l->tbuf) {
		sophon_mm_free(vm, l->tbuf,
					sizeof(Sophon_Char) * l->tbuf_size);
	}

	if (l->ibuf) {
		sophon_mm_free(vm, l->ibuf,
					sizeof(Sophon_U8) * SOPHON_LEX_INPUT_BUF_SIZE);
	}

	if (l->cbuf) {
		sophon_mm_free(vm, l->cbuf,
					sizeof(Sophon_Char) * SOPHON_LEX_INPUT_BUF_SIZE);
	}

	sophon_mm_free(vm, l, sizeof(Sophon_LexData));
}

Sophon_Token
sophon_lex (Sophon_VM *vm, Sophon_TokenValue *val, Sophon_Location *loc)
{
	Sophon_LexData *l = (Sophon_LexData*)vm->lex_data;
	Sophon_Token t;
	const LexCond *cond;
	Sophon_Int sym;
	Sophon_U8 action;
	Sophon_Int fetch;
	Sophon_U16 state;
	Sophon_U16 edge;
	Sophon_Bool first;

	SOPHON_ASSERT(vm && val && loc);

	l->flags &= ~(SOPHON_LEX_FL_LAST_LT | SOPHON_LEX_FL_COMMENT_LT);

	/*Get regular expression*/
	if (l->cond == C_REGEXP) {
		t = lex_get_re(vm, val);

		l->cond = C_BEGIN;
		loc->last_line   = l->line;
		loc->last_column = l->column;
		return t;
	}

retry:
	/*Get the next token*/
	action = 0xFF;
	fetch  = 0;
	cond = &js_lex_conds[l->cond];
	l->tlen = 0;
	first = SOPHON_TRUE;
	state = 0;

	while (1) {
		Sophon_Bool jump = SOPHON_FALSE;
			
		sym = lex_getc(vm);

		if (first && (sym >= 0)) {
			loc->first_line   = l->line;
			loc->first_column = l->column;
			first = SOPHON_FALSE;
		}

		if (cond->state_action[state] != 0xFF) {
			action = cond->state_action[state];
			fetch  = 0;
		}

		if (sym >= 0) {
			fetch++;
		}

		edge = cond->state_edges[state];
		while (edge != 0xFFFF) {
			Sophon_U32 srange = cond->edge_symbol[edge];
			Sophon_U16 min, max;

			min = srange & 0xFFFF;
			max = (srange >> 16);
			
			if ((sym >= min) && (sym <= max)) {
				state = cond->edge_dest[edge];
				jump = SOPHON_TRUE;
				break;
			}

			edge = cond->edge_next[edge];
		}

		if (!jump) {
			if (action != 0xFF) {
				while (fetch-- > 0) {
					lex_unget(vm, l->tbuf[l->tlen - 1]);
				}

				if (action == A_NONE)
					goto retry;

				t = lex_action(vm, action, val);
				if (t == 0)
					goto retry;

				break;
			}

			if((sym == SOPHON_ERR_EOF) && (state == 0)) {
				t = SOPHON_ERR_EOF;
			} else {
				lex_error(vm, SOPHON_PARSER_ERROR,
							"illegal character");
				t = SOPHON_ERR_LEX;
			}

			break;
		}
	}

	if (first) {
		loc->first_line   = l->line;
		loc->first_column = l->column;
	}

	loc->last_line   = l->line;
	loc->last_column = l->column;

	if (l->flags & SOPHON_LEX_FL_SHELL_CONT) {
		l->flags &= ~SOPHON_LEX_FL_SHELL_CONT;
		l->flags |= SOPHON_LEX_FL_LAST_LT;
	}

	return t;
}

void
sophon_lex_to_regexp_cond (Sophon_VM *vm, Sophon_Token t)
{
	Sophon_LexData *l;

	SOPHON_ASSERT(vm);

	l = (Sophon_LexData*)vm->lex_data;
	SOPHON_ASSERT(l);

	l->tlen = 0;

	if (t == T_DIV_ASSIGN) {
		lex_tbuf_append(vm, '=');
	}

	l->cond = C_REGEXP;
}

