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
#include <sophon_closure.h>
#include <sophon_array.h>
#include <sophon_object.h>
#include <sophon_debug.h>

/*Allocate a new property*/
static Sophon_Property*
prop_alloc (Sophon_VM *vm, Sophon_Value getv, Sophon_Value setv,
			Sophon_U8 attrs, Sophon_U8 flags)
{
	Sophon_Property *prop;

	if (flags & (SOPHON_PROP_HAVE_GET|SOPHON_PROP_HAVE_SET)) {
		attrs |= SOPHON_PROP_ATTR_ACCESSOR;
	} else {
		attrs &= ~SOPHON_PROP_ATTR_ACCESSOR;
	}

	/*Add the new property*/
	if (attrs & SOPHON_PROP_ATTR_ACCESSOR) {
		Sophon_AccessorProperty *aprop;

		aprop = (Sophon_AccessorProperty*)sophon_mm_alloc_ensure(vm,
					sizeof(Sophon_AccessorProperty));

		if (flags & SOPHON_PROP_HAVE_GET) {
			aprop->getv = getv;
		} else {
			sophon_value_set_undefined(vm, &prop->getv);
		}

		if (flags & SOPHON_PROP_HAVE_SET) {
			aprop->setv = setv;
		} else {
			sophon_value_set_undefined(vm, &prop->setv);
		}

		prop = (Sophon_Property*)aprop;
	} else {
		prop = (Sophon_Property*)sophon_mm_alloc_ensure(vm,
					sizeof(Sophon_Property));

		if (flags & SOPHON_PROP_HAVE_VALUE) {
			prop->value = getv;
		} else {
			sophon_value_set_undefined(vm, &prop->value);
		}
	}

	prop->attrs = attrs;

	return prop;
}

/*Free a property*/
static void
prop_free (Sophon_VM *vm, Sophon_Property *prop)
{
	sophon_mm_free(vm, prop,
				(prop->attrs & SOPHON_PROP_ATTR_ACCESSOR) ?
				sizeof(Sophon_AccessorProperty) :
				sizeof(Sophon_Property));
}

static Sophon_Result
prop_reset (Sophon_VM *vm, Sophon_Property **pp, Sophon_Property *prop,
			Sophon_Value getv, Sophon_Value setv,
			Sophon_U8 attrs, Sophon_U8 flags)
{
	Sophon_AccessorProperty *aprop = (Sophon_AccessorProperty*)prop;
	Sophon_Property *nprop;
	Sophon_U8 mask;

	mask = flags & SOPHON_DATA_PROP_ATTR;

#define VEQU(a,b) sophon_value_equal(vm, (Sophon_Ptr)(a), (Sophon_Ptr)(b))

	if ((attrs & mask) == (prop->attrs & mask)) {
		if ((flags & SOPHON_PROP_HAVE_VALUE) &&
					!(prop->attrs & SOPHON_PROP_ATTR_ACCESSOR) &&
					VEQU(getv, prop->value)) {
			return SOPHON_OK;
		} else if ((flags & (SOPHON_PROP_HAVE_GET|SOPHON_PROP_HAVE_SET)) &&
					(prop->attrs & SOPHON_PROP_ATTR_ACCESSOR)) {
			Sophon_Bool equ = SOPHON_TRUE;

			if ((flags & SOPHON_PROP_HAVE_GET) && VEQU(getv, aprop->getv))
				equ = SOPHON_FALSE;
			if ((flags & SOPHON_PROP_HAVE_SET) && VEQU(getv, aprop->setv))
				equ = SOPHON_FALSE;

			if (equ)
				return SOPHON_OK;
		} else {
			return SOPHON_OK;
		}
	}

	if (!(prop->attrs & SOPHON_PROP_ATTR_CONFIGURABLE))
		return SOPHON_ERR_ACCESS;

	/*Reset the property*/
	if ((attrs & SOPHON_PROP_ATTR_ACCESSOR) != 
				(prop->attrs & SOPHON_PROP_ATTR_ACCESSOR)) {
		nprop = prop_alloc(vm, getv, setv, attrs, flags);

		nprop->next = prop->next;
		*pp = nprop;

		prop_free(vm, prop);
	} else {
		prop->attrs &= ~mask;
		prop->attrs |= mask & attrs;

		if (flags & SOPHON_PROP_HAVE_VALUE)
			prop->value = getv;
		if (flags & SOPHON_PROP_HAVE_GET)
			prop->getv = getv;
		if (flags & SOPHON_PROP_HAVE_SET)
			prop->setv = setv;
	}

	return SOPHON_OK;
}

