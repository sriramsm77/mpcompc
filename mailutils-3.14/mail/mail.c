/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 1999-2022 Free Software Foundation, Inc.

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

#include "mail.h"
#include "mailutils/cli.h"
#include "mailutils/mu_auth.h"

#include <mailutils/monitor.h>
#include <mailutils/mpc.h>

/* Global variables and constants*/
mu_mailbox_t mbox;            /* Mailbox being operated upon */
size_t total;                 /* Total number of messages in the mailbox */
int interactive;              /* Is the session interactive */
int read_recipients;          /* Read recipients from the message (mail -t) */
mu_url_t secondary_url;       /* URL of the mailbox given with the -f option */
static mu_list_t command_list;/* List of commands to be executed after parsing
				 command line */
const char *program_version = "mail (" PACKAGE_STRING ")";


#define HINT_SEND_MODE   0x1
#define HINT_FILE_OPTION 0x2
#define HINT_BYNAME      0x4

int hint;
char *file;
char *user;

int mime_option;
int skip_empty_attachments;
char *default_encoding;
char *default_content_type;
static char *content_name;
static char *content_filename;

static void
cli_f_option (struct mu_parseopt *po, struct mu_option *opt, char const *arg)
{
  hint |= HINT_FILE_OPTION;
}

static void
cli_file_option (struct mu_parseopt *po, struct mu_option *opt, char const *arg)
{
  if (arg)
    file = mu_strdup (arg);
  hint |= HINT_FILE_OPTION;
}

static void
cli_command_option (struct mu_parseopt *po, struct mu_option *opt,
		    char const *arg)
{
  switch (opt->opt_short)
    {
    case 'e':
      util_cache_command (&command_list, "setq mode=exist");
      break;

    case 'p':
      util_cache_command (&command_list, "setq mode=print");
      break;

    case 'r':
      util_cache_command (&command_list, "set return-address=%s", arg);
      hint |= HINT_SEND_MODE;      
      break;

    case 'q':
      util_cache_command (&command_list, "set quit");
      break;

    case 't':
      read_recipients = 1;
      util_cache_command (&command_list, "set editheaders");
      hint |= HINT_SEND_MODE;      
      break;

    case 'H':
      util_cache_command (&command_list, "setq mode=headers");
      break;

    case 'i':
      util_cache_command (&command_list, "set ignore");
      break;

    case 'n':
      util_do_command ("set norc");
      break;

    case 'N':
      util_cache_command (&command_list, "set noheader");
      break;

    case 'E':
      util_cache_command (&command_list, "%s", arg);
      break;

    case 'F':
      hint |= HINT_SEND_MODE;
      hint |= HINT_BYNAME;
      break;

    case 0:
      mu_parseopt_error (po, _("--%s: option should have been recognized"),
			 opt->opt_long);
      exit (po->po_exit_error);

    default:
      mu_parseopt_error (po, _("-%c: option should have been recognized"),
			 opt->opt_short);
      exit (po->po_exit_error);
    }
}

static void
cli_subject (struct mu_parseopt *po, struct mu_option *opt, char const *arg)
{
  hint |= HINT_SEND_MODE;
  send_append_header2 (MU_HEADER_SUBJECT, arg, COMPOSE_REPLACE);
  util_cache_command (&command_list, "set noasksub");
}

static void
cli_append (struct mu_parseopt *po, struct mu_option *opt, char const *arg)
{
  hint |= HINT_SEND_MODE;
  send_append_header (arg);
}

static void
cli_attach (struct mu_parseopt *po, struct mu_option *opt, char const *arg)
{
  int fd = -1;

  hint |= HINT_SEND_MODE;
  if (strcmp (arg, "-") == 0)
    {
      arg = NULL;
      fd = 0;
    }
  if (send_attach_file (fd, arg, content_filename, content_name,
			default_content_type, default_encoding))
    exit (po->po_exit_error);

  mime_option = 1;
  
  free (content_name);
  content_name = NULL;
  free (content_filename);
  content_filename = NULL;
}

static void
cli_attach_fd (struct mu_parseopt *po, struct mu_option *opt, char const *arg)
{
  int rc, fd;

  hint |= HINT_SEND_MODE;
  rc = mu_str_to_c (arg, mu_c_int, &fd, NULL);
  if (rc)
    {
      mu_parseopt_error (po, _("%s: bad descriptor"), arg);
      exit (po->po_exit_error);
    }

  send_attach_file (fd, NULL, content_filename, content_name,
		    default_content_type, default_encoding);

  free (content_name);
  content_name = NULL;
  free (content_filename);
  content_filename = NULL;
}

