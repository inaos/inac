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

static int __section_count = 0;
static int __named_section_count = 0;
 
static ina_rc_t __ina_section_handler(ina_conffile_entry_t *entries)
{
    __section_count++;
    return INA_SUCCESS;
}

static ina_rc_t __ina_named_section_handler(const char *name, ina_conffile_entry_t *entries)
{
    __named_section_count++;
    return INA_SUCCESS;
}

void test_conffile_process_without_filepath()
{
    ina_conffile_t *cf = NULL;
    ina_conffile_section_t *cs = NULL;
    
    INA_ASSERT_SUCCEED(ina_conffile_init(&cf, NULL));
    INA_TRACE("filepath=%s", ina_str_cstr(cf->filepath));
    INA_ASSERT_EQUAL(0, strcmp(ina_str_cstr(cf->filepath), "test.conf"));
    
    INA_ASSERT_SUCCEED(ina_conffile_add_section(cf, "Debug", INA_YES, __ina_section_handler, &cs));
    INA_ASSERT_NOTNULL(cs);
    INA_ASSERT_SUCCEED(ina_conffile_add_key(cs, "command_latency", INA_CONFFILE_VALUE_TYPE_NUMBER, INA_YES));
    INA_ASSERT_SUCCEED(ina_conffile_add_key(cs, "other_latency", INA_CONFFILE_VALUE_TYPE_NUMBER, INA_NO));
    
    cs = NULL;
    INA_ASSERT_SUCCEED(ina_conffile_add_named_section(cf, "Iface", INA_YES, __ina_named_section_handler, &cs));
    INA_ASSERT_NOTNULL(cs);
    INA_ASSERT_SUCCEED(ina_conffile_add_key(cs, "ip", INA_CONFFILE_VALUE_TYPE_STRING, INA_YES));
    INA_ASSERT_SUCCEED(ina_conffile_add_key(cs, "mask", INA_CONFFILE_VALUE_TYPE_STRING, INA_YES));
    
    INA_ASSERT_SUCCEED(ina_conffile_process(cf));
}
void test_conffile_init_destroy()
{
    ina_conffile_t *cf = NULL;
    INA_ASSERT_SUCCEED(ina_conffile_init(&cf, NULL));
    INA_ASSERT_NOTNULL(cf);
    INA_ASSERT_SUCCEED(ina_conffile_destroy(&cf));
    INA_ASSERT_NULL(cf);
}

