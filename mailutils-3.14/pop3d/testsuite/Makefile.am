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

AUTOMAKE_OPTIONS = dejagnu
DEJATOOL = pop3d
RUNTESTFLAGS = 
CLEANFILES = *.log pop3d.rc
EXTRA_DIST = pop3d.rcin
test_dirs = config lib pop3d

dist-hook:
	here=`cd $(top_builddir)/$(subdir) && pwd`; \
	srcdir=`cd $(srcdir) && pwd`; \
	distdir=`cd $(distdir) && pwd`; \
	for dir in $(test_dirs); \
	do \
            cd $$srcdir;\
            mkdir $$distdir/$$dir;\
	    cd $$dir;\
	    for file in DISTFILES `cat DISTFILES`; do \
	        d=$$srcdir/$$dir; \
	        if test -d $$d/$$file; then \
                    cp -pr $$d/$$file $$distdir/$$dir/$$file; \
		else \
	            test -f $$distdir/$$dir/$$file \
                        || cp -p $$d/$$file $$distdir/$$dir/$$file || exit; \
                fi; \
	    done;\
        done;\
	cd $$here

site.exp: Makefile remote.exp
	@echo 'Making a new site.exp file...'
	@test ! -f site.bak || rm -f site.bak
	@echo '## these variables are automatically generated by make ##' > $@-t
	@echo '# Do not edit here.  If you wish to override these values' >> $@-t
	@echo '# edit the last section' >> $@-t
	@echo 'set tool $(DEJATOOL)' >> $@-t
	@echo "set top_srcdir `cd $(top_srcdir); pwd`" >> $@-t
	@echo "set srcdir `cd $(srcdir); pwd`" >> $@-t
	@echo 'set objdir' `pwd` >> $@-t
	@echo 'set host_alias "$(host_alias)"' >> $@-t
	@echo 'set host_triplet $(host_triplet)' >> $@-t
	@echo 'set target_alias "$(target_alias)"' >> $@-t
	@echo 'set target_triplet $(target_triplet)' >> $@-t
	@echo 'set build_alias "$(build_alias)"' >> $@-t
	@echo 'set build_triplet $(build_triplet)' >> $@-t
	@echo '## All variables above are generated by configure. Do Not Edit ##' >> $@-t
	@test ! -f site.exp || sed '1,/^## All variables above are.*##/ d' site.exp >> $@-t
	@test ! -f site.exp || mv site.exp site.bak
	@mv $@-t site.exp

remote.exp:;
	@echo 'Making a new remote.exp file...'
	@test ! -f remote.bak || rm -f remote.bak
	@echo '## These variables are used to set up for the remote testing.' >> $@-t
	@echo '## Please, read file README in this directory for instructions' >> $@-t
	@echo '## on how to use this file' >> $@-t
	@echo "set host_board `hostname`" >> $@-t
	@echo 'set board_info($$host_board,connect) rlogin' >> $@-t
	@echo 'set board_info($$host_board,shell_prompt) "\\$$ "' >> $@-t
	@echo "set board_info(\$$host_board,top_srcdir) `cd $(top_srcdir); pwd`" >> $@-t
	@echo "set board_info(\$$host_board,srcdir) `cd $(srcdir); pwd`" >> $@-t
	@echo "set board_info(\$$host_board,objdir) `pwd`" >> $@-t
	@echo "set board_info(\$$host_board,top_srcdir) `cd $(top_srcdir); pwd`" >> $@-t
	@echo "set board_info(\$$host_board,top_builddir) `cd $(top_builddir); pwd`" >> $@-t
	@test ! -f remote.exp || mv remote.exp remote.bak
	@mv $@-t remote.exp


DISTCLEANFILES=*.exp *.log *.sum 

distclean-local:
	-rm -rf	data
