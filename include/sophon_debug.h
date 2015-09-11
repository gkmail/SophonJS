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

#ifndef _SOPHON_DEBUG_H_
#define _SOPHON_DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sophon_types.h"

#if defined(SOPHON_HAVE_ASSERT) && defined(SOPHON_DEBUG_INFO)
	#define SOPHON_ASSERT(a) assert(a)
#else
	#define SOPHON_ASSERT(a)
#endif

#ifdef SOPHON_DEBUG
	#ifdef SOPHON_HAVE_FILE_LINE
		#define SOPHON_INFO(a) \
				SOPHON_MACRO_BEGIN \
					sophon_prerr("I/\"%s\" %d: ", __FILE__, __LINE__); \
					sophon_prerr a; \
					sophon_prerr("\n"); \
				SOPHON_MACRO_END
		#define SOPHON_ERROR(a) \
				SOPHON_MACRO_BEGIN \
					sophon_prerr("E/\"%s\" %d: ", __FILE__, __LINE__); \
					sophon_prerr a; \
					sophon_prerr("\n"); \
				SOPHON_MACRO_END
		#define SOPHON_WARNING(a) \
				SOPHON_MACRO_BEGIN \
					sophon_prerr("W/\"%s\" %d: ", __FILE__, __LINE__); \
					sophon_prerr a; \
					sophon_prerr("\n"); \
				SOPHON_MACRO_END
	#else
		#define SOPHON_INFO(a) \
				SOPHON_MACRO_BEGIN \
					sophon_prerr("I/: "); \
					sophon_prerr a; \
					sophon_prerr("\n"); \
				SOPHON_MACRO_END
		#define SOPHON_ERROR(a) \
				SOPHON_MACRO_BEGIN \
					sophon_prerr("E/: "); \
					sophon_prerr a; \
					sophon_prerr("\n"); \
				SOPHON_MACRO_END
		#define SOPHON_WARNING(a) \
				SOPHON_MACRO_BEGIN \
					sophon_prerr("W/: "); \
					sophon_prerr a; \
					sophon_prerr("\n"); \
				SOPHON_MACRO_END
	#endif
#else
	#define SOPHON_INFO(a)
	#define SOPHON_ERROR(a)
	#define SOPHON_WARNING(a)
#endif /*SOPHON_DEBUG*/

#ifdef __cplusplus
}
#endif

#endif