void
sophon_object_init (Sophon_VM *vm, Sophon_Object *obj)
{
	SOPHON_ASSERT(vm && obj);

	obj->gc_type = SOPHON_GC_OBJECT;

	obj->flags = SOPHON_OBJ_FL_EXTENSIBLE;
	obj->props = NULL;
	obj->prop_bucket = 0;
	obj->prop_count  = 0;

	sophon_value_set_null(vm, &obj->protov);
	sophon_value_set_undefined(vm, &obj->primv);
}

Sophon_Object*
sophon_object_create (Sophon_VM *vm)
{
	Sophon_Object *obj;

	SOPHON_ASSERT(vm);

	obj = (Sophon_Object*)sophon_mm_alloc_ensure(vm, sizeof(Sophon_Object));

	obj->gc_flags = 0;
	sophon_object_init(vm, obj);

	return obj;
}

void
sophon_object_destroy (Sophon_VM *vm, Sophon_Object *obj)
{
	Sophon_Property **p, **plast;
	Sophon_U32 size;

	SOPHON_ASSERT(vm && obj);

	if (obj->gc_flags & SOPHON_GC_FL_CLOSURE) {
		size = SOPHON_GC_CLOSURE_SIZE;
	} else if (obj->gc_flags & SOPHON_GC_FL_ARRAY) {
		size = SOPHON_GC_ARRAY_SIZE;
	} else {
		size = sizeof(Sophon_Object);
	}

	p = obj->props;
	if (p) {
		plast = p + obj->prop_bucket;
		while (p < plast) {
			Sophon_Property *prop, *pnext;

			prop = *p;
			while (prop) {
				pnext = prop->next;
				prop_free(vm, prop);
				prop = pnext;
			}
			p++;
		}

		sophon_mm_free(vm, obj->props,
					sizeof(Sophon_Property*) * obj->prop_bucket);
	}

	sophon_mm_free(vm, obj, size);
}

Sophon_Property*
sophon_object_get_prop (Sophon_VM *vm, Sophon_Object *obj,
			Sophon_String *name)
{
	Sophon_Property *prop;
	Sophon_U16 kv;

	SOPHON_ASSERT(vm && obj && name);

	if (!obj->prop_count)
		return NULL;

	kv = ((Sophon_UIntPtr)name) % obj->prop_bucket;
	prop = obj->props[kv];
	while (prop) {
		if (prop->name == name)
			return prop;

		prop = prop->next;
	}

	return NULL;
}

Sophon_Result
sophon_object_define_prop (Sophon_VM *vm, Sophon_Object *obj,
			Sophon_String *name, Sophon_Value getv, Sophon_Value setv,
			Sophon_U8 attrs, Sophon_U8 flags,
			Sophon_Bool thr)
{
	Sophon_Property *prop, *pnext;
	Sophon_U16 kv;

	SOPHON_ASSERT(vm && obj && name);

	if (flags & (SOPHON_PROP_HAVE_SET|SOPHON_PROP_HAVE_GET))
		attrs |= SOPHON_PROP_ATTR_ACCESSOR;
	else if (flags & SOPHON_PROP_HAVE_VALUE)
		attrs &= ~SOPHON_PROP_ATTR_ACCESSOR;

	/*Check the old property*/
	if (obj->prop_count) {
		Sophon_Property **pp;

		kv = ((Sophon_UIntPtr)name) % obj->prop_bucket;
		pp = &obj->props[kv];
		prop = *pp;
		while (prop) {
			if (prop->name == name) {
				/*Reset the property*/
				Sophon_Result r;

				r = prop_reset(vm, pp, prop,
							getv, setv, attrs, flags);
				if ((r != SOPHON_OK) && thr)
					sophon_throw(vm, vm->type_errv);

				return r;
			}

			pp   = &prop->next;
			prop = *pp;
		}
	}

	if (!(obj->flags & SOPHON_OBJ_FL_EXTENSIBLE)) {
		sophon_throw(vm, vm->type_errv);
		return SOPHON_ERR_RDONLY;
	}

	/*Resize the property buffer size*/
	if (obj->prop_count >= obj->prop_bucket * 3) {
		Sophon_Property **buf;
		Sophon_Int size = SOPHON_MAX(16, obj->prop_count);
		Sophon_Int i;

		SOPHON_ASSERT(size <= SOPHON_MAX_U16);

		buf = (Sophon_Property**)sophon_mm_alloc_ensure(vm,
					size * sizeof(Sophon_Property*));
		sophon_memset(buf, 0, size * sizeof(Sophon_Property*));

		for (i = 0; i < obj->prop_bucket; i++) {
			for (prop = obj->props[i]; prop; prop = pnext) {
				pnext = prop->next;

				kv = ((Sophon_UIntPtr)prop->name) % size;
				prop->next = buf[kv];
				buf[kv] = prop;
			}
		}

		if (obj->props) {
			sophon_mm_free(vm, obj->props,
						obj->prop_bucket * sizeof(Sophon_Property*));
		}

		obj->props = buf;
		obj->prop_bucket = size;
	}

	/*Add the new property*/
	prop = prop_alloc(vm, getv, setv, attrs, flags);
	kv = ((Sophon_UIntPtr)name) % obj->prop_bucket;
	prop->next = obj->props[kv];
	obj->props[kv] = prop;

	obj->prop_count++;

	return SOPHON_OK;
}

