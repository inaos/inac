/*
 * Copyright (c) 2015-2017, INAOS GmbH
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
#ifndef _LIBINAC_LICENSE_H_
#define _LIBINAC_LICENSE_H_

/*
 * DESIGN:
 * -------
 *
 * - Client: A server/host at a customers site, 1 file per customer
 * - Registry: SQLITE Database storing all customers and its license details
 * - Demo purpose: Possibility to have a file-less check for a hard-coded date for expiry
 * - Security:
 *   + PKI based: Client(the software has private key statically linked into program-code.
 *     Public-Key is shipped togehter with the license-request.
 *   + License/Features per Host
 *   + HOST-ID, generated from CPU-Signature (Family, Model, Stepping) and MAC-Address of a choosen interface.
 *   + HOST-ID uses SHA-256 (e.g. from AxTLS)
 *   + PKI based on: Curve25519 and Poly-1305-AES from D.J.B
 *
 * - Server registrations process:
 *   + Initialize client-api, the private-key should be part of the program-code
 *   + Create a new request, public-key can be obtained from the client-api
 *   + CPU-Signature and MAC-Address are added to the request
 *   + Request-file gets generated and shipped to registry
 *
 * - Generating a license for a customer:
 *   + The registry also has a private key shipped as part of the program-code.. therefore the registry-code/binary are valuable
 *   + Reading the request-file provides the public-key of the remote-host as well as the HOST-ID
 *   + When generating the license file we use Poly-1305 as to sign/authenticate the body (which contains the host-ids and license-features)
 *   + Like this we make sure the file-content can not be changed
 *
 * - Veriying the license
 *   + The API will open the file, start reading the authenticator (Poly-1305) and verify the file integrity
 *   + For this it requires the public-key from the registry which is embedded into the license-file
 *   + Then the client will generate its HOST-ID and iterate through the license to check if there is an entry for this host
 *   + If yes it will read the licensed features and return a signed/encoded (Poly-1305) result value. The idea would be to make sure 
 *     the protected verification routing has actually been called. And the private key for the signing is hard-coded in the protected 
 *     verification routine.
 *   + Without protecting the assembly code for the verifycation routine it would be rather simple to bypass this mechanism
 *     therefore we'll have to "virtualize" at least the verification method.
 *     We could use this: http://tigress.cs.arizona.edu/transformPage/docs/virtualize/index.html
 *     Commercial solution: http://www.oreans.com/codevirtualizer.php, http://vmpsoft.com/products/matrix/
 *
 * - Hard-coded expiry (without license file) for demo-build
 *   + Same protection as for the verification method
 *
 * - Request-File features:
 *   + Public-Key of software itself
 *   + HOST-ID of the host/server/machine to license
 *   + Binary
 *   + No special security requirements
 *
 * - License-File features:
 *   + Binary
 *   + 16-Byte authenticator for Poly-1305
 *   + Number of Host entries
 *   + Array of host/feature structures/entries (host-id, features, expiry)
 * 
 */


#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* opaque */
typedef struct ina_license_client_s ina_license_client_t;
typedef struct ina_license_file_s ina_license_file_t;
typedef struct ina_license_request_s ina_license_request_t;
typedef struct ina_license_features_s ina_license_features_t;
typedef struct ina_license_feature_s ina_license_feature_t;
typedef struct ina_license_registry_s ina_license_registry_t;
typedef struct ina_license_registry_s ina_license_verify_result_t;

typedef enum ina_license_feature_type_e {
	INA_LICENSE_FEATURE_TYPE_INT,
	INA_LICENSE_FEATURE_TYPE_STRING,
} ina_license_feature_type_t;

/*
 *
 */
INA_API(ina_rc_t) ina_license_client_init(ina_license_client_t **c, char *private_key[32]);
/*
 *
 */
INA_API(ina_rc_t) ina_license_client_verify(ina_license_client_t *c, ina_license_file_t *lf, ina_license_verify_result_t *result);
/*
 *
 */
INA_API(ina_rc_t) ina_license_client_get_public_key(ina_license_client_t *c, char *public_key[32]);
/*
 *
 */
