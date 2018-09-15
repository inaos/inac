/*
 * Copyright (c) 2013-2018, INAOS GmbH
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

#define __INA_ENUM_SECTIONS     "sections"
#define __INA_ENUM_KEYS         "keys"
#define __INA_ENUM_CHILDREN     "children"
#define __INA_ATTR_NAME         "name"
#define __INA_ATTR_NAMED        "named"
#define __INA_ATTR_CONFIGURED   "configured"
#define __INA_ATTR_REQUIRED     "required"
#define __INA_ATTR_TYPENAME     "typename"
#define __INA_ATTR_VALUE        "value"
#define __INA_ATTR_DEFAULT      "default"
#define __INA_ATTR_HAS_VALUE    "has_value"
#define __INA_VAL_STRING        "string"
#define __INA_VAL_NUMBER        "number"

/* Configuration file data */
struct ina_conffile_s {
    ina_str_t filepath;                /* Filepath */
    ina_ljit_ctx_t *lctx;              /* LuaJIT context */
    int prepared;                      /* INA_YES if prepared */
    ina_mempool_t *mempool;            /* Memory pool */
    ina_hashtable_t *sections;
};

/* Single entry */
typedef struct ina_conffile_entry_s {
    ina_str_t key;
    ina_conffile_value_type_t value_type;
    union value_u {
        ina_str_t s;
        double n;
    } value;
} ina_conffile_entry_t;

typedef struct ina_conffile_section_key_s {
    ina_str_t name;
    int required;
    ina_conffile_value_type_t value_type;
} ina_conffile_section_key_t;

typedef struct ina_conffile_entries_s {
    ina_str_t key;
    ina_hashtable_t *entries;
} ina_conffile_entries_t;

struct ina_conffile_section_s {
    ina_conffile_t *cf;
    ina_str_t name;
    int required;
    int named;
    int configured;
    ina_hashtable_t *keys;
    ina_conffile_section_cb_t section_cb;
    ina_hashtable_t *entries;
};

/* Build LUA section table */
static ina_rc_t __ina_build_section_table(ina_conffile_t*);
/* Prepare configuration file */
static ina_rc_t __ina_prepare(ina_conffile_t*);
/* Processs the LUA section table */
static ina_rc_t __ina_process_section_table(ina_conffile_t*);
/* Process configuration file entries */
static ina_rc_t __ina_process_entries(ina_conffile_t*,
                                      ina_conffile_entries_t*);
/* Internal getter function for a value */
static ina_rc_t __ina_get_value(ina_conffile_t*, const char*, const char*, 
                                const char*, 
                                ina_conffile_entry_t**);


