/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>
#include "config.h"

static const unsigned char base64_decode_tab[] = {
    66,66,66,66,66,66,66,66,66,64,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,62,66,66,66,63,52,53,
    54,55,56,57,58,59,60,61,66,66,66,65,66,66,66, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,66,66,66,66,66,66,26,27,28,
    29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66
};


INA_API(ina_rc_t) ina_util_base64_encode_length(size_t in_length, unsigned int line_length, size_t *out_length)
{
    size_t adjustment, code_padded_size, newline_size = 0;
    INA_VERIFY_NOT_NULL(out_length);
    INA_VERIFY(line_length != 0);

    adjustment = ( (in_length % 3) ? (3 - (in_length % 3)) : 0);
    code_padded_size = ( (in_length + adjustment) / 3) * 4;
    if (line_length > 0) {
        newline_size = ((code_padded_size) / line_length) * 2;
    }

    *out_length = code_padded_size + newline_size + 1;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_util_base64_encode_chunk(const void* data_buf, size_t dataLength, char* result, size_t resultSize)
{
    const char base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const uint8_t *data = (const uint8_t *)data_buf;
    size_t resultIndex = 0;
    size_t x;
    uint32_t n = 0;
    int padCount = dataLength % 3;
    uint8_t n0, n1, n2, n3;
    INA_VERIFY_NOT_NULL(data_buf);
    INA_VERIFY_NOT_NULL(result);
    
    /* increment over the length of the string, three characters at a time */
    for (x = 0; x < dataLength; x += 3) {
        /* these three 8-bit (ASCII) characters become one 24-bit number */
        n = data[x] << 16;
        
        if((x+1) < dataLength) {
            n += data[x+1] << 8;
        }
        
        if((x+2) < dataLength) {
            n += data[x+2];
        }
        
        /* this 24-bit number gets separated into four 6-bit numbers */
        n0 = (uint8_t)(n >> 18) & 63;
        n1 = (uint8_t)(n >> 12) & 63;
        n2 = (uint8_t)(n >> 6) & 63;
        n3 = (uint8_t)n & 63;
        
        /*
         * if we have one byte available, then its encoding is spread
         * out over two characters
         */
        if (resultIndex >= resultSize) {
            return INA_ERROR(INA_NN_BUFFER|INA_ERR_TOO_SMALL);   /* indicate failure: buffer too small */
        }
        result[resultIndex++] = base64chars[n0];
        if(resultIndex >= resultSize) {
            return INA_ERROR(INA_NN_BUFFER|INA_ERR_TOO_SMALL);   /* indicate failure: buffer too small */
        }
        result[resultIndex++] = base64chars[n1];

        /*
         * if we have only two bytes available, then their encoding is
         * spread out over three chars
         */
        if((x+1) < dataLength) {
            if (resultIndex >= resultSize) {
                return INA_ERROR(INA_NN_BUFFER|INA_ERR_TOO_SMALL);   /* indicate failure: buffer too small */
            }
            result[resultIndex++] = base64chars[n2];
        }
        
        /*
         * if we have all three bytes available, then their encoding is spread
         * out over four characters
         */
        if((x+2) < dataLength) {
            if (resultIndex >= resultSize) {
                return INA_ERROR(INA_NN_BUFFER|INA_ERR_TOO_SMALL);   /* indicate failure: buffer too small */
            }
            result[resultIndex++] = base64chars[n3];
        }
    }
    /*
     * create and add padding that is required if we did not have a multiple of 3
     * number of characters available
     */
    if (padCount > 0) {
        for (; padCount < 3; padCount++) {
            if (resultIndex >= resultSize) {
                return INA_ERROR(INA_NN_BUFFER|INA_ERR_TOO_SMALL);   /* indicate failure: buffer too small */
            }
            result[resultIndex++] = '=';
        }
    }
    if (resultIndex >= resultSize) {
        return INA_ERROR(INA_NN_BUFFER|INA_ERR_TOO_SMALL);   /* indicate failure: buffer too small */
    }
    result[resultIndex] = 0;
    
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_util_base64_decode_chunk(char *in, size_t in_len, unsigned char *out, size_t max_out, size_t *out_len)
{
#define __INA_UTILS_BASE64_WHITESPACE 64
#define __INA_UTILS_BASE64_EQUALS     65
#define __INA_UTILS_BASE64_INVALID    66
    char *end = in + in_len;
    size_t buf = 1, len = 0;
    INA_VERIFY_NOT_NULL(in);
    INA_VERIFY_NOT_NULL(out);
    INA_VERIFY_NOT_NULL(out_len);

    *out_len = 0;

    while (in < end) {
        unsigned char c = base64_decode_tab[(int)(*in++)];
 
        switch (c) {
            case __INA_UTILS_BASE64_WHITESPACE: 
                continue;   /* skip whitespace */
            case __INA_UTILS_BASE64_INVALID:
                return INA_ERROR(INA_NN_INPUT|INA_ERR_INVALID);   /* invalid input, return error */
            case __INA_UTILS_BASE64_EQUALS:                 
                /* pad character, end of data */
                in = end;
                continue;
            default:
                buf = buf << 6 | c;
 
                /* If the buffer is full, split it into bytes */
                if (buf & 0x1000000) {
                    if ((len += 3) > max_out) {
                        return INA_ERROR(INA_NN_BUFFER|INA_ERR_OVERFLOW); /* buffer overflow */
                    }
                    *out++ = (unsigned char)(buf >> 16);
                    *out++ = (unsigned char)(buf >> 8);
                    *out++ = (unsigned char)buf;
                    buf = 1;
                }   
        }
    }
 
    if (buf & 0x40000) {
        if ((len += 2) > max_out) {
            return INA_ERROR(INA_NN_BUFFER|INA_ERR_OVERFLOW); /* buffer overflow */
        }
        *out++ = (unsigned char)(buf >> 10);
        *out++ = (unsigned char)(buf >> 2);
    }
    else if (buf & 0x1000) {
        if (++len > max_out) {
            return INA_ERROR(INA_NN_BUFFER|INA_ERR_OVERFLOW); /* buffer overflow */
        }
        *out++ = (unsigned char)(buf >> 4);
    }
 
    *out_len = len; /* modify to reflect the actual output size */

    return INA_SUCCESS;
}

INA_API(int) ina_util_dbl_cmp_abs(double x, double y)
{
    return fabs(x - y) <= DBL_EPSILON;
}

INA_API(int) ina_util_dbl_cmp_rel(double x, double y)
{
    return fabs(x - y) <= DBL_EPSILON * INA_MAX(fabs(x), fabs(y));
}

INA_API(int) ina_util_dbl_cmp_save(double x, double y)
{
    return fabs(x - y) <= DBL_EPSILON * INA_MAX(1.0f, INA_MAX(fabs(x), fabs(y)));
}
