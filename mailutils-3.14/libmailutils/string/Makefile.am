# GNU Mailutils -- a suite of utilities for electronic mail
# Copyright (C) 2010-2022 Free Software Foundation, Inc.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 3 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General
# Public License along with this library.  If not, see
# <http://www.gnu.org/licenses/>.

noinst_LTLIBRARIES = libstring.la

libstring_la_SOURCES = \
 cpystr.c\
 cstrcasecmp.c\
 cstrcasestr.c\
 cstrescape.c\
 cstrunescape.c\
 cstrlower.c\
 cstrupper.c\
 expvar.c\
 hexstr.c\
 safefilename.c\
 stpcpy.c\
 str_to_c.c\
 strcount.c\
 strltrim.c\
 strpref.c\
 strskip.c\
 strsuf.c\
 stripws.c\
 strlst.c\
 strrtrim.c\
 strtosize.c\
 trueans.c\
 unfold.c\
 asnprintf.c\
 asprintf.c\
 muctype.c\
 vasnprintf.c\
 mkfilename.c\
 version.c\
 wordsplit.c\
 xdecode.c

EXTRA_DIST=to_sn.c to_un.c

AM_CPPFLAGS = \
 $(MU_LIB_COMMON_INCLUDES)\
 -I$(srcdir)\
 -I$(top_srcdir)/libmailutils\
 -I$(top_srcdir)/libmailutils/wordsplit