static struct mu_option mail_options[] = {
  { NULL,     'f', NULL,      MU_OPTION_HIDDEN,
    NULL,
    mu_c_string, NULL, cli_f_option },
  { "file",   0,  N_("FILE"), MU_OPTION_ARG_OPTIONAL|MU_OPTION_HIDDEN,
    NULL,
    mu_c_string, NULL, cli_file_option },

  { "exist",  'e', NULL,      MU_OPTION_DEFAULT,
    N_("return true if mail exists"),
    mu_c_string, NULL, cli_command_option },

  { "byname", 'F', NULL,      MU_OPTION_DEFAULT,
    N_("save messages according to sender"),
    mu_c_string, NULL, cli_command_option },

  { "headers", 'H', NULL,     MU_OPTION_DEFAULT,
    N_("write a header summary and exit"),
    mu_c_string, NULL, cli_command_option },

  { "ignore",  'i', NULL,     MU_OPTION_DEFAULT,
    N_("ignore interrupts"),
    mu_c_string, NULL, cli_command_option },

  { "norc",    'n', NULL,     MU_OPTION_DEFAULT,
    N_("do not read the system mailrc file"),
    mu_c_string, NULL, cli_command_option },

  { "nosum",   'N', NULL,     MU_OPTION_DEFAULT,
    N_("do not display initial header summary"),
    mu_c_string, NULL, cli_command_option },

  { "print",   'p', NULL,     MU_OPTION_DEFAULT,
    N_("print all mail to standard output"),
    mu_c_string, NULL, cli_command_option },
  { "read",    0,   NULL,     MU_OPTION_ALIAS },

  { "return-address", 'r', N_("ADDRESS"), MU_OPTION_DEFAULT,
    N_("use address as the return address when sending mail"),
    mu_c_string, NULL, cli_command_option },

  { "quit",    'q', NULL,     MU_OPTION_DEFAULT,
    N_("cause interrupts to terminate program"),
    mu_c_string, NULL, cli_command_option },

  { "subject", 's', N_("SUBJ"), MU_OPTION_DEFAULT,
    N_("send a message with the given SUBJECT"),
    mu_c_string, NULL, cli_subject },

  { "to",      't', NULL,       MU_OPTION_DEFAULT,
    N_("read recipients from the message header"),
    mu_c_string, NULL, cli_command_option },

  { "user",    'u', N_("USER"), MU_OPTION_DEFAULT,
    N_("operate on USER's mailbox"),
    mu_c_string, &user },

  { "append",   'a', N_("HEADER: VALUE"), MU_OPTION_DEFAULT,
    N_("append given header to the message being sent"),
    mu_c_string, NULL, cli_append },

  { "alternative", 0, NULL, MU_OPTION_DEFAULT,
    N_("force multipart/alternative content type"),
    mu_c_bool, &multipart_alternative },

  { "skip-empty-attachments", 0, NULL, MU_OPTION_DEFAULT,
    N_("skip attachments with empty body"),
    mu_c_bool, &skip_empty_attachments },

  { "exec" ,    'E', N_("COMMAND"), MU_OPTION_DEFAULT,
    N_("execute COMMAND"),
    mu_c_string, NULL, cli_command_option },

  { "encoding",  0, N_("NAME"), MU_OPTION_DEFAULT,
    N_("set encoding for subsequent --attach options"),
    mu_c_string, &default_encoding },

  { "content-type", 0, N_("TYPE"), MU_OPTION_DEFAULT,
    N_("set content type for subsequent --attach options"),
    mu_c_string, &default_content_type },

  { "content-name", 0, N_("NAME"), MU_OPTION_DEFAULT,
    /* TRANSLATORS: Don't translate "Content-Type" and "name"! */
    N_("set the Content-Type name parameter for the next --attach option"),
    mu_c_string, &content_name },
  { "content-filename", 0, N_("NAME"), MU_OPTION_DEFAULT,
    /* TRANSLATORS: Don't translate "Content-Disposition" and "filename"! */
    N_("set the Content-Disposition filename parameter for the next --attach option"),
    mu_c_string, &content_filename },

  { "attach",  'A', N_("FILE"), MU_OPTION_DEFAULT,
    N_("attach FILE"),
    mu_c_string, NULL, cli_attach },

