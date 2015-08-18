/**
* PS Move API - An interface for the PS Move Motion Controller
* Copyright (c) 2011, 2012 Thomas Perl <m@thp.io>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*    1. Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*
*    2. Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
**/

/* Performance measurement structures and functions */
#ifndef __PSMOVE_TIME_H
#define __PSMOVE_TIME_H

#include "psmove_platform_config.h"

#include <time.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
struct timespec {               /* definition per POSIX.4 */
	time_t          tv_sec;         /* seconds */
	long            tv_nsec;        /* and nanoseconds */
};

typedef int64_t __int64_t;
#endif // _MSC_VER

typedef struct timespec PSMove_timestamp;

ADDAPI void 
ADDCALL psmove_sleep(unsigned long milliseconds);

ADDAPI void 
ADDCALL psmove_usleep(__int64_t usec);

ADDAPI enum PSMove_Bool
ADDCALL psmove_time_init();

ADDAPI PSMove_timestamp
ADDCALL psmove_timestamp();

ADDAPI PSMove_timestamp
ADDCALL psmove_timestamp_diff(PSMove_timestamp a, PSMove_timestamp b);

ADDAPI double 
ADDCALL psmove_timestamp_value(PSMove_timestamp ts);

/**
* \brief Get milliseconds since first library use.
*
* This function is used throughout the library to take care of timing and
* measurement. It implements a cross-platform way of getting the current
* time, relative to library use.
*
* \return Time (in ms) since first library use.
**/
ADDAPI long 
ADDCALL psmove_util_get_ticks();

#ifdef __cplusplus
}
#endif

#endif // __PSMOVE_TIME_H