/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
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