  { "attach-fd",  0, N_("FD"), MU_OPTION_DEFAULT,
    N_("attach from file descriptor FD"),
    mu_c_string, NULL, cli_attach_fd },

  { "mime",    'M',  NULL, MU_OPTION_DEFAULT,
    N_("compose MIME messages"),
    mu_c_bool, &mime_option },

  MU_OPTION_END
}, *options[] = { mail_options, NULL };

static const char *alt_args[] = {
  N_("[OPTION...] [file]"),
  N_("--file [OPTION...] [file]"),
  N_("--file=file [OPTION...]"),
  NULL
};

static struct mu_cli_setup cli = {
  options,
  NULL,
  /* TRANSLATORS: "mail" is the name of the program. Don't translate it. */
  N_("GNU mail -- process mail messages.\n"
     "If -f or --file is given, mail operates on the mailbox named "
     "by the first argument, or the user's mbox, if no argument given."),
  N_("[address...]"),
  alt_args,
  NULL,
  1,
  1
};

static char *mail_capa[] = {
  "address",
  "debug",
  "mailbox",
  "locking",
  NULL
};

static char *
mail_cmdline (void *closure, int cont MU_ARG_UNUSED)
{
  char *prompt = (char*) closure;
  char *rc;

  while (1)
    {
      if (mailvar_is_true (mailvar_name_autoinc)
	  && !mu_mailbox_is_updated (mbox))
	{
	  mu_mailbox_messages_count (mbox, &total);
	  page_invalidate (0);
	  mu_printf (_("New mail has arrived.\n"));
	}

      rc = ml_readline (prompt);

      if (ml_got_interrupt ())
	{
	  mu_error (_("Interrupt"));
	  continue;
	}

      if (!rc && mailvar_is_true (mailvar_name_ignoreeof))
	{
	  mu_error (_("Use \"quit\" to quit."));
	  continue;
	}

      break;
    }
  return rc;
}

static char *default_setup[] = {
  /* "set noallnet", */
  "setq append",
  "set asksub",
  "set crt",
  "set noaskbcc",
  "set askcc",
  "set noautoprint",
  "set nobang",
  "set nocmd",
  /*  "set nodebug",*/
  "set nodot",
  "set escape=~",
  "set noflipr",
  "set nofolder",
  "set header",
  "set nohold",
  "set noignore",
  "set noignoreeof",
  "set indentprefix=\"\t\"",
  "setq keep",
  "set nokeepsave",
  "set nometoo",
  "set noonehop",
  "set nooutfolder",
  "set nopage",
  "set prompt=\"? \"",
  "set norecord",
  "set save",
  "set nosendmail",
  "set nosendwait",
  "set noshowto",
  "set nosign",
  "set noSign",
  "set toplines=5",
  "set autoinc",
  "set regex",
  "set replyprefix=\"Re: \"",
  "set charset=auto",
  "set useragent",
  "unfold subject",
  "sender mail-followup-to reply-to from",
  "set nocmd",
  "set metamail",
  "set recursivealiases",
  "set noinplacealiases",
  "set fromfield",
  "set headline=\"%>%a%4m %18f %16d %3L/%-5o %s\"",
  "unset folder",
  "set fullnames",
  "set outfilename=local",

  /* Start in mail reading mode */
  "setq mode=read",
  "set noquit",
  "set rc",

  "set noflipr",
  "set noshowto",
  "set nobang",

  "set nullbody", /* Null message body is traditionally allowed */
  "set nullbodymsg=\"" N_("Null message body; hope that's ok") "\"",

  /* These settings are not yet used */
  "set noonehop",
  "set nosendwait",
};

static void
do_and_quit (const char *command)
{
  int rc = util_do_command ("%s", command);
  mu_mailbox_close (mbox);
  exit (rc != 0);
}

#ifdef MPC_SUPPORT

