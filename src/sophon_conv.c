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
#include <sophon_conv.h>
#include <sophon_debug.h>

static Sophon_Result
utf8_to_utf16 (const Sophon_U8 *in_buf,
			Sophon_U32 *in_size,
			Sophon_U8 *out_buf,
			Sophon_U32 *out_size,
			Sophon_Bool swap)
{
	Sophon_U32 sleft = *in_size;
	Sophon_U32 dleft = *out_size;
	const Sophon_U8 *src = in_buf;
	Sophon_U8 *dst = out_buf;
	Sophon_U16 uc, tmp;

	while ((sleft >= 1) && (dleft >= 2)) {
		Sophon_Int bytes;

		if ((src[0] & 0xF8) == 0xF0) {
			Sophon_U32 code;

			if (sleft < 4)
				break;

			code = ((src[0] & 0x07) << 18) | ((src[1] & 0x3F) << 12) |
						((src[2] & 0x3F) << 6) | (src[3] & 0x3F);

			uc = (((code - 0x10000) >> 10) & 0x3FF) + 0xD800;
			if (swap)
				tmp = (uc >> 8) | (uc << 8);
			else
				tmp = uc;
			*(Sophon_U16*)dst = tmp;
			dst += 2;
			dleft -= 2;

			uc = ((code - 0x10000) & 0x3FF) + 0xDC00;
			bytes = 4;
		} else if ((src[0] & 0xF0) == 0xE0) {
			if (sleft < 3)
				break;

			uc = ((src[0] & 0x0F) << 12) | ((src[1] & 0x3F) << 6) |
						(src[2] & 0x3F);
			bytes = 3;
		} else if ((src[0] & 0xE0) == 0xC0) {
			if (sleft < 2)
				break;

			uc = ((src[0] & 0x1F) << 6) | (src[1] & 0x3F);
			bytes = 2;
		} else if (!(src[0] & 0x80)) {
			uc = src[0];
			bytes = 1;
		} else {
			bytes = 0;
			SOPHON_ERROR(("illegal UTF8 code 0x%02x", src[0]));
			break;
		}

		if (swap) {
			tmp = (uc >> 8) | (uc << 8);
		} else {
			tmp = uc;
		}

		*(Sophon_U16*)dst = tmp;

		src += bytes;
		dst += 2;
		sleft -= bytes;
		dleft -= 2;
	}

	*in_size  = *in_size - sleft;
	*out_size = *out_size - dleft;
	return SOPHON_OK;
}

static Sophon_Result
utf16_to_utf8 (const Sophon_U8 *in_buf,
			Sophon_U32 *in_size,
			Sophon_U8 *out_buf,
			Sophon_U32 *out_size,
			Sophon_Bool swap)
{
	Sophon_U32 sleft = *in_size;
	Sophon_U32 dleft = *out_size;
	const Sophon_U8 *src = in_buf;
	Sophon_U8 *dst = out_buf;
	Sophon_U16 uc, tmp;
	Sophon_Int bytes;

	while ((sleft >= 2) && (dleft >= 1)) {
		tmp = *(Sophon_U16*)src;
		if (swap) {
			uc = (tmp >> 8) | (tmp << 8);
		} else {
			uc = tmp;
		}

		if ((uc >= 0xDC00) && (uc <= 0xDFFF)) {
			break;
		} else if ((uc >= 0xD800) && (uc <= 0xDBFF)) {
			Sophon_U32 code;
			Sophon_U16 uc2;

			src += 2;
			sleft -= 2;

			tmp = *(Sophon_U16*)src;
			if (swap) {
				uc2 = (tmp >> 8) | (tmp << 8);
			} else {
				uc2 = tmp;
			}

			if ((uc2 < 0xDC00) || (uc2 > 0xDFFF))
				break;

			code = (uc - 0xD800) * 0x400 + (uc2 - 0xDC00) + 0x10000;
			dst[0] = (code >> 18) | 0xF0;
			dst[1] = ((code >> 12) & 0x3F) | 0x80;
			dst[2] = ((code >> 6) & 0x3F) | 0x80;
			dst[3] = (code & 0x3F) | 0x80;
			bytes = 4;
		} else if (uc <= 0x7F) {
			dst[0] = uc;
			bytes = 1;
		} else if ((uc > 0x7F) && (uc <= 0x7FF)) {
			if (dleft < 2)
				break;

			dst[0] = (uc >> 6) | 0xC0;
			dst[1] = (uc & 0x3F) | 0x80;
			bytes = 2;
		} else {
			if (dleft < 3)
				break;

			dst[0] = (uc >> 12) | 0xE0;
			dst[1] = ((uc >> 6) & 0x3F) | 0x80;
			dst[2] = (uc & 0x3F) | 0x80;
			bytes = 3;
		}

		src += 2;
		dst += bytes;
		sleft -= 2;
		dleft -= bytes;
	}

	*in_size  = *in_size - sleft;
	*out_size = *out_size - dleft;
	return SOPHON_OK;
}

