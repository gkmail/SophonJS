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
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static FILE *fp;
static char line[1024];
static unsigned int last = 0;
static unsigned int range_begin = 0;
static unsigned int code;
static int count = 0;

static void
dump_range ()
{
	if (count != 0)
		printf(",");

	if ((count % 8) == 0)
		printf("\n\t");
	else
		printf(" ");

	if ((range_begin != 0) && (range_begin != last)) {
		printf("0x%08x", (last << 16) | range_begin);
		range_begin = code;
	} else {
		printf("0x%08x", (last << 16) | last);
	}

	count++;
	range_begin = code;
}

int
main (int argc, char **argv)
{
	int i;
	
	assert(argc > 2);

	fp = fopen("tools/UnicodeData.txt", "r");

	printf("#include <sophon_types.h>\n\n");
	printf("static const Sophon_U32 %s_table[] = {", argv[1]);

	while (1) {
		char *begin;
		char *ptr;
		char *type;

		if (!fgets(line, sizeof(line), fp))
			break;

		begin = line;
		ptr = strchr(begin, ';');
		assert(ptr);

		*ptr = 0;
		code = strtol(begin, NULL, 16);

		begin = ptr + 1;
		ptr = strchr(begin, ';');
		assert(ptr);

		begin = ptr + 1;
		ptr = strchr(begin, ';');
		assert(ptr);

		type = begin;
		*ptr = 0;

		for (i = 0; i < argc; i++){
			if (!strcmp(argv[i], type)) {

				if ((last != 0) && ((code - last) > 1)) {
					dump_range();
				}

				if (range_begin == 0)
					range_begin = code;

				last = code;
			}
		}
	}

	if (range_begin ) {
		dump_range();
	}

	printf("\n};\n");

	fclose(fp);

	return 0;
}

