/*
* Copyright (c) 2012-2013, INAOS GmbH
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

static ina_iscp_msg_t __send_msg;
static ina_rc_t __send_rc;
static int __open_count = 0;
static int __clse_count = 0;
static int __retn_count = 0;
static int __send_count = 0;
static int __recv_count = 0;
static int __handler_count = 0;
static int __p_count = 0;

static ina_rc_t __null_open_cb(void *user_data, int send)
{
   ++__open_count;
   return INA_SUCCESS;
}

static ina_rc_t __null_clse_cb(void *user_data, int send)
{
   ++__clse_count;
   return INA_SUCCESS;
}


static ina_rc_t __null_send_cb(void *user_data, ina_iscp_msg_t *msg)
{
   INA_TEST_ASSERT_NOT_NULL(msg);
   ++__send_count;
   ina_mem_cpy(&__send_msg, msg, msg->length);
   return INA_SUCCESS;
}

static ina_rc_t __null_recv_cb(void *user_data, ina_iscp_msg_t *msg)
{   
   INA_TEST_ASSERT_NOT_NULL(msg);
   ++__recv_count;
   ina_mem_cpy(msg, &__send_msg, __send_msg.length);
   return INA_SUCCESS;
}

static ina_rc_t __null_retn_cb(void *user_data, ina_iscp_msg_t *msg)
{
   INA_TEST_ASSERT_NOT_NULL(msg);
   ++__retn_count;
   __send_rc = msg->rc;
  /* printf("__send_buf->length=%d\n", __send_buf->length);*/
   return INA_SUCCESS;
}

static ina_rc_t __null_handler(int cmd_id, int count, const ina_iscp_param_t *params, int r_count, ina_iscp_param_t *retvals)
{
   ++__handler_count;
   __p_count = count;
   return INA_SUCCESS;
}

static ina_rc_t __null_handler2(int cmd_id, int count, const ina_iscp_param_t *params, int r_count, ina_iscp_param_t *retvals)
{   
   ++__handler_count;
   while (params) {
       ++__p_count;
   }
   INA_TEST_ASSERT_EQUAL_INTEGER(count, __p_count);
   return INA_SUCCESS;
}

static ina_rc_t __check_params_handler(int cmd_id, int count, const ina_iscp_param_t *params, int r_count, ina_iscp_param_t *retvals)
{   
   int i;

   ++__handler_count;

   for (i = 0; i < count; ++i) {
       ++__p_count;
       if (__p_count == 1) {
           INA_TEST_ASSERT_EQUAL_INTEGER(INA_ISCP_TYPE_INT64, params->type);
           INA_TEST_ASSERT_EQUAL_INTEGER(20, params->value.i);
       }
       if (__p_count == 2) {
           INA_TEST_ASSERT_EQUAL_INTEGER(INA_ISCP_TYPE_DBL, params->type);
           INA_TEST_ASSERT_EQUAL_FLOATING(5.2, params->value.d);
       } 
       if (__p_count == 3) {
           INA_TEST_ASSERT_EQUAL_INTEGER(INA_ISCP_TYPE_STR, params->type);
           INA_TEST_ASSERT_EQUAL_INTEGER(0, strcmp("test", params->value.s));
       }
       ++params;
    }

    ina_iscp_set_return_values(retvals, r_count,
        INA_ISCP_TYPE_DBL, 55.5,
        INA_ISCP_TYPE_STR, "blabla",
        INA_ISCP_TYPE_INT64, (int64_t)12);
   return INA_SUCCESS;
}

INA_TEST_DATA(iscp_tcp) {
    ina_test_hid_t hid;
    ina_iscp_ctx_t *iscp;
};

INA_TEST_SETUP(iscp_tcp) {
    INA_TEST_HELPER_INVOKE(&data->hid, iscp_tcp, tcp_server, "127.0.0.1", "9999", NULL);
    ina_iscp_create_tcp(&data->iscp, "127.0.0.1", 9999);
}

INA_TEST_TEARDOWN(iscp_tcp) {
    INA_TEST_HELPER_TERMINATE(&data->hid);
    ina_iscp_destroy(&data->iscp);
    ina_err_reset();
}

INA_TEST_FIXTURE(iscp_tcp, send_negative_double) {
      INA_TEST_ASSERT_SUCCEED(ina_iscp_register(data->iscp, 3, 1, 0, NULL));
      INA_TEST_ASSERT_SUCCEED(ina_iscp_send(data->iscp, 3, INA_ISCP_TYPE_DBL, -3.2));
}

