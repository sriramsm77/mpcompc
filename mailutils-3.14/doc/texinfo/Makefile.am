## This file is part of GNU Mailutils.
## Copyright (C) 2001-2022 Free Software Foundation, Inc.
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

info_TEXINFOS = mailutils.texi 

mailutils_TEXINFOS = \
 fdl.texi\
 getdate.texi\
 macros.texi\
 mu-mh.texi\
 programs.texi\
 sieve.texi\
 strftime.texi\
 usage.texi\
 version.texi\
 $(programs_TEXINFOS)

programs_TEXINFOS = \
 programs/decodemail.texi\
 programs/frm.texi\
 programs/mail.texi\
 programs/messages.texi\
 programs/movemail.texi\
 programs/readmsg.texi\
 programs/sieve.texi\
 programs/guimb.texi\
 programs/mda.texi\
 programs/lmtpd.texi\
 programs/putmail.texi\
 programs/mimeview.texi\
 programs/pop3d.texi\
 programs/imap4d.texi\
 programs/comsatd.texi\
 programs/mailutils.texi\
 programs/dotlock.texi

clean-local:
	rm -rf manual

AM_MAKEINFOFLAGS = @IMPRIMATUR_MAKEINFOFLAGS@
# Imprimatur setup
imprimatur_INPUT=$(info_TEXINFOS) $(mailutils_TEXINFOS)
include ../imprimatur/imprimatur.mk
#CHECK_DOCS=$(top_srcdir)/@IMPRIMATUR_MODULE_DIR@/check-docs.sh

# Checking
all-check-docs: imprimatur-basic-checks

check-docs:
	@$(MAKE) -k all-check-docs

#

master-menu: imprimatur-master-menu
untabify: imprimatur-untabify
final: imprimatur-final

# Web manual
#EXTRA_DIST = \
# gendocs_template

GENDOCS=$(srcdir)/gendocs.sh --no-copy-images --html '--init-file=$(abs_srcdir)/html.init'
TEXI2DVI=texi2dvi -t '@set $(RENDITION)' -E

.PHONY: manual
manual:
	TEXINPUTS=$(srcdir):$(top_srcdir)/build-aux:$(TEXINPUTS) \
	 MAKEINFO="$(MAKEINFO) $(MAKEINFOFLAGS) $(AM_MAKEINFOFLAGS)" \
	 TEXI2DVI="$(TEXI2DVI)" \
	$(GENDOCS) $(PACKAGE) '$(PACKAGE_NAME) manual'

manual.tar.bz2: manual
	tar cfj manual.tar.bz2 manual

man-tar: manual.tar.bz2
