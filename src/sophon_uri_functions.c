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

#define URI_MASK_ALPHA 1
#define URI_MASK_SHARP 2
#define URI_MASK_RESERVED 4

static Sophon_Bool
uri_char_in_set (Sophon_Char chr, Sophon_U32 mask)
{
	if ((mask & URI_MASK_ALPHA) && sophon_isalpha(chr))
		return SOPHON_TRUE;
	if ((mask & URI_MASK_SHARP) && (chr == '#'))
		return SOPHON_TRUE;
	if (mask & URI_MASK_RESERVED) {
		switch (chr) {
			case ';':
			case '/':
			case '?':
			case ':':
			case '@':
			case '&':
			case '=':
			case '+':
			case '$':
			case ',':
				return SOPHON_TRUE;
		}
	}

	return SOPHON_FALSE;
}

#define uri_hex_char(n) (((n) < 10) ? (n) + '0' : (n) - 10 + 'A')

static Sophon_Result
uri_encode (Sophon_VM *vm, Sophon_String *sin, Sophon_String **psout,
			Sophon_U32 mask)
{
	Sophon_Char *sbuf = sophon_string_chars(vm, sin);
	Sophon_U32 slen = sophon_string_length(vm, sin);
	Sophon_Char *dbuf, *src, *dst;
	Sophon_U32 dlen;
	Sophon_String *sout;
	Sophon_Result r = SOPHON_ERR_THROW;

	if (!slen) {
		*psout = vm->empty_str;
		return SOPHON_OK;
	}

	dbuf = sophon_mm_alloc_ensure(vm, slen * 9 * sizeof(Sophon_Char));
	src  = sbuf;
	dst  = dbuf;

	while (*src) {
		if (uri_char_in_set(*src, mask)) {
			*dst++ = *src++;
		} else {
			Sophon_U32 v;

			if ((*src >= 0xDC00) && (*src <= 0xDFFF))
				goto end;

			if ((*src < 0xD800) || (*src > 0xDBFF)) {
				v = *src++;
			} else {
				if ((src[1] == 0) || (src[1] < 0xDC00) || (src[1] > 0xDFFF))
					goto end;

				v = (src[0] - 0xD800) * 0x400 + (src[1] - 0xDC00) + 0x10000;
				src += 2;
			}

			if (v <= 0x7F) {
				*dst++ = '%';
				*dst++ = uri_hex_char(v >> 4);
				*dst++ = uri_hex_char(v & 0xF);
			} else if (v <= 0x7FF) {
				*dst++ = '%';
				*dst++ = uri_hex_char((v >> 10) | 0xC);
				*dst++ = uri_hex_char((v >> 6) & 0x0F);
				*dst++ = '%';
				*dst++ = uri_hex_char(((v >> 4) & 0x03) | 0x8);
				*dst++ = uri_hex_char(v & 0x0F);
			} else if (v <= 0xFFFF) {
				*dst++ = '%';
				*dst++ = 'E';
				*dst++ = uri_hex_char(v >> 12);
				*dst++ = '%';
				*dst++ = uri_hex_char(((v >> 10) & 0x03) | 0x8);
				*dst++ = uri_hex_char((v >> 6) & 0x0F);
				*dst++ = '%';
				*dst++ = uri_hex_char(((v >> 4) & 0x03) | 0x8);
				*dst++ = uri_hex_char(v & 0x0F);
			} else {
				*dst++ = '%';
				*dst++ = 'F';
				*dst++ = uri_hex_char(v >> 18);
				*dst++ = '%';
				*dst++ = uri_hex_char(((v >> 16) & 0x03) | 0x8);
				*dst++ = uri_hex_char((v >> 12) & 0x0F);
				*dst++ = '%';
				*dst++ = uri_hex_char(((v >> 10) & 0x03) | 0x8);
				*dst++ = uri_hex_char((v >> 6) & 0x0F);
				*dst++ = '%';
				*dst++ = uri_hex_char(((v >> 4) & 0x03) | 0x8);
				*dst++ = uri_hex_char(v & 0x0F);
			}
		}
	}

	dlen = dst - dbuf;
	sout = sophon_string_from_chars(vm, dbuf, dlen);
	*psout = sout;
	r = SOPHON_OK;
end:
	if (r != SOPHON_OK) {
		sophon_throw(vm, vm->URIError, "Illegal characters in URI");
	}
	sophon_mm_free(vm, dbuf, slen * 9 * sizeof(Sophon_Char));
	return r;
}

static Sophon_Int
uri_char_hex(Sophon_VM *vm, Sophon_Char *src)
{
	Sophon_Int n;

	if ((src[0] >= '0') && (src[0] <= '9'))
		n = src[0] - '0';
	else if ((src[0] >= 'a') && (src[0] <= 'f'))
		n = src[0] - 'a' + 10;
	else if ((src[0] >= 'A') && (src[0] <= 'F'))
		n = src[0] - 'A' + 10;
	else
		return SOPHON_ERR_THROW;

	return n;
}

static Sophon_Int
uri_code (Sophon_VM *vm, Sophon_Char *src)
{
	Sophon_Int n1, n2;

	if ((src[0] != '%') || !src[1] || !src[2])
		return SOPHON_ERR_THROW;

	if ((n1 = uri_char_hex(vm, src + 1)) < 0)
		return n1;

	if ((n2 = uri_char_hex(vm, src + 2)) < 0)
		return n2;

	return (n1 << 4) | n2;
}

