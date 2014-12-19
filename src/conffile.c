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

/* Single entry */
struct ina_conffile_entry_s {
    unsigned long id;
    ina_str_t key;
    ina_conffile_value_type_t value_type;
    union value_u {
        ina_str_t s;
        double n;
    } value;
    UT_hash_handle hh;
};

typedef struct ina_conffile_section_key_s {
    unsigned long id;
    ina_str_t name;
    int required;
    ina_conffile_value_type_t value_type;
    UT_hash_handle hh;
} ina_conffile_section_key_t;

typedef struct ina_conffile_section_res_s {
    unsigned long id;
    ina_str_t key;
    ina_conffile_entry_t *entries;
    UT_hash_handle hh;
} ina_conffile_section_res_t;

struct ina_conffile_section_s {
    unsigned long id;
    ina_conffile_t *cf;
    ina_str_t name;
    int required;
    int named;
    int configured;
    ina_conffile_section_key_t *keys;
    ina_conffile_section_cb_t section_cb;
    ina_conffile_section_res_t *results;
    UT_hash_handle hh;
};

/* Build LUA section table */
static ina_rc_t __ina_build_section_table(ina_conffile_t*);
/* Prepare configuration file */
static ina_rc_t __ina_prepare(ina_conffile_t*);
/* Processs the LUA section table */
static ina_rc_t __ina_process_section_table(ina_conffile_t*);
/* Process configuration file entries */
static ina_rc_t __ina_process_entries(ina_conffile_t*, 
                                      ina_conffile_section_res_t*);
/* Internal getter function for a value */
static ina_rc_t __ina_get_value(ina_conffile_t*, const char*, const char*, 
                                const char*, 
                                ina_conffile_entry_t**);


