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
#include <sophon_conv.h>
#include <sophon_debug.h>

Sophon_String*
sophon_string_create (Sophon_VM *vm, const Sophon_Char *cstr)
{
	Sophon_U32 len;

	SOPHON_ASSERT(vm);

	len = sophon_ucstrlen(cstr);
	return sophon_string_from_chars(vm, cstr, len);
}

Sophon_String*
sophon_string_from_chars (Sophon_VM *vm, const Sophon_Char *chars,
			Sophon_U32 len)
{
	Sophon_String *str;
	Sophon_U32 size;

	SOPHON_ASSERT(vm);

	if (!len && vm->empty_str)
		return vm->empty_str;

	size = sizeof(Sophon_String) + len * sizeof(Sophon_Char);
	str = (Sophon_String*)sophon_mm_alloc(vm, size);
	if (!str)
		return NULL;

	SOPHON_GC_HEADER_INIT(str, SOPHON_GC_String);

	str->length = len;

	if (chars) {
		sophon_memcpy(str->chars, chars, sizeof(Sophon_Char) * len);
	}

	str->chars[str->length] = 0;

	sophon_gc_add(vm, (Sophon_GCObject*)str);
	return str;
}

#ifndef SOPHON_8BITS_CHAR

Sophon_String*
sophon_string_from_utf8_cstr (Sophon_VM *vm, const char *cstr)
{
	Sophon_U32 len;

	SOPHON_ASSERT(vm);

	len = cstr ? sophon_strlen(cstr) : 0;

	return sophon_string_from_utf8_chars(vm, cstr, len);
}

Sophon_String*
sophon_string_from_utf8_chars (Sophon_VM *vm, const char *chars,
			Sophon_U32 len)
{
	Sophon_Char *buf = NULL;
	Sophon_Char *ucs;
	Sophon_U32 ulen;
	Sophon_String *str = NULL;

	SOPHON_ASSERT(vm);

	if (chars && len) {
		Sophon_Conv conv;
		Sophon_U32 in_size;
		Sophon_U32 out_size;
		Sophon_Result r;

		buf = (Sophon_Char*)sophon_mm_alloc(vm, sizeof(Sophon_Char) * len);
		if (! buf)
			return NULL;

		in_size  = len;
		out_size = sizeof(Sophon_Char) * len;

		r = sophon_conv_create(&conv, SOPHON_UTF8, SOPHON_UTF16);
		if (r != SOPHON_OK)
			goto end;

		r = sophon_conv(conv, (Sophon_U8*)chars, &in_size,
					(Sophon_U8*)buf, &out_size);
		sophon_conv_destroy(conv);

		if (r != SOPHON_OK)
			goto end;

		ucs  = buf;
		ulen = out_size / sizeof(Sophon_Char);
	} else {
		ucs  = NULL;
		ulen = 0;
	}

	str = sophon_string_from_chars(vm, ucs, ulen);

end:
	if (buf) {
		sophon_mm_free(vm, buf, sizeof(Sophon_Char) * len);
	}

	return str;
}

Sophon_Result
sophon_string_to_utf8_cstr (Sophon_VM *vm,
			Sophon_String *str,
			char *buf,
			Sophon_U32 *len)
{
	Sophon_Conv conv;
	Sophon_Char *in_buf;
	Sophon_U32 ulen, in_size;
	Sophon_U32 out_size;
	Sophon_Result r;

	SOPHON_ASSERT(vm && str && buf && len);

	in_buf   = sophon_string_chars(vm, str);
	ulen     = sophon_string_length(vm, str) * sizeof(Sophon_Char);
	in_size  = ulen;
	out_size = *len;

	r = sophon_conv_create(&conv, SOPHON_UTF16, SOPHON_UTF8);
	if (r != SOPHON_OK)
		return r;

	r = sophon_conv(conv, (Sophon_U8*)in_buf, &in_size,
				(Sophon_U8*)buf, &out_size);
	sophon_conv_destroy(conv);

	if (r != SOPHON_OK)
		return r;

	if ((in_size != ulen) && (out_size >= *len))
		return SOPHON_ERR_2BIG;

	buf[out_size] = 0;
	*len = out_size;

	return out_size;
}