static Sophon_Result
uri_decode (Sophon_VM *vm, Sophon_String *sin, Sophon_String **psout,
			Sophon_U32 mask)
{
	Sophon_Char *sbuf = sophon_string_chars(vm, sin);
	Sophon_U32 slen = sophon_string_length(vm, sin);
	Sophon_Char *dbuf, *src, *dst;
	Sophon_U32 dlen;
	Sophon_String *sout;
	Sophon_Result r = SOPHON_ERR_THROW;

	if (!slen) {
		*psout = vm->empty_str;
		return SOPHON_OK;
	}

	dbuf = sophon_mm_alloc_ensure(vm, slen * sizeof(Sophon_Char));
	src  = sbuf;
	dst  = dbuf;

	while (*src) {
		if (*src != '%') {
			*dst++ = *src++;
		} else {
			Sophon_Int n1, n2, n3, n4, v;

			if ((n1 = uri_code(vm, src)) < 0)
				goto end;
			src += 3;

			if (!(n1 & 0x80)) {
				if (uri_char_in_set(n1, mask)) {
					*dst++ = src[-3];
					*dst++ = src[-2];
					*dst++ = src[-1];
				} else {
					*dst++ = n1;
				}
			} else if ((n1 & 0xE0) == 0xC0) {
				if ((n2 = uri_code(vm, src)) < 0)
					goto end;
				if ((n2 & 0xC0) != 0x80)
					goto end;
				src += 3;
				*dst++ = ((n1 & 0x1F) << 6) | (n2 & 0x3F);
			} else if ((n1 & 0xF0) == 0xE0) {
				if ((n2 = uri_code(vm, src)) < 0)
					goto end;
				if ((n2 & 0xC0) != 0x80)
					goto end;
				src += 3;
				if ((n3 = uri_code(vm, src)) < 0)
					goto end;
				if ((n3 & 0xC0) != 0x80)
					goto end;
				src += 3;
				*dst++ = ((n1 & 0x0F) << 12) | ((n2 & 0x3F) << 6) |
							(n3 & 0x3F);
			} else if ((n1 & 0xF8) == 0xF0) {
				if ((n2 = uri_code(vm, src)) < 0)
					goto end;
				if ((n2 & 0xC0) != 0x80)
					goto end;
				src += 3;
				if ((n3 = uri_code(vm, src)) < 0)
					goto end;
				if ((n3 & 0xC0) != 0x80)
					goto end;
				src += 3;
				if ((n4 = uri_code(vm, src)) < 0)
					goto end;
				if ((n4 & 0xC0) != 0x80)
					goto end;
				src += 3;

				v = ((n1 & 0x7) << 18) | ((n2 & 0x3F) << 12) |
							((n3 & 0x3F) << 6) | (n4 & 0x3F);

				*dst++ = (((v - 0x10000) >> 10) & 0x3FF) + 0xD800;
				*dst++ = ((v - 0x10000) & 0x3FF) + 0xDC00;
			} else {
				goto end;
			}
		}
	}

	dlen = dst - dbuf;
	sout = sophon_string_from_chars(vm, dbuf, dlen);
	*psout = sout;
	r = SOPHON_OK;
end:
	if (r != SOPHON_OK) {
		sophon_throw(vm, vm->URIError, "Illegal characters in URI");
	}
	sophon_mm_free(vm, dbuf, slen * sizeof(Sophon_Char));
	return r;
}


static GLOBAL_FUNC(decodeURI)
{
	Sophon_String *str, *nstr;
	Sophon_Result r;

	if ((r = sophon_value_to_string(vm, SOPHON_ARG(0), &str)) != SOPHON_OK)
		return r;

	if ((r = uri_decode(vm, str, &nstr, URI_MASK_RESERVED | URI_MASK_SHARP))
				!= SOPHON_OK)
		return r;

	sophon_value_set_string(vm, retv, nstr);

	return SOPHON_OK;
}

static GLOBAL_FUNC(decodeURIComponent)
{
	Sophon_String *str, *nstr;
	Sophon_Result r;

	if ((r = sophon_value_to_string(vm, SOPHON_ARG(0), &str)) != SOPHON_OK)
		return r;

	if ((r = uri_decode(vm, str, &nstr, 0)) != SOPHON_OK)
		return r;

	sophon_value_set_string(vm, retv, nstr);

	return SOPHON_OK;
}

static GLOBAL_FUNC(encodeURI)
{
	Sophon_String *str, *nstr;
	Sophon_Result r;

	if ((r = sophon_value_to_string(vm, SOPHON_ARG(0), &str)) != SOPHON_OK)
		return r;

	if ((r = uri_encode(vm, str, &nstr,
						URI_MASK_ALPHA|URI_MASK_RESERVED|URI_MASK_SHARP))
				!= SOPHON_OK)
		return r;

	sophon_value_set_string(vm, retv, nstr);

	return SOPHON_OK;
}

static GLOBAL_FUNC(encodeURIComponent)
{
	Sophon_String *str, *nstr;
	Sophon_Result r;

	if ((r = sophon_value_to_string(vm, SOPHON_ARG(0), &str)) != SOPHON_OK)
		return r;

	if ((r = uri_encode(vm, str, &nstr, URI_MASK_ALPHA)) != SOPHON_OK)
		return r;

	sophon_value_set_string(vm, retv, nstr);

	return SOPHON_OK;
}

