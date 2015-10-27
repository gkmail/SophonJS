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
#include <sophon_date.h>
#include <sophon_string.h>
#include <sophon_vm.h>
#include <sophon_debug.h>

#ifdef SOPHON_HAVE_TIME

static Sophon_Int
tz_offset (Sophon_VM *vm)
{
	time_t t, loc, utc;

	t = time(NULL);
	loc = mktime(localtime(&t));
	utc = mktime(gmtime(&t));

	return (loc - utc) * 1000;
}

static Sophon_Result
time_now (Sophon_VM *vm, Sophon_Time *time)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	*time = tv.tv_sec * 1000. + tv.tv_usec / 1000;

	return SOPHON_OK;
}

static Sophon_Result
date_to_time (Sophon_VM *vm, Sophon_Date *date, Sophon_Time *time)
{
	struct tm d;
	time_t t;
	Sophon_Time st;

	sophon_memset(&d, 0, sizeof(d));

	d.tm_sec   = date->sec;
	d.tm_min   = date->min;
	d.tm_hour  = date->hour;
	d.tm_mday  = date->mday;
    d.tm_mon   = date->mon;
	d.tm_year  = date->year;
	d.tm_isdst = date->isdst;

	t = mktime(&d);
	if (t == -1)
		return SOPHON_ERR_ARG;

	st = (t * 1000.) + date->msec;

	*time = st;

	return SOPHON_OK;
}

static Sophon_Result
tm_to_date (Sophon_Date *date, Sophon_Time *time, struct tm *d)
{
	Sophon_Int msec;

	date->sec   = d->tm_sec;
	date->min   = d->tm_min;
	date->hour  = d->tm_hour;
	date->yday  = d->tm_yday;
	date->mday  = d->tm_mday;
	date->wday  = d->tm_wday;
	date->mon   = d->tm_mon;
	date->year  = d->tm_year;
	date->isdst = d->tm_isdst;

	msec = sophon_fmod(*time, 1000.);

	date->msec  = (*time >= 0) ? msec : 1000 + msec;

	return SOPHON_OK;
}

static Sophon_Result
time_to_utc_date (Sophon_VM *vm, Sophon_Time *time, Sophon_Date *date)
{
	struct tm *d;
	time_t t;

	t = sophon_floor(*time / 1000);

	if (!(d = gmtime(&t)))
		return SOPHON_ERR_ARG;

	return tm_to_date(date, time, d);
}

static Sophon_Result
time_to_local_date (Sophon_VM *vm, Sophon_Time *time, Sophon_Date *date)
{
	struct tm *d;
	time_t t;

	t = sophon_floor(*time / 1000);

	if (!(d = localtime(&t)))
		return SOPHON_ERR_ARG;

	return tm_to_date(date, time, d);
}

#else  /*!defined(SOPHON_HAVE_TIME)*/
	#error no date functions defined
#endif /*SOPHON_HAVE_TIME*/

static Sophon_Result
parse_num (Sophon_VM *vm, Sophon_Char *begin, Sophon_Int *num)
{
	Sophon_Char *ptr = begin;
	Sophon_Int s, n;

	if (*ptr == '+') {
		s = 1;
		ptr++;
	} else if (*ptr == '-') {
		s = -1;
		ptr++;
	} else {
		s = 1;
	}

	if (sophon_isdigit(*ptr)) {
		n = *ptr++ - '0';
	} else {
		return SOPHON_ERR_PARSE;
	}

	while (sophon_isdigit(*ptr)) {
		n *= 10;
		n += *ptr - '0';
		ptr++;
	}

	*num = s * n;
	return ptr - begin;
}

static void
print_num (Sophon_VM *vm, Sophon_Char *ptr, Sophon_Int n, Sophon_Int len)
{
	Sophon_Char *chr = ptr + len - 1;

	n = SOPHON_ABS(n);

	while (chr >= ptr) {
		*chr = (n % 10) + '0';
		n /= 10;
		chr--;
	}
}

Sophon_Int
sophon_timezone_offset (Sophon_VM *vm)
{
	return tz_offset(vm);
}

Sophon_Result
sophon_time_now (Sophon_VM *vm, Sophon_Time *time)
{
	SOPHON_ASSERT(time);

	return time_now(vm, time);
}

Sophon_Result
sophon_date_to_time (Sophon_VM *vm, Sophon_Date *date, Sophon_Time *time)
{
	Sophon_Time t;
	Sophon_Result r;

	SOPHON_ASSERT(date && time);

	if ((r = date_to_time(vm, date, &t)) != SOPHON_OK)
		return r;

	*time = t;
	return SOPHON_OK;
}

