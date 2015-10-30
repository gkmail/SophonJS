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

#define OBJECT_FUNC_PROP(name, argc)\
	SOPHON_FUNCTION_PROP(name, GLOBAL_ATTRS, object_##name##_func, argc)
#define OBJECT_FUNC(name)\
	SOPHON_FUNC(object_##name##_func)

/*Create a property descriptor object*/
static Sophon_Value
prop_desc_to_obj (Sophon_VM *vm, Sophon_PropDesc *desc)
{
	Sophon_Object *obj;
	Sophon_Value objv;
	Sophon_Bool b;

	obj = sophon_object_create(vm);
	sophon_value_set_object(vm, &objv, obj);

	if (desc->attrs & SOPHON_PROP_ATTR_ACCESSOR) {
		sophon_value_define_prop(vm, objv, SOPHON_VALUE_GC(vm->get_str),
					desc->getv,
					SOPHON_VALUE_UNDEFINED,
					SOPHON_DATA_PROP_ATTR,
					SOPHON_FL_DATA_PROP);
		sophon_value_define_prop(vm, objv, SOPHON_VALUE_GC(vm->set_str),
					desc->setv,
					SOPHON_VALUE_UNDEFINED,
					SOPHON_DATA_PROP_ATTR,
					SOPHON_FL_DATA_PROP);
	} else {
		sophon_value_define_prop(vm, objv, SOPHON_VALUE_GC(vm->value_str),
					desc->value,
					SOPHON_VALUE_UNDEFINED,
					SOPHON_DATA_PROP_ATTR,
					SOPHON_FL_DATA_PROP);

		b = desc->attrs & SOPHON_PROP_ATTR_WRITABLE;
		sophon_value_define_prop(vm, objv, SOPHON_VALUE_GC(vm->writable_str),
					SOPHON_VALUE_BOOL(b),
					SOPHON_VALUE_UNDEFINED,
					SOPHON_DATA_PROP_ATTR,
					SOPHON_FL_DATA_PROP);
	}

	b = desc->attrs & SOPHON_PROP_ATTR_ENUMERABLE;
	sophon_value_define_prop(vm, objv, SOPHON_VALUE_GC(vm->enumerable_str),
				SOPHON_VALUE_BOOL(b),
				SOPHON_VALUE_UNDEFINED,
				SOPHON_DATA_PROP_ATTR,
				SOPHON_FL_DATA_PROP);

	b = desc->attrs & SOPHON_PROP_ATTR_CONFIGURABLE;
	sophon_value_define_prop(vm, objv, SOPHON_VALUE_GC(vm->configurable_str),
				SOPHON_VALUE_BOOL(b),
				SOPHON_VALUE_UNDEFINED,
				SOPHON_DATA_PROP_ATTR,
				SOPHON_FL_DATA_PROP);

	return objv;
}

static Sophon_Result
obj_add_prop (Sophon_VM *vm, Sophon_Value v, Sophon_Value namev,
			Sophon_Value propv)
{
	Sophon_Value attrv;
	Sophon_Value getv, setv;
	Sophon_U8 attrs;
	Sophon_U32 flags;

	sophon_value_set_undefined(vm, &getv);
	sophon_value_set_undefined(vm, &setv);
	attrs = 0;
	flags = SOPHON_FL_THROW;

	if (sophon_value_get(vm, propv, SOPHON_VALUE_GC(vm->configurable_str),
					&attrv, SOPHON_FL_NONE) == SOPHON_OK) {
		if (sophon_value_to_bool(vm, attrv))
			attrs |= SOPHON_PROP_ATTR_CONFIGURABLE;
		flags |= SOPHON_FL_HAVE_CONFIGURABLE;
	}

	if (sophon_value_get(vm, propv, SOPHON_VALUE_GC(vm->enumerable_str),
					&attrv, SOPHON_FL_NONE) == SOPHON_OK) {
		if (sophon_value_to_bool(vm, attrv))
			attrs |= SOPHON_PROP_ATTR_ENUMERABLE;
		flags |= SOPHON_FL_HAVE_ENUMERABLE;
	}

	if (sophon_value_get(vm, propv, SOPHON_VALUE_GC(vm->writable_str),
					&attrv, SOPHON_FL_NONE) == SOPHON_OK) {
		if (sophon_value_to_bool(vm, attrv))
			attrs |= SOPHON_PROP_ATTR_WRITABLE;
		flags |= SOPHON_FL_HAVE_WRITABLE;
	}

	if (sophon_value_get(vm, propv, SOPHON_VALUE_GC(vm->get_str),
					&getv, SOPHON_FL_NONE) == SOPHON_OK) {
		flags |= SOPHON_FL_HAVE_GET;
	}

	if (sophon_value_get(vm, propv, SOPHON_VALUE_GC(vm->set_str),
					&setv, SOPHON_FL_NONE) == SOPHON_OK) {
		flags |= SOPHON_FL_HAVE_SET;
	}

	if (sophon_value_get(vm, propv, SOPHON_VALUE_GC(vm->value_str),
					&getv, SOPHON_FL_NONE) == SOPHON_OK) {
		flags |= SOPHON_FL_HAVE_VALUE;
	}

	return sophon_value_define_prop(vm, v, namev, getv, setv, attrs, flags);
}

static Sophon_Int
obj_add_each_prop (Sophon_VM *vm, Sophon_Property *prop, Sophon_Ptr data)
{
	Sophon_Value v = (Sophon_Value)data;
	Sophon_Value namev, propv;
	Sophon_Result r;

	if (!(prop->attrs & SOPHON_PROP_ATTR_ENUMERABLE))
		return 1;

	namev = SOPHON_VALUE_GC(prop->name);

	if (prop->attrs & SOPHON_PROP_ATTR_ACCESSOR) {
		Sophon_AccessorProperty *aprop = (Sophon_AccessorProperty*)prop;

		if ((r = sophon_value_call(vm, aprop->getv, v, NULL, 0, &propv, 0))
					!= SOPHON_OK)
			return r;
	} else {
		propv = prop->value;
	}

	if ((r = obj_add_prop(vm, v, namev, propv)) != SOPHON_OK)
		return r;

	return 1;
}

/*Add properties to the object*/
static Sophon_Result
obj_add_props (Sophon_VM *vm, Sophon_Value objv, Sophon_Value propsv)
{
	if(sophon_value_for_each_prop(vm, propsv, obj_add_each_prop,
					(Sophon_Ptr)objv) != 1)
		return SOPHON_ERR_THROW;

	return SOPHON_OK;
}

static OBJECT_FUNC(call)
{
	Sophon_Object *obj = NULL;
	Sophon_Value v;
	Sophon_Result r;

	v = SOPHON_ARG(0);

	if (sophon_value_is_null(v) || sophon_value_is_undefined(v)) {
		obj = sophon_object_create(vm);
	} else {
		if ((r = sophon_value_to_object(vm, v, &obj)) != SOPHON_OK)
			return r;
	}

	sophon_value_set_object(vm, retv, obj);

	return SOPHON_OK;
}

static OBJECT_FUNC(getPrototypeOf)
{
	Sophon_Value v = SOPHON_ARG(0);
	Sophon_Object *obj;

	if (!sophon_value_is_object(v)) {
		sophon_throw(vm, vm->TypeError,
				"Value is not an object");
		return SOPHON_ERR_THROW;
	}

	sophon_value_to_object(vm, v, &obj);
	*retv = obj->protov;

	return SOPHON_OK;
}

static OBJECT_FUNC(getOwnPropertyDescriptor)
{
	Sophon_Value v = SOPHON_ARG(0);
	Sophon_Value namev = SOPHON_ARG(1);
	Sophon_PropDesc desc;
	Sophon_Result r;

	if ((r = sophon_value_prop_desc(vm, v, namev, &desc)) != SOPHON_OK) {
		sophon_value_set_undefined(vm, retv);
		return SOPHON_OK;
	}

	*retv = prop_desc_to_obj(vm, &desc);
	return SOPHON_OK;
}

static Sophon_Int
array_append_prop_name (Sophon_VM *vm, Sophon_Property *prop,
			Sophon_Ptr data)
{
	Sophon_Array *arr = (Sophon_Array*)data;
	Sophon_U32 id = sophon_array_get_length(vm, arr);
	Sophon_Value namev;

	sophon_value_set_string(vm, &namev, prop->name);
	sophon_array_set_item(vm, arr, id, namev);

	return 1;
}

static OBJECT_FUNC(getOwnPropertyNames)
{
	Sophon_Value v = SOPHON_ARG(0);
	Sophon_Array *arr;
	Sophon_Result r;

	arr = sophon_array_create(vm);

	r = sophon_value_for_each_prop(vm, v, array_append_prop_name, arr);
	if (r != 1)
		return r;

	sophon_value_set_array(vm, retv, arr);
	return SOPHON_OK;
}

static OBJECT_FUNC(create)
{
	Sophon_Value protov, propsv;
	Sophon_Object *obj;
	Sophon_Result r;

	protov = SOPHON_ARG(0);
	if (sophon_value_is_null(protov) || !sophon_value_is_object(protov)) {
		sophon_throw(vm, vm->TypeError, "Value is not an object");
		return SOPHON_ERR_THROW;
	}

	obj = sophon_object_create(vm);
	obj->protov = protov;

	propsv = SOPHON_ARG(1);
	if (!sophon_value_is_undefined(propsv)) {
		if ((r = obj_add_props(vm, SOPHON_VALUE_GC(obj), propsv)) != SOPHON_OK)
			return r;
	}

	sophon_value_set_object(vm, retv, obj);
	return SOPHON_OK;
}

static OBJECT_FUNC(defineProperty)
{
	Sophon_Value v = SOPHON_ARG(0);
	Sophon_Value namev, propv;
	Sophon_Result r;

	if (!sophon_value_is_object(v)) {
		sophon_throw(vm, vm->TypeError, "Value is not an object");
		return SOPHON_ERR_THROW;
	}

	namev = SOPHON_ARG(1);
	propv = SOPHON_ARG(2);

	if ((r = obj_add_prop(vm, v, namev, propv)) != SOPHON_OK)
		return r;

	*retv = v;
	return SOPHON_OK;
}

static OBJECT_FUNC(defineProperties)
{
	Sophon_Value v = SOPHON_ARG(0);
	Sophon_Value propsv = SOPHON_ARG(1);
	Sophon_Result r;

	if (!sophon_value_is_object(v)) {
		sophon_throw(vm, vm->TypeError, "Value is not an object");
		return SOPHON_ERR_THROW;
	}

	if ((r = obj_add_props(vm, v, propsv)) != SOPHON_OK)
		return r;

	*retv = v;
	return SOPHON_OK;
}

static Sophon_Int
obj_seal_prop(Sophon_VM *vm, Sophon_Property *prop, Sophon_Ptr data)
{
	prop->attrs &= ~SOPHON_PROP_ATTR_CONFIGURABLE;
	return 1;
}

static OBJECT_FUNC(seal)
{
	Sophon_Object *obj;
	Sophon_Value v = SOPHON_ARG(0);

	if (!sophon_value_is_object(v)) {
		sophon_throw(vm, vm->TypeError, "Value is not an object");
		return SOPHON_ERR_THROW;
	}
	sophon_value_to_object(vm, v, &obj);

	sophon_value_for_each_prop(vm, v, obj_seal_prop, NULL);
	obj->gc_flags &= ~SOPHON_GC_FL_EXTENSIBLE;

	*retv = v;
	return SOPHON_OK;
}

static Sophon_Int
obj_freeze_prop(Sophon_VM *vm, Sophon_Property *prop, Sophon_Ptr data)
{
	prop->attrs &= ~(SOPHON_PROP_ATTR_CONFIGURABLE|SOPHON_PROP_ATTR_WRITABLE);
	return 1;
}

static OBJECT_FUNC(freeze)
{
	Sophon_Value v = SOPHON_ARG(0);
	Sophon_Object *obj;

	if (!sophon_value_is_object(v)) {
		sophon_throw(vm, vm->TypeError, "Value is not an object");
		return SOPHON_ERR_THROW;
	}
	sophon_value_to_object(vm, v, &obj);

	sophon_value_for_each_prop(vm, v, obj_freeze_prop, NULL);
	obj->gc_flags &= ~SOPHON_GC_FL_EXTENSIBLE;

	*retv = v;
	return SOPHON_OK;
}

static OBJECT_FUNC(preventExtensions)
{
	Sophon_Value v = SOPHON_ARG(0);
	Sophon_Object *obj;

	if (!sophon_value_is_object(v)) {
		sophon_throw(vm, vm->TypeError, "Value is not an object");
		return SOPHON_ERR_THROW;
	}
	sophon_value_to_object(vm, v, &obj);

	obj->gc_flags &= ~SOPHON_GC_FL_EXTENSIBLE;

	*retv = v;
	return SOPHON_OK;
}

static Sophon_Int
obj_is_sealed_prop(Sophon_VM *vm, Sophon_Property *prop, Sophon_Ptr data)
{
	if (prop->attrs & SOPHON_PROP_ATTR_CONFIGURABLE)
		return 0;

	return 1;
}

static OBJECT_FUNC(isSealed)
{
	Sophon_Value v = SOPHON_ARG(0);
	Sophon_Object *obj;
	Sophon_Bool b;

	if (!sophon_value_is_object(v)) {
		sophon_throw(vm, vm->TypeError, "Value is not an object");
		return SOPHON_ERR_THROW;
	}
	sophon_value_to_object(vm, v, &obj);

	if (obj->gc_flags & SOPHON_GC_FL_EXTENSIBLE) {
		b = SOPHON_FALSE;
	} else {
		b = sophon_value_for_each_prop(vm, v, obj_is_sealed_prop, NULL) ?
			SOPHON_TRUE : SOPHON_FALSE;
	}

	sophon_value_set_bool(vm, retv, b);
	return SOPHON_OK;
}

static Sophon_Int
obj_is_frozen_prop(Sophon_VM *vm, Sophon_Property *prop, Sophon_Ptr data)
{
	if (prop->attrs &
				(SOPHON_PROP_ATTR_CONFIGURABLE|SOPHON_PROP_ATTR_WRITABLE))
		return 0;

	return 1;
}

static OBJECT_FUNC(isFrozen)
{
	Sophon_Value v = SOPHON_ARG(0);
	Sophon_Object *obj;
	Sophon_Bool b;

	if (!sophon_value_is_object(v)) {
		sophon_throw(vm, vm->TypeError, "Value is not an object");
		return SOPHON_ERR_THROW;
	}
	sophon_value_to_object(vm, v, &obj);

	if (obj->gc_flags & SOPHON_GC_FL_EXTENSIBLE) {
		b = SOPHON_FALSE;
	} else {
		b = sophon_value_for_each_prop(vm, v, obj_is_frozen_prop, NULL) ?
			SOPHON_TRUE : SOPHON_FALSE;
	}

	sophon_value_set_bool(vm, retv, b);
	return SOPHON_OK;
}

static OBJECT_FUNC(isExtensible)
{
	Sophon_Value v = SOPHON_ARG(0);
	Sophon_Object *obj;
	Sophon_Bool b;

	if (!sophon_value_is_object(v)) {
		sophon_throw(vm, vm->TypeError, "Value is not an object");
		return SOPHON_ERR_THROW;
	}
	sophon_value_to_object(vm, v, &obj);

	if (obj->gc_flags & SOPHON_GC_FL_EXTENSIBLE) {
		b = SOPHON_TRUE;
	} else {
		b = SOPHON_FALSE;
	}

	sophon_value_set_bool(vm, retv, b);
	return SOPHON_OK;
}

static Sophon_Int
array_append_prop_key (Sophon_VM *vm, Sophon_Property *prop,
			Sophon_Ptr data)
{
	Sophon_Array *arr = (Sophon_Array*)data;
	Sophon_U32 id;
	Sophon_Value namev;

	if (!(prop->attrs & SOPHON_PROP_ATTR_ENUMERABLE))
		return 1;

	id = sophon_array_get_length(vm, arr);
	sophon_value_set_string(vm, &namev, prop->name);
	sophon_array_set_item(vm, arr, id, namev);

	return 1;
}

static OBJECT_FUNC(keys)
{
	Sophon_Value v = SOPHON_ARG(0);
	Sophon_Array *arr;
	Sophon_Result r;

	arr = sophon_array_create(vm);

	r = sophon_value_for_each_prop(vm, v, array_append_prop_key, arr);
	if (r != 1)
		return r;

	sophon_value_set_array(vm, retv, arr);
	return SOPHON_OK;
}

static OBJECT_FUNC(toString)
{
	Sophon_String *str;

	if (sophon_value_is_undefined(thisv)) {
		str = sophon_string_from_utf8_cstr(vm, "[object Undefined]");
	} else if (sophon_value_is_null(thisv)) {
		str = sophon_string_from_utf8_cstr(vm, "[object Null]");
	} else {
		Sophon_Value cv;
		Sophon_Result r;
		Sophon_String *head_str;
		Sophon_String *tail_str;
		Sophon_String *type_str;

		r = sophon_value_get(vm, thisv, SOPHON_VALUE_GC(vm->__class_str),
					&cv, SOPHON_NONE);
		if (r == SOPHON_OK) {
			if ((r = sophon_value_to_string(vm, cv, &type_str)) != SOPHON_OK)
				return r;
		} else {
			type_str = sophon_string_from_utf8_cstr(vm, "Object");
		}

		head_str = sophon_string_from_utf8_cstr(vm, "[object ");
		tail_str = vm->rs_str;

		str = sophon_string_concat(vm, head_str, type_str);
		str = sophon_string_concat(vm, str, tail_str);
	}

	sophon_value_set_string(vm, retv, str);

	return SOPHON_OK;
}

static OBJECT_FUNC(toLocaleString)
{
	Sophon_Value cv;
	Sophon_Result r;

	r = sophon_value_get(vm, thisv, SOPHON_VALUE_GC(vm->toString_str), &cv,
				SOPHON_FL_OWN);
	if (r != SOPHON_OK) {
		sophon_throw(vm, vm->TypeError,
				"\"toString\" property has not been defined");
		return SOPHON_ERR_THROW;
	}

	return sophon_value_call(vm, cv, thisv, NULL, 0, retv, 0);
}

static OBJECT_FUNC(valueOf)
{
	if (SOPHON_VALUE_IS_OBJECT(thisv)) {
		Sophon_Object *obj = SOPHON_VALUE_GET_OBJECT(thisv);

		if (sophon_value_is_object(obj->primv))
			*retv = thisv;
		else
			*retv = obj->primv;
	} else {
		*retv = thisv;
	}

	return SOPHON_OK;
}

static OBJECT_FUNC(hasOwnProperty)
{
	Sophon_Value namev = SOPHON_ARG(0);
	Sophon_Result r;
	Sophon_Bool b;

	if ((r = sophon_value_prop_desc(vm, thisv, namev, NULL)) < 0)
		return r;

	b = (r == SOPHON_OK);
	sophon_value_set_bool(vm, retv, b);

	return SOPHON_OK;
}

static OBJECT_FUNC(isPrototypeOf)
{
	Sophon_Value v = SOPHON_ARG(0);
	Sophon_Object *this_obj, *obj;
	Sophon_Bool b = SOPHON_FALSE;
	Sophon_Result r;

	if (sophon_value_is_object(v)) {
		if ((r = sophon_value_to_object(vm, v, &obj)) != SOPHON_OK)
			return r;
		if ((r = sophon_value_to_object(vm, thisv, &this_obj)) != SOPHON_OK)
			return r;

		while (1) {
			if (!sophon_value_is_object(obj->protov))
				break;

			if ((r = sophon_value_to_object(vm, obj->protov, &obj))
						!= SOPHON_OK)
				return r;

			if (obj == this_obj) {
				b = SOPHON_TRUE;
				break;
			}
		}
	}

	sophon_value_set_bool(vm, retv, b);

	return SOPHON_OK;
}

static OBJECT_FUNC(propertyIsEnumerable)
{
	Sophon_Value namev = SOPHON_ARG(0);
	Sophon_PropDesc desc;
	Sophon_Result r;
	Sophon_Bool b;

	r = sophon_value_prop_desc(vm, thisv, namev, &desc);
	if (r < 0)
		return r;

	b = (desc.attrs & SOPHON_PROP_ATTR_ENUMERABLE);
	sophon_value_set_bool(vm, retv, b);

	return SOPHON_OK;
}

static const Sophon_Decl
object_prototype_props[] = {
	SOPHON_STRING_PROP([Class], 0, "Object"),
	OBJECT_FUNC_PROP(toString, 0),
	OBJECT_FUNC_PROP(toLocaleString, 0),
	OBJECT_FUNC_PROP(valueOf, 0),
	OBJECT_FUNC_PROP(hasOwnProperty, 1),
	OBJECT_FUNC_PROP(isPrototypeOf, 1),
	OBJECT_FUNC_PROP(propertyIsEnumerable, 1),
	{NULL}
};

static const Sophon_Decl
object_object_props[] = {
	SOPHON_OBJECT_PROP(prototype, 0, object_prototype_props),
	SOPHON_FUNCTION_PROP([Call], GLOBAL_ATTRS, object_call_func, 1),
	OBJECT_FUNC_PROP(getPrototypeOf, 1),
	OBJECT_FUNC_PROP(getOwnPropertyDescriptor, 2),
	OBJECT_FUNC_PROP(getOwnPropertyNames, 1),
	OBJECT_FUNC_PROP(create, 2),
	OBJECT_FUNC_PROP(defineProperty, 3),
	OBJECT_FUNC_PROP(defineProperties, 2),
	OBJECT_FUNC_PROP(seal, 1),
	OBJECT_FUNC_PROP(freeze, 1),
	OBJECT_FUNC_PROP(preventExtensions, 1),
	OBJECT_FUNC_PROP(isSealed, 1),
	OBJECT_FUNC_PROP(isFrozen, 1),
	OBJECT_FUNC_PROP(isExtensible, 1),
	OBJECT_FUNC_PROP(keys, 1),
	{NULL}
};

