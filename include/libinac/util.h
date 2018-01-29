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
#ifndef _LIBINAC_UTIL_H_
#define _LIBINAC_UTIL_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Base64 encoding - calculate output length.
 *
 * Parameters
 *  in_length    Input length
 *  line_length  Number of chars par line
 *  out_length   Where to store the output length.
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_util_base64_encode_length(size_t in_length,
                                                unsigned int line_length,
                                                size_t *out_length);

/*
 * Base64 encoding - encode a chunk
 *
 * Parameters
 *  data_buf    Input data
 *  dataLength  Input data length in bytes.
 *  result      Encoded data
 *  resultSize  Length of encoded data
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_util_base64_encode_chunk(const void* data_buf,
                                               size_t dataLength,
                                               char* result,
                                               size_t resultSize);

/*
 * Base64 decoding - decode a chunk
 *
 * Parameters
 *  in       Input data
 *  in_len   Length of input data
 *  out      Output buffer
 *  max_out  Maximal length of output to produce
 *  out_len  Where to write output length
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_util_base64_decode_chunk(char *in,
                                               size_t in_len,
                                               unsigned char *out,
                                               size_t max_out,
                                               size_t *out_len);

/*
 * Absolute tolerance test fails when X and Y become "large"
 * This is the fastest possible comparision that is correct for 
 * x, y < ?
 *
 * Parameters
 *  x,y  Double values to compare
 *
 * Return
 * >  0 equal
 * <= 0 not equal
 */
INA_API(int) ina_util_dbl_cmp_abs(double x, double y);

/*
 * Relative tolerance test fails when X and Y become "small"
 *
 * Parameters
 *  x,y  Double values to compare
 *
 * Return
 * >  0 equal
 * <= 0 not equal
 */
INA_API(int) ina_util_dbl_cmp_rel(double x, double y);

/*
 * Combined tolerance test that should work safely for all cases
 * however is the "slowest" variant
 * Parameters
 *  x,y  Double values to compare
 *
 * Return
 * >  0 equal
 * <= 0 not equal
 */
INA_API(int) ina_util_dbl_cmp_save(double x, double y);
     
#ifdef __cplusplus
}
#endif 

#endif