Sophon_Result
sophon_object_delete_prop (Sophon_VM *vm, Sophon_Object *obj,
			Sophon_String *name, Sophon_Bool thr)
{
	Sophon_Property *prop, **pp;
	Sophon_U16 kv;

	if (!obj->prop_count)
		return SOPHON_OK;

	kv = ((Sophon_UIntPtr)name) % obj->prop_bucket;
	pp = &obj->props[kv];
	prop = *pp;

	while (prop) {
		if (prop->name == name) {
			*pp = prop->next;

			if (!(prop->attrs & SOPHON_PROP_ATTR_CONFIGURABLE)) {
				if (thr) {
					sophon_throw(vm, vm->type_errv);
				}

				return SOPHON_ERR_ACCESS;
			}

			obj->prop_count--;
			sophon_mm_free(vm, prop,
						(prop->attrs & SOPHON_PROP_ATTR_ACCESSOR) ?
						sizeof(Sophon_AccessorProperty) :
						sizeof(Sophon_Property));
			return SOPHON_OK;
		}

		pp   = &prop->next;
		prop = *pp;
	}

	return SOPHON_OK;
}

Sophon_Result
sophon_object_get (Sophon_VM *vm, Sophon_Object *obj, Sophon_Value thisv,
			Sophon_String *name, Sophon_Value *v)
{
	Sophon_Result r;

	SOPHON_ASSERT(vm && obj && name && v);

	while (1) {
		if (obj->prop_count) {
			Sophon_Property *prop;
			Sophon_U16 kv;

			kv   = ((Sophon_UIntPtr)name) % obj->prop_bucket;
			prop = obj->props[kv];
			while (prop) {
				if (prop->name == name) {
					if (prop->attrs & SOPHON_PROP_ATTR_ACCESSOR) {
						Sophon_AccessorProperty *aprop =
							(Sophon_AccessorProperty*)prop;
						return sophon_value_call(vm, aprop->getv, thisv,
									NULL, 0, v);
					} else {
						*v = prop->value;
					}
					return SOPHON_OK;
				}

				prop = prop->next;
			}
		}

		if (SOPHON_VALUE_IS_NULL(obj->protov))
			break;

		r = sophon_value_get_object(vm, obj->protov, &obj);
		if (r != SOPHON_OK)
			return r;
	}

	sophon_value_set_undefined(vm, v);
	return SOPHON_OK;
}

Sophon_Result
sophon_object_put(Sophon_VM *vm, Sophon_Object *obj, Sophon_Value thisv,
			Sophon_String *name, Sophon_Value v, Sophon_Bool thr)
{
	Sophon_Object *base;

	SOPHON_ASSERT(vm && obj && name);

	base = obj;
	while (1) {
		if (obj->prop_count) {
			Sophon_Property *prop;
			Sophon_U16 kv;

			kv   = ((Sophon_UIntPtr)name) % obj->prop_bucket;
			prop = obj->props[kv];
			while (prop) {
				if (prop->name == name) {
					if (prop->attrs & SOPHON_PROP_ATTR_ACCESSOR) {
						Sophon_AccessorProperty *aprop =
							(Sophon_AccessorProperty*)prop;

						if (SOPHON_VALUE_IS_UNDEFINED(aprop->setv)) {
							sophon_throw(vm, vm->type_errv);
							return SOPHON_ERR_RDONLY;
						}

						return sophon_value_call(vm, aprop->setv, thisv,
									&v, 1, NULL);
					} else {
						if (!(prop->attrs & SOPHON_PROP_ATTR_WRITABLE)) {
							sophon_throw(vm, vm->type_errv);
							return SOPHON_ERR_RDONLY;
						}

						prop->value = v;
						return SOPHON_OK;
					}
				}

				prop = prop->next;
			}
		}

		if (SOPHON_VALUE_IS_NULL(obj->protov))
			break;

		if (sophon_value_get_object(vm, obj->protov, &obj) != SOPHON_OK)
			break;
	}

	return sophon_object_define_prop(vm, base, name,
				v, SOPHON_VALUE_UNDEFINED,
				SOPHON_DATA_PROP_ATTR, SOPHON_DATA_PROP_FLAGS,
				thr);
}

