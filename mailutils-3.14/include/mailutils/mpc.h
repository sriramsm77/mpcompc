/* 
 * mpc.h - The include file for MPC
 *
 *
 */
#ifndef __MPC_H__
#define __MPC_H__

#include <mailutils/sys/header.h>

#define MPC_SUPPORT
#define MAX_MPC_SERVERS 2
#define MAX_MPC_CHUNKS  MAX_MPC_SERVERS

/* The node in the linked list that stores the
    mpc chunk information, including the message number
    to chunk id mapping
 */
typedef struct _mpc_mbox_info_t_ {
    int  msgno;
    int  chunk_id;
    mu_message_t mesg; /* The mesg that contains the body of the chunk/email */
    struct _mpc_mbox_info_t_ *next;    
} mpc_mbox_info_t;

/*
  The hashmap that stores the link list of 
   chunks for a given chunk ID
 */
typedef struct _mpc_chunk_msg_map_t_ {
    char mpc_chunk_id[50]; 
    int  max_chunks;
    mpc_mbox_info_t *head;
    struct _mpc_chunk_msg_map_t_ *next;
} mpc_chunk_msg_map_t;    

#endif /* __MPC_H__ */

