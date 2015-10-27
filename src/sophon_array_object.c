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

#define ARRAY_FUNC_PROP(name, argc)\
	    SOPHON_FUNCTION_PROP(name, GLOBAL_ATTRS, array_##name##_func, argc)
#define ARRAY_FUNC(name)\
	    SOPHON_FUNC(array_##name##_func)

static ARRAY_FUNC(call)
{
	Sophon_U32 len;
	Sophon_Array *arr;
	Sophon_Result r;

	arr = sophon_array_create(vm);

	if (argc == 1) {
		if ((r = sophon_value_to_u32(vm, SOPHON_ARG(0), &len)) != SOPHON_OK)
			return r;
	} else {
		len = argc;
	}

	if ((r = sophon_array_set_length(vm, arr, len)) != SOPHON_OK) {
		return r;
	}

	if (argc > 1) {
		sophon_memcpy(arr->v, argv, argc * sizeof(Sophon_Value));
	}

	sophon_value_set_array(vm, retv, arr);
	return SOPHON_OK;
}

static ARRAY_FUNC(isArray)
{
	Sophon_Bool b = sophon_value_is_array(SOPHON_ARG(0));

	sophon_value_set_bool(vm, retv, b);
	return SOPHON_OK;
}

static ARRAY_FUNC(length_get)
{
	Sophon_Array *arr;
	Sophon_Result r;

	if ((r = sophon_value_to_array(vm, thisv, &arr)) != SOPHON_OK)
		return r;

	sophon_value_set_int(vm, retv, arr->len);
	return SOPHON_OK;
}

static ARRAY_FUNC(length_set)
{
	Sophon_Array *arr;
	Sophon_U32 len;
	Sophon_Result r;

	if ((r = sophon_value_to_array(vm, thisv, &arr)) != SOPHON_OK)
		return r;

	if ((r = sophon_value_to_u32(vm, SOPHON_ARG(0), &len)) != SOPHON_OK)
		return r;

	if ((r = sophon_array_set_length(vm, arr, len)) != SOPHON_OK) {
		return r;
	}

	return SOPHON_OK;
}

static ARRAY_FUNC(join)
{
	Sophon_Array *arr;
	Sophon_Result r;
	Sophon_Value sepv = SOPHON_ARG(0);
	Sophon_Value itemv;
	Sophon_String *str, *sep, *item;
	Sophon_U32 id;

	if ((r = sophon_value_to_array(vm, thisv, &arr)) != SOPHON_OK)
		return r;
	
	if (sophon_value_is_undefined(sepv)) {
		sep = vm->comma_str;
	} else {
		if ((r = sophon_value_to_string(vm, sepv, &sep)) != SOPHON_OK)
			return r;
	}

	str = vm->empty_str;

	for (id = 0; id < arr->len; id++) {
		itemv = arr->v[id];

		if (sophon_value_is_null(itemv) || sophon_value_is_undefined(itemv)
				|| (itemv == SOPHON_ARRAY_UNUSED)) {
			item = vm->empty_str;
		} else {
			if ((r = sophon_value_to_string(vm, itemv, &item)) != SOPHON_OK)
				return r;
		}

		if (id != 0)
			str = sophon_string_concat(vm, str, sep);

		str = sophon_string_concat(vm, str, item);
	}

	sophon_value_set_string(vm, retv, str);

	return SOPHON_OK;
}

static ARRAY_FUNC(toString)
{
	return array_join_func(vm, thisv, argv, argc, retv);
}

static ARRAY_FUNC(toLocaleString)
{
	return array_toString_func(vm, thisv, argv, argc, retv);
}

static ARRAY_FUNC(concat)
{
	Sophon_Array *arr, *narr;
	Sophon_U32 len, nlen;
	Sophon_Result r;

	if ((r = sophon_value_to_array(vm, thisv, &arr)) != SOPHON_OK)
		return r;

	len = sophon_array_get_length(vm, arr);
	nlen = len + argc;

	narr = sophon_array_create(vm);
	if ((r = sophon_array_set_length(vm, narr, nlen)) != SOPHON_OK) {
		return r;
	}

	sophon_memcpy(narr->v, arr->v, len * sizeof(Sophon_Value));
	if (argc) {
		sophon_memcpy(narr->v + len, argv, argc * sizeof(Sophon_Value));
	}

	sophon_value_set_array(vm, retv, narr);
	return SOPHON_OK;
}

static ARRAY_FUNC(pop)
{
	Sophon_Array *arr;
	Sophon_U32 len;
	Sophon_Result r;

	if ((r = sophon_value_to_array(vm, thisv, &arr)) != SOPHON_OK)
		return r;

	len = sophon_array_get_length(vm, arr);
	if (len) {
		sophon_array_get_item(vm, arr, len - 1, retv);
		sophon_array_set_length(vm, arr, len - 1);
	}

	return SOPHON_OK;
}

static ARRAY_FUNC(push)
{
	Sophon_Array *arr;
	Sophon_U32 len;
	Sophon_Result r;

	if ((r = sophon_value_to_array(vm, thisv, &arr)) != SOPHON_OK)
		return r;

	len = sophon_array_get_length(vm, arr);

	if (argc) {
		if ((r = sophon_array_set_length(vm, arr, len + argc)) != SOPHON_OK) {
			return r;
		}

		sophon_memcpy(arr->v + len, argv, sizeof(Sophon_Value) * argc);
	}

	sophon_value_set_int(vm, retv, len + argc);
	return SOPHON_OK;
}

static ARRAY_FUNC(reverse)
{
	Sophon_Array *arr;
	Sophon_U32 len, u, l;
	Sophon_Result r;

	if ((r = sophon_value_to_array(vm, thisv, &arr)) != SOPHON_OK)
		return r;

	len = sophon_array_get_length(vm, arr);
	if (len > 1) {
		u = len - 1;
		l = 0;

		while (1) {
			Sophon_Value t;

			t = arr->v[u];
			arr->v[u] = arr->v[l];
			arr->v[l] = t;

			u--;
			l++;

			if (u <= l)
				break;
		}
	}

	*retv = thisv;
	return SOPHON_OK;
}

static ARRAY_FUNC(shift)
{
	Sophon_Array *arr;
	Sophon_U32 len;
	Sophon_Result r;

	if ((r = sophon_value_to_array(vm, thisv, &arr)) != SOPHON_OK)
		return r;

	len = sophon_array_get_length(vm, arr);
	if (len) {
		sophon_array_get_item(vm, arr, 0, retv);
		sophon_memmove(arr->v, arr->v + 1, (len - 1) * sizeof(Sophon_Value));
		sophon_array_set_length(vm, arr, len - 1);
	}

	return SOPHON_OK;
}

static ARRAY_FUNC(slice)
{
	Sophon_Array *arr, *narr;
	Sophon_Int start, end, len, nlen;
	Sophon_Result r;

	if ((r = sophon_value_to_array(vm, thisv, &arr)) != SOPHON_OK)
		return r;

	len = sophon_array_get_length(vm, arr);

	if ((r = sophon_value_to_int(vm, SOPHON_ARG(0), &start)) != SOPHON_OK)
		return r;

	if (start < 0)
		start = SOPHON_MAX(len + start, 0);
	else
		start = SOPHON_MIN(start, len);

	if (sophon_value_is_undefined(SOPHON_ARG(1))) {
		end = len;
	} else {
		if ((r = sophon_value_to_int(vm, SOPHON_ARG(1), &end)) != SOPHON_OK)
			return r;

		if (end < 0)
			end = SOPHON_MAX(len + end, 0);
		else
			end = SOPHON_MIN(end, len);
	}

	nlen = SOPHON_MAX(SOPHON_MIN(end - start, len), 0);
	narr = sophon_array_create(vm);
	if (nlen) {
		if ((r = sophon_array_set_length(vm, narr, nlen)) != SOPHON_OK)
			return r;
		sophon_memcpy(narr->v, arr->v + start, nlen * sizeof(Sophon_Value));
	}

	sophon_value_set_array(vm, retv, narr);
	return SOPHON_OK;
}

static Sophon_Int
array_cmp_func (Sophon_VM *vm, Sophon_Value v1, Sophon_Value v2,
		Sophon_Ptr arg)
{
	Sophon_Value cmpv = (Sophon_Value)arg;
	Sophon_Result r;
	Sophon_Int i = 0;

	if ((v1 == SOPHON_ARRAY_UNUSED) && (v2 == SOPHON_ARRAY_UNUSED))
		return 0;

	if (v1 == SOPHON_ARRAY_UNUSED)
		return 1;

	if (v2 == SOPHON_ARRAY_UNUSED)
		return -1;

	if (sophon_value_is_undefined(v1) && sophon_value_is_undefined(v2))
		return 0;

	if (sophon_value_is_undefined(v1))
		return 1;

	if (sophon_value_is_undefined(v2))
		return -1;

	if (!sophon_value_is_undefined(cmpv)) {
		Sophon_Value args[2];
		Sophon_Value ret;

		args[0] = v1;
		args[1] = v2;
		r = sophon_value_call(vm, cmpv, SOPHON_VALUE_UNDEFINED, args, 2,
				&ret, 0);
		if (r == SOPHON_OK)
			sophon_value_to_int(vm, ret, &i);
	} else {
		Sophon_String *s1, *s2;

		if ((r = sophon_value_to_string(vm, v1, &s1)) != SOPHON_OK)
			return 0;
		if ((r = sophon_value_to_string(vm, v2, &s2)) != SOPHON_OK)
			return 0;

		i = sophon_string_cmp(vm, s1, s2);
	}

	return i;
}

static ARRAY_FUNC(sort)
{
	Sophon_Array *arr;
	Sophon_Result r;

	if ((r = sophon_value_to_array(vm, thisv, &arr)) != SOPHON_OK)
		return r;

	if ((r = sophon_array_sort(vm, arr, array_cmp_func,
					(Sophon_Ptr)SOPHON_ARG(0))) != SOPHON_OK)
		return r;

	*retv = thisv;
	return SOPHON_OK;
}

static ARRAY_FUNC(splice)
{
	Sophon_Array *arr, *narr;
	Sophon_Int start, del, len, nlen, add, left;
	Sophon_Result r;

	if ((r = sophon_value_to_array(vm, thisv, &arr)) != SOPHON_OK)
		return r;

	len = sophon_array_get_length(vm, arr);

	if ((r = sophon_value_to_int(vm, SOPHON_ARG(0), &start)) != SOPHON_OK)
		return r;

	if (start < 0)
		start = SOPHON_MAX(len + start, 0);
	else
		start = SOPHON_MIN(start, len);

	if ((r = sophon_value_to_int(vm, SOPHON_ARG(1), &del)) != SOPHON_OK)
		return r;

	del = SOPHON_MIN(SOPHON_MAX(del, 0), len - start);
	add = SOPHON_MAX(argc - 2, 0);

	nlen = len - del + add;

	narr = sophon_array_create(vm);
	if ((r = sophon_array_set_length(vm, narr, nlen)) != SOPHON_OK)
		return r;

	if (start)
		sophon_memcpy(narr->v, arr->v, start * sizeof(Sophon_Value));

	if (add)
		sophon_memcpy(narr->v + start, argv + 2, add * sizeof(Sophon_Value));

	left = len - start - del;
	if (left)
		sophon_memcpy(narr->v + start + add, arr->v + start + del,
				left * sizeof(Sophon_Value));

	sophon_value_set_array(vm, retv, narr);
	return SOPHON_OK;
}

static ARRAY_FUNC(unshift)
{
	Sophon_Array *arr;
	Sophon_U32 len;
	Sophon_Result r;

	if ((r = sophon_value_to_array(vm, thisv, &arr)) != SOPHON_OK)
		return r;

	len = sophon_array_get_length(vm, arr);
	if (argc) {
		if ((r = sophon_array_set_length(vm, arr, len + argc)) != SOPHON_OK)
			return r;

		if (len)
			sophon_memmove(arr->v + argc, arr->v, len * sizeof(Sophon_Value));

		sophon_memcpy(arr->v, argv, argc * sizeof(Sophon_Value));
	}

	sophon_value_set_int(vm, retv, len + argc);
	return SOPHON_OK;
}

static ARRAY_FUNC(indexOf)
{
	Sophon_Array *arr;
	Sophon_Value v = SOPHON_ARG(0);
	Sophon_Int from, len, id;
	Sophon_Result r;

	if ((r = sophon_value_to_array(vm, thisv, &arr)) != SOPHON_OK)
		return r;

	len = sophon_array_get_length(vm, arr);

	if (sophon_value_is_undefined(SOPHON_ARG(1))) {
		from = 0;
	} else {
		if ((r = sophon_value_to_int(vm, SOPHON_ARG(1), &from)) != SOPHON_OK)
			return r;

		if (from < 0) {
			from = SOPHON_MAX(len + from, 0);
		}
	}

	for (id = from; id < len; id++) {
		if (arr->v[id] != SOPHON_ARRAY_UNUSED) {
			if (sophon_value_same(vm, arr->v[id], v))
				break;
		}
	}

	if (id >= len)
		id = -1;

	sophon_value_set_int(vm, retv, id);
	return SOPHON_OK;
}

static ARRAY_FUNC(lastIndexOf)
{
	Sophon_Array *arr;
	Sophon_Value v = SOPHON_ARG(0);
	Sophon_Int from, len, id;
	Sophon_Result r;

	if ((r = sophon_value_to_array(vm, thisv, &arr)) != SOPHON_OK)
		return r;

	len = sophon_array_get_length(vm, arr);

	if (sophon_value_is_undefined(SOPHON_ARG(1))) {
		from = len - 1;
	} else {
		if ((r = sophon_value_to_int(vm, SOPHON_ARG(1), &from)) != SOPHON_OK)
			return r;

		if (from < 0) {
			from = len + from;
		} else {
			from = SOPHON_MIN(from, len - 1);
		}
	}

	for (id = from; id >= 0; id--) {
		if (arr->v[id] != SOPHON_ARRAY_UNUSED) {
			if (sophon_value_same(vm, arr->v[id], v))
				break;
		}
	}

	if (id < 0)
		id = -1;

	sophon_value_set_int(vm, retv, id);
	return SOPHON_OK;
}

static ARRAY_FUNC(every)
{
	Sophon_Array *arr;
	Sophon_Value this_arg = SOPHON_ARG(1);
	Sophon_Value cb = SOPHON_ARG(0);
	Sophon_Value item, args[3], ret;
	Sophon_Bool b = SOPHON_TRUE;
	Sophon_Int id;
	Sophon_Result r;

	if ((r = sophon_value_to_array(vm, thisv, &arr)) != SOPHON_OK)
		return r;

	args[2] = thisv;
	sophon_array_for_each(arr, id, item) {
		args[0] = item;
		sophon_value_set_int(vm, &args[1], id);

		if ((r = sophon_value_call(vm, cb, this_arg, args, 3, &ret, 0))
				!= SOPHON_OK)
			return r;

		if (!sophon_value_to_bool(vm, ret)) {
			b = SOPHON_FALSE;
			break;
		}
	}

	sophon_value_set_bool(vm, retv, b);
	return SOPHON_OK;
}

static ARRAY_FUNC(some)
{
	Sophon_Array *arr;
	Sophon_Value this_arg = SOPHON_ARG(1);
	Sophon_Value cb = SOPHON_ARG(0);
	Sophon_Value item, args[3], ret;
	Sophon_Bool b = SOPHON_FALSE;
	Sophon_Int id;
	Sophon_Result r;

	if ((r = sophon_value_to_array(vm, thisv, &arr)) != SOPHON_OK)
		return r;

	args[2] = thisv;
	sophon_array_for_each(arr, id, item) {
		args[0] = item;
		sophon_value_set_int(vm, &args[1], id);

		if ((r = sophon_value_call(vm, cb, this_arg, args, 3, &ret, 0))
				!= SOPHON_OK)
			return r;

		if (sophon_value_to_bool(vm, ret)) {
			b = SOPHON_TRUE;
			break;
		}
	}

	sophon_value_set_bool(vm, retv, b);
	return SOPHON_OK;

}

static ARRAY_FUNC(forEach)
{
	Sophon_Array *arr;
	Sophon_Value this_arg = SOPHON_ARG(1);
	Sophon_Value cb = SOPHON_ARG(0);
	Sophon_Value item, args[3], ret;
	Sophon_Int id;
	Sophon_Result r;

	if ((r = sophon_value_to_array(vm, thisv, &arr)) != SOPHON_OK)
		return r;

	args[2] = thisv;
	sophon_array_for_each(arr, id, item) {
		args[0] = item;
		sophon_value_set_int(vm, &args[1], id);

		if ((r = sophon_value_call(vm, cb, this_arg, args, 3, &ret, 0))
				!= SOPHON_OK)
			return r;
	}

	return SOPHON_OK;
}

static ARRAY_FUNC(map)
{
	Sophon_Array *arr, *narr;
	Sophon_Value this_arg = SOPHON_ARG(1);
	Sophon_Value cb = SOPHON_ARG(0);
	Sophon_Value item, args[3], ret;
	Sophon_Int id, len;
	Sophon_Result r;

	if ((r = sophon_value_to_array(vm, thisv, &arr)) != SOPHON_OK)
		return r;

	len = sophon_array_get_length(vm, arr);
	narr = sophon_array_create(vm);
	if ((r = sophon_array_set_length(vm, narr, len)) != SOPHON_OK)
		return r;

	args[2] = thisv;
	sophon_array_for_each(arr, id, item) {
		args[0] = item;
		sophon_value_set_int(vm, &args[1], id);

		if ((r = sophon_value_call(vm, cb, this_arg, args, 3, &ret, 0))
				!= SOPHON_OK)
			return r;

		narr->v[id] = ret;
	}

	sophon_value_set_array(vm, retv, narr);
	return SOPHON_OK;
}

static ARRAY_FUNC(filter)
{
	Sophon_Array *arr, *narr;
	Sophon_Value this_arg = SOPHON_ARG(1);
	Sophon_Value cb = SOPHON_ARG(0);
	Sophon_Value item, args[3], ret;
	Sophon_Int id, nid;
	Sophon_Result r;

	if ((r = sophon_value_to_array(vm, thisv, &arr)) != SOPHON_OK)
		return r;

	narr = sophon_array_create(vm);

	args[2] = thisv;
	nid = 0;
	sophon_array_for_each(arr, id, item) {
		args[0] = item;
		sophon_value_set_int(vm, &args[1], id);

		if ((r = sophon_value_call(vm, cb, this_arg, args, 3, &ret, 0))
				!= SOPHON_OK)
			return r;

		if (sophon_value_to_bool(vm, ret)) {
			sophon_array_set_item(vm, narr, nid++, item);
		}
	}

	sophon_value_set_array(vm, retv, narr);
	return SOPHON_OK;
}

static ARRAY_FUNC(reduce)
{
	Sophon_Array *arr;
	Sophon_Value cb = SOPHON_ARG(0);
	Sophon_Value init = SOPHON_ARG(1);
	Sophon_Value item, args[4], ret;
	Sophon_Int id, len;
	Sophon_Result r;

	if ((r = sophon_value_to_array(vm, thisv, &arr)) != SOPHON_OK)
		return r;

	len = sophon_array_get_length(vm, arr);

	if (sophon_value_is_undefined(init)) {
		if (!len) {
			sophon_throw(vm, vm->TypeError,
					"Array's length is 0 and not initial value set");
			return SOPHON_ERR_THROW;
		}

		sophon_array_for_each(arr, id, item) {
			init = item;
			break;
		}

		if (sophon_value_is_undefined(init)) {
			sophon_throw(vm, vm->TypeError,
					"No value set in the array");
			return SOPHON_ERR_THROW;
		}

		id++;
	} else {
		id = 0;
	}

	args[0] = init;
	args[3] = thisv;
	for (; id < len; id++) {
		item = arr->v[id];
		if (item != SOPHON_ARRAY_UNUSED) {
			args[1] = item;
			sophon_value_set_int(vm, &args[2], id);

			if ((r = sophon_value_call(vm, cb, SOPHON_VALUE_UNDEFINED,
							args, 4, &ret, 0)) != SOPHON_OK)
				return r;
			args[0] = ret;
		}
	}

	*retv = args[0];
	return SOPHON_OK;
}

static ARRAY_FUNC(reduceRight)
{
	Sophon_Array *arr;
	Sophon_Value cb = SOPHON_ARG(0);
	Sophon_Value init = SOPHON_ARG(1);
	Sophon_Value item, args[4], ret;
	Sophon_Int id, len;
	Sophon_Result r;

	if ((r = sophon_value_to_array(vm, thisv, &arr)) != SOPHON_OK)
		return r;

	len = sophon_array_get_length(vm, arr);

	if (sophon_value_is_undefined(init)) {
		if (!len) {
			sophon_throw(vm, vm->TypeError,
					"Array's length is 0 and no initial value set");
			return SOPHON_ERR_THROW;
		}

		sophon_array_for_each_r(arr, id, item) {
			init = item;
			break;
		}

		if (sophon_value_is_undefined(init)) {
			sophon_throw(vm, vm->TypeError,
					"No value set in the array");
			return SOPHON_ERR_THROW;
		}

		id--;
	} else {
		id = len - 1;
	}

	args[0] = init;
	args[3] = thisv;
	for (; id >= 0; id--) {
		item = arr->v[id];
		if (item != SOPHON_ARRAY_UNUSED) {
			args[1] = item;
			sophon_value_set_int(vm, &args[2], id);

			if ((r = sophon_value_call(vm, cb, SOPHON_VALUE_UNDEFINED,
							args, 4, &ret, 0)) != SOPHON_OK)
				return r;
			args[0] = ret;
		}
	}

	*retv = args[0];
	return SOPHON_OK;
}

static const Sophon_Decl
array_prototype_props[] = {
	SOPHON_STRING_PROP([Class], 0, "Array"),
	SOPHON_ACCESSOR_PROP(length, 0, array_length_get_func,
			array_length_set_func),
	ARRAY_FUNC_PROP(toString, 0),
	ARRAY_FUNC_PROP(toLocaleString, 0),
	ARRAY_FUNC_PROP(concat, 1),
	ARRAY_FUNC_PROP(join, 1),
	ARRAY_FUNC_PROP(pop, 0),
	ARRAY_FUNC_PROP(push, 1),
	ARRAY_FUNC_PROP(reverse, 0),
	ARRAY_FUNC_PROP(shift, 0),
	ARRAY_FUNC_PROP(slice, 2),
	ARRAY_FUNC_PROP(sort, 1),
	ARRAY_FUNC_PROP(splice, 2),
	ARRAY_FUNC_PROP(unshift, 1),
	ARRAY_FUNC_PROP(indexOf, 1),
	ARRAY_FUNC_PROP(lastIndexOf, 1),
	ARRAY_FUNC_PROP(every, 1),
	ARRAY_FUNC_PROP(some, 1),
	ARRAY_FUNC_PROP(forEach, 1),
	ARRAY_FUNC_PROP(map, 1),
	ARRAY_FUNC_PROP(filter, 1),
	ARRAY_FUNC_PROP(reduce, 1),
	ARRAY_FUNC_PROP(reduceRight, 1),
	{NULL}
};

static const Sophon_Decl
array_object_props[] = {
	SOPHON_OBJECT_PROP(prototype, 0, array_prototype_props),
	SOPHON_FUNCTION_PROP([Call], GLOBAL_ATTRS, array_call_func, 1),
	ARRAY_FUNC_PROP(isArray, 1),
	{NULL}
};

