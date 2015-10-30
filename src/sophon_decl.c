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
#include <sophon_string.h>
#include <sophon_decl.h>
#include <sophon_object.h>
#include <sophon_module.h>
#include <sophon_function.h>
#include <sophon_closure.h>
#include <sophon_debug.h>

Sophon_Closure*
sophon_closure_load (Sophon_VM *vm, Sophon_Module *mod, Sophon_String *name,
			Sophon_NativeFunc nf, Sophon_U8 argc)
{
	Sophon_Int func_id;
	Sophon_Function *func;

	SOPHON_ASSERT(vm && nf);

	if (!mod)
		mod = vm->glob_module;

	func_id = sophon_module_add_func(vm, mod, name,
				SOPHON_FUNC_FL_NATIVE);
	func = sophon_module_get_func(mod, func_id);
	func->f.native = nf;
	func->argc = argc;

	return  sophon_closure_create(vm, func);
}

Sophon_Result
sophon_decl_load (Sophon_VM *vm, Sophon_Module *mod, Sophon_Object *obj,
			const Sophon_Decl *decls)
{
	const Sophon_Decl *decl = decls;
	Sophon_Result r;

	SOPHON_ASSERT(vm && decls);

	if (!mod) {
		mod = vm->glob_module;

		if (!obj) {
			if ((r = sophon_value_to_object(vm, mod->globv, &obj))
						!= SOPHON_OK)
				return r;
		}
	}

	if (!decl)
		return SOPHON_OK;

	while (decl->name) {
		Sophon_String *name;
		Sophon_String *str;
		Sophon_Closure *clos;
		Sophon_Object *cobj;
		Sophon_Value getv = SOPHON_VALUE_UNDEFINED;
		Sophon_Value setv = SOPHON_VALUE_UNDEFINED;
		Sophon_NativeFunc nf;
		Sophon_U8 attrs = 0;
		Sophon_U32 flags = 0;

		name = sophon_string_from_utf8_cstr(vm, decl->name);
		name = sophon_string_intern(vm, name);

		attrs = decl->attrs;

		switch (decl->type) {
			case SOPHON_DECL_UNDEFINED:
				sophon_value_set_undefined(vm, &getv);
				flags = SOPHON_FL_DATA_PROP;
				break;
			case SOPHON_DECL_NULL:
				sophon_value_set_null(vm, &getv);
				flags = SOPHON_FL_DATA_PROP;
				break;
			case SOPHON_DECL_BOOL:
				sophon_value_set_bool(vm, &getv,
							SOPHON_DECL_GET_BOOL(decl));
				flags = SOPHON_FL_DATA_PROP;
				break;
			case SOPHON_DECL_INT:
				sophon_value_set_int(vm, &getv,
							SOPHON_DECL_GET_INT(decl));
				flags = SOPHON_FL_DATA_PROP;
				break;
			case SOPHON_DECL_DOUBLE:
				sophon_value_set_number(vm, &getv,
							SOPHON_DECL_GET_DOUBLE(decl));
				flags = SOPHON_FL_DATA_PROP;
				break;
			case SOPHON_DECL_STRING:
				str = sophon_string_from_utf8_cstr(vm,
							SOPHON_DECL_GET_STRING(decl));
				sophon_value_set_string(vm, &getv, str);
				flags = SOPHON_FL_DATA_PROP;
				break;
			case SOPHON_DECL_FUNCTION:
				nf = SOPHON_DECL_GET_FUNCTION(decl);
				if (nf) {
					clos = sophon_closure_load(vm, mod, name, nf,
								SOPHON_DECL_GET_ARGC(decl));
					sophon_value_set_closure(vm, &getv, clos);
				} else {
					sophon_value_set_undefined(vm, &getv);
				}
				flags = SOPHON_FL_DATA_PROP;
				break;
			case SOPHON_DECL_ACCESSOR:
				nf = SOPHON_DECL_GET_GET(decl);
				if (nf) {
					clos = sophon_closure_load(vm, mod, name, nf, 0);
					sophon_value_set_closure(vm, &getv, clos);
				} else {
					sophon_value_set_undefined(vm, &getv);
				}

				nf = SOPHON_DECL_GET_SET(decl);
				if (nf) {
					clos = sophon_closure_load(vm, mod, name, nf, 1);
					sophon_value_set_closure(vm, &setv, clos);
				} else {
					sophon_value_set_undefined(vm, &setv);
				}

				flags = SOPHON_FL_ACCESSOR_PROP;
				break;
			case SOPHON_DECL_OBJECT:
				cobj = sophon_object_create(vm);
				sophon_decl_load(vm, mod, cobj, SOPHON_DECL_GET_OBJECT(decl));
				sophon_value_set_object(vm, &getv, cobj);
				flags = SOPHON_FL_DATA_PROP;
				break;
		}

		if (name == vm->__call_str) {
			obj->primv = getv;
		} else {
			sophon_value_define_prop(vm, SOPHON_VALUE_GC(obj),
						SOPHON_VALUE_GC(name), getv, setv, attrs, flags);
		}

		decl++;
	}

	return SOPHON_OK;
}

