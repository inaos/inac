/*
 * Copyright (c) 2012, INAOS GmbH
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

void test_error_pack_rc() 
{
    INA_TRACE("test_error_pack_rc");
    
    ina_rc_t rcc;
    ina_rc_t rc;
    
    rcc = 16850951;
    rc = 0;
    rc = INA_RC_PACK(1,2,7,4);
    
    printf("rc = %u\n", rc);
    printf("id = %u\n", INA_RC_ID(rc));
    printf("mod = %u\n", INA_RC_MOD(rc));
    printf("func = %u\n", INA_RC_OSFN(rc));
    printf("reason = %u\n", INA_RC_REASON(rc));
    
    /*INA_ASSERT_EQUAL(rcc, rc); */
    INA_ASSERT_EQUAL(1, INA_RC_MOD(rc));
    INA_ASSERT_EQUAL(2, INA_RC_OSFN(rc));
    INA_ASSERT_EQUAL(7, INA_RC_REASON(rc));
    INA_ASSERT_EQUAL(4, INA_RC_ID(rc));
    
    rc = INA_RC_PACK(15,15,255,1023);
    printf("rc = %u\n", rc);
    printf("id = %u\n", INA_RC_ID(rc));
    printf("mod = %u\n", INA_RC_MOD(rc));
    printf("func = %u\n", INA_RC_OSFN(rc));
    printf("reason = %u\n", INA_RC_REASON(rc));

    INA_ASSERT_EQUAL(15, INA_RC_MOD(rc));
    INA_ASSERT_EQUAL(15, INA_RC_OSFN(rc));
    INA_ASSERT_EQUAL(255, INA_RC_REASON(rc));
    INA_ASSERT_EQUAL(1023, INA_RC_ID(rc));   

    /*rc = INA_RC_PACK(63,31,511,1023);
    printf("rc = %u\n", rc);
    printf("id = %u\n", INA_RC_ID(rc));
    printf("mod = %u\n", INA_RC_MOD(rc));
    printf("func = %u\n", INA_RC_OSFN(rc));
    printf("reason = %u\n", INA_RC_REASON(rc));

    INA_ASSERT_EQUAL(63, INA_RC_MOD(rc));
    INA_ASSERT_EQUAL(31, INA_RC_OSFN(rc));
    INA_ASSERT_EQUAL(511, INA_RC_REASON(rc));
    INA_ASSERT_EQUAL(1023, INA_RC_ID(rc));   */
} 

void test_error_push_and_clear()
{
    INA_TRACE("test_error_push_and_clear");

    ina_rc_t rc1;
    ina_rc_t rc2;
    
    INA_ASSERT_EQUAL(INA_SUCCESS, ina_err_clear(INA_ERR_STATE_CLEAR));
    rc1 = ina_err_push(1,2,3,__FILE__, __LINE__ , "test 1");
    INA_ASSERT_EQUAL(rc1, ina_err_peek());
    INA_ASSERT_EQUAL(rc1, ina_err_peek_last());
    INA_ASSERT_EQUAL(ina_err_peek(), ina_err_peek_last());
    
    rc2 = INA_ERR_PUSH(1,2,5, "test error");
    INA_ASSERT_EQUAL(rc2, ina_err_peek());
    INA_ASSERT_NOTEQUAL(rc1, ina_err_peek());
    INA_ASSERT_EQUAL(rc1, ina_err_peek_last());
    INA_ASSERT_NOTEQUAL(rc1, rc2);
    
    INA_ASSERT_EQUAL(INA_SUCCESS, ina_err_clear(INA_ERR_STATE_CLEAR));
    INA_ASSERT_EQUAL(INA_SUCCESS, ina_err_peek());
    INA_ASSERT_EQUAL(INA_SUCCESS, ina_err_peek_last());
}


void test_error_push_and_peek()
{
    INA_TRACE("test_error_push_and_peek");
    
    size_t i;
    ina_rc_t rc;

    INA_ASSERT_EQUAL(INA_SUCCESS, ina_err_clear(INA_ERR_STATE_CLEAR));
    
    for (i = 0; i < 10; ++i) {
        INA_ERR_PUSH_BASIC(300+i, "This is an error");
    }
    
    i = 0;
    rc = INA_ERR_PEEK_FIRST;
    while (!(rc = ina_err_peek_next(rc))) {
        INA_ASSERT_FALSE(INA_SUCCEED(rc));
    }
}

void test_error_macros()
{
    INA_ERR_ERROR_MSGLEN;
    INA_ERR_ERROR_MSGFMT;
    
    INA_STR_ERROR_ALLOC;
}
