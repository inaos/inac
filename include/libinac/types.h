/*
 * Copyright (c) 2012-2015, INAOS GmbH
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
#ifndef _LIBINAC_TYPES_H_
#define _LIBINAC_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Return code */
typedef uint32_t ina_rc_t;

/* Decimal type */
INA_VS_BEGIN_PACK
typedef struct INA_PACKED ina_decimal_s {
    int32_t exponent;
    int64_t mantissa;
} ina_decimal_t;
INA_VS_END_PACK

/*
 * Copy a decimal.
 */
INA_INLINE void ina_cpy_decimal(const ina_decimal_t *src, ina_decimal_t *dst)
{
    dst->exponent = src->exponent;
    dst->mantissa = src->mantissa;
}
/*
 * Compare decimal values:
 * - returns 0 if the decimals are equal
 * - returns > 0 if the lhs is bigger then rhs
 * - return < 0 if the lhs is smaller then rhs
 */
INA_INLINE int ina_cmp_decimal(const ina_decimal_t *lhs, const ina_decimal_t *rhs)
{
    return memcmp(lhs, rhs, sizeof(ina_decimal_t));
}
/*
 * Convert a double to a decimal type.
 */
INA_INLINE void ina_dbl_to_decimal(double dbl, ina_decimal_t *dec)
{
    double tmp = frexp(dbl, &dec->exponent);
    dec->mantissa = (int64_t)(tmp * (double)pow((double)FLT_RADIX, DBL_MANT_DIG));
}
/*
 * Convert decimal to a double type.
 */
INA_INLINE double ina_dbl_from_decimal(const ina_decimal_t *dec)
{
    double tmp = dec->mantissa / (double)pow((double)FLT_RADIX, DBL_MANT_DIG);
    return ldexp(tmp, dec->exponent);
}

#ifdef INA_BSTRING_ENABLED
#include <bstring/bstrlib.h>
#define ina_str_t bstring
#ifdef INA_STRING_ENABLED
#error String library already defined.
#endif
#define INA_STRING_DEFINED 1
#else
typedef char * ina_str_t;
#endif

#ifdef INA_OS_WIN32
typedef HANDLE ina_handle_t;
typedef char ina_semkey_t[MAX_PATH];
#else
typedef int ina_handle_t;
typedef int ina_semkey_t;
#endif

#ifdef __cplusplus
}
#endif 

#endif
