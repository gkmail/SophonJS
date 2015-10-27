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

/*Example: Extend native functions*/

#include <sophonjs.h>

/*JS source code*/
static const char js[] =
"console.log(\"mybool: %s\", mybool);\n"
"console.log(\"myint: %d\", myint);\n"
"console.log(\"mypi: %f\", mypi);\n"
"console.log(\"mystr: %s\", mystr);\n"
"console.log(\"myfunc(100): %d\", myfunc(100));\n"
"myaccessor=19491009;\n"
"console.log(\"myaccessor: %d\", myaccessor);\n"
"console.log(\"myobj.prop: %d\", myobj.prop);\n"
;
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

/*Native function*/
static SOPHON_FUNC(myfunc)
{
	Sophon_Int n, i, sum;
	Sophon_Result r;

	/*Convert argument from JS type to native type*/
	if ((r = sophon_value_to_int(vm, SOPHON_ARG(0), &n)) != SOPHON_OK)
		return r;

	/*Do something*/
	for (i = 1, sum = 0; i <= n; i++)
		sum += i;

	/*Convert return value from native type to JS type*/
	sophon_value_set_int(vm, retv, sum);

	return SOPHON_OK;
}

static int native_int_value = 0;

/*Native accessor get function*/
static SOPHON_FUNC(myget)
{
	/*Convert native type to JS type*/
	sophon_value_set_int(vm, retv, native_int_value);
	return SOPHON_OK;
}

/*Native accessor set function*/
static SOPHON_FUNC(myset)
{
	Sophon_Int i;
	Sophon_Result r;

	/*Convert argument from JS type to native type*/
	if ((r = sophon_value_to_int(vm, SOPHON_ARG(0), &i)) != SOPHON_OK)
		return r;

	native_int_value = i;
	return SOPHON_OK;
}

#define ATTRS\
	(SOPHON_PROP_ATTR_WRITABLE|SOPHON_PROP_ATTR_ENUMERABLE|\
	 SOPHON_PROP_ATTR_CONFIGURABLE)

/*Native functions and properties declaration*/
static const Sophon_Decl
myobj_prop_decls[] = {
	/*Object's property*/
	SOPHON_INT_PROP(prop, ATTRS, 12345678),

	/*End*/
	{NULL}
};

static const Sophon_Decl
decls[] = {
	/*Boolean property: name, attributes, value*/
	SOPHON_BOOL_PROP(mybool, ATTRS, SOPHON_TRUE),

	/*Integer property: name, attributes, value*/
	SOPHON_INT_PROP(myint, ATTRS, 19781009),

	/*Double precision property: name, attributes, value*/
	SOPHON_DOUBLE_PROP(mypi, ATTRS, 3.1415926),

	/*String property: name, attributes, value*/
	SOPHON_STRING_PROP(mystr, ATTRS, "sophon js"),

	/*Function property: name, attributes, native function, arguments count*/
	SOPHON_FUNCTION_PROP(myfunc, ATTRS, myfunc, 1),

	/*Accessor property: name, attributes, get function, set function*/
	SOPHON_ACCESSOR_PROP(myaccessor, ATTRS, myget, myset),

	/*Object property: name, attributes, object's properties*/
	SOPHON_OBJECT_PROP(myobj, ATTRS, myobj_prop_decls),

	/*End*/
	{NULL}
};

int
main (int argc, char **args)
{
	Sophon_VM *vm;
	Sophon_Module *mod;
	Sophon_Result r;

	/*1. Create a sophon virtual machine*/
	vm = sophon_vm_create();

	/*2. Extend native functions*/
	sophon_decl_load(vm,        /*Virtual machine*/
				NULL,           /*Module to store the native data.
								  NULL is the global module.*/
				NULL,           /*Object to store the native data.
								  NULL is the global object of the module*/
				decls);

	/*3. Create a sophon module object*/
	mod = sophon_module_create(vm);

	/*4. Parse the JS source code and store the instructions to the module*/
	r = sophon_parse(vm,        /*Virtual machine*/
				mod,            /*Module to store the generated instructions*/
				SOPHON_UTF8,    /*Source code encoding*/
				input_func,     /*Source code input function*/
				(Sophon_Ptr)js, /*Input function's first parameter*/
				0);             /*Flags, 0*/

	/*5. Check parse result*/
	if (r != SOPHON_OK) {
		fprintf(stderr, "parse error\n");
		goto end;
	}

	/*6. Run the instructions*/
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

	return 0;
}