Sophon_Result
sophon_string_new_utf8_cstr (Sophon_VM *vm,
			Sophon_String *str,
			char **pbuf,
			Sophon_U32 *plen)
{
	char *buf;
	Sophon_U32 size;
	Sophon_U32 len;
	Sophon_Result r;

	SOPHON_ASSERT(vm && str && pbuf && plen);

	size = sophon_string_length(vm, str);
	len  = SOPHON_UTF8_BUF_SIZE(size);
	buf  = (char*)sophon_mm_alloc(vm, len);
	if (! buf)
		return SOPHON_ERR_NOMEM;

	size = len;
	r = sophon_string_to_utf8_cstr(vm, str, buf, &size);
	if (r < 0){
		sophon_mm_free(vm, buf, len);
		return r;
	}

	*plen = len;
	*pbuf = buf;
	return r;
}

#else /*defined(SOPHON_8BITS_CHAR)*/

Sophon_Result
sophon_string_to_utf8_cstr (Sophon_VM *vm,
			Sophon_String *str,
			char *buf,
			Sophon_U32 *plen)
{
	Sophon_Char *cstr;
	Sophon_U32 len;

	SOPHON_ASSERT(vm && str && buf && plen);

	cstr = sophon_string_chars(vm, str);
	len  = sophon_string_length(vm, str);

	if (len >= *plen)
		return SOPHON_ERR_2BIG;

	sophon_memcpy(buf, cstr, len + 1);
	*plen = len;


	return len;
}

Sophon_Result
sophon_string_new_utf8_cstr (Sophon_VM *vm,
			Sophon_String *str,
			char **pbuf,
			Sophon_U32 *plen)
{
	SOPHON_ASSERT(vm && str && pbuf && plen);

	*pbuf = sophon_string_chars(vm, str);
	*plen = sophon_string_length(vm, str);

	return *plen;
}

#endif /*SOPHON_8BITS_CHAR*/

void
sophon_string_destroy (Sophon_VM *vm, Sophon_String *str)
{
	Sophon_U32 size;

	SOPHON_ASSERT(vm && str);

	if (str->gc_flags & SOPHON_GC_FL_INTERN) {
		 sophon_hash_remove(vm, &vm->str_intern_hash, sophon_string_key,
					sophon_string_equal, str);
	}

	size = sizeof(Sophon_String) + str->length * sizeof(Sophon_Char);

	sophon_mm_free(vm, str, size);
}

Sophon_String*
sophon_string_intern_real (Sophon_VM *vm, Sophon_String *str)
{
	Sophon_Result r;
	Sophon_HashEntry *ent;

	SOPHON_ASSERT(vm && str);

	r = sophon_hash_add(vm, &vm->str_intern_hash, sophon_string_key,
				sophon_string_equal, str, &ent);
	if (r < 0)
		return NULL;

	if (r == SOPHON_NONE)
		return (Sophon_String*)ent->value;

	ent->value = (Sophon_Ptr)str;
	str->gc_flags |= SOPHON_GC_FL_INTERN;

	return str;
}

Sophon_Int
sophon_string_cmp (Sophon_VM *vm, Sophon_String *s1, Sophon_String *s2)
{
	Sophon_Char *cs1, *cs2;
	Sophon_U32 l1, l2;
	Sophon_Int r;

	SOPHON_ASSERT(vm && s1 && s2);

	if(s1 == s2)
		return 0;

	cs1 = sophon_string_chars(vm, s1);
	cs2 = sophon_string_chars(vm, s2);
	l1  = sophon_string_length(vm, s1);
	l2  = sophon_string_length(vm, s2);

	while ((l1 > 0) && (l2 > 0)) {
		r = *cs1 - *cs2;
		if (r != 0)
			return r;

		cs1 ++;
		cs2 ++;
		l1 --;
		l2 --;
	}

	return l1 - l2;
}

