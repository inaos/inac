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

INA_API(ina_rc_t) ina_ljit_init(ina_ljit_ctx_t **ctx)
{
    *ctx = (ina_ljit_ctx_t*)ina_mem_alloc(sizeof(ina_ljit_ctx_t));
    if (*ctx == NULL) {
        return INA_ERR_PUSH_LAST;
    }
    (*ctx)->lstate = luaL_newstate();
    if ((*ctx)->lstate == NULL) {
        ina_mem_free(*ctx);
        ctx = NULL;
        return INA_LJIT_ENSTATE;
    }
    luaL_openlibs((*ctx)->lstate);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ljit_destroy(ina_ljit_ctx_t **ctx)
{
    if (*ctx == NULL) {
        return INA_SUCCESS;
    }
    INA_ASSERT_NOTNULL((*ctx)->lstate);
    lua_close((*ctx)->lstate);
    ina_mem_free(*ctx);
    *ctx = NULL;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ljit_call(ina_ljit_ctx_t *ctx, const char* fname, const char *sig, ...)
{
    va_list vl;
    int narg;
    int nres;;

    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_NOTNULL(ctx->lstate);

    va_start(vl, sig);
    lua_getglobal(ctx->lstate, fname);  /* get function */

    /* push arguments */
    narg = 0;
    while (*sig) {
        switch (*sig++) {
            case 'd':  
                lua_pushnumber(ctx->lstate, va_arg(vl, double));
                break;
            case 'i': 
                lua_pushnumber(ctx->lstate, va_arg(vl, int));
                break;
            case 's':
                lua_pushstring(ctx->lstate, va_arg(vl, char *));
                break;
            case '>':
              goto endwhile;
            default:
              return INA_LJIT_EPARAM;
         }
         narg++;
         luaL_checkstack(ctx->lstate, 1, "too many arguments");
    } endwhile:

    /* do the call */
    nres = strlen(sig);
    if (lua_pcall(ctx->lstate, narg, nres, 0) != 0) {
        return INA_LJIT_ECALL(lua_tostring(ctx->lstate, -1));
    }
    
    /* retrieve results */
    nres = -nres;  /* stack index of first result */
    while (*sig) {
        switch (*sig++) {
            case 'd':  /* double result */
              if (!lua_isnumber(ctx->lstate, nres)) {
                  return INA_LJIT_ERESULT;
              }
              *va_arg(vl, double *) = lua_tonumber(ctx->lstate, nres);
              break;
            case 'i':  /* int result */
              if (!lua_isnumber(ctx->lstate, nres)) {
                  return INA_LJIT_ERESULT;
              }
              *va_arg(vl, int *) = (int)lua_tonumber(ctx->lstate, nres);
              break;

            case 's':  /* string result */
              if (!lua_isstring(ctx->lstate, nres)) {
                  return INA_LJIT_ERESULT;
              }
              *va_arg(vl, const char **) = lua_tostring(ctx->lstate, nres);
              break;
            default:
              return INA_LJIT_EPARAM;
        }
        nres++;
    }
    va_end(vl);
    return INA_SUCCESS;
}
