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
#include <sophon_function.h>
#include <sophon_closure.h>
#include <sophon_module.h>
#include <sophon_insfile.h>
#include <sophon_debug.h>

#define SOPHON_INSFILE_MAGIC   0x534F4A53

#ifndef SOPHON_INSFILE_VERSION
	#define SOPHON_INSFILE_VERSION 0x00000001
#endif

enum {
	TAG_UNDEF,
	TAG_NULL,
	TAG_BOOL,
	TAG_INT,
	TAG_DOUBLE,
	TAG_STRING,
	TAG_INTERN_STRING
};

static void
put_u8 (Sophon_IOFunc func, Sophon_Ptr data, Sophon_U8 u8)
{
	func(data, &u8, 1);
}

static Sophon_Result
get_u8 (Sophon_IOFunc func, Sophon_Ptr data, Sophon_U8 *u8)
{
	if (func(data, u8, 1) != 1)
		return SOPHON_ERR_EOF;
	return SOPHON_OK;
}

static void
put_u16 (Sophon_IOFunc func, Sophon_Ptr data, Sophon_U16 u16)
{
	Sophon_U8 buf[2];

	buf[0] = u16 >> 8;
	buf[1] = u16 & 0xFF;

	func(data, buf, 2);
}

static Sophon_Result
get_u16 (Sophon_IOFunc func, Sophon_Ptr data, Sophon_U16 *u16)
{
	Sophon_U8 buf[2];

	if (func(data, buf, 2) != 2)
		return SOPHON_ERR_EOF;

	*u16 = (buf[0] << 8) | buf[1];
	return SOPHON_OK;
}

static void
put_u32 (Sophon_IOFunc func, Sophon_Ptr data, Sophon_U32 u32)
{
	Sophon_U8 buf[4];

	buf[0] = u32 >> 24;
	buf[1] = u32 >> 16;
	buf[2] = u32 >> 8;
	buf[3] = u32 & 0xFF;

	func(data, buf, 4);
}

static Sophon_Result
get_u32 (Sophon_IOFunc func, Sophon_Ptr data, Sophon_U32 *u32)
{
	Sophon_U8 buf[4];

	if (func(data, buf, 4) != 4)
		return SOPHON_ERR_EOF;

	*u32 = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
	return SOPHON_OK;
}

static void
put_double (Sophon_IOFunc func, Sophon_Ptr data, Sophon_Double d)
{
	Sophon_U8 *p = (Sophon_U8*)(&d);
	Sophon_U8 buf[8];

#ifdef SOPHON_LITTLE_ENDIAN
	buf[0] = p[7];
	buf[1] = p[6];
	buf[2] = p[5];
	buf[3] = p[4];
	buf[4] = p[3];
	buf[5] = p[2];
	buf[6] = p[1];
	buf[7] = p[0];
#else
	sophon_memcpy(buf, p, sizeof(buf));
#endif

	func(data, buf, 8);
}

static Sophon_Result
get_double (Sophon_IOFunc func, Sophon_Ptr data, Sophon_Double *d)
{
	Sophon_U8 *p;
	Sophon_U8 buf[8];

	if (func(data, buf, 8) != 8)
		return SOPHON_ERR_EOF;

	p = (Sophon_U8*)d;
#ifdef SOPHON_LITTLE_ENDIAN
	p[0] = buf[7];
	p[1] = buf[6];
	p[2] = buf[5];
	p[3] = buf[4];
	p[4] = buf[3];
	p[5] = buf[2];
	p[6] = buf[1];
	p[7] = buf[0];
#else
	sophon_memcpy(p, buf, sizeof(buf));
#endif

	return SOPHON_OK;
}

static void
put_str (Sophon_VM *vm, Sophon_IOFunc func, Sophon_Ptr data,
			Sophon_String *str)
{
	Sophon_Char *cstr;
	Sophon_U32 len;
	Sophon_U16 c;

	cstr = sophon_string_chars(vm, str);
	len  = sophon_string_length(vm, str);

	put_u32(func, data, len);
	while (len--) {
		c = *cstr++;
		put_u16(func, data, c);
	}
}

static Sophon_Result
get_str (Sophon_VM *vm, Sophon_IOFunc func, Sophon_Ptr data,
			Sophon_Bool can_be_empty, Sophon_String **pstr)
{
	Sophon_U32 len;
	Sophon_U16 ch;
	Sophon_String *str;
	Sophon_Char *ptr;
	Sophon_Result r;

	if ((r = get_u32(func, data, &len)) != SOPHON_OK)
		return r;

	if (!len && can_be_empty) {
		*pstr = NULL;
		return SOPHON_OK;
	}

	str = sophon_string_from_chars(vm, NULL, len);
	ptr = sophon_string_chars(vm, str);
	while (len--) {
		if ((r = get_u16(func, data, &ch)) != SOPHON_OK)
			return r;
		*ptr++ = ch;
	}

	*pstr = str;
	return SOPHON_OK;
}

