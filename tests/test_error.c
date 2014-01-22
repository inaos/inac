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

INA_TEST(error, repush_success)
{
    INA_TEST_ASSERT_SUCCESS(ina_err_reset());
    INA_TEST_ASSERT_SUCCESS(ina_err_peek());

    INA_TEST_ASSERT_SUCCEED(INA_ERR_REPUSH(INA_SUCCESS));
    INA_TEST_ASSERT_SUCCESS(INA_ERR_REPUSH(INA_SUCCESS));
    INA_TEST_ASSERT_SUCCESS(ina_err_peek());
}

INA_TEST(error, repush)
{
    ina_rc_t rc;

    INA_TEST_ASSERT_SUCCESS(ina_err_reset());
    INA_TEST_ASSERT_SUCCESS(ina_err_peek());
    
    INA_ERR_EMSGLEN;
    INA_ERR_EMSGFMT;
    INA_STR_EALLOC;
    INA_ERR_PUSH_LAST;
    INA_ERR_PUSH_LAST;
    ina_err_repush(INA_EAGAIN, __FILE__, __LINE__);
    INA_ERR_PUSH_LAST;

    rc = ina_err_peek();
    INA_TEST_ASSERT_EQUAL_INTEGER(INA_EAGAIN, INA_RC_REASON(rc));
    rc = ina_err_peek_next(rc);
    INA_TEST_ASSERT_EQUAL_INTEGER(INA_EAGAIN, INA_RC_REASON(rc));    
    rc = ina_err_peek_next(rc);
    INA_TEST_ASSERT_EQUAL_INTEGER(INA_EALLOC, INA_RC_REASON(rc));
    rc = ina_err_peek_next(rc);
    INA_TEST_ASSERT_EQUAL_INTEGER(INA_EALLOC, INA_RC_REASON(rc));
    rc = ina_err_peek_next(rc);
    INA_TEST_ASSERT_EQUAL_INTEGER(INA_EALLOC, INA_RC_REASON(rc));
    rc = ina_err_peek_next(rc);
    INA_TEST_ASSERT_EQUAL_INTEGER(INA_EMSGFMT, INA_RC_REASON(rc));
    rc = ina_err_peek_next(rc);
    INA_TEST_ASSERT_EQUAL_INTEGER(INA_EMSGLEN, INA_RC_REASON(rc));
}

INA_TEST(error, push_a_million_errors)
{
    size_t i;

   
    INA_TEST_ASSERT_SUCCESS(ina_err_reset());
    INA_TEST_ASSERT_SUCCESS(ina_err_peek());

    for (i = 0; i < 1000000; ++i) {
        INA_ERR_PUSH(1,2,5, "test error");
        INA_TEST_ASSERT_FALSE(INA_SUCCEED(ina_err_peek()));
    }
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
    INA_TEST_ASSERT_SUCCESS(ina_err_peek());
    INA_ERR_EMSGLEN;
    INA_TEST_ASSERT_EQUAL_INTEGER(INA_SUCCESS, ina_err_fmtmsg(ina_err_peek(), msg2, 100));
    INA_TEST_MSG("msg2=%s", ina_str_cstr(msg2));
    ina_str_free(msg1);
    ina_str_free(msg2);
}

INA_TEST(error, macros)
{
     INA_ERR_EMSGLEN;
     INA_ERR_EMSGFMT;
     INA_STR_EALLOC;
}

INA_TEST(error, push_and_peek)
{
    size_t i;
    ina_rc_t rc;

    INA_TEST_ASSERT_SUCCESS(ina_err_reset());
    INA_TEST_ASSERT_SUCCESS(ina_err_peek());

    for (i = 0; i < 10; ++i) {
        INA_ERR_PUSH_BASIC(300+i, "This is an error");
    }

     i = 0;
     rc = INA_ERR_PEEK_FIRST;
     while (!(rc = ina_err_peek_next(rc))) {
         INA_TEST_ASSERT_FALSE(INA_SUCCEED(rc));
     }
}

INA_TEST(error, push_and_clear)
{
	ina_rc_t rc1;
    ina_rc_t rc2;

    INA_TEST_ASSERT_SUCCESS(ina_err_reset());
    rc1 = ina_err_push(1,2,3,__FILE__, __LINE__ , "test 1");
    INA_TEST_ASSERT_EQUAL_INTEGER(rc1, ina_err_peek());
    INA_TEST_ASSERT_EQUAL_INTEGER(rc1, ina_err_peek_last());
    INA_TEST_ASSERT_EQUAL_INTEGER(ina_err_peek(), ina_err_peek_last());

    rc2 = INA_ERR_PUSH(1,2,5, "test error");
    INA_TEST_ASSERT_EQUAL_INTEGER(rc2, ina_err_peek());
    INA_TEST_ASSERT_NOT_EQUAL_INTEGER(rc1, ina_err_peek());
    INA_TEST_ASSERT_EQUAL_INTEGER(rc1, ina_err_peek_last());
    INA_TEST_ASSERT_NOT_EQUAL_INTEGER(rc1, rc2);

    INA_TEST_ASSERT_SUCCESS(ina_err_reset());
    INA_TEST_ASSERT_SUCCESS(ina_err_peek());
    INA_TEST_ASSERT_SUCCESS(ina_err_peek_last());
 }

INA_TEST(error, error_pack_rc) 
{
    ina_rc_t rcc;
    ina_rc_t rc;

    rcc = 16846855;
    rc = 0;
    rc = INA_RC_PACK(1,2,7,4);
    
    INA_TRACE3("rc = %u", rc);
    INA_TRACE3("id = %u", INA_RC_ID(rc));
    INA_TRACE3("mod = %u", INA_RC_MOD(rc));
    INA_TRACE3("func = %u", INA_RC_OSFN(rc));
    INA_TRACE3("reason = %u", INA_RC_REASON(rc));
    
    INA_TEST_ASSERT_EQUAL_INTEGER(rcc, rc);
    INA_TEST_ASSERT_EQUAL_INTEGER(1, INA_RC_MOD(rc));
    INA_TEST_ASSERT_EQUAL_INTEGER(2, INA_RC_OSFN(rc));
    INA_TEST_ASSERT_EQUAL_INTEGER(7, INA_RC_REASON(rc));
    INA_TEST_ASSERT_EQUAL_INTEGER(4, INA_RC_ID(rc));
    INA_TEST_ASSERT_FALSE(INA_RC_FATAL(rc));
    
    rc = INA_RC_PACK(15,15,255,1023);
    INA_TEST_ASSERT_EQUAL_INTEGER(15, INA_RC_MOD(rc));
    INA_TEST_ASSERT_EQUAL_INTEGER(15, INA_RC_OSFN(rc));
    INA_TEST_ASSERT_EQUAL_INTEGER(255, INA_RC_REASON(rc));
    INA_TEST_ASSERT_EQUAL_INTEGER(1023, INA_RC_ID(rc));   

    rc = INA_RC_PACK(63,31,511,1023);
    INA_TEST_ASSERT_EQUAL_INTEGER(63, INA_RC_MOD(rc));
    INA_TEST_ASSERT_EQUAL_INTEGER(31, INA_RC_OSFN(rc));
    INA_TEST_ASSERT_EQUAL_INTEGER(511, INA_RC_REASON(rc));
    INA_TEST_ASSERT_EQUAL_INTEGER(1023, INA_RC_ID(rc));
} 
