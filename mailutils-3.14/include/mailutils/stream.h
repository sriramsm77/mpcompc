/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2009-2022 Free Software Foundation, Inc.

   GNU Mailutils is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GNU Mailutils is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Mailutils.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef _MAILUTILS_STREAM_H
#define _MAILUTILS_STREAM_H

#include <stdlib.h>
#include <stdarg.h>
#include <mailutils/types.h>

#ifdef __cplusplus
extern "C" {
#endif

enum mu_buffer_type
  {
    mu_buffer_none,
    mu_buffer_line,
    mu_buffer_full
  };

#define MU_SEEK_SET      0
#define MU_SEEK_CUR      1
#define MU_SEEK_END      2

#define MU_STREAM_READ	      0x00000001
#define MU_STREAM_WRITE	      0x00000002
#define MU_STREAM_RDWR        (MU_STREAM_READ|MU_STREAM_WRITE)
#define MU_STREAM_SEEK        0x00000004
#define MU_STREAM_APPEND      0x00000008
#define MU_STREAM_CREAT	      0x00000010
/* So far used only by TCP streams. */
#define MU_STREAM_NONBLOCK    0x00000020
/* Not used                   0x00000040 */
/* Not used. Intended for mailboxes only. */
#define MU_STREAM_NONLOCK     0x00000080
/* Not used as well           0x00000100  */
/* FIXME: This one affects only mailboxes */  
#define MU_STREAM_QACCESS     0x00000200

#define MU_STREAM_RDTHRU      0x00000400
#define MU_STREAM_WRTHRU      0x00000800

#define MU_STREAM_IRGRP       0x00001000
#define MU_STREAM_IWGRP       0x00002000
#define MU_STREAM_IROTH       0x00004000
#define MU_STREAM_IWOTH       0x00008000
#define MU_STREAM_IMASK       0x0000F000

  /* Ioctl families */
#define MU_IOCTL_TRANSPORT        0  
#define MU_IOCTL_PROGSTREAM       1 /* Program stream */
#define MU_IOCTL_SEEK_LIMITS      2 /* Seek limits (get/set),
				       Arg: mu_off_t[2] */
#define MU_IOCTL_SUBSTREAM        3 /* Substream (get/set) */
#define MU_IOCTL_TRANSPORT_BUFFER 4 /* get/set */
#define MU_IOCTL_ECHO             5 /* get/set */
#define MU_IOCTL_NULLSTREAM       6 /* Null stream (see below) */
#define MU_IOCTL_LOGSTREAM        7 /* Log stream (see below) */
#define MU_IOCTL_XSCRIPTSTREAM    8 /* Transcript stream (see below) */
#define MU_IOCTL_FD               9 /* File descriptor manipulation */
#define MU_IOCTL_SYSLOGSTREAM    10 /* Syslog stream (see below) */
#define MU_IOCTL_FILTER          11 /* Filter streams (see below) */
#define MU_IOCTL_TOPSTREAM       12 /* Same as MU_IOCTL_SUBSTREAM, but
				       always returns the topmost substream.
				    */
#define MU_IOCTL_TLSSTREAM       13 /* TLS stream */
#define MU_IOCTL_WORDWRAPSTREAM  14 /* Word-wrapper stream */
#define MU_IOCTL_TCPSTREAM       15 /* TCP stream */

#define MU_IOCTL_TIMEOUT         16 /* Get or set the I/O timeout value
				       (struct timeval) */

  /* Opcodes common for various families */
#define MU_IOCTL_OP_GET 0
#define MU_IOCTL_OP_SET 1  

  /* Opcodes for MU_IOCTL_PROGSTREAM */  
#define MU_IOCTL_PROG_STATUS 0
#define MU_IOCTL_PROG_PID    1

  /* Opcodes for MU_IOCTL_NULLSTREAM */
  /* Set read pattern.
     Argument: struct mu_nullstream_pattern *pat.
     If pat==NULL, any reads from that stream will return EOF. */
#define MU_IOCTL_NULLSTREAM_SET_PATTERN 0
  /* Set pattern class for reads:  Argument int *pclass (a class mask
     from mailutils/cctype.h) */
#define MU_IOCTL_NULLSTREAM_SET_PATCLASS 1
  /* Limit stream size.  Argument: mu_off_t *psize; */
#define MU_IOCTL_NULLSTREAM_SETSIZE 2
  /* Lift the size limit.  Argument: NULL */
#define MU_IOCTL_NULLSTREAM_CLRSIZE 3
  
    /* Get or set logging severity.
       Arg: unsigned *
    */
#define MU_IOCTL_LOGSTREAM_GET_SEVERITY 0
#define MU_IOCTL_LOGSTREAM_SET_SEVERITY 1

  /* Codes 2 and 3 are unused now.
     For their prior use, see
      http://mailutils.org/wiki/Source_location_API#Deprecated_interface
  */

  /* Get or set log mode.
     Arg: int *
  */
#define MU_IOCTL_LOGSTREAM_GET_MODE     4
#define MU_IOCTL_LOGSTREAM_SET_MODE     5

  /* Set locus line.
     Arg: unsigned *
  */
#define MU_IOCTL_LOGSTREAM_SET_LOCUS_LINE  6
  /* Set locus column.
     Arg: unsigned *
  */
#define MU_IOCTL_LOGSTREAM_SET_LOCUS_COL   7
  
  /* Advance locus line.
     Arg: NULL (increment by 1)
          int *
  */
#define MU_IOCTL_LOGSTREAM_ADVANCE_LOCUS_LINE 8
  /* Advance locus column.
     Arg: NULL (increment by 1)
          int *
  */

#define MU_IOCTL_LOGSTREAM_ADVANCE_LOCUS_COL  9

  /* Suppress output of messages having severity lower than the
     given threshold.
     Arg: int *
  */
#define MU_IOCTL_LOGSTREAM_SUPPRESS_SEVERITY  10
  /* Same as above, but:
     Arg: const char *
  */
#define MU_IOCTL_LOGSTREAM_SUPPRESS_SEVERITY_NAME 11

  /* Get or set severity output mask.
     Arg: int *
  */
#define MU_IOCTL_LOGSTREAM_GET_SEVERITY_MASK 12
#define MU_IOCTL_LOGSTREAM_SET_SEVERITY_MASK 13 

  /* Clone the stream.
     Arg: mu_stream_t*
  */
#define MU_IOCTL_LOGSTREAM_CLONE 14

  /* Get locus range.
     Arg: struct mu_locus_range *
  */
#define MU_IOCTL_LOGSTREAM_GET_LOCUS_RANGE   15

  /* Set locus range.
     Arg: struct mu_locus_range *
  */
#define MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE   16

  /* Get prefix.
     Arg: char **
  */
#define MU_IOCTL_LOGSTREAM_GET_PREFIX        17

  /* Set prefix.
     Arg: char *
  */
#define MU_IOCTL_LOGSTREAM_SET_PREFIX        18  
  
  /* Opcodes for MU_IOCTL_XSCRIPTSTREAM */
  /* Swap transcript levels.
     Arg: int *X

     New transcript level is set to *X.
     If setting separate levels for each channel, use MU_XSCRIPT_LEVEL_PACK
     macro to pack them into one integer value. Use
     MU_IOCTL_XSCRIPTSTREAM_CHANNEL to configure single channel.
     
     Upon successful return, previous levels are stored in X. Use the
     MU_XSCRIPT_LEVEL_UNPACK macro to retrieve level for a particular
     channel.
  */
#define MU_IOCTL_XSCRIPTSTREAM_LEVEL 0
  
  /* Reconfigure a stream channel
     Arg: struct mu_xscript_channel *
   */
#define MU_IOCTL_XSCRIPTSTREAM_CHANNEL 1

  /* Opcodes for MU_IOCTL_FD */
  /* Get "borrow state".  Borrowed descriptors remain in open state
     after the stream is closed.
     Arg: int *
  */
#define MU_IOCTL_FD_GET_BORROW 0
  /* Set borrow state.
     Arg: int *
  */
#define MU_IOCTL_FD_SET_BORROW 1

  /* Opcodes for MU_IOCTL_SYSLOGSTREAM */
  /* Set logger function.
     Arg: void (*) (int, const char *, ...)
  */
#define MU_IOCTL_SYSLOGSTREAM_SET_LOGGER 0

  /* Get logger function.
     Arg: void (**) (int, const char *, ...)
  */
#define MU_IOCTL_SYSLOGSTREAM_GET_LOGGER 1

  /* Filter streams */
  /* Get or set disabled state:
     Arg: int*
  */
#define MU_IOCTL_FILTER_GET_DISABLED 0
#define MU_IOCTL_FILTER_SET_DISABLED 1  

  /* Set transcoder output buffer size.
     Arg: size_t*
     Has effect only if the stream is unbuffered
   */
#define MU_IOCTL_FILTER_SET_OUTBUF_SIZE  2
  
  /* TLS transport streams */
  /* Get cipher info.
     Arg: mu_property_t *
     On success, the following keys are defined: "protocol", "cipher", "mac"
  */
#define MU_IOCTL_TLS_GET_CIPHER_INFO 0
  
#define MU_TRANSPORT_INPUT  0
#define MU_TRANSPORT_OUTPUT 1
#define MU_TRANSPORT_VALID_TYPE(n) \
  ((n) == MU_TRANSPORT_INPUT || (n) == MU_TRANSPORT_OUTPUT)

/* Word wrapper streams */
/* Get left margin. */
#define MU_IOCTL_WORDWRAP_GET_MARGIN      0
/* Set left margin */  
#define MU_IOCTL_WORDWRAP_SET_MARGIN      1
/* Shift left margin relative to current position */ 
#define MU_IOCTL_WORDWRAP_MOVE_MARGIN     2
/* Set left margin for the next line */
#define MU_IOCTL_WORDWRAP_SET_NEXT_MARGIN 3
/* Get current column */
#define MU_IOCTL_WORDWRAP_GET_COLUMN      4

  /* TCP streams */

  /* Get socket name.
     Arg: struct mu_sockaddr **
  */
#define MU_IOCTL_TCP_GETSOCKNAME          0
  
  
struct mu_nullstream_pattern
{
  char *pattern;
  size_t size;
};
  
struct mu_buffer_query
{
  int type;                     /* One of MU_TRANSPORT_ defines */
  enum mu_buffer_type buftype;  /* Buffer type */
  size_t bufsize;               /* Buffer size */
};

/* Statistics */
enum
  {
    MU_STREAM_STAT_IN,          /* Bytes read */
    MU_STREAM_STAT_OUT,         /* Bytes written */
    MU_STREAM_STAT_READS,       /* Number of reads */
    MU_STREAM_STAT_WRITES,      /* Number of writes */
    MU_STREAM_STAT_SEEKS,       /* Number of seeks */
    MU_STREAM_STAT_INLN,        /* Lines read */
    MU_STREAM_STAT_OUTLN,       /* Lines written */
    MU_STREAM_STAT_IN8BIT,      /* 8-bit octets read */
    MU_STREAM_STAT_OUT8BIT,     /* 8-bit octets written */
    _MU_STREAM_STAT_MAX
  };

#define MU_STREAM_STAT_MASK(n)  (1U<<(n+1))

#define MU_STREAM_STAT_MASK_ALL  \
  (MU_STREAM_STAT_MASK (MU_STREAM_STAT_IN) | \
   MU_STREAM_STAT_MASK (MU_STREAM_STAT_OUT) |   \
   MU_STREAM_STAT_MASK (MU_STREAM_STAT_READS) | \
   MU_STREAM_STAT_MASK (MU_STREAM_STAT_WRITES) | \
   MU_STREAM_STAT_MASK (MU_STREAM_STAT_SEEKS) | \
   MU_STREAM_STAT_MASK (MU_STREAM_STAT_INLN) | \
   MU_STREAM_STAT_MASK (MU_STREAM_STAT_OUTLN))
   
typedef mu_off_t mu_stream_stat_buffer[_MU_STREAM_STAT_MAX];
int mu_stream_set_stat (mu_stream_t stream, int statmask,
			mu_stream_stat_buffer statbuf);
int mu_stream_get_stat (mu_stream_t stream, int *pstatmask,
			mu_off_t **pstatbuf);
  
#define MU_STREAM_DEFBUFSIZ 8192
extern size_t mu_stream_default_buffer_size;

void mu_stream_ref (mu_stream_t stream);
void mu_stream_unref (mu_stream_t stream);
void mu_stream_destroy (mu_stream_t *pstream);
int mu_stream_open (mu_stream_t stream);
const char *mu_stream_strerror (mu_stream_t stream, int rc);
int mu_stream_err (mu_stream_t stream);
int mu_stream_last_error (mu_stream_t stream);
void mu_stream_clearerr (mu_stream_t stream);
int mu_stream_seterr (struct _mu_stream *stream, int code, int perm);

int mu_stream_is_open (mu_stream_t stream);
int mu_stream_eof (mu_stream_t stream);
int mu_stream_seek (mu_stream_t stream, mu_off_t offset, int whence,
		    mu_off_t *pres);
int mu_stream_skip_input_bytes (mu_stream_t stream, mu_off_t count,
				mu_off_t *pres);

int mu_stream_set_buffer (mu_stream_t stream, enum mu_buffer_type type,
			  size_t size);
int mu_stream_get_buffer (mu_stream_t stream,
			  struct mu_buffer_query *qry);
int mu_stream_read (mu_stream_t stream, void *buf, size_t size, size_t *pread);
int mu_stream_timed_readdelim (mu_stream_t stream, char *buf, size_t size,
			       int delim, struct timeval *to, size_t *pread);
static inline int
mu_stream_readdelim (mu_stream_t stream, char *buf, size_t size,
		     int delim, size_t *pread)
{
  return mu_stream_timed_readdelim (stream, buf, size, delim, NULL, pread);
}

static inline int
mu_stream_timed_readline (mu_stream_t stream, char *buf, size_t size,
			  struct timeval *tv,
			  size_t *pread)
{
  return mu_stream_timed_readdelim (stream, buf, size, '\n', tv, pread);
}

static inline int
mu_stream_readline (mu_stream_t stream, char *buf, size_t size, size_t *pread)
{
  return mu_stream_timed_readline (stream, buf, size, NULL, pread);
}

int mu_stream_timed_getdelim (mu_stream_t stream, char **pbuf, size_t *psize,
			      int delim, struct timeval *to, size_t *pread);

static inline int
mu_stream_getdelim (mu_stream_t stream, char **pbuf, size_t *psize,
		    int delim, size_t *pread)
{
  return mu_stream_timed_getdelim (stream, pbuf, psize, delim, NULL, pread);
}

static inline int
mu_stream_timed_getline (mu_stream_t stream, char **pbuf, size_t *psize,
			 struct timeval *to,
			 size_t *pread)
{
  return mu_stream_timed_getdelim (stream, pbuf, psize, '\n', to, pread);
}

static inline int
mu_stream_getline (mu_stream_t stream, char **pbuf, size_t *psize,
		   size_t *pread)
{
  return mu_stream_timed_getline (stream, pbuf, psize, NULL, pread);
}

int mu_stream_write (mu_stream_t stream, const void *buf, size_t size,
		     size_t *pwrite);
int mu_stream_writeline (mu_stream_t stream, const char *buf, size_t size);
int mu_stream_flush (mu_stream_t stream);
int mu_stream_close (mu_stream_t stream);
int mu_stream_size (mu_stream_t stream, mu_off_t *psize);
int mu_stream_ioctl (mu_stream_t stream, int family, int opcode, void *ptr);
int mu_stream_truncate (mu_stream_t stream, mu_off_t);
int mu_stream_shutdown (mu_stream_t stream, int how);

#define MU_STREAM_READY_RD 0x1
#define MU_STREAM_READY_WR 0x2
#define MU_STREAM_READY_EX 0x4  
struct timeval;  /* Needed for the following declaration */ 

int mu_stream_wait   (mu_stream_t stream, int *pflags, struct timeval *);

void mu_stream_get_flags (mu_stream_t stream, int *pflags);
int mu_stream_set_flags (mu_stream_t stream, int fl);
int mu_stream_clr_flags (mu_stream_t stream, int fl);

int mu_stream_vprintf (mu_stream_t str, const char *fmt, va_list ap);
int mu_stream_printf (mu_stream_t stream, const char *fmt, ...)
                      MU_PRINTFLIKE(2,3);

int mu_stream_copy (mu_stream_t dst, mu_stream_t src, mu_off_t size,
		    mu_off_t *pcsz);
int mu_stream_copy_wcb (mu_stream_t dst, mu_stream_t src, mu_off_t size,
			void (*cbf) (char *, size_t, void *), void *cbd,
			mu_off_t *pcsz);
int mu_stream_copy_nl (mu_stream_t dst, mu_stream_t src, mu_off_t size,
		       mu_off_t *pcsz);

int mu_stream_header_copy (mu_stream_t dst, mu_stream_t src, char **exclude_names);

int mu_stream_shift (mu_stream_t str, mu_off_t off_a, mu_off_t off_b,
		     size_t bufsize);

int mu_file_stream_create (mu_stream_t *pstream, const char *filename, int flags);
struct mu_tempfile_hints;  
int mu_temp_file_stream_create (mu_stream_t *pstream,
				struct mu_tempfile_hints *hints, int flags);
int mu_fd_stream_create (mu_stream_t *pstream, char const *filename, int fd,
			 int flags);

#define MU_STDIN_FD  0
#define MU_STDOUT_FD 1
#define MU_STDERR_FD 2
int mu_stdio_stream_create (mu_stream_t *pstream, int fd, int flags);

int mu_memory_stream_create (mu_stream_t *pstream, int flags);
int mu_static_memory_stream_create (mu_stream_t *pstream, const void *mem,
				    size_t size);
int mu_fixed_memory_stream_create (mu_stream_t *pstream, void *mem,
				   size_t size, int flags);

int mu_mapfile_stream_create (mu_stream_t *pstream, const char *filename,
			      int flags);
int mu_socket_stream_create (mu_stream_t *pstream, const char *filename,
			     int flags);

int mu_streamref_create_abridged (mu_stream_t *pref, mu_stream_t str,
				  mu_off_t start, mu_off_t end);
int mu_streamref_create (mu_stream_t *pref, mu_stream_t str);

int mu_tcp_stream_create_from_sa (mu_stream_t *pstream,
				  struct mu_sockaddr *remote_addr,
				  struct mu_sockaddr *source_addr, int flags);

int mu_tcp_stream_create_with_source_ip (mu_stream_t *stream,
					 const char *host, unsigned port,
					 unsigned long source_ip,
					 int flags);
int mu_tcp_stream_create_with_source_host (mu_stream_t *stream,
					   const char *host, unsigned port,
					   const char *source_host,
					   int flags);
int mu_tcp_stream_create (mu_stream_t *stream, const char *host, unsigned port,
			  int flags);

/* Transcript log levels */
#define MU_XSCRIPT_NORMAL  0  /* Normal transcript */
#define MU_XSCRIPT_SECURE  1  /* Security-related data filtered out */
#define MU_XSCRIPT_PAYLOAD 2  /* Actual payload (may be copious) */

#define MU_XSCRIPT_LEVEL_PACK_BIT 0x100
#define MU_XSCRIPT_LEVEL_IS_PACKED(m) ((m) & MU_XSCRIPT_LEVEL_PACK_BIT)

/* Packing/unpacking of channel modes for the
   MU_IOCTL_XSCRIPTSTREAM.MU_IOCTL_XSCRIPTSTREAM_LEVEL ioctl.
*/
#define MU_XSCRIPT_LEVEL_MASK(chan, mode)	\
  (((mode) & 0x3) << ((chan)<<2))
#define MU_XSCRIPT_LEVEL_UNMASK(chan, mode)	\
  ((((mode) & 0xff) >> ((chan)<<2)) & 0x3)

/* Combine input and output channel modes into one integer, unless they
   are the same, in which case return input mode unchanged.
 */
#define MU_XSCRIPT_LEVEL_PACK(imode, omode)	\
  ((imode) == (omode)				\
   ? (imode)					\
   : (MU_XSCRIPT_LEVEL_PACK_BIT			\
      | MU_XSCRIPT_LEVEL_MASK(0,imode)		\
      | MU_XSCRIPT_LEVEL_MASK(1,omode)))
#define MU_XSCRIPT_LEVEL_UNPACK(chan, pack)	\
  (MU_XSCRIPT_LEVEL_IS_PACKED(pack)		\
   ? MU_XSCRIPT_LEVEL_UNMASK(chan, pack) : (pack))

struct mu_xscript_channel
{
  int cd;                   /* Channel descriptor */
  int level;                /* Channel transcript level */
  size_t length;            /* Length of data, for MU_XSCRIPT_PAYLOAD */
};
	
int mu_xscript_stream_create(mu_stream_t *pref, mu_stream_t transport,
			     mu_stream_t logstr,
			     const char *prefix[]);

int mu_iostream_create (mu_stream_t *pref, mu_stream_t in, mu_stream_t out);
int mu_dbgstream_create(mu_stream_t *pref, int severity);

int mu_rdcache_stream_create (mu_stream_t *pstream, mu_stream_t transport,
			      int flags);

int mu_nullstream_create (mu_stream_t *pref, int flags);

int mu_wordwrap_stream_create (mu_stream_t *pstream, mu_stream_t transport,
			       size_t left_margin, size_t right_margin);

extern size_t mu_temp_file_threshold_size;
int mu_temp_stream_create (mu_stream_t *pstream, size_t threshold);

#ifdef __cplusplus
}
#endif
  
#endif
