/*
 * Copyright (c) 2013-2014, INAOS GmbH
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

typedef enum __ina_template_table_type_s {
    __INA_TEMPLATE_TABLE_TYPE_HASH,
    __INA_TEMPLATE_TABLE_TYPE_ARRAY
} __ina_template_table_type_t;

typedef enum __ina_template_datatype_s {
    __INA_TEMPLATE_DATATYPE_NUMBER,
    __INA_TEMPLATE_DATATYPE_BOOLEAN,
    __INA_TEMPLATE_DATATYPE_STRING,
} __ina_template_datatype_t;

typedef struct __ina_template_data_s {
    __ina_template_table_type_t key_type;
    __ina_template_datatype_t value_type;
    union {
        const char *str_key;
        unsigned int int_key;
    };
    union {
        double d_val;
        const char *s_val;
        int b_val;
    };
} __ina_template_data_t;

struct ina_template_table_s {
    unsigned long key;
    __ina_template_table_type_t parent_type;
    ina_template_table_t *parent;
    ina_str_t str_id;
    int int_id;
    __ina_template_table_type_t type;
    ina_template_ctx_t *ctx;
    ina_template_env_t *env;
    ina_template_table_t *tables;
    UT_hash_handle hh;
};

struct ina_template_env_s {
    unsigned long key;
    ina_str_t id;
    ina_str_t tpl;
    ina_template_ctx_t *ctx;
    ina_template_table_t *tables;
    UT_hash_handle hh;
};

struct ina_template_ctx_s {
    ina_ljit_ctx_t *lctx;
    ina_template_env_t *envs;
};

static ina_rc_t __ina_template_prepare_stack(ina_template_table_t *tbl, int *depth)
{
    lua_State *l = tbl->ctx->lctx->lstate;

    if (tbl->parent != NULL) {
        if (!INA_SUCCEED(__ina_template_prepare_stack(tbl->parent, depth))) {
            return INA_ERR_PUSH_LAST;
        }
        if (tbl->type == __INA_TEMPLATE_TABLE_TYPE_HASH) {
            lua_getfield(l, -1, ina_str_cstr(tbl->str_id));
        }
        else {
            lua_rawgeti(l, -1, tbl->int_id);
        }
    }
    else {
        lua_getglobal(l, ina_str_cstr(tbl->env->id));
        lua_getfield(l, 1, ina_str_cstr(tbl->str_id));
    }

    (*depth)++;

    return INA_SUCCESS;
}

static ina_rc_t __ina_template_clean_stack(ina_template_table_t *tbl, int depth)
{
    lua_State *l = tbl->ctx->lctx->lstate;

    if (depth > 0) {
        lua_pop(l, depth); /* pop the tables from the stack */
    }
    lua_pop(l, 1); /* pop the env from the stack */

    return INA_SUCCESS;
}

static ina_rc_t __ina_template_env_set(__ina_template_data_t *data, ina_template_env_t *env)
{
    lua_State *l = env->ctx->lctx->lstate;

    INA_ASSERT_TRUE(data->key_type == __INA_TEMPLATE_TABLE_TYPE_HASH);
    INA_ASSERT_NOTNULL(data->str_key);

    lua_getglobal(l, ina_str_cstr(env->id));
    switch (data->value_type) {
        case __INA_TEMPLATE_DATATYPE_NUMBER:
            lua_pushnumber(l, data->d_val);
            break;
        case __INA_TEMPLATE_DATATYPE_BOOLEAN:
            lua_pushboolean(l, data->b_val);
            break;
        case __INA_TEMPLATE_DATATYPE_STRING:
            lua_pushstring(l, data->s_val);
            break;
    }
    lua_setfield(l, 1, data->str_key);
    lua_pop(l, 1); /* pop the env from the stack */

    return INA_SUCCESS;
}

