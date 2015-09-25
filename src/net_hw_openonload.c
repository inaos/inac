/*
 * Copyright (c) 2015, INAOS GmbH
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

#include "net_hw.h"

#ifdef INA_OS_LINUX

#include <onload_ext.h>

INA_API(ina_rc_t) __ina_net_hw_onload_enabled(ina_net_hw_ctx_t *ctx)
{
    if (onload_is_present()) {
        return INA_SUCCESS;
    }
    return INA_FAILURE; 
}

INA_API(ina_rc_t) __ina_net_hw_onload_feature_check(ina_net_hw_ctx_t *ctx, ina_net_hw_feature_t feature)
{
    int fd = 0;
    switch (feature) {
        case INA_NET_HW_FEATURE_ZERO_COPY_UDP_RECEIVE:
            return INA_SUCCESS;
        case INA_NET_HW_FEATURE_ZERO_COPY_UDP_SEND:
            return INA_SUCCESS;
        case INA_NET_HW_FEATURE_ZERO_COPY_TCP_RECEIVE:
            return INA_SUCCESS;
        case INA_NET_HW_FEATURE_ZERO_COPY_TCP_SEND:
            return INA_SUCCESS;
        case INA_NET_HW_FEATURE_MSG_WARM:
            if (ctx->data == NULL) {
                return INA_FAILURE;
            }
            fd = (int)ctx->data;
            if (onload_fd_check_feature(fd, ONLOAD_FD_FEAT_MSG_WARM) > 0) {
                return INA_SUCCESS;
            }
            else {
                return INA_FAILURE;
            }
        case INA_NET_HW_FEATURE_TPL_SEND:
            return INA_SUCCESS;
        case INA_NET_HW_FEATURE_LOOPBACK:
            return INA_SUCCESS;
        default:
            return INA_ENYI;
    }
    return INA_FAILURE;
}

INA_API(ina_rc_t) __ina_net_hw_onload_accelerate_loopback(ina_net_hw_ctx_t *ctx, int fd, const char *alias)
{
    int flag = 1;
    int rc = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, (void*)&flag, sizeof(flag));
    if (rc < 0) {
        return INA_FAILURE; 
    }
    rc = onload_set_stackname(ONLOAD_ALL_THREADS, ONLOAD_SCOPE_PROCESS, alias);
    if (rc != 0) {
        return INA_FAILURE;
    }
    return INA_SUCCESS;
}

#else
INA_API(ina_rc_t) __ina_net_hw_onload_enabled(ina_net_hw_ctx_t *ctx)
{
    return INA_ENYI;
}
INA_API(ina_rc_t) __ina_net_hw_onload_feature_check(ina_net_hw_ctx_t *ctx, ina_net_hw_feature_t feature)
{
    return INA_ENYI;
}
INA_API(ina_rc_t) __ina_net_hw_onload_accelerate_loopback(ina_net_hw_ctx_t *ctx, int fd, const char *alias)
{
    return INA_ENYI;
}
#endif
