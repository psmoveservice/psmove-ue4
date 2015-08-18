
/**
* PS Move API - An interface for the PS Move Motion Controller
* Copyright (c) 2015 bwalker <brendan@millerwalker.net>
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

#ifndef PSMOVE_PLATFORM_CONFIG_H
#define PSMOVE_PLATFORM_CONFIG_H

#include "psmove_config.h"

#ifdef _MSC_VER
#define __INLINE__ __inline
#define snprintf _snprintf  // Not exactly the same, but close enough.
#define strdup _strdup
#else
#define __INLINE__ inline
#endif

#ifdef _WIN32
#  define ADDCALL __cdecl
#  if defined(BUILDING_STATIC_LIBRARY)
#    define ADDAPI
#  elif defined(USING_STATIC_LIBRARY)
#    define ADDAPI
#  elif defined(BUILDING_SHARED_LIBRARY)
#    define ADDAPI __declspec(dllexport)
#  else /* using shared library */
#    define ADDAPI __declspec(dllimport)
#  endif
#else
#  define ADDAPI
#  define ADDCALL 
#endif

#endif // PSMOVE_PLATFORM_CONFIG_H