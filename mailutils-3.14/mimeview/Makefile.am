## This file is part of GNU Mailutils.
## Copyright (C) 2005-2022 Free Software Foundation, Inc.
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

SUBDIRS = . tests

AM_CPPFLAGS = \
 $(MU_APP_COMMON_INCLUDES)\
 -DDEFAULT_CUPS_CONFDIR=\"@DEFAULT_CUPS_CONFDIR@\"

bin_PROGRAMS = mimeview
mimeview_SOURCES = mimeview.c

mimeview_LDADD = \
 $(MU_APP_LIBRARIES)\
 $(MU_LIB_MAILUTILS)\
 $(MU_COMMON_LIBRARIES)
 