static ina_rc_t __ina_template_tbl_set(__ina_template_data_t *data, ina_template_table_t *tbl)
{
    lua_State *l = tbl->ctx->lctx->lstate;
    int depth = 0;
    
    __ina_template_prepare_stack(tbl, &depth);
    switch (data->value_type) {
        case __INA_TEMPLATE_DATATYPE_NUMBER:
            lua_pushnumber(l, data->d_val);
            break;
        case __INA_TEMPLATE_DATATYPE_BOOLEAN:
            lua_pushboolean(l, data->b_val);
            break;
        case __INA_TEMPLATE_DATATYPE_STRING:
            lua_pushstring(l, data->s_val);
            break;
    }
    if (data->key_type == __INA_TEMPLATE_TABLE_TYPE_HASH) {
        lua_setfield(l, -2, data->str_key);
    }
    else {
        lua_rawseti(l, -2, data->int_key);
    }
    __ina_template_clean_stack(tbl, depth);

    return INA_SUCCESS;
}

static ina_rc_t __ina_template_new_table(__ina_template_data_t *data, ina_template_env_t *env, ina_template_table_t **tbl)
{
    lua_State *l = env->ctx->lctx->lstate;

    lua_getglobal(l, ina_str_cstr(env->id));
    lua_newtable(l);
    lua_setfield(l, 1, data->str_key);
    lua_pop(l, 1); /* pop the env from the stack */

    *tbl = (ina_template_table_t*)ina_mem_alloc(sizeof(ina_template_table_t));
    (*tbl)->key = INA_HASH_CSTR_TO_SDBM(data->str_key);
    (*tbl)->type = data->key_type;
    (*tbl)->str_id = ina_str_new_fromcstr(data->str_key);
    (*tbl)->ctx = env->ctx;
    (*tbl)->env = env;
    (*tbl)->parent = NULL;
    (*tbl)->tables = NULL;

    HASH_ADD_ULONG(env->tables, key, *tbl);

    return INA_SUCCESS;
}

static ina_rc_t __ina_template_new_nested_table(__ina_template_data_t *data, __ina_template_table_type_t type, 
                                                ina_template_table_t *tbl, ina_template_table_t **nested)
{
    int depth = 0;
    lua_State *l = tbl->ctx->lctx->lstate;

    __ina_template_prepare_stack(tbl, &depth);
    lua_newtable(l);
    if (type == __INA_TEMPLATE_TABLE_TYPE_HASH) {
        lua_setfield(l, -2, data->str_key);
    }
    else {
        lua_rawseti(l, -2, data->int_key);
    }
    __ina_template_clean_stack(tbl, depth);

    *nested = (ina_template_table_t*)ina_mem_alloc(sizeof(ina_template_table_t));
    if (type == __INA_TEMPLATE_TABLE_TYPE_HASH) {
        (*nested)->key = INA_HASH_CSTR_TO_SDBM(data->str_key);
    }
    else {
        (*nested)->key = data->int_key;
    }
    (*nested)->parent = tbl;
    (*nested)->parent_type = tbl->type;
    (*nested)->type = type;
    if (type == __INA_TEMPLATE_TABLE_TYPE_HASH) {
        (*nested)->str_id = ina_str_new_fromcstr(data->str_key);
    }
    else {
        (*nested)->int_id = data->int_key;
    }
    (*nested)->ctx = tbl->ctx;
    (*nested)->env = tbl->env;

    HASH_ADD_ULONG(tbl->tables, key, *nested);

    return INA_SUCCESS;
}

