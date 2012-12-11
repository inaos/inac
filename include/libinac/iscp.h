/*
 * Copyright (c) 2012, INAOS GmbH
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

/*
 * INAOS Simple Command Protocol 
 */

#define INA_ISCP_BUFFER_SIZE  (2048)  /* Max size of command data */

#define INA_ISCP_TYPE_INT64  (0)  /* uint32_t  4 bytes*/
#define INA_ISCP_TYPE_STR    (1)  /* uint16_t (length) + char[lenght] */
#define INA_ISCP_TYPE_DBL    (2)  /* double 8 bytes */

/* ISCP context: Implementation specific 
 * data (socket descriptor for instance) */
typedef struct ina_iscp_cxt_s {
    void *data;   
} ina_iscp_ctx_t;

/* Send callback */
typedef ina_rc_t (*ina_iscp_send_cb)(ina_iscp_ctx_t*, size_t, const unsigned char*);
/* Receive callback */
typedef ina_rc_t (*ina_iscp_recv_cb)(ina_iscp_ctx_t*, size_t*, unsigned char*);
/* ISCP parameter */
typedef struct ina_iscp_param_s {
    uint8_t type;
    union {
        int64_t   n;
        double    f;
        ina_str_t s;
    } value;
} ina_iscp_param_t;

/* Command handler */
typedef ina_rc_t (*ina_iscp_handler)(int, int, ina_iscp_param_t*);

/* Internal send/receive buffer */
typedef struct ina_iscp_buf_s {
    uint16_t length;    /* store the command buffer size */
    uint32_t cmd_uid;   /* UID for sent commands */
    uint16_t cmd_id;    /* identify the command */
    /* Parameters 
     * [1 byte, parameter type][parameter]
     * 
     * [INA_ISCP_TYPE_INT][b1][b2][b3][b4]
     * [INA_ISPP_TYPE_STRING][string lenght b1][b2 str lenght][b1][b2][bxx...]
     * [INA_ISCP_TYPE_FLOAT][b1][b2][b3][b4][b5][b6][b7][b8]
     */
    unsigned char cmd_data[INA_ISCP_BUFFER_SIZE]; 
} ina_iscp_buf_t;

/*
 * Initialize internal structrues for ISCP
 *
 * Parameters
 * send_cb      Send callback function
 * recv_cb      Receive callback function
 *
 * Return Value
 * INA_SUCCESS if no error occurred
 */
INA_API(ina_rc_t) ina_iscp_init(ina_iscp_send_cb send_cb, ina_iscp_recv_cb recv_cb);

/*
 * Reset ISCP status an remove all regsitred commands.
 *
 * Return Value:
 * INA_SUCCESS if successfully cleared.
 */
INA_API(ina_rc_t) ina_iscp_reset(void);

/*
 * Register  command definition. Only used on "server" side.
 *
 * Parameters
 * cmd_id       Command identifier
 * p_count      Number of commands to send or to receive
 * handler      Command handler function
 *
 * Return Value
 * INA_SUCCESS if no error occurred
 */
INA_API(ina_rc_t) ina_iscp_register(int cmd_id,int p_count, ina_iscp_handler handler);
/*
 * Send a command synchronously.
 * Like:
 * ina_iscp_send(fd, INAFX_ISCP_SUBSCRIBE, 
 *                   INA_ISCP_TYPE_INT, 1,
 *                   INA_ISCP_TYPE_STR, "127.0.0.1"
 *                   INA_ISCP_TYPE_FLOAT, 2.3,
 *                   INA_ISCP_TYPE_INT, 3);
 *
 * Parameters
 * fd       socket descriptor
 * cmd_id   Commmand identifier
 * ...      Command parameter list
 *
 * Return Value
 * INA SUCCESS if command was successfully sent to the server and 
 *             executed by the receiver w/o error.
 */
INA_API(ina_rc_t) ina_iscp_send(ina_iscp_ctx_t* ctx, int cmd_id, ...);

/*
 * Check and receive a previously regsisterd command. If a
 * command was received the command handler will be invoked
 * synchronously.
 *
 * Parameters
 * fd       Socket descriptior
 * nc       Max number of command to accept.
 * timeout  Number of milliseconds to wait for a command
 *
 * Return Value:
 * INA_SUCCESS if no error occurred.
 */
INA_API(ina_rc_t) ina_iscp_recv(ina_iscp_ctx_t* ctx, int nc, int timeout);

#endif

