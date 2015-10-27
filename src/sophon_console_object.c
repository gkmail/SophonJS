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

#define CONSOLE_FUNC_PROP(name, argc)\
	SOPHON_FUNCTION_PROP(name, GLOBAL_ATTRS, console_##name##_func, argc)
#define CONSOLE_FUNC(name)\
	SOPHON_FUNC(console_##name##_func)

enum {
	CONSOLE_LOG,
	CONSOLE_DEBUG,
	CONSOLE_INFO,
	CONSOLE_WARN,
	CONSOLE_ERROR,
	CONSOLE_ASSERT
};

#define console_pr sophon_pr

#ifdef SOPHON_HAVE_TIME
static Sophon_Int
console_now ()
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
#endif

static void
console_chars (const Sophon_Char *chars, Sophon_U32 len)
{
#ifdef SOPHON_8BITS_CHAR
	char buf[len+1];

	sophon_memcpy(buf, chars, len);
	buf[len] = 0;
#else
	char buf[len * 3 + 1];
	Sophon_Conv conv;
	Sophon_U32 in_len, out_len;
	Sophon_Result r;

	sophon_conv_create(&conv, SOPHON_UTF16, SOPHON_UTF8);

	in_len  = len * sizeof(Sophon_Char);
	out_len = sizeof(buf);

	r = sophon_conv(conv, (Sophon_U8*)chars, &in_len,
				(Sophon_U8*)buf, &out_len);
	if (r != SOPHON_OK)
		return;

	buf[out_len] = 0;

	sophon_conv_destroy(conv);
#endif

	console_pr("%s", buf);
}

static void
console_str (Sophon_VM *vm, Sophon_String *str)
{
	Sophon_Char *chars;
	Sophon_U32 len;

	chars = sophon_string_chars(vm, str);
	len   = sophon_string_length(vm, str);

	console_chars(chars, len);
}

static void
console_value (Sophon_VM *vm, Sophon_Value v)
{
	Sophon_Value jsonv, ostrv;
	Sophon_String *ostr;
	Sophon_Result r;

	if ((r = sophon_value_get(vm, vm->glob_module->globv,
				SOPHON_VALUE_GC(vm->JSON_str),
				&jsonv, 0)) != SOPHON_OK)
		return;
	if ((r = sophon_value_call_prop(vm, jsonv,
				SOPHON_VALUE_GC(vm->stringify_str),
				&v, 1,
				&ostrv, 0)) != SOPHON_OK)
		return;

	if ((r = sophon_value_to_string(vm, ostrv, &ostr))
				!= SOPHON_OK)
		return;

	console_str(vm, ostr);
}

static Sophon_Result
console_output (Sophon_VM *vm, int type, Sophon_Value thisv,
			Sophon_Value *argv, Sophon_Int argc, Sophon_Value *retv)
{
	Sophon_String *str;
	const Sophon_Char *chars, *cptr, *cend, *cnext;
	Sophon_U32 len, aid;
	const char *tag;
	Sophon_Result r;

	switch (type) {
		default:
		case CONSOLE_LOG:
			tag = "LOG";
			break;
		case CONSOLE_DEBUG:
			tag = "DEBUG";
			break;
		case CONSOLE_INFO:
			tag = "INFO";
			break;
		case CONSOLE_WARN:
			tag = "WARN";
			break;
		case CONSOLE_ERROR:
			tag = "ERROR";
			break;
		case CONSOLE_ASSERT:
			tag = "ASSERT";
			break;
	}

	console_pr("%s: ", tag);

	if ((r = sophon_value_to_string(vm, SOPHON_ARG(0), &str)) != SOPHON_OK)
		return r;

	chars = sophon_string_chars(vm, str);
	len   = sophon_string_length(vm, str);

	aid  = 1;
	cptr = chars;
	cend = chars + len;
	while (cptr < cend) {
		Sophon_U32 cnt;
		Sophon_Value v;

		cnext = sophon_ucstrchr(cptr, '%');
		if (cnext) {
			cnt = cnext - cptr;
		} else {
			cnt = cend - cptr;
		}

		if (cnt) {
			console_chars(cptr, cnt);
		}

		if (cnext) {
			switch (cnext[1]) {
				case 'd':
				case 'i': {
					Sophon_Int i;

					cptr = cnext + 2;
					v = SOPHON_ARG(aid);
					aid++;
					if ((r = sophon_value_to_int(vm, v, &i)) != SOPHON_OK)
						return r;
					console_pr("%d", i);
					break;
				}
				case 's': {
					Sophon_String *astr;

					cptr = cnext + 2;
					v = SOPHON_ARG(aid);
					aid++;

					if ((r = sophon_value_to_string(vm, v, &astr))
								!= SOPHON_OK)
						return r;
					console_str(vm, astr);
					break;
				}
				case 'f': {
					Sophon_Double d;

					cptr = cnext + 2;
					v = SOPHON_ARG(aid);
					aid++;

					if ((r = sophon_value_to_number(vm, v, &d)) != SOPHON_OK)
						return r;
					console_pr("%e", d);
					break;
				}
				case 'o': {
					cptr = cnext + 2;
					v = SOPHON_ARG(aid);
					aid++;

					console_pr("object ");
					console_value(vm, v);
					break;
				}
				default:
					console_chars(cnext, 1);
					cptr = cnext + 1;
					break;
			}
		} else {
			break;
		}
	}

	console_pr("\n");
	return SOPHON_OK;
}

static CONSOLE_FUNC(log)
{
	return console_output(vm, CONSOLE_LOG, thisv, argv, argc, retv);
}

static CONSOLE_FUNC(debug)
{
	return console_output(vm, CONSOLE_DEBUG, thisv, argv, argc, retv);
}

static CONSOLE_FUNC(info)
{
	return console_output(vm, CONSOLE_INFO, thisv, argv, argc, retv);
}

static CONSOLE_FUNC(warn)
{
	return console_output(vm, CONSOLE_WARN, thisv, argv, argc, retv);
}

static CONSOLE_FUNC(error)
{
	return console_output(vm, CONSOLE_ERROR, thisv, argv, argc, retv);
}

static CONSOLE_FUNC(assert)
{
	Sophon_Bool b;

	b = sophon_value_to_bool(vm, SOPHON_ARG(0));

	if (!b)
		return console_output(vm, CONSOLE_ASSERT, thisv,
					argv + 1, argc - 1, retv);
	else
		return SOPHON_OK;
}

static CONSOLE_FUNC(dir)
{
	console_value(vm, SOPHON_ARG(0));
	return SOPHON_OK;
}

static CONSOLE_FUNC(trace)
{
	sophon_trace(vm);
	return SOPHON_OK;
}

static CONSOLE_FUNC(time)
{
	Sophon_String *str;
	Sophon_HashEntry *ent;
	Sophon_Result r;

	if ((r = sophon_value_to_string(vm, SOPHON_ARG(0), &str)) != SOPHON_OK)
		return r;

	r = sophon_hash_add(vm, &vm->timer_hash,
				sophon_string_key,
				sophon_string_equal,
				(Sophon_Ptr)str,
				&ent);
	if (r < 0)
		return r;

	console_pr("TIMER: ");
	console_str(vm, str);
	console_pr(" start\n");

	ent->value = (Sophon_Ptr)(Sophon_IntPtr)console_now();
	return SOPHON_OK;
}

static CONSOLE_FUNC(timeEnd)
{
	Sophon_String *str;
	Sophon_HashEntry *ent;
	Sophon_Int start;
	Sophon_Result r;

	if ((r = sophon_value_to_string(vm, SOPHON_ARG(0), &str)) != SOPHON_OK)
		return r;

	r = sophon_hash_lookup(vm, &vm->timer_hash,
				sophon_string_key,
				sophon_string_equal,
				(Sophon_Ptr)str,
				&ent);
	if (r < 0)
		return r;
	if (r == 0)
		return SOPHON_OK;

	start = (Sophon_IntPtr)ent->value;

	console_pr("TIMER: ");
	console_str(vm, str);
	console_pr(" end %d ms\n", console_now() - start);

	sophon_hash_remove(vm, &vm->timer_hash,
				sophon_string_key,
				sophon_string_equal,
				(Sophon_Ptr)str);
	return SOPHON_OK;
}

static const Sophon_Decl
console_object_props[] = {
	CONSOLE_FUNC_PROP(log, 1),
	CONSOLE_FUNC_PROP(debug, 1),
	CONSOLE_FUNC_PROP(info, 1),
	CONSOLE_FUNC_PROP(warn, 1),
	CONSOLE_FUNC_PROP(error, 1),
	CONSOLE_FUNC_PROP(assert, 1),
	CONSOLE_FUNC_PROP(dir, 1),
	CONSOLE_FUNC_PROP(trace, 0),
	CONSOLE_FUNC_PROP(time, 1),
	CONSOLE_FUNC_PROP(timeEnd, 1),
	{NULL}
};

