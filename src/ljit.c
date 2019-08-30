/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>
#include "config.h"


#ifndef INA_OS_WINDOWS
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
INA_LJIT_IMPORT(inac,lsocket);
INA_LJIT_IMPORT(inac,ldebug);
INA_LJIT_IMPORT(inac,ltest);
INA_LJIT_IMPORT(inac,lprocqry);

INA_LJIT_PACKAGE(ljit);
INA_LJIT_IMPORT(ljit, bc);
INA_LJIT_IMPORT(ljit, bcsave);
INA_LJIT_IMPORT(ljit, dis_x64);
INA_LJIT_IMPORT(ljit, dis_x86);
INA_LJIT_IMPORT(ljit, v);
INA_LJIT_IMPORT(ljit, vmdef);
INA_LJIT_IMPORT(ljit, dump);

INA_API(ina_rc_t) ina_ljit_ctx_new(ina_ljit_ctx_t **ctx)
{   
    ina_str_t cur_path = NULL;
    ina_str_t new_path = NULL;

    INA_VERIFY_NOT_NULL(ctx);

    *ctx = (ina_ljit_ctx_t*)ina_mem_alloc(sizeof(ina_ljit_ctx_t));
    INA_RETURN_IF_NULL(*ctx);
    (*ctx)->lstate = luaL_newstate();
    if ((*ctx)->lstate == NULL) {
		INA_MEM_FREE_SAFE(*ctx);
        return INA_ERROR(INA_ES_STATE | INA_ERR_NOT_CREATED);
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

INA_API(void) ina_ljit_ctx_free(ina_ljit_ctx_t **ctx)
{
	INA_VERIFY_FREE(ctx);
    if (((*ctx)->lstate) != NULL) {
        lua_close((*ctx)->lstate);
    }
	INA_MEM_FREE_SAFE(*ctx);
}

unsigned long ina_ljit_hash_sbdm(const char *str)
{ 
    INA_ASSERT_NOT_NULL(str);
    return INA_HASH_CSTR_TO_SDBM(str);
}

INA_API(ina_rc_t) ina_ljit_call(ina_ljit_ctx_t *ctx, const char* fname, const char *sig, ...)
{
    va_list vl;
    int narg;
    int nres;
    char *cfname;

    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(ctx->lstate);
    INA_VERIFY_NOT_NULL(fname);
    INA_VERIFY_NOT_NULL(sig);

    /* Global function or object method? */
	cfname = (char*)strchr(fname, '.');
    if (!cfname) {
        /* get function */
        lua_getglobal(ctx->lstate, fname); 
    } else {    
        char *obj_name_c;
        ina_str_t obj_name = ina_str_new_fromcstr(fname);
        INA_ASSERT_NOT_NULL(obj_name);
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
                return INA_ERROR(INA_ES_TYPE | INA_ERR_INVALID);
         }
         narg++;
         luaL_checkstack(ctx->lstate, 1, "too many arguments");
    } endwhile:


    /* do the call */
    nres = (int)strlen(sig); /* We can assume a function would not return more than INT_MAX variables */
    if (lua_pcall(ctx->lstate, narg, nres, 0) != 0) {
        INA_ERROR(INA_ES_SCRIPT | INA_ERR_FAILED);
        lua_pop(ctx->lstate, 1);
        return ina_err_get_rc();
    }
    
    /* retrieve results */
    nres = -nres;  /* stack index of first result */
    while (*sig) {
        switch (*sig++) {
            case 'd':  /* double result */
              if (!lua_isnumber(ctx->lstate, nres)) {
                  return INA_ERROR(INA_ES_TYPE | INA_ERR_INVALID);
              }
              *va_arg(vl, double *) = lua_tonumber(ctx->lstate, nres);
              break;
            case 'i':  /* int result */
              if (!lua_isnumber(ctx->lstate, nres)) {
                  return INA_ERROR(INA_ES_TYPE | INA_ERR_INVALID);;
              }
              *va_arg(vl, int *) = (int)lua_tonumber(ctx->lstate, nres);
              break;

            case 's':  /* string result */
              if (lua_isstring(ctx->lstate, nres)) {
                  *va_arg(vl, const char **) = lua_tostring(ctx->lstate, nres);
              } else if (lua_type(ctx->lstate, nres) == 10) { 
                  *va_arg(vl, const char **) = INA_LJIT_TOCSTRING(ctx, nres);
              } else {
                  return INA_ERROR(INA_ES_TYPE | INA_ERR_INVALID);
              }
              break;
            case 'c': /* void pointer */
              if (lua_type(ctx->lstate, nres) == 10) { 
                  *va_arg(vl, const void **) = INA_LJIT_TOPOINTER(ctx, nres, const void*);
              } else {
                  return INA_ERROR(INA_ES_TYPE | INA_ERR_INVALID);

              }
              break;            
            default:
              return INA_ERROR(INA_ES_TYPE | INA_ERR_INVALID);
        }
        nres++;
    }
    va_end(vl);
    lua_pop(ctx->lstate, 1);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_ljit_dostring(ina_ljit_ctx_t *ctx, const char *code)
{
    INA_VERIFY_NOT_NULL(ctx);
    INA_VERIFY_NOT_NULL(code);
    if (luaL_dostring(ctx->lstate, code) != 0) {
        INA_ERROR(INA_ES_SCRIPT | INA_ERR_FAILED);
        /*INA_ERRMSG(INA_EEXCALL, lua_tostring(ctx->lstate, -1), NULL);*/
        lua_pop(ctx->lstate, 1);
        return ina_err_get_rc();
    }
    return INA_SUCCESS;
}

INA_API(const char*) ina_ljit_last_error(ina_ljit_ctx_t *ctx)
{
    INA_ASSERT_NOT_NULL(ctx);
    return luaL_checkstring(ctx->lstate, 1);
}

INA_API(ina_rc_t) ina_ljit_dump_stack(ina_ljit_ctx_t *ctx)
{
    int i;

    INA_VERIFY_NOT_NULL(ctx);

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
    INA_ASSERT_NOT_NULL(ctx);
    INA_ASSERT_NOT_NULL(ctx->lstate);
    if (lua_type(ctx->lstate, narg) != 10) {
        luaL_typerror(ctx->lstate, narg, "cdata");
    }
    return lua_topointer(ctx->lstate, narg);
}
