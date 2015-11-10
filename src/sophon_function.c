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

#include <sophon_types.h>
#include <sophon_mm.h>
#include <sophon_util.h>
#include <sophon_vm.h>
#include <sophon_function.h>
#include <sophon_object.h>
#include <sophon_string.h>
#include <sophon_debug.h>

Sophon_Result
sophon_function_add_var (Sophon_VM *vm, Sophon_Function *func,
			Sophon_FuncVarType type, Sophon_String *name,
			Sophon_U32 flags)
{
	Sophon_Result r;
	Sophon_UIntPtr id;
	Sophon_HashEntry *ent;

	SOPHON_ASSERT(vm && func && name);

	r = sophon_hash_add(vm, &func->var_hash,
				sophon_direct_key,
				sophon_direct_equal,
				(Sophon_Ptr)name,
				&ent);
	if ((r == SOPHON_OK) ||
				((r == SOPHON_NONE) && (flags & SOPHON_FL_FORCE))) {
		if (type == SOPHON_FUNC_VAR) {
			id = func->varc++;
		} else {
			id = func->argc++ | SOPHON_VAR_FL_ARG;
		}

		ent->value = (Sophon_Ptr)id;
	}

	return r;
}

Sophon_Int
sophon_function_lookup_var (Sophon_VM *vm, Sophon_Function *func,
			Sophon_String *name)
{
	Sophon_Result r;
	Sophon_HashEntry *ent;

	SOPHON_ASSERT(vm && func && name);

	r = sophon_hash_lookup(vm, &func->var_hash,
				sophon_direct_key,
				sophon_direct_equal,
				(Sophon_Ptr)name,
				&ent);
	if (r == SOPHON_OK) {
		return (Sophon_UIntPtr)ent->value;
	} else if (r == SOPHON_NONE) {
		return SOPHON_ERR_NOTEXIST;
	}

	return r;
}