INA_API(ina_rc_t) ina_conffile_init(ina_conffile_t **cf)
{
    INA_ASSERT_NOTNULL(cf);

    *cf = (ina_conffile_t*)ina_mem_alloc(sizeof(ina_conffile_t));
    if (*cf == NULL) {
        return INA_ERR_PUSH_LAST;
    }

    if (!INA_SUCCEED(ina_ljit_init(&(*cf)->lctx))) {
        ina_mem_free(*cf);
        *cf = NULL;
        return INA_ERR_PUSH_LAST;
    }
    if (!INA_SUCCEED(ina_mempool_create(&(*cf)->mempool, 
                                        4094, 
                                        INA_MEM_DYNAMIC, 
                                        NULL))) {
        ina_ljit_destroy(&(*cf)->lctx);
        ina_mem_free(*cf);
        *cf = NULL;
        return INA_ERR_PUSH_LAST;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_conffile_destroy(ina_conffile_t **cf)
{
    INA_ASSERT_NOTNULL(cf);

    if (*cf == NULL) {
        return INA_SUCCESS;
    }
    ina_ljit_destroy(&(*cf)->lctx);
    ina_mempool_release((*cf)->mempool, INA_YES);
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

    INA_ASSERT_NOTNULL(cf);
    INA_ASSERT_NOTNULL(name);
    INA_ASSERT_NOTNULL(section);
    
    if (cf->prepared == INA_YES) {
        return INA_CONFFILE_EPREPARED;
    }

    key = INA_HASH_CSTR_TO_SDBM(name);
    HASH_FIND_ULONG(cf->sections, &key, check);
    if (check != NULL) {
        return INA_CONFFILE_EDUPSEC;
    }

    *section = (ina_conffile_section_t*)ina_mempool_dalloc(cf->mempool, 
                                            sizeof(ina_conffile_section_t));
    sp = *section;
    if (sp == NULL) {
        return INA_ERR_PUSH_LAST;
    }

    sp->id = key;
    sp->cf = cf;
    sp->name = ina_str_new_fromcstr_using_pool(name, cf->mempool);
    sp->named = named;
    sp->section_cb = cb;
    sp->required = required;
    HASH_ADD_ULONG(cf->sections, id, sp);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_conffile_add_key(ina_conffile_section_t *section, 
                            const char *name, 
                            ina_conffile_value_type_t value_type, 
                            int required)
{
    unsigned long k;
    ina_conffile_section_key_t *key;
    ina_conffile_entry_t *check = NULL;

    INA_ASSERT_NOTNULL(section);
    INA_ASSERT_NOTNULL(name);
    
    k = INA_HASH_CSTR_TO_SDBM(name);

    if (check != NULL) {
        return INA_CONFFILE_EDUPKEY;
    }

    key = (ina_conffile_section_key_t*)ina_mempool_dalloc(
                                        section->cf->mempool,
                                        sizeof(ina_conffile_section_key_t));
    if (key == NULL) {
        return INA_ERR_PUSH_LAST;
    }

    key->id = k;
    key->name = ina_str_new_fromcstr_using_pool(name, section->cf->mempool);
    key->required = required;
    key->value_type = value_type;
    HASH_ADD_ULONG(section->keys, id, key);
    return INA_SUCCESS;
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
                                            ina_conffile_entry_t *entries, 
                                            const char* key)
{
    ina_conffile_entry_t *entry = NULL;
    unsigned long k;

    INA_ASSERT_NOTNULL(entries);
    INA_ASSERT_NOTNULL(key);

    k = INA_HASH_CSTR_TO_SDBM(key);

    HASH_FIND_ULONG(entries, &k, entry);
    if (entry != NULL) {
        return INA_SUCCESS;
    }
    return INA_FAILURE;
}


INA_API(ina_rc_t) ina_conffile_get_string(ina_conffile_t *cf,
                                          const char *section_name,
                                          const char *section_key, 
                                          const char *key,
                                          const ina_str_t *value)
{
    ina_conffile_entry_t *entry = NULL;

    INA_ASSERT_NOTNULL(cf);
    INA_ASSERT_NOTNULL(section_name);
    INA_ASSERT_NOTNULL(key);
    INA_ASSERT_NOTNULL(value);

    __ina_get_value(cf, section_name, section_key, key, &entry);
    if (entry != NULL) {
        if (entry->value_type != INA_CONFFILE_VALUE_TYPE_STRING) {
            return INA_CONFFILE_ETYPE;
        }
        if (entry->value.s != NULL) {
            *((ina_str_t*)value) = entry->value.s;
            return INA_SUCCESS;
        }
    }
    return INA_FAILURE;
}

INA_API(ina_rc_t) ina_conffile_get_string_from_entries(
                                            ina_conffile_entry_t *entries, 
                                            const char* key, 
                                            const ina_str_t *value)
{
    ina_conffile_entry_t *entry = NULL;
    unsigned long k;

    INA_ASSERT_NOTNULL(entries);
    INA_ASSERT_NOTNULL(key);
    INA_ASSERT_NOTNULL(value);
    
    k = INA_HASH_CSTR_TO_SDBM(key);

    HASH_FIND_ULONG(entries, &k, entry);
    if (entry != NULL) {
        if (entry->value_type != INA_CONFFILE_VALUE_TYPE_STRING) {
            return INA_CONFFILE_ETYPE;
        }
        if (entry->value.s != NULL) {
            *((ina_str_t*)value) = entry->value.s;
            return INA_SUCCESS;
        }
    }
    return INA_FAILURE;
}

INA_API(ina_rc_t) ina_conffile_get_number(ina_conffile_t *cf, 
                                          const char *section_name,
                                          const char *section_key,
                                          const char *key,
                                          double *value)
{
    ina_conffile_entry_t *entry = NULL;

    INA_ASSERT_NOTNULL(cf);
    INA_ASSERT_NOTNULL(section_name);
    INA_ASSERT_NOTNULL(key);
    INA_ASSERT_NOTNULL(value);

    __ina_get_value(cf, section_name, section_key, key, &entry);
    if (entry != NULL) {
        if (entry->value_type != INA_CONFFILE_VALUE_TYPE_NUMBER) {
            return INA_CONFFILE_ETYPE;
        }
        *value = entry->value.n;
        return INA_SUCCESS;
    }
    return INA_FAILURE;
}

INA_API(ina_rc_t) ina_conffile_get_number_from_entries(
                                            ina_conffile_entry_t *entries, 
                                            const char* key, 
                                            double *value)
{
    ina_conffile_entry_t *entry = NULL;
    unsigned long k;

    INA_ASSERT_NOTNULL(entries);
    INA_ASSERT_NOTNULL(key);
    INA_ASSERT_NOTNULL(value);
    
    k = INA_HASH_CSTR_TO_SDBM(key);

    HASH_FIND_ULONG(entries, &k, entry);
    if (entry != NULL) {
        if (entry->value_type != INA_CONFFILE_VALUE_TYPE_NUMBER) {
            return INA_CONFFILE_ETYPE;
        }
        *value = entry->value.n;
        return INA_SUCCESS;
    }
    return INA_FAILURE;
}

                    
INA_API(ina_rc_t) ina_conffile_process(ina_conffile_t *cf, const char *filepath)
{
    ina_conffile_section_t *s, *stmp;

    INA_ASSERT_NOTNULL(cf);
 
    /* Almost one section must be there */
    if (cf->sections == NULL) {
        return INA_FAILURE;
    }
    
    if (cf->prepared != INA_YES) {
        if (!INA_SUCCEED(__ina_prepare(cf))) {
            return INA_ERR_PUSH_LAST;
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
        return INA_LJIT_ELUA(cf->lctx);
    }

    /* process section table */
    if (!INA_SUCCEED(__ina_process_section_table(cf))) {
        return INA_FAILURE;
    }

    /* invoke callbacks */
    HASH_ITER(hh, cf->sections, s, stmp) {
        if (s->section_cb != NULL) {
            if (!s->named) {
                if (!INA_SUCCEED(s->section_cb(s->name, 
						NULL, 
						s->results->entries))) {
                    return INA_FAILURE;
                }
            } else {
                ina_conffile_section_res_t *r, *rtmp;
                HASH_ITER(hh, s->results, r, rtmp) {
                    if (!INA_SUCCEED(s->section_cb(s->name, 
						    r->key, 
						    r->entries))) {
                        return INA_FAILURE;
                    }
                }
            }
        }
    }
    return INA_SUCCESS;
}

static ina_rc_t 
__ina_prepare(ina_conffile_t *cf)
{
    INA_ASSERT_NOTNULL(cf);

    if (!INA_SUCCEED(ina_ljit_init(&cf->lctx))) {
        return INA_ERR_PUSH_LAST;
    }
    if (cf->prepared == INA_YES) {
        return INA_SUCCESS;
    }

    /* create the sections table */
    lua_newtable(cf->lctx->lstate);
    lua_setglobal(cf->lctx->lstate, __INA_ENUM_SECTIONS);

    /* construct sections table */
    if (!INA_SUCCEED(__ina_build_section_table(cf))) {
        return INA_FAILURE;
    }
    cf->prepared = INA_YES;
    return INA_SUCCESS;
}

static ina_rc_t 
__ina_build_section_table(ina_conffile_t *cf)
{
    ina_conffile_section_t *s, *stmp;
    ina_conffile_section_key_t *k, *ktmp;
    lua_State* lstate = cf->lctx->lstate;

    lua_getglobal(lstate, __INA_ENUM_SECTIONS);

    HASH_ITER(hh, cf->sections, s, stmp) {
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

        HASH_ITER(hh, s->keys, k, ktmp) {
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
        lua_rawset(lstate, -3);
        lua_rawset(lstate, -3);
    }
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
            ina_conffile_section_res_t *res;
            unsigned long sk = INA_HASH_CSTR_TO_SDBM(name);
 
            HASH_FIND_ULONG(cf->sections, &sk, s);
            INA_ASSERT_NOTNULL(s);

            if (!named) {
                const char *rk = __INA_ATTR_DEFAULT;
                unsigned long rki = INA_HASH_CSTR_TO_SDBM(rk);

                res = (ina_conffile_section_res_t*)ina_mempool_dalloc(
                                        cf->mempool,
                                        sizeof(ina_conffile_section_res_t));
                INA_ASSERT_NOTNULL(res);
                res->id = rki;
                res->key = ina_str_new_fromcstr_using_pool(rk, cf->mempool);
                res->entries = NULL;

                HASH_ADD_ULONG(s->results, id, res);

                lua_getfield(lstate, -1 , __INA_ENUM_KEYS);
                __ina_process_entries(cf, res);
            } else {
                lua_getfield(lstate, -1 , __INA_ENUM_CHILDREN);
                lua_pushnil(lstate);
                while(lua_next(lstate, -2)) {
                    const char *rk;
                    unsigned long rki;

                    rk = lua_tostring(lstate, -2);
                    rki = INA_HASH_CSTR_TO_SDBM(rk);

                    res = (ina_conffile_section_res_t*)ina_mempool_dalloc(
                                        cf->mempool,
				                        sizeof(ina_conffile_section_res_t));
                    INA_ASSERT_NOTNULL(res);
                    res->id = rki;
                    res->key = ina_str_new_fromcstr_using_pool(rk, cf->mempool);
                    res->entries = NULL;

                    HASH_ADD_ULONG(s->results, id, res);
                    __ina_process_entries(cf, res);
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
__ina_process_entries(ina_conffile_t *cf, ina_conffile_section_res_t *res)
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
            entry->id = INA_HASH_CSTR_TO_SDBM(k);
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
            HASH_ADD_ULONG(res->entries, id, entry);
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
    ina_conffile_section_res_t *entries = NULL;
    ina_conffile_entry_t *e = NULL;
    unsigned long k = INA_HASH_CSTR_TO_SDBM(section_name);

    /* First section lookup */
    HASH_FIND_ULONG(cf->sections, &k, section);
    if (section == NULL) {
        return INA_FAILURE;
    }
    
    /* Second section lockup for named sections */
    if (section->named) {
        k = INA_HASH_CSTR_TO_SDBM(section_key);
        HASH_FIND_ULONG(section->results, &k, entries);
    } else {
        entries = section->results;
    }
    if (entries == NULL || entries->entries == NULL) {
        return INA_FAILURE;
    }

    /* Lookup value */
    k = INA_HASH_CSTR_TO_SDBM(key);
    HASH_FIND_ULONG(entries->entries, &k, e);
    if (e == NULL) {
        return INA_FAILURE;
    }
    *entry = e;
    return INA_SUCCESS;
}
