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
#include <sophon_string.h>
#include <sophon_vm.h>
#include <sophon_debug.h>

const static Sophon_Double pow_table[] = {
    10.,
    100.,
    1.0e4,
    1.0e8,
    1.0e16,
    1.0e32,
    1.0e64,
    1.0e128,
    1.0e256
};

const static Sophon_Double neg_pow_table[] = {
    1.0e-1,
    1.0e-2,
    1.0e-4,
    1.0e-8,
    1.0e-16,
    1.0e-32,
    1.0e-64,
    1.0e-128,
    1.0e-256
};

static Sophon_Int
calc_isize (Sophon_Double d, Sophon_Double *dr)
{
	Sophon_Int min, max, mid;
	Sophon_Int pos;
	Sophon_Int size = 0;

	pos = SOPHON_ARRAY_SIZE(pow_table) - 1;

	while (d > 10.) {
		min = 0;
		max = pos + 1;

		while (1) {
			mid = (max + min) >> 1;

			if (mid == min) {
				pos = min;
				break;
			}

			if (pow_table[mid] == d) {
				pos = mid;
				break;
			}

			if (pow_table[mid] < d)
				min = mid;
			else
				max = mid;
		}

		d /= pow_table[pos];
		size += (1 << pos);
	}

	if (dr)
		*dr = d;

	return size + 1;
}

static Sophon_Int
calc_esize (Sophon_Double d, Sophon_Double *dr)
{
	Sophon_Int min, max, mid;
	Sophon_Int pos;
	Sophon_Int size = 0;

	pos = SOPHON_ARRAY_SIZE(pow_table) - 1;

	while (d < 1.0) {

		min = 0;
		max = pos + 1;

		while (1) {
			mid = (max + min) >> 1;

			if (mid == min) {
				pos = min;
				break;
			}

			if (neg_pow_table[mid] == d) {
				pos = mid;
				break;
			}

			if (neg_pow_table[mid] < d)
				max = mid;
			else
				min = mid;
		}

		d *= pow_table[pos];
		size += (1 << pos);
	}

	if (dr)
		*dr = d;

	return size;
}

static Sophon_String*
dtostr_special (Sophon_VM *vm, Sophon_Double d)
{
	if (sophon_isnan(d)) {
		return vm->NaN_str;
	}

	if (sophon_isinf(d)) {
		if (d > 0) {
			return vm->Infinity_str;
		} else {
			return sophon_string_from_utf8_cstr(vm, "-Infinity");
		}
	}

	return NULL;
}

static Sophon_String*
dtostr_fstr (Sophon_VM *vm, Sophon_Double d, Sophon_Int radix,
			Sophon_Int sign, Sophon_Int isize, Sophon_Int fsize)
{
	Sophon_String *str;
	Sophon_Int size, n;
	Sophon_Char *chars, *ptr, *iptr;
	Sophon_Double dt;

	if (isize == 0)
		isize++;
	size = sign + isize + fsize;
	if (fsize)
		size++;

	str   = sophon_string_from_chars(vm, NULL, size);
	chars = sophon_string_chars(vm, str);

	iptr = chars;
	if (sign)
		*iptr++ = '-';

	dt  = d;

	if (dt >= 1) {
		ptr = iptr + isize;

		while (dt >= 1.) {
			n = sophon_fmod(dt, (Sophon_Double)radix);
			ptr--;

			if (n > 9) {
				*ptr = n - 10 + 'a';
			} else {
				*ptr = n + '0';
			}

			dt /= (Sophon_Double)radix;
		}
	} else {
		*iptr = '0';
	}

	iptr += isize;

	if (fsize) {
		*iptr++ = '.';

		ptr = iptr;
		dt = d - sophon_floor(d);

		while (fsize--) {
			dt *= radix;
			n = ((Sophon_Int)sophon_floor(dt)) % radix;

			if (n > 9) {
				*ptr = n - 10 + 'a';
			} else {
				*ptr = n + '0';
			}

			ptr++;
		}
	}

	return str;
}

