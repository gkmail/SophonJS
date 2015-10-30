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

#ifndef _JS_KEYWORDS_H_
#define _JS_KEYWORDS_H_

#ifdef __cplusplus
extern "C" {
#endif

#define FOR_EACH_KEYWORD(t) \
	t(null)\
	t(true)\
	t(false)\
	t(break)\
	t(do)\
	t(in)\
	t(typeof)\
	t(case)\
	t(else)\
	t(instanceof)\
	t(var)\
	t(catch)\
	t(export)\
	t(new)\
	t(void)\
	t(class)\
	t(extends)\
	t(return)\
	t(while)\
	t(const)\
	t(finally)\
	t(super)\
	t(with)\
	t(continue)\
	t(for)\
	t(switch)\
	t(yield)\
	t(debugger)\
	t(function)\
	t(this)\
	t(default)\
	t(if)\
	t(throw)\
	t(delete)\
	t(import)\
	t(try)\
	t(enum)\
	t(await)\
	t(get)\
	t(set)\
	t(public)\
	t(interface)\
	t(implements)

#ifdef __cplusplus
}
#endif

#endif

