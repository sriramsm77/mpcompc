/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 1999-2022 Free Software Foundation, Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General
   Public License along with this library.  If not, see
   <http://www.gnu.org/licenses/>. */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif  
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sieve-priv.h>

#define SIEVE_RT_ARG(m,n,t) ((m)->prog[(m)->pc+(n)].t)
#define SIEVE_RT_ADJUST(m,n) (m)->pc+=(n)

#define INSTR_DISASS(m) ((m)->state == mu_sieve_state_disass)
#define INSTR_DEBUG(m) \
  (INSTR_DISASS(m) || mu_debug_level_p (mu_sieve_debug_handle, MU_DEBUG_TRACE9)) 

void
_mu_i_sv_instr_locus (mu_sieve_machine_t mach)
{
  mu_locus_point_set_file (&mach->locus.beg,
			   mu_i_sv_id_str (mach, SIEVE_RT_ARG (mach, 0, pc)));
  mach->locus.beg.mu_line = SIEVE_RT_ARG (mach, 1, unum);
  mach->locus.beg.mu_col = SIEVE_RT_ARG (mach, 2, unum);

  mu_locus_point_set_file (&mach->locus.end,
			   mu_i_sv_id_str (mach, SIEVE_RT_ARG (mach, 3, pc)));
  mach->locus.end.mu_line = SIEVE_RT_ARG (mach, 4, unum);
  mach->locus.end.mu_col = SIEVE_RT_ARG (mach, 5, unum);
  mu_stream_ioctl (mach->errstream, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE, &mach->locus);
  if (INSTR_DEBUG (mach))
    mu_i_sv_debug (mach, mach->pc - 1, "LOCUS");
  SIEVE_RT_ADJUST (mach, 6);
}
		 
static int
instr_run (mu_sieve_machine_t mach, char const *what)
{
  int rc = 0;
  mu_sieve_handler_t han = SIEVE_RT_ARG (mach, 0, handler);
  mach->argstart = SIEVE_RT_ARG (mach, 1, pc);
  mach->argcount = SIEVE_RT_ARG (mach, 2, pc);
  mach->tagcount = SIEVE_RT_ARG (mach, 3, pc);
  mach->identifier = SIEVE_RT_ARG (mach, 4, string);
  mach->comparator = SIEVE_RT_ARG (mach, 5, comp);
				   
  SIEVE_RT_ADJUST (mach, 6);

  if (INSTR_DEBUG (mach))
    mu_i_sv_debug_command (mach, mach->pc - 7, what);
  else
    mu_i_sv_trace (mach, what);
  
  if (!INSTR_DISASS (mach))
    rc = han (mach);
  mach->argstart = 0;
  mach->argcount = 0;
  mach->tagcount = 0;
  mach->identifier = NULL;
  mach->comparator = NULL;
  return rc;
}

void
_mu_i_sv_instr_action (mu_sieve_machine_t mach)
{
  mach->action_count++;
  instr_run (mach, "ACTION");
}

void
_mu_i_sv_instr_test (mu_sieve_machine_t mach)
{
  mach->reg = instr_run (mach, "TEST");
}

void
_mu_i_sv_instr_not (mu_sieve_machine_t mach)
{
  if (INSTR_DEBUG (mach))
    mu_i_sv_debug (mach, mach->pc - 1, "NOT");
  if (INSTR_DISASS (mach))
    return;
  mach->reg = !mach->reg;
}

void
_mu_i_sv_instr_branch (mu_sieve_machine_t mach)
{
  long num = SIEVE_RT_ARG (mach, 0, number);

  SIEVE_RT_ADJUST (mach, 1);
  if (INSTR_DEBUG (mach))
    mu_i_sv_debug (mach, mach->pc - 2, "BRANCH %lu",
		   (unsigned long)(mach->pc + num));
  if (INSTR_DISASS (mach))
    return;

  mach->pc += num;
}

void
_mu_i_sv_instr_brz (mu_sieve_machine_t mach)
{
  long num = SIEVE_RT_ARG (mach, 0, number);
  SIEVE_RT_ADJUST (mach, 1);

  if (INSTR_DEBUG (mach))
    mu_i_sv_debug (mach, mach->pc - 2, "BRZ %lu",
		   (unsigned long)(mach->pc + num));
  if (INSTR_DISASS (mach))
    return;
  
  if (!mach->reg)
    mach->pc += num;
}
  
void
_mu_i_sv_instr_brnz (mu_sieve_machine_t mach)
{
  long num = SIEVE_RT_ARG (mach, 0, number);
  SIEVE_RT_ADJUST (mach, 1);

  if (INSTR_DEBUG (mach))
    mu_i_sv_debug (mach, mach->pc - 2, "BRNZ %lu",
		   (unsigned long)(mach->pc + num));
  if (INSTR_DISASS (mach))
    return;
  
  if (mach->reg)
    mach->pc += num;
}
  
