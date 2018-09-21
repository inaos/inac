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

static int __section_count = 0;
static int __named_section_count = 0;
 
static ina_rc_t __ina_section_handler(const char* section_name, const char* section_key, ina_conffile_entries_t *entries, void* user_data)
{
    INA_UNUSED(user_data);
    double command_latency = 0;
    INA_TEST_ASSERT_NOT_NULL(section_name);
    INA_TEST_ASSERT_NULL(section_key);
    INA_TEST_ASSERT_NOT_NULL(entries);
    INA_TEST_ASSERT_SUCCEED(ina_conffile_has_value_in_entries(entries, "command_latency"));
    INA_TEST_ASSERT_FAILED(ina_conffile_has_value_in_entries(entries, "other_latency"));
    INA_TEST_ASSERT_SUCCEED(ina_conffile_get_number_from_entries(entries, "command_latency", &command_latency));
    INA_TEST_ASSERT_EQUAL_FLOATING(1000.0, command_latency);
    __section_count++;
    return INA_SUCCESS;
}

static ina_rc_t __ina_named_section_handler(const char *section_name, const char* section_key, ina_conffile_entries_t *entries, void* user_data)
{
    INA_UNUSED(user_data);
    INA_TEST_ASSERT_SUCCEED(ina_conffile_has_value_in_entries(entries, "ip"));
    INA_TEST_ASSERT_SUCCEED(ina_conffile_has_value_in_entries(entries, "mask"));

    INA_TEST_ASSERT_NOT_NULL(section_name);
    INA_TEST_ASSERT_NOT_NULL(section_key);
    INA_TEST_ASSERT_NOT_NULL(entries);
    __named_section_count++;
    return INA_SUCCESS;
}

INA_TEST(conffile , using_macros_with_filepath)
{
    ina_conffile_t *cf = NULL;

    __section_count = 0;
    __named_section_count = 0;       
    
    INA_CONFFILE(cf, "test_filepath.conf", NULL,
        INA_CONFFILE_SECTION("debug", INA_YES, __ina_section_handler,
            INA_CONFFILE_NUMBER_KEY("command_latency", INA_YES),
            INA_CONFFILE_NUMBER_KEY("other_latency", INA_NO),
            INA_CONFFILE_STRING_KEY("command_name", INA_NO)),
        INA_CONFFILE_NAMED_SECTION("iface", INA_YES, __ina_named_section_handler,
            INA_CONFFILE_STRING_KEY("ip", INA_YES),
            INA_CONFFILE_STRING_KEY("mask", INA_YES)));

    INA_TEST_ASSERT_EQUAL_FLOATING(1, __section_count);
    INA_TEST_ASSERT_EQUAL_FLOATING(2, __named_section_count);
}

INA_TEST(conffile , using_macros)
{
    ina_conffile_t *cf = NULL;

    __section_count = 0;
    __named_section_count = 0;   

    INA_CONFFILE(cf, NULL, NULL,
        INA_CONFFILE_SECTION("debug", INA_YES, __ina_section_handler,
            INA_CONFFILE_NUMBER_KEY("command_latency", INA_YES),
            INA_CONFFILE_NUMBER_KEY("other_latency", INA_NO),
            INA_CONFFILE_STRING_KEY("command_name", INA_YES)),
        INA_CONFFILE_NAMED_SECTION("iface", INA_YES, __ina_named_section_handler,
            INA_CONFFILE_STRING_KEY("ip", INA_YES),
            INA_CONFFILE_STRING_KEY("mask", INA_YES)));
    INA_TEST_ASSERT_NOT_NULL(cf);
    INA_TEST_ASSERT_EQUAL_INT(1, __section_count);
    INA_TEST_ASSERT_EQUAL_INT(2, __named_section_count);
}