INA_API(ina_rc_t) ina_license_client_destory(ina_license_client_t **c);
/*
 *
 */
INA_API(ina_rc_t) ina_license_demo_expiry(time_t epoch_expiry_timestamp, ina_license_verify_result_t *result);
/*
 *
 */
INA_API(ina_rc_t) ina_license_file_open(const char *lic_file, ina_license_file_t **lf);
/*
 *
 */
INA_API(ina_rc_t) ina_license_file_close(ina_license_file_t **lf);
/*
 *
 */
INA_API(ina_rc_t) ina_license_file_features_localhost(ina_license_file_t *lf, ina_license_features_t **f);
/*
 *
 */
INA_API(ina_rc_t) ina_license_feature_id(ina_license_features_t *f, uint32_t *id);
/*
 *
 */
INA_API(ina_rc_t) ina_license_feature_type(ina_license_features_t *f, uint32_t id, ina_license_feature_type_t *t);
/*
 *
 */
INA_API(ina_rc_t) ina_license_feature_name(ina_license_features_t *f, uint32_t id, ina_str_t *name);
/*
 *
 */
INA_API(ina_rc_t) ina_license_feature_int(ina_license_features_t *f, uint32_t id, int *value);
/*
 *
 */
INA_API(ina_rc_t) ina_license_feature_string(ina_license_features_t *f, uint32_t id, ina_str_t *value);
/*
 * mjd = modified julian date
 */
INA_API(ina_rc_t) ina_license_feature_expiry(ina_license_features_t *f, uint32_t id, int *mjd);
/*
 *
 */
INA_API(ina_rc_t) ina_license_feature_new(ina_license_feature_t **f, uint32_t id, ina_str_t name,
										  ina_license_feature_type_t type, int iv, ina_str_t sv,
										  int mjd_expiry);
/*
 *
 */										  
INA_API(ina_rc_t) ina_license_feature_free(ina_license_feature_t **f);
/*
 *
 */
INA_API(ina_rc_t) ina_license_request_new(ina_license_request_t **r, char *public_key[32]);
/*
 *
 */
INA_API(ina_rc_t) ina_license_request_new_from_file(ina_license_request_t **r, const char *request_file_path);
/*
 *
 */
INA_API(ina_rc_t) ina_license_request_free(ina_license_request_t **r);
/*
 *
 */
INA_API(ina_rc_t) ina_license_request_set_cpu_id(ina_license_request_t *r, char familiy, char model, char stepping);
/*
 *
 */
INA_API(ina_rc_t) ina_license_request_set_mac_addr(ina_license_request_t *r, char *mac[6]);
/*
 *
 */
INA_API(ina_rc_t) ina_license_request_get_hostid(ina_license_request_t *r, char *hostid[32]);
/*
 *
 */
INA_API(ina_rc_t) ina_license_request_get_public_key(ina_license_request_t *r, char *public_key[32]);
/*
 *
 */
INA_API(ina_rc_t) ina_license_request_write_file(ina_license_request_t *r, const char *request_file_path);
/*
 *
 */
INA_API(ina_rc_t) ina_license_registry_open(const char *path_to_registry, char *private_key[32], ina_license_registry_t **registry);
/*
 *
 */
INA_API(ina_rc_t) ina_license_registry_close(ina_license_registry_t **registry);
/*
 *
 */
INA_API(ina_rc_t) ina_license_registry_add_customer(ina_license_registry_t *r, const char *customer_id);
/*
 *
 */
INA_API(ina_rc_t) ina_license_registry_add_host_to_customer(ina_license_registry_t *r, const char *customer_id,
															const char *hostid , const char *public_key);
/*
 *
 */
INA_API(ina_rc_t) ina_license_registry_add_feature_to_host(ina_license_registry_t *r, const char *customer_id, 
														   const char *hostid, ina_license_feature_t *feature);
/*
 *
 */
INA_API(ina_rc_t) ina_license_registry_generate_license(ina_license_registry_t *registry, const char *customer_id, const char *path_to_license);

#ifdef __cplusplus
}
#endif

#endif
