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

noinst_LTLIBRARIES = libcfg.la

libcfg_la_SOURCES = \
 driver.c\
 format.c\
 lexer.c\
 parser.c

localedir = $(datadir)/locale
AM_CPPFLAGS = \
 $(MU_LIB_COMMON_INCLUDES) -I/libmailutils\
 -DSYSCONFDIR=\"$(sysconfdir)\"\
 -DLOCALEDIR=\"$(localedir)\"

EXTRA_DIST = \
 lexer.l\
 parser.y\
 parser.h

BUILT_SOURCES = parser.c parser.h lexer.c

parser.c parser.h: parser.y
lexer.c: lexer.l

AM_YFLAGS=-vdt
AM_LFLAGS=-dp

.l.c:
	$(FLEX) -o $@ $(AM_LFLAGS) $<
.y.c:
	$(BISON) -o $@ $(AM_YFLAGS) $<

