/*
 * Copyright (c) 2013, INAOS GmbH
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

#include <contribs/axtls/ssl.h>

/* AX TLS SSL connection object */
struct ina_ssl_cn_s {
    int server;
    int blocking;
    SSL *cn_impl;
};

/* AX TLS context */
struct ina_ssl_ctx_s {
    uint32_t        options;
    SSL_CTX        *ctx_impl;
    ina_ssl_cn_t   *cn;
};

static const char* __ina_ssl_error_lookup(int);

INA_API(ina_rc_t) ina_ssl_init(ina_ssl_ctx_t **ctx,
                               int32_t num_sessions, 
                               uint32_t options)
{
    uint32_t ax_options = 0;

    *ctx = (ina_ssl_ctx_t*)ina_mem_alloc(sizeof(struct ina_ssl_ctx_s));
    if (*ctx == NULL) {
        return INA_ERR_PUSH_LAST;
    }

    (*ctx)->options = options;

    if ((*ctx)->options&INA_SSL_ACCEPT_SELF_SIGNED) {
        ax_options = SSL_SERVER_VERIFY_LATER;
    }
    (*ctx)->ctx_impl = ssl_ctx_new(ax_options, num_sessions);

    if ((*ctx)->ctx_impl == NULL) {
        return INA_SSL_EINIT;
    }
     
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ssl_destroy(ina_ssl_ctx_t **ctx)
{
    if (*ctx != NULL) {
        ssl_ctx_free((*ctx)->ctx_impl);
        ina_mem_free(*ctx);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ssl_client_new(ina_ssl_ctx_t *ctx, 
                                     ina_ssl_cn_t **cn, 
                                     int fd)
{
    INA_ASSERT_NOTNULL(ctx);

    if (ctx->cn != NULL) {
        return INA_SSL_EINIT;
    }

    ctx->cn = (ina_ssl_cn_t*)ina_mem_alloc(sizeof(struct ina_ssl_cn_s));
    if (ctx->cn == NULL) {
        return INA_ERR_PUSH_LAST;
    }

    if (ctx->options&INA_SSL_BLOCKING) {
        ctx->cn->blocking = INA_YES;        
    }

    ctx->cn->cn_impl = ssl_client_new(ctx->ctx_impl, fd, NULL, 0);
    if (ctx->cn->cn_impl == NULL) {
        return INA_SSL_EINIT;
    }
    ctx->cn->server = INA_NO;
    *cn = ctx->cn;

    if (ctx->options&INA_SSL_ACCEPT_SELF_SIGNED) {
        if (ina_ssl_handshake_status(*cn) == INA_SUCCESS) {
            if (ssl_verify_cert((*cn)->cn_impl) == 
                SSL_X509_ERROR(X509_VFY_ERROR_SELF_SIGNED)) {
                return INA_SUCCESS;
            }
        } 
    }
    return ina_ssl_handshake_status(*cn);
}

INA_API(ina_rc_t) ina_ssl_client_free(ina_ssl_ctx_t *ctx, ina_ssl_cn_t **cn)
{
    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_NOTNULL(ctx->cn);
    INA_ASSERT_FALSE((*cn)->server);

    ssl_free(ctx->cn->cn_impl);
    ina_mem_free(ctx->cn);
    ctx->cn = NULL;

    *cn = NULL;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ssl_server_new(ina_ssl_ctx_t *ctx, 
                                     ina_ssl_cn_t **cn, 
                                     int client_fd)
{
    INA_ASSERT_NULL(ctx->cn);
    ctx->cn = (ina_ssl_cn_t*)ina_mem_alloc(sizeof(struct ina_ssl_cn_s));
    if (ctx->cn)

    ctx->cn->cn_impl = ssl_server_new(ctx->ctx_impl, client_fd);
    if (ctx->cn->cn_impl == NULL) {
        return INA_SSL_EINIT;
    }
    ctx->cn->server = INA_YES;

    *cn = ctx->cn;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ssl_server_free(ina_ssl_ctx_t *ctx, ina_ssl_cn_t **cn)
{
    INA_ASSERT_NOTNULL(ctx->cn);
    INA_ASSERT_TRUE((*cn)->server);

    ssl_free(ctx->cn->cn_impl);
    ina_mem_free(ctx->cn);
    ctx->cn = NULL;

    *cn = NULL;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ssl_read(ina_ssl_cn_t *cn, 
                               unsigned char **buf, 
                               size_t *bytes_read)
{
    int ret;

    INA_ASSERT_NOTNULL(cn);

read:
    ret = ssl_read(cn->cn_impl, buf);
    if (ret == SSL_OK) {
        if (cn->blocking && ssl_handshake_status(cn->cn_impl) == SSL_OK) {
            goto read;
        }
        return INA_SSL_EAGAIN;
    }
    if (ret < 0) {
        return INA_SSL_ERROR(INA_EREAD, __ina_ssl_error_lookup(ret));    
    }
    *bytes_read = ret;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ssl_write(ina_ssl_cn_t *cn, 
                                const unsigned char *buf, 
                                size_t buf_len,
                                size_t *bytes_written)
{
    int ret;

    INA_ASSERT_NOTNULL(cn);

    ret = ssl_write(cn->cn_impl, buf, buf_len);
    if (ret < 0) {
        return INA_SSL_ERROR(INA_EREAD, __ina_ssl_error_lookup(ret));
    }
    *bytes_written = ret;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ssl_handshake(ina_ssl_cn_t *cn)
{
    INA_ASSERT_NOTNULL(cn);

    if (ssl_renegotiate(cn->cn_impl) != SSL_OK) {
        return INA_SSL_EAGAIN;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ssl_handshake_status(ina_ssl_cn_t *cn)
{
    INA_ASSERT_NOTNULL(cn);
    if (ssl_handshake_status(cn->cn_impl) != SSL_OK) {
        return INA_SSL_EAGAIN;
    }
    return INA_SUCCESS;
}


#define __INA_ERRMSG_DECL(id) static const char *errmsg_##id = #id
#define __INA_ERRMSG(id) errmsg_##id

static const char* 
__ina_ssl_error_lookup(int err)
{
    __INA_ERRMSG_DECL(SSL_ERROR_DEAD);
    __INA_ERRMSG_DECL(SSL_CLOSE_NOTIFY);
    __INA_ERRMSG_DECL(SSL_ERROR_CONN_LOST);
    __INA_ERRMSG_DECL(SSL_ERROR_SOCK_SETUP_FAILURE);
    __INA_ERRMSG_DECL(SSL_ERROR_INVALID_HANDSHAKE);
    __INA_ERRMSG_DECL(SSL_ERROR_INVALID_PROT_MSG);
    __INA_ERRMSG_DECL(SSL_ERROR_INVALID_HMAC);
    __INA_ERRMSG_DECL(SSL_ERROR_INVALID_VERSION);
    __INA_ERRMSG_DECL(SSL_ERROR_INVALID_SESSION);
    __INA_ERRMSG_DECL(SSL_ERROR_NO_CIPHER);
    __INA_ERRMSG_DECL(SSL_ERROR_BAD_CERTIFICATE);
    __INA_ERRMSG_DECL(SSL_ERROR_INVALID_KEY);
    __INA_ERRMSG_DECL(SSL_ERROR_FINISHED_INVALID);
    __INA_ERRMSG_DECL(SSL_ERROR_NO_CERT_DEFINED);
    __INA_ERRMSG_DECL(SSL_ERROR_NO_CLIENT_RENOG);
    __INA_ERRMSG_DECL(SSL_ERROR_NOT_SUPPORTED);
    __INA_ERRMSG_DECL(SSL_UNKNOWN_ERROR);
        
    switch (err) {
        case SSL_ERROR_DEAD:
            return __INA_ERRMSG(SSL_ERROR_DEAD);
        case SSL_CLOSE_NOTIFY:
            return __INA_ERRMSG(SSL_CLOSE_NOTIFY);
        case SSL_ERROR_CONN_LOST:
            return __INA_ERRMSG(SSL_ERROR_CONN_LOST);
        case SSL_ERROR_SOCK_SETUP_FAILURE:
            return __INA_ERRMSG(SSL_ERROR_SOCK_SETUP_FAILURE);
        case SSL_ERROR_INVALID_HANDSHAKE:
            return __INA_ERRMSG(SSL_ERROR_INVALID_HANDSHAKE);
        case SSL_ERROR_INVALID_PROT_MSG:
            return __INA_ERRMSG(SSL_ERROR_INVALID_PROT_MSG);
        case SSL_ERROR_INVALID_HMAC:
            return __INA_ERRMSG(SSL_ERROR_INVALID_HMAC);
        case SSL_ERROR_INVALID_VERSION:
            return __INA_ERRMSG(SSL_ERROR_INVALID_VERSION);
        case SSL_ERROR_INVALID_SESSION:
            return __INA_ERRMSG(SSL_ERROR_INVALID_SESSION);
        case SSL_ERROR_NO_CIPHER:
            return __INA_ERRMSG(SSL_ERROR_NO_CIPHER);
        case SSL_ERROR_BAD_CERTIFICATE:
            return __INA_ERRMSG(SSL_ERROR_BAD_CERTIFICATE);
        case SSL_ERROR_INVALID_KEY:
            return __INA_ERRMSG(SSL_ERROR_INVALID_KEY);
        case SSL_ERROR_FINISHED_INVALID:
            return __INA_ERRMSG(SSL_ERROR_FINISHED_INVALID);
        case SSL_ERROR_NO_CERT_DEFINED:
            return __INA_ERRMSG(SSL_ERROR_NO_CERT_DEFINED);
        case SSL_ERROR_NO_CLIENT_RENOG:
            return __INA_ERRMSG(SSL_ERROR_NO_CLIENT_RENOG);
        case SSL_ERROR_NOT_SUPPORTED:
            return __INA_ERRMSG(SSL_ERROR_NOT_SUPPORTED);
        default:
            return __INA_ERRMSG(SSL_UNKNOWN_ERROR);
    }
}
