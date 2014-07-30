/*
 * Copyright (c) 2012-2013, INAOS GmbH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the INAOS GmbH nor the names of its contributors
 *       may be used to endorse or promote products derived from this software 
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL INAOS GmbH BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */
#ifndef _LIBINAC_ISCP_H_
#define _LIBINAC_ISCP_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * INAOS Simple Command Protocol 
 */

#define INA_ISCP_NET_TIMEOUT  (10)    /* Default Net connect timeout in seconds */
#define INA_ISCP_BUFFER_SIZE  (2048)  /* Max size of command data */
#define INA_ISCP_HDR_SIZE (sizeof(uint16_t)*4+sizeof(uint32_t)+sizeof(ina_rc_t))

#define INA_ISCP_TYPE_INT64  (1)  /* uint64_t 8 bytes*/
#define INA_ISCP_TYPE_DBL    (2)  /* double 8 bytes */
#define INA_ISCP_TYPE_STR    (3)  /* uint32_t (length) + char[length] */

#define INA_ISCP_SEND_CMD(cmd_id, p_count, r_count)   \
 { cmd_id, p_count, r_count, NULL, {NULL} }

#define INA_ISCP_SENDRECV_CMD(cmd_id, p_count, r_count, handler)   \
 { cmd_id, p_count, r_count, handler, {NULL} }
 
#define INA_ISCP_CMDS(name, ...)     \
ina_iscp_cmd_t name[] = {            \
    __VA_ARGS__,                     \
    INA_ISCP_SEND_CMD(0, 0, 0),      \
    };
    
/* Backend */
typedef enum ina_iscp_backend_e {
    INA_ISCP_NONE = 0,
    INA_ISCP_INET,
    INA_ISCP_DEFAULT = INA_ISCP_INET,
} ina_iscp_backend_t;

/* ISCP parameter */
typedef struct ina_iscp_param_s {
    uint8_t type;
    union {
        int64_t   i;
        double    d;
        ina_str_t s;
    } value;
} ina_iscp_param_t;

/* Command handler */
typedef ina_rc_t (*ina_iscp_handler_t)(int, int, const ina_iscp_param_t*, int, ina_iscp_param_t**);

/* Internal registry entry */
typedef struct ina_ispc_cmd_s {
    int cmd_id;
    uint16_t p_count;
    uint16_t r_count;
    ina_iscp_handler_t handler;
    UT_hash_handle hh;
} ina_iscp_cmd_t;

/* Internal send/receive message */
typedef struct ina_iscp_msg_s {
    uint16_t length;    /* store the command buffer size */
    uint32_t cmd_uid;   /* UID for sent commands */
    uint16_t cmd_id;    /* identify the command */
    uint16_t p_count;   /* send parameter count */
    uint16_t r_count;   /* retuen value count */
    ina_rc_t rc;        /* return code */
    /* Parameters 
     * [1 byte, parameter type][parameter]
     * 
     * [INA_ISCP_TYPE_INT64][b1][b2][b3][b4]
     * [INA_ISPP_TYPE_STR][string lenght b1][b2 str lenght][b1][b2][bxx...]
     * [INA_ISCP_TYPE_DBL][b1][b2][b3][b4][b5][b6][b7][b8]
     *
     * [CRC32] 
     */
    unsigned char cmd_data[INA_ISCP_BUFFER_SIZE]; 
} ina_iscp_msg_t;

/* Open channel callback */
typedef ina_rc_t (*ina_iscp_open_cb)(void *user_data, int send);
/* Close channel callback */
typedef ina_rc_t (*ina_iscp_clse_cb)(void *user_data, int send);
/* Send callback */
typedef ina_rc_t (*ina_iscp_send_cb)(void *user_data, ina_iscp_msg_t*);
/* Receive callback */
typedef ina_rc_t (*ina_iscp_recv_cb)(void *user_data, ina_iscp_msg_t*);
/* Reponse callback */
typedef ina_rc_t (*ina_iscp_retn_cb)(void *user_data, ina_iscp_msg_t*);

/* ISCP context */
typedef struct ina_iscp_ctx_s {
    ina_iscp_backend_t backend;
    ina_iscp_open_cb open_cb;
    ina_iscp_clse_cb clse_cb;
    ina_iscp_send_cb send_cb;
    ina_iscp_recv_cb recv_cb;
    ina_iscp_retn_cb retn_cb;
    ina_timer_t      *timer;
    ina_time_event_t *time_event;
    ina_iscp_cmd_t   *cmds;
    ina_mempool_t    *mempool;
    ina_iscp_msg_t   last_response;
    void *user_data;
} ina_iscp_ctx_t;

/* ISCP context for TCP IP */
typedef struct ina_iscp_tcp_data_s {
    ina_str_t addr;     /* IP */
    int       port;     /* Port */
    int       fd;       /* File descriptor */
    int       lfd;      /* File descriptor for listener */
    int       timeout_sec; /* Timeout for TCP connect  default 10 seconds */
} ina_iscp_tcp_data_t;

/*
 * Create a generic ISCP context
 *
 * Parameters
 * ctx          Pointer to a context pointer to create
 * backend      Specifies the type of backend to use
 *
 * Return Value
 * INA_SUCCESS if no error occurred
 */
INA_API(ina_rc_t) ina_iscp_create(ina_iscp_ctx_t **ctx, ina_iscp_backend_t backend);

