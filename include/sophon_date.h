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

#ifndef _SOPHON_DATE_H_
#define _SOPHON_DATE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sophon_types.h"
#include "sophon_gc.h"

/**\brief Date*/
typedef struct {
	Sophon_Int year; /**< Year*/
	Sophon_Int mon;  /**< Month*/
	Sophon_Int yday; /**< Day in the year*/
	Sophon_Int mday; /**< Day in the month*/
	Sophon_Int wday; /**< Day in the week*/
	Sophon_Int hour; /**< Hour*/
	Sophon_Int min;  /**< Minute*/
	Sophon_Int sec;  /**< Second*/
	Sophon_Int msec; /**< Millisecond*/
	Sophon_Int isdst;/**< Daylight saving time*/
} Sophon_Date;

/**\brief Time value*/
typedef Sophon_Number Sophon_Time;

/**\brief Date type*/
typedef enum {
	SOPHON_DATE_UTC,  /**< UTC date*/
	SOPHON_DATE_LOCAL /**< Local date*/
} Sophon_DateType;

/**\brief Output date*/
#define SOPHON_DATE_FL_DATE 1
/**\brief Output time*/
#define SOPHON_DATE_FL_TIME 2
/**\brief Output UTC string*/
#define SOPHON_DATE_FL_UTC  4
/**\brief Output ISO string*/
#define SOPHON_DATE_FL_ISO  8
/**\brief Output UTC string*/
#define SOPHON_DATE_FL_FULL (SOPHON_DATE_FL_DATE|SOPHON_DATE_FL_TIME)

/**
 * \brief Get timezone offset in milliseconds
 * \param[in] vm The current virtual machine
 * \return The timezone offset
 */
extern Sophon_Int   sophon_timezone_offset (Sophon_VM *vm);

/**
 * \brief Get current time
 * \param[in] vm The current virtual machine
 * \param[out] time Return the time value
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_time_now (Sophon_VM *vm, Sophon_Time *time);

/**
 * \brief Convert the date to time value
 * \param[in] vm The current virtual machine
 * \param[in] date The date value
 * \param[out] time Return the time value
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_date_to_time (Sophon_VM *vm, Sophon_Date *date,
						Sophon_Time *time);

/**
 * \brief Convert the time value to date value
 * \param[in] vm The current virtual machine
 * \param[in] time The time value
 * \param type UTC/local date
 * \param[out] date Return the date value
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_time_to_date (Sophon_VM *vm, Sophon_Time *time,
						Sophon_DateType type, Sophon_Date *date);

/**
 * \brief Parse the string to the time value
 * \param[in] vm The current virtual machine
 * \param[in] str The string
 * \param[out] time Return the time value
 * \retval SOPHON_OK On success
 * \retval <0 On error
 */
extern Sophon_Result sophon_time_parse (Sophon_VM *vm, Sophon_String *str,
						Sophon_Time *time);

/**
 * \brief Convert the date value to string
 * \param[in] vm The current virtual machine
 * \param flags Output flags
 * \param[in] date The date value
 * \return The string
 */
extern Sophon_String* sophon_date_to_string (Sophon_VM *vm,
						Sophon_U32 flags, Sophon_Date *date);

#ifdef __cplusplus
}
#endif

#endif

