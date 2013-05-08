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

INA_TEST(types, types_decimal)
{
    ina_decimal_t d1;
    ina_decimal_t d2;
    double dbl1 = 1.0;
    double dbl2 = 1.0;

    ina_dbl_to_decimal(dbl1, &d1);
    ina_dbl_to_decimal(dbl2, &d2);
    INA_TEST_ASSERT_EQUAL_FLOATING(d1.exponent, d2.exponent);
    INA_TEST_ASSERT_EQUAL_INTEGER(d1.mantissa, d2.mantissa);
    INA_TEST_ASSERT_EQUAL_FLOATING(1.0, ina_dbl_from_decimal(&d1));
    INA_TEST_ASSERT_EQUAL_FLOATING(1.0, ina_dbl_from_decimal(&d2));
    ina_dbl_to_decimal(2.5, &d1);
    INA_TEST_ASSERT_NOT_EQUAL_FLOATING(d1.exponent, d2.exponent);
    INA_TEST_ASSERT_NOT_EQUAL_INTEGER(d1.mantissa, d2.mantissa);
    ina_cpy_decimal(&d1, &d2);
    INA_TEST_ASSERT_EQUAL_FLOATING(d1.exponent, d2.exponent);
    INA_TEST_ASSERT_EQUAL_INTEGER(d1.mantissa, d2.mantissa);    
}