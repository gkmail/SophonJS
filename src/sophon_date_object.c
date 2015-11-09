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

#define DATE_FUNC_PROP(name, argc)\
	SOPHON_FUNCTION_PROP(name, GLOBAL_ATTRS, date_##name##_func, argc)
#define DATE_FUNC(name)\
	SOPHON_FUNC(date_##name##_func)

static Sophon_Result
get_date_args (Sophon_VM *vm, Sophon_Value *argv, Sophon_Int argc,
			Sophon_Date *date)
{
	Sophon_Number d;
	Sophon_Int n;
	Sophon_Result r;

	sophon_memset(date, 0, sizeof(Sophon_Date));
	date->mday = 1;

	if ((r = sophon_value_to_number(vm, argv[0], &d)) != SOPHON_OK)
		return r;
	if (!sophon_isnan(d) && (d >= 0) && (d <= 99)) {
		date->year = d;
	} else {
		date->year = d - 1900;
	}
	if (argc < 2) return SOPHON_OK;

	if ((r = sophon_value_to_int(vm, argv[1], &n)) != SOPHON_OK)
		return r;
	date->mon = n;
	if (argc < 3) return SOPHON_OK;

	if ((r = sophon_value_to_int(vm, argv[2], &n)) != SOPHON_OK)
		return r;
	date->mday = n;
	if (argc < 4) return SOPHON_OK;

	if ((r = sophon_value_to_int(vm, argv[3], &n)) != SOPHON_OK)
		return r;
	date->hour = n;
	if (argc < 5) return SOPHON_OK;

	if ((r = sophon_value_to_int(vm, argv[4], &n)) != SOPHON_OK)
		return r;
	date->min = n;
	if (argc < 6) return SOPHON_OK;

	if ((r = sophon_value_to_int(vm, argv[5], &n)) != SOPHON_OK)
		return r;
	date->sec = n;
	if (argc < 7) return SOPHON_OK;

	if ((r = sophon_value_to_int(vm, argv[6], &n)) != SOPHON_OK)
		return r;
	date->msec = n;

	return SOPHON_OK;
}

static DATE_FUNC(call)
{
	Sophon_Time time;
	Sophon_Date date;
	Sophon_String *str;
	Sophon_Result r;

	if (sophon_value_is_undefined(thisv)) {
		if (argc == 0) {
			if ((r = sophon_time_now(vm, &time)) != SOPHON_OK) {
				sophon_throw(vm, vm->TypeError,
						"Cannot get current time");
				return r;
			}

			if ((r = sophon_time_to_date(vm, &time, SOPHON_DATE_LOCAL, &date))
						!= SOPHON_OK) {
				sophon_throw(vm, vm->TypeError,
						"Cannot convert time to date");
				return r;
			}
		} else {
			if ((r = get_date_args(vm, argv, argc, &date)) != SOPHON_OK)
				return r;
		}

		str = sophon_date_to_string(vm, SOPHON_DATE_FL_FULL, &date);

		sophon_value_set_string(vm, retv, str);
	} else {
		Sophon_Object *obj;

		if (argc == 0) {
			if ((r = sophon_time_now(vm, &time)) != SOPHON_OK) {
				sophon_throw(vm, vm->TypeError,
						"Cannot get current time");
				return r;
			}
		} else if (argc == 1) {
			Sophon_Value pv;

			r = sophon_value_to_prim(vm, SOPHON_ARG(0), &pv,
						SOPHON_PRIM_UNDEF);
			if (r != SOPHON_OK)
				return r;

			if (sophon_value_is_string(pv)) {
				Sophon_String *str;

				if ((r = sophon_value_to_string(vm, pv, &str)) != SOPHON_OK)
					return r;

				if ((r = sophon_time_parse(vm, str, &time)) != SOPHON_OK) {
					time = SOPHON_NAN;
				}
			} else {
				Sophon_Number n;

				if ((r = sophon_value_to_number(vm, pv, &n)) != SOPHON_OK)
					return r;

				if (sophon_isinf(n))
					time = SOPHON_NAN;
				else
					time = sophon_trunc(n);
			}
		} else {
			if ((r = get_date_args(vm, argv, argc, &date)) != SOPHON_OK)
				return r;

			if ((r = sophon_date_to_time(vm, &date, &time)) != SOPHON_OK) {
				sophon_throw(vm, vm->TypeError,
						"Cannot convert date to time");
				return r;
			}
		}

		if ((r = sophon_value_to_object(vm, thisv, &obj)) != SOPHON_OK)
			return r;

		sophon_value_set_number(vm, &obj->primv, time);
	}

	return SOPHON_OK;
}

static DATE_FUNC(parse)
{
	Sophon_String *str;
	Sophon_Time time;
	Sophon_Result r;

	if ((r = sophon_value_to_string(vm, SOPHON_ARG(0), &str)) != SOPHON_OK)
		return r;

	if ((r = sophon_time_parse(vm, str, &time)) != SOPHON_OK) {
		time = SOPHON_NAN;
	}

	sophon_value_set_number(vm, retv, time);

	return SOPHON_OK;
}

static DATE_FUNC(UTC)
{
	Sophon_Date date;
	Sophon_Time time;
	Sophon_Result r;

	if ((r = get_date_args(vm, argv, argc, &date)) != SOPHON_OK)
		return r;

	if ((r = sophon_date_to_time(vm, &date, &time))	!= SOPHON_OK) {
		time = SOPHON_NAN;
	}

	sophon_value_set_number(vm, retv, time);

	return SOPHON_OK;
}

#define DATE_TOSTR_FUNC(name, type, mode) \
static DATE_FUNC(name)\
{\
	Sophon_Time time;\
	Sophon_Date date;\
	Sophon_String *str;\
	Sophon_Result r;\
	if (!sophon_value_is_number(thisv)) {\
		sophon_throw(vm, vm->TypeError, "Value is not a number");\
		return SOPHON_ERR_THROW;\
	}\
	if ((r = sophon_value_to_number(vm, thisv, &time)) != SOPHON_OK)\
		return r;\
	if ((r = sophon_time_to_date(vm, &time, type, &date))\
				!= SOPHON_OK) {\
		sophon_throw(vm, vm->TypeError, "Cannot convert time to date");\
		return r;\
	}\
	str = sophon_date_to_string(vm, mode, &date);\
	sophon_value_set_string(vm, retv, str);\
	return SOPHON_OK;\
}

static DATE_FUNC(valueOf)
{
	Sophon_Time time;
	Sophon_Result r;

	if (!sophon_value_is_number(thisv)) {
		sophon_throw(vm, vm->TypeError, "Value is not a number");
		return SOPHON_ERR_THROW;
	}

	if ((r = sophon_value_to_number(vm, thisv, &time)) != SOPHON_OK)
		return r;

	sophon_value_set_number(vm, retv, time);
	return SOPHON_OK;
}

static DATE_FUNC(getTime)
{
	return date_valueOf_func(vm, thisv, argv, argc, retv);
}

DATE_TOSTR_FUNC(toString, SOPHON_DATE_LOCAL, SOPHON_DATE_FL_FULL)
DATE_TOSTR_FUNC(toDateString, SOPHON_DATE_LOCAL, SOPHON_DATE_FL_DATE)
DATE_TOSTR_FUNC(toTimeString, SOPHON_DATE_LOCAL, SOPHON_DATE_FL_TIME)
DATE_TOSTR_FUNC(toUTCString, SOPHON_DATE_UTC,
			SOPHON_DATE_FL_FULL|SOPHON_DATE_FL_UTC)
DATE_TOSTR_FUNC(toISOString, SOPHON_DATE_UTC,
			SOPHON_DATE_FL_FULL|SOPHON_DATE_FL_UTC|SOPHON_DATE_FL_ISO)

static DATE_FUNC(toLocaleString)
{
	return date_toString_func(vm, thisv, argv, argc, retv);
}

static DATE_FUNC(toLocaleDateString)
{
	return date_toDateString_func(vm, thisv, argv, argc, retv);
}

static DATE_FUNC(toLocaleTimeString)
{
	return date_toTimeString_func(vm, thisv, argv, argc, retv);
}

#define DATE_GET_FUNC(name, type, field) \
static DATE_FUNC(name)\
{\
	Sophon_Time time;\
	Sophon_Date date;\
	Sophon_Result r;\
	if ((r = sophon_value_to_number(vm, thisv, &time)) != SOPHON_OK)\
		return r;\
	if ((r = sophon_time_to_date(vm, &time, type, &date)) != SOPHON_OK)\
		return r;\
	sophon_value_set_int(vm, retv, date.field);\
	return SOPHON_OK;\
}

DATE_GET_FUNC(getFullYear, SOPHON_DATE_LOCAL, year+1900)
DATE_GET_FUNC(getUTCFullYear, SOPHON_DATE_UTC, year+1900)
DATE_GET_FUNC(getMonth, SOPHON_DATE_LOCAL, mon)
DATE_GET_FUNC(getUTCMonth, SOPHON_DATE_UTC, mon)
DATE_GET_FUNC(getDate, SOPHON_DATE_LOCAL, mday)
DATE_GET_FUNC(getUTCDate, SOPHON_DATE_UTC, mday)
DATE_GET_FUNC(getDay, SOPHON_DATE_LOCAL, wday)
DATE_GET_FUNC(getUTCDay, SOPHON_DATE_UTC, wday)
DATE_GET_FUNC(getHours, SOPHON_DATE_LOCAL, hour)
DATE_GET_FUNC(getUTCHours, SOPHON_DATE_UTC, hour)
DATE_GET_FUNC(getMinutes, SOPHON_DATE_LOCAL, min)
DATE_GET_FUNC(getUTCMinutes, SOPHON_DATE_UTC, min)
DATE_GET_FUNC(getSeconds, SOPHON_DATE_LOCAL, sec)
DATE_GET_FUNC(getUTCSeconds, SOPHON_DATE_UTC, sec)
DATE_GET_FUNC(getMilliseconds, SOPHON_DATE_LOCAL, msec)
DATE_GET_FUNC(getUTCMilliseconds, SOPHON_DATE_UTC, msec)

static DATE_FUNC(getTimezoneOffset)
{
	sophon_value_set_int(vm, retv, -vm->tz_offset/60000);
	return SOPHON_OK;
}

static DATE_FUNC(setTime)
{
	Sophon_Object *obj;
	Sophon_Time time;
	Sophon_Result r;

	if ((r = sophon_value_to_object(vm, thisv, &obj)) != SOPHON_OK)
		return r;

	if ((r = sophon_value_to_number(vm, SOPHON_ARG(0), &time)) != SOPHON_OK)
		return r;

	sophon_value_set_number(vm, &obj->primv, time);
	return SOPHON_OK;
}

#define DATE_SET_FUNC(name, type, field) \
static DATE_FUNC(name)\
{\
	Sophon_Object *obj;\
	Sophon_Time time;\
	Sophon_Date date;\
	Sophon_Int i;\
	Sophon_Result r;\
	if ((r = sophon_value_to_object(vm, thisv, &obj)) != SOPHON_OK)\
		return r;\
	if ((r = sophon_value_to_number(vm, thisv, &time)) != SOPHON_OK)\
		return r;\
	if ((r = sophon_value_to_int(vm, SOPHON_ARG(0), &i)) != SOPHON_OK)\
		return r;\
	if ((r = sophon_time_to_date(vm, &time, type, &date)) != SOPHON_OK) {\
		sophon_throw(vm, vm->TypeError, "Cannot convert time to date");\
		return r;\
	}\
	date.field;\
	if ((r = sophon_date_to_time(vm, &date, &time)) != SOPHON_OK) {\
		sophon_throw(vm, vm->TypeError, "Cannot convert date to time");\
		return r;\
	}\
	sophon_value_set_number(vm, &obj->primv, time);\
	return SOPHON_OK;\
}

#define DATE_SET_FUNC_2(name, type, f1, f2) \
static DATE_FUNC(name)\
{\
	Sophon_Object *obj;\
	Sophon_Time time;\
	Sophon_Date date;\
	Sophon_Int i;\
	Sophon_Result r;\
	if ((r = sophon_value_to_object(vm, thisv, &obj)) != SOPHON_OK)\
		return r;\
	if ((r = sophon_value_to_number(vm, thisv, &time)) != SOPHON_OK)\
		return r;\
	if ((r = sophon_value_to_int(vm, SOPHON_ARG(0), &i)) != SOPHON_OK)\
		return r;\
	if ((r = sophon_time_to_date(vm, &time, type, &date)) != SOPHON_OK) {\
		sophon_throw(vm, vm->TypeError, "Cannot convert time to date");\
		return r;\
	}\
	date.f1;\
	if (argc > 1) {\
		if ((r = sophon_value_to_int(vm, SOPHON_ARG(1), &i)) != SOPHON_OK)\
			return r;\
		date.f2;\
	}\
	if ((r = sophon_date_to_time(vm, &date, &time)) != SOPHON_OK) {\
		sophon_throw(vm, vm->TypeError, "Cannot convert date to time");\
		return r;\
	}\
	sophon_value_set_number(vm, &obj->primv, time);\
	return SOPHON_OK;\
}

#define DATE_SET_FUNC_3(name, type, f1, f2, f3) \
static DATE_FUNC(name)\
{\
	Sophon_Object *obj;\
	Sophon_Time time;\
	Sophon_Date date;\
	Sophon_Int i;\
	Sophon_Result r;\
	if ((r = sophon_value_to_object(vm, thisv, &obj)) != SOPHON_OK)\
		return r;\
	if ((r = sophon_value_to_number(vm, thisv, &time)) != SOPHON_OK)\
		return r;\
	if ((r = sophon_value_to_int(vm, SOPHON_ARG(0), &i)) != SOPHON_OK)\
		return r;\
	if ((r = sophon_time_to_date(vm, &time, type, &date)) != SOPHON_OK) {\
		sophon_throw(vm, vm->TypeError, "Cannot convert time to date");\
		return r;\
	}\
	date.f1;\
	if (argc > 1) {\
		if ((r = sophon_value_to_int(vm, SOPHON_ARG(1), &i)) != SOPHON_OK)\
			return r;\
		date.f2;\
	}\
	if (argc > 2) {\
		if ((r = sophon_value_to_int(vm, SOPHON_ARG(2), &i)) != SOPHON_OK)\
			return r;\
		date.f3;\
	}\
	if ((r = sophon_date_to_time(vm, &date, &time)) != SOPHON_OK) {\
		sophon_throw(vm, vm->TypeError, "Cannot convert date to time");\
		return r;\
	}\
	sophon_value_set_number(vm, &obj->primv, time);\
	return SOPHON_OK;\
}

#define DATE_SET_FUNC_4(name, type, f1, f2, f3, f4) \
static DATE_FUNC(name)\
{\
	Sophon_Object *obj;\
	Sophon_Time time;\
	Sophon_Date date;\
	Sophon_Int i;\
	Sophon_Result r;\
	if ((r = sophon_value_to_object(vm, thisv, &obj)) != SOPHON_OK)\
		return r;\
	if ((r = sophon_value_to_number(vm, thisv, &time)) != SOPHON_OK)\
		return r;\
	if ((r = sophon_value_to_int(vm, SOPHON_ARG(0), &i)) != SOPHON_OK)\
		return r;\
	if ((r = sophon_time_to_date(vm, &time, type, &date)) != SOPHON_OK) {\
		sophon_throw(vm, vm->TypeError, "Cannot convert time to date");\
		return r;\
	}\
	date.f1;\
	if (argc > 1) {\
		if ((r = sophon_value_to_int(vm, SOPHON_ARG(1), &i)) != SOPHON_OK)\
			return r;\
		date.f2;\
	}\
	if (argc > 2) {\
		if ((r = sophon_value_to_int(vm, SOPHON_ARG(2), &i)) != SOPHON_OK)\
			return r;\
		date.f3;\
	}\
	if (argc > 3) {\
		if ((r = sophon_value_to_int(vm, SOPHON_ARG(3), &i)) != SOPHON_OK)\
			return r;\
		date.f4;\
	}\
	if ((r = sophon_date_to_time(vm, &date, &time)) != SOPHON_OK) {\
		sophon_throw(vm, vm->TypeError, "Cannot convert date to time");\
		return r;\
	}\
	sophon_value_set_number(vm, &obj->primv, time);\
	return SOPHON_OK;\
}

DATE_SET_FUNC(setMilliseconds, SOPHON_DATE_LOCAL, msec = i)
DATE_SET_FUNC(setUTCMilliseconds, SOPHON_DATE_UTC, msec = i)
DATE_SET_FUNC_2(setSeconds, SOPHON_DATE_LOCAL, sec = i, msec = i)
DATE_SET_FUNC_2(setUTCSeconds, SOPHON_DATE_UTC, sec = i, msec = i)
DATE_SET_FUNC_3(setMinutes, SOPHON_DATE_LOCAL, min = i, sec = i, msec = i)
DATE_SET_FUNC_3(setUTCMinutes, SOPHON_DATE_UTC, min = i, sec = i, msec = i)
DATE_SET_FUNC_4(setHours, SOPHON_DATE_LOCAL, hour = i, min = i, sec = i,
			msec = i)
DATE_SET_FUNC_4(setUTCHours, SOPHON_DATE_UTC, hour = i, min = i, sec = i,
			msec = i)
DATE_SET_FUNC(setDate, SOPHON_DATE_LOCAL, mday = i)
DATE_SET_FUNC(setUTCDate, SOPHON_DATE_UTC, mday = i)
DATE_SET_FUNC_2(setMonth, SOPHON_DATE_LOCAL, mon = i, mday = i)
DATE_SET_FUNC_2(setUTCMonth, SOPHON_DATE_UTC, mon = i, mday = i)
DATE_SET_FUNC_3(setFullYear, SOPHON_DATE_LOCAL, year = i - 1900, mon = i,
			mday = i)
DATE_SET_FUNC_3(setUTCFullYear, SOPHON_DATE_UTC, year = i - 1900, mon = i,
			mday = i)

static DATE_FUNC(toJSON)
{
	Sophon_Time time;
	Sophon_Date date;
	Sophon_String *str;
	Sophon_Result r;

	if ((r = sophon_value_to_number(vm, thisv, &time)) != SOPHON_OK)
		return r;

	if (sophon_isinf(time)) {
		sophon_value_set_null(vm, retv);
		return SOPHON_OK;
	}

	if ((r = sophon_time_to_date(vm, &time, SOPHON_DATE_UTC, &date))
				!= SOPHON_OK) {
		sophon_throw(vm, vm->TypeError, "Cannot convert time to date");
		return r;
	}

	str = sophon_date_to_string(vm, SOPHON_DATE_FL_FULL|
				SOPHON_DATE_FL_UTC|
				SOPHON_DATE_FL_ISO, &date);
	sophon_value_set_string(vm, retv, str);
	return SOPHON_OK;
}

#ifdef DATE_YEAR_FUNC
DATE_GET_FUNC(getYear, SOPHON_DATE_LOCAL, year)
DATE_SET_FUNC(getYear, SOPHON_DATE_LOCAL, year)
#endif

#ifdef DATE_TO_GMT_STRING_FUNC
static DATE_FUNC(toGMTString)
{
	return date_toUTCString_func(vm, thisv, argv, argc, retv);
}
#endif


static const Sophon_Decl
date_prototype_props[] = {
	SOPHON_STRING_PROP([Class], 0, "Date"),
	DATE_FUNC_PROP(toString, 0),
	DATE_FUNC_PROP(toDateString, 0),
	DATE_FUNC_PROP(toTimeString, 0),
	DATE_FUNC_PROP(toLocaleString, 0),
	DATE_FUNC_PROP(toLocaleDateString, 0),
	DATE_FUNC_PROP(toLocaleTimeString, 0),
	DATE_FUNC_PROP(valueOf, 0),
	DATE_FUNC_PROP(getTime, 0),
	DATE_FUNC_PROP(getFullYear, 0),
	DATE_FUNC_PROP(getUTCFullYear, 0),
	DATE_FUNC_PROP(getMonth, 0),
	DATE_FUNC_PROP(getUTCMonth, 0),
	DATE_FUNC_PROP(getDate, 0),
	DATE_FUNC_PROP(getUTCDate, 0),
	DATE_FUNC_PROP(getDay, 0),
	DATE_FUNC_PROP(getUTCDay, 0),
	DATE_FUNC_PROP(getHours, 0),
	DATE_FUNC_PROP(getUTCHours, 0),
	DATE_FUNC_PROP(getMinutes, 0),
	DATE_FUNC_PROP(getUTCMinutes, 0),
	DATE_FUNC_PROP(getSeconds, 0),
	DATE_FUNC_PROP(getUTCSeconds, 0),
	DATE_FUNC_PROP(getMilliseconds, 0),
	DATE_FUNC_PROP(getUTCMilliseconds, 0),
	DATE_FUNC_PROP(getTimezoneOffset, 0),
	DATE_FUNC_PROP(setTime, 1),
	DATE_FUNC_PROP(setMilliseconds, 1),
	DATE_FUNC_PROP(setUTCMilliseconds, 1),
	DATE_FUNC_PROP(setSeconds, 2),
	DATE_FUNC_PROP(setUTCSeconds, 2),
	DATE_FUNC_PROP(setMinutes, 3),
	DATE_FUNC_PROP(setUTCMinutes, 3),
	DATE_FUNC_PROP(setHours, 4),
	DATE_FUNC_PROP(setUTCHours, 4),
	DATE_FUNC_PROP(setDate, 1),
	DATE_FUNC_PROP(setUTCDate, 1),
	DATE_FUNC_PROP(setMonth, 2),
	DATE_FUNC_PROP(setUTCMonth, 2),
	DATE_FUNC_PROP(setFullYear, 3),
	DATE_FUNC_PROP(setUTCFullYear, 3),
	DATE_FUNC_PROP(toUTCString, 0),
	DATE_FUNC_PROP(toISOString, 0),
	DATE_FUNC_PROP(toJSON, 1),
#ifdef DATE_YEAR_FUNC
	DATE_FUNC_PROP(getYear, 0),
	DATE_FUNC_PROP(setYear, 1),
#endif
#ifdef DATE_TO_GMT_STRING_FUNC
	DATE_FUNC_PROP(toGMTString, 0),
#endif
	{NULL}
};

static const Sophon_Decl
date_object_props[] = {
	SOPHON_OBJECT_PROP(prototype, 0, date_prototype_props),
	SOPHON_FUNCTION_PROP([Call], GLOBAL_ATTRS, date_call_func, 7),
	DATE_FUNC_PROP(parse, 1),
	DATE_FUNC_PROP(UTC, 7),
	{NULL}
};

