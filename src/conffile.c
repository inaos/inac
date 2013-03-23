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

#include <lauxlib.h>
#include <lualib.h>

struct ina_conffile_entry_s {
	uint32_t id;
	char *key;
	char *sv;
	double nv;
	UT_hash_handle hh;
};

typedef struct ina_conffile_section_key_s {
	uint32_t id;
	ina_str_t name;
	int required;
	ina_conffile_value_type_t valuetype;
	UT_hash_handle hh;
} ina_conffile_section_key_t;

typedef struct ina_conffile_section_res_s {
	uint32_t id;
	ina_str_t name;
	ina_conffile_entry_t *entries;
	UT_hash_handle hh;
} ina_conffile_section_res_t;

struct ina_conffile_section_s {
	unsigned long id;
	ina_str_t name;
	int required;
	int named;
	int configured;
	ina_conffile_section_key_t *keys;
	section_callback section_cb;
	named_section_callback named_section_cb;
	ina_conffile_section_res_t *results;
	UT_hash_handle hh;
};

static lua_State *__ina_conffile_lstate = NULL;
static ina_conffile_section_t *__ina_conffile_section_head;
static int __ina_conffile_inited = 0;

INA_API(ina_rc_t) ina_conffile_init(void)
{
	if (__ina_conffile_inited) {
		return INA_FAILURE;
	}

	__ina_conffile_lstate = luaL_newstate();
    luaL_openlibs(__ina_conffile_lstate);

	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_conffile_destroy(void)
{
	if (!__ina_conffile_inited) {
		return INA_FAILURE;
	}

	lua_close(__ina_conffile_lstate);

	return INA_SUCCESS;
}

static ina_rc_t __ina_conffile_add_internal(const char *name, int required, 
	section_callback cb1, named_section_callback cb2, ina_conffile_section_t **section, int named)
{
	uint32_t key;
	ina_conffile_section_t *sp;
	ina_conffile_section_t *check;

	key = INA_HASH_CSTR_TO_SDBM(name);
	HASH_FIND_ULONG(__ina_conffile_section_head, &key, check);
	if (check != NULL) {
		return INA_FAILURE;
	}

	*section = (ina_conffile_section_t*)ina_mem_alloc(sizeof(ina_conffile_section_t));
	sp = *section;

	sp->id = key;
	sp->name = ina_str_fromcstr(name);
	sp->named = named;
	sp->section_cb = cb1;
	sp->named_section_cb = cb2;
	sp->required = required;
	sp->results = NULL;
	sp->configured = 0;
	
	HASH_ADD_ULONG(__ina_conffile_section_head, id, sp);

	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_conffile_add_section(const char *name, int required, 
	section_callback cb, ina_conffile_section_t **section)
{
	if (!INA_SUCCEED(__ina_conffile_add_internal(name, required, cb, NULL, section, 0))) {
		return ina_err_peek();
	}

	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_conffile_add_named_section(const char *name, int required, 
	named_section_callback cb, ina_conffile_section_t **section)
{
	if (!INA_SUCCEED(__ina_conffile_add_internal(name, required, NULL, cb, section, 1))) {
		return ina_err_peek();
	}

	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_conffile_add_key(ina_conffile_section_t *section, const char *name, 
	ina_conffile_value_type_t value_type, int required)
{
	ina_conffile_section_key_t *key;

	INA_ASSERT_NOTNULL(section);
	INA_ASSERT_NOTNULL(name);
	
	key = (ina_conffile_section_key_t*)ina_mem_alloc(sizeof(ina_conffile_section_key_t));
	key->id = INA_HASH_CSTR_TO_SDBM(name);
	key->name = ina_str_fromcstr(name);
	key->required = required;
	key->valuetype = value_type;

	HASH_ADD_ULONG(section->keys, id, key);

	return INA_SUCCESS;
}

static ina_rc_t __ina_conffile_construct_section_table()
{
	ina_conffile_section_t *s, *stmp;
	ina_conffile_section_key_t *k, *ktmp;

	lua_getglobal(__ina_conffile_lstate, "sections");

	HASH_ITER(hh, __ina_conffile_section_head, s, stmp) {

		lua_pushstring(__ina_conffile_lstate, s->name);
		lua_newtable(__ina_conffile_lstate);

		/* set name */
		lua_pushstring(__ina_conffile_lstate, "name");
		lua_pushstring(__ina_conffile_lstate, s->name);
		lua_rawset(__ina_conffile_lstate, -3);

		/* set named */
		lua_pushstring(__ina_conffile_lstate, "named");
		lua_pushboolean(__ina_conffile_lstate, s->named);
		lua_rawset(__ina_conffile_lstate, -3);

		/* set required */
		lua_pushstring(__ina_conffile_lstate, "named");
		lua_pushboolean(__ina_conffile_lstate, s->named);
		lua_rawset(__ina_conffile_lstate, -3);

		/* set configured */
		lua_pushstring(__ina_conffile_lstate, "configured");
		lua_pushboolean(__ina_conffile_lstate, s->configured);
		lua_rawset(__ina_conffile_lstate, -3);

		/* set keys */
		lua_pushstring(__ina_conffile_lstate, "keys");
		lua_newtable(__ina_conffile_lstate);
		
		HASH_ITER(hh, s->keys, k, ktmp) {
			lua_pushstring(__ina_conffile_lstate, k->name);
			lua_newtable(__ina_conffile_lstate);

			lua_pushstring(__ina_conffile_lstate, "required");
			lua_pushboolean(__ina_conffile_lstate, k->required);
			lua_rawset(__ina_conffile_lstate, -3);

			lua_pushstring(__ina_conffile_lstate, "typename");
			if (k->valuetype == INA_CONFFILE_VALUE_TYPE_STRING) {
				lua_pushstring(__ina_conffile_lstate, "string");
			}
			else {
				lua_pushstring(__ina_conffile_lstate, "number");
			}
			lua_rawset(__ina_conffile_lstate, -3);

			lua_rawset(__ina_conffile_lstate, -3);
		}
		lua_rawset(__ina_conffile_lstate, -3);

		lua_rawset(__ina_conffile_lstate, -3);
	}
	return INA_SUCCESS;
}

static ina_rc_t __ina_conffile_process_entries(ina_conffile_section_res_t *res)
{
	lua_pushnil(__ina_conffile_lstate);
	while(lua_next(__ina_conffile_lstate, -2)) {
		int has_value;
		const char *k = lua_tostring(__ina_conffile_lstate, -2);
		const char *tn;
		ina_conffile_entry_t *entry;
					
		lua_getfield(__ina_conffile_lstate, -1 , "typename");
		tn = lua_tostring(__ina_conffile_lstate, -1);

		lua_getfield(__ina_conffile_lstate, -1 , "has_value");
		has_value = lua_toboolean(__ina_conffile_lstate, -1);
		if (has_value) {
			entry = (ina_conffile_entry_t*)ina_mem_alloc(sizeof(ina_conffile_entry_t));
			entry->id = INA_HASH_CSTR_TO_SDBM(k);
			entry->key = ina_str_fromcstr(k);
			lua_getfield(__ina_conffile_lstate, -1 , "value");
			if (strcmp(tn, "string") == 0) {
				strcpy(entry->sv, lua_tostring(__ina_conffile_lstate, -1));
				entry->nv = 0;
			}
			else {
				entry->sv = NULL;
				entry->nv = lua_tonumber(__ina_conffile_lstate, -1);
			}
			HASH_ADD_ULONG(res->entries, id, entry);
		}
		lua_pop(__ina_conffile_lstate, 1);
	}
	lua_pop(__ina_conffile_lstate, 1);

	return INA_SUCCESS;
}

static ina_rc_t __ina_conffile_process_section_table()
{
	lua_getglobal(__ina_conffile_lstate, "sections");
	lua_pushnil(__ina_conffile_lstate);

	while(lua_next(__ina_conffile_lstate, -2)) {
		const char *name;
		int named;
		int configured;

		lua_getfield(__ina_conffile_lstate, -1 , "name");
		name = lua_tostring(__ina_conffile_lstate, -1);

		lua_getfield(__ina_conffile_lstate, -1 , "named");
		named = lua_toboolean(__ina_conffile_lstate, -1);

		lua_getfield(__ina_conffile_lstate, -1 , "configured");
		configured = lua_toboolean(__ina_conffile_lstate, -1);

		if (configured) {
			ina_conffile_section_t *s;
			ina_conffile_section_res_t *res;
			uint32_t sk = INA_HASH_CSTR_TO_SDBM(name);

			HASH_FIND_ULONG(__ina_conffile_section_head, &sk, s);
			INA_ASSERT_NOTNULL(s);

			if (!named) {
				const char *rk = "default";
				uint32_t rki = INA_HASH_CSTR_TO_SDBM(rk);

				res = (ina_conffile_section_res_t*)ina_mem_alloc(sizeof(ina_conffile_section_res_t));
				res->id = rki;
				res->name = ina_str_fromcstr(rk);
				res->entries = NULL;

				HASH_ADD_ULONG(s->results, id, res);

				lua_getfield(__ina_conffile_lstate, -1 , "keys");
				__ina_conffile_process_entries(res);
			}
			else {
				lua_getfield(__ina_conffile_lstate, -1 , "children");
				lua_pushnil(__ina_conffile_lstate);
				while(lua_next(__ina_conffile_lstate, -2)) {
					const char *rk;
					uint32_t rki;

					rk = lua_tostring(__ina_conffile_lstate, -2);
					rki = INA_HASH_CSTR_TO_SDBM(rk);

					res = (ina_conffile_section_res_t*)ina_mem_alloc(sizeof(ina_conffile_section_res_t));
					res->id = rki;
					res->name = ina_str_fromcstr(rk);
					res->entries = NULL;

					HASH_ADD_ULONG(s->results, id, res);

					__ina_conffile_process_entries(res);

					lua_pop(__ina_conffile_lstate, 1);
				}
				lua_pop(__ina_conffile_lstate, 1);
			}
		}
		lua_pop(__ina_conffile_lstate, 1);
	}

	lua_pop(__ina_conffile_lstate, 1);

	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_conffile_process(int pos, char **argv)
{
	ina_str_t conf_file_path;
	int ret;
	ina_conffile_section_t *s, *stmp;

	INA_ASSERT_TRUE(__ina_conffile_inited);
	INA_ASSERT_NOTNULL(__ina_conffile_section_head);

	/* 
	 * by convention the conf-file path is [binary-name].conf 
	 * in the current workdir if nothing else is specified
	 */
	if (argv[pos] == NULL) {
		conf_file_path = ina_str_vsprintf("%s.conf", argv[0]);
	}
	else {
		conf_file_path = ina_str_fromcstr(argv[pos]);
	}

	/* set the config-file path */
	lua_pushstring(__ina_conffile_lstate, ina_str_cstr(conf_file_path));
	lua_setglobal(__ina_conffile_lstate, "conf_file_path");

	/* create the sections table */
	lua_newtable(__ina_conffile_lstate);
	lua_setglobal(__ina_conffile_lstate, "sections");

	/* construct sections table */
	if (!INA_SUCCEED(__ina_conffile_construct_section_table())) {
		return INA_FAILURE;
	}

	/* invoke lua */
	ret = luaL_dostring(__ina_conffile_lstate, "local cf = require(\"conffile\")\n cf.process(sections, conf_file_path)\n");
	if (ret != 0) {
        return INA_FAILURE;
    }

	/* process section table */
	if (!INA_SUCCEED(__ina_conffile_process_section_table())) {
		return INA_FAILURE;
	}

	/* invoke callbacks */
	HASH_ITER(hh, __ina_conffile_section_head, s, stmp) {
		if (!s->named) {
			s->section_cb(s->results->entries);
		}
		else {
			ina_conffile_section_res_t *r, *rtmp;
			HASH_ITER(hh, s->results, r, rtmp) {
				s->named_section_cb(r->name, r->entries);
			}
		}
	}

	return INA_SUCCESS;
}
