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

sysincludedir=$(pkgincludedir)/sys
sysinclude_HEADERS = \
 amd.h\
 attribute.h\
 auth.h\
 body.h\
 dbm.h\
 debcat.h\
 dotmail.h\
 envelope.h\
 file_stream.h\
 filter.h\
 folder.h\
 gsasl-stream.h\
 header_stream.h\
 header.h\
 imap.h\
 imapio.h\
 iterator.h\
 iostream.h\
 list.h\
 logstream.h\
 mailbox.h\
 mailcap.h\
 mailer.h\
 mapfile_stream.h\
 mboxrd.h\
 memory_stream.h\
 message_stream.h\
 message.h\
 mime.h\
 mimetypes.h\
 monitor.h\
 msgset.h\
 nullstream.h\
 observer.h\
 pop3.h\
 prog_stream.h\
 property.h\
 rdcache_stream.h\
 registrar.h\
 smtp.h\
 streamref.h\
 streamtrans.h\
 stream.h\
 syslogstream.h\
 temp_file_stream.h\
 temp_stream.h\
 tls-stream.h\
 url.h\
 xscript-stream.h\
 $(top_srcdir)/libmailutils/wordsplit/wordsplit.h

BUILT_SOURCES = debcat.h

debcat.h: $(top_srcdir)/libmailutils/diag/debcat
	$(AM_V_GEN)$(AWK) -f $(mu_aux_dir)/debcat.awk $(top_srcdir)/libmailutils/diag/debcat > debcat.h

