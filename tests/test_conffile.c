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
 
static ina_rc_t __ina_section_handler(const char* section_name, const char* section_key, ina_conffile_entry_t *entries)
{
    double command_latency = 0;
    INA_TEST_ASSERT_NOT_NULL(section_name);
    INA_TEST_ASSERT_NULL(section_key);
    INA_TEST_ASSERT_NOT_NULL(entries);
    INA_TEST_ASSERT_SUCCEED(ina_conffile_has_value_in_entries(entries, "command_latency"));
    INA_TEST_ASSERT_NOTSUCCEED(ina_conffile_has_value_in_entries(entries, "other_latency"));
    INA_TEST_ASSERT_SUCCEED(ina_conffile_get_number_from_entries(entries, "command_latency", &command_latency));
    INA_TEST_ASSERT_EQUAL(1000.0, command_latency);
    __section_count++;
    return INA_SUCCESS;
}

static ina_rc_t __ina_named_section_handler(const char *section_name, const char* section_key, ina_conffile_entry_t *entries)
{
    INA_TEST_ASSERT_SUCCEED(ina_conffile_has_value_in_entries(entries, "ip"));
    INA_TEST_ASSERT_SUCCEED(ina_conffile_has_value_in_entries(entries, "mask"));

    INA_TEST_ASSERT_NOT_NULL(section_name);
    INA_TEST_ASSERT_NOT_NULL(section_key);
    INA_TEST_ASSERT_NOT_NULL(entries);
    __named_section_count++;
    return INA_SUCCESS;
}
INA_TEST(conffile , using_macros_autodestroy)
{
    INA_CONFFILE(NULL,
        INA_CONFFILE_SECTION("debug", INA_YES, __ina_section_handler,
            INA_CONFFILE_NUMBER_KEY("command_latency", INA_YES),
            INA_CONFFILE_NUMBER_KEY("other_latency", INA_NO)),
        INA_CONFFILE_NAMED_SECTION("iface", INA_YES, __ina_named_section_handler,
            INA_CONFFILE_STRING_KEY("ip", INA_YES),
            INA_CONFFILE_STRING_KEY("mask", INA_YES)));

    INA_TEST_ASSERT_EQUAL(2, __section_count);
    INA_TEST_ASSERT_EQUAL(4, __named_section_count);
}

INA_TEST(conffile, process_without_filepath)
{
    ina_conffile_t *cf = NULL;
    ina_conffile_section_t *cs = NULL;

    __section_count = 0;
    __named_section_count = 0;
    
    INA_TEST_ASSERT_SUCCEED(ina_conffile_init(&cf));
 
    INA_TEST_ASSERT_SUCCEED(ina_conffile_add_section(cf, "debug", INA_YES, INA_NO, __ina_section_handler, &cs));
    INA_TEST_ASSERT_NOT_NULL(cs);
    INA_TEST_ASSERT_SUCCEED(ina_conffile_add_key(cs, "command_latency", INA_CONFFILE_VALUE_TYPE_NUMBER, INA_YES));
    INA_TEST_ASSERT_SUCCEED(ina_conffile_add_key(cs, "other_latency", INA_CONFFILE_VALUE_TYPE_NUMBER, INA_NO));
    
    cs = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_conffile_add_section(cf, "iface", INA_YES, INA_YES, __ina_named_section_handler, &cs));
    INA_TEST_ASSERT_NOT_NULL(cs);
    INA_TEST_ASSERT_SUCCEED(ina_conffile_add_key(cs, "ip", INA_CONFFILE_VALUE_TYPE_STRING, INA_YES));
    INA_TEST_ASSERT_SUCCEED(ina_conffile_add_key(cs, "mask", INA_CONFFILE_VALUE_TYPE_STRING, INA_YES));
    
    __section_count = 0;
    __named_section_count = 0;
    
    INA_TEST_ASSERT_SUCCEED(ina_conffile_process(cf, NULL));
    INA_TEST_ASSERT_EQUAL(0, strcmp(ina_str_cstr(cf->filepath), "test.conf"));
    INA_TEST_ASSERT_EQUAL(1, __section_count);
    INA_TEST_ASSERT_EQUAL(2, __named_section_count);
    
    INA_TEST_ASSERT_SUCCEED(ina_conffile_destroy(&cf));
    INA_TEST_ASSERT_NULL(cf);

}
INA_TEST(conffile, init_destroy)
{
    ina_conffile_t *cf = NULL;

    INA_TEST_ASSERT_SUCCEED(ina_conffile_init(&cf));
    INA_TEST_ASSERT_NOT_NULL(cf);
    INA_TEST_ASSERT_SUCCEED(ina_conffile_destroy(&cf));
    INA_TEST_ASSERT_NULL(cf);
}

