--
-- Copyright (c) 2015, INAOS GmbH
-- All rights reserved.
--
-- Redistribution and use in source and binary forms, with or without
-- modification, are permitted provided that the following conditions are met:
--     * Redistributions of source code must retain the above copyright
--       notice, this list of conditions and the following disclaimer.
--     * Redistributions in binary form must reproduce the above copyright
--       notice, this list of conditions and the following disclaimer in the
--       documentation and/or other materials provided with the distribution.
--     * Neither the name of the INAOS GmbH nor the names of its contributors
--       may be used to endorse or promote products derived from this software
--       without specific prior written permission.
--
-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
-- AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
-- IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
-- ARE DISCLAIMED. IN NO EVENT SHALL INAOS GmbH BE LIABLE FOR ANY DIRECT,
-- INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
-- (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
-- SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
-- CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
-- STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
-- ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
-- OF SUCH DAMAGE.
--

local ffi = require("ffi")

local MAX_CONCURRENT_REQUESTS = 128
local http = {}

ffi.cdef[[
typedef uint32_t ina_rc_t;

ina_rc_t ina_net_tcp_connect(int *fd, const char *addr, int port, int timeout_sec);

ina_rc_t ina_net_read(int fd, unsigned char *buf, int nb, int* nb_read);

ina_rc_t ina_net_write(int fd, const unsigned char *buf, int nb, int* nb_write);

ina_rc_t ina_net_nonblock(int fd);

ina_rc_t ina_net_block(int fd);

ina_rc_t ina_net_set_read_timeout(int fd, int msec);

ina_rc_t ina_net_set_write_timeout(int fd, int msec);

ina_rc_t ina_net_close(int fd);

typedef enum ina_http_parser_type_e {
        INA_HTTP_PARSER_TYPE_REQUEST,
        INA_HTTP_PARSER_TYPE_RESPONSE,
        INA_HTTP_PARSER_TYPE_BOTH
} ina_http_parser_type_t;

typedef enum ina_http_parser_method_e {
        INA_HTTP_PARSER_METHOD_DELETE,
        INA_HTTP_PARSER_METHOD_GET,
        INA_HTTP_PARSER_METHOD_HEAD,
        INA_HTTP_PARSER_METHOD_POST,
        INA_HTTP_PARSER_METHOD_PUT,
        INA_HTTP_PARSER_METHOD_CONNECT,
        INA_HTTP_PARSER_METHOD_OPTIONS,
        INA_HTTP_PARSER_METHOD_TRACE,
} ina_http_parser_method_t;

typedef enum http_parser_url_fields_e {
        INA_HTTP_PARSER_UF_SCHEMA    = 0,
        INA_HTTP_PARSER_UF_HOST      = 1,
        INA_HTTP_PARSER_UF_PORT      = 2,
        INA_HTTP_PARSER_UF_PATH      = 3,
        INA_HTTP_PARSER_UF_QUERY     = 4,
        INA_HTTP_PARSER_UF_FRAGMENT  = 5,
        INA_HTTP_PARSER_UF_USERINFO  = 6,
        INA_HTTP_PARSER_UF_MAX        = 7
} http_parser_url_fields_t;

/* opaque */
typedef struct ina_http_parser_s ina_http_parser_t;

/* opaque */
typedef struct ina_http_url_s ina_http_url_t;

/* opaque */
typedef struct ina_http_header_s ina_http_header_t;

typedef struct ina_http_ctx_s {
        int parser_pool_size;
        ina_http_parser_t *parsers;
} ina_http_ctx_t;

ina_rc_t ina_http_init(ina_http_ctx_t **ctx, ina_http_parser_type_t parser_type, int parser_pool_size);

ina_rc_t ina_http_destroy(ina_http_ctx_t **ctx);

ina_rc_t ina_http_parser_borrow(ina_http_ctx_t *ctx, ina_http_parser_t **p);

ina_rc_t ina_http_parser_release(ina_http_ctx_t *ctx, ina_http_parser_t **p);

ina_rc_t ina_http_parser_url_get(ina_http_parser_t *p, ina_http_url_t **url);

ina_rc_t ina_http_url_get_field(ina_http_url_t *url, uint16_t mask, const char **begin, uint16_t *len);

ina_rc_t ina_http_url_get_port(ina_http_url_t *url, uint16_t *port);

ina_rc_t ina_http_parser_header_first(ina_http_parser_t *p, ina_http_header_t **first);

]]

local function _establish_connection(r, host, port)
end

local function _close_connection(r)
end

local meta_request = {
  __index = {
    get = function(r, url, headers)
    end,
    post = function(r, url, headers, payload)
    end,
    head = function(r, url, headers)
    end,
    put = function(r, url, headers, payload)
    end,
    delete = function(r, url, headers)
    end,
  }
}

local meta_response = {
  __index = {
    headers = function(r)
    end,
  }
}

local _http_ctx = ffi.new("ina_http_ctx_t *[1]")

http._request_num = 0

http.init = function()
  if ffi.C.ina_http_init(_http_ctx, ffi.C.INA_HTTP_PARSER_TYPE_RESPONSE, 
      MAX_CONCURRENT_REQUESTS) > 0 then
    error("Could not initialize http module")
  end
end

http.destroy = function()
  ffi.C.ina_http_destroy(_http_ctx)
end

http.new_request = function()
  if http._request_num >= MAX_CONCURRENT_REQUESTS then
    error("Maximum number of concurrent requests: "..MAX_CONCURRENT_REQUESTS.." reached")
  end
  local r = {}
  r._fd = 0
  r._parser_ptr = ffi.new("ina_http_parser_t *[1]")
  r._connected = false
  if ffi.C.ina_http_parser_borrow(_http_ctx[0], r._parser_ptr) > 0 then
    error("Could not borrow a http-parser")
  end
  http._request_num = http._request_num + 1
  setmetatable(r, meta_request)
  return r
end

http.free_request = function(r)
  if not r or not type(r) == "table" then
    error("Argument 'request' must be present and a reference")
  end
  if ffi.C.ina_http_parser_release(_http_ctx[0], r._parser_ptr) > 0 then
    error("Could not return the http-parser")
  end
  http._request_num = http._request_num - 1
end

http.url_encode_kv = function(params)
end

http.url_decode_kv = function(str)
end

return http

