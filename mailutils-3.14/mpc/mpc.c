/* 
 * mpc.c 
 *	- Contains routines that enable MPC in mailx
 *
 */

#include "mail.h"
#include <mailutils/mime.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "srvquery.h"
#include "mpc.h"

static *mesg_body_chunks[MAX_MPC_SERVERS];

static
int chunkifyEMailBody(compose_env_t *env, int max_chunks) 
{
    int i;

    mu_stream_seek (env->compstr, 0, MU_SEEK_SET, NULL);

    i = 0;
    while (!mu_stream_eof(env->compstr)) {
        mu_stream_create(mesg_body_chunks[
    
}

static
char *getDomainFromEmailAddress(const char *email_addr) 
{
    int index = 0;
    char *ret_ptr = NULL;

    index = 0;
    while (index < strlen(email_addr)) {
        if (email_addr[index] == '@') {
            ret_ptr = &email_addr[index + 1];
            break;
        }
        index++;
    }                
    return(ret_ptr);
}

static
int getSRVRecords(const char *domain, SRVRecord *srvRec, int max_srv_recs) 
{
	char srv_record_str[256];
	int i;
	SRVRecord temp_srv_rec;

	for (i=0; i<max_srv_recs; i++) {
		memset(&temp_srv_rec, 0, sizeof(SRVRecord));
		srv_record_str[0] = '\0';
		snprintf(srv_record_str, sizeof(srv_record_str)-1, "_mpc%d._tcp.%s", i, domain);
		if (resolveSRV(srv_record_str, &temp_srv_rec) == -1) {
			return (i);
		} else {
			/* Ok, we got a SRV record, so save it in the return structure */
			srvRec[i].priority = temp_srv_rec.priority;
			srvRec[i].weight   = temp_srv_rec.weight;
			srvRec[i].port     = temp_srv_rec.port;
			strncpy(srvRec[i].dname, temp_srv_rec.dname, MAXCDNAME);
		}
	}

	return (i);
}

static
int mpc_mail_compose_send (compose_env_t *env, int save_to) 
{
	int done = 0;
	int rc;
	char *savefile = NULL;
	int int_cnt;
	char *escape;
	int i;
    char *to_addr = NULL;
    char *to_domain = NULL;
	SRVRecord srv_recs[MAX_MPC_SERVERS];

	/* Prepare environment */
	rc = mu_temp_stream_create (&env->compstr, 0);
	if (rc) {
		mu_error (_("Cannot open temporary file: %s"), mu_strerror (rc));
		return 1;
	}

	for (i=0; i<MAX_MPC_SERVERS; i++) {
		memset(&srv_recs[i], sizeof(SRVRecord), 0);
	}

	ml_clear_interrupt ();
	int_cnt = 0;
	while (!done) {
		char *buf;
		buf = ml_readline (" \b");

		if (ml_got_interrupt ()) {
			if (buf)
				free (buf);
				if (mailvar_is_true (mailvar_name_ignore)) {
					mu_printf ("@\n");
				} else {
					if (++int_cnt == 2)
						break;
					mu_error (_("\n(Interrupt -- one more to kill letter)"));
				}
				continue;
			}

		if (!buf) {
			if (interactive && mailvar_is_true (mailvar_name_ignoreeof)) {
				mu_error (mailvar_is_true (mailvar_name_dot) 
                          ?  _("Use \".\" to terminate letter.") 
                          : _("Use \"~.\" to terminate letter."));
				continue;
			} else
				break;
		}

		int_cnt = 0;

		if (strcmp (buf, ".") == 0 && mailvar_is_true (mailvar_name_dot))
			done = 1;
		else if (interactive 
				 && mailvar_get (&escape, mailvar_name_escape, mailvar_type_string, 0) == 0
	       		 && buf[0] == escape[0]) {
			if (buf[1] == buf[0])
				mu_stream_printf (env->compstr, "%s\n", buf + 1);
			else if (buf[1] == '.')
				done = 1;
			else if (buf[1] == 'x') {
				int_cnt = 2;
				done = 1;
			} else {
				struct mu_wordsplit ws;

				if (mu_wordsplit (buf + 1, &ws, MU_WRDSF_DEFFLAGS) == 0) {
					if (ws.ws_wordc > 0) {
						const struct mail_escape_entry *entry = 
									mail_find_escape (ws.ws_wordv[0]);

						if (entry)
							(*entry->escfunc) (ws.ws_wordc, ws.ws_wordv, env);
						else
							mu_error (_("Unknown escape %s"), ws.ws_wordv[0]);
					} else
						mu_error (_("Unfinished escape"));
					mu_wordsplit_free (&ws);
				} else {
					mu_error (_("Cannot parse escape sequence: %s"),
								  mu_wordsplit_strerror (&ws));
				}
	    	}
		} else
			mu_stream_printf (env->compstr, "%s\n", buf);
		mu_stream_flush (env->compstr);
		free (buf);
    }

    /* If interrupted, dump the file to dead.letter.  */
    if (int_cnt) {
        save_dead_message_env (env);
        return 1;
    }

    /* In mailx compatibility mode, ask for Cc and Bcc after editing
        the body of the message */
    if (mailvar_is_true (mailvar_name_mailx))
        read_cc_bcc (env);

    /* First from the domain mentioned in TO, get the 
        SRV record. We then parse the SRV record to 
        determine the number of chunks we need to make
     */
    /* env->compstr has the complete body of the email 
       We will need to split it into required number of chunks 
    */
    /* After we split the email into chunks, we 
        add the X-MPC-Via header and send it to 
        each one of them
    */

    if (mu_header_sget_value (env->header, MU_HEADER_TO, &to_addr)) { 
        to_domain = getDomainFromEmailAddress(to_addr);
        if (to_domain != NULL) {
            srv_rec_cnt = getSRVRecords(to_domain, srv_recs, MAX_MPC_SERVERS);
            if (srv_rec_cnt <= 0) {
                /* MPC SRV records are not configured in the receiver's domain */
                srv_rec_cnt = 0;
            }            
        }
    }            

    /* Prepare the header */
    if (mailvar_is_true (mailvar_name_useragent))
        mu_header_set_value (env->header, MU_HEADER_USER_AGENT, program_version, 1);

    if (util_header_expand_aliases (&env->header) == 0) {
        mu_message_t msg = NULL;
        int status = 0;
        int sendit = (compose_header_get (env, MU_HEADER_TO, NULL) ||
                        compose_header_get (env, MU_HEADER_CC, NULL) ||
                        compose_header_get (env, MU_HEADER_BCC, NULL));
        int total_chunks = srv_rec_cnt;
        int curr_chunk = 0;

        do {
            status = mu_message_create (&msg, NULL);
            if (status)
                break;
	  
            /* Fill the body. */
            mu_stream_seek (env->compstr, 0, MU_SEEK_SET, NULL);
            status = fill_body (msg, env->compstr);
            if (status)
                break;
	  
            mu_message_set_header (msg, env->header, NULL);
            env->header = NULL;

            status = add_attachments (env, &msg);
            if (status)
                break;
	  
            message_add_date (msg);

            /* Save outgoing message */
            if (save_to) {
                mu_header_t hdr;
                char const *rcpt;

                mu_message_get_header (msg, &hdr);
                if (mu_header_sget_value (hdr, MU_HEADER_TO, &rcpt) == 0) {
                    mu_address_t addr = NULL;
                    struct mu_address hint = MU_ADDRESS_HINT_INITIALIZER;

                    mu_address_create_hint (&addr, rcpt, &hint, 0);
                    savefile = util_outfilename (addr);
                    mu_address_destroy (&addr);
                }
            }
            util_save_outgoing (msg, savefile);
            if (savefile)
                free (savefile);

            /* Check if we need to save the message to files or pipes.  */
            if (env->outfiles) {
                int i;
                for (i = 0; i < env->nfiles; i++) {
                    /* Pipe to a cmd.  */
                    if (env->outfiles[i][0] == '|')
                        status = msg_to_pipe (env->outfiles[i] + 1, msg);
                        /* Save to a file.  */
                    else {
                        mu_mailbox_t mbx = NULL;
                        status = mu_mailbox_create_default (&mbx, env->outfiles[i]);
                        if (status == 0) {
                            status = mu_mailbox_open (mbx, MU_STREAM_WRITE | MU_STREAM_CREAT);
                            if (status == 0) {
                                status = mu_mailbox_append_message (mbx, msg);
                                if (status)
                                    mu_error (_("Cannot append message: %s"),
                                                mu_strerror (status));
                                mu_mailbox_close (mbx);
                            }
                            mu_mailbox_destroy (&mbx);
                        }
                        if (status)
                            mu_error (_("Cannot create mailbox %s: %s"), 
                                        env->outfiles[i],
                                        mu_strerror (status));
                    }
                }
            }
	  
            /* Do we need to Send the message on the wire?  */
            if (status == 0 && sendit) {
                status = send_message (msg);
                if (status) {
                    mu_error (_("cannot send message: %s"),
                                mu_strerror (status));
                    save_dead_message (msg);
                }
            }
        } while (0);

        mu_stream_destroy (&env->compstr);
        mu_message_destroy (&msg, NULL);
        return status;
    } else
        save_dead_message_env (env);

    return 1;
}

