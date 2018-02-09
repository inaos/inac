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
    INA_TEST_ASSERT_EQUAL_INTEGER(INA_RC_PACK(INA_ERR_FAILED),
                                  ina_err_set_last_rc(INA_RC_PACK(INA_ERR_FAILED)));
    INA_TEST_ASSERT_EQUAL_INTEGER(INA_ERROR(INA_ERR_NOT_INITIALIZED),
                                  ina_err_set_last_rc(INA_RC_PACK(INA_ERR_NOT_INITIALIZED)));
}

INA_TEST(error, clear)
{
    INA_TEST_ASSERT_FAILED(INA_ERROR(INA_ERR_NOT_INITIALIZED));
    INA_TEST_ASSERT_FAILED(ina_err_get_last_rc());
    INA_TEST_ASSERT_SUCCEED(ina_err_clear_last_rc());
    INA_TEST_ASSERT_SUCCEED(ina_err_get_last_rc());

}
INA_TEST(error, strerror)
{
    char msg[INA_ERR_MSGLEN];

    INA_TEST_ASSERT_SUCCEED(ina_err_clear_last_rc());
    INA_TEST_ASSERT_SUCCEED(ina_err_get_last_rc());
    INA_ERROR(INA_NN_DEVICE|INA_ERR_IN_USE);
    INA_TEST_MSG("%s", ina_err_strerror(ina_err_get_last_rc(), msg));
}

INA_TEST(error, error_pack_rc) 
{
    ina_rc_t rcc;
    ina_rc_t rc;

    rcc = 2147483652 ;
    rc = 0;
    rc = INA_RC_PACK(INA_NN_ACCESS|INA_ERR_NOT_ALLOWED);
    
    INA_TRACE3("rc = %u", rc);
    INA_TRACE3("reason = %u", INA_RC_A(rc));
    
    INA_TEST_ASSERT_EQUAL_INTEGER(rcc, rc);
    INA_TEST_ASSERT_EQUAL_INTEGER(4 , INA_RC_A(rc));
    INA_TEST_ASSERT_EQUAL_INTEGER(2147483652, ina_err_set_last_rc(rc));
    INA_TEST_ASSERT_EQUAL_INTEGER(4,   ina_err_get_last_rc());

}
