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

#define REGEXP_FUNC_PROP(name, argc)\
	SOPHON_FUNCTION_PROP(name, GLOBAL_ATTRS, regexp_##name##_func, argc)
#define REGEXP_FUNC(name)\
	SOPHON_FUNC(regexp_##name##_func)

static REGEXP_FUNC(call)
{
	Sophon_Value patternv = SOPHON_ARG(0);
	Sophon_Value flagsv = SOPHON_ARG(1);
	Sophon_String *pstr;
	Sophon_String *fstr;
	Sophon_U32 flags = 0;
	Sophon_RegExp *re;
	Sophon_Result r;

	if (sophon_value_is_regexp(patternv)) {
		if (sophon_value_is_undefined(flagsv)) {
			*retv = patternv;
			return SOPHON_OK;
		} else {
			sophon_throw(vm, vm->TypeError, "\"flags\" is not undefined");
			return SOPHON_ERR_THROW;
		}
	}

	if (sophon_value_is_undefined(patternv)) {
		pstr = vm->empty_str;
	} else {
		if ((r = sophon_value_to_string(vm, patternv, &pstr)) != SOPHON_OK)
			return r;
	}

	if (!sophon_value_is_undefined(flagsv)) {
		Sophon_Char *ch;

		if ((r = sophon_value_to_string(vm, flagsv, &fstr)) != SOPHON_OK)
			return r;

		ch = sophon_string_chars(vm, fstr);
		while (*ch) {
			switch (*ch) {
				case 'i':
					if (flags & SOPHON_REGEXP_FL_I)
						goto syntax_error;
					flags |= SOPHON_REGEXP_FL_I;
					break;
				case 'm':
					if (flags & SOPHON_REGEXP_FL_M)
						goto syntax_error;
					flags |= SOPHON_REGEXP_FL_M;
					break;
				case 'g':
					if (flags & SOPHON_REGEXP_FL_G)
						goto syntax_error;
					flags |= SOPHON_REGEXP_FL_G;
					break;
				default:
					goto syntax_error;
			}
			ch++;
		}
	}

	if (!(re = sophon_regexp_create(vm, pstr, flags)))
		goto syntax_error;

	sophon_value_set_regexp(vm, retv, re);
	return SOPHON_OK;
syntax_error:
	sophon_throw(vm, vm->SyntaxError, "Syntax error");
	return SOPHON_ERR_THROW;
}

static REGEXP_FUNC(source_get)
{
	Sophon_RegExp *re;
	Sophon_Result r;

	if ((r = sophon_value_to_regexp(vm, thisv, &re)) != SOPHON_OK)
		return r;

	sophon_value_set_string(vm, retv, re->str);
	return SOPHON_OK;
}

#define REGEXP_FLAG_GET_FUNC(name, flag)\
static REGEXP_FUNC(name##_get)\
{\
	Sophon_RegExp *re;\
	Sophon_Result r;\
	Sophon_Bool b;\
	if ((r = sophon_value_to_regexp(vm, thisv, &re)) != SOPHON_OK)\
		return r;\
	b = re->flags & SOPHON_REGEXP_FL_##flag;\
	sophon_value_set_bool(vm, retv, b);\
	return SOPHON_OK;\
}

REGEXP_FLAG_GET_FUNC(global, G)
REGEXP_FLAG_GET_FUNC(ignoreCase, I)
REGEXP_FLAG_GET_FUNC(multiline, M)

static REGEXP_FUNC(lastIndex_get)
{
	Sophon_RegExp *re;
	Sophon_Result r;

	if ((r = sophon_value_to_regexp(vm, thisv, &re)) != SOPHON_OK)
		return r;

	sophon_value_set_int(vm, retv, re->last);
	return SOPHON_OK;
}

static REGEXP_FUNC(lastIndex_set)
{
	Sophon_RegExp *re;
	Sophon_Int i;
	Sophon_Result r;

	if ((r = sophon_value_to_regexp(vm, thisv, &re)) != SOPHON_OK)
		return r;

	if ((r = sophon_value_to_int(vm, SOPHON_ARG(0), &i)) != SOPHON_OK)
		return r;

	re->last = i;
	return SOPHON_OK;
}

static REGEXP_FUNC(toString)
{
	Sophon_RegExp *re;
	Sophon_String *str;
	Sophon_Result r;

	if ((r = sophon_value_to_regexp(vm, thisv, &re)) != SOPHON_OK)
		return r;

	str = vm->div_str;
	str = sophon_string_concat(vm, str, re->str);
	str = sophon_string_concat(vm, str, vm->div_str);

	if (re->flags) {
		Sophon_Char buf[16];
		Sophon_Char *ch = buf;
		Sophon_String *fstr;

		if (re->flags & SOPHON_REGEXP_FL_I)
			*ch++ = 'i';
		if (re->flags & SOPHON_REGEXP_FL_G)
			*ch++ = 'g';
		if (re->flags & SOPHON_REGEXP_FL_M)
			*ch++ = 'm';

		*ch = 0;

		fstr = sophon_string_create(vm, buf);
		str = sophon_string_concat(vm, str, fstr);
	}

	sophon_value_set_string(vm, retv, str);
	return SOPHON_OK;
}

static REGEXP_FUNC(test)
{
	Sophon_RegExp *re;
	Sophon_String *str;
	Sophon_Bool b;
	Sophon_U32 len;
	Sophon_ReMatch match;
	Sophon_Result r;

	if ((r = sophon_value_to_regexp(vm, thisv, &re)) != SOPHON_OK)
		return r;

	if ((r = sophon_value_to_string(vm, SOPHON_ARG(0), &str)) != SOPHON_OK)
		return r;

	len  = sophon_string_length(vm, str);
	if ((re->last < 0) || (re->last > len))
		re->last = 0;

	if ((r = sophon_regexp_match(vm, re, str, re->last, &match, 1)) < 0)
		return r;

	if (r == 0) {
		re->last = 0;
	} else {
		Sophon_Int last = match.end - sophon_string_chars(vm, str);

		if (last == re->last)
			re->last++;
		else
			re->last = last;
	}

	b = (r != SOPHON_NONE);
	
	sophon_value_set_bool(vm, retv, b);
	return SOPHON_OK;
}

static REGEXP_FUNC(exec)
{
	Sophon_RegExp *re;
	Sophon_String *str;
	Sophon_Int mcnt;
	Sophon_U32 len;
	Sophon_Char *chars;
	Sophon_Result r;

	if ((r = sophon_value_to_regexp(vm, thisv, &re)) != SOPHON_OK)
		return r;

	if ((r = sophon_value_to_string(vm, SOPHON_ARG(0), &str)) != SOPHON_OK)
		return r;

	chars = sophon_string_chars(vm, str);
	len   = sophon_string_length(vm, str);

	if ((re->last < 0) || (re->last > len))
		re->last = 0;

	mcnt = sophon_regexp_match_size(re);

	{
		Sophon_ReMatch match[mcnt];
		Sophon_Array *arr;
		Sophon_String *mstr;
		Sophon_Value arrv, iv;
		Sophon_Int i, last;

		if ((r = sophon_regexp_match(vm, re, str, re->last, match, mcnt)) < 0)
			return r;

		if (r == SOPHON_NONE) {
			re->last = 0;
			sophon_value_set_null(vm, retv);
			return SOPHON_OK;
		}

		last = match[0].end - chars;
		if (last == re->last)
			re->last++;
		else
			re->last = last;

		arr = sophon_array_create(vm);
		sophon_array_set_length(vm, arr, mcnt);

		for (i = 0; i < mcnt; i++) {
			Sophon_Value itemv;

			if (match[i].begin) {
				mstr = sophon_string_from_chars(vm, match[i].begin,
							match[i].end - match[i].begin);
				sophon_value_set_string(vm, &itemv, mstr);
			} else {
				sophon_value_set_undefined(vm, &itemv);
			}

			sophon_array_set_item(vm, arr, i, itemv);
		}

		sophon_value_set_array(vm, &arrv, arr);
		sophon_value_define_prop(vm, arrv,
					SOPHON_VALUE_GC(vm->input_str),
					SOPHON_VALUE_GC(str),
					SOPHON_VALUE_UNDEFINED,
					SOPHON_DATA_PROP_ATTR,
					SOPHON_FL_DATA_PROP);

		sophon_value_set_int(vm, &iv, match[0].begin - chars);
		sophon_value_define_prop(vm, arrv,
					SOPHON_VALUE_GC(vm->index_str),
					iv,
					SOPHON_VALUE_UNDEFINED,
					SOPHON_DATA_PROP_ATTR,
					SOPHON_FL_DATA_PROP);

		*retv = arrv;
	}

	return SOPHON_OK;
}

static const Sophon_Decl
regexp_prototype_props[] = {
	SOPHON_STRING_PROP([Class], 0, "RegExp"),
	SOPHON_ACCESSOR_PROP(source, 0, regexp_source_get_func, NULL),
	SOPHON_ACCESSOR_PROP(global, 0, regexp_global_get_func, NULL),
	SOPHON_ACCESSOR_PROP(ignoreCase, 0, regexp_ignoreCase_get_func, NULL),
	SOPHON_ACCESSOR_PROP(multiline, 0, regexp_multiline_get_func, NULL),
	SOPHON_ACCESSOR_PROP(lastIndex, SOPHON_PROP_ATTR_WRITABLE,
				regexp_lastIndex_get_func,
				regexp_lastIndex_set_func),
	REGEXP_FUNC_PROP(toString, 0),
	REGEXP_FUNC_PROP(test, 1),
	REGEXP_FUNC_PROP(exec, 1),
	{NULL}
};

static const Sophon_Decl
regexp_object_props[] = {
	SOPHON_OBJECT_PROP(prototype, 0, regexp_prototype_props),
	SOPHON_FUNCTION_PROP([Call], GLOBAL_ATTRS, regexp_call_func, 2),
	{NULL}
};

