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
#ifndef _LIBINAC_NET_HW_H__
#define _LIBINAC_NET_HW_H__

#include <libinac/lib.h>
#include "config.h"

/* functionn pointers */
typedef ina_rc_t (*ina_net_hw_enabled_fp)(ina_net_hw_ctx_t *ctx);
typedef ina_rc_t (*ina_net_hw_feature_check_fp)(ina_net_hw_ctx_t *ctx, ina_net_hw_feature_t feature);
typedef ina_rc_t (*ina_net_hw_accelerate_loopback_fp)(ina_net_hw_ctx_t *ctx, int fc, const char *alias);

typedef struct __ina_net_hw_func_s {
    ina_net_hw_enabled_fp enabled_fp;
    ina_net_hw_feature_check_fp feature_check_fp;
    ina_net_hw_accelerate_loopback_fp accelerate_loopback_fp;
} __ina_net_hw_func_t;

/* Solarflare Openonload */
INA_API(ina_rc_t) __ina_net_hw_onload_enabled(ina_net_hw_ctx_t *ctx);
INA_API(ina_rc_t) __ina_net_hw_onload_feature_check(ina_net_hw_ctx_t *ctx, ina_net_hw_feature_t feature);
INA_API(ina_rc_t) __ina_net_hw_onload_accelerate_loopback(ina_net_hw_ctx_t *ctx, int fd, const char *alias);
INA_API(ina_rc_t) INA_INLINE __ina_net_hw_onload_select(__ina_net_hw_func_t *f)
{
    f->enabled_fp = __ina_net_hw_onload_enabled;
    f->feature_check_fp = __ina_net_hw_onload_feature_check;
    f->accelerate_loopback_fp = __ina_net_hw_onload_accelerate_loopback;
    return INA_SUCCESS;
}

/* Mellanox VMA */
INA_API(ina_rc_t) __ina_net_hw_vma_enabled(ina_net_hw_ctx_t *ctx);
INA_API(ina_rc_t) __ina_net_hw_vma_feature_check(ina_net_hw_ctx_t *ctx, ina_net_hw_feature_t feature);
INA_API(ina_rc_t) __ina_net_hw_vma_accelerate_loopback(ina_net_hw_ctx_t *ctx, int fd, const char *alias);
INA_API(ina_rc_t) INA_INLINE __ina_net_hw_vma_select(__ina_net_hw_func_t *f)
{
    f->enabled_fp = __ina_net_hw_vma_enabled;
    f->feature_check_fp = __ina_net_hw_vma_feature_check;
    f->accelerate_loopback_fp = __ina_net_hw_vma_accelerate_loopback;
    return INA_SUCCESS;
}

#endif