static int
_mpc_get_chunk_info (mu_message_t mesg, size_t msgno, mpc_chunk_msg_map_t **chunk_map_head)
{
    int rc;
    char *mpc_chunk_id = NULL;
    char *mpc_chunk_value_str = NULL;
    int curr_chunk = 0;
    int max_chunks = 0;
    mu_header_t hdr;

    mpc_mbox_info_t *mpc_msg_node = NULL;
    mpc_mbox_info_t *mpc_msg_prev = NULL;
    mpc_mbox_info_t *mpc_msg_temp = NULL;

    mpc_chunk_msg_map_t *node = NULL;
    mpc_chunk_msg_map_t *prev = NULL;
    mpc_chunk_msg_map_t *temp = NULL;

    /* Get the header list */      
    mu_message_get_header (mesg, &hdr);

    /* The headers in the email are parsed, let us extract the X-MPC-ID if present */ 
    rc = mu_header_aget_value_n(hdr, MU_HEADER_MPC_CHUNK_ID, 1, (char **)&mpc_chunk_id);
    if (rc) {
        if (rc == MU_ERR_NOENT) {
            /* This email message does not contain any MPC Chunk ID */
            fprintf(stderr, "No MPC Chunk ID in the message - %ld \n", msgno);
            return (0);
        } else {
            fprintf(stderr, "Error while checking for X-MPC-Chunk-ID in the message - %ld \n", msgno);
            return (-1);
        }
    } 

    /* Parse the chunk value header and obtain the curr chunk value */
    rc = mu_header_aget_value_n(hdr, MU_HEADER_MPC_CHUNK_VALUE, 1, (char **)&mpc_chunk_value_str);
    if (rc) {
        fprintf(stderr, "Error while getting the MPC chunk value in message %ld \n", msgno);
        return (-1);
    } else {
        sscanf(mpc_chunk_value_str, "[%3d/%3d]", &curr_chunk, &max_chunks);
        if ((curr_chunk == 0) && (max_chunks == 0)) {
            /* This is a coalesced message. Nothing to be done here */
            return (0);
        }
    }

    /* So, this message is a MPC fragment and a chunk (not a coalesced one) */

    /* First search for the given MPC Chunk ID in the list */
    /*  If found, add this message in the appropriate position based on the chunk value */
    /*  If not found, create a new entry in the hash map and add this message */
    if (*chunk_map_head == NULL) {
        *chunk_map_head = (mpc_chunk_msg_map_t *) malloc(sizeof(mpc_chunk_msg_map_t));
        if (*chunk_map_head == NULL) {
            fprintf(stderr, "Out of memory!!, bailing out!!\n");
            return (-1);
        }
        (*chunk_map_head)->next = NULL; 
        (*chunk_map_head)->head = NULL;
        (*chunk_map_head)->mpc_chunk_id[0] = '\0';
        (*chunk_map_head)->max_chunks = -1;

        node = *chunk_map_head;

        /* strncpy was giving some memory corruption issues, so had to resort to this */
        if (strlen(mpc_chunk_id) > sizeof(node->mpc_chunk_id)) 
            mpc_chunk_id[sizeof(node->mpc_chunk_id)-2] = '\0';
        strcpy(node->mpc_chunk_id, mpc_chunk_id);

    } else {
        node = *chunk_map_head;
        prev = NULL;

        while (node != NULL) {
            if (strncmp(node->mpc_chunk_id, mpc_chunk_id, sizeof(node->mpc_chunk_id)) == 0) {
                break;
            } else {
                prev = node;
                node = node->next;
            }
        }
    }

    if (node == NULL) {
        temp = (mpc_chunk_msg_map_t *) malloc(sizeof(mpc_chunk_msg_map_t));
        if (temp == NULL) {
            fprintf(stderr, "Out of memory!!, bailing out!!\n");
            return (-1);
        } 
        temp->next = NULL; 
        temp->head = NULL;
        temp->mpc_chunk_id[0] = '\0';
        temp->max_chunks = -1;

        if (strlen(mpc_chunk_id) > sizeof(temp->mpc_chunk_id)) 
            mpc_chunk_id[sizeof(temp->mpc_chunk_id)-2] = '\0';
        strcpy(temp->mpc_chunk_id, mpc_chunk_id);
        prev->next = temp; 
        temp->next = NULL;
        node = temp;
    }

    /* Parse the chunk value header and obtain the curr chunk value */
    node->max_chunks = max_chunks;

    /* Create a node and insert it in the map*/
    mpc_msg_temp = (mpc_mbox_info_t *) malloc(sizeof(mpc_mbox_info_t));                
    if (mpc_msg_temp == NULL) {
        fprintf(stderr, "Out of memory!!, bailing out!!\n");
        return (-1);
    }
    mpc_msg_temp->msgno = -1;
    mpc_msg_temp->chunk_id = -1;
    mpc_msg_temp->mesg = NULL;
    mpc_msg_temp->next = NULL; 

    mpc_msg_temp->msgno = msgno;
    mpc_msg_temp->chunk_id = curr_chunk;
    mpc_msg_temp->mesg = mesg;
    
    /* Insert it in the right spot */
    if (node->head == NULL) {
        node->head = mpc_msg_temp;
    } else {
        mpc_msg_node = node->head;                   
        while (mpc_msg_node != NULL) {
            if (mpc_msg_node->chunk_id > curr_chunk) {
                break;
            } else {
                mpc_msg_prev = mpc_msg_node;
                mpc_msg_node = mpc_msg_node->next;
            }
        }
        mpc_msg_temp->next = mpc_msg_prev->next;
        mpc_msg_prev->next = mpc_msg_temp;
    }

    if (mpc_chunk_id) 
        free (mpc_chunk_id);
    if (mpc_chunk_value_str)
        free (mpc_chunk_value_str);

    return 0;
}

