/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 1999-2022 Free Software Foundation, Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General
   Public License along with this library.  If not, see
   <http://www.gnu.org/licenses/>. */

#ifndef _MAILUTILS_ERROR_H
#define _MAILUTILS_ERROR_H

#include <stdarg.h>

#include <mailutils/types.h>
#include <mailutils/diag.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int mu_verror (const char *fmt, va_list ap);
extern int mu_error (const char *fmt, ...) MU_PRINTFLIKE(1,2);

#ifdef __cplusplus
}
#endif

#endif /* _MAILUTILS_ERROR_H */