INA_TEST(conffile , using_macros_without_section_handler)
{
    ina_conffile_t *cf = NULL;
    ina_str_t value = NULL;
    double dbl_value = 0.0;

    INA_CONFFILE(cf, NULL, NULL,
        INA_CONFFILE_SECTION("debug", INA_YES, NULL,
            INA_CONFFILE_NUMBER_KEY("command_latency", INA_YES),
            INA_CONFFILE_NUMBER_KEY("other_latency", INA_NO),
            INA_CONFFILE_STRING_KEY("command_name", INA_YES)),
        INA_CONFFILE_NAMED_SECTION("iface", INA_YES, NULL,
            INA_CONFFILE_STRING_KEY("ip", INA_YES),
            INA_CONFFILE_STRING_KEY("mask", INA_YES)));
    INA_TEST_ASSERT_NOT_NULL(cf);

    INA_TEST_ASSERT_SUCCEED(ina_conffile_get_number(cf, "debug", NULL, "command_latency", &dbl_value));
    INA_TEST_ASSERT_EQUAL_FLOATING(1000.0, dbl_value);

    INA_TEST_ASSERT_SUCCEED(ina_conffile_get_string(cf, "debug", NULL, "command_name", &value));
    INA_TEST_ASSERT_NOT_NULL(value);
    INA_TEST_ASSERT_EQUAL_STR("my_cmd", ina_str_cstr(value));

    INA_TEST_ASSERT_SUCCEED(ina_conffile_get_string(cf, "iface", "lo0", "ip", &value));
    INA_TEST_ASSERT_NOT_NULL(value);
    INA_TEST_ASSERT_EQUAL_STR("127.0.0.1", ina_str_cstr(value));
    value = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_conffile_get_string(cf, "iface", "lo0", "mask", &value));
    INA_TEST_ASSERT_NOT_NULL(value);
    INA_TEST_ASSERT_EQUAL_STR("255.0.0.0", ina_str_cstr(value));
    value = NULL;    
    INA_TEST_ASSERT_SUCCEED(ina_conffile_get_string(cf, "iface", "lo1", "ip", &value));
    INA_TEST_ASSERT_NOT_NULL(value);
    INA_TEST_ASSERT_EQUAL_STR("127.0.0.2", ina_str_cstr(value));
    value = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_conffile_get_string(cf, "iface", "lo1", "mask", &value));
    INA_TEST_ASSERT_NOT_NULL(value);
    INA_TEST_ASSERT_EQUAL_STR("255.0.0.1", ina_str_cstr(value));
    value = NULL;
    ina_conffile_free(&cf);
}


INA_TEST(conffile, try_anonymous_section)
{
    ina_conffile_t *cf = NULL;
    ina_conffile_section_t *cs = NULL;

    INA_TEST_ASSERT_SUCCEED(ina_conffile_new(&cf));
 
    INA_TEST_ASSERT_SUCCEED(ina_conffile_add_section(cf, "debug", 
                                INA_YES, INA_NO, 
                                __ina_section_handler, &cs));
    INA_TEST_ASSERT_NOT_NULL(cs);
    INA_TEST_ASSERT_SUCCEED(ina_conffile_add_key(cs, "command_latency", 
                                INA_CONFFILE_VALUE_TYPE_NUMBER, INA_YES));
    INA_TEST_ASSERT_SUCCEED(ina_conffile_add_key(cs, "other_latency", 
                                INA_CONFFILE_VALUE_TYPE_NUMBER, INA_NO));
    
    INA_TEST_ASSERT_SUCCEED(ina_conffile_add_key(cs, "command_name", 
                                INA_CONFFILE_VALUE_TYPE_STRING, INA_NO));

    cs = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_conffile_add_section(cf, "iface", 
                                INA_YES, INA_YES, 
                                __ina_named_section_handler, 
                                &cs));
    INA_TEST_ASSERT_NOT_NULL(cs);
    INA_TEST_ASSERT_SUCCEED(ina_conffile_add_key(cs, "ip", 
                                INA_CONFFILE_VALUE_TYPE_STRING, INA_YES));
    INA_TEST_ASSERT_SUCCEED(ina_conffile_add_key(cs, "mask", 
                                INA_CONFFILE_VALUE_TYPE_STRING, INA_YES));

    INA_TEST_ASSERT_FAILED(ina_conffile_process(cf,
                                "test_conffile_anonymous_section.conf", NULL));
}


