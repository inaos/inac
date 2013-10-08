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
#include <stdio.h>
#include <libinac/lib.h>
 
#define _INA_TEMPLATE_TEST_TPL "$(foreach funcs " \
    "${type} x = ${name}( ${table.concat(args, ', ')} ) {" \
        "$(code)" \
        "$(when stuff x = $x; y = $y;) " \
        "reutrn $(exit);" \
    "})"

#define _INA_TEMPLATE_TEST_EXPECTED "int x = bill( a, b, c ) " \
    "{somethingx = 99; y = 34; reutrn 1;}char * x = bert( one, " \
    "two, three ) {something else reutrn 2;}"

INA_TEST(template, simple)
{
    ina_str_t tpl;
    ina_template_ctx_t *ctx;
    ina_template_env_t *env;
    ina_template_table_t *tbl;
    ina_template_table_t *funcs;
    ina_template_table_t *arr1;
    ina_template_table_t *nestarr;
    ina_template_table_t *nestarr2;
    ina_template_table_t *nest1;
    ina_template_table_t *args1;
    ina_template_table_t *args2;
    ina_template_table_t *stuff1;
    ina_str_t test1;
    ina_str_t out;

    INA_TEST_ASSERT_SUCCEED(ina_template_init(&ctx));

    test1 = ina_str_fromcstr("dasfsda");
    tpl = ina_str_fromcstr(_INA_TEMPLATE_TEST_TPL);

    INA_TEST_ASSERT_SUCCEED(ina_template_init(&ctx));

    INA_TEST_ASSERT_SUCCEED(ina_template_compile(ctx, "test1", tpl, &env));
    
    INA_TEST_ASSERT_SUCCEED(ina_template_set_number(env, "exit", 1));
    INA_TEST_ASSERT_SUCCEED(ina_template_set_boolean(env, "stuff", 0));
    INA_TEST_ASSERT_SUCCEED(ina_template_set_string(env, "test_str", test1));
    
    INA_TEST_ASSERT_SUCCEED(ina_template_new_hash(env, "garb", &tbl));
    INA_TEST_ASSERT_SUCCEED(ina_template_new_hash(env, "funcs", &funcs));
    
    INA_TEST_ASSERT_SUCCEED(ina_template_hash_set_number(tbl, "anum", 10));
    INA_TEST_ASSERT_SUCCEED(ina_template_hash_set_boolean(tbl, "abool", 1));
    INA_TEST_ASSERT_SUCCEED(ina_template_new_array(env, "testArr", &arr1));
    INA_TEST_ASSERT_SUCCEED(ina_template_array_set_number(arr1, 1, 10));
    INA_TEST_ASSERT_SUCCEED(ina_template_array_set_boolean(arr1, 2, 1));
    INA_TEST_ASSERT_SUCCEED(ina_template_array_set_string(arr1, 3, "billybob"));
    INA_TEST_ASSERT_SUCCEED(ina_template_table_new_nested_hash(tbl, "nested1", &nest1));
    INA_TEST_ASSERT_SUCCEED(ina_template_hash_set_string(nest1, "foo", "bar"));

    INA_TEST_ASSERT_SUCCEED(ina_template_table_new_nested_array(funcs, 1, &nestarr));
    INA_TEST_ASSERT_SUCCEED(ina_template_hash_set_string(nestarr, "type", "int"));
    INA_TEST_ASSERT_SUCCEED(ina_template_hash_set_string(nestarr, "name", "bill"));
    INA_TEST_ASSERT_SUCCEED(ina_template_table_new_nested_hash(nestarr, "args", &args1));
    INA_TEST_ASSERT_SUCCEED(ina_template_array_set_string(args1, 1, "a"));
    INA_TEST_ASSERT_SUCCEED(ina_template_array_set_string(args1, 2, "b"));
    INA_TEST_ASSERT_SUCCEED(ina_template_array_set_string(args1, 3, "c"));
    INA_TEST_ASSERT_SUCCEED(ina_template_hash_set_string(nestarr, "code", "something"));
    INA_TEST_ASSERT_SUCCEED(ina_template_table_new_nested_hash(nestarr, "stuff", &stuff1));
    INA_TEST_ASSERT_SUCCEED(ina_template_hash_set_number(stuff1, "x", 99));
    INA_TEST_ASSERT_SUCCEED(ina_template_hash_set_number(stuff1, "y", 34));

    INA_TEST_ASSERT_SUCCEED(ina_template_table_new_nested_array(funcs, 2, &nestarr2));
    INA_TEST_ASSERT_SUCCEED(ina_template_hash_set_string(nestarr2, "type", "char *"));
    INA_TEST_ASSERT_SUCCEED(ina_template_hash_set_string(nestarr2, "name", "bert"));
    INA_TEST_ASSERT_SUCCEED(ina_template_table_new_nested_hash(nestarr2, "args", &args2));
    INA_TEST_ASSERT_SUCCEED(ina_template_array_set_string(args2, 1, "one"));
    INA_TEST_ASSERT_SUCCEED(ina_template_array_set_string(args2, 2, "two"));
    INA_TEST_ASSERT_SUCCEED(ina_template_array_set_string(args2, 3, "three"));
    INA_TEST_ASSERT_SUCCEED(ina_template_hash_set_string(nestarr2, "code", "something else"));
    INA_TEST_ASSERT_SUCCEED(ina_template_hash_set_number(nestarr2, "exit", 2));

    INA_TEST_ASSERT_SUCCEED(ina_template_render(env, &out));

    INA_TEST_ASSERT_EQUAL_STR(_INA_TEMPLATE_TEST_EXPECTED, ina_str_cstr(out));
    ina_str_destroy(out);

    INA_TEST_ASSERT_SUCCEED(ina_template_destroy(&ctx));

    INA_TEST_ASSERT_SUCCEED(ina_template_destroy(&ctx));
}
