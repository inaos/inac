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
#include <libinac/lib.h>
#include "config.h"

/* Internal registry entry */
typedef struct ina_ispc_cmd_s {
    int cmd_id;
    uint16_t p_count;
    ina_iscp_handler handler;
    UT_hash_handle hh;
} ina_iscp_cmd_t;

static ina_iscp_recv_cb __recv_cb = NULL;
static ina_iscp_send_cb __send_cb = NULL;
static ina_iscp_cmd_t  *__cmds = NULL;
static ina_mempool_t   *__mempool = NULL;

/*
 * Net callback to send an ISCP command.
 */
static ina_rc_t __ina_net_send_cb(ina_iscp_ctx_t*, ina_iscp_msg_t*);

/*
 * Net callback to receive an ISCP command.
 */
static ina_rc_t __ina_net_recv_cb(ina_iscp_ctx_t*, ina_iscp_msg_t*);


INA_API(ina_rc_t) ina_iscp_init(ina_iscp_backend_t backend)
{
    switch (backend) {
		case INA_ISCP_NONE:
			/* Callsbacks will be provided externally */
			break;
		case INA_ISCP_INET:
        {
            ina_iscp_set_callbacks(__ina_net_send_cb, __ina_net_recv_cb);
            break;
        }
        default: {
            return INA_FAILURE;
            break;
        }
    }

    if (__mempool == NULL) {
        return ina_mempool_create(&__mempool, 
                    2*1024*1024, 
                    INA_MEM_DYNAMIC|INA_MEM_FILLZERO, 
                    NULL);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_iscp_set_callbacks(ina_iscp_send_cb send_cb, ina_iscp_recv_cb recv_cb)
{
    __recv_cb = recv_cb;
    if (__recv_cb == NULL) {
        return INA_FAILURE;
    }
    __send_cb = send_cb;
    if (__send_cb == NULL) {
        return INA_FAILURE;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_iscp_reset(void) 
{
    __send_cb = NULL;
    __recv_cb = NULL;
    HASH_CLEAR(hh, __cmds);
    INA_ASSERT_NULL(__cmds);
    return ina_mempool_release(__mempool, 0);
}

INA_API(ina_rc_t) ina_iscp_register(int cmd_id, int p_count, ina_iscp_handler handler)
{
    ina_iscp_cmd_t *cmd;

    INA_ASSERT(cmd_id > 0);
    INA_ASSERT(p_count >= 0);
 
    if (__send_cb == NULL || __recv_cb == NULL) {
        /* TODO: sepfific error */
        return INA_FAILURE;
    }

    HASH_FIND_INT(__cmds, &cmd_id, cmd);
    if (cmd != NULL) {
        if (cmd->cmd_id == cmd_id &&
            cmd->p_count == p_count)  {
                cmd->handler = handler;
                return INA_SUCCESS;
        }
        return INA_FAILURE;
    }

    cmd = (ina_iscp_cmd_t*)ina_mempool_dalloc(__mempool, sizeof(ina_iscp_cmd_t));
    if (cmd == NULL) {
        return ina_err_peek();
    }

    cmd->cmd_id = cmd_id;
    cmd->p_count = p_count;
    cmd->handler = handler;

    HASH_ADD_INT(__cmds, cmd_id, cmd);

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_iscp_send(ina_iscp_ctx_t *ctx, int cmd_id, ...)
{
    ina_iscp_cmd_t* cmd;
    ina_iscp_msg_t* msg;
    size_t n;
    size_t p;
    uint8_t type;
    uint32_t crc;
    va_list params;

    INA_ASSERT_NOTNULL(ctx);

    cmd = NULL;
    HASH_FIND_INT(__cmds, &cmd_id, cmd);
    if (cmd == NULL) {
        /* TODO: Specific error */
        return INA_FAILURE;
    }

    /* Allocate buffer */
    msg = (ina_iscp_msg_t*)ina_mempool_dalloc(__mempool, sizeof(ina_iscp_msg_t));

    n = 0;
    p = cmd->p_count;
    type = -1;

    va_start(params, cmd_id);

    while (p--) {
        type = (uint8_t)va_arg(params, int);

        INA_TRACE3("msg offset=%ld", n + INA_ISCP_HDR_SIZE);
        INA_TRACE3("msg->type=%d  ", type);

        msg->cmd_data[n] = type;
        n += sizeof(uint8_t);
        switch (type) {
            case INA_ISCP_TYPE_INT64:
            {
                int64_t i;
                i = va_arg(params, int64_t);
                msg->cmd_data[n] = i & 0xff;
                msg->cmd_data[++n] = (i>>8)  & 0xff;
                msg->cmd_data[++n] = (i>>16) & 0xff;
                msg->cmd_data[++n] = (i>>24) & 0xff;
                msg->cmd_data[++n] = (i>>32) & 0xff;
                msg->cmd_data[++n] = (i>>40) & 0xff;
                msg->cmd_data[++n] = (i>>48) & 0xff;
                msg->cmd_data[++n] = (i>>56) & 0xff;
                ++n;
                break;
            }
            case INA_ISCP_TYPE_DBL:
            {
                /* FIXME: find a better solution */
                double d;
                d = va_arg(params, double);
                ina_mem_cpy(&msg->cmd_data[n], &d, sizeof(double));
                n += sizeof(double);
                break;
            }
            case INA_ISCP_TYPE_STR:
            {
                const char* str;
                int32_t i;

                str = va_arg(params, char*);
                i = strlen(str);
                msg->cmd_data[n] = i & 0xff;
                msg->cmd_data[++n] = (i>>8)  & 0xff;
                msg->cmd_data[++n] = (i>>16) & 0xff;
                msg->cmd_data[++n] = (i>>24) & 0xff;
                ina_mem_cpy(&msg->cmd_data[++n], str, i);
                n += i;
                msg->cmd_data[n] = 0;
                ++n;
                break;
            }
            default:
            {
                return INA_FAILURE;
            }
        }
    }
    va_end(params);
    /* Fill up header fields */
    msg->length = n + INA_ISCP_HDR_SIZE + sizeof(uint32_t);
    msg->cmd_id = cmd->cmd_id;
    msg->cmd_uid = 1; /* FIMXE: UID Generator */
    msg->p_count = cmd->p_count;

    /* Calculate CRC and append it to the message */
    INA_TRACE3("crc pos %ld", msg->length-sizeof(uint32_t));
    crc = ina_util_crc32(0, (unsigned char*)msg, msg->length-sizeof(uint32_t));
    INA_TRACE3("crc=%u crc-length=%ld", crc,  msg->length-sizeof(uint32_t));
    msg->cmd_data[n] = crc & 0xff;
    msg->cmd_data[++n] = (crc>>8)  & 0xff;
    msg->cmd_data[++n] = (crc>>16) & 0xff;
    msg->cmd_data[++n] = (crc>>24) & 0xff;

    INA_TRACE2("Message sending with id %d", msg->cmd_id);
    INA_TRACE3("- msg->cmd_id->%d", msg->cmd_id);
    INA_TRACE3("- msg->length->%d", msg->length);
    INA_TRACE3("- msg->cmd_uid->%d", msg->cmd_uid);
    INA_TRACE3("- msg->p_count->%d", msg->p_count);

    return __send_cb(ctx, msg);
}

INA_API(ina_rc_t) ina_iscp_recv(ina_iscp_ctx_t *ctx, int nc, int timeout) 
{
    ina_iscp_msg_t *msg;

    INA_ASSERT_NOTNULL(ctx);
    
    msg = (ina_iscp_msg_t*)ina_mempool_dalloc(__mempool, sizeof(ina_iscp_msg_t));
    
    /* TODO Timer event */
    if (INA_SUCCEED(__recv_cb(ctx, msg))) {
        size_t n;
        int p;
        ina_iscp_cmd_t *cmd;
        ina_iscp_param_t *params;
        int ci;
        uint32_t crc;
 
        INA_TRACE2("Message received with cmd_id %d", msg->cmd_id);
        INA_TRACE3("- msg->cmd_id->%d", msg->cmd_id);
        INA_TRACE3("- msg->length->%d", msg->length);
        INA_TRACE3("- msg->cmd_uid->%d", msg->cmd_uid);
        INA_TRACE3("- msg->p_count->%d", msg->p_count);

        /* We need exactlly an int */
        ci = msg->cmd_id;

        HASH_FIND_INT(__cmds, &ci, cmd);
        if (cmd == NULL) {
            INA_TRACE("Command discard with id %d", msg->cmd_id);
            /* TODO: sepcific error */
            return INA_FAILURE;
        }

        /* Validate CRC */
        INA_TRACE3("crc pos=%ld", msg->length-sizeof(uint32_t));
        crc = *(uint32_t*)&((unsigned char*)(msg))[msg->length-sizeof(uint32_t)];
        INA_TRACE3("crc=%u crc-length=%ld", crc, msg->length-sizeof(uint32_t));

        /*if (crc != ina_util_crc32(0, (unsigned char*)msg, msg->length-sizeof(uint32_t))) {
            INA_TRACE("Invalid crc (%d)", crc);
            return INA_FAILURE;
        }*/

        p = 0;
        n = 0;
        params = (ina_iscp_param_t*)ina_mempool_dalloc(
                                        __mempool,
                                        sizeof(ina_iscp_param_t)*(msg->p_count));

        while (n+INA_ISCP_HDR_SIZE < msg->length-sizeof(uint32_t)) {
            params[p].type = *(uint8_t*)&msg->cmd_data[n];
            INA_TRACE3("msg->type->%d", params[p].type);

            n+= sizeof(uint8_t);
            switch (params[p].type) {
                case INA_ISCP_TYPE_INT64:
                {
                    params[p].value.i = *(int64_t*)&msg->cmd_data[n];
                    n += sizeof(int64_t);
                    INA_TRACE3("- Parameter %d type=int64_t value=%lld", p, params[p].value.i);
                    break;
                }
                case INA_ISCP_TYPE_DBL:
                {
                    params[p].value.d = *(double*)&msg->cmd_data[n];
                    n += sizeof(double);
                    INA_TRACE3("- Parameter %d type=double value=%f", p, params[p].value.d);
                    break;
                }
                case INA_ISCP_TYPE_STR:
                {
                    int32_t i;
                    i = *(int32_t*)&msg->cmd_data[n];
                    n += sizeof(int32_t);
                    params[p].value.s = ina_str_fromcstr((const char*)&msg->cmd_data[n]);
                    INA_TRACE3("- Parameter %d type=string value=%s", p, params[p].value.s);
                    INA_TRACE3("   - string length=%d", i);
                    n += i+1;
                    break;
                }
                default:  {
                    INA_TRACE_MSG("Invalid type!");
                    /* TODO: specific error */
                    return INA_FAILURE;
                }
            }
            ++p;
        }
        if (p == msg->p_count) {
            if (cmd->handler != NULL) {
                return cmd->handler(msg->cmd_id, msg->p_count, params);
            }
        }
    }
    return INA_FAILURE;
}

static ina_rc_t
__ina_net_send_cb(ina_iscp_ctx_t *ctx, ina_iscp_msg_t *msg)
{
    int nb_write;
    nb_write = 0;
    return ina_net_write(*(int*)ctx->data, (unsigned char*)msg, msg->length, &nb_write);
}

static ina_rc_t
__ina_net_recv_cb(ina_iscp_ctx_t *ctx, ina_iscp_msg_t *msg)
{   
    int nb_read;
    nb_read = 0;
    if (INA_SUCCEED(ina_net_read(*(int*)ctx->data, (unsigned char*)msg, INA_ISCP_HDR_SIZE, &nb_read))) {
        if (nb_read > 0) {
            nb_read = msg->length;
            nb_read -= INA_ISCP_HDR_SIZE;
            if (nb_read > 0) {
                return ina_net_read(*(int*)ctx->data, &((unsigned char*)msg)[INA_ISCP_HDR_SIZE],nb_read, &nb_read);
            }
        }
    }
    return INA_FAILURE;
}