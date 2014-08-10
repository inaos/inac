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
#include <libinac/lib.h>
#include "config.h"

/*
 * Net callback to open an ISCP channel.
 */
static ina_rc_t __ina_net_open_cb(void*, int);

/*
 * Net callback to close an ISCP channel.
 */
static ina_rc_t __ina_net_clse_cb(void*, int);

/*
 * Net callback to send an ISCP command.
 */
static ina_rc_t __ina_net_send_cb(void*, ina_iscp_msg_t*);

/*
 * Net callback to receive an ISCP command.
 */
static ina_rc_t __ina_net_recv_cb(void*, ina_iscp_msg_t*);

/*
 * Net callback to return an ISCP command result.
 */
static ina_rc_t __ina_net_retn_cb(void*, ina_iscp_msg_t*);


INA_API(ina_rc_t) ina_iscp_create(ina_iscp_ctx_t **ctx, ina_iscp_backend_t backend)
{
    ina_rc_t rc = INA_SUCCESS;

    *ctx = (ina_iscp_ctx_t*)ina_mem_alloc(sizeof(ina_iscp_ctx_t));
    if (*ctx == NULL) {
        return INA_ERR_PUSH_LAST;
    }
    (*ctx)->backend = backend;
    
    switch ((*ctx)->backend) {
        case INA_ISCP_NONE:
            /* Callsbacks will be provided externally */
            break;
        case INA_ISCP_INET: {
            rc = ina_iscp_set_callbacks(*ctx, __ina_net_open_cb,
                                             __ina_net_clse_cb,
                                             __ina_net_send_cb,
                                             __ina_net_recv_cb,
                                             __ina_net_retn_cb);
            break;
        };
        default: 
        {
            break;
        }
    }
    
    if (!INA_SUCCEED(rc)) {
        return INA_ERR_PUSH_LAST;
    }

    if (!INA_SUCCEED(ina_mempool_create(&(*ctx)->mempool, 
            2*1024*1024, 
            INA_MEM_DYNAMIC, 
            NULL))) {
        ina_mem_free(*ctx);
        *ctx = NULL;
        return INA_ERR_PUSH_LAST;
    }
    
    /* Create timer  */
    if (!INA_SUCCEED(ina_timer_init(&(*ctx)->timer))) {
        return INA_ERR_PUSH_LAST;
    }

    /* Create a time event for ISCP accept */
    (*ctx)->time_event = ina_timer_create_event((*ctx)->timer, 300);
    if ((*ctx)->time_event == NULL) {
        return INA_ERR_PUSH_LAST;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_iscp_create_tcp(ina_iscp_ctx_t **ctx, const char* addr, int port)
{
    ina_iscp_tcp_data_t *data = NULL;

    if (!INA_SUCCEED(ina_iscp_create(ctx, INA_ISCP_INET))) {
        return INA_ERR_PUSH_LAST;
    }
    
    data = (ina_iscp_tcp_data_t*)ina_mempool_dalloc((*ctx)->mempool, sizeof(ina_iscp_tcp_data_t));
    if (data == NULL) {
        return INA_ERR_PUSH_LAST;
    }

    data->addr = ina_str_pfromcstr(addr, (*ctx)->mempool);
    data->port = port;
    data->fd   = -1;
    data->lfd  = -1;
    data->timeout_sec = INA_ISCP_NET_TIMEOUT;
    (*ctx)->user_data = data;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_iscp_set_callbacks(ina_iscp_ctx_t *ctx,
                        ina_iscp_open_cb open_cb, ina_iscp_clse_cb clse_cb,
                        ina_iscp_send_cb send_cb, ina_iscp_recv_cb recv_cb, 
                        ina_iscp_retn_cb retn_cb)
{
    INA_ASSERT_NOTNULL(ctx);
    ctx->open_cb = open_cb;
    if (ctx->open_cb == NULL) {
        return INA_ISCP_EOPENCB;
    }
    ctx->clse_cb = clse_cb;
    if (ctx->clse_cb == NULL) {
        return INA_ISCP_ECLSECB;
    }
    
    ctx->recv_cb = recv_cb;
    if (ctx->recv_cb == NULL) {
        return INA_ISCP_ESENDCB;
    }
    ctx->send_cb = send_cb;
    if (ctx->send_cb == NULL) {
        return INA_ISCP_ERECVCB;
    }
    ctx->retn_cb = retn_cb;
    if (ctx->retn_cb == NULL) {
        return INA_ISCP_ERETNCB;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_iscp_destroy(ina_iscp_ctx_t **ctx) 
{
    if (*ctx == NULL) {
        return INA_SUCCESS;
    }
    
    HASH_CLEAR(hh, (*ctx)->cmds);
    INA_ASSERT_NULL((*ctx)->cmds);

    (*ctx)->clse_cb((*ctx)->user_data, 1);
    (*ctx)->clse_cb((*ctx)->user_data, 0);    

    if (!INA_SUCCEED(ina_mempool_release((*ctx)->mempool, 0))) {
        return INA_ERR_PUSH_LAST;
    }
    ina_timer_delete_event((*ctx)->timer, (*ctx)->time_event);
    ina_timer_destroy(&(*ctx)->timer);
    ina_mem_free(*ctx);
    *ctx = NULL;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_iscp_register(ina_iscp_ctx_t *ctx, int cmd_id, int p_count, int r_count, ina_iscp_handler_t handler)
{
    ina_iscp_cmd_t *cmd;

    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT(cmd_id > 0);
    INA_ASSERT(p_count >= 0);
    INA_ASSERT(r_count >= 0);
 
    HASH_FIND_INT(ctx->cmds, &cmd_id, cmd);
    if (cmd != NULL) {
        if (cmd->cmd_id == cmd_id &&
            cmd->p_count == p_count &&
            cmd->r_count == r_count)  {
                cmd->handler = handler;
                return INA_SUCCESS;
        }
        return INA_ISCP_ECMDREG;
    }

    cmd = (ina_iscp_cmd_t*)ina_mempool_dalloc(ctx->mempool, sizeof(ina_iscp_cmd_t));
    if (cmd == NULL) {
        return INA_ERR_PUSH_LAST;
    }

    cmd->cmd_id = cmd_id;
    cmd->p_count = p_count;
    cmd->r_count = r_count;
    cmd->handler = handler;

    HASH_ADD_INT(ctx->cmds, cmd_id, cmd);

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_iscp_register_ex(ina_iscp_ctx_t *ctx, ina_iscp_cmd_t *cmds)
{
    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_NOTNULL(cmds);

    while (cmds->cmd_id >0) {
        INA_TRACE3("Register cmd with ID %d", cmds->cmd_id);
        if (!INA_SUCCEED(ina_iscp_register(ctx, cmds->cmd_id, cmds->p_count, cmds->r_count, cmds->handler))) {
            return INA_ERR_PUSH_LAST;
        }
        ++cmds;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_iscp_send(ina_iscp_ctx_t *ctx, int cmd_id, ...)
{
    ina_iscp_cmd_t* cmd;
    ina_iscp_msg_t  msg;  
    size_t n;
    size_t p;
    uint8_t type;
    uint32_t crc;
    va_list params;

    INA_ASSERT_NOTNULL(ctx);

    cmd = NULL;
    HASH_FIND_INT(ctx->cmds, &cmd_id, cmd);
    if (cmd == NULL) {
        return INA_ISCP_ECMDREG;
    }

    n = 0;
    p = cmd->p_count;
    type = -1;

    va_start(params, cmd_id);

    while (p--) {
        type = (uint8_t)va_arg(params, int);

        INA_TRACE3("msg offset=%ld", n + INA_ISCP_HDR_SIZE);
        INA_TRACE3("msg.type=%d  ", type);

        msg.cmd_data[n] = type;
        n += sizeof(uint8_t);
        switch (type) {
            case INA_ISCP_TYPE_INT64:
            {
                int64_t i;
                i = va_arg(params, int64_t);
                msg.cmd_data[n] = i & 0xff;
                msg.cmd_data[++n] = (i>>8)  & 0xff;
                msg.cmd_data[++n] = (i>>16) & 0xff;
                msg.cmd_data[++n] = (i>>24) & 0xff;
                msg.cmd_data[++n] = (i>>32) & 0xff;
                msg.cmd_data[++n] = (i>>40) & 0xff;
                msg.cmd_data[++n] = (i>>48) & 0xff;
                msg.cmd_data[++n] = (i>>56) & 0xff;
                ++n;
                break;
            }
            case INA_ISCP_TYPE_DBL:
            {
                /* FIXME: find a better solution */
                double d;
                d = va_arg(params, double);
                ina_mem_cpy(&msg.cmd_data[n], &d, sizeof(double));
                n += sizeof(double);
                break;
            }
            case INA_ISCP_TYPE_STR:
            {
                const char* str;
                int32_t i;

                str = va_arg(params, char*);
                i = strlen(str);
                msg.cmd_data[n] = i & 0xff;
                msg.cmd_data[++n] = (i>>8)  & 0xff;
                msg.cmd_data[++n] = (i>>16) & 0xff;
                msg.cmd_data[++n] = (i>>24) & 0xff;
                ina_mem_cpy(&msg.cmd_data[++n], str, i);
                n += i;
                msg.cmd_data[n] = 0;
                ++n;
                break;
            }
            default:
            {
                return INA_ISCP_ETYPE;
            }
        }
    }
    va_end(params);

    /* Fill up header fields */
    msg.length = n + INA_ISCP_HDR_SIZE + sizeof(uint32_t);
    msg.cmd_id = cmd->cmd_id;
    msg.cmd_uid = 1; /* FIMXE: UID Generator */
    msg.p_count = cmd->p_count;
    msg.r_count = cmd->r_count;

    /* Calculate CRC and append it to the message */
    INA_TRACE3("crc pos %ld", msg.length-sizeof(uint32_t));
    crc = ina_util_hash_crc32(0, &msg, msg.length-sizeof(uint32_t));
    INA_TRACE3("crc=%u crc-length=%ld", crc,  msg.length-sizeof(uint32_t));
    msg.cmd_data[n] = crc & 0xff;
    msg.cmd_data[++n] = (crc>>8)  & 0xff;
    msg.cmd_data[++n] = (crc>>16) & 0xff;
    msg.cmd_data[++n] = (crc>>24) & 0xff;

    INA_TRACE2("Message sending with id %d", msg.cmd_id);
    INA_TRACE3("- msg.cmd_id->%d", msg.cmd_id);
    INA_TRACE3("- msg.length->%d", msg.length);
    INA_TRACE3("- msg.cmd_uid->%d", msg.cmd_uid);
    INA_TRACE3("- msg.p_count->%d", msg.p_count);
    INA_TRACE3("- msg.r_count->%d", msg.r_count);
    
    if (INA_SUCCEED(ctx->open_cb(ctx->user_data, 1))) {
        ina_rc_t rc = ctx->send_cb(ctx->user_data, &msg);
        ina_mem_cpy(&ctx->last_response, &msg, sizeof(ina_iscp_msg_t));
        ctx->clse_cb(ctx->user_data, 1);
        if (rc != INA_SUCCESS) {
            return INA_ISCP_ERROR(INA_RC_REASON(rc), "ISCP command failed");
        }
        return INA_SUCCESS;
    }
    return INA_ERR_PUSH_LAST;
}

INA_API(ina_rc_t) ina_iscp_recv(ina_iscp_ctx_t *ctx, int nc, int wait_msec) 
{
    ina_iscp_msg_t msg;
    ina_rc_t rc;

    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_TRUE(nc > 0);
    INA_ASSERT_TRUE(wait_msec >= 0);
    
    rc = INA_SUCCESS;

    /* Open ISCP channel */
    if (!INA_SUCCEED(ctx->open_cb(ctx->user_data, 0))) {
        return INA_ERR_PUSH_LAST;
    }

    while (nc--) {
        rc = ctx->recv_cb(ctx->user_data, &msg);
        if (INA_SUCCEED(rc)) {
            size_t n;
            int p;
            int idx;
            ina_iscp_cmd_t *cmd;
            ina_iscp_param_t *params;
            ina_iscp_param_t **retvals;
            int ci;
            uint32_t crc;
 
            INA_TRACE2("Message received with cmd_id %d", msg.cmd_id);
            INA_TRACE3("- msg.cmd_id->%d", msg.cmd_id);
            INA_TRACE3("- msg.length->%d", msg.length);
            INA_TRACE3("- msg.cmd_uid->%d", msg.cmd_uid);
            INA_TRACE3("- msg.p_count->%d", msg.p_count);
            INA_TRACE3("- msg.r_count->%d", msg.r_count);

            /* We need exactlly an int */
            ci = msg.cmd_id;

            HASH_FIND_INT(ctx->cmds, &ci, cmd);
            if (cmd == NULL || cmd->handler == NULL) {
                INA_TRACE("Command discard with id %d", msg.cmd_id);
                msg.rc = INA_ISCP_ECMDREG;
                msg.r_count = 0;
                ctx->retn_cb(ctx->user_data, &msg);
                return ina_err_peek();
            }

            /* Validate CRC */
            INA_TRACE3("crc pos=%ld", msg.length-sizeof(uint32_t));
            crc = *(uint32_t*)&((unsigned char*)(&msg))[msg.length-sizeof(uint32_t)];
            INA_TRACE3("crc=%u crc-length=%ld", crc, msg.length-sizeof(uint32_t));

            /*if (crc != ina_util_crc32(0, (unsigned char*)msg, msg->length-sizeof(uint32_t))) {
                INA_TRACE("Invalid crc (%d)", crc);
                return INA_FAILURE;
            }*/

            p = 0;
            n = 0;
            params = (ina_iscp_param_t*)ina_mempool_dalloc(
                                            ctx->mempool,
                                            sizeof(ina_iscp_param_t)*(msg.p_count));
      
            while (n+INA_ISCP_HDR_SIZE < msg.length-sizeof(uint32_t)) {
                params[p].type = *(uint8_t*)&msg.cmd_data[n];
                INA_TRACE3("msg.type->%d", params[p].type);

                n+= sizeof(uint8_t);
                switch (params[p].type) {
                    case INA_ISCP_TYPE_INT64:
                    {
                        params[p].value.i = *(int64_t*)&msg.cmd_data[n];
                        n += sizeof(int64_t);
                        INA_TRACE3("- Parameter %d type=int64_t value=%lld", p, params[p].value.i);
                        break;
                    }
                    case INA_ISCP_TYPE_DBL:
                    {
                        params[p].value.d = *(double*)&msg.cmd_data[n];
                        n += sizeof(double);
                        INA_TRACE3("- Parameter %d type=double value=%f", p, params[p].value.d);
                        break;
                    }
                    case INA_ISCP_TYPE_STR:
                    {
                        int32_t i;
                        i = *(int32_t*)&msg.cmd_data[n];
                        n += sizeof(int32_t);
                        params[p].value.s = ina_str_fromcstr((const char*)&msg.cmd_data[n]);
                        INA_TRACE3("- Parameter %d type=string value=%s", p, params[p].value.s);
                        INA_TRACE3("   - string length=%d", i);
                        n += i+1;
                        break;
                    }
                    default:  {
                        INA_TRACE_MSG("Invalid type!");
                        msg.rc = INA_FAILURE;
                        msg.r_count = 0;
                        ctx->retn_cb(ctx, &msg);
                        return INA_ISCP_ETYPE;
                    }
                }
                ++p;
            }

            if (p != msg.p_count) {
                return INA_ISCP_ECMDREG;
            }
            
            /* Store RC from command handler */
            INA_TRACE3("Call command handler for cmd_id %d", msg.cmd_id);
            msg.rc = cmd->handler(msg.cmd_id, msg.p_count, params, msg.r_count, &retvals);
            
            /* We return RC back to the callee */
            p = 0;
            n = 0;
            idx = 0;
            if (msg.r_count > 0 && INA_SUCCEED(msg.rc)) {
                while (p < msg.r_count) {
                    INA_TRACE3("response msg offset=%ld", n + INA_ISCP_HDR_SIZE);
                    INA_TRACE3("response msg.type=%d ", param->type);
                    msg.cmd_data[n] = retvals[idx]->type;
                    n += sizeof(uint8_t);
                    switch (retvals[idx]->type) {
                        case INA_ISCP_TYPE_INT64:
                        {
                            int64_t i;
                            i = retvals[idx]->value.i;
                            INA_TRACE3("response msg.value.i offset=%ld, value=%ld ",n, i);
                            msg.cmd_data[n] = i & 0xff;
                            msg.cmd_data[++n] = (i>>8)  & 0xff;
                            msg.cmd_data[++n] = (i>>16) & 0xff;
                            msg.cmd_data[++n] = (i>>24) & 0xff;
                            msg.cmd_data[++n] = (i>>32) & 0xff;
                            msg.cmd_data[++n] = (i>>40) & 0xff;
                            msg.cmd_data[++n] = (i>>48) & 0xff;
                            msg.cmd_data[++n] = (i>>56) & 0xff;
                            ++n;
                            break;
                        }
                        case INA_ISCP_TYPE_DBL:
                        {
                            /* FIXME: find a better solution */
                            double d;
                            d = retvals[idx]->value.d;
                            ina_mem_cpy(&msg.cmd_data[n], &d, sizeof(double));
                            INA_TRACE3("response msg.value.d offset=%ld, value=%f ",n, d);
                            n += sizeof(double);
                            break;
                        }
                        case INA_ISCP_TYPE_STR:
                        {
                            const char* str;
                            int32_t i;

                            str = ina_str_cstr(retvals[idx]->value.s);
                            i = strlen(str);
                            msg.cmd_data[n] = i & 0xff;
                            msg.cmd_data[++n] = (i>>8)  & 0xff;
                            msg.cmd_data[++n] = (i>>16) & 0xff;
                            msg.cmd_data[++n] = (i>>24) & 0xff;
                            ina_mem_cpy(&msg.cmd_data[++n], str, i);
                            n += i;
                            msg.cmd_data[n] = 0;
                            ++n;
                            break;
                        }
                        default:
                            return INA_ISCP_ETYPE;
                    }
                    ++p;
                    ++idx;
                    params++;
                }            
            }
            msg.length = n + INA_ISCP_HDR_SIZE + sizeof(uint32_t);
            return ctx->retn_cb(ctx->user_data, &msg);
        }
        if (rc != INA_EWAIT) {
            return ina_err_peek();
        }
        
        if (nc && wait_msec > 0) {
            INA_TRACE3("Next ISCP in %d msec", wait_msec);
            ina_time_sleep(wait_msec);
        }
    }
    return rc;
}

INA_API(ina_rc_t) ina_iscp_set_return_values(ina_iscp_param_t ***values_ptr, int count, ...){
    int c;
    ina_iscp_param_t **values;
    va_list params;
    INA_ASSERT(count >= 0);

    INA_TRACE3("ISCP setting %d return values...", count);
    
    if (count == 0) {
        return INA_SUCCESS;
    }

    *values_ptr = (ina_iscp_param_t**)ina_mem_alloc(sizeof(ina_iscp_param_t*)*count);
    if (*values_ptr == NULL) {
        return INA_ERR_PUSH_LAST;
    }
    c = 0;

    values = *values_ptr;
    va_start(params, count);
    while (c++ < count) {
        int i = c - 1;
        values[i] = (ina_iscp_param_t*)ina_mem_alloc(sizeof(ina_iscp_param_t));
        values[i]->type = (uint8_t)va_arg(params, int);
        switch (values[i]->type) {
            case INA_ISCP_TYPE_INT64:
                values[i]->value.i = va_arg(params, int64_t);
                INA_TRACE3("%d: int64_t %ld", c, param->value.i);
                break;
            case INA_ISCP_TYPE_DBL:
                values[i]->value.d = va_arg(params, double);
                INA_TRACE3("%d: double %f", c, param->value.d);
                break;
            case INA_ISCP_TYPE_STR:
                values[i]->value.s = ina_str_fromcstr(va_arg(params, char*));
                INA_TRACE3("%d: string %s",c, ina_str_cstr(param->value.s));
                break;
            default:
                INA_TRACE("%d: invalid type! %d", c, values[i]->type);
                return INA_ISCP_ETYPE;
        }
    }
    va_end(params);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_iscp_get_last_return_values(const ina_iscp_ctx_t *ctx, ...)
{

    int p = 0;
    int n = 0;
    int type = -1;
    int rtype = -1;
    va_list params;
    

    INA_ASSERT_NOTNULL(ctx);
    if (!INA_SUCCEED(ctx->last_response.rc)) {
        return ctx->last_response.rc;
    }

    va_start(params, ctx);
     
    while (n+INA_ISCP_HDR_SIZE < ctx->last_response.length-sizeof(uint32_t)) {
        type = *(uint8_t*)&ctx->last_response.cmd_data[n];
        INA_TRACE3("last_response.type->%d", type);
        rtype = (uint8_t)va_arg(params, int);
        n+= sizeof(uint8_t);

        if (type != rtype) {
            INA_TRACE3("Type not matching (requested %d found: %d)", type, rtype);
        }

        switch (type) {
            case INA_ISCP_TYPE_INT64:
                *va_arg(params, int64_t *) = *(int64_t*)&ctx->last_response.cmd_data[n];
                INA_TRACE3("- Return value %d type=int64_t value=%lld", p, *(int64_t*)&ctx->last_response.cmd_data[n]);
                n += sizeof(int64_t);
                break;
            case INA_ISCP_TYPE_DBL:
                *va_arg(params, double *)= *(double*)&ctx->last_response.cmd_data[n];
                INA_TRACE3("- Return value %d type=double value=%f", p, *(double*)&ctx->last_response.cmd_data[n]);
                n += sizeof(double);
                break;
            case INA_ISCP_TYPE_STR: {
                int32_t i;
                i = *(int32_t*)&ctx->last_response.cmd_data[n];
                n += sizeof(int32_t);
                *va_arg(params, ina_str_t*)= ina_str_fromcstr((const char*)&ctx->last_response.cmd_data[n]);
                INA_TRACE3("- Return %d type=string value=%s", p, (const char*)&ctx->last_response.cmd_data[n]);
                INA_TRACE3("   - string length=%d", i);
                n += i+1;
                break;
            }
            default: 
                INA_TRACE("%d: invalid type! %d", p, type);;
                return INA_ISCP_ETYPE;
        }
        ++p;
    }
    va_end(params);

    if (p != ctx->last_response.r_count) {
        return INA_ISCP_ECMDREG;
    }
 
    return INA_SUCCESS;
}

static ina_rc_t 
__ina_net_open_cb(void* user_data, int send)
{
    ina_iscp_tcp_data_t *data = (ina_iscp_tcp_data_t*)user_data;

    /* Open channel for sending **/
    if (send == 1) {    
        INA_TRACE3("ISCP channel for send");
        /* Check if the channel is sill open */
        if (data->fd == -1) {
             INA_TRACE3("Open ISCP channel for send");
            if (!INA_SUCCEED(ina_net_tcp_connect(&data->fd, ina_str_cstr(data->addr), data->port, 0))) {
                return ina_err_peek();
            }
            INA_TRACE3("Open ISCP channel ready to send");
        }
        /*ina_net_set_read_timeout(data->fd, 100);
        ina_net_set_write_timeout(data->fd, 100);*/
        return INA_SUCCESS;
    }

    if (data->lfd  == -1) {
        INA_TRACE3("Open ISCP channel for receive port %d, address %s", 
            data->port,
            ina_str_cstr(data->addr));
        
        /* Open chnannel for receiving */
        if (!INA_SUCCEED(ina_net_tcp_server(&data->lfd, data->port, ina_str_cstr(data->addr)))) {
            return ina_err_peek();
        }
  
        if (!INA_SUCCEED(ina_net_nonblock(data->lfd))) {
            ina_net_close(data->lfd);
            data->lfd = -1;
            data->fd = -1;
            return ina_err_peek();
        }
        /*ina_net_set_read_timeout(data->lfd, 100);
        ina_net_set_write_timeout(data->lfd, 100);*/
        INA_TRACE3("ISCP channel ready to receive");
    }
    return INA_SUCCESS;
}

static ina_rc_t 
__ina_net_clse_cb(void* user_data, int send)
{
    ina_iscp_tcp_data_t *data = (ina_iscp_tcp_data_t*)user_data;
    if (send == 1 && data->fd != -1) {
        INA_TRACE3("ISCP close client fd %d", data->fd);
        ina_net_close(data->fd);
        data->fd = -1;
    } else if (data->lfd != -1){
        INA_TRACE3("ISCP close server fd %d", data->fd);
        ina_net_close(data->lfd);
        data->lfd = -1;
    }
    return INA_SUCCESS;
}

static ina_rc_t
__ina_net_send_cb(void *user_data, ina_iscp_msg_t *msg)
{
     int nb_write;
     int nb_read;
     ina_iscp_tcp_data_t *data = (ina_iscp_tcp_data_t*)user_data;

     nb_write = 0;
     nb_read = 0;

     INA_TRACE3("ISCP send on fd %d", data->fd);

     if (INA_SUCCEED(ina_net_write(data->fd, (unsigned char*)msg, msg->length, &nb_write))) {
         unsigned char *buf;
         int tot_nb_read;

         INA_TRACE3("ISCP read response on fd %d", data->fd);

         buf = (unsigned char*)msg;

r1:
         if (INA_SUCCEED(ina_net_read(data->fd, buf, sizeof(ina_iscp_msg_t), &nb_read))) {
             tot_nb_read = nb_read;
             buf += nb_read;
             if (tot_nb_read < INA_ISCP_HDR_SIZE) {
                 goto r1;
             }

             if (tot_nb_read < msg->length) {
r2:
                 if (INA_SUCCEED(ina_net_read(data->fd, buf,sizeof(ina_iscp_msg_t)-tot_nb_read, &nb_read))) {
                     tot_nb_read += nb_read;
                     buf += nb_read;
                     if (tot_nb_read < msg->length) {
                         goto r2;
                     }
                     return INA_SUCCESS;
                 }
             } else {
                 return INA_SUCCESS;
             }
         }
     }
     return INA_ISCP_ESEND;
}

static ina_rc_t
__ina_net_recv_cb(void *user_data, ina_iscp_msg_t *msg)
{
     int nb_read;
     int tot_nb_read;
     ina_iscp_tcp_data_t *data = (ina_iscp_tcp_data_t*)user_data;
     unsigned char *buf;
     nb_read = 0;

     INA_TRACE3("Acpect ISCP on fd %d", data->lfd);

     if (INA_SUCCEED(ina_net_tcp_accept(&data->fd, data->lfd, NULL, NULL))) {
         if (data->fd != -1) {
             INA_TRACE3("Accepted ISCP fd %d", data->fd);
             /*if (!INA_SUCCEED(ina_net_nonblock(data->fd))) {
                 ina_net_close(data->fd);
                 data->fd = -1;
             }*/
             /*ina_net_set_read_timeout(data->fd, 100);
             ina_net_set_write_timeout(data->fd, 100);*/
         }
     }

     if (data->fd == -1) {
         INA_TRACE3("Return EWAIT for next ISCP on fd %d", data->lfd);
         return INA_EWAIT;
     }

     buf = (unsigned char*)msg;
r1:
     if (INA_SUCCEED(ina_net_read(data->fd, buf, sizeof(ina_iscp_msg_t), &nb_read))) {
         tot_nb_read = nb_read;
         buf += nb_read;
         if (tot_nb_read < INA_ISCP_HDR_SIZE) {
             goto r1;
         }

         if (tot_nb_read < msg->length) {
r2:
             if (INA_SUCCEED(ina_net_read(data->fd, buf,sizeof(ina_iscp_msg_t)-tot_nb_read, &nb_read))) {
                 tot_nb_read += nb_read;
                 buf += nb_read;
                 if (tot_nb_read < msg->length) {
                     goto r2;
                 }
                 return INA_SUCCESS;
             }
         } else {
             return INA_SUCCESS;
         }
     }
     return INA_ISCP_ERECV;
}


static ina_rc_t
__ina_net_retn_cb(void *user_data, ina_iscp_msg_t *msg)
{
    int nb_write;
    ina_iscp_tcp_data_t *data = (ina_iscp_tcp_data_t*)user_data;

    INA_ASSERT_NOTNULL(msg);
    nb_write = 0;
    return ina_net_write(data->fd, (unsigned char*)msg, msg->length, &nb_write);
}