static ina_rc_t __ina_template_table_destroy(ina_template_table_t *head)
{
    ina_template_table_t *tbl, *ttbl;

    HASH_ITER(hh, head, tbl, ttbl) {
        HASH_DELETE(hh, head, tbl);
        if (tbl->tables != NULL) {
            __ina_template_table_destroy(tbl->tables);
        }
        if (tbl->type == __INA_TEMPLATE_TABLE_TYPE_HASH) {
            ina_str_free(tbl->str_id);
        }
        ina_mem_free(tbl);
    }

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_template_init(ina_template_ctx_t **ctx)
{
    *ctx = (ina_template_ctx_t*)ina_mem_alloc(sizeof(ina_template_ctx_t));
    if (!INA_SUCCEED(ina_ljit_init(&(*ctx)->lctx))) {
        ina_mem_free(*ctx);
        *ctx = NULL;
        return INA_ERR_PUSH_LAST;
    }
    (*ctx)->envs = NULL;

    /* load the template-engine into global namespace */
    if (luaL_dostring((*ctx)->lctx->lstate, "template = require(\"ltemplate\")") != 0) {
        return INA_LJIT_ELUA((*ctx)->lctx);
    }

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_template_destroy(ina_template_ctx_t **ctx)
{
    ina_template_env_t *env, *tenv;

    if (*ctx == NULL) {
        return INA_SUCCESS;
    }

    HASH_ITER(hh, (*ctx)->envs, env, tenv) {
        HASH_DELETE(hh, (*ctx)->envs, env);
        __ina_template_table_destroy(env->tables);
        ina_str_free(env->id);
        ina_str_free(env->tpl);
        ina_mem_free(env);
    }

    ina_ljit_destroy(&(*ctx)->lctx);
    ina_mem_free(*ctx);
    *ctx = NULL;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_template_compile(ina_template_ctx_t *ctx, const char *id, 
                                       ina_str_t tpl, ina_template_env_t **env)
{
    *env = (ina_template_env_t*)ina_mem_alloc(sizeof(ina_template_env_t));
    (*env)->key = INA_HASH_CSTR_TO_SDBM(id);
    (*env)->id = ina_str_new_fromcstr(id);
    (*env)->tables = NULL;
    (*env)->tpl = ina_str_dup(tpl);
    (*env)->ctx = ctx;
    
    lua_newtable(ctx->lctx->lstate);
    lua_setglobal(ctx->lctx->lstate, id);

    HASH_ADD_ULONG(ctx->envs, key, *env);

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_template_render(ina_template_env_t *env, ina_str_t *out)
{
    lua_State *l = env->ctx->lctx->lstate;

    lua_getglobal(l, "template");
    lua_getfield(l, -1, "process");
    lua_pushstring(l, ina_str_cstr(env->tpl));
    lua_getglobal(l, ina_str_cstr(env->id));
    if (lua_pcall(l, 2, 1, 0) != 0) {
        return INA_LJIT_ELUA(env->ctx->lctx);
    }
    *out = ina_str_new_fromcstr(luaL_checkstring(l, -1));
    lua_pop(l, 2);

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_template_set_number(ina_template_env_t *env, const char *key, double num)
{
    __ina_template_data_t arg;
    arg.key_type = __INA_TEMPLATE_TABLE_TYPE_HASH;
    arg.str_key = key;
    arg.value_type = __INA_TEMPLATE_DATATYPE_NUMBER;
    arg.d_val = num;
    return __ina_template_env_set(&arg, env);
}

INA_API(ina_rc_t) ina_template_set_boolean(ina_template_env_t *env, const char *key, int boolean)
{
    __ina_template_data_t arg;
    arg.key_type = __INA_TEMPLATE_TABLE_TYPE_HASH;
    arg.str_key = key;
    arg.value_type = __INA_TEMPLATE_DATATYPE_BOOLEAN;
    arg.b_val = boolean;
    return __ina_template_env_set(&arg, env);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_template_set_string(ina_template_env_t *env, const char *key, ina_str_t str)
{
    __ina_template_data_t arg;
    arg.key_type = __INA_TEMPLATE_TABLE_TYPE_HASH;
    arg.str_key = key;
    arg.value_type = __INA_TEMPLATE_DATATYPE_STRING;
    arg.s_val = ina_str_cstr(str);
    return __ina_template_env_set(&arg, env);
}

INA_API(ina_rc_t) ina_template_new_hash(ina_template_env_t *env, const char *key, ina_template_table_t **tbl)
{
    __ina_template_data_t arg;
    arg.key_type = __INA_TEMPLATE_TABLE_TYPE_HASH;
    arg.str_key = key;
    return __ina_template_new_table(&arg, env, tbl);
}

INA_API(ina_rc_t) ina_template_new_array(ina_template_env_t *env, const char *key, ina_template_table_t **tbl)
{
    __ina_template_data_t arg;
    arg.key_type = __INA_TEMPLATE_TABLE_TYPE_ARRAY;
    arg.str_key = key;
    return __ina_template_new_table(&arg, env, tbl);
}

INA_API(ina_rc_t) ina_template_table_new_nested_hash(ina_template_table_t *tbl, const char *key, ina_template_table_t **nested)
{
    __ina_template_data_t arg;
    arg.str_key = key;
    return __ina_template_new_nested_table(&arg, __INA_TEMPLATE_TABLE_TYPE_HASH, tbl, nested);
}

INA_API(ina_rc_t) ina_template_table_new_nested_array(ina_template_table_t *tbl, unsigned int idx, ina_template_table_t **nested)
{
    __ina_template_data_t arg;
    arg.int_key = idx;
    return __ina_template_new_nested_table(&arg, __INA_TEMPLATE_TABLE_TYPE_ARRAY, tbl, nested);
}

INA_API(ina_rc_t) ina_template_hash_set_number(ina_template_table_t *t, const char *key, double num)
{
    __ina_template_data_t arg;

    arg.value_type = __INA_TEMPLATE_DATATYPE_NUMBER;
    arg.d_val = num;
    arg.key_type = __INA_TEMPLATE_TABLE_TYPE_HASH;
    arg.str_key = key;

    return __ina_template_tbl_set(&arg, t);
}

INA_API(ina_rc_t) ina_template_hash_set_boolean(ina_template_table_t *t, const char *key, int boolean)
{
    __ina_template_data_t arg;

    arg.value_type = __INA_TEMPLATE_DATATYPE_BOOLEAN;
    arg.b_val = boolean;
    arg.key_type = __INA_TEMPLATE_TABLE_TYPE_HASH;
    arg.str_key = key;

    return __ina_template_tbl_set(&arg, t);
}

INA_API(ina_rc_t) ina_template_hash_set_string(ina_template_table_t *t, const char *key, ina_str_t str)
{
    __ina_template_data_t arg;

    arg.value_type = __INA_TEMPLATE_DATATYPE_STRING;
    arg.s_val = ina_str_cstr(str);
    arg.key_type = __INA_TEMPLATE_TABLE_TYPE_HASH;
    arg.str_key = key;

    return __ina_template_tbl_set(&arg, t);
}

INA_API(ina_rc_t) ina_template_array_set_number(ina_template_table_t *t, unsigned int idx, double num)
{
    __ina_template_data_t arg;

    arg.value_type = __INA_TEMPLATE_DATATYPE_NUMBER;
    arg.d_val = num;
    arg.key_type = __INA_TEMPLATE_TABLE_TYPE_ARRAY;
    arg.int_key = idx;

    return __ina_template_tbl_set(&arg, t);
}

INA_API(ina_rc_t) ina_template_array_set_boolean(ina_template_table_t *t, unsigned int idx, int boolean)
{
    __ina_template_data_t arg;

    arg.value_type = __INA_TEMPLATE_DATATYPE_BOOLEAN;
    arg.b_val = boolean;
    arg.key_type = __INA_TEMPLATE_TABLE_TYPE_ARRAY;
    arg.int_key = idx;

    return __ina_template_tbl_set(&arg, t);
}

INA_API(ina_rc_t) ina_template_array_set_string(ina_template_table_t *t, unsigned int idx, ina_str_t str)
{
    __ina_template_data_t arg;

    arg.value_type = __INA_TEMPLATE_DATATYPE_STRING;
    arg.s_val = ina_str_cstr(str);
    arg.key_type = __INA_TEMPLATE_TABLE_TYPE_ARRAY;
    arg.int_key = idx;

    return __ina_template_tbl_set(&arg, t);
}
