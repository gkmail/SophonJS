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

#ifdef SOPHON_UTF8_CHAR
#define string_len(cstr)   sophon_strlen(cstr)
#define string_cmp(s1, s2) sophon_strcmp(s1, s2)
#else
static Sophon_U32
string_len(const Sophon_Char *str)
{
	const Sophon_Char *p = str;
	Sophon_U32 len = 0;

	if (!str)
		return 0;

	while(*p) {
		len++;
		p++;
	}

	return len;
}

static Sophon_Int
string_cmp(const Sophon_Char *s1, const Sophon_Char *s2)
{
	Sophon_Int r;

	SOPHON_ASSERT(s1 && s2);

	if (s1 == s2)
		return 0;

	while (1) {
		r = *s1 - *s2;
		if (r)
			return r;

		if (!*s1)
			break;

		s1++;
		s2++;
	}

	return 0;
}
#endif /*SOPHON_UTF8_CHAR*/

Sophon_String*
sophon_string_create (Sophon_VM *vm, const Sophon_Char *cstr)
{
	Sophon_U32 len;

	SOPHON_ASSERT(vm);

	len = string_len(cstr);
	return sophon_string_from_chars(vm, cstr, len);
}

Sophon_String*
sophon_string_from_chars (Sophon_VM *vm, const Sophon_Char *chars,
			Sophon_U32 len)
{
	Sophon_String *str;
	Sophon_U32 size;

	SOPHON_ASSERT(vm);

	size = sizeof(Sophon_String) + len * sizeof(Sophon_Char);
	str = (Sophon_String*)sophon_mm_alloc(vm, size);
	if (!str)
		return NULL;

	SOPHON_GC_HEADER_INIT(str, SOPHON_GC_STRING);

	if (chars && len) {
		str->length = len;

		if (str->length) {
			memcpy(str->chars, chars, sizeof(Sophon_Char) * len);
		}
	} else {
		str->length = 0;
	}

	str->chars[str->length] = 0;

	sophon_gc_add(vm, (Sophon_GCObject*)str);
	return str;
}

#ifndef SOPHON_UTF8_CHAR

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
	ulen     = sophon_string_len(vm, str) * sizeof(Sophon_Char);
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

	size = sophon_string_len(vm, str);
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

#else /*defined(SOPHON_UTF8_CHAR)*/

Sophon_Result
sophon_string_to_utf8_cstr (Sophon_VM *vm,
			Sophon_String *str,
			char *buf,
			Sophon_U32 *plen)
{
	Sophon_Char *cstr;
	Sophon_U32 len;

	SOPHON_ASSERT(vm && str && buf && len);

	cstr = sophon_string_chars(vm, str);
	len  = sophon_string_len(vm, str);

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
	*plen = sophon_string_len(vm, str);

	return *plen;
}

#endif /*SOPHON_UTF8_CHAR*/

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

	SOPHON_ASSERT(vm && s1 && s2);

	if(s1 == s2)
		return 0;

	cs1 = sophon_string_chars(vm, s1);
	cs2 = sophon_string_chars(vm, s2);

	return string_cmp(cs1, cs2);
}


