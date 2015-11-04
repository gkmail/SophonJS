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
#include <sophon_arguments.h>
#include <sophon_regexp.h>
#include <sophon_object.h>
#include <sophon_string.h>
#include <sophon_hash.h>
#include <sophon_debug.h>

/*Allocate a new property*/
static Sophon_Property*
prop_alloc (Sophon_VM *vm, Sophon_Value getv, Sophon_Value setv,
			Sophon_U8 attrs, Sophon_U8 flags)
{
	Sophon_Property *prop;

	if (flags & (SOPHON_FL_HAVE_GET|SOPHON_FL_HAVE_SET)) {
		attrs |= SOPHON_PROP_ATTR_ACCESSOR;
		attrs |= SOPHON_ACCESSOR_PROP_ATTR & ~flags;
	} else {
		attrs &= ~SOPHON_PROP_ATTR_ACCESSOR;
		attrs |= SOPHON_DATA_PROP_ATTR & ~flags;
	}

	/*Add the new property*/
	if (attrs & SOPHON_PROP_ATTR_ACCESSOR) {
		Sophon_AccessorProperty *aprop;

		aprop = (Sophon_AccessorProperty*)sophon_mm_alloc_ensure(vm,
					sizeof(Sophon_AccessorProperty));

		if (flags & SOPHON_FL_HAVE_GET) {
			aprop->getv = getv;
		} else {
			sophon_value_set_undefined(vm, &aprop->getv);
		}

		if (flags & SOPHON_FL_HAVE_SET) {
			aprop->setv = setv;
		} else {
			sophon_value_set_undefined(vm, &aprop->setv);
		}

		prop = (Sophon_Property*)aprop;
	} else {
		prop = (Sophon_Property*)sophon_mm_alloc_ensure(vm,
					sizeof(Sophon_Property));

		if (flags & SOPHON_FL_HAVE_VALUE) {
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
	Sophon_PropIter *pi;

	/*Reset the iterators*/
	for (pi = vm->pi_stack; pi; pi = pi->bottom) {
		if (pi->prop == prop)
			pi->prop = prop->next;
	}

	/*Free the property*/
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
	Sophon_U8 accessor;
	Sophon_U8 mask;

	mask = flags & SOPHON_DATA_PROP_ATTR;

	if (flags & (SOPHON_FL_HAVE_GET|SOPHON_FL_HAVE_SET))
		accessor = SOPHON_PROP_ATTR_ACCESSOR;
	else if (flags & SOPHON_FL_HAVE_VALUE)
		accessor = 0;
	else
		accessor = prop->attrs & SOPHON_PROP_ATTR_ACCESSOR;

	if (!(flags & SOPHON_FL_FORCE)) {
		if (((attrs & mask) == (prop->attrs & mask)) &&
					(accessor == (prop->attrs & SOPHON_PROP_ATTR_ACCESSOR))) {
			if ((flags & SOPHON_FL_HAVE_VALUE) &&
						sophon_value_same(vm, getv, prop->value)) {
				return SOPHON_OK;
			} else if (flags & (SOPHON_FL_HAVE_GET|SOPHON_FL_HAVE_SET)) {
				Sophon_Bool equ = SOPHON_TRUE;

				if ((flags & SOPHON_FL_HAVE_GET) &&
							!sophon_value_same(vm, getv, aprop->getv))
					equ = SOPHON_FALSE;
				if ((flags & SOPHON_FL_HAVE_SET) &&
							!sophon_value_same(vm, setv, aprop->setv))
					equ = SOPHON_FALSE;

				if (equ)
					return SOPHON_OK;
			} else {
				return SOPHON_OK;
			}
		}

		if (!(prop->attrs & SOPHON_PROP_ATTR_CONFIGURABLE))
			return SOPHON_ERR_ACCESS;
	}

	/*Reset the property*/
	if (accessor !=	(prop->attrs & SOPHON_PROP_ATTR_ACCESSOR)) {
		nprop = prop_alloc(vm, getv, setv, attrs, flags);

		nprop->name = prop->name;
		nprop->next = prop->next;
		*pp = nprop;

		prop_free(vm, prop);
	} else {
		prop->attrs &= ~mask;
		prop->attrs |= mask & attrs;

		if (flags & SOPHON_FL_HAVE_VALUE)
			prop->value = getv;
		if (flags & SOPHON_FL_HAVE_GET)
			aprop->getv = getv;
		if (flags & SOPHON_FL_HAVE_SET)
			aprop->setv = setv;
	}

	return SOPHON_OK;
}

void
sophon_object_init (Sophon_VM *vm, Sophon_Object *obj)
{
	SOPHON_ASSERT(vm && obj);

	obj->gc_type = SOPHON_GC_Object;

	obj->props = NULL;
	obj->prop_bucket = 0;
	obj->prop_count  = 0;
	obj->protov = vm->Object_protov;

	sophon_value_set_undefined(vm, &obj->primv);
}

Sophon_Object*
sophon_object_create (Sophon_VM *vm)
{
	Sophon_Object *obj;

	SOPHON_ASSERT(vm);

	obj = (Sophon_Object*)sophon_mm_alloc_ensure(vm, sizeof(Sophon_Object));

	obj->gc_flags = SOPHON_GC_FL_EXTENSIBLE;
	sophon_object_init(vm, obj);

	sophon_gc_add(vm, (Sophon_GCObject*)obj);

	return obj;
}

void
sophon_object_destroy (Sophon_VM *vm, Sophon_Object *obj)
{
	Sophon_Property **p, **plast;
	Sophon_U32 size;

	SOPHON_ASSERT(vm && obj);

#define OBJECT_SIZE(o)\
	if (obj->gc_flags & SOPHON_GC_FL_##o) {\
		size = SOPHON_MAX(sizeof(Sophon_Object), sizeof(Sophon_##o));\
	} else

	SOPHON_FOR_EACH_PRIM_OBJ(OBJECT_SIZE)
	{
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

static Sophon_Bool
value_is_array (Sophon_VM *vm, Sophon_Value v, Sophon_Array **parr)
{
	if (!sophon_value_is_array(v))
		return SOPHON_FALSE;

	sophon_value_to_array(vm, v, parr);
	return SOPHON_TRUE;
}

static Sophon_Bool
value_is_string (Sophon_VM *vm, Sophon_Value v, Sophon_String **pstr)
{
	if (!sophon_value_is_string(v))
		return SOPHON_FALSE;

	sophon_value_to_string(vm, v, pstr);
	return SOPHON_TRUE;
}

static Sophon_Bool
name_is_index (Sophon_VM *vm, Sophon_Value namev, Sophon_U32 *pid)
{
	Sophon_U32 i;

	if (SOPHON_VALUE_IS_INT(namev)) {
		i = SOPHON_VALUE_GET_INT(namev);
		if (i != 0xFFFFFFFF) {
			*pid = i;
			return SOPHON_OK;
		}
	} else if (SOPHON_VALUE_IS_NUMBER(namev)) {
		Sophon_Number d;

		d = SOPHON_VALUE_GET_NUMBER(namev);
		i = (Sophon_U32)d;
		if ((d == (Sophon_Number)i) && (i != 0xFFFFFFFF)) {
			*pid = i;
			return SOPHON_OK;
		}
	} else if (SOPHON_VALUE_IS_GC(namev)) {
		Sophon_GCObject *gco = SOPHON_VALUE_GET_GC(namev);

		if (gco->gc_type == SOPHON_GC_String) {
			Sophon_String *str, *nstr;
			Sophon_Char *chars, *end;
			Sophon_U32 len;
			Sophon_Number n;
			Sophon_Result r;

			nstr = SOPHON_VALUE_GET_STRING(namev);
			if (!sophon_string_cmp(vm, nstr, vm->length_str))
				return SOPHON_FALSE;

			str   = SOPHON_VALUE_GET_STRING(namev);
			chars = sophon_string_chars(vm, str);
			len   = sophon_string_length(vm, str);

			r = sophon_strtod(chars, &end, -1, &n);
			if ((r == SOPHON_OK) && (end == chars + len)) {
				Sophon_Value nv;
				Sophon_U32 u32;

				sophon_value_set_number(vm, &nv, n);
				sophon_value_to_u32(vm, nv, &u32);

				if (u32 != 0xFFFFFFFF) {
					*pid = u32;
					return SOPHON_OK;
				}
			}
		} else if (gco->gc_type == SOPHON_GC_Object) {
			Sophon_Object *obj = (Sophon_Object*)gco;
			return name_is_index(vm, obj->primv, pid);
		}
	}

	return SOPHON_FALSE;
}

Sophon_Result
sophon_value_prop_desc (Sophon_VM *vm, Sophon_Value v,
			Sophon_Value namev, Sophon_PropDesc *desc)
{
	Sophon_Object *obj;
	Sophon_String *name;
	Sophon_Property *prop;
	Sophon_U16 kv;
	Sophon_Result r;
	Sophon_Array *arr;
	Sophon_U32 id;

	SOPHON_ASSERT(vm);

	/*Array item property*/
	if (value_is_array(vm, v, &arr) && name_is_index(vm, namev, &id)) {
		if (!sophon_array_has_item(vm, arr, id))
			return SOPHON_NONE;

		if (desc) {
			desc->attrs = SOPHON_DATA_PROP_ATTR;
			desc->value = arr->v[id];
		}
		return SOPHON_OK;
	}

	/*Lookup property hash table*/
	obj = sophon_value_get_class(vm, v);

	if ((r = sophon_value_to_string(vm, namev, &name)) != SOPHON_OK)
		return r;

	name = sophon_string_intern(vm, name);

	if (!obj->prop_count)
		return SOPHON_NONE;

	kv = ((Sophon_UIntPtr)name) % obj->prop_bucket;
	prop = obj->props[kv];
	while (prop) {
		if (prop->name == name) {
			if (desc) {
				if (prop->attrs & SOPHON_PROP_ATTR_ACCESSOR) {
					Sophon_AccessorProperty *ap =
						(Sophon_AccessorProperty*)prop;

					desc->getv  = ap->getv;
					desc->setv  = ap->setv;
					desc->attrs = ap->attrs;
				} else {
					desc->value = prop->value;
					desc->attrs = prop->attrs;
				}
			}

			return SOPHON_OK;
		}

		prop = prop->next;
	}

	return SOPHON_NONE;
}

Sophon_Result
sophon_value_define_prop (Sophon_VM *vm, Sophon_Value v,
			Sophon_Value namev, Sophon_Value getv, Sophon_Value setv,
			Sophon_U8 attrs, Sophon_U32 flags)
{
	Sophon_Object *obj;
	Sophon_String *name;
	Sophon_Property *prop, *pnext;
	Sophon_U16 kv;
	Sophon_Result r;
	Sophon_Array *arr;
	Sophon_U32 id;

	SOPHON_ASSERT(vm);

	/*Array item property*/
	if (value_is_array(vm, v, &arr) && name_is_index(vm, namev, &id)) {
		if (flags & SOPHON_FL_HAVE_VALUE) {
			r = sophon_array_set_item(vm, arr, id, getv);
		} else if (flags & (SOPHON_FL_HAVE_GET|SOPHON_FL_HAVE_SET)) {
			sophon_throw(vm, vm->TypeError,
					"Cannot set array item as accessor");
			return SOPHON_ERR_THROW;
		} else {
			r = sophon_array_set_length(vm, arr, id);
		}

		return r;
	}

	/*Add normal property*/
	if ((r = sophon_value_to_object(vm, v, &obj)) != SOPHON_OK)
		return r;

	if ((r = sophon_value_to_string(vm, namev, &name)) != SOPHON_OK)
		return r;

	name = sophon_string_intern(vm, name);

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
				if (r != SOPHON_OK) {
					if (flags & SOPHON_FL_THROW)
						sophon_throw(vm, vm->TypeError,
								"Property is not configurable");
					else
						r = SOPHON_OK;
				}

				return r;
			}

			pp   = &prop->next;
			prop = *pp;
		}
	}

	if (!(obj->gc_flags & SOPHON_GC_FL_EXTENSIBLE) &&
				!(flags & SOPHON_FL_FORCE)) {
		if (flags & SOPHON_FL_THROW) {
			sophon_throw(vm, vm->TypeError,
					"Object is not extensible");
			r = SOPHON_ERR_RDONLY;
		} else {
			r = SOPHON_OK;
		}

		return r;
	}

	/*Resize the property buffer size*/
	if (obj->prop_count >= obj->prop_bucket * 3) {
		Sophon_Property **buf;
		Sophon_Int size = SOPHON_MAX(9, obj->prop_count);
		Sophon_Int i;

		if (size > SOPHON_MAX_U16)
			sophon_fatal("too many properties in the object");

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

	prop->name = name;
	prop->next = obj->props[kv];
	obj->props[kv] = prop;

	obj->prop_count++;

	return SOPHON_OK;
}

Sophon_Result
sophon_value_delete_prop (Sophon_VM *vm, Sophon_Value v,
			Sophon_Value namev, Sophon_U32 opts)
{
	Sophon_Object *obj;
	Sophon_String *name;
	Sophon_Property *prop, **pp;
	Sophon_U16 kv;
	Sophon_Result r;
	Sophon_Array *arr;
	Sophon_U32 id;

	/*Array item property*/
	if (value_is_array(vm, v, &arr) && name_is_index(vm, namev, &id)) {
		sophon_array_delete_item(vm, arr, id);
		return SOPHON_OK;
	}

	/*Normal property*/
	if (!sophon_value_is_object(v)) {
		return (opts & SOPHON_FL_NONE) ? SOPHON_NONE : SOPHON_OK;
	}

	if ((r = sophon_value_to_object(vm, v, &obj)) != SOPHON_OK)
		return r;

	if ((r = sophon_value_to_string(vm, namev, &name)) != SOPHON_OK)
		return r;

	name = sophon_string_intern(vm, name);

	if (!obj->prop_count)
		return (opts & SOPHON_FL_NONE) ? SOPHON_NONE : SOPHON_OK;

	kv = ((Sophon_UIntPtr)name) % obj->prop_bucket;
	pp = &obj->props[kv];
	prop = *pp;

	while (prop) {
		if (prop->name == name) {
			*pp = prop->next;

			if (!(prop->attrs & SOPHON_PROP_ATTR_CONFIGURABLE)) {
				if (opts & SOPHON_FL_THROW) {
					sophon_throw(vm, vm->TypeError,
							"Property is not configurable");
					return SOPHON_ERR_ACCESS;
				} else {
					return SOPHON_OK;
				}
			}

			obj->prop_count--;
			prop_free(vm, prop);
			return SOPHON_OK;
		}

		pp   = &prop->next;
		prop = *pp;
	}

	return (opts & SOPHON_FL_NONE) ? SOPHON_NONE : SOPHON_OK;
}

#include <sophon_module.h>

Sophon_Result
sophon_value_get (Sophon_VM *vm, Sophon_Value thisv,
			Sophon_Value namev, Sophon_Value *getv, Sophon_U32 flags)
{
	Sophon_Object *obj;
	Sophon_String *name;
	Sophon_Result r;
	Sophon_Array *arr;
	Sophon_String *str;
	Sophon_U32 id;

	SOPHON_ASSERT(vm && getv);

	/*Array item property*/
	if (value_is_array(vm, thisv, &arr) && name_is_index(vm, namev, &id)) {
		return sophon_array_get_item(vm, arr, id, getv);
	}

	/*String item*/
	if (value_is_string(vm, thisv, &str) && name_is_index(vm, namev, &id)) {
		return sophon_string_get_item(vm, str, id, getv);
	}

	/*Normal property*/
	if (!(obj = sophon_value_get_class(vm, thisv))) {
		sophon_throw(vm, vm->TypeError,
				"Value is null or undefined");
		return SOPHON_ERR_THROW;
	}

	if ((r = sophon_value_to_string(vm, namev, &name)) != SOPHON_OK)
		return r;

	name = sophon_string_intern(vm, name);

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
									NULL, 0, getv, 0);
					} else {
						*getv = prop->value;
					}
					return SOPHON_OK;
				}

				prop = prop->next;
			}
		}

		if (flags & SOPHON_FL_OWN)
			break;

		if (SOPHON_VALUE_IS_NULL(obj->protov))
			break;

		r = sophon_value_to_object(vm, obj->protov, &obj);
		if (r != SOPHON_OK)
			return r;
	}

	if (flags & SOPHON_FL_NONE)
		return SOPHON_NONE;

	sophon_value_set_undefined(vm, getv);
	return SOPHON_OK;
}