static void
put_value (Sophon_VM *vm, Sophon_IOFunc func, Sophon_Ptr data,
			Sophon_Value v)
{
	if (sophon_value_is_undefined(v)) {
		put_u8(func, data, TAG_UNDEF);
	} else if (sophon_value_is_null(v)) {
		put_u8(func, data, TAG_NULL);
	} else if (sophon_value_is_bool(v)) {
		Sophon_Bool b;

		b = sophon_value_to_bool(vm, v);
		put_u8(func, data, TAG_BOOL);
		put_u8(func, data, b ? 1 : 0);
	} else if (sophon_value_is_string(v)) {
		Sophon_String *str;

		sophon_value_to_string(vm, v, &str);
		put_u8(func, data, (str->gc_flags & SOPHON_GC_FL_INTERN) ?
					TAG_INTERN_STRING : TAG_STRING);
		put_str(vm, func, data, str);
	} else if (sophon_value_is_number(v)) {
		Sophon_Number n;

		sophon_value_to_number(vm, v, &n);

		if (n == (Sophon_Number)(Sophon_Int)(n)) {
			put_u8(func, data, TAG_INT);
			put_u32(func, data, (Sophon_Int)n);
		} else {
			put_u8(func, data, TAG_DOUBLE);
			put_double(func, data, n);
		}
	} else {
		SOPHON_ASSERT(0);
	}
}

static Sophon_Result
get_value (Sophon_VM *vm, Sophon_IOFunc func, Sophon_Ptr data,
			Sophon_Value *v)
{
	Sophon_U8 tag, u8;
	Sophon_U32 u32;
	Sophon_Double d;
	Sophon_String *str;
	Sophon_Result r;

	if ((r = get_u8(func, data, &tag)) != SOPHON_OK)
		return r;

	switch (tag) {
		case TAG_UNDEF:
			sophon_value_set_undefined(vm, v);
			break;
		case TAG_NULL:
			sophon_value_set_null(vm, v);
			break;
		case TAG_BOOL:
			if ((r = get_u8(func, data, &u8)) != SOPHON_OK)
				return r;
			sophon_value_set_bool(vm, v, u8);
			break;
		case TAG_INT:
			if ((r = get_u32(func, data, &u32)) != SOPHON_OK)
				return r;
			sophon_value_set_int(vm, v, u32);
			break;
		case TAG_DOUBLE:
			if ((r = get_double(func, data, &d)) != SOPHON_OK)
				return r;
			sophon_value_set_number(vm, v, d);
			break;
		case TAG_STRING:
			if ((r = get_str(vm, func, data, SOPHON_FALSE, &str)) != SOPHON_OK)
				return r;
			sophon_value_set_string(vm, v, str);
			break;
		case TAG_INTERN_STRING:
			if ((r = get_str(vm, func, data, SOPHON_FALSE, &str)) != SOPHON_OK)
				return r;
			str = sophon_string_intern(vm, str);
			sophon_value_set_string(vm, v, str);
			break;
		default:
			SOPHON_ERROR(("illegal value tag"));
			return SOPHON_ERR_PARSE;
	}

	return SOPHON_OK;
}

typedef struct {
	Sophon_IOFunc func;
	Sophon_Ptr    data;
} StoreVarParams;

static void
store_var (Sophon_VM *vm, Sophon_HashEntry *ent, Sophon_Ptr ptr)
{
	StoreVarParams *p = (StoreVarParams*)ptr;
	Sophon_String *name = (Sophon_String*)ent->key;
	Sophon_U16 id = (Sophon_U16)(Sophon_UIntPtr)ent->value;

	put_str(vm, p->func, p->data, name);
	put_u16(p->func, p->data, id);
}

static void
put_func (Sophon_VM *vm, Sophon_IOFunc func, Sophon_Ptr data,
			Sophon_Function *f)
{
	StoreVarParams svp;
	Sophon_U16 i;

	SOPHON_ASSERT(!(f->flags & SOPHON_FUNC_FL_NATIVE));

	if (f->name)
		put_str(vm, func, data, f->name);
	else
		put_u32(func, data, 0);

	put_u8(func, data, f->flags);
	put_u8(func, data, f->argc);
	put_u16(func, data, f->varc);
	put_u16(func, data, f->stack_size);
	put_u16(func, data, f->ibuf_size);

	SOPHON_ASSERT(f->var_hash.count == f->varc + f->argc);

	svp.func = func;
	svp.data = data;
	sophon_hash_for_each(vm, &f->var_hash, store_var, &svp);

	func(data, f->f.ibuf, f->ibuf_size);

#ifdef SOPHON_LINE_INFO
	put_u16(func, data, f->lbuf_size);
	for (i = 0; i < f->lbuf_size; i++) {
		put_u16(func, data, f->lbuf[i].line);
		put_u16(func, data, f->lbuf[i].offset);
	}
#endif
}