Sophon_Int
sophon_string_casecmp (Sophon_VM *vm, Sophon_String *s1, Sophon_String *s2)
{
	Sophon_Char *cs1, *cs2;
	Sophon_U32 l1, l2;
	Sophon_Int r;

	SOPHON_ASSERT(vm && s1 && s2);

	if(s1 == s2)
		return 0;

	cs1 = sophon_string_chars(vm, s1);
	cs2 = sophon_string_chars(vm, s2);
	l1  = sophon_string_length(vm, s1);
	l2  = sophon_string_length(vm, s2);

	while ((l1 > 0) && (l2 > 0)) {
		r = sophon_tolower(*cs1) - sophon_tolower(*cs2);
		if (r != 0)
			return r;

		cs1 ++;
		cs2 ++;
		l1 --;
		l2 --;
	}

	return l1 - l2;
}

Sophon_String*
sophon_string_concat (Sophon_VM *vm, Sophon_String *s1, Sophon_String *s2)
{
	Sophon_U32 len1, len2;
	Sophon_String *nstr;

	SOPHON_ASSERT(vm && s1 && s2);

	len1 = sophon_string_length(vm, s1);
	len2 = sophon_string_length(vm, s2);

	nstr = sophon_string_from_chars(vm, NULL, len1 + len2);
	if (nstr) {
		Sophon_Char *dst = nstr->chars;
		Sophon_Char *src;

		if (len1) {
			src = sophon_string_chars(vm, s1);
			sophon_memcpy(dst, src, len1 * sizeof(Sophon_Char));
		}

		if (len2) {
			src = sophon_string_chars(vm, s2);
			sophon_memcpy(dst + len1, src, len2 * sizeof(Sophon_Char));
		}
	}

	return nstr;
}

#define HEX_CHAR(n) (((n) < 10) ? (n) + '0' : (n) - 10 + 'a')

