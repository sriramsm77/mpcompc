## This file is part of GNU Mailutils.
## Copyright (C) 1999-2022 Free Software Foundation, Inc.
##
## GNU Mailutils is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License as
## published by the Free Software Foundation; either version 2, or (at
## your option) any later version.
##
## GNU Mailutils is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with GNU Mailutils.  If not, see <http://www.gnu.org/licenses/>. 

AM_YFLAGS=-vt
AM_CPPFLAGS = \
 $(MU_APP_COMMON_INCLUDES) $(MU_AUTHINCS)\
 -DSITE_MAIL_RC=\"@SITE_MAIL_RC@\"

mailbindir=@MAILBINDIR@

mailbin_PROGRAMS = mail

mail_LDADD = \
 $(MU_APP_LIBRARIES)\
 $(MU_LIB_MAILBOX)\
 $(MU_LIB_MAILER)\
 $(MU_LIB_AUTH)\
 $(MU_AUTHLIBS)\
 $(MU_LIB_MAILUTILS)\
 @READLINE_LIBS@ $(MU_COMMON_LIBRARIES)

mail_SOURCES = \
 alias.c\
 alt.c\
 cd.c\
 copy.c\
 decode.c\
 delete.c\
 dp.c\
 echo.c\
 edit.c\
 envelope.c\
 eq.c\
 escape.c\
 exit.c\
 file.c\
 folders.c\
 from.c\
 headers.c\
 help.c\
 hold.c\
 if.c\
 inc.c\
 list.c\
 mail.c\
 mail.h\
 mailline.c\
 mailvar.c\
 mbox.c\
 msgset.c\
 next.c\
 page.c\
 pipe.c\
 previous.c\
 print.c\
 quit.c\
 reply.c\
 retain.c\
 save.c\
 send.c\
 set.c\
 setenv.c\
 shell.c\
 size.c\
 source.c\
 struct.c\
 summary.c\
 table.c\
 tag.c\
 top.c\
 touch.c\
 unalias.c\
 undelete.c\
 unread.c\
 unset.c\
 util.c\
 version.c\
 visual.c\
 write.c\
 z.c

BUILT_SOURCES = msgset.c
EXTRA_DIST = msgset.y

msgset.c: msgset.y

.y.c:
	$(BISON) -o $@ $(AM_YFLAGS) $<


SUBDIRS = . testsuite tests

