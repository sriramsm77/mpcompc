# This file is part of GNU Mailutils.
# Copyright (C) 2019-2022 Free Software Foundation, Inc.
#
# GNU Mailutils is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or (at
# your option) any later version.
#
# GNU Mailutils is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Mailutils.  If not, see <http://www.gnu.org/licenses/>.

include $(top_srcdir)/testsuite/testsuite.am

## -------------------------- ##
## Non-installable programs
## -------------------------- ##

AM_CPPFLAGS = \
 $(MU_LIB_COMMON_INCLUDES)\
 -I$(top_srcdir)\
 -I$(top_srcdir)/libmailutils/tests

noinst_PROGRAMS = \
 mbop
LDADD = -L$(top_builddir)/libmailutils/tests -lmu_tesh $(MU_LIB_DOTMAIL) $(MU_LIB_MAILUTILS)

## ------------ ##
## Test suite.  ##
## ------------ ##

TESTSUITE_AT += \
 autodetect.at\
 count.at\
 env.at\
 attr.at\
 recent.at\
 unseen.at\
 header.at\
 body.at\
 uidnext.at\
 uidvalidity.at\
 uid.at\
 qget.at\
 append.at\
 delete.at\
 setattr.at\
 notify.at\
 rospool.at

		