static int
maildir_mpc_coaelesce_mails_unlocked (mu_mailbox_t mailbox, size_t total_mesgs)
{
    int i;
    int status;

    mu_header_t hdr;
    mu_off_t size = 0;
    mu_message_t mesg;

    mpc_chunk_msg_map_t *head = NULL;    
    mpc_chunk_msg_map_t *node = NULL;    
    mpc_chunk_msg_map_t *prev = NULL;    

    mpc_mbox_info_t *first_chunk_node = NULL;
    mpc_mbox_info_t *chunk_node = NULL;

    mu_body_t first_chunk_body = NULL;
    mu_body_t chunk_node_body  = NULL;

    mu_stream_t first_chunk_stream = NULL;
    mu_stream_t chunk_node_stream  = NULL;

    mu_attribute_t attr;

    mu_stream_t new_stream = NULL;

    char mpc_chunk_value_str[20];

    /* First, analyze the headers */
    for (i=1; i<=total_mesgs; i++) {
        if (util_get_message (mbox, i, &mesg) == 0) {
            if (_mpc_get_chunk_info (mesg, i, &head) != 0) {
                fprintf(stderr, "Got an error while getting chunk info - %d\n", i);
                continue;
            }
        }
    }

    node = head;
    while (node != NULL) {
        first_chunk_node = node->head;
        chunk_node = first_chunk_node->next;

        /* Get the stream reference of the 1st mesg chunk */
        if (mu_message_get_body (first_chunk_node->mesg, &first_chunk_body) != 0) {
            fprintf(stderr, "Could not get message body\n");
            return (-1);
        }
        status = mu_body_get_streamref (first_chunk_body, &first_chunk_stream);
        if (status) {
            mu_error (_("get_stream error: %s"), mu_strerror (status));
            return (-1);
        }

        /* Create a new email stream */
        if ((mu_memory_stream_create(&new_stream, MU_STREAM_RDWR)) != 0) {
            return (-1);
        }

        mu_stream_seek(first_chunk_stream, 0, MU_SEEK_SET, NULL);
        mu_stream_seek(new_stream, 0, MU_SEEK_SET, NULL);
        mu_stream_copy(new_stream, first_chunk_stream, 0, NULL);           

        while (chunk_node != NULL) {

            /* We need to coalesce the chunk_node's body into first_chunk_node's body 
                and delete the chunk_node mesg */
            if (mu_message_get_body (chunk_node->mesg, &chunk_node_body) != 0) {
                fprintf(stderr, "Could not get message body\n");
                return (-1);
            }
            status = mu_body_get_streamref (chunk_node_body, &chunk_node_stream);
            if (status) {
                mu_error (_("get_stream error: %s"), mu_strerror (status));
                return (-1);
            }
            mu_stream_seek(chunk_node_stream, 0, MU_SEEK_SET, NULL);
        
            if (mu_stream_size (new_stream, &size) != 0) {
                return(-1);
            }
            mu_stream_seek(new_stream, size, MU_SEEK_SET, NULL);
            mu_stream_copy(new_stream, chunk_node_stream, 0, NULL);           

#if 0
    /* Shows body of message - enable for debugging */
            {
                mu_off_t size;
                char buf[16384];
                mu_stream_seek(new_stream, 0, MU_SEEK_SET, NULL);
                if (mu_stream_size (new_stream, &size) != 0) {
                    return(-1);
                }
                if (size > 0) {
                    if (!mu_stream_read(new_stream, &buf, size, NULL)) {
                        fprintf(stderr, "\n---------------------------------\n");
                        fputs(buf, stderr);
                        fprintf(stderr, "\n---------------------------------\n");
                    } else {
                        fprintf(stderr, "Error occurred while reading env->mpc_chunks[%d}\n", i);
                    }
                }
            }
#endif

            first_chunk_node->next = chunk_node->next;

            /* Delete the message from the mailbox */
            mu_body_destroy(&chunk_node_body, chunk_node->mesg);            

            mu_message_get_attribute (chunk_node->mesg, &attr);
            mu_attribute_set_deleted (attr);
            mu_attribute_unset_userflag (attr, MAIL_ATTRIBUTE_PRESERVED);
            cond_page_invalidate (chunk_node->msgno);

            chunk_node->mesg = NULL; /* Do we need to free the mesg? */
            chunk_node->next = NULL;
            free(chunk_node);
            
            chunk_node = first_chunk_node->next;
        }

        /* Handle the first node now */
        /* Delete the old body before setting the body to the new stream */
        mu_body_destroy(&first_chunk_body, first_chunk_node->mesg);        

        /* We need to modify the MPC Chunk ID value in the header to prevent further coalescing on 
            an already coalesced email */
        /* Get the header list */      
        mu_message_get_header (first_chunk_node->mesg, &hdr);

        /* Set the MPC Chunk ID value in the header to [000/000] indicating 
            that this is a coalesced email */
        sprintf(mpc_chunk_value_str, "[%03d/%03d]", 0, 0);
        mu_header_set_value (hdr, MU_HEADER_MPC_CHUNK_VALUE , mpc_chunk_value_str, 1);
        mu_message_set_header (first_chunk_node->mesg, hdr, NULL);

        /* First set the first chunk stream to the combined stream */
        if (mu_body_set_stream(first_chunk_body, new_stream, first_chunk_node->mesg) != 0) {
            fprintf(stderr, "Could not set the stream of the first email!!\n");
            return (-1);
        }

        /* Append this message as a new message */
        if (mu_mailbox_append_message(mailbox, first_chunk_node->mesg) != 0) {
            fprintf(stderr, "Could not set the stream of the first email!!\n");
            return (-1);
        }

        /* Delete the first message as well, now that the coalesced message has 
            been added as a new message
         */
        mu_message_get_attribute (first_chunk_node->mesg, &attr);
        mu_attribute_set_deleted (attr);
        mu_attribute_unset_userflag (attr, MAIL_ATTRIBUTE_PRESERVED);
        cond_page_invalidate (first_chunk_node->msgno);

        first_chunk_node->mesg = NULL;
        free (first_chunk_node);
        node->head = NULL;

        prev = node;
        node = node->next;
        free (prev);
    }
    head = NULL; 

    return (0);                        
}
#endif