static Sophon_Result
utf16_swap (const Sophon_U8 *in_buf,
			Sophon_U32 *in_size,
			Sophon_U8 *out_buf,
			Sophon_U32 *out_size)
{
	Sophon_U32 sleft = *in_size;
	Sophon_U32 dleft = *out_size;
	const Sophon_U8 *src = in_buf;
	Sophon_U8 *dst = out_buf;

	while ((sleft >= 2) && (dleft >= 2)) {
		dst[0] = src[1];
		dst[1] = src[0];

		src += 2;
		dst += 2;
		sleft -= 2;
		dleft -= 2;
	}

	*in_size  = *in_size - sleft;
	*out_size = *out_size - dleft;
	return SOPHON_OK;
}

#define SOPHON_CONV_CODE(from, to) (((from) << 16) | (to))

Sophon_Result
sophon_conv_create (Sophon_Conv *conv,
			Sophon_Encoding from,
			Sophon_Encoding to)
{
	Sophon_UIntPtr code;

	code = SOPHON_CONV_CODE(from, to);

	*conv = (Sophon_Ptr)code;

	return SOPHON_OK;
}

void
sophon_conv_destroy (Sophon_Conv conv)
{
}

Sophon_Result
sophon_conv (Sophon_Conv conv,
			const Sophon_U8 *in_buf,
			Sophon_U32 *in_size,
			Sophon_U8 *out_buf,
			Sophon_U32 *out_size)
{
	Sophon_UIntPtr code = (Sophon_UIntPtr)conv;
	Sophon_Result r;

	SOPHON_ASSERT(in_buf && in_size && out_buf && out_size);

	if ((code >> 16) == (code & 0xFFFF)) {
		Sophon_U32 size = SOPHON_MIN(*in_size, *out_size);

		if(in_buf != out_buf) {
			sophon_memcpy(out_buf, in_buf, size);
		}

		*out_size = size;
		return SOPHON_OK;
	}

	switch (code) {
		case SOPHON_CONV_CODE(SOPHON_UTF8, SOPHON_UTF16LE):
			r = utf8_to_utf16(in_buf, in_size, out_buf, out_size,
						(SOPHON_UTF16 != SOPHON_UTF16LE));
			break;
		case SOPHON_CONV_CODE(SOPHON_UTF16LE, SOPHON_UTF8):
			r = utf16_to_utf8(in_buf, in_size, out_buf, out_size,
						(SOPHON_UTF16 != SOPHON_UTF16LE));
			break;
		case SOPHON_CONV_CODE(SOPHON_UTF8, SOPHON_UTF16BE):
			r = utf8_to_utf16(in_buf, in_size, out_buf, out_size,
						(SOPHON_UTF16 != SOPHON_UTF16BE));
			break;
		case SOPHON_CONV_CODE(SOPHON_UTF16BE, SOPHON_UTF8):
			r = utf16_to_utf8(in_buf, in_size, out_buf, out_size,
						(SOPHON_UTF16 != SOPHON_UTF16BE));
			break;
		case SOPHON_CONV_CODE(SOPHON_UTF16LE, SOPHON_UTF16BE):
		case SOPHON_CONV_CODE(SOPHON_UTF16BE, SOPHON_UTF16LE):
			r = utf16_swap(in_buf, in_size, out_buf, out_size);
			break;
		default:
			return SOPHON_ERR_NOTSUPP;
	}

	return r;
}

