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

SUBDIRS = . etc tests
bindir = @MH_BIN_DIR@

bin_PROGRAMS = \
 ali\
 anno\
 burst\
 comp\
 fmtcheck\
 folder\
 forw\
 inc\
 install-mh\
 mark\
 mhl\
 mhn\
 mhparam\
 mhpath\
 mhseq\
 msgchk\
 pick\
 prompter\
 refile\
 repl\
 rmf\
 rmm\
 scan\
 show\
 send\
 sortm\
 whatnow\
 whom

prompter_LDADD = $(mh_LIBS)

if MU_COND_READLINE
  PROMPTER_FUN=prompter-rl.c
  prompter_LDADD += @READLINE_LIBS@
else
  PROMPTER_FUN=prompter-tty.c
  prompter_LDADD += @CURSES_LIBS@
endif

prompter_SOURCES = \
 prompter.c\
 prompter.h\
 $(PROMPTER_FUN)

noinst_LIBRARIES = libmh.a

libmh_a_SOURCES= \
 compcommon.c\
 mboxprop.c\
 mh_alias_gram.c\
 mh_alias_lex.c\
 mh_getopt.c\
 mh_global.c\
 mh_format.c\
 mh_init.c\
 mh_list.c\
 mh_fmtgram.c\
 mh_msgset.c\
 mh_sequence.c\
 mh_stream.c\
 mh_whatnow.c\
 mh_whom.c\
 scanfmt.c\
 seqelim.c\
 whatnowenv.c
noinst_HEADERS = mh.h mh_alias_gram.h mh_format.h mh_getopt.h

BUILT_SOURCES=\
	mh_fmtgram.c\
        pick-gram.c\
        pick-gram.h\
        mh_alias_gram.c\
        mh_alias_gram.h\
        mh_alias_lex.c

MAINTAINERCLEANFILES=$(BUILT_SOURCES)

mhlibdir = $(pkgdatadir)/mh

AM_CPPFLAGS =\
 $(MU_APP_COMMON_INCLUDES)\
 -D_GNU_SOURCE -DMHLIBDIR=\"$(mhlibdir)\"\
 -DMHBINDIR=\"$(bindir)\"

mh_LIBS = \
 ./libmh.a\
 $(MU_APP_LIBRARIES)\
 $(MU_LIB_MAILBOX)\
 $(MU_LIB_MAILER)\
 $(MU_LIB_AUTH)\
 $(MU_AUTHLIBS)\
 $(MU_LIB_MAILUTILS)\
 $(MU_COMMON_LIBRARIES)
LDADD = $(mh_LIBS)
scan_LDADD = $(mh_LIBS) @CURSES_LIBS@
pick_SOURCES = pick.c pick.h pick-gram.c pick-gram.h

AM_YFLAGS=-dt

inc_LDADD = \
 ./libmh.a\
 $(MU_APP_LIBRARIES)\
 ../lib/libmuscript.a\
 @LIBMU_SCM@ @GUILE_LIBS@\
 @LIBMU_SCM_DEPS@\
 $(MU_LIB_PY) @PYTHON_LIBS@\
 $(MU_LIB_SIEVE)\
 $(MU_LIB_MAILBOX)\
 $(MU_LIB_MAILER)\
 $(MU_LIB_AUTH)\
 $(MU_AUTHLIBS)\
 $(MU_LIB_MAILUTILS)\
 $(MU_COMMON_LIBRARIES)

install-exec-hook: 
	@here=`pwd`; \
	cd $(DESTDIR)$(bindir); \
	rm -f folders next prev; \
	$(LN_S) folder folders; \
        $(LN_S) show next; \
        $(LN_S) show prev; \
	cd $$here

EXTRA_DIST = pick-gram.y mh_alias_gram.y mh_fmtgram.y mh_alias_lex.l

pick-gram.c pick-gram.h: pick-gram.y
mh_alias_gram.c mh_alias_gram.h: mh_alias_gram.y
mh_alias_lex.c: mh_alias_lex.l

mh_fmtgram.c: mh_fmtgram.y
	$(BISON) -o $@ -t $<

.l.c:
	$(FLEX) -o $@ $(AM_LFLAGS) $<
.y.c:
	$(BISON) -o $@ $(AM_YFLAGS) $<