int
main (int argc, char **argv)
{
  char *mode = NULL, *prompt = NULL, *p;
  int i, rc;

  mu_stdstream_setup (MU_STDSTREAM_RESET_NONE);
  set_cursor (1);

  /* Native Language Support */
  MU_APP_INIT_NLS ();

  /* Register the desired formats.  */
  mu_register_all_formats ();

  mu_auth_register_module (&mu_auth_tls_module);

  interactive = isatty (fileno (stdin));
#ifdef HAVE_SIGACTION
  {
    struct sigaction act;
    act.sa_handler = SIG_IGN;
    sigemptyset (&act.sa_mask);
    act.sa_flags = 0;
    sigaction (SIGPIPE, &act, NULL);
  }
#else
  signal (SIGPIPE, SIG_IGN);
#endif

  /* set up the default environment */
  if (!getenv ("HOME"))
    setenv ("HOME", util_get_homedir (), 0);

  /* Set up the default environment */
  setenv ("DEAD", util_fullpath ("~/dead.letter"), 0);
  setenv ("EDITOR", "ed", 0);
  setenv ("LISTER", "ls", 0);
  setenv ("MAILRC", util_fullpath ("~/.mailrc"), 0);
  setenv ("MBOX", util_fullpath ("~/mbox"), 0);
  setenv ("PAGER", "more", 0);
  setenv ("SHELL", "sh", 0);
  setenv ("VISUAL", "vi", 0);

  /* set defaults for execution */
  util_do_command ("setq PID=\"%lu\"", (unsigned long) getpid ());
  for (i = 0; i < sizeof (default_setup)/sizeof (default_setup[0]); i++)
    util_do_command ("%s", default_setup[i]);

  p = getenv ("LINES");
  if (p && p[strspn (p, "0123456789")] == 0)
    util_do_command ("set screen=%s", p);
  else
    util_do_command ("set screen=%d", util_getlines ());

  p = getenv ("COLUMNS");
  if (p && p[strspn (p, "0123456789")] == 0)
    util_do_command ("set columns=%s", p);
  else
    util_do_command ("set columns=%d", util_getcols ());

  /* Set the default mailer to sendmail. FIXME: Minor memory leak. */
  mailvar_set (mailvar_name_sendmail,
	       mu_strdup ("sendmail:" PATH_SENDMAIL), mailvar_type_string,
	       MOPTF_OVERWRITE);

  /* argument parsing */
  mu_cli (argc, argv, &cli, mail_capa, NULL, &argc, &argv);

  if (default_content_type || default_encoding)
    mime_option = 1;
  if (mime_option)
    util_cache_command (&command_list, "set mime");

  if (read_recipients)
    {
      argv += argc;
      argc = 0;
    }

  if ((hint & (HINT_SEND_MODE|HINT_FILE_OPTION)) ==
      (HINT_SEND_MODE|HINT_FILE_OPTION))
    {
      mu_error (_("conflicting options"));
      exit (1);
    }
  else if (hint & HINT_FILE_OPTION)
    {
      if (file)
	{
	  if (argc)
	    {
	      mu_error (_("-f requires at most one command line argument"));
	      exit (1);
	    }
	}
      else if (argc)
	{
	  if (argc > 1)
	    {
	      mu_error (_("-f requires at most one command line argument"));
	      exit (1);
	    }
	  file = mu_strdup (argv[0]);
	}
      else if (user)
	mu_asprintf (&file, "~/%s/mbox", user);
      else
	file = mu_strdup ("~/mbox");
    }
  else if (argc || (hint & HINT_SEND_MODE))
    util_cache_command (&command_list, "setq mode=send");
  else if (user)
    mu_asprintf (&file, "%%%s", user);


  /* read system-wide mail.rc and user's .mailrc */
  if (mailvar_is_true (mailvar_name_rc))
    util_do_command ("source %s", SITE_MAIL_RC);
  if ((p = getenv ("MAILRC")) && *p)
    util_do_command ("source %s", getenv ("MAILRC"));

  util_run_cached_commands (&command_list);

  if (interactive)
    {
      /* Reset standard error stream so that it does not print program
	 name before the actual diagnostic message. */
      mu_stream_t errstr;
      int rc = mu_stdstream_strerr_create (&errstr, MU_STRERR_STDERR, 0, 0,
					   NULL, NULL);
      if (rc == 0)
	{
	  mu_stream_destroy (&mu_strerr);
	  mu_strerr = errstr;
	}
    }
  else
    {
      util_do_command ("set nocrt");
      util_do_command ("set noasksub");
      util_do_command ("set noaskcc");
      util_do_command ("set noaskbcc");
    }

  /* how should we be running? */
  if (mailvar_get (&mode, mailvar_name_mode, mailvar_type_string, 1))
    exit (EXIT_FAILURE);

  /* Interactive mode */

  ml_readline_init ();
  mail_set_my_name (user);

  /* Mode is just sending */
  if (strcmp (mode, "send") == 0)
    {
      --argv;
      ++argc;
      if (hint & HINT_BYNAME)
	argv[0] = "Mail";
      else
	argv[0] = "mail";
      return mail_send (argc, argv)
	       ? (mailvar_is_true (mailvar_name_mailx) ? 0 : EXIT_FAILURE)
	       : 0;
    }
  /* Or acting as a normal reader */
  else
    {
      if ((rc = mu_mailbox_create_default (&mbox, file)) != 0)
	{
	  if (file)
	    mu_error (_("Cannot create mailbox %s: %s"), file,
			mu_strerror (rc));
	  else
	    mu_error (_("Cannot create mailbox: %s"),
			mu_strerror (rc));
	  exit (EXIT_FAILURE);
	}

      if (file)
	{
	  /* Save URL of the file for further use */
	  mu_url_t url;

	  if (mu_mailbox_get_url (mbox, &url) == 0)
	    {
	      rc = mu_url_dup (url, &secondary_url);
	      if (rc)
		{
		  mu_diag_funcall (MU_DIAG_ERROR, "mu_url_dup", NULL, rc);
		  exit (EXIT_FAILURE);
		}
	    }
	  /* Destroy the content of file prior to freeing it: it can contain
	     password, although such usage is discouraged */
	  memset (file, 0, strlen (file));
	  free (file);
	}

      if ((rc = mu_mailbox_open (mbox, MU_STREAM_RDWR|MU_STREAM_CREAT)) != 0)
	{
	  if (rc == EACCES)
	    {
	      rc = mu_mailbox_open (mbox, MU_STREAM_READ);
	      if (rc == 0)
		mu_diag_output (MU_DIAG_WARNING, _("mailbox opened read-only"));
	      //FIXME: Disable 'q' and similar commands
	    }
	  if (rc)
	    {
	      mu_url_t url = NULL;
	      mu_mailbox_get_url (mbox, &url);
	      mu_error (_("Cannot open mailbox %s: %s"),
			mu_url_to_string (url), mu_strerror (rc));
	      mu_mailbox_destroy (&mbox);
	    }
	}

      if (rc)
	total = 0;
      else
	{
	  if ((rc = mu_mailbox_scan (mbox, 1, &total)) != 0)
	    {
	      mu_url_t url = NULL;
	      mu_mailbox_get_url (mbox, &url);
	      mu_error (_("Cannot read mailbox %s: %s"),
			  mu_url_to_string (url), mu_strerror (rc));
	      exit (EXIT_FAILURE);
	    }

#ifdef MPC_SUPPORT
        /* Coaelesce mails in new/ */
        //mu_monitor_wrlock (mbox.monitor);
        rc = maildir_mpc_coaelesce_mails_unlocked (mbox, total);
        if (rc != 0) {
            fprintf(stderr, "Unable to coalesce emails\n");
            exit (EXIT_FAILURE);
        }
        //mu_monitor_unlock (mbox.monitor);

	  if ((rc = mu_mailbox_scan (mbox, 1, &total)) != 0)
	    {
	      mu_url_t url = NULL;
	      mu_mailbox_get_url (mbox, &url);
	      mu_error (_("Cannot read mailbox %s: %s"),
			  mu_url_to_string (url), mu_strerror (rc));
	      exit (EXIT_FAILURE);
	    }

#endif

	  if (strcmp (mode, "exist") == 0)
	    {
	      mu_mailbox_close (mbox);
	      return (total < 1) ? 1 : 0;
	    }
	  else if (strcmp (mode, "print") == 0)
	    do_and_quit ("print *");
	  else if (strcmp (mode, "headers") == 0)
	    do_and_quit ("from *");
	  else if (strcmp (mode, "read"))
	    {
	      mu_error (_("Unknown mode `%s'"), mode);
	      util_do_command (mailvar_name_quit);
	      return 1;
	    }
	}

      if (total == 0
	  && (strcmp (mode, "read")
	      || !mailvar_is_true (mailvar_name_emptystart)))
	{
	  if (secondary_url)
	    mail_summary (0, NULL);
	  else
	    mu_printf (_("No mail for %s\n"), user ? user : mail_whoami ());
	  return 1;
	}

      /* initial commands */
      if (mailvar_is_true (mailvar_name_header))
	{
	  util_do_command ("summary");
	  util_do_command ("headers");
	}

      mailvar_get (&prompt, mailvar_name_prompt, mailvar_type_string, 0);
      mail_mainloop (mail_cmdline, (void*) prompt, 1);
      mu_printf ("\n");
      util_do_command (mailvar_name_quit);
      return 0;
    }
  /* We should never reach this point */
  return 1;
}


void
mail_mainloop (char *(*input) (void *, int),
	       void *closure, int do_history)
{
  char *command, *cmd;

  while ((command = (*input) (closure, 0)) != NULL)
    {
      int len = strlen (command);
      while (len > 0 && command[len-1] == '\\')
	{
	  char *buf;
	  char *command2 = (*input) (closure, 1);

	  if (!command2)
	    {
	      command[len-1] = 0;
	      break;
	    }
	  command[len-1] = '\0';
	  buf = mu_alloc ((len + strlen (command2)) * sizeof (char));
	  strcpy (buf, command);
	  strcat (buf, command2);
	  free (command);
	  command = buf;
	  len = strlen (command);
	}
      cmd = mu_str_stripws (command);
      util_do_command ("%s", cmd);
#ifdef WITH_READLINE
      if (do_history && !(mu_isspace (cmd[0]) || cmd[0] == '#'))
	add_history (cmd);
#endif
      free (command);
    }
}

int
mail_warranty (int argc MU_ARG_UNUSED, char **argv MU_ARG_UNUSED)
{
  mu_version_print (mu_strout);
  return 0;
}