Sophon_String*
sophon_string_escape (Sophon_VM *vm, Sophon_String *str)
{
	Sophon_String *nstr;
	Sophon_Char *cstr = sophon_string_chars(vm, str);
	Sophon_U32 len = sophon_string_length(vm, str);
	Sophon_U32 left;
	Sophon_Char *buf, *src, *dst;
	
	buf = sophon_mm_alloc_ensure(vm, len * 6 * sizeof(Sophon_Char));
	src = cstr;
	dst = buf;
	left = len;
	
	while (left) {
		if (*src == '\n') {
			*dst++ = '\\';
			*dst++ = 'n';
		} else if (*src == '\r') {
			*dst++ = '\\';
			*dst++ = 'r';
		} else if (*src == '\v') {
			*dst++ = '\\';
			*dst++ = 'v';
		} else if (*src == '\f') {
			*dst++ = '\\';
			*dst++ = 'f';
		} else if (*src == '\b') {
			*dst++ = '\\';
			*dst++ = 'b';
		} else if (*src == '\t') {
			*dst++ = '\\';
			*dst++ = 't';
		} else if (*src == 0) {
			*dst++ = '\\';
			*dst++ = '0';
		} else if (*src == '\\') {
			*dst++ = '\\';
			*dst++ = '\\';
		} else if (*src == '\'') {
			*dst++ = '\\';
			*dst++ = '\'';
		} else if (*src == '\"') {
			*dst++ = '\\';
			*dst++ = '\"';
		} else if (sophon_isprint(*src)) {
			*dst++ = *src;
		} else if (((Sophon_U16)(*src)) <= 0xFF) {
			*dst++ = '\\';
			*dst++ = 'x';
			*dst++ = HEX_CHAR(((*src) >> 4) & 0x0F);
			*dst++ = HEX_CHAR((*src) & 0x0F);
		} else {
			*dst++ = '\\';
			*dst++ = 'u';
			*dst++ = HEX_CHAR(((*src) >> 12) & 0x0F);
			*dst++ = HEX_CHAR(((*src) >> 8) & 0x0F);
			*dst++ = HEX_CHAR(((*src) >> 4) & 0x0F);
			*dst++ = HEX_CHAR((*src) & 0x0F);
		}

		src++;
		left--;
	}
	
	nstr = sophon_string_from_chars(vm, buf, dst - buf);
	sophon_mm_free(vm, buf, len * 6 * sizeof(Sophon_Char));

	return nstr;
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

Sophon_String*
sophon_string_unescape (Sophon_VM *vm, Sophon_String *str)
{
	Sophon_String *nstr;
	Sophon_Char *cstr = sophon_string_chars(vm, str);
	Sophon_U32 len = sophon_string_length(vm, str);
	Sophon_U32 left;
	Sophon_Char *buf, *src, *dst;
	Sophon_Result r = SOPHON_ERR_LEX;
	
	buf = sophon_mm_alloc_ensure(vm, len * sizeof(Sophon_Char));
	src = cstr;
	dst = buf;
	left = len;
	
	while (left) {
		if (*src != '\\') {
			*dst++ = *src++;
			left--;
		} else {
			switch (src[1]) {
				case 'n':
					*dst++ = '\n';
					src += 2;
					left -= 2;
					break;
				case 'r':
					*dst++ = '\r';
					src += 2;
					left -= 2;
					break;
				case 'v':
					*dst++ = '\v';
					src += 2;
					left -= 2;
					break;
				case 'f':
					*dst++ = '\f';
					src += 2;
					left -= 2;
					break;
				case 'b':
					*dst++ = '\b';
					src += 2;
					left -= 2;
					break;
				case 't':
					*dst++ = '\t';
					src += 2;
					left -= 2;
					break;
				case '0':
					*dst++ = '\0';
					src += 2;
					left -= 2;
					break;
				case 'x':
					if (!sophon_isxdigit(src[2]) ||
								!sophon_isxdigit(src[3]))
						goto end;
					*dst++ = (hex_val(src[2]) << 4) |
								hex_val(src[3]);
					src += 4;
					left -= 4;
					break;
				case 'u':
					if (!sophon_isxdigit(src[2]) ||
								!sophon_isxdigit(src[3]) ||
								!sophon_isxdigit(src[4]) ||
								!sophon_isxdigit(src[5]))
						goto end;
					*dst++ = (hex_val(src[2]) << 12) |
								(hex_val(src[3]) << 8) |
								(hex_val(src[4]) << 4) |
								hex_val(src[5]);
					src += 6;
					left -= 6;
					break;
				default:
					if (sophon_isprint(src[1])) {
						*dst++ = src[1];
						src += 2;
						left -= 2;
					} else {
						goto end;
					}
					break;
			}
		}
	}

	r = SOPHON_OK;
	
end:
	if (r == SOPHON_OK) {
		nstr = sophon_string_from_chars(vm, buf, dst - buf);
	} else {
		sophon_throw(vm, vm->SyntaxError, "Illegal character");
		nstr = NULL;
	}

	sophon_mm_free(vm, buf, len * sizeof(Sophon_Char));

	return nstr;
}

Sophon_Result
sophon_string_get_item (Sophon_VM *vm, Sophon_String *str,
			Sophon_U32 id, Sophon_Value *retv)
{
	Sophon_U32 len;

	SOPHON_ASSERT(vm && str && retv);

	len = sophon_string_length(vm, str);

	if (id < len) {
		Sophon_String *sub;

		sub = sophon_string_from_chars(vm, str->chars + id, 1);
		sophon_value_set_string(vm, retv, sub);
	} else {
		sophon_value_set_undefined(vm, retv);
	}

	return SOPHON_OK;
}

