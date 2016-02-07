/*
 * Copyright (c) 2012-2016, INAOS GmbH
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


#ifndef INA_OS_WIN32
#define __INA_LPATH "./?.lua;"
#else
#define __INA_LPATH ".\\?.lua;"
#endif

/* Import LuaJIT modules */
INA_LJIT_PACKAGE(luatest);
INA_LJIT_IMPORT(luatest,luamock);
INA_LJIT_IMPORT(luatest,luaspec);

INA_LJIT_PACKAGE(inac);
INA_LJIT_IMPORT(inac,lconffile);
INA_LJIT_IMPORT(inac,ltemplate);
INA_LJIT_IMPORT(inac,lsocket);
INA_LJIT_IMPORT(inac,ldebug);
INA_LJIT_IMPORT(inac,lcsv);
INA_LJIT_IMPORT(inac,ldate);
INA_LJIT_IMPORT(inac,ltest);

INA_LJIT_PACKAGE(ljit);
INA_LJIT_IMPORT(ljit, bc);
INA_LJIT_IMPORT(ljit, bcsave);
INA_LJIT_IMPORT(ljit, dis_x64);
INA_LJIT_IMPORT(ljit, dis_x86);
INA_LJIT_IMPORT(ljit, v);
INA_LJIT_IMPORT(ljit, vmdef);
INA_LJIT_IMPORT(ljit, dump);

