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

#ifndef _GNU_CONFIG_H_
#define _GNU_CONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <ctype.h>
#include <wctype.h>
#include <limits.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>

#define SOPHON_INLINE inline

#define SOPHON_HAVE_FILE_LINE 1
#define SOPHON_HAVE_REALLOC 1
#define SOPHON_HAVE_ASSERT  1
#define SOPHON_HAVE_MEMSET  1
#define SOPHON_HAVE_MEMCPY  1
#define SOPHON_HAVE_MEMMOVE 1
#define SOPHON_HAVE_FPRINTF 1
#define SOPHON_HAVE_ABORT   1
#define SOPHON_HAVE_EXIT    1
#define SOPHON_HAVE_STRLEN  1
#define SOPHON_HAVE_STRCMP  1
#define SOPHON_HAVE_STRCASECMP  1
#define SOPHON_HAVE_STRNCMP  1
#define SOPHON_HAVE_STRNCASECMP  1
#define SOPHON_HAVE_STRSTR  1
#define SOPHON_HAVE_STRCHR  1
#define SOPHON_HAVE_MATH    1
#define SOPHON_HAVE_DRAND48 1
#define SOPHON_HAVE_RAND    1
#define SOPHON_HAVE_TOWUPPER 1
#define SOPHON_HAVE_TOWLOWER 1
#define SOPHON_HAVE_ISWSPACE 1
#define SOPHON_HAVE_TIME     1
#define SOPHON_HAVE_GNUC_EXT 1

#endif

