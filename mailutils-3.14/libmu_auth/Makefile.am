## This file is part of GNU Mailutils.
## Copyright (C) 2002-2022 Free Software Foundation, Inc.
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

AM_CPPFLAGS = \
 $(MU_LIB_COMMON_INCLUDES)\
 -DSITE_VIRTUAL_PWDDIR=\"@SITE_VIRTUAL_PWDDIR@\" \
 -DSITE_CRAM_MD5_PWD=\"@SITE_CRAM_MD5_PWD@\"

lib_LTLIBRARIES = libmu_auth.la

libmu_auth_la_SOURCES = \
 gsasl.c\
 ldap.c\
 pam.c\
 radius.c\
 sql.c\
 sql.h\
 virtual.c 

libmu_auth_la_SOURCES += tlsconf.c tlsvar.c

if MU_COND_GNUTLS
  libmu_auth_la_SOURCES += \
    tls.c\
    tlsiostr.c\
    tlsfdstr.c
else
  libmu_auth_la_SOURCES += notls.c
endif

libmu_auth_la_LIBADD = $(MU_LIB_MAILUTILS) $(MU_AUTHLIBS) @SQLLIB@ @LTLIBINTL@
libmu_auth_la_LDFLAGS = -version-info @VI_CURRENT@:@VI_REVISION@:@VI_AGE@

SUBDIRS = . tests
