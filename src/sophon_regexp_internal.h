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

#ifndef _SOPHON_REGEXP_INTERNAL_H_
#define _SOPHON_REGEXP_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sophon_regexp.h"

typedef struct ReData_s   ReData;
typedef struct ReNode_s   ReNode;
typedef struct ReIns_s    ReIns;
typedef struct ReInput_s  ReInput;
typedef struct ReBranch_s ReBranch;
typedef struct ReCtxt_s   ReCtxt;

typedef enum {
	N_LA,
	N_NLA,
	N_ANY,
	N_CHAR,
	N_CLASS,
	N_NCLASS,
	N_BOL,
	N_EOL,
	N_B,
	N_NB,
	N_REF,
	N_CAP,
	N_ALTER,
	N_REPEAT
} ReNodeType;

struct ReInput_s {
	ReNode           *root;
	ReNode           *nodes;
	Sophon_CharRange *cr_buf;
	Sophon_U32        cr_len;
	Sophon_U32        cr_cap;
	Sophon_Char      *chars;
	Sophon_U32        len;
	Sophon_U32        pos;
	Sophon_U8         cap_begin;
	Sophon_U8         cap_end;
	Sophon_U8         loop_level;
	Sophon_U8         loop_depth;
};

struct ReNode_s {
	ReNodeType       type;
	ReNode          *alloc_next;
	ReNode          *prev;
	ReNode          *next;
	union {
		Sophon_Char     ch;
		Sophon_U8       ref;
		ReNode         *la;
		struct {
			ReNode     *node;
			Sophon_U32  id;
		} capture;
		struct {
			Sophon_U32  begin;
			Sophon_U32  end;
		} clazz;
		struct {
			ReNode     *body;
			Sophon_Int  min;
			Sophon_Int  max;
			Sophon_U32  isize;
			Sophon_Bool lazy;
			Sophon_U8   cap;
			Sophon_U8   loop;
		} repeat;
		struct {
			ReNode     *left;
			ReNode     *right;
		} alter;
	} n;
};

typedef enum {
	I_LA,
	I_NLA,
	I_ANY,
	I_CHAR,
	I_CLASS,
	I_NCLASS,
	I_BOL,
	I_EOL,
	I_B,
	I_NB,
	I_REF,
	I_CAPB,
	I_CAPE,
	I_BRANCH,
	I_JMP,
	I_CLEAR,
	I_EMPTY,
	I_END
} ReInsType;

struct ReIns_s {
	ReInsType  type;
	union {
		Sophon_Char     ch;
		Sophon_U8       ref;
		Sophon_U8       cap;
		ReIns          *jmp;
		struct {
			Sophon_U32  begin;
			Sophon_U32  end;
		} clazz;
		struct {
			ReIns      *b1;
			ReIns      *b2;
		} branch;
		struct {
			ReIns      *jmp;
			Sophon_U8   cap;
		} empty;
	} i;
};

struct ReData_s {
	Sophon_U32          ibuf_size;
	ReIns              *ibuf;
	Sophon_U32          cr_size;
	Sophon_CharRange   *cr_buf;
	Sophon_U8           cap_count;
	Sophon_U8           loop_count;
};

struct ReBranch_s {
	Sophon_Char        *ch;
	ReIns              *ins;
	Sophon_ReMatch      match[1];
};

struct ReCtxt_s {
	Sophon_Char        *chars;
	Sophon_U32          start;
	Sophon_U32          len;
	Sophon_CharRange   *cr_buf;
	Sophon_U32          flags;
	Sophon_U32          br_count;
	Sophon_U32          br_cap;
	ReBranch           *branch;
	Sophon_U8           cap_count;
	Sophon_U8           loop_count;
};

#ifdef __cplusplus
}
#endif

#endif