static Sophon_String*
dtostr_estr (Sophon_VM *vm, Sophon_Double d, Sophon_Int f,
			Sophon_Int sign, Sophon_Int esign, Sophon_Int exp)
{
	Sophon_String *str;
	Sophon_Char *chars, *ptr;
	Sophon_Int esize, size;
	Sophon_Double dt;
	Sophon_Int n;

	if (exp > 99)
		esize = 3;
	else if (exp > 9)
		esize = 2;
	else
		esize = 1;

	size = sign + 2 + esign + esize;
	if (f)
		size += f + 1;

	str = sophon_string_from_chars(vm, NULL, size);
	chars = sophon_string_chars(vm, str);

	ptr = chars;
	if (sign)
		*ptr++ = '-';

	dt = d;
	n = sophon_fmod(dt, 10.);
	*ptr++ = n + '0';

	if (f) {
		*ptr++ = '.';

		while (f--) {
			dt *= 10.;
			n = ((Sophon_Int)sophon_floor(dt)) % 10;
			*ptr++ = n + '0';
		}
	}

	*ptr++ = 'e';
	if (esign)
		*ptr++ = '-';

	if (exp > 99)
		*ptr++ = (exp / 100) + '0';

	if (exp > 9)
		*ptr++ = ((exp / 10) % 10) + '0';

	*ptr++ = (exp % 10) + '0';

	return str;
}

static Sophon_String*
dtostr_exp (Sophon_VM *vm, Sophon_Double d, Sophon_Int f)
{
	Sophon_Int sign, esign, exp;
	Sophon_Double d_abs, dr;

	if (d < 0) {
		d_abs = -d;
		sign  = 1;
	} else {
		d_abs = d;
		sign  = 0;
	}

	if (d_abs < 1.0) {
		esign = 1;
		exp = calc_esize(d_abs, &dr);
	} else {
		esign = 0;
		exp = calc_isize(d_abs, &dr) - 1;
	}

	if (f < 0)
		f = 6;

	return dtostr_estr(vm, dr, f, sign, esign, exp);
}

static Sophon_String*
dtostr_fixed (Sophon_VM *vm, Sophon_Double d, Sophon_Int f)
{
	Sophon_Double d_abs;
	Sophon_Int sign, isize;

	if (SOPHON_ABS(d) >= 1e21) {
		return dtostr_exp(vm, d, -1);
	}

	if (d < 0) {
		d_abs = -d;
		sign  = 1;
	} else {
		d_abs = d;
		sign  = 0;
	}

	isize = calc_isize(d_abs, NULL);

	return dtostr_fstr(vm, d_abs, 10, sign, isize, f);
}

static Sophon_String*
dtostr_prec (Sophon_VM *vm, Sophon_Double d, Sophon_Int f)
{
	Sophon_U32 sign, esign, exp;
	Sophon_Double d_abs, dr;

	if (d < 0) {
		sign  = 1;
		d_abs = -d;
	} else {
		sign  = 0;
		d_abs = d;
	}

	if (d_abs < 1.0) {
		esign = 1;
		exp = calc_esize(d_abs, &dr);

		if (exp < 6)
			return dtostr_fstr(vm, d_abs, 10, sign, 1, f + exp - 1);
	} else {
		esign = 0;
		exp = calc_isize(d_abs, &dr) - 1;

		if (exp < f)
			return dtostr_fstr(vm, d_abs, 10, sign, exp + 1, f - exp - 1);
	}

	return dtostr_estr(vm, dr, f - 1, sign, esign, exp);
}

static Sophon_String*
dtostr_radix (Sophon_VM *vm, Sophon_Double d, Sophon_Int radix)
{
	Sophon_U32 sign, isize, fsize, prec;
	Sophon_Double d_abs, dt;

	if (radix == 0)
		radix = 10;

	SOPHON_ASSERT((radix >= 2) && (radix <= 36));

	if (d < 0) {
		sign  = 1;
		d_abs = -d;
	} else {
		sign  = 0;
		d_abs = d;
	}

	dt = d_abs;
	isize = 0;
	while (dt >= 1.) {
		isize++;
		dt /= (Sophon_Double)radix;
	}

	dt = d_abs - sophon_floor(d_abs);
	fsize = 0;
	prec  = 1;
	while (dt != sophon_floor(dt)) {
		dt *= (Sophon_Double)radix;

		if (sophon_floor(dt) > 0)
			prec *= radix;

		if (prec > 1000000)
			break;

		fsize++;
	}

	return dtostr_fstr(vm, d_abs, radix, sign, isize, fsize);
}

Sophon_String*
sophon_dtostr (Sophon_VM *vm, Sophon_Double d, Sophon_D2StrMode mode,
			Sophon_Int param)
{
	Sophon_String *str = NULL;

	if ((str = dtostr_special(vm, d)))
		return str;

	switch (mode) {
		case SOPHON_D2STR_RADIX:
			str = dtostr_radix(vm, d, param);
			break;
		case SOPHON_D2STR_FIXED:
			str = dtostr_fixed(vm, d, param);
			break;
		case SOPHON_D2STR_EXP:
			str = dtostr_exp(vm, d, param);
			break;
		case SOPHON_D2STR_PREC:
			str = dtostr_prec(vm, d, param);
			break;
		default:
			SOPHON_ASSERT(0);
	}

	return str;
}

