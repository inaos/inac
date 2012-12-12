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


INA_API(ina_rc_t) ina_iscp_init(ina_iscp_send_cb send_cb, ina_iscp_recv_cb recv_cb)
{
    __send_cb = send_cb;
    if (__send_cb == NULL) {
        return INA_FAILURE;
    }
    __recv_cb = recv_cb;
    if (__recv_cb == NULL) {
        return INA_FAILURE;
    }
    if (__mempool == NULL) {
        return ina_mempool_create(&__mempool, 2*1024*1024, INA_MEM_DYNAMIC, NULL);
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
    ina_iscp_buf_t* buf;
    size_t n;
    size_t p;
    int type;
    va_list params;

    INA_ASSERT_NOTNULL(ctx);

    cmd = NULL;
    HASH_FIND_INT(__cmds, &cmd_id, cmd);
    if (cmd == NULL) {
        /* TODO: Specific error */
        return INA_FAILURE;
    }

    /* Allocate buffer */
    buf = (ina_iscp_buf_t*)ina_mempool_dalloc(__mempool, sizeof(ina_iscp_buf_t));

    type = -1;
    n = 0;
    p = cmd->p_count;

    va_start(params, cmd_id);

    while (p--) {
        type = va_arg(params, int);
        buf->cmd_data[n] = (short)type;
        n += sizeof(short);
        switch (type) {
            case INA_ISCP_TYPE_INT64:
            {
                int64_t i;
                i = va_arg(params, int64_t);
                buf->cmd_data[n++] = i & 0xff;
                buf->cmd_data[n++] = (i>>8)  & 0xff;
                buf->cmd_data[n++] = (i>>16) & 0xff;
                buf->cmd_data[n++] = (i>>24) & 0xff;
                buf->cmd_data[n++] = (i>>32) & 0xff;
                buf->cmd_data[n++] = (i>>40) & 0xff;
                buf->cmd_data[n++] = (i>>48) & 0xff;
                buf->cmd_data[n++] = (i>>56) & 0xff;
                break;
            }
            case INA_ISCP_TYPE_DBL:
            {
                /* FIXME: find a better solution */
                double d;
                d = va_arg(params, double);
                ina_mem_cpy(&buf->cmd_data[n++], &d, 8);
                break;
            }
            case INA_ISCP_TYPE_STR:
            {
                const char* str;
                int32_t i;

                str = va_arg(params, char*);                
                i = strlen(str);

                buf->cmd_data[n++] = i & 0xff;
                buf->cmd_data[n++] = (i>>8)  & 0xff;
                buf->cmd_data[n++] = (i>>16) & 0xff;
                buf->cmd_data[n++] = (i>>24) & 0xff;
    
                strcpy((char*)&buf->cmd_data[n], str);
                n += i;
                break;
            }
            default:
            {
                return INA_FAILURE;
            }
        }
    }
    va_end(params);

    buf->length = n + sizeof(uint16_t)*2+sizeof(uint32_t);
    buf->cmd_id = cmd->cmd_id;
    buf->cmd_uid = 1; /* TODO: Command UID generation */

    return __send_cb(ctx, buf->length, (const unsigned char*)buf);
}

INA_API(ina_rc_t) ina_iscp_recv(ina_iscp_ctx_t *ctx, int nc, int timeout) 
{
    size_t size;
    ina_iscp_buf_t *buf;
    
    INA_ASSERT_NOTNULL(ctx);
    
    buf = (ina_iscp_buf_t*)ina_mempool_dalloc(__mempool, sizeof(ina_iscp_buf_t));
    
    /* TODO Timer event */
    if (INA_SUCCEED(__recv_cb(ctx, &size, (unsigned char*)buf))) {
        size_t n;
        size_t p;
        ina_iscp_cmd_t *cmd;
        ina_iscp_param_t *params;

        HASH_FIND_INT(__cmds, &buf->cmd_id, cmd);
        if (cmd == NULL) {
            return INA_SUCCESS;
        }

        p = 0;
        n = sizeof(uint16_t)*2+sizeof(uint32_t);
        params = (ina_iscp_param_t*)ina_mempool_dalloc(
                                        __mempool,
                                        sizeof(ina_iscp_param_t)*(buf->p_count+1));

        while (n < buf->length) {
            params[p].type = (*(uint8_t*)(&buf->cmd_data[n]));
            switch (params[p].type) {
                case INA_ISCP_TYPE_INT64:
                {
                    params[p].value.i = (*(int64_t*)(&buf->cmd_data[n]));
                    n += sizeof(int64_t);
                    break;
                }
                case INA_ISCP_TYPE_DBL:
                {
                    params[p].value.d = (*(double*)(&buf->cmd_data[n]));
                    n += sizeof(double);
                    break;
                }
                case INA_ISCP_TYPE_STR:
                {
                    int32_t i;
                    i = (*(int32_t*)(&buf->cmd_data[n]));
                    n += sizeof(int32_t);
                    params[p].value.s = ina_str_fromcstr((const char*)&buf->cmd_data[n]);
                    break;
                }
                default: 
                    return INA_FAILURE;
            }
            ++p;
        }
        return cmd->handler(buf->cmd_id, p, params);
    }
    return INA_FAILURE;
}