Sophon_Result
sophon_value_put (Sophon_VM *vm, Sophon_Value thisv,
			Sophon_Value namev, Sophon_Value setv, Sophon_U32 flags)
{
	Sophon_Object *obj;
	Sophon_String *name;
	Sophon_Result r;
	Sophon_Array *arr;
	Sophon_U32 id;

	SOPHON_ASSERT(vm);

	/*Array item property*/
	if (value_is_array(vm, thisv, &arr) && name_is_index(vm, namev, &id)) {
		return sophon_array_set_item(vm, arr, id, setv);
	}

	/*Normal property*/
	if (!(obj = sophon_value_get_class(vm, thisv))) {
		sophon_throw(vm, vm->TypeError,
				"Value is null or undefined");
		return SOPHON_ERR_THROW;
	}

	if ((r = sophon_value_to_string(vm, namev, &name)) != SOPHON_OK)
		return r;

	name = sophon_string_intern(vm, name);

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
						Sophon_Value retv;

						if (SOPHON_VALUE_IS_UNDEFINED(aprop->setv)) {
							if (flags & SOPHON_FL_THROW) {
								sophon_throw(vm, vm->TypeError,
										"Set function is undefined");
								return SOPHON_ERR_RDONLY;
							} else {
								return SOPHON_OK;
							}
						}

						return sophon_value_call(vm, aprop->setv, thisv,
									&setv, 1, &retv, 0);
					} else {
						if (!(prop->attrs & SOPHON_PROP_ATTR_WRITABLE) &&
									!sophon_value_same(vm, prop->value, setv)) {
							if (flags & SOPHON_FL_THROW) {
								sophon_throw(vm, vm->TypeError,
										"Property is not writable");
								return SOPHON_ERR_RDONLY;
							} else {
								return SOPHON_OK;
							}
						}

						prop->value = setv;
						return SOPHON_OK;
					}
				}

				prop = prop->next;
			}
		}

		if (flags & SOPHON_FL_OWN)
			break;

		if (SOPHON_VALUE_IS_NULL(obj->protov))
			break;

		if (sophon_value_to_object(vm, obj->protov, &obj) != SOPHON_OK)
			break;
	}

	if (flags & SOPHON_FL_NONE)
		return SOPHON_NONE;

	return sophon_value_define_prop(vm, thisv, namev,
				setv, SOPHON_VALUE_UNDEFINED,
				SOPHON_DATA_PROP_ATTR, SOPHON_FL_DATA_PROP|flags);
}

Sophon_Int
sophon_value_for_each_prop(Sophon_VM *vm, Sophon_Value v,
			Sophon_PropForEachFunc func, Sophon_Ptr data)
{
	Sophon_Int i;
	Sophon_Property *prop;
	Sophon_Object *obj;
	Sophon_Int r;
	Sophon_Array *arr;

	SOPHON_ASSERT(vm && func);

	if (value_is_array(vm, v, &arr)) {
		Sophon_U32 id;
		Sophon_Value item;

		sophon_array_for_each(arr, id, item) {
			Sophon_Property prop;
			Sophon_Value idv;
			Sophon_String *str;

			sophon_value_set_int(vm, &idv, id);
			if ((r = sophon_value_to_string(vm, idv, &str)) != SOPHON_OK)
				return r;

			prop.name  = str;
			prop.attrs = SOPHON_DATA_PROP_ATTR;
			prop.value = item;
			
			if ((r = func(vm, &prop, data)) != 1)
				return r;
		}
	}

	if ((r = sophon_value_to_object(vm, v, &obj)) != SOPHON_OK)
		return SOPHON_ERR_THROW;

	for (i = 0; i < obj->prop_bucket; i++) {
		for (prop = obj->props[i]; prop; prop = prop->next) {
			r = func(vm, prop, data);
			if (r != 1)
				return r;
		}
	}

	return 1;
}