static Sophon_Result
get_func (Sophon_VM *vm, Sophon_IOFunc func, Sophon_Ptr data,
			Sophon_Function *f)
{
	Sophon_Result r;
	Sophon_U16 i, cnt;

	if ((r = get_str(vm, func, data, SOPHON_TRUE, &f->name)) != SOPHON_OK)
		return r;

	if ((r = get_u8(func, data, &f->flags)) != SOPHON_OK)
		return r;
	if ((r = get_u8(func, data, &f->argc)) != SOPHON_OK)
		return r;
	if ((r = get_u16(func, data, &f->varc)) != SOPHON_OK)
		return r;
	if ((r = get_u16(func, data, &f->stack_size)) != SOPHON_OK)
		return r;
	if ((r = get_u16(func, data, &f->ibuf_size)) != SOPHON_OK)
		return r;

	cnt = f->argc + f->varc;
	for (i = 0; i < cnt; i++) {
		Sophon_String *name;
		Sophon_U16 id;
		Sophon_HashEntry *ent;

		if ((r = get_str(vm, func, data, SOPHON_FALSE, &name)) != SOPHON_OK)
			return r;
		if ((r = get_u16(func, data, &id)) != SOPHON_OK)
			return r;

		name = sophon_string_intern(vm, name);
		sophon_hash_add(vm, &f->var_hash,
					sophon_direct_key,
					sophon_direct_equal,
					(Sophon_Ptr)name,
					&ent);
		ent->value = (Sophon_Ptr)(Sophon_UIntPtr)id;
	}

	if (f->ibuf_size) {
		f->f.ibuf = (Sophon_U8*)sophon_mm_alloc_ensure(vm, f->ibuf_size);
		if ((r = func(data, f->f.ibuf, f->ibuf_size)) != f->ibuf_size)
			return SOPHON_ERR_PARSE;
	}

#ifdef SOPHON_LINE_INFO
	if ((r = get_u16(func, data, &f->lbuf_size)) != SOPHON_OK)
		return r;

	if (f->lbuf_size) {
		f->lbuf = (Sophon_LineInfo*)sophon_mm_alloc_ensure(vm,
					sizeof(Sophon_LineInfo) * f->lbuf_size);

		for (i = 0; i < f->lbuf_size; i++) {
			if ((r = get_u16(func, data, &f->lbuf[i].line)) != SOPHON_OK)
				return r;
			if ((r = get_u16(func, data, &f->lbuf[i].offset)) != SOPHON_OK)
				return r;
		}
	}
#endif

	return SOPHON_OK;
}

Sophon_Result
sophon_insfile_load (Sophon_VM *vm, Sophon_Module *mod,
			Sophon_IOFunc func, Sophon_Ptr data)
{
	Sophon_U32 u32;
	Sophon_U16 cnt;
	Sophon_U16 i;

	SOPHON_ASSERT(vm && mod && func);

	SOPHON_INFO(("begin"));
	if (get_u32(func, data, &u32) != SOPHON_OK)
		goto error;
	if (u32 != SOPHON_INSFILE_MAGIC)
		goto error;
	SOPHON_INFO(("magic"));
	if (get_u32(func, data, &u32) != SOPHON_OK)
		goto error;
	if (u32 != SOPHON_INSFILE_VERSION)
		goto error;
	SOPHON_INFO(("version"));

	if (get_str(vm, func, data, SOPHON_TRUE, &mod->name) != SOPHON_OK)
		goto error;

	if (get_u16(func, data, &cnt) != SOPHON_OK)
		goto error;
	SOPHON_INFO(("const %d", cnt));
	for (i = 0; i < cnt; i++) {
		Sophon_Value v;

		if (get_value(vm, func, data, &v) != SOPHON_OK)
			goto error;
		sophon_module_add_const(vm, mod, v);
	}

	if (get_u16(func, data, &cnt) != SOPHON_OK)
		goto error;
	SOPHON_INFO(("func %d", cnt));
	for (i = 0; i < cnt; i++) {
		Sophon_Function *f;
		Sophon_Int fid;

		fid = sophon_module_add_func(vm, mod, NULL, 0);
		f   = sophon_module_get_func(mod, fid);

		if (get_func(vm, func, data, f) != SOPHON_OK)
			goto error;

		if (f->flags & SOPHON_FUNC_FL_GLOBAL) {
			Sophon_Closure *clos;

			clos = sophon_closure_create(vm, f);
			sophon_value_set_closure(vm, &mod->globv, clos);
		}
	}

	return SOPHON_OK;
error:
	SOPHON_ERROR(("illegal instruction file"));
	return SOPHON_ERR_PARSE;
}

Sophon_Result
sophon_insfile_store (Sophon_VM *vm, Sophon_Module *mod,
			Sophon_IOFunc func, Sophon_Ptr data)
{
	Sophon_U16 i;

	SOPHON_ASSERT(vm && mod && func);

	put_u32(func, data, SOPHON_INSFILE_MAGIC);
	put_u32(func, data, SOPHON_INSFILE_VERSION);

	if (mod->name)
		put_str(vm, func, data, mod->name);
	else
		put_u32(func, data, 0);

	put_u16(func, data, mod->const_count);
	for (i = 0; i < mod->const_count; i++) {
		put_value(vm, func, data, mod->consts[i]);
	}

	put_u16(func, data, mod->func_count);
	for (i = 0; i < mod->func_count; i++) {
		put_func(vm, func, data, mod->funcs[i]);
	}


	return SOPHON_OK;
}

