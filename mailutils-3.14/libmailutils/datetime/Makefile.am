# GNU Mailutils -- a suite of utilities for electronic mail
# Copyright (C) 2011-2022 Free Software Foundation, Inc.
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

noinst_LTLIBRARIES = libdatetime.la

libdatetime_la_SOURCES = \
 dow.c\
 doy.c\
 jd.c\
 parsedate.c\
 scantime.c\
 streamftime.c\
 strftime.c\
 tab.c\
 tzlocal.c\
 tzutc.c\
 unixtime.c\
 utcoff.c\
 yd.c

BUILT_SOURCES = parsedate.c
EXTRA_DIST = parsedate.y

parsedate.c: parsedate.y
.y.c:
	$(BISON) -o $@ $(AM_YFLAGS) $<

AM_CPPFLAGS = $(MU_LIB_COMMON_INCLUDES) -I/libmailutils

