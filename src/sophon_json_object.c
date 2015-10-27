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

#include <sophon_json.h>

#define JSON_FUNC_PROP(name, argc)\
	SOPHON_FUNCTION_PROP(name, GLOBAL_ATTRS, json_##name##_func, argc)
#define JSON_FUNC(name)\
	SOPHON_FUNC(json_##name##_func)

static JSON_FUNC(parse)
{
	StrInputParams inp;
	Sophon_JSONParseParams p;
	Sophon_String *str;
	Sophon_Result r;

	if ((r = sophon_value_to_string(vm, SOPHON_ARG(0), &str)) != SOPHON_OK)
		return r;

	p.reviver = SOPHON_ARG(1);

	inp.chars = sophon_string_chars(vm, str);
	inp.len   = sophon_string_length(vm, str);
	inp.off   = 0;

	return sophon_json_parse(vm, SOPHON_ENC, str_input, &inp, retv, &p);
}

static JSON_FUNC(stringify)
{
	Sophon_Value v = SOPHON_ARG(0);
	Sophon_Value rep = SOPHON_ARG(1);
	Sophon_Value space = SOPHON_ARG(2);
	Sophon_JSONStringifyParams p;
	Sophon_String *str;
	Sophon_Result r;

	p.whitelist = NULL;
	p.space     = NULL;
	sophon_value_set_undefined(vm, &p.replacer);

	if (!sophon_value_is_undefined(rep)) {
		if (sophon_value_is_array(rep)) {
			if ((r = sophon_value_to_array(vm, rep, &p.whitelist))
						!= SOPHON_OK)
				return r;
		} else {
			p.replacer = rep;
		}
	}

	if (!sophon_value_is_undefined(space)) {
		if (sophon_value_is_number(space)) {
			Sophon_Int i;

			if ((r = sophon_value_to_int(vm, space, &i)) != SOPHON_OK)
				return r;

			if (i > 0) {
				Sophon_Char *ptr, *pend;

				i = SOPHON_MIN(i, 10);
				p.space = sophon_string_from_chars(vm, NULL, 10);

				ptr  = sophon_string_chars(vm, p.space);
				pend = ptr + sophon_string_length(vm, p.space);
				while (ptr < pend) {
					*ptr++ = 0x20;
				}
			}
		} else if (sophon_value_is_string(space)) {
			Sophon_String *str;
			Sophon_Char *chars;
			Sophon_S32 len;

			if ((r = sophon_value_to_string(vm, space, &str)) != SOPHON_OK)
				return r;

			chars = sophon_string_chars(vm, str);
			len = sophon_string_length(vm, str);
			if (len <= 10) {
				p.space = str;
			} else {
				len = SOPHON_MIN(len, 10);
				p.space = sophon_string_from_chars(vm, chars, len);
			}
		}
	}

	str = sophon_json_stringify(vm, v, &p);
	if (!str)
		return SOPHON_ERR_TYPE;

	sophon_value_set_string(vm, retv, str);
	return SOPHON_OK;
}

static const Sophon_Decl
json_object_props[] = {
	JSON_FUNC_PROP(parse, 1),
	JSON_FUNC_PROP(stringify, 1),
	{NULL}
};

