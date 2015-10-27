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

/*Example: Shell mode*/

#include <sophonjs.h>

/*Command input buffer*/
#define INPUT_BUF_SIZE 4096
static char       input_buf[INPUT_BUF_SIZE];
static Sophon_U32 input_pos = 0;
static Sophon_U32 input_len = 0;

/*Command input function*/
static Sophon_Int
buf_input_func (Sophon_Ptr data, Sophon_U8 *buf, Sophon_Int size)
{
	int cnt;

	/* If the command is not end, parser invoke the input function with buf == NULL.
	 * When buf == NULL, read the next line as the input code.
	 */
	if (!buf) {
		printf("   ");
		if (fgets(input_buf, sizeof(input_buf), stdin)) {
			input_pos = 0;
			input_len = strlen(input_buf);
			return 0;
		}
	}

	/*Copy the command to the parser buffer*/
	if (input_pos >= input_len)
		return 0;

	cnt = SOPHON_MIN(size, input_len);
	sophon_memcpy(buf, input_buf + input_pos, cnt);
	input_pos += cnt;
	input_len -= cnt;

	return cnt;
}

int
main (int argc, char **args)
{
	Sophon_VM *vm;
	Sophon_Value retv;
	Sophon_Result r;

	/*1. Create a sophon virtual machine*/
	vm = sophon_vm_create();

	/*2. Enter shell mode loop*/
	while (1) {

		/*3. Read command*/
		printf(">> ");
		if (fgets(input_buf, sizeof(input_buf), stdin)) {
			/*Quit when input "quit"*/
			if (!strcmp(input_buf, "quit\n"))
				break;

			input_pos = 0;
			input_len = strlen(input_buf);

			/*4. Parse and run the command*/
			r = sophon_eval(vm,
						SOPHON_UTF8,
						buf_input_func,
						NULL,
						SOPHON_PARSER_FL_SHELL,
						&retv);
			if (r == SOPHON_OK) {
				/*Dump the return value */
				sophon_value_dump(vm, retv);
				printf("\n");
			} else {
				/*Catch exceptions*/
				sophon_catch(vm, NULL);
			}
		}
	}

	/*After all, release the sophon virtual machine*/
	sophon_vm_destroy(vm);

	return 0;
}