/*
 * Create a generic ISCP context
 *
 * Parameters
 * ctx          Pointer to a context pointer to create
 *
 * Return Value
 * INA_SUCCESS if no error occurred
 */
INA_API(ina_rc_t) ina_iscp_create_tcp(ina_iscp_ctx_t **ctx, const char* addr, int port);

/*
 * Set the send and receive callbacks.
 *
 * Parameters
 * ctx          Valid ISCP context
 * open_cb      Open channel callback
 * clse_cb      Close channel callback
 * send_cb      Send callback function
 * recv_cb      Receive callback
 * retn_cb      Return callback
 *
 * Return Value
 * INA_SUCCESS if no error occurred
 * EINVAL      if any of the paramaters is invalid
 */
INA_API(ina_rc_t) ina_iscp_set_callbacks(ina_iscp_ctx_t *ctx,
                                         ina_iscp_open_cb open_cb,
                                         ina_iscp_clse_cb clse_cb,
                                         ina_iscp_send_cb send_cb,
                                         ina_iscp_recv_cb recv_cb,
                                         ina_iscp_retn_cb retn_cb);
/*
 * Reset ISCP status and remove all registred commands.
 *
 * Parameters
 * ctx          Pointer to a context pointer to create
 *
 * Return Value:
 * INA_SUCCESS if successfully cleared.
 */
INA_API(ina_rc_t) ina_iscp_destroy(ina_iscp_ctx_t **ctx);

/*
 * Register a ISCP command.
 *
 * Parameters
 * ctx          Valid ISCP context
 * cmd_id       Command identifier
 * p_count      Number of parameters to send or to receive
 * r_count      Number of return values to send back  
 * handler      Command handler function called when the command is 
 *              received.
 *
 * Return Value
 * INA_SUCCESS if no error occurred
 */
INA_API(ina_rc_t) ina_iscp_register(ina_iscp_ctx_t *ctx, int cmd_id,int p_count, 
                                      int r_count, ina_iscp_handler_t handler);

/*
 * Register one or more ISCP commands at once. Use INA_ISCP_CMDS, 
 * INA_ISCP_SEND_CMD and INA_ISCP_SENDRECV_CMD macros to declare the 
 * command array
 *
 * Parameters
 * ctx          Valid ISCP context
 * cmds         Command pointer array
 *
 * Return Value
 * INA_SUCCESS if no error occurred
 */                                      
INA_API(ina_rc_t) ina_iscp_register_ex(ina_iscp_ctx_t *ctx, ina_iscp_cmd_t *cmds);

/*
 * Send a command synchronously.
 * Like:
 * ina_iscp_send(ctx, INAFX_ISCP_SUBSCRIBE, 
 *                   INA_ISCP_TYPE_INT, 1,
 *                   INA_ISCP_TYPE_STR, "127.0.0.1"
 *                   INA_ISCP_TYPE_DBL, 2.3,
 *                   INA_ISCP_TYPE_INT, 3);
 *
 * Parameters
 * ctx       Valid ISCP context
 * cmd_id   Commmand identifier
 * ...      Command parameter list
 *
 * Return Value
 * INA SUCCESS if command was successfully sent to the server and 
 *             executed by the receiver w/o error.
 */
INA_API(ina_rc_t) ina_iscp_send(ina_iscp_ctx_t *ctx, int cmd_id, ...);

/*
 * Make return values. Use this function only in ISCP handler.
 *
 * Like:
 * ina_iscp_params_t *value = NULL;
 * ina_iscp_return_values(&values, 
 *                   INA_ISCP_TYPE_INT, 1,
 *                   INA_ISCP_TYPE_STR, "127.0.0.1"
 *                   INA_ISCP_TYPE_DBL, 2.3,
 *                   INA_ISCP_TYPE_INT, 3);
 *
 * Parameters
 * count    Number of values
 * retvals  Return values handle
 * ...      Return values list
 *
 * Return Value
 * INA SUCCESS
 */
INA_API(ina_rc_t) ina_iscp_set_return_values(ina_iscp_param_t **values, int count, ...);
/*
 * Get return values from the last ISCP call.
 *
 * Like:
 * int i;
 * ina_str_t str;
 * double d;
 * ina_iscp_get_last_return_values(ctx, 
 *                   INA_ISCP_TYPE_INT, &i,
 *                   INA_ISCP_TYPE_STR, &str
 *                   INA_ISCP_TYPE_DBL, &d);
 *
 * Parameters
 * ctx      Valid ISCP context
 * retvals  Return values handle
 * ...      Return values list
 *
 * Return Value
 * INA SUCCESS
 */
INA_API(ina_rc_t) ina_iscp_get_last_return_values(const ina_iscp_ctx_t *ctx, ...);

/*
 * Check and receive a previously regsisterd command. If a
 * command was received the command handler will be invoked
 * synchronously.
 *
 * Parameters
 * ctx   Valid ISCP context
 * nc    Num of loops.
 * wait  Number of milliseconds to wait for next try
 *
 * Return Value:
 * INA_SUCCESS if no error occurred.
 */
INA_API(ina_rc_t) ina_iscp_recv(ina_iscp_ctx_t *ctx, int nc, int wait_msec);

#ifdef __cplusplus
}
#endif 

#endif

