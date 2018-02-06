/*
 * Copyright (c) 2012-2018, INAOS GmbH
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

INA_TEST(error, get_set_rc)
{
    INA_TEST_ASSERT_EQUAL_INTEGER(INA_RC_PACK(INA_EINVAL),
                                  ina_err_set_rc(INA_RC_PACK(INA_EINVAL),
                                                 INA_ERR_AT));
    INA_TEST_ASSERT_EQUAL_INTEGER(INA_ERR(INA_EINVAL),
                                  ina_err_set_rc(INA_RC_PACK(INA_EINVAL),
                                                 INA_ERR_AT));
}

INA_TEST(error, message_formatting)
{
    ina_str_t msg1;
    ina_str_t msg2;

    msg1 = ina_str_new_fromcstr("Message size error");
    msg2 = ina_str_new(100);

    INA_TEST_ASSERT_NOT_NULL(msg1);
    INA_TEST_ASSERT_NOT_NULL(msg2);
    
    INA_TEST_ASSERT_SUCCESS(ina_err_reset());
    INA_TEST_ASSERT_SUCCESS(ina_err_get_last_rc());
    INA_ERR(INA_EAGAIN);
    INA_TEST_ASSERT_EQUAL_INTEGER(INA_SUCCESS, ina_err_fmtmsg(ina_err_get_last_rc(), msg2, 100));
    INA_TEST_MSG("msg2=%s", ina_str_cstr(msg2));
    ina_str_free(msg1);
    ina_str_free(msg2);
}

INA_TEST(error, get_errmsg)
{
    ina_rc_t rc;

    rc = INA_ERRMSG (10, "This is error 1", NULL);
    INA_TEST_ASSERT_EQUAL_STR("This is error 1", ina_err_get_msg(rc));
    INA_TEST_ASSERT_EQUAL_STR("This is error 1", ina_err_get_last_msg());
}


INA_TEST(error, error_pack_rc) 
{
    ina_rc_t rcc;
    ina_rc_t rc;

    rcc = 2147483652;
    rc = 0;
    rc = INA_RC_PACK((4|INA_ERR_FLAG_FATAL));
    
    INA_TRACE3("rc = %u", rc);
    INA_TRACE3("reason = %u", INA_RC_REASON(rc));
    
    INA_TEST_ASSERT_EQUAL_INTEGER(rcc, rc);
    INA_TEST_ASSERT_EQUAL_INTEGER(4 , INA_RC_REASON(rc));
    INA_TEST_ASSERT_EQUAL_INTEGER(2147483652, ina_err_set_rc(rc, ""));
    INA_TEST_ASSERT_EQUAL_INTEGER(4,   ina_err_get_last_error());
    INA_TEST_ASSERT_TRUE(INA_RC_FATAL(rc));
    
    rc = INA_RC_PACK(255);
    INA_TEST_ASSERT_EQUAL_INTEGER(255, INA_RC_REASON(rc));

    rc = INA_RC_PACK(511);
    INA_TEST_ASSERT_EQUAL_INTEGER(511, INA_RC_REASON(rc));
}