void
mu_sieve_abort (mu_sieve_machine_t mach)
{
  longjmp (mach->errbuf, MU_ERR_FAILURE);
}

void
mu_sieve_set_data (mu_sieve_machine_t mach, void *data)
{
  mach->data = data;
}

void *
mu_sieve_get_data (mu_sieve_machine_t mach)
{
  return mach->data;
}

int
mu_sieve_get_locus (mu_sieve_machine_t mach, struct mu_locus_range *loc)
{
  return mu_locus_range_copy (loc, &mach->locus);
}

mu_mailbox_t
mu_sieve_get_mailbox (mu_sieve_machine_t mach)
{
  return mach->mailbox;
}

mu_message_t
mu_sieve_get_message (mu_sieve_machine_t mach)
{
  if (!mach->msg)
    mu_mailbox_get_message (mach->mailbox, mach->msgno, &mach->msg);
  return mach->msg;
}

size_t
mu_sieve_get_message_num (mu_sieve_machine_t mach)
{
  return mach->msgno;
}

const char *
mu_sieve_get_identifier (mu_sieve_machine_t mach)
{
  return mach->identifier;
}

void
mu_sieve_get_argc (mu_sieve_machine_t mach, size_t *args, size_t *tags)
{
  if (args)
    *args = mach->argcount;
  if (tags)
    *tags = mach->tagcount;
}

int
mu_sieve_is_dry_run (mu_sieve_machine_t mach)
{
  return mach->dry_run;
}

int
mu_sieve_set_dry_run (mu_sieve_machine_t mach, int val)
{
  if (mach->state != mu_sieve_state_compiled)
    return EINVAL; /* FIXME: another error code */
  return mach->dry_run = val;
}

int
sieve_run (mu_sieve_machine_t mach)
{
  int rc;

  mu_sieve_stream_save (mach);
  
  rc = setjmp (mach->errbuf);
  if (rc == 0)
    {
      mach->action_count = 0;

      mu_i_sv_init_variables (mach);
      
      for (mach->pc = 1; mach->prog[mach->pc].handler; )
	(*mach->prog[mach->pc++].instr) (mach);

      if (mach->action_count == 0)
	mu_sieve_log_action (mach, "IMPLICIT KEEP", NULL);
  
      if (INSTR_DEBUG (mach))
	mu_i_sv_debug (mach, mach->pc, "STOP");
    }
  
  mu_sieve_stream_restore (mach);
  
  return rc;
}

int
mu_sieve_disass (mu_sieve_machine_t mach)
{
  int rc;

  if (mach->state != mu_sieve_state_compiled)
    return EINVAL; /* FIXME: Error code */
  mach->state = mu_sieve_state_disass;
  rc = sieve_run (mach);
  mach->state = mu_sieve_state_compiled;
  return rc;
}
  
static int
_sieve_action (mu_observer_t obs, size_t type, void *data, void *action_data)
{
  mu_sieve_machine_t mach;
  
  if (type != MU_EVT_MESSAGE_ADD)
    return 0;

  mach = mu_observer_get_owner (obs);
  mach->msgno++;
  mu_mailbox_get_message (mach->mailbox, mach->msgno, &mach->msg);
  sieve_run (mach);
  return 0;
}

int
mu_sieve_mailbox (mu_sieve_machine_t mach, mu_mailbox_t mbox)
{
  int rc;
  size_t total;
  mu_observer_t observer;
  mu_observable_t observable;
  
  if (!mach || !mbox)
    return EINVAL;

  if (mach->state != mu_sieve_state_compiled)
    return EINVAL; /* FIXME: Error code */
  mach->state = mu_sieve_state_running;
  
  mu_observer_create (&observer, mach);
  mu_observer_set_action (observer, _sieve_action, mach);
  mu_mailbox_get_observable (mbox, &observable);
  mu_observable_attach (observable, MU_EVT_MESSAGE_ADD, observer);
  
  mach->mailbox = mbox;
  mach->msgno = 0;
  rc = mu_mailbox_scan (mbox, 1, &total);
  if (rc)
    mu_sieve_error (mach, _("mu_mailbox_scan: %s"), mu_strerror (errno));

  mu_observable_detach (observable, observer);
  mu_observer_destroy (&observer, mach);

  mach->state = mu_sieve_state_compiled;
  mach->mailbox = NULL;
  
  return rc;
}

int
mu_sieve_message (mu_sieve_machine_t mach, mu_message_t msg)
{
  int rc;
  
  if (!mach || !msg)
    return EINVAL;

  if (mach->state != mu_sieve_state_compiled)
    return EINVAL; /* FIXME: Error code */
  mach->state = mu_sieve_state_running;

  mach->msgno = 1;
  mach->msg = msg;
  mach->mailbox = NULL;
  rc = sieve_run (mach);
  mach->state = mu_sieve_state_compiled;
  mach->msg = NULL;
  
  return rc;
}
