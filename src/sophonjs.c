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

#include <stdio.h>
#include <sophonjs.h>

/*Work mode*/
static enum {
	MODE_RUN_JS,
	MODE_STORE_INS,
	MODE_LOAD_INS
} mode = MODE_RUN_JS;

/*Sophon resource*/
static Sophon_VM *vm;
static char **js_args;
static int    js_argc;

/*Input file*/
static char      *input_filename = NULL;
static FILE      *input_fp = NULL;

/*Instruction file name*/
static char      *ins_filename = NULL;
static FILE      *ins_fp = NULL;

static Sophon_Int
file_input_func (Sophon_Ptr data, Sophon_U8 *buf, Sophon_Int size)
{
	FILE *fp = (FILE*)data;

	return fread(buf, 1, size, fp);
}

static Sophon_Int
file_output_func (Sophon_Ptr data, Sophon_U8 *buf, Sophon_Int size)
{
	FILE *fp = (FILE*)data;

	return fwrite(buf, 1, size, fp);
}

/*Input buffer*/
#define INPUT_BUF_SIZE 4096
static char       input_buf[INPUT_BUF_SIZE];
static Sophon_U32 input_pos = 0;
static Sophon_U32 input_len = 0;

static Sophon_Int
buf_input_func (Sophon_Ptr data, Sophon_U8 *buf, Sophon_Int size)
{
	int cnt;

	if (!buf) {
		sophon_pr("   ");
		if (fgets(input_buf, sizeof(input_buf), stdin)) {
			input_pos = 0;
			input_len = sophon_strlen(input_buf);
			return 0;
		}
	}

	if (input_pos >= input_len)
		return 0;

	cnt = SOPHON_MIN(size, input_len);
	sophon_memcpy(buf, input_buf + input_pos, cnt);
	input_pos += cnt;
	input_len -= cnt;

	return cnt;
}

/*Output usage*/
static const char*
usage_str =
"usage: sophonjs [OPTIONS] [FILE]\n"
"Sophon Javascript Engine\n"
"option:\n"
"\t-o FILE     store module to an instruction file\n"
"\t-i FILE     load module from an instruction file\n"
"\t-v          output version number\n"
;

static void
usage (void)
{
	sophon_pr("%s", usage_str);
}

static void
version (void)
{
	sophon_pr("%s\n", SOPHON_VERSION);
}

/*Parse arguments*/
static void
parse_args (int argc, char **argv)
{
	char *arg;
	int i;

	for (i = 1; i < argc; i++) {
		arg = argv[i];

		if (arg[0] == '-') {
			char *pname = arg + 1;

			if (!strcmp(pname, "")) {
				input_fp = stdin;
				break;
			} else if (!strcmp(pname, "o")) {
				if (i + 1 >= argc)
					goto error;

				mode = MODE_STORE_INS;
				ins_filename = argv[i + 1];
				i++;
			} else if (!strcmp(pname, "i")) {
				if (i + 1 >= argc)
					goto error;

				mode = MODE_LOAD_INS;
				ins_filename = argv[i + 1];
				i++;
				break;
			} else if (!strcmp(pname, "help")) {
				usage();
				exit(0);
			} else if (!strcmp(pname, "v")) {
				version();
				exit(0);
			} else {
				usage();
				exit(1);
			}
		} else {
			input_filename = arg;
			break;
		}
	}

	js_args = argv + i;
	js_argc = argc - i;

	return;

error:
	fprintf(stderr, "illegal options\n");
	usage();
	exit(1);
}

/*Run a javascript file*/
static Sophon_Result
js_file (void)
{
	Sophon_Value *argv = NULL;
	Sophon_Module *mod;
	Sophon_String *str;
	Sophon_Result r;
	int size;
	int i;

	/*Load arguments*/
	size = SOPHON_MAX(1, js_argc);
	argv = sophon_mm_alloc_ensure(vm,
				sizeof(Sophon_Value) * size);

	if (js_argc > 0) {
		str = sophon_string_from_utf8_cstr(vm, js_args[0]);
		sophon_value_set_gc(vm, &argv[0], (Sophon_GCObject*)str);
	} else {
		sophon_value_set_gc(vm, &argv[0], (Sophon_GCObject*)vm->empty_str);
	}

	for (i = 1; i < js_argc; i++) {

		str = sophon_string_from_utf8_cstr(vm, js_args[i]);
		sophon_value_set_gc(vm, &argv[i], (Sophon_GCObject*)str);
	}

	/*Create module*/
	mod = sophon_module_create(vm);

	if ((mode != MODE_LOAD_INS) && input_filename) {
		Sophon_String *name;

		name = sophon_string_from_utf8_cstr(vm, input_filename);
		sophon_module_set_name(mod, name);
	}

	/*Parse the input file*/
	if (mode == MODE_LOAD_INS) {
		r = sophon_insfile_load(vm, mod, file_input_func,
					(Sophon_Ptr)ins_fp);
	} else {
		r = sophon_parse(vm, mod, SOPHON_UTF8, file_input_func,
					(Sophon_Ptr)input_fp, SOPHON_PARSER_FL_EVAL);
	}

	if (r == SOPHON_OK) {
		Sophon_Value retv;

		if (mode == MODE_STORE_INS) {
			r = sophon_insfile_store(vm, mod, file_output_func,
						(Sophon_Ptr)ins_fp);
		} else {
			r = sophon_module_call(vm, mod, argv, js_argc, &retv);
		}
	}

	/*Free buffers*/
	if (argv) {
		sophon_mm_free(vm, argv,
					sizeof(Sophon_Value) * size);
	}

	return r;
}

/*Run a javascript shell*/
static Sophon_Result
js_shell (void)
{
	Sophon_Result r;
	Sophon_Value retv;

	while (1) {
		sophon_pr(">> ");
		if (fgets(input_buf, sizeof(input_buf), stdin)) {
			if (!strcmp(input_buf, "quit\n"))
				break;

			input_pos = 0;
			input_len = sophon_strlen(input_buf);

			r = sophon_eval(vm, SOPHON_UTF8, buf_input_func,
						NULL, SOPHON_PARSER_FL_SHELL, &retv);
			if (r == SOPHON_OK) {
				sophon_value_dump(vm, retv);
				sophon_pr("\n");
			} else {
				sophon_catch(vm, NULL);
			}
		}
	}

	return SOPHON_OK;
}

int
main (int argc, char **argv)
{
	Sophon_Result r;

	vm = sophon_vm_create();

	parse_args(argc, argv);

	if (input_fp || input_filename || ins_filename) {
		if (input_filename) {
			input_fp = fopen(input_filename, "rb");
		}

		if (mode == MODE_LOAD_INS) {
			ins_fp = fopen(ins_filename, "rb");
		} else if (mode == MODE_STORE_INS) {
			ins_fp = fopen(ins_filename, "wb");
		}

		r = js_file();

		if (input_fp && input_fp != stdin)
			fclose(input_fp);
		if (ins_fp)
			fclose(ins_fp);
	} else {
		r = js_shell();
	}

	sophon_vm_destroy(vm);

	return (r == SOPHON_OK) ? 0 : 1;
}

