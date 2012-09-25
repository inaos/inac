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
 *     * Neither the name of the INAOS GmbH nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL INAOS GmbH BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <libinac/lib.h>
#include "../config.h"

#ifdef INA_STRING_CRT_ENABLED

INA_API(ina_str_t) ina_str_newlen(const void *anystr, size_t len, ina_mempool_t *pool)
{
    INA_FAIL_IF(len < 0);

    char* p = (char*)malloc(len+1);
    INA_FAIL_IF(p == NULL);

    if (anystr) {
        memcpy(p, anystr, len);
    } else {
        memset(p,0,len);
    }

    p[len] = '\0';
    ina_err_setlast(INA_SUCCESS);
    return p;
}

INA_API(ina_str_t) ina_str_new(ina_mempool_t *pool)
{
    return ina_str_newlen("", 0, pool);
}

INA_API(ina_rc_t) ina_str_free(ina_str_t s, ina_mempool_t *pool)
{
    free(s);
    return INA_RC_OK;
}

INA_API(ina_str_t) ina_str_dup(ina_str_t s, ina_mempool_t *pool)
{
    return ina_str_newlen(s, strlen(s), pool);
}

INA_API(const char*) inac_str_cstr(ina_str_t s)
{
    return strdup(s);
}

INA_API(ina_str_t) ina_str_cat(const ina_str_t s1, const ina_str_t s2, ina_mempool_t *pool)
{
    return NULL;
}

INA_API(ina_rc_t) ina_str_cmp(const ina_str_t s1, const ina_str_t s2)
{
    return INA_RC_OK;
}

INA_API(const ina_str_t) ina_str_strstr(const ina_str_t s1, const ina_str_t s2)
{
    return NULL;
}
INA_API(const ina_str_t) ina_str_strrch(const ina_str_t s1, const ina_str_t s2)
{
    return NULL;
}

#endif