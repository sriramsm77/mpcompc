/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2009-2022 Free Software Foundation, Inc.

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

/* Ctype functions for ASCII character set */

#ifndef _MAILUTILS_MUCTYPE_H
#define _MAILUTILS_MUCTYPE_H

#ifdef __cplusplus
extern "C" {
#endif

#define MU_CTYPE_ALPHA   0x00001
#define MU_CTYPE_DIGIT   0x00002
#define MU_CTYPE_BLANK   0x00004
#define MU_CTYPE_CNTRL   0x00008
#define MU_CTYPE_GRAPH   0x00010
#define MU_CTYPE_LOWER   0x00020
#define MU_CTYPE_UPPER   0x00040
#define MU_CTYPE_PRINT   0x00080
#define MU_CTYPE_PUNCT   0x00100
#define MU_CTYPE_SPACE   0x00200
#define MU_CTYPE_XLETR   0x00400
#define MU_CTYPE_ENDLN   0x00800
#define MU_CTYPE_TSPEC   0x01000 /* tspecials: RFC 2045, section 5.1. */
#define MU_CTYPE_IDENT   0x02000 /* Valid identifier constituent: alnum or _ */
#define MU_CTYPE_HEADR   0x04000 /* Valid header name constituent: alnum, _,
				    or - */
#define MU_CTYPE_IMSPC   0x08000 /* Internet Message Format Specials:
				    RFC2822, 3.2.1 */
#define MU_CTYPE_NWCTL   0x10000 /* Internet Message NO-WS-CTL:
				    RFC2822, 3.2.1 */
#define MU_CTYPE_IMATM   0x20000 /* Internet Message atom constituent */

#define MU_C_TAB_MAX     128

extern int mu_c_tab[MU_C_TAB_MAX];

#define mu_c_is_class(c, class) \
  (((unsigned)(c)) < 128 && mu_c_tab[(unsigned)(c)] & (class))  

#define mu_isalpha(c) mu_c_is_class (c, MU_CTYPE_ALPHA)
#define mu_iscntrl(c) mu_c_is_class (c, MU_CTYPE_CNTRL)
#define mu_isdigit(c) mu_c_is_class (c, MU_CTYPE_DIGIT)
#define mu_isgraph(c) mu_c_is_class (c, MU_CTYPE_GRAPH)
#define mu_islower(c) mu_c_is_class (c, MU_CTYPE_LOWER)
#define mu_isprint(c) mu_c_is_class (c, MU_CTYPE_PRINT)
#define mu_ispunct(c) mu_c_is_class (c, MU_CTYPE_PUNCT)
#define mu_isspace(c) mu_c_is_class (c, MU_CTYPE_SPACE)
#define mu_isupper(c) mu_c_is_class (c, MU_CTYPE_UPPER)
#define mu_isxdigit(c) mu_c_is_class (c, MU_CTYPE_DIGIT|MU_CTYPE_XLETR)
#define mu_isalnum(c) mu_c_is_class (c, MU_CTYPE_ALPHA|MU_CTYPE_DIGIT)
#define mu_isascii(c) (((unsigned)c) < MU_C_TAB_MAX)
#define mu_isblank(c) mu_c_is_class (c, MU_CTYPE_BLANK)
#define mu_isendln(c) mu_c_is_class (c, MU_CTYPE_ENDLN)
#define mu_istspec(c) mu_c_is_class (c, MU_CTYPE_TSPEC)
#define mu_isident(c) mu_c_is_class (c, MU_CTYPE_IDENT)
#define mu_isheadr(c) mu_c_is_class (c, MU_CTYPE_HEADR)
#define mu_isimspc(c) mu_c_is_class (c, MU_CTYPE_IMSPC)
#define mu_isnwctl(c) mu_c_is_class (c, MU_CTYPE_NWCTL)
#define mu_isimatm(c) mu_c_is_class (c, MU_CTYPE_IMATM)

#define mu_tolower(c)					\
  ({ int __c = (c);					\
    (__c >= 'A' && __c <= 'Z' ? __c - 'A' + 'a' : __c); \
  })

#define mu_toupper(c)					\
  ({ int __c = (c);					\
    (__c >= 'a' && __c <= 'z' ? __c - 'a' + 'A' : __c); \
  })
  
#ifdef __cplusplus
}
#endif
  
#endif