INA_TEST_FIXTURE(iscp_tcp, send_tcp) {
    double d = 0.0;
    int64_t i = 0;
    ina_str_t str = NULL;

    INA_TEST_ASSERT_NOT_NULL(data->iscp);
    INA_TEST_ASSERT_SUCCEED(ina_iscp_register(data->iscp, 1, 3, 3, NULL));

    INA_TEST_ASSERT_SUCCEED(ina_iscp_send(data->iscp, 1, 
                            INA_ISCP_TYPE_INT64, 20,
                            INA_ISCP_TYPE_DBL, 5.2,
                            INA_ISCP_TYPE_STR, "test"));

    INA_TEST_ASSERT_SUCCEED(ina_iscp_get_last_return_values(data->iscp, 
                                  INA_ISCP_TYPE_DBL, &d,
                                  INA_ISCP_TYPE_STR, &str,
                                  INA_ISCP_TYPE_INT64, &i));

    INA_TEST_ASSERT_EQUAL_FLOATING(55.5, d);
    INA_TEST_ASSERT_EQUAL_INTEGER(12, i);
    INA_TEST_ASSERT_EQUAL_STR("blabla", ina_str_cstr(str));    
                            
}

INA_TEST(iscp_null, send_recv_checkparams)
{
    ina_iscp_ctx_t *ctx = NULL;
 
    __send_count = 0;
    __recv_count = 0;
    __p_count = 0;
    __handler_count = 0;
    /*double d = 0.0;
    int64_t i = 0;
    ina_str_t str = NULL;*/

    INA_TEST_ASSERT_SUCCESS(ina_iscp_create(&ctx, INA_ISCP_NONE));
    INA_TEST_ASSERT_SUCCEED(ina_iscp_set_callbacks(ctx, __null_open_cb, 
                                                   __null_clse_cb, 
                                                   __null_send_cb,
                                                   __null_recv_cb, 
                                                   __null_retn_cb));
    INA_TEST_ASSERT_SUCCEED(ina_iscp_register(ctx, 1, 3, 3, __check_params_handler));

    INA_TEST_ASSERT_SUCCEED(ina_iscp_send(ctx, 1, 
                            INA_ISCP_TYPE_INT64, 20,
                            INA_ISCP_TYPE_DBL, 5.2,
                            INA_ISCP_TYPE_STR, "test"));
    INA_TEST_ASSERT_EQUAL_INTEGER(1, __send_count);
    INA_TEST_ASSERT_EQUAL_INTEGER(1, __send_msg.cmd_id);
    INA_TEST_ASSERT_SUCCEED(ina_iscp_recv(ctx, 1000, 1));
    INA_TEST_ASSERT_EQUAL_INTEGER(1, __recv_count);
    INA_TEST_ASSERT_EQUAL_INTEGER(1, __handler_count);
    INA_TEST_ASSERT_EQUAL_INTEGER(3, __p_count);
    /*INA_TEST_ASSERT_SUCCEED(ina_iscp_get_last_return_values(ctx, 
                                  INA_ISCP_TYPE_DBL, &d,
                                  INA_ISCP_TYPE_STR, &str,
                                  INA_ISCP_TYPE_INT64, &i));
    INA_TEST_ASSERT_EQUAL_FLOATING(55.5, d);
    INA_TEST_ASSERT_EQUAL_INTEGER(12, i);
    INA_TEST_ASSERT_EQUAL_STR("blabla", ina_str_cstr(str));*/    
 
}

INA_TEST(iscp, send_local)
{
   ina_iscp_ctx_t *ctx = NULL;

   __send_count = 0;

   INA_TEST_ASSERT_SUCCEED(ina_iscp_destroy(&ctx));
   INA_TEST_ASSERT_SUCCESS(ina_iscp_create(&ctx, INA_ISCP_INET));
   INA_TEST_ASSERT_NOTSUCCEED(ina_iscp_send(ctx, 1, 
                            INA_ISCP_TYPE_INT64, 300,
                            INA_ISCP_TYPE_DBL, 3.2,
                            INA_ISCP_TYPE_STR, "test"));
   INA_TEST_ASSERT_EQUAL_INTEGER(0, __send_count);
   INA_TEST_ASSERT_SUCCESS(ina_iscp_create(&ctx, INA_ISCP_INET));
   INA_TEST_ASSERT_SUCCEED(ina_iscp_set_callbacks(ctx, __null_open_cb, 
                                                   __null_clse_cb, 
                                                   __null_send_cb,
                                                   __null_recv_cb, 
                                                   __null_retn_cb));
   INA_TEST_ASSERT_NOTSUCCEED(ina_iscp_send(ctx, 1, 
                           INA_ISCP_TYPE_INT64, 300,
                           INA_ISCP_TYPE_DBL, 3.2,
                           INA_ISCP_TYPE_STR, "test"));
   INA_TEST_ASSERT_SUCCEED(ina_iscp_register(ctx, 1, 3, 0, NULL));
   INA_TEST_ASSERT_SUCCEED(ina_iscp_send(ctx, 1, 
                           INA_ISCP_TYPE_INT64, 20,
                           INA_ISCP_TYPE_DBL, 5.2,
                           INA_ISCP_TYPE_STR, "test-2"));
   INA_TEST_ASSERT_NOTSUCCEED(ina_iscp_send(ctx, 1, 
                           INA_ISCP_TYPE_INT64, 20,
                           INA_ISCP_TYPE_STR, "test-2"));
   INA_TEST_ASSERT_EQUAL_INTEGER(1, __send_count);
}

