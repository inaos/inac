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
	unsigned long id;
	char *key;
	char *sv;
	double nv;
	UT_hash_handle hh;
};

typedef struct ina_conffile_section_res_s {
	unsigned long id;
	ina_str_t name;
	ina_conffile_entry_t *entries;
	UT_hash_handle hh;
} ina_conffile_section_res_t;

struct ina_conffile_section_s {
	unsigned long id;
	ina_str_t name;
	int required;
	int named;
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

INA_API(ina_rc_t) ina_conffile_add_section(const char *name, int required, 
	section_callback cb, ina_conffile_section_t **section)
{
	unsigned long key;
	ina_conffile_section_t *sp;

	

	*section = (ina_conffile_section_t*)ina_mem_alloc(sizeof(ina_conffile_section_t));
	sp = *section;

	

	return INA_SUCCESS;
}

