## This file is part of GNU Mailutils.
## Copyright (C) 2004-2022 Free Software Foundation, Inc.
##
## GNU Mailutils is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License as
## published by the Free Software Foundation; either version 3, or (at
## your option) any later version.
##
## GNU Mailutils is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with GNU Mailutils.  If not, see <http://www.gnu.org/licenses/>.

MU_CXX_EXAMPLES_LIST = \
 addr\
 http\
 iconv\
 listop\
 lsf\
 mailcap\
 mimetest\
 msg-send\
 murun\
 sfrom\
 url-parse

EXTRA_PROGRAMS=$(MU_CXX_EXAMPLES_LIST)
noinst_PROGRAMS = $(MU_CXX_EXAMPLES_BUILD)

AM_CPPFLAGS = $(MU_LIB_COMMON_INCLUDES) 

AM_LDFLAGS = \
 ../../lib/libmuaux.la\
 ../../libmu_cpp/libmu_cpp.la\
 $(MU_LIB_MAILUTILS)\
 $(MU_COMMON_LIBRARIES)

addr_SOURCES = addr.cc
http_SOURCES = http.cc
iconv_SOURCES = iconv.cc
listop_SOURCES = listop.cc
lsf_SOURCES = lsf.cc
mailcap_SOURCES = mailcap.cc
mimetest_SOURCES = mimetest.cc
msg_send_SOURCES = msg-send.cc
murun_SOURCES = murun.cc
sfrom_SOURCES = sfrom.cc
url_parse_SOURCES = url-parse.cc

LDADD =\
 ../../lib/libmuaux.la\
 ../../libmu_cpp/libmu_cpp.la\
 $(MU_LIB_MAILBOX)\
 $(MU_LIB_AUTH)\
 $(MU_LIB_MAILER)\
 $(MU_LIB_SIEVE)\
 $(MU_AUTHLIBS)\
 $(MU_LIB_MAILUTILS)