INA_TEST(iscp, setup)
{
    ina_iscp_ctx_t *ctx = NULL;

    INA_TEST_ASSERT_SUCCESS(ina_iscp_create(&ctx, INA_ISCP_INET));
    INA_TEST_ASSERT_SUCCEED(ina_iscp_set_callbacks(ctx, __null_open_cb, 
                                                    __null_clse_cb, 
                                                    __null_send_cb,
                                                    __null_recv_cb, 
                                                    __null_retn_cb));
    INA_TEST_ASSERT_NOTSUCCEED(ina_iscp_set_callbacks(ctx, NULL,NULL, NULL, __null_recv_cb, NULL));
    INA_TEST_ASSERT_NOTSUCCEED(ina_iscp_set_callbacks(ctx, NULL, NULL, NULL, NULL, NULL));
    INA_TEST_ASSERT_NOTSUCCEED(ina_iscp_set_callbacks(ctx, NULL, NULL, __null_send_cb, NULL, NULL));
    INA_TEST_ASSERT_SUCCEED(ina_iscp_set_callbacks(ctx, __null_open_cb, 
                                                    __null_clse_cb, 
                                                    __null_send_cb,
                                                    __null_recv_cb, 
                                                    __null_retn_cb));
    INA_TEST_ASSERT_SUCCEED(ina_iscp_register(ctx, 1, 3, 0,__null_handler));
    INA_TEST_ASSERT_SUCCEED(ina_iscp_register(ctx, 1, 3, 0,__null_handler));
    INA_TEST_ASSERT_SUCCEED(ina_iscp_register(ctx, 1, 3, 0,__null_handler2));
    INA_TEST_ASSERT_NOTSUCCEED(ina_iscp_register(ctx, 1, 2, 0, __null_handler));
    INA_TEST_ASSERT_NOTSUCCEED(ina_iscp_register(ctx, 1, 4, 0, __null_handler2));
    INA_TEST_ASSERT_SUCCEED(ina_iscp_destroy(&ctx));
    INA_TEST_ASSERT_SUCCESS(ina_iscp_create(&ctx, INA_ISCP_INET));
    INA_TEST_ASSERT_SUCCEED(ina_iscp_register(ctx, 1, 4, 0, __null_handler2));
    INA_TEST_ASSERT_SUCCEED(ina_iscp_set_callbacks(ctx, __null_open_cb, 
                                                    __null_clse_cb, 
                                                    __null_send_cb,
                                                    __null_recv_cb, 
                                                    __null_retn_cb));
    INA_TEST_ASSERT_SUCCEED(ina_iscp_register(ctx, 1, 4, 0, __null_handler2));
}

INA_TEST(iscp, iscp_regsiter_ex)
{
    ina_iscp_ctx_t *ctx = NULL;
    ina_iscp_cmd_t cmds[] = {
         INA_ISCP_SEND_CMD(1,1,0),
         INA_ISCP_SENDRECV_CMD(2,3,0,__null_handler2),
     };
     INA_ISCP_CMDS(cmds2,
          INA_ISCP_SENDRECV_CMD(4,1,0,__null_handler2),
          INA_ISCP_SENDRECV_CMD(5,3,0, __null_handler2));
     
     
    INA_TEST_ASSERT_SUCCESS(ina_iscp_create(&ctx, INA_ISCP_NONE));
    INA_TEST_ASSERT_NOT_NULL(ctx);
    INA_TEST_ASSERT_SUCCEED(ina_iscp_register_ex(ctx, cmds));
    INA_TEST_ASSERT_SUCCEED(ina_iscp_register_ex(ctx, cmds2));
}
