# GNU Mailutils -- a suite of utilities for electronic mail
# Copyright (C) 2000-2022 Free Software Foundation, Inc.
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

SUBDIRS = \
 auth\
 base\
 address\
 list\
 sockaddr\
 cidr\
 cfg\
 cli\
 diag\
 filter\
 locus\
 mailbox\
 mailcap\
 mailer\
 mime\
 mimetypes\
 msgset\
 opt\
 server\
 string\
 stream\
 stdstream\
 property\
 url\
 imapio\
 datetime\
 wicket\
 .\
 tests

lib_LTLIBRARIES = libmailutils.la

libmailutils_la_SOURCES =  

libmailutils_la_LIBADD = \
 $(MU_COMMON_LIBRARIES)\
 auth/libauth.la\
 base/libbase.la\
 address/libaddress.la\
 sockaddr/libsockaddr.la\
 cidr/libcidr.la\
 cfg/libcfg.la\
 cli/libcli.la\
 datetime/libdatetime.la\
 diag/libdiag.la\
 filter/libfilter.la\
 imapio/libimapio.la\
 list/liblist.la\
 locus/liblocus.la\
 mailbox/libmailbox.la\
 mailcap/libmailcap.la\
 mailer/libmailer.la\
 mime/libmime.la\
 mimetypes/libmimetypes.la\
 msgset/libmsgset.la\
 opt/libopt.la\
 property/libproperty.la\
 server/libserver.la\
 string/libstring.la\
 stream/libstream.la\
 stdstream/libstdstream.la\
 url/liburl.la\
 wicket/libwicket.la

libmailutils_la_LDFLAGS = -version-info @VI_CURRENT@:@VI_REVISION@:@VI_AGE@

EXTRA_DIST =\
 wordsplit/wordsplit.c\
 wordsplit/wordsplit.h\
 wordsplit/wordsplit.at\
 wordsplit/wsp.c