INA_API(ina_rc_t) ina_conffile_new(ina_conffile_t **cf)
{
    INA_VERIFY_NOT_NULL(cf);

    *cf = (ina_conffile_t*)ina_mem_alloc(sizeof(ina_conffile_t));
    INA_RETURN_IF(*cf == NULL);
    ina_mem_set(*cf, 0, sizeof(ina_conffile_t));

    if (INA_FAILED(ina_ljit_init(&(*cf)->lctx))) {
        ina_mem_free(*cf);
        *cf = NULL;
        return ina_err_get_last_rc();
    }
    if (INA_FAILED(ina_mempool_new(&(*cf)->mempool,
                                        4094, 
                                        INA_MEM_DYNAMIC, 
                                        NULL))) {
        ina_ljit_destroy(&(*cf)->lctx);
        ina_mem_free(*cf);
        *cf = NULL;
        return ina_err_get_last_rc();
    }
    if (INA_FAILED(ina_hashtable_new(INA_HASHTABLE_STR_KEY,
                      INA_HASHTABLE_HASH_DEFAULT,
                      INA_HASHTABLE_TYPE_DEFAULT,
                      INA_HASHTABLE_GROW_DEFAULT,
                      INA_HASHTABLE_SHRINK_DEFAULT,
                      INA_HASHTABLE_DEFAULT_CAPACITY,
                      INA_HASHTABLE_CF_DEFAULT, &(*cf)->sections))) {

        ina_mempool_free(&(*cf)->mempool);
        ina_ljit_destroy(&(*cf)->lctx);
        ina_mem_free(*cf);
        *cf = NULL;
        return ina_err_get_last_rc();
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_conffile_free(ina_conffile_t **cf)
{
    INA_VERIFY_NOT_NULL(cf);
    INA_VERIFY_NOT_NULL(*cf);

    INA_MUST_SUCCEED(ina_ljit_destroy(&(*cf)->lctx));
    INA_MUST_SUCCEED(ina_mempool_free(&(*cf)->mempool));
    INA_MUST_SUCCEED(ina_hashtable_free(&(*cf)->sections));
    ina_mem_free(*cf);
    *cf = NULL;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_conffile_add_section(ina_conffile_t *cf, 
            const char *name, int required, int named, 
            ina_conffile_section_cb_t cb, 
            ina_conffile_section_t **section)
{
    unsigned long key;
    ina_conffile_section_t *sp;
    ina_conffile_section_t *check;

    INA_VERIFY_NOT_NULL(cf);
    INA_VERIFY_NOT_NULL(name);
    INA_VERIFY_NOT_NULL(section);

    *section = NULL;
    
    if (cf->prepared == INA_YES) {
        return INA_ERROR(INA_ERR_INITIALIZED);
    }

    if (INA_SUCCEED(ina_hashtable_get_str(cf->sections, name, (void**)&check))) {
        return INA_ERROR(INA_ERR_NOT_UNIQUE);
    }

    *section = (ina_conffile_section_t*)ina_mempool_dalloc(cf->mempool, 
                                            sizeof(ina_conffile_section_t));
    sp = *section;
    if (sp == NULL) {
        *section = NULL;
        return ina_err_get_last_rc();
    }

    sp->cf = cf;
    sp->name = ina_str_new_fromcstr_using_pool(name, cf->mempool);
    sp->named = named;
    sp->section_cb = cb;
    sp->required = required;

    ina_hashtable_new(INA_HASHTABLE_STR_KEY,
                      INA_HASHTABLE_HASH_DEFAULT,
                      INA_HASHTABLE_TYPE_DEFAULT,
                      INA_HASHTABLE_GROW_DEFAULT,
                      INA_HASHTABLE_SHRINK_DEFAULT,
                      INA_HASHTABLE_DEFAULT_CAPACITY,
                      INA_HASHTABLE_CF_DEFAULT, &sp->keys);

    return ina_hashtable_set_str(cf->sections, sp->name, sp);;
}

INA_API(ina_rc_t) ina_conffile_add_key(ina_conffile_section_t *section, 
                            const char *name, 
                            ina_conffile_value_type_t value_type, 
                            int required)
{
    unsigned long k;
    ina_conffile_section_key_t *key;

    INA_VERIFY_NOT_NULL(section);
    INA_VERIFY_NOT_NULL(name);

    if (INA_SUCCEED(ina_hashtable_get_str(section->keys, name, (void**)&key))) {
        return INA_ERROR(INA_ERR_NOT_UNIQUE);
    }
    key = (ina_conffile_section_key_t*)ina_mempool_dalloc(
                                        section->cf->mempool,
                                        sizeof(ina_conffile_section_key_t));
    if (key == NULL) {
        return ina_err_get_last_rc();
    }

    key->name = ina_str_new_fromcstr_using_pool(name, section->cf->mempool);
    key->required = required;
    key->value_type = value_type;
    return ina_hashtable_set_str(section->keys, name, key);
}

INA_API(ina_rc_t) ina_conffile_has_value(ina_conffile_t *cf,
                                         const char *section_name,
                                         const char *section_key,
                                         const char *key)
{
    ina_conffile_entry_t *entry = NULL;

    INA_ASSERT_NOTNULL(cf);
    INA_ASSERT_NOTNULL(section_name);
    INA_ASSERT_NOTNULL(key);
    return __ina_get_value(cf, section_name, section_key, key, &entry);
}

INA_API(ina_rc_t) ina_conffile_has_value_in_entries(
                                            ina_conffile_entries_t *entries,
                                            const char* key)
{
    ina_conffile_entry_t *entry = NULL;
    INA_VERIFY_NOT_NULL(entries);
    INA_VERIFY_NOT_NULL(key);

    return ina_hashtable_get_str(entries->entries, key, (void**)&entry);
}


INA_API(ina_rc_t) ina_conffile_get_string(ina_conffile_t *cf,
                                          const char *section_name,
                                          const char *section_key, 
                                          const char *key,
                                          const ina_str_t *value)
{
    ina_conffile_entry_t *entry = NULL;

    INA_VERIFY_NOT_NULL(cf);
    INA_VERIFY_NOT_NULL(section_name);
    INA_VERIFY_NOT_NULL(key);
    INA_VERIFY_NOT_NULL(value);

    __ina_get_value(cf, section_name, section_key, key, &entry);
    if (entry != NULL) {
        if (entry->value_type != INA_CONFFILE_VALUE_TYPE_STRING) {
            return INA_ERROR(INA_ERR_INVALID|INA_NN_TYPE);
        }
        if (entry->value.s != NULL) {
            *((ina_str_t*)value) = entry->value.s;
            return INA_SUCCESS;
        }
    }
    return INA_ERROR(INA_ERR_NOT_EXISTS);
}

INA_API(ina_rc_t) ina_conffile_get_string_from_entries(
                                            ina_conffile_entries_t *entries,
                                            const char* key, 
                                            const ina_str_t *value)
{
    ina_conffile_entry_t *entry = NULL;

    INA_VERIFY_NOT_NULL(entries);
    INA_VERIFY_NOT_NULL(key);
    INA_VERIFY_NOT_NULL(value);

    if (INA_SUCCEED(ina_hashtable_get_str(entries->entries, key, (void**)&entry))) {
        if (entry->value_type != INA_CONFFILE_VALUE_TYPE_STRING) {
            return INA_ERROR(INA_ERR_INVALID|INA_NN_TYPE);
        }
        if (entry->value.s != NULL) {
            *((ina_str_t*)value) = entry->value.s;
            return INA_SUCCESS;
        }
    }
    return INA_ERROR(INA_ERR_NOT_EXISTS);
}

INA_API(ina_rc_t) ina_conffile_get_number(ina_conffile_t *cf, 
                                          const char *section_name,
                                          const char *section_key,
                                          const char *key,
                                          double *value)
{
    ina_conffile_entry_t *entry = NULL;

    INA_VERIFY_NOT_NULL(cf);
    INA_VERIFY_NOT_NULL(section_name);
    INA_VERIFY_NOT_NULL(key);
    INA_VERIFY_NOT_NULL(value);

    __ina_get_value(cf, section_name, section_key, key, &entry);
    if (entry != NULL) {
        if (entry->value_type != INA_CONFFILE_VALUE_TYPE_NUMBER) {
            return INA_ERROR(INA_ERR_INVALID|INA_NN_TYPE);
        }
        *value = entry->value.n;
        return INA_SUCCESS;
    }
    return INA_ERROR(INA_ERR_NOT_EXISTS);
}

INA_API(ina_rc_t) ina_conffile_get_number_from_entries(
                                            ina_conffile_entries_t *entries,
                                            const char* key, 
                                            double *value)
{
    ina_conffile_entry_t *entry = NULL;

    INA_VERIFY_NOT_NULL(entries);
    INA_VERIFY_NOT_NULL(key);
    INA_VERIFY_NOT_NULL(value);
    
    if (INA_SUCCEED(ina_hashtable_get_str(entries->entries, key, (void**)&entry))) {
        if (entry->value_type != INA_CONFFILE_VALUE_TYPE_NUMBER) {
            return INA_ERROR(INA_ERR_INVALID|INA_NN_TYPE);
        }
        *value = entry->value.n;
        return INA_SUCCESS;
    }
    return INA_ERROR(INA_ERR_NOT_EXISTS);
}

                    
INA_API(ina_rc_t) ina_conffile_process(ina_conffile_t *cf, const char *filepath)
{
    ina_conffile_section_t *s;
    ina_hashtable_iter_t *iter;

    INA_VERIFY_NOT_NULL(cf);
 
    /* Almost one section must be there */
    if (cf->sections == NULL) {
        return INA_ERROR(INA_NN_CONFIGURATION|INA_ERR_EMPTY);
    }
    
    if (cf->prepared != INA_YES) {
        if (INA_FAILED(__ina_prepare(cf))) {
            return ina_err_get_last_rc();
        }
    }

    lua_getglobal(cf->lctx->lstate, __INA_ENUM_SECTIONS);

    /* set the config-file path */
    if (filepath != NULL) {
        cf->filepath = ina_str_new_fromcstr_using_pool(filepath, cf->mempool);
    }
    if (cf->filepath == NULL) {
        cf->filepath = ina_str_new(128);
        if (ina_str_snprintf(&cf->filepath, 128, "%s.conf", ina_app_get_name()) > 128) {
            ina_str_t filepath = ina_str_dup_using_pool(cf->filepath, cf->mempool);
            ina_str_free(cf->filepath);
            cf->filepath = filepath;
        }
    }

    lua_pushstring(cf->lctx->lstate, ina_str_cstr(cf->filepath));
    lua_setglobal(cf->lctx->lstate, "conf_file");
    
    /* invoke lua */
    if (luaL_dostring(cf->lctx->lstate,
        "local cf = require('lconffile')\n cf.process(sections, conf_file)\n") 
		    != 0) {
        /*INA_ERRMSG(INA_EEXCALL, lua_tostring(cf->lctx->lstate, -1), NULL);*/
        INA_ERROR(INA_NN_SCRIPT|INA_ERR_FAILED);
        lua_pop(cf->lctx->lstate, 1);
        return ina_err_get_last_rc();
    }

    /* process section table */
    if (INA_FAILED(__ina_process_section_table(cf))) {
        return ina_err_get_last_rc();
    }

    /* invoke callbacks */
    ina_hashtable_iter_new(cf->sections, &iter);
    while (INA_SUCCEED(ina_hashtable_iter_next(iter, (void**)&s))) {
        if (s->section_cb != NULL) {
            ina_hashtable_iter_t *iter2;
            ina_conffile_entries_t *entries;
            ina_hashtable_iter_new(s->entries, &iter2);
            while INA_SUCCEED(ina_hashtable_iter_next(iter2, (void**)&entries)) {
                const char *key = (s->named?entries->key:NULL);
                if (INA_FAILED((s->section_cb(s->name, key, entries)))) {
                    return ina_err_get_last_rc();
                }
            }
        }
    }
    return INA_SUCCESS;
}

static ina_rc_t 
__ina_prepare(ina_conffile_t *cf)
{
    if (INA_FAILED(ina_ljit_init(&cf->lctx))) {
        return ina_err_get_last_rc();
    }
    if (cf->prepared == INA_YES) {
        return INA_SUCCESS;
    }

    /* create the sections table */
    lua_newtable(cf->lctx->lstate);
    lua_setglobal(cf->lctx->lstate, __INA_ENUM_SECTIONS);

    /* construct sections table */
    if (INA_FAILED(__ina_build_section_table(cf))) {
        return ina_err_get_last_rc();
    }
    cf->prepared = INA_YES;
    return INA_SUCCESS;
}

static ina_rc_t 
__ina_build_section_table(ina_conffile_t *cf)
{
    ina_conffile_section_t *s;
    ina_conffile_section_key_t *k;
    ina_hashtable_iter_t *iter;

    lua_State* lstate = cf->lctx->lstate;

    lua_getglobal(lstate, __INA_ENUM_SECTIONS);

    ina_hashtable_iter_new(cf->sections, &iter);
    while (INA_SUCCEED(ina_hashtable_iter_next(iter, (void**)&s))) {
        ina_hashtable_iter_t *iter2;
        lua_pushstring(lstate, s->name);
        lua_newtable(lstate);

        /* set name */
        lua_pushstring(lstate, __INA_ATTR_NAME);
        lua_pushstring(lstate, s->name);
        lua_rawset(lstate, -3);

        /* set named */
        lua_pushstring(lstate, __INA_ATTR_NAMED);
        lua_pushboolean(lstate, s->named);
        lua_rawset(lstate, -3);

        /* set required */  
        lua_pushstring(lstate, __INA_ATTR_REQUIRED);
        lua_pushboolean(lstate, s->required);
        lua_rawset(lstate, -3);

        /* set configured */
        lua_pushstring(lstate, __INA_ATTR_CONFIGURED);
        lua_pushboolean(lstate, s->configured);
        lua_rawset(lstate, -3);

        /* set keys */
        lua_pushstring(lstate, __INA_ENUM_KEYS);
        lua_newtable(lstate);

        ina_hashtable_iter_new(s->keys, &iter2);
        while (INA_SUCCEED(ina_hashtable_iter_next(iter2, (void**)&k))) {
            lua_pushstring(lstate, k->name);
            lua_newtable(lstate);

            lua_pushstring(lstate, __INA_ATTR_REQUIRED);
            lua_pushboolean(lstate, k->required);
            lua_rawset(lstate, -3);

            lua_pushstring(lstate, __INA_ATTR_TYPENAME);
            if (k->value_type == INA_CONFFILE_VALUE_TYPE_STRING) {
                lua_pushstring(lstate, __INA_VAL_STRING);
            } else {
                lua_pushstring(lstate, __INA_VAL_NUMBER);
            }
            lua_rawset(lstate, -3);
            lua_rawset(lstate, -3);
        }
        ina_hashtable_iter_free(&iter2);
        lua_rawset(lstate, -3);
        lua_rawset(lstate, -3);
    }
    ina_hashtable_iter_free(&iter);
    return INA_SUCCESS;
}

static ina_rc_t 
__ina_process_section_table(ina_conffile_t *cf)
{
    lua_State *lstate = cf->lctx->lstate;

    lua_getglobal(lstate, __INA_ENUM_SECTIONS);
    lua_pushnil(lstate);

    while(lua_next(lstate, -2)) {
        const char *name;
        int named;
        int configured;
 
        lua_getfield(lstate, -1 , __INA_ATTR_NAME);
        name = lua_tostring(lstate, -1);
        lua_pop(lstate, 1);

        lua_getfield(lstate, -1 , __INA_ATTR_NAMED);
        named = lua_toboolean(lstate, -1);
        lua_pop(lstate, 1);

        lua_getfield(lstate, -1 , __INA_ATTR_CONFIGURED);
        configured = lua_toboolean(lstate, -1);
        lua_pop(lstate, 1);

        if (configured) {
            ina_conffile_section_t *s;
            ina_conffile_entries_t *e;

            ina_hashtable_get_str(cf->sections, name, (void**)&s);

            if (!named) {
                const char *rk = __INA_ATTR_DEFAULT;
                unsigned long rki = INA_HASH_CSTR_TO_SDBM(rk);

                e = (ina_conffile_entries_t*)ina_mempool_dalloc(
                                        cf->mempool,
                                        sizeof(ina_conffile_entries_t));
                e->key = ina_str_new_fromcstr_using_pool(rk, cf->mempool);
                ina_hashtable_new(INA_HASHTABLE_STR_KEY,
                                  INA_HASHTABLE_HASH_DEFAULT,
                                  INA_HASHTABLE_TYPE_DEFAULT,
                                  INA_HASHTABLE_GROW_DEFAULT,
                                  INA_HASHTABLE_SHRINK_DEFAULT,
                                  INA_HASHTABLE_DEFAULT_CAPACITY,
                                  INA_HASHTABLE_CF_DEFAULT, &e->entries);

                ina_hashtable_set_str(s->entries, e->key, e);

                lua_getfield(lstate, -1 , __INA_ENUM_KEYS);
                __ina_process_entries(cf, e);
            } else {
                lua_getfield(lstate, -1 , __INA_ENUM_CHILDREN);
                lua_pushnil(lstate);
                while(lua_next(lstate, -2)) {
                    const char *rk;
                    unsigned long rki;

                    rk = lua_tostring(lstate, -2);

                    e = (ina_conffile_entries_t*)ina_mempool_dalloc(
                                        cf->mempool,
                                        sizeof(ina_conffile_entries_t));
                    e->key = ina_str_new_fromcstr_using_pool(rk, cf->mempool);
                    ina_hashtable_new(INA_HASHTABLE_STR_KEY,
                                      INA_HASHTABLE_HASH_DEFAULT,
                                      INA_HASHTABLE_TYPE_DEFAULT,
                                      INA_HASHTABLE_GROW_DEFAULT,
                                      INA_HASHTABLE_SHRINK_DEFAULT,
                                      INA_HASHTABLE_DEFAULT_CAPACITY,
                                      INA_HASHTABLE_CF_DEFAULT, &e->entries);
                    ina_hashtable_set_str(s->entries, e->key, e);
                    __ina_process_entries(cf, e);
                }
                lua_pop(lstate, 1);
            }
        }
        lua_pop(lstate, 1);
    }
    lua_pop(lstate, 1);
    return INA_SUCCESS;
}

static ina_rc_t 
__ina_process_entries(ina_conffile_t *cf, ina_conffile_entries_t *entries)
{
    lua_State *lstate = cf->lctx->lstate;
    lua_pushnil(lstate);
    while(lua_next(lstate, -2)) {
        int has_value;
        const char *k = lua_tostring(lstate, -2);
        const char *tn;
        ina_conffile_entry_t *entry;

        lua_getfield(lstate, -1 , __INA_ATTR_TYPENAME);
        tn = lua_tostring(lstate, -1);
        lua_pop(lstate, 1);

        lua_getfield(lstate, -1 , __INA_ATTR_HAS_VALUE);
        has_value = lua_toboolean(lstate, -1);
        lua_pop(lstate, 1);

        if (has_value) {
            entry = (ina_conffile_entry_t*)ina_mempool_dalloc(
                                            cf->mempool,
                                            sizeof(ina_conffile_entry_t));
            INA_ASSERT_NOTNULL(entry);
            entry->key = ina_str_new_fromcstr_using_pool(k, cf->mempool);
            lua_getfield(lstate, -1 , __INA_ATTR_VALUE);
            if (strcmp(tn, __INA_VAL_STRING) == 0) {
                entry->value_type = INA_CONFFILE_VALUE_TYPE_STRING;
                entry->value.s = ina_str_new_fromcstr_using_pool(
                                                lua_tostring(lstate, -1),
                                                cf->mempool);
            } else {
                entry->value_type = INA_CONFFILE_VALUE_TYPE_NUMBER;
                entry->value.n = lua_tonumber(lstate, -1);
            }
            lua_pop(lstate, 1);
            ina_hashtable_set_str(entries->entries, entry->key, entry);
        }
        lua_pop(lstate, 1);
    }
    lua_pop(lstate, 1);
    return INA_SUCCESS;
}

static ina_rc_t 
__ina_get_value(ina_conffile_t *cf, const char* section_name, 
                const char *section_key, const char *key, 
                ina_conffile_entry_t **entry)
{
    ina_conffile_section_t *section = NULL;
    ina_conffile_entries_t *entries = NULL;
    ina_conffile_entry_t *e = NULL;

    /* First section lookup */
    if (INA_FAILED(ina_hashtable_get_str(cf->sections, section_key, (void**)&section))) {
        return INA_ERROR(INA_NN_SECTION|INA_ERR_NOT_EXISTS);
    }
    
    /* Second section lockup for named sections */
    if (section->named) {
        ina_hashtable_get_str(section->entries, section_key, (void**)&entries);
    } else {
        ina_hashtable_get_str(section->entries, __INA_ATTR_DEFAULT, (void**)&entries);
    }
    if (entries == NULL || entries->entries == NULL) {
        return INA_ERROR(INA_NN_SECTION|INA_ERR_EMPTY);
    }

    /* Lookup value */
    ina_hashtable_get_str(entries->entries, key, (void**)&e);
    if (e == NULL) {
        return INA_ERROR(INA_NN_KEY|INA_ERR_NOT_EXISTS);
    }
    *entry = e;
    return INA_SUCCESS;
}
