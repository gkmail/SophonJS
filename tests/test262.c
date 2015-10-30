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

#include <sophonjs.h>

#define TEST_PATH "tests/test262/"

static const char*
test_names[] = {
	"ch06",
	"ch07",
	"ch08",
	"ch09",
	"ch10",
	/*"ch11",
	"ch12",
	"ch13",
	"ch14",
	"ch15",
	"annexB",*/
	NULL
};

static Sophon_VM *vm;

static Sophon_U8
conv (Sophon_U8 c)
{
	if ((c >= 'A') && (c <= 'Z'))
		return c - 'A';
	if ((c >= 'a') && (c <= 'z'))
		return c - 'a' + 26;
	if ((c >= '0') && (c <= '9'))
		return c - '0' + 52;

	switch (c) {
		case '+':
			return 62;
		case '/':
			return 63;
		case '=':
			return 0;
		default:
			fprintf(stderr, "illegal code\n");
			break;
	}

	return 0;
}

static SOPHON_FUNC(decode)
{
	Sophon_String *str, *rstr;
	Sophon_Char *src, *ps;
	char *dst, *pd;
	Sophon_Int slen, dlen, sleft, dleft;
	Sophon_Result r;

	if ((r = sophon_value_to_string(vm, SOPHON_ARG(0), &str)) != SOPHON_OK)
		return r;

	src  = sophon_string_chars(vm, str);
	slen = sophon_string_length(vm, str);

	if (!slen) {
		sophon_value_set_string(vm, retv, vm->empty_str);
		return SOPHON_OK;
	}

	dlen = slen / 4 * 3;
	if (src[slen - 1] == '=') {
		if (src[slen - 2] == '=') {
			dlen -= 2;
		} else {
			dlen --;
		}
	}

	dst = (char*)malloc(dlen + 1);

	ps = src;
	pd = dst;
	sleft = slen;
	dleft = dlen;

	while (sleft > 0) {
		Sophon_U32 u32;

		if (*ps == 0)
			break;

		u32 = (conv(ps[0]) << 18) | (conv(ps[1]) << 12) | (conv(ps[2]) << 6) |
				conv(ps[3]);

		*pd++ = (u32 >> 16) & 0xFF;
		dleft--;

		if (dleft > 0) {
			*pd++ = (u32 >> 8) & 0xFF;
			dleft--;
		}

		if (dleft > 0) {
			*pd++ = u32 & 0xFF;
			dleft--;
		}

		ps += 4;
		sleft -= 4;
	}

	*pd = 0;
	/*printf("%s\n", dst);*/

	rstr = sophon_string_from_utf8_chars(vm, dst, dlen);

	free(dst);

	sophon_value_set_string(vm, retv, rstr);

	return SOPHON_OK;
}

static SOPHON_FUNC(dump)
{
	Sophon_String *str;
	char *cstr;
	Sophon_U32 len;
	Sophon_Result r;

	if ((r = sophon_value_to_string(vm, SOPHON_ARG(0), &str)) != SOPHON_OK)
		return r;

	sophon_string_new_utf8_cstr(vm, str, &cstr, &len);
	printf("test case:\n");
	printf("%s\n", cstr);
	sophon_string_free_utf8_cstr(vm, cstr, len);

	return SOPHON_OK;
}

static const Sophon_Decl
decls[] = {
	SOPHON_FUNCTION_PROP(decode, 0, decode, 1),
	SOPHON_FUNCTION_PROP(dump, 0, dump, 1),
	{NULL}
};

static Sophon_Int
file_input (Sophon_Ptr data, Sophon_U8 *buf, Sophon_Int size)
{
	FILE *fp = (FILE*)data;

	return fread(buf, 1, size, fp);
}

static Sophon_Result
run_json (Sophon_Value jsonv)
{
	Sophon_String *name;
	Sophon_Value retv;
	Sophon_Result r;

	name = sophon_string_from_utf8_cstr(vm, "myTest262");
	name = sophon_string_intern(vm, name);

	r = sophon_value_call_prop(vm, vm->glob_module->globv,
				SOPHON_VALUE_GC(name),
				&jsonv, 1, &retv, 0);

	return r;
}

static Sophon_Result
load (const char *name)
{
	char nbuf[256];
	FILE *fp;
	Sophon_Value v;
	Sophon_Result r;

	sprintf(nbuf, "%s%s.js", TEST_PATH, name);
	if (!(fp = fopen(nbuf, "r"))) {
		fprintf(stderr, "cannot find \"%s\"\n", name);
		return -1;
	}

	r = sophon_eval(vm, SOPHON_UTF8, file_input, (Sophon_Ptr)fp, 0, &v);

	fclose(fp);

	if (r != SOPHON_OK)
		fprintf(stderr, "load \"%s\" failed\n", name);

	return r;
}

static Sophon_Result
run_test (const char *name)
{
	char nbuf[256];
	FILE *fp;
	Sophon_Value v;
	Sophon_Result r;

	sprintf(nbuf, "%s%s.json", TEST_PATH, name);

	if (!(fp = fopen(nbuf, "r"))) {
		fprintf(stderr, "cannot find \"%s\"\n", name);
		return -1;
	}

	r = sophon_json_parse(vm, SOPHON_UTF8, file_input, (Sophon_Ptr)fp,
				&v, NULL);
	if (r != SOPHON_OK) {
		fprintf(stderr, "\"%s\" parse error\n", name);
	}

	fclose(fp);

	if (r == SOPHON_OK) {
		printf("run test \"%s\"\n", name);
		r = run_json(v);
	}

	return r;
}

int
main (int argc, char **args)
{
	const char **pn;
	Sophon_Result r;

	vm = sophon_vm_create();
	sophon_decl_load(vm, NULL, NULL, decls);

	if ((r = load("sta")) != SOPHON_OK)
		goto end;
	if ((r = load("main")) != SOPHON_OK)
		goto end;

	pn = test_names;
	while (*pn) {
		if (run_test(*pn) != SOPHON_OK) {
			r = -1;
			break;
		}
		pn++;
	}

end:
	sophon_vm_destroy(vm);

	return (r == SOPHON_OK) ? 0 : 1;
}

