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

/*Example: Instruction file store and load*/

#include <sophonjs.h>

/*JS source code*/
static const char js[] = "console.log(\"hello, world!\")";
static int pos = 0;

/*JS source code input function*/
static Sophon_Int
input_func (Sophon_Ptr data, Sophon_U8 *buf, Sophon_Int size)
{
	char *src = (char*)data;
	int left = strlen(src) - pos;
	int cnt = (left < size) ? left : size;

	if (cnt)
		memcpy(buf, src + pos, cnt);

	pos += cnt;
	return cnt;
}

/*File output function*/
static Sophon_Int
file_output_func (Sophon_Ptr data, Sophon_U8 *buf, Sophon_Int size)
{
	FILE *fp = (FILE*)data;

	return fwrite(buf, 1, size, fp);
}

/*Store the JS instructions to a file*/
static void
ins_store()
{
	Sophon_VM *vm;
	Sophon_Module *mod;
	FILE *fp;
	Sophon_Result r;

	/*1. Create a sophon virtual machine*/
	vm = sophon_vm_create();

	/*2. Create a sophon module object*/
	mod = sophon_module_create(vm);

	/*3. Parse the JS source code and store the instructions to the module*/
	r = sophon_parse(vm,        /*Virtual machine*/
				mod,            /*Module to store the generated instructions*/
				SOPHON_UTF8,    /*Source code encoding*/
				input_func,     /*Source code input function*/
				(Sophon_Ptr)js, /*Input function's first parameter*/
				0);             /*Flags, 0*/

	/*4. Check parse result*/
	if (r != SOPHON_OK) {
		fprintf(stderr, "parse error\n");
		goto end;
	}

	/*5. Store the instructions to a file*/
	fp = fopen("example.si", "wb");
	sophon_insfile_store(vm,      /*Virtual machine*/
				mod,              /*Module*/
				file_output_func, /*Instruction output function*/
				(Sophon_Ptr)fp);  /*Output function's first parameter*/
	fclose(fp);

end:
	/*After all, release the sophon virtual machine*/
	sophon_vm_destroy(vm);
}

/*File input function*/
static Sophon_Int
file_input_func (Sophon_Ptr data, Sophon_U8 *buf, Sophon_Int size)
{
	FILE *fp = (FILE*)data;

	return fread(buf, 1, size, fp);
}

/*Load the JS instructions from a file*/
static void
ins_load()
{
	Sophon_VM *vm;
	Sophon_Module *mod;
	FILE *fp;
	Sophon_Result r;

	/*1. Create a sophon virtual machine*/
	vm = sophon_vm_create();

	/*2. Create a sophon module object*/
	mod = sophon_module_create(vm);

	/*3. Load instruction file*/
	fp = fopen("example.si", "rb");
	r = sophon_insfile_load(vm,  /*Virtual machine*/
				mod,             /*Module to store the instructions*/
				file_input_func, /*Instruction input function*/
				(Sophon_Ptr)fp); /*Input function's first parameter*/
	fclose(fp);

	/*4. Check load result*/
	if (r != SOPHON_OK) {
		fprintf(stderr, "load error\n");
		goto end;
	}

	/*5. Run the instructions*/
	r = sophon_module_call(vm, /*Virtual machine*/
				mod,           /*Module*/
				NULL,          /*Arguments array*/
				0,             /*Argument count*/
				NULL);         /*Return value*/
	if (r != SOPHON_OK) {
		fprintf(stderr, "exception\n");
		goto end;
	}

end:
	/*After all, release the sophon virtual machine*/
	sophon_vm_destroy(vm);
}

int
main (int argc, char **args)
{
	ins_store();

	ins_load();

	return 0;
}
