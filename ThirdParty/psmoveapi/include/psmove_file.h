
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
#ifndef __PSMOVE_FILE_H
#define __PSMOVE_FILE_H

#ifdef _MSC_VER
struct _iobuf;
typedef struct _iobuf FILE;
#else
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

ADDAPI FILE*
ADDCALL psmove_file_open(const char *filename, const char *mode);

ADDAPI void
ADDCALL psmove_file_close(FILE* file_pointer);

/**
* \brief Get an integer from an environment variable
*
* Utility function used to get configuration from environment
* variables.
*
* \param environment_variable_name The name of the environment variable
*
* \return The integer value of the environment variable, or -1 if
*         the variable is not set or could not be parsed as integer.
**/
ADDAPI int
ADDCALL psmove_util_get_env_int(const char *environment_variable_name);

/**
* \brief Set an int value for environment variable
*
* Utility function used to set configuration from environment
* variables.
*
* \param environment_variable_name The name of the environment variable
* \param int_value The value string
*
* \return True on success
**/
ADDAPI enum PSMove_Bool
ADDCALL psmove_util_set_env_int(const char *environment_variable_name, const int int_value);

/**
* \brief Get a string from an environment variable
*
* Utility function used to get configuration from environment
* variables.
*
* \param environment_variable_name The name of the environment variable
* \param buffer_size The size of the output buffer
* \param out_buffer The buffer the environment variable string is writted to
*
* \return True on success
**/
ADDAPI char *
ADDCALL psmove_util_get_env_string(const char *environment_variable_name);

/**
* \brief Set a string value for environment variable
*
* Utility function used to set configuration from environment
* variables.
*
* \param environment_variable_name The name of the environment variable
* \param string_value The value string 
*
* \return True on success
**/
ADDAPI enum PSMove_Bool
ADDCALL psmove_util_set_env_string(const char *environment_variable_name, const char *string_value);

/**
* \brief Get local save directory for settings.
*
* The local save directory is a PS Move API-specific directory where the
* library and its components will store files such as calibration data,
* tracker state and configuration files.
*
* \return The local save directory for settings.
*         The returned value is reserved in static memory - it must not be freed!
**/
ADDAPI const char *
ADDCALL psmove_util_get_data_dir();

/**
* \brief Get a filename path in the local save directory.
*
* This is a convenience function wrapping psmove_util_get_data_dir()
* and will give the absolute path of the given filename.
*
* The data directory will be created in case it doesn't exist yet.
*
* \param filename The basename of the file (e.g. \c myfile.txt)
*
* \return The absolute filename to the file. The caller must
*         free() the result when it is not needed anymore.
* \return On error, \c NULL is returned.
**/
ADDAPI char *
ADDCALL psmove_util_get_file_path(const char *filename);

/**
* \brief Get a filename path in the system save directory.
*
* This is a convenience function, which gives the absolute path for
* a file stored in system-wide data directory.
*
* The data directory will NOT be created in case it doesn't exist yet.
*
* \param filename The basename of the file (e.g. \c myfile.txt)
*
* \return The absolute filename to the file. The caller must
*         free() the result when it is not needed anymore.
* \return On error, \c NULL is returned.
**/
ADDAPI char *
ADDCALL psmove_util_get_system_file_path(const char *filename);

#ifdef __cplusplus
}
#endif

#endif // __PSMOVE_FILE_H