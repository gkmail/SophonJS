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

#include <sophon_util.h>

#ifdef SOPHON_HAVE_FPRINTF

void
sophon_prerr(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

void
sophon_vprerr(const char *fmt, va_list ap)
{
	vfprintf(stderr, fmt, ap);
}

#else

#error not "fprintf" defined

#endif /*SOPHON_HAVE_FPRITNF*/

void
sophon_warning(const char *fmt, ...)
{
	va_list ap;

	sophon_prerr("Warning: ");

	va_start(ap, fmt);
	sophon_vprerr(fmt, ap);
	va_end(ap);

	sophon_prerr("\n");
}

void
sophon_error(const char *fmt, ...)
{
	va_list ap;

	sophon_prerr("Error: ");

	va_start(ap, fmt);
	sophon_vprerr(fmt, ap);
	va_end(ap);

	sophon_prerr("\n");
}

void
sophon_fatal(const char *fmt, ...)
{
	va_list ap;

	sophon_prerr("Fatal: ");

	va_start(ap, fmt);
	sophon_vprerr(fmt, ap);
	va_end(ap);

	sophon_prerr("\n");

#ifdef SOPHON_HAVE_ABORT
	abort();
#elif defined(SOPHON_HAVE_EXIT)
	exit(1);
#endif
}