INA_TEST(conffile, process_with_filepath)
{
    ina_conffile_t *cf = NULL;
    ina_conffile_section_t *cs = NULL;

    __section_count = 0;
    __named_section_count = 0;
    
    INA_TEST_ASSERT_SUCCEED(ina_conffile_new(&cf));
 
    INA_TEST_ASSERT_SUCCEED(ina_conffile_add_section(cf, "debug", INA_YES, INA_NO, __ina_section_handler, &cs));
    INA_TEST_ASSERT_NOT_NULL(cs);
    INA_TEST_ASSERT_SUCCEED(ina_conffile_add_key(cs, "command_latency", INA_CONFFILE_VALUE_TYPE_NUMBER, INA_YES));
    INA_TEST_ASSERT_SUCCEED(ina_conffile_add_key(cs, "other_latency", INA_CONFFILE_VALUE_TYPE_NUMBER, INA_NO));
    INA_TEST_ASSERT_SUCCEED(ina_conffile_add_key(cs, "command_name", INA_CONFFILE_VALUE_TYPE_STRING, INA_NO));
    cs = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_conffile_add_section(cf, "iface", INA_YES, INA_YES, __ina_named_section_handler, &cs));
    INA_TEST_ASSERT_NOT_NULL(cs);
    INA_TEST_ASSERT_SUCCEED(ina_conffile_add_key(cs, "ip", INA_CONFFILE_VALUE_TYPE_STRING, INA_YES));
    INA_TEST_ASSERT_SUCCEED(ina_conffile_add_key(cs, "mask", INA_CONFFILE_VALUE_TYPE_STRING, INA_YES));
    
    __section_count = 0;
    __named_section_count = 0;
    
    INA_TEST_ASSERT_SUCCEED(ina_conffile_process(cf, NULL, NULL));
    INA_TEST_ASSERT_EQUAL_FLOATING(1, __section_count);
    INA_TEST_ASSERT_EQUAL_FLOATING(2, __named_section_count);
    
    ina_conffile_free(&cf);
    INA_TEST_ASSERT_NULL(cf);

}

INA_TEST(conffile, duplicate_key)
{
    ina_conffile_t *cf = NULL;
    ina_conffile_section_t *cs = NULL;

    __section_count = 0;
    __named_section_count = 0;

    INA_TEST_ASSERT_SUCCEED(ina_conffile_new(&cf));

    INA_TEST_ASSERT_SUCCEED(ina_conffile_add_section(cf, "debug", INA_YES, INA_NO, __ina_section_handler, &cs));
    INA_TEST_ASSERT_NOT_NULL(cs);
    INA_TEST_ASSERT_SUCCEED(ina_conffile_add_key(cs, "command_latency", INA_CONFFILE_VALUE_TYPE_NUMBER, INA_YES));
    INA_TEST_ASSERT_FAILED(ina_conffile_add_key(cs, "command_latency", INA_CONFFILE_VALUE_TYPE_NUMBER, INA_YES));
    ina_conffile_free(&cf);
}


INA_TEST(conffile, process_without_filepath)
{
    ina_conffile_t *cf = NULL;
    ina_conffile_section_t *cs = NULL;

    __section_count = 0;
    __named_section_count = 0;
    
    INA_TEST_ASSERT_SUCCEED(ina_conffile_new(&cf));
 
    INA_TEST_ASSERT_SUCCEED(ina_conffile_add_section(cf, "debug", INA_YES, INA_NO, __ina_section_handler, &cs));
    INA_TEST_ASSERT_NOT_NULL(cs);
    INA_TEST_ASSERT_SUCCEED(ina_conffile_add_key(cs, "command_latency", INA_CONFFILE_VALUE_TYPE_NUMBER, INA_YES));
    INA_TEST_ASSERT_SUCCEED(ina_conffile_add_key(cs, "other_latency", INA_CONFFILE_VALUE_TYPE_NUMBER, INA_NO));
    INA_TEST_ASSERT_SUCCEED(ina_conffile_add_key(cs, "command_name", INA_CONFFILE_VALUE_TYPE_STRING, INA_NO));   
    cs = NULL;
    INA_TEST_ASSERT_SUCCEED(ina_conffile_add_section(cf, "iface", INA_YES, INA_YES, __ina_named_section_handler, &cs));
    INA_TEST_ASSERT_NOT_NULL(cs);
    INA_TEST_ASSERT_SUCCEED(ina_conffile_add_key(cs, "ip", INA_CONFFILE_VALUE_TYPE_STRING, INA_YES));
    INA_TEST_ASSERT_SUCCEED(ina_conffile_add_key(cs, "mask", INA_CONFFILE_VALUE_TYPE_STRING, INA_YES));
    
    __section_count = 0;
    __named_section_count = 0;
    
    INA_TEST_ASSERT_SUCCEED(ina_conffile_process(cf, NULL, NULL));
    INA_TEST_ASSERT_EQUAL_FLOATING(1, __section_count);
    INA_TEST_ASSERT_EQUAL_FLOATING(2, __named_section_count);
    
    ina_conffile_free(&cf);
    INA_TEST_ASSERT_NULL(cf);

}

INA_TEST(conffile, new_free)
{
    ina_conffile_t *cf = NULL;

    INA_TEST_ASSERT_SUCCEED(ina_conffile_new(&cf));
    INA_TEST_ASSERT_NOT_NULL(cf);
    ina_conffile_free(&cf);
    INA_TEST_ASSERT_NULL(cf);
}

