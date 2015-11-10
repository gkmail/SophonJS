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

#define STRING_FUNC_PROP(name, argc)\
	SOPHON_FUNCTION_PROP(name, GLOBAL_ATTRS, string_##name##_func, argc)
#define STRING_FUNC(name)\
	SOPHON_FUNC(string_##name##_func)

#ifdef SOPHON_8BITS_CHAR
	#define char_isspace(c) sophon_isspace(c)
	#define char_toupper(c) sophon_toupper(c)
	#define char_tolower(c) sophon_tolower(c)
#else
#ifdef SOPHON_HAVE_TOWUPPER
	#define char_toupper(c) sophon_towupper(c)
#else
	#define char_toupper(c) sophon_toupper(c)
#endif /*SOPHON_HAVE_TOWUPPER*/

#ifdef SOPHON_HAVE_TOWLOWER
	#define char_tolower(c) sophon_towlower(c)
#else
	#define char_tolower(c) sophon_tolower(c)
#endif /*SOPHON_HAVE_TOWLOWER*/

#ifdef SOPHON_HAVE_ISWSPACE
	#define char_isspace(c) sophon_iswspace(c)
#else
static const Sophon_U32
uc_space_table[] = {
	0x00090009, 0x000d000a, 0x00200020, 0x00a000a0,
	0x16801680, 0x180e180e, 0x200a2000, 0x20292028,
	0x202f202f, 0x205f205f, 0x30003000, 0xfefffeff
};

static Sophon_Bool
char_isspace (Sophon_Char ch)
{
	return sophon_char_table_search(ch, uc_space_table,
				SOPHON_ARRAY_SIZE(uc_space_table));
}
#endif /*SOPHON_HAVE_ISWSPACE*/
#endif /*SOPHON_8BITS_CHAR*/

static STRING_FUNC(call)
{
	Sophon_Value v = SOPHON_ARG(0);
	Sophon_String *str;
	Sophon_Result r;

	if (argc == 0) {
		str = vm->empty_str;
	} else {
		if ((r = sophon_value_to_string(vm, v, &str)) != SOPHON_OK)
			return r;
	}

	if (sophon_value_is_undefined(thisv)) {
		sophon_value_set_string(vm, retv, str);
	} else {
		Sophon_Object *obj;

		if ((r = sophon_value_to_object(vm, thisv, &obj)) != SOPHON_OK)
			return r;

		sophon_value_set_string(vm, &obj->primv, str);
	}

	return SOPHON_OK;
}

static STRING_FUNC(fromCharCode)
{
	Sophon_String *str;

	if (argc) {
		Sophon_Char *ptr;
		Sophon_U16 code;
		Sophon_Int i;
		Sophon_Result r;

		str = sophon_string_from_chars(vm, NULL, argc);
		ptr = sophon_string_chars(vm, str);
		for (i = 0; i < argc; i++) {
			if ((r = sophon_value_to_u16(vm, argv[i], &code)) != SOPHON_OK)
				return r;

			*ptr++ = code;
		}
	} else {
		str = vm->empty_str;
	}

	sophon_value_set_string(vm, retv, str);

	return SOPHON_OK;
}

static STRING_FUNC(toString)
{
	Sophon_String *str;
	Sophon_Result r;

	if (!sophon_value_is_string(thisv)) {
		sophon_throw(vm, vm->TypeError, "Value is not a string");
		return SOPHON_ERR_THROW;
	}

	if ((r = sophon_value_to_string(vm, thisv, &str)) != SOPHON_OK)
		return r;

	sophon_value_set_string(vm, retv, str);

	return SOPHON_OK;
}

static STRING_FUNC(valueOf)
{
	Sophon_String *str;
	Sophon_Result r;

	if (!sophon_value_is_string(thisv)) {
		sophon_throw(vm, vm->TypeError, "Value is not a string");
		return SOPHON_ERR_THROW;
	}

	if ((r = sophon_value_to_string(vm, thisv, &str)) != SOPHON_OK)
		return r;

	sophon_value_set_string(vm, retv, str);

	return SOPHON_OK;
}

static STRING_FUNC(charAt)
{
	Sophon_String *str, *rstr;
	Sophon_Int pos;
	Sophon_Char *chars, chr;
	Sophon_U32 len;
	Sophon_Result r;

	if (sophon_value_is_null(thisv) || sophon_value_is_undefined(thisv)) {
		sophon_throw(vm, vm->TypeError, "Value is null or undefined");
		return SOPHON_ERR_THROW;
	}

	if ((r = sophon_value_to_string(vm, thisv, &str)) != SOPHON_OK)
		return r;

	if ((r = sophon_value_to_int(vm, SOPHON_ARG(0), &pos)) != SOPHON_OK)
		return r;

	chars = sophon_string_chars(vm, str);
	len   = sophon_string_length(vm, str);

	if ((pos < 0) || (pos >= len)) {
		rstr = vm->empty_str;
	} else {
		chr  = chars[pos];
		rstr = sophon_string_from_chars(vm, &chr, 1);
	}

	sophon_value_set_string(vm, retv, rstr);

	return SOPHON_OK;
}

static STRING_FUNC(charCodeAt)
{
	Sophon_String *str;
	Sophon_Int pos;
	Sophon_Char *chars;
	Sophon_U32 len;
	Sophon_Result r;

	if (sophon_value_is_null(thisv) || sophon_value_is_undefined(thisv)) {
		sophon_throw(vm, vm->TypeError, "Value is null or undefined");
		return SOPHON_ERR_THROW;
	}

	if ((r = sophon_value_to_string(vm, thisv, &str)) != SOPHON_OK)
		return r;

	if ((r = sophon_value_to_int(vm, SOPHON_ARG(0), &pos)) != SOPHON_OK)
		return r;

	chars = sophon_string_chars(vm, str);
	len   = sophon_string_length(vm, str);

	if ((pos < 0) || (pos >= len)) {
		sophon_value_set_number(vm, retv, SOPHON_NAN);
	} else {
		sophon_value_set_int(vm, retv, chars[pos]);
	}

	return SOPHON_OK;
}

static STRING_FUNC(concat)
{
	Sophon_String *str, *nstr;
	Sophon_Int i;
	Sophon_Result r;

	if (sophon_value_is_null(thisv) || sophon_value_is_undefined(thisv)) {
		sophon_throw(vm, vm->TypeError, "Value is null or undefined");
		return SOPHON_ERR_THROW;
	}

	if ((r = sophon_value_to_string(vm, thisv, &str)) != SOPHON_OK)
		return r;

	for (i = 0; i < argc; i++) {
		if ((r = sophon_value_to_string(vm, argv[i], &nstr)) != SOPHON_OK)
			return r;

		str = sophon_string_concat(vm, str, nstr);
	}

	sophon_value_set_string(vm, retv, str);

	return SOPHON_OK;
}

static STRING_FUNC(indexOf)
{
	Sophon_String *str, *sub;
	Sophon_Int idx, pos = 0;
	Sophon_U32 len;
	Sophon_Char *cstr, *csub;
	const Sophon_Char *find;
	Sophon_Result r;

	if (sophon_value_is_null(thisv) || sophon_value_is_undefined(thisv)) {
		sophon_throw(vm, vm->TypeError, "Value is null or undefined");
		return SOPHON_ERR_THROW;
	}

	if ((r = sophon_value_to_string(vm, thisv, &str)) != SOPHON_OK)
		return r;

	if ((r = sophon_value_to_string(vm, SOPHON_ARG(0), &sub)) != SOPHON_OK)
		return r;

	cstr = sophon_string_chars(vm, str);
	len  = sophon_string_length(vm, str);
	csub = sophon_string_chars(vm, sub);

	if (!sophon_value_is_undefined(SOPHON_ARG(1))) {
		if ((r = sophon_value_to_int(vm, SOPHON_ARG(1), &pos)) != SOPHON_OK)
			return r;

		pos = SOPHON_MIN(SOPHON_MAX(0, pos), len);
	}

	find = sophon_ucstrstr(cstr + pos, csub);
	idx  = find ? find - cstr : -1;

	sophon_value_set_int(vm, retv, idx);

	return SOPHON_OK;
}

static STRING_FUNC(lastIndexOf)
{
	Sophon_String *str, *sub;
	Sophon_Int pos, idx = -1;
	Sophon_Int len;
	Sophon_Char *cstr, *csub, *p1, *p2;
	Sophon_Result r;

	if (sophon_value_is_null(thisv) || sophon_value_is_undefined(thisv)) {
		sophon_throw(vm, vm->TypeError, "Value is null or undefined");
		return SOPHON_ERR_THROW;
	}

	if ((r = sophon_value_to_string(vm, thisv, &str)) != SOPHON_OK)
		return r;

	if ((r = sophon_value_to_string(vm, SOPHON_ARG(0), &sub)) != SOPHON_OK)
		return r;

	cstr = sophon_string_chars(vm, str);
	csub = sophon_string_chars(vm, sub);
	len  = sophon_string_length(vm, str);

	pos = len;

	if (!sophon_value_is_undefined(SOPHON_ARG(1))) {
		Sophon_Number n;

		if ((r = sophon_value_to_number(vm, SOPHON_ARG(1), &n)) != SOPHON_OK)
			return r;

		if (!sophon_isnan(n)) {
			pos = n;
			pos = SOPHON_MIN(len, SOPHON_MAX(0, pos));
		}
	}

	while (pos >= 0) {
		p1 = cstr + pos;
		p2 = csub;

		while (1) {
			if (!*p2) {
				idx = pos;
				goto end;
			}

			if (*p1 != *p2)
				break;

			p1++;
			p2++;
		}

		pos--;
	}

end:
	sophon_value_set_int(vm, retv, idx);

	return SOPHON_OK;
}

static STRING_FUNC(localeCompare)
{
	Sophon_String *s1, *s2;
	Sophon_Result r;
	Sophon_Int cr;

	if (sophon_value_is_null(thisv) || sophon_value_is_undefined(thisv)) {
		sophon_throw(vm, vm->TypeError, "Value is null or undefined");
		return SOPHON_ERR_THROW;
	}

	if ((r = sophon_value_to_string(vm, thisv, &s1)) != SOPHON_OK)
		return r;

	if ((r = sophon_value_to_string(vm, SOPHON_ARG(0), &s2)) != SOPHON_OK)
		return r;

	cr = sophon_string_cmp(vm, s1, s2);
	sophon_value_set_int(vm, retv, cr);

	return SOPHON_OK;
}

#ifdef SOPHON_REGEXP

static STRING_FUNC(match)
{
	Sophon_Value rev = SOPHON_ARG(0);
	Sophon_String *str, *re_str;
	Sophon_RegExp *re;
	Sophon_Result r;
	Sophon_Char *chars;
	Sophon_ReMatch match;
	Sophon_Array *arr = NULL;
	Sophon_U32 id = 0;

	if (sophon_value_is_null(thisv) || sophon_value_is_undefined(thisv)) {
		sophon_throw(vm, vm->TypeError, "Value is null or undefined");
		return SOPHON_ERR_THROW;
	}

	if ((r = sophon_value_to_string(vm, thisv, &str)) != SOPHON_OK)
		return r;

	if (sophon_value_is_regexp(rev)) {
		if ((r = sophon_value_to_regexp(vm, rev, &re)) != SOPHON_OK)
			return r;
	} else {
		if ((r = sophon_value_to_string(vm, rev, &re_str)) != SOPHON_OK)
			return r;
		if (!(re = sophon_regexp_create(vm, re_str, 0)))
			return SOPHON_ERR_THROW;

		sophon_value_set_regexp(vm, &rev, re);
	}

	if (!(re->flags & SOPHON_REGEXP_FL_G)) {
		return sophon_value_call_prop(vm, rev,
				SOPHON_VALUE_GC(vm->exec_str),
				&thisv, 1, retv, 0);
	}

	chars = sophon_string_chars(vm, str);
	re->last = 0;
	while (1) {
		Sophon_String *sub;
		Sophon_Int last = re->last;

		if ((r = sophon_regexp_match(vm, re, str, last, &match, 1)) < 0)
			return r;

		if (r == 0) {
			re->last = 0;
			break;
		}

		last = match.end - chars;
		if (last == re->last)
			re->last++;
		else
			re->last = last;

		if (!arr)
			arr = sophon_array_create(vm);

		sub = sophon_string_from_chars(vm, match.begin,
				match.end - match.begin);
		sophon_array_set_item(vm, arr, id++, SOPHON_VALUE_GC(sub));
	}

	if (arr)
		sophon_value_set_array(vm, retv, arr);
	else
		sophon_value_set_null(vm, retv);

	return SOPHON_OK;
}

typedef enum {
	REP_TEXT,
	REP_MATCH,
	REP_CAP,
	REP_HEAD,
	REP_TAIL,
	REP_DOLLAR
} RepInsType;

typedef struct {
	RepInsType type;
	union {
		Sophon_ReMatch   text;
		Sophon_U32       cap;
	} r;
} RepIns;

typedef struct {
	RepIns     *ibuf;
	Sophon_U32  ilen;
	Sophon_U32  icap;
} RepData;

static RepIns*
rep_add_ins (Sophon_VM *vm, RepData *rd)
{
	if (rd->ilen >= rd->icap) {
		Sophon_U32 size = SOPHON_MAX(16, rd->icap * 2);

		rd->ibuf = (RepIns*)sophon_mm_realloc_ensure(vm,
				rd->ibuf,
				sizeof(RepIns) * rd->icap,
				sizeof(RepIns) * size);
		rd->icap = size;
	}

	return &rd->ibuf[rd->ilen++];
}

static Sophon_Result
rep_parse (Sophon_VM *vm, RepData *rd, Sophon_String *str)
{
	const Sophon_Char *chars = sophon_string_chars(vm, str);
	const Sophon_Char *ptr = chars;
	const Sophon_Char *begin = ptr;
	const Sophon_Char *next;
	RepIns *pi;

	while (*ptr) {
		next = sophon_ucstrchr(ptr, '$');
		if (next) {
			if (ptr != next) {
				pi = rep_add_ins(vm, rd);
				pi->type = REP_TEXT;
				pi->r.text.begin = (Sophon_Char*)begin;
				pi->r.text.end   = (Sophon_Char*)next;
			}

			switch (next[1]) {
				case '$':
					pi = rep_add_ins(vm, rd);
					pi->type = REP_DOLLAR;
					ptr = next + 2;
					begin = ptr;
					break;
				case '&':
					pi = rep_add_ins(vm, rd);
					pi->type = REP_MATCH;
					ptr = next + 2;
					begin = ptr;
					break;
				case '`':
					pi = rep_add_ins(vm, rd);
					pi->type = REP_HEAD;
					ptr = next + 2;
					begin = ptr;
					break;
				case '\'':
					pi = rep_add_ins(vm, rd);
					pi->type = REP_TAIL;
					ptr = next + 2;
					begin = ptr;
					break;
				default: {
					Sophon_Int n = 0;

					if (sophon_isdigit(next[1])) {
						if (sophon_isdigit(next[2])) {
							n = (next[1] - '0') * 10 +
									(next[2] - '0');
							ptr = next + 3;
							begin = ptr;
						} else {
							n = next[1] - '0';
							ptr = next + 2;
							begin = ptr;
						}
					}

					if (n) {
						pi = rep_add_ins(vm, rd);
						pi->type  = REP_CAP;
						pi->r.cap = n;
					} else {
						ptr = next + 1;
					}
					break;
				}
			}
		} else {
			if (*ptr) {
				pi = rep_add_ins(vm, rd);
				pi->type = REP_TEXT;
				pi->r.text.begin = (Sophon_Char*)begin;
				pi->r.text.end   = (Sophon_Char*)begin +
						sophon_ucstrlen(begin);
			}
			break;
		}
	}

	return SOPHON_OK;
}

static Sophon_String*
rep_replace (Sophon_VM *vm, RepData *rd, Sophon_String *orig,
		Sophon_ReMatch *match, Sophon_U32 size, Sophon_String *str)
{
	RepIns *pi, *pi_end;
	Sophon_Char *chars;
	Sophon_U32 len;

	chars = sophon_string_chars(vm, orig);
	len = sophon_string_length(vm, orig);

	pi = rd->ibuf;
	pi_end = pi + rd->ilen;

	while (pi < pi_end) {
		switch (pi->type) {
			case REP_TEXT:
				str = sophon_string_concat(vm, str,
						sophon_string_from_chars(vm, pi->r.text.begin,
							pi->r.text.end - pi->r.text.begin));
				break;
			case REP_MATCH:
				str = sophon_string_concat(vm, str,
						sophon_string_from_chars(vm, match[0].begin,
							match[0].end - match[0].begin));
				break;
			case REP_CAP:
				if (pi->r.cap < size) {
					Sophon_Int n = pi->r.cap;
					str = sophon_string_concat(vm, str,
							sophon_string_from_chars(vm, match[n].begin,
								match[n].end - match[n].begin));
				}
				break;
			case REP_HEAD:
				str = sophon_string_concat(vm, str,
						sophon_string_from_chars(vm, chars,
							match[0].begin - chars));
				break;
			case REP_TAIL:
				str = sophon_string_concat(vm, str,
						sophon_string_from_chars(vm, match[0].end,
							chars + len - match[0].end));
				break;
			case REP_DOLLAR:
				str = sophon_string_concat(vm, str, vm->dollar_str);
				break;
			default:
				SOPHON_ASSERT(0);
		}
		pi++;
	}

	return str;
}

static void
rep_free (Sophon_VM *vm, RepData *rd)
{
	if (rd->ibuf)
		sophon_mm_free(vm, rd->ibuf, sizeof(RepIns) * rd->icap);
}

static Sophon_String*
rep_replace_func (Sophon_VM *vm, Sophon_Value func, Sophon_String *orig,
		Sophon_ReMatch *match, Sophon_U32 size, Sophon_String *str)
{
	Sophon_Value rargv[size + 2];
	Sophon_Value rr;
	Sophon_String *tstr;
	Sophon_Char *chars;
	Sophon_U32 id;
	Sophon_Result r;

	chars = sophon_string_chars(vm, orig);

	for (id = 0; id < size; id++) {
		tstr = sophon_string_from_chars(vm, match[id].begin,
					match[id].end - match[id].begin);
		sophon_value_set_string(vm, &rargv[id], tstr);
	}
	
	sophon_value_set_int(vm, &rargv[size], match[0].begin - chars);
	sophon_value_set_string(vm, &rargv[size + 1], orig);

	if ((r = sophon_value_call(vm, func, SOPHON_VALUE_UNDEFINED,
						rargv, size + 2, &rr, 0)) != SOPHON_OK)
		return NULL;

	if ((r = sophon_value_to_string(vm, rr, &tstr)) != SOPHON_OK)
		return NULL;
	
	str = sophon_string_concat(vm, str, tstr);

	return str;
}

static STRING_FUNC(replace)
{
	Sophon_String *str, *sub_str, *rep_str, *rstr;
	Sophon_Value rep_func;
	Sophon_RegExp *re;
	Sophon_Result r;
	Sophon_Char *chars;
	Sophon_U32 len;
	Sophon_Int mcnt;
	RepData rd;

	rd.ibuf = NULL;
	rd.ilen = 0;
	rd.icap = 0;

	if (sophon_value_is_null(thisv) || sophon_value_is_undefined(thisv)) {
		sophon_throw(vm, vm->TypeError, "Value is null or undefined");
		return SOPHON_ERR_THROW;
	}

	if ((r = sophon_value_to_string(vm, thisv, &str)) != SOPHON_OK)
		return r;

	if (sophon_value_is_closure(SOPHON_ARG(1))) {
		rep_func = SOPHON_ARG(1);
		rep_str  = NULL;
	} else {
		if ((r = sophon_value_to_string(vm, SOPHON_ARG(1), &rep_str)) != SOPHON_OK)
			return r;

		if ((r = rep_parse(vm, &rd, rep_str)) != SOPHON_OK)
			return r;

		sophon_value_set_undefined(vm, &rep_func);
	}

	rstr = vm->empty_str;

	chars = sophon_string_chars(vm, str);
	len   = sophon_string_length(vm, str);

	if (!sophon_value_is_regexp(SOPHON_ARG(0))) {
		const Sophon_Char *sub, *mcstr;
		Sophon_U32 slen, alen;
		Sophon_ReMatch match;

		if ((r = sophon_value_to_string(vm, SOPHON_ARG(0),
						&sub_str)) != SOPHON_OK) {
			goto end;
		}

		sub  = sophon_string_chars(vm, sub_str);
		slen = sophon_string_length(vm, sub_str);

		if (!(mcstr = sophon_ucstrstr(chars, sub))) {
			rstr = str;
			r = SOPHON_OK;
			goto end;
		}

		match.begin = (Sophon_Char*)mcstr;
		match.end   = (Sophon_Char*)mcstr + slen;

		alen = mcstr - chars;
		if (alen) {
			rstr = sophon_string_concat(vm, rstr,
						sophon_string_from_chars(vm, chars, alen));
		}
	
		if (rep_str) {
			rstr = rep_replace(vm, &rd, str, &match, 1, rstr);
		} else {
			rstr = rep_replace_func(vm, rep_func, str, &match, 1, rstr);
		}

		alen = len - (mcstr - chars) - slen;
		if (alen) {
			rstr = sophon_string_concat(vm, rstr,
						sophon_string_from_chars(vm, match.end, alen));
		}

		sophon_value_set_string(vm, retv, rstr);
		r = SOPHON_OK;
		goto end;
	}

	if ((r = sophon_value_to_regexp(vm, SOPHON_ARG(0), &re)) != SOPHON_OK)
		goto end;

	if (re->flags & SOPHON_REGEXP_FL_G)
		re->last = 0;

	rstr  = vm->empty_str;
	mcnt  = sophon_regexp_match_size(re);
	while (1) {
		Sophon_ReMatch match[mcnt];
		Sophon_U32 last = re->last;

		r = sophon_regexp_match(vm, re, str, last, match, mcnt);
		if (r < 0)
			break;

		if (r == 0) {
			if (last < len)
				rstr = sophon_string_concat(vm, rstr,
						sophon_string_from_chars(vm,
							chars + last, len - last));
			re->last = 0;
			sophon_value_set_string(vm, retv, rstr);
			r = SOPHON_OK;
			break;
		} else {
			Sophon_U32 alen = match[0].begin - chars - last;

			if (alen) {
				rstr = sophon_string_concat(vm, rstr,
						sophon_string_from_chars(vm,
							chars + last, alen));
			}

			if (rep_str) {
				rstr = rep_replace(vm, &rd, str, match, mcnt, rstr);
			} else {
				rstr = rep_replace_func(vm, rep_func, str, match, mcnt, rstr);
			}

			last = match[0].end - chars;
			if (last == re->last) {
				if (last < len)
					rstr = sophon_string_concat(vm, rstr,
								sophon_string_from_chars(vm,
									chars + last, 1));
				re->last++;
			} else {
				re->last = last;
			}

			if (!(re->flags & SOPHON_REGEXP_FL_G)) {
				if (len > re->last) {
					rstr = sophon_string_concat(vm, rstr,
							sophon_string_from_chars(vm,
								chars + re->last, len - re->last));
				}
				break;
			}
		}
	}

end:
	if (rep_str)
		rep_free(vm, &rd);

	if (r == SOPHON_OK)
		sophon_value_set_string(vm, retv, rstr);

	return r;
}

static STRING_FUNC(search)
{
	Sophon_Value rev = SOPHON_ARG(0);
	Sophon_String *str, *re_str;
	Sophon_RegExp *re;
	Sophon_ReMatch match;
	Sophon_Int i;
	Sophon_Result r;

	if (sophon_value_is_null(thisv) || sophon_value_is_undefined(thisv)) {
		sophon_throw(vm, vm->TypeError, "Value is null or undefined");
		return SOPHON_ERR_THROW;
	}

	if ((r = sophon_value_to_string(vm, thisv, &str)) != SOPHON_OK)
		return r;

	if (sophon_value_is_regexp(rev)) {
		if ((r = sophon_value_to_regexp(vm, rev, &re)) != SOPHON_OK)
			return r;
	} else {
		if ((r = sophon_value_to_string(vm, rev, &re_str)) != SOPHON_OK)
			return r;
		if (!(re = sophon_regexp_create(vm, re_str, 0)))
			return SOPHON_ERR_THROW;
	}

	r = sophon_regexp_match(vm, re, str, 0, &match, 1);
	if (r < 0)
		return r;

	if (r == 0)
		i = -1;
	else
		i = match.begin - sophon_string_chars(vm, str);

	sophon_value_set_int(vm, retv, i);
	return SOPHON_OK;
}

static STRING_FUNC(split)
{
	Sophon_Array *arr;
	Sophon_String *str;
	Sophon_RegExp *sep_re = NULL;
	Sophon_String *sep_str = NULL;
	Sophon_Value sepv = SOPHON_ARG(0);
	Sophon_U32 lim, id, index, len, slen = 0;
	Sophon_Int mcnt;
	Sophon_Char *chars, *sub = NULL;
	Sophon_Result r;

	if (sophon_value_is_null(thisv) || sophon_value_is_undefined(thisv)) {
		sophon_throw(vm, vm->TypeError, "Value is null or undefined");
		return SOPHON_ERR_THROW;
	}

	if (sophon_value_is_undefined(SOPHON_ARG(1))) {
		lim = 0xFFFFFFFF;
	} else {
		if ((r = sophon_value_to_u32(vm, SOPHON_ARG(1), &lim)) != SOPHON_OK)
			return r;
	}

	arr = sophon_array_create(vm);

	if (lim == 0) {
		sophon_value_set_array(vm, retv, arr);
		return SOPHON_OK;
	}

	if ((r = sophon_value_to_string(vm, thisv, &str)) != SOPHON_OK)
		return r;

	if (sophon_value_is_undefined(sepv)) {
		sophon_array_set_item(vm, arr, 0, SOPHON_VALUE_GC(str));
		sophon_value_set_array(vm, retv, arr);
		return SOPHON_OK;
	}

	if (sophon_value_is_regexp(sepv)) {
		if ((r = sophon_value_to_regexp(vm, sepv, &sep_re)) != SOPHON_OK)
			return r;
	} else {
		if ((r = sophon_value_to_string(vm, sepv, &sep_str)) != SOPHON_OK)
			return r;
	}

	id    = 0;
	index = 0;
	chars = sophon_string_chars(vm, str);
	len   = sophon_string_length(vm, str);

	if (sep_re) {
		mcnt = sophon_regexp_match_size(sep_re);
	} else {
		mcnt = 1;
		sub  = sophon_string_chars(vm, sep_str);
		slen = sophon_string_length(vm, sep_str);
	}

	while (index <= len) {
		Sophon_ReMatch match[mcnt];
		Sophon_Bool found = SOPHON_FALSE;
		Sophon_U32 nindex, mindex, cap, ilen;
		Sophon_String *sstr;

		if (sep_re) {
			r = sophon_regexp_match(vm, sep_re, str, index, match, mcnt);
			if (r < 0)
				return r;

			if (r > 0) {
				nindex = match[0].end - chars;
				mindex = match[0].begin - chars;
				found = SOPHON_TRUE;
			} else {
				mindex = len;
				nindex = len;
			}
		} else {
			const Sophon_Char *ptr;

			ptr = sophon_ucstrstr(chars + index, sub);
			if (ptr) {
				mindex = ptr - chars;
				nindex = mindex + slen;
				found = SOPHON_TRUE;
			} else {
				mindex = len;
				nindex = len;
			}
		}

		ilen = mindex - index;

		if ((ilen == 0) && found && (mindex == nindex)) {
			ilen++;
			nindex++;

			if (nindex >= len)
				found = SOPHON_FALSE;
		}

		sstr = sophon_string_from_chars(vm, chars + index, ilen);
		sophon_array_set_item(vm, arr, id++, SOPHON_VALUE_GC(sstr));

		if ((lim != 0xFFFFFFFF) && (id >= lim))
			goto end;

		for (cap = 1; cap < mcnt; cap++) {
			Sophon_Value cv;

			if (match[cap].begin) {
				sstr = sophon_string_from_chars(vm, match[cap].begin,
							match[cap].end - match[cap].begin);
				sophon_value_set_string(vm, &cv, sstr);
			} else {
				sophon_value_set_undefined(vm, &cv);
			}

			sophon_array_set_item(vm, arr, id++, cv);

			if ((lim != 0xFFFFFFFF) && (id >= lim))
				goto end;
		}

		if (!found)
			break;

		index = nindex;
	}

end:
	sophon_value_set_array(vm, retv, arr);
	return SOPHON_OK;
}

#endif /*SOPHON_REGEXP*/

static STRING_FUNC(slice)
{
	Sophon_String *str, *sub;
	Sophon_Char *cstr;
	Sophon_Result r;
	Sophon_Int begin, end, len, slen;

	if (sophon_value_is_null(thisv) || sophon_value_is_undefined(thisv)) {
		sophon_throw(vm, vm->TypeError, "Value is null or undefined");
		return SOPHON_ERR_THROW;
	}

	if ((r = sophon_value_to_string(vm, thisv, &str)) != SOPHON_OK)
		return r;

	if ((r = sophon_value_to_int(vm, SOPHON_ARG(0), &begin)) != SOPHON_OK)
		return r;

	cstr = sophon_string_chars(vm, str);
	len  = sophon_string_length(vm, str);

	if (sophon_value_is_undefined(SOPHON_ARG(1))) {
		end = len;
	} else {
		if ((r = sophon_value_to_int(vm, SOPHON_ARG(1), &end)) != SOPHON_OK)
			return r;
	}

	if (begin < 0) {
		begin = SOPHON_MAX(len + begin, 0);
	} else {
		begin = SOPHON_MIN(begin, len);
	}

	if (end < 0) {
		end = SOPHON_MAX(len + end, 0);
	} else {
		end = SOPHON_MIN(end, len);
	}

	slen  = SOPHON_MAX(end - begin, 0);

	sub = sophon_string_from_chars(vm, cstr + begin, slen);
	sophon_value_set_string(vm, retv, sub);

	return SOPHON_OK;
}

static STRING_FUNC(substring)
{
	Sophon_String *str, *sub;
	Sophon_Char *cstr;
	Sophon_Result r;
	Sophon_Int begin, end, len, slen;

	if (sophon_value_is_null(thisv) || sophon_value_is_undefined(thisv)) {
		sophon_throw(vm, vm->TypeError, "Value is null or undefined");
		return SOPHON_ERR_THROW;
	}

	if ((r = sophon_value_to_string(vm, thisv, &str)) != SOPHON_OK)
		return r;

	if ((r = sophon_value_to_int(vm, SOPHON_ARG(0), &begin)) != SOPHON_OK)
		return r;

	cstr = sophon_string_chars(vm, str);
	len  = sophon_string_length(vm, str);

	if (sophon_value_is_undefined(SOPHON_ARG(1))) {
		end = len;
	} else {
		if ((r = sophon_value_to_int(vm, SOPHON_ARG(1), &end)) != SOPHON_OK)
			return r;
	}

	begin = SOPHON_MIN(SOPHON_MAX(begin, 0), len);
	end   = SOPHON_MIN(SOPHON_MAX(end, 0), len);
	if (begin > end) {
		Sophon_Int tmp = end;

		end = begin;
		begin = tmp;
	}

	slen  = end - begin;

	sub = sophon_string_from_chars(vm, cstr + begin, slen);
	sophon_value_set_string(vm, retv, sub);

	return SOPHON_OK;
}

static STRING_FUNC(toLowerCase)
{
	Sophon_String *str, *nstr;
	Sophon_Char *src, *dst;
	Sophon_Result r;
	Sophon_Int len, left;

	if (sophon_value_is_null(thisv) || sophon_value_is_undefined(thisv)) {
		sophon_throw(vm, vm->TypeError, "Value is null or undefined");
		return SOPHON_ERR_THROW;
	}

	if ((r = sophon_value_to_string(vm, thisv, &str)) != SOPHON_OK)
		return r;

	len = sophon_string_length(vm, str);

	nstr = sophon_string_from_chars(vm, NULL, len);

	src = sophon_string_chars(vm, str);
	dst = sophon_string_chars(vm, nstr);
	left = len;

	while (left--) {
		*dst = char_tolower(*src);
		dst++;
		src++;
	}

	sophon_value_set_string(vm, retv, nstr);

	return SOPHON_OK;
}

static STRING_FUNC(toLocaleLowerCase)
{
	return string_toLowerCase_func(vm, thisv, argv, argc, retv);
}

static STRING_FUNC(toUpperCase)
{
	Sophon_String *str, *nstr;
	Sophon_Char *src, *dst;
	Sophon_Result r;
	Sophon_Int len, left;

	if (sophon_value_is_null(thisv) || sophon_value_is_undefined(thisv)) {
		sophon_throw(vm, vm->TypeError, "Value is null or undefined");
		return SOPHON_ERR_THROW;
	}

	if ((r = sophon_value_to_string(vm, thisv, &str)) != SOPHON_OK)
		return r;

	len = sophon_string_length(vm, str);

	nstr = sophon_string_from_chars(vm, NULL, len);

	src = sophon_string_chars(vm, str);
	dst = sophon_string_chars(vm, nstr);
	left = len;

	while (left--) {
		*dst = char_toupper(*src);
		dst++;
		src++;
	}

	sophon_value_set_string(vm, retv, nstr);

	return SOPHON_OK;
}

static STRING_FUNC(toLocaleUpperCase)
{
	return string_toUpperCase_func(vm, thisv, argv, argc, retv);
}

static STRING_FUNC(trim)
{
	Sophon_String *str, *nstr;
	Sophon_Char *cstr, *begin, *end;
	Sophon_Result r;
	Sophon_Int len;

	if (sophon_value_is_null(thisv) || sophon_value_is_undefined(thisv)) {
		sophon_throw(vm, vm->TypeError, "Value is null or undefined");
		return SOPHON_ERR_THROW;
	}

	if ((r = sophon_value_to_string(vm, thisv, &str)) != SOPHON_OK)
		return r;

	cstr = sophon_string_chars(vm, str);
	len  = sophon_string_length(vm, str);

	begin = cstr;
	end   = begin + len;
	while (char_isspace(*begin) && (begin < end)) {
		begin++;
	}

	if (begin < end) {
		while (sophon_isspace(end[-1])) {
			end--;
		}
	}

	nstr = sophon_string_from_chars(vm, begin, end - begin);
	sophon_value_set_string(vm, retv, nstr);

	return SOPHON_OK;
}

#ifdef SOPHON_STRING_SUBSTR_FUNC
static STRING_FUNC(substr)
{
	Sophon_String *str, *sub;
	Sophon_Char *cstr;
	Sophon_Result r;
	Sophon_Int start, len, slen;

	if (sophon_value_is_null(thisv) || sophon_value_is_undefined(thisv)) {
		sophon_throw(vm, vm->TypeError, "Value is null or undefined");
		return SOPHON_ERR_THROW;
	}

	if ((r = sophon_value_to_string(vm, thisv, &str)) != SOPHON_OK)
		return r;

	if ((r = sophon_value_to_int(vm, SOPHON_ARG(0), &start)) != SOPHON_OK)
		return r;

	cstr = sophon_string_chars(vm, str);
	len  = sophon_string_length(vm, str);

	if (start < 0)
		start = SOPHON_MAX(len + start, 0);

	if (sophon_value_is_undefined(SOPHON_ARG(1))) {
		slen = len - start;
	} else {
		if ((r = sophon_value_to_int(vm, SOPHON_ARG(1), &slen)) != SOPHON_OK)
			return r;
		slen = SOPHON_MIN(slen, len - start);
	}

	if (slen <= 0) {
		sub = vm->empty_str;
	} else {
		sub = sophon_string_from_chars(vm, cstr + start, slen);
	}

	sophon_value_set_string(vm, retv, sub);

	return SOPHON_OK;
}
#endif

static STRING_FUNC(length_get)
{
	Sophon_String *str;
	Sophon_Result r;

	if (sophon_value_is_null(thisv) || sophon_value_is_undefined(thisv)) {
		sophon_throw(vm, vm->TypeError, "Value is null or undefined");
		return SOPHON_ERR_THROW;
	}

	if ((r = sophon_value_to_string(vm, thisv, &str)) != SOPHON_OK)
		return r;

	sophon_value_set_int(vm, retv, sophon_string_length(vm, str));
	return SOPHON_OK;
}

static const Sophon_Decl
string_prototype_props[] = {
	SOPHON_STRING_PROP([Class], 0, "String"),
	SOPHON_ACCESSOR_PROP(length, 0, string_length_get_func, NULL),
	STRING_FUNC_PROP(toString, 0),
	STRING_FUNC_PROP(valueOf, 0),
	STRING_FUNC_PROP(charAt, 1),
	STRING_FUNC_PROP(charCodeAt, 1),
	STRING_FUNC_PROP(concat, 1),
	STRING_FUNC_PROP(indexOf, 1),
	STRING_FUNC_PROP(lastIndexOf, 1),
	STRING_FUNC_PROP(localeCompare, 1),
#ifdef SOPHON_REGEXP
	STRING_FUNC_PROP(match, 1),
	STRING_FUNC_PROP(replace, 2),
	STRING_FUNC_PROP(search, 1),
	STRING_FUNC_PROP(split, 2),
#endif
	STRING_FUNC_PROP(slice, 2),
	STRING_FUNC_PROP(substring, 2),
	STRING_FUNC_PROP(toLowerCase, 0),
	STRING_FUNC_PROP(toLocaleLowerCase, 0),
	STRING_FUNC_PROP(toUpperCase, 0),
	STRING_FUNC_PROP(toLocaleUpperCase, 0),
	STRING_FUNC_PROP(trim, 0),
#ifdef SOPHON_STRING_SUBSTR_FUNC
	STRING_FUNC_PROP(substr, 2),
#endif
	{NULL}
};

static const Sophon_Decl
string_object_props[] = {
	SOPHON_OBJECT_PROP(prototype, 0, string_prototype_props),
	SOPHON_FUNCTION_PROP([Call], GLOBAL_ATTRS, string_call_func, 1),
	STRING_FUNC_PROP(fromCharCode, 1),
	{NULL}
};