Sophon_Result
sophon_time_to_date (Sophon_VM *vm, Sophon_Time *time, Sophon_DateType type,
			Sophon_Date *date)
{
	SOPHON_ASSERT(date && time);

	if (type == SOPHON_DATE_LOCAL)
		return time_to_local_date(vm, time, date);
	else
		return time_to_utc_date(vm, time, date);
}

Sophon_Result
sophon_time_parse (Sophon_VM *vm, Sophon_String *str, Sophon_Time *time)
{
	Sophon_Char *ptr;
	Sophon_Date date;
	Sophon_Result r;

	SOPHON_ASSERT(str && time);

	sophon_memset(&date, 0, sizeof(date));

	ptr = sophon_string_chars(vm, str);
	while (*ptr && sophon_isspace(*ptr))
		ptr++;

	if ((r = parse_num(vm, ptr, &date.year)) < 0)
		return r;
	date.year -= 1900;
	ptr += r;
	if (*ptr++ != '-')
		return SOPHON_ERR_PARSE;
	if ((r = parse_num(vm, ptr, &date.mon)) < 0)
		return r;
	date.mon--;
	ptr += r;
	if (*ptr++ != '-')
		return SOPHON_ERR_PARSE;
	if ((r = parse_num(vm, ptr, &date.mday)) < 0)
		return r;
	ptr += r;

	if ((*ptr == 'T') || (*ptr == ' ')) {
		ptr++;
		if ((r = parse_num(vm, ptr, &date.hour)) < 0)
			return r;
		ptr += r;
		if (*ptr++ != ':')
			return SOPHON_ERR_PARSE;
		if ((r = parse_num(vm, ptr, &date.min)) < 0)
			return r;
		ptr += r;
		if (*ptr++ != ':')
			return SOPHON_ERR_PARSE;
		if ((r = parse_num(vm, ptr, &date.sec)) < 0)
			return r;
		ptr += r;
		if (*ptr++ == '.') {
			if ((r = parse_num(vm, ptr, &date.msec)) < 0)
				return r;
			ptr += r;
		}
	} else {
		date.mday = 1;
	}

	return sophon_date_to_time(vm, &date, time);
}

Sophon_String*
sophon_date_to_string (Sophon_VM *vm, Sophon_U32 flags, Sophon_Date *date)
{
	Sophon_String *str;
	Sophon_Char *ptr;
	Sophon_Int size = 0;

	SOPHON_ASSERT(date);

	if (flags & SOPHON_DATE_FL_DATE) {
		if (date->year + 1900 < 0)
			size++;

		if (SOPHON_ABS(date->year + 1900) > 9999)
			size += 6;
		else
			size += 4;

		size += 6;

		if (flags & SOPHON_DATE_FL_TIME)
			size++;
	}

	if (flags & SOPHON_DATE_FL_TIME) {
		size += 12;

		if (flags & SOPHON_DATE_FL_UTC)
			size++;
		else
			size += 6;
	}

	str = sophon_string_from_chars(vm, NULL, size);
	ptr = sophon_string_chars(vm, str);

	if (flags & SOPHON_DATE_FL_DATE) {
		if (date->year + 1900 < 0)
			*ptr++ = '-';

		if (SOPHON_ABS(date->year) > 9999) {
			print_num(vm, ptr, date->year + 1900, 6);
			ptr += 6;
		} else {
			print_num(vm, ptr, date->year + 1900, 4);
			ptr += 4;
		}

		*ptr++ = '-';
		print_num(vm, ptr, date->mon + 1, 2);
		ptr += 2;

		*ptr++ = '-';
		print_num(vm, ptr, date->mday, 2);
		ptr += 2;

		if (flags & SOPHON_DATE_FL_TIME)
			*ptr++ = (flags & SOPHON_DATE_FL_ISO) ? 'T' : ' ';
	}

	if (flags & SOPHON_DATE_FL_TIME) {
		print_num(vm, ptr, date->hour, 2);
		ptr += 2;

		*ptr++ = ':';
		print_num(vm, ptr, date->min, 2);
		ptr += 2;

		*ptr++ = ':';
		print_num(vm, ptr, date->sec, 2);
		ptr += 2;

		*ptr++ = '.';
		print_num(vm, ptr, date->msec, 3);
		ptr += 3;


		if (!(flags & SOPHON_DATE_FL_UTC)) {
			Sophon_Int n = vm->tz_offset;

			*ptr++ = (n > 0) ? '+' : '-';

			n = SOPHON_ABS(n);
			print_num(vm, ptr, n / 3600000, 2);
			ptr += 2;

			*ptr++ = ':';
			print_num(vm, ptr, (n % 3600000) / 60000, 2);
			ptr += 2;
		} else {
			*ptr++ = 'Z';
		}
	}

	return str;
}