INA_API(ina_rc_t) ina_ljit_init(ina_ljit_ctx_t **ctx)
{   
    ina_str_t cur_path = NULL;
    ina_str_t new_path = NULL;

    INA_ASSERT_NOTNULL(ctx);

    *ctx = (ina_ljit_ctx_t*)ina_mem_alloc(sizeof(ina_ljit_ctx_t));
    (*ctx)->lstate = luaL_newstate();
    if ((*ctx)->lstate == NULL) {
        ina_mem_free(*ctx);
        ctx = NULL;
        return INA_LJIT_ENSTATE;
    }
    luaL_openlibs((*ctx)->lstate);
    lua_getglobal((*ctx)->lstate, "package");
    lua_getfield((*ctx)->lstate, -1, "path");
    cur_path = ina_str_new_fromcstr(lua_tostring((*ctx)->lstate, -1));
    new_path = ina_str_new(ina_str_len(cur_path)+10);
    ina_str_catcstr(new_path, __INA_LPATH);
    ina_str_cat(new_path, cur_path);
    lua_pop((*ctx)->lstate, 1 );
    lua_pushstring((*ctx)->lstate, ina_str_cstr(new_path));
    lua_setfield((*ctx)->lstate, -2, "path");
    lua_pop((*ctx)->lstate, 1);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ljit_destroy(ina_ljit_ctx_t **ctx)
{
    INA_ASSERT_NOTNULL(ctx);

    if (*ctx == NULL) {
        return INA_SUCCESS;
    }
    INA_ASSERT_NOTNULL((*ctx)->lstate);
    lua_close((*ctx)->lstate);
    ina_mem_free(*ctx);
    *ctx = NULL;
    return INA_SUCCESS;
}

unsigned long ina_ljit_hash_sbdm(const char *str)
{ 
    INA_ASSERT_NOTNULL(str);
    return INA_HASH_CSTR_TO_SDBM(str);
}

void ina_ljit_dbl_to_decimal(double dbl, ina_decimal_t *dec)
{   
    INA_ASSERT_NOTNULL(dec);
    ina_dbl_to_decimal(dbl, dec);
}

double ina_ljit_dbl_from_decimal(const ina_decimal_t *dec)
{
    INA_ASSERT_NOTNULL(dec);
    return(ina_dbl_from_decimal(dec));
}

INA_API(ina_rc_t) ina_ljit_call(ina_ljit_ctx_t *ctx, const char* fname, const char *sig, ...)
{
    va_list vl;
    int narg;
    int nres;
    char *cfname;

    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_NOTNULL(ctx->lstate);
    INA_ASSERT_NOTNULL(fname);

    /* Global function or object method? */
    if (!(cfname = (char*)strchr(fname, '.'))) {
        /* get function */
        lua_getglobal(ctx->lstate, fname); 
    } else {    
        char *obj_name_c;
        ina_str_t obj_name = ina_str_new_fromcstr(fname);
        INA_ASSERT_NOTNULL(obj_name);
        obj_name_c = (char*)ina_str_cstr(obj_name);
        obj_name_c[cfname - fname] = '\0';
        lua_getglobal(ctx->lstate, obj_name_c);
        cfname++;
        lua_getfield(ctx->lstate, -1, cfname);
        ina_str_free(obj_name);
    }

    va_start(vl, sig);
    
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
            case 'c':
                lua_pushlightuserdata(ctx->lstate, va_arg(vl, void *));
                break;
            case '<':
                goto endwhile;
                break;
            default:
                return INA_LJIT_EPARAM;
         }
         narg++;
         luaL_checkstack(ctx->lstate, 1, "too many arguments");
    } endwhile:


    /* do the call */
    nres = strlen(sig);
    if (lua_pcall(ctx->lstate, narg, nres, 0) != 0) {
        return INA_LJIT_ELUA(ctx);
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
              if (lua_isstring(ctx->lstate, nres)) {
                  *va_arg(vl, const char **) = lua_tostring(ctx->lstate, nres);
              } else if (lua_type(ctx->lstate, nres) == 10) { 
                  *va_arg(vl, const char **) = INA_LJIT_TOCSTRING(ctx, nres);
              } else {
                  return INA_LJIT_ERESULT;
              }
              break;
            case 'c': /* void pointer */
              if (lua_type(ctx->lstate, nres) == 10) { 
                  *va_arg(vl, const void **) = INA_LJIT_TOPOINTER(ctx, nres, const void*);
              } else {
                  return INA_LJIT_ERESULT;
              }
              break;            
            default:
              return INA_LJIT_EPARAM;
        }
        nres++;
    }
    va_end(vl);
    lua_pop(ctx->lstate, 1);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ljit_dostring(ina_ljit_ctx_t *ctx, const char *code)
{
    INA_ASSERT_NOTNULL(ctx);
    INA_ASSERT_NOTNULL(code);
    if (luaL_dostring(ctx->lstate, code) != 0) {
        return INA_LJIT_ELUA(ctx);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ljit_dump_stack(ina_ljit_ctx_t *ctx)
{
    int i;

    INA_ASSERT_NOTNULL(ctx);

    i = lua_gettop(ctx->lstate);
    fprintf(stdout, " \n----------------  Lua Stack Dump ----------------\n" );
    while(i) {
        int t = lua_type(ctx->lstate, i);
        switch (t) {
            case LUA_TSTRING:
                fprintf(stdout, "%d:`%s'\n", i, lua_tostring(ctx->lstate, i));
                break;
            case LUA_TBOOLEAN:
                  fprintf(stdout, "%d: %s\n",i,lua_toboolean(ctx->lstate, i) ? "true" : "false");
                  break;
            case LUA_TNUMBER:
                  fprintf(stdout, "%d: %g\n",  i, lua_tonumber(ctx->lstate, i));
                  break;
            default: 
                fprintf(stdout, "%d: %s\n", i, lua_typename(ctx->lstate, t)); 
                break;  
        }
        i--;
    }
    fprintf(stdout, "--------------- Lua Stack Dump Finished ---------------\n" );
    return INA_SUCCESS;
}

/*
 *
 */
INA_API(const void*) ina_ljit_checkcdata(ina_ljit_ctx_t *ctx, int narg)
{
    INA_ASSERT_NOTNULL(ctx);
    if (lua_type(ctx->lstate, narg) != 10) {
        luaL_typerror(ctx->lstate, narg, "cdata");
    }
    return lua_topointer(ctx->lstate, narg);
}
