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

static ina_iscp_msg_t *__send_msg;
static ina_iscp_rc_t *__send_rc;
static int __open_count = 0;
static int __clse_count = 0;
static int __retn_count = 0;
static int __send_count = 0;
static int __recv_count = 0;
static int __handler_count = 0;
static int __p_count = 0;
static int __stop = 0;

static ina_rc_t __stop_handler(int cmd_id, int count, ina_iscp_param_t *params)
{
   __stop = 1;
   return INA_SUCCESS;
}

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
   INA_TEST_ASSERT_NOTNULL(msg);
   ++__send_count;
   __send_msg = msg;
  /* printf("__send_buf->length=%d\n", __send_buf->length);*/
   return INA_SUCCESS;
}

static ina_rc_t __null_recv_cb(void *user_data, ina_iscp_msg_t *msg)
{   
   ina_iscp_msg_t *recv_msg;

   INA_TEST_ASSERT_NOTNULL(msg);
   ++__recv_count;
   ina_mem_cpy(msg, __send_msg, __send_msg->length);
   recv_msg = msg;
   /*printf("__send_buf->length=%d\n", recv_buf->length);*/
   return INA_SUCCESS;
}

static ina_rc_t __null_retn_cb(void *user_data, ina_iscp_rc_t *rc)
{
   INA_TEST_ASSERT_NOTNULL(rc);
   ++__retn_count;
   __send_rc = rc;
  /* printf("__send_buf->length=%d\n", __send_buf->length);*/
   return INA_SUCCESS;
}

static ina_rc_t __null_handler(int cmd_id, int count, ina_iscp_param_t *params)
{
   ++__handler_count;
   __p_count = count;
   return INA_SUCCESS;
}

static ina_rc_t __null_handler2(int cmd_id, int count, ina_iscp_param_t *params)
{   
   ++__handler_count;
   while (params) {
       ++__p_count;
   }
   INA_TEST_ASSERT_EQUAL(count, __p_count);
   return INA_SUCCESS;
}

static ina_rc_t __check_params_handler(int cmd_id, int count, ina_iscp_param_t *params)
{   
   int i;

   ++__handler_count;

   for (i = 0; i < count; ++i) {
       ++__p_count;
       if (__p_count == 1) {
           INA_TEST_ASSERT_EQUAL(INA_ISCP_TYPE_INT64, params->type);
           INA_TEST_ASSERT_EQUAL(20, params->value.i);
       }
       if (__p_count == 2) {
           INA_TEST_ASSERT_EQUAL(INA_ISCP_TYPE_DBL, params->type);
           INA_TEST_ASSERT_EQUAL(5.2, params->value.d);
       } 
       if (__p_count == 3) {
           /*printf("%s", params->value.s);*/
           INA_TEST_ASSERT_EQUAL(INA_ISCP_TYPE_STR, params->type);
           INA_TEST_ASSERT_EQUAL(0, strcmp("test", params->value.s));
       }
       ++params;
   }
   return INA_SUCCESS;
}

void test_iscp_send_tcp() 
{
   /*int server_fd = -1;
   int fd = -1;*/
   ina_iscp_ctx_t *iscp = NULL;

   INA_TRACE_MSG("test_iscp_send_recv_local");
   INA_TEST_ASSERT_SUCCEED(ina_iscp_destroy(&iscp));
   INA_TEST_ASSERT_NULL(iscp);
   INA_TEST_ASSERT_SUCCESS(ina_iscp_create(&iscp, INA_ISCP_INET));
   INA_TEST_ASSERT_NOTNULL(iscp);
   INA_TEST_ASSERT_SUCCEED(ina_iscp_register(iscp, 1, 3, __check_params_handler));
   INA_TEST_ASSERT_SUCCEED(ina_iscp_register(iscp, 2, 0, __stop_handler));


   /*INA_TEST_SPAWN_BEGIN();

   INA_TEST_ASSERT_SUCCEED(ina_net_tcp_connect(&fd, "127.0.0.1", 999));
   iscp.data = &fd;
   INA_TEST_ASSERT_SUCCEED(ina_iscp_send(&iscp, 1, 
                           INA_ISCP_TYPE_INT64, 20,
                           INA_ISCP_TYPE_DBL, 5.2,
                           INA_ISCP_TYPE_STR, "test"));


   INA_TEST_SPAWN_CODE_BEGIN()

   INA_TEST_ASSERT_SUCCEED(ina_net_tcp_server(&server_fd, 999, "127.0.0.1"));
   INA_TEST_ASSERT_SUCCEED(ina_net_nonblock(server_fd));
 
   while (!__stop && server_fd) {
       if (fd != -1) {
           iscp.data = &fd;
           ina_iscp_recv(&iscp, 1000, 1);
           ina_net_close(fd);
           fd = -1;
       }

       ina_time_sleep(100);

       if (fd == -1) {
           if (INA_SUCCEED(ina_net_tcp_accept(&fd, server_fd, NULL, NULL))) {
               if (fd != -1 && !INA_SUCCEED(ina_net_nonblock(fd))) {
                   ina_net_close(fd);
                   fd = -1;
               }
           }
       }
   }
   INA_TEST_SPAWN_CODE_END();
   INA_TEST_SPAWN_END();*/

}
void test_iscp_send_recv_checkparams()
{
    ina_iscp_ctx_t *ctx = NULL;
 
    __send_count = 0;
    __recv_count = 0;
    __p_count = 0;
    __send_msg = NULL;
    __handler_count = 0;

    INA_TRACE_MSG("test_iscp_send_recv_local");
    INA_TEST_ASSERT_SUCCEED(ina_iscp_destroy(&ctx));
    INA_TEST_ASSERT_SUCCESS(ina_iscp_create(&ctx, INA_ISCP_INET));
    INA_TEST_ASSERT_SUCCEED(ina_iscp_set_callbacks(ctx, __null_open_cb, 
                                                   __null_clse_cb, 
                                                   __null_send_cb,
                                                   __null_recv_cb, 
                                                   __null_retn_cb));
    INA_TEST_ASSERT_SUCCEED(ina_iscp_register(ctx, 1, 3, __check_params_handler));

    INA_TEST_ASSERT_SUCCEED(ina_iscp_send(ctx, 1, 
                            INA_ISCP_TYPE_INT64, 20,
                            INA_ISCP_TYPE_DBL, 5.2,
                            INA_ISCP_TYPE_STR, "test"));
    INA_TEST_ASSERT_EQUAL(1, __send_count);
    INA_TEST_ASSERT_NOTNULL(__send_msg);    
    INA_TEST_ASSERT_SUCCEED(ina_iscp_recv(ctx, 1000, 1));
    INA_TEST_ASSERT_EQUAL(1, __recv_count);
    INA_TEST_ASSERT_EQUAL(1, __handler_count);
    INA_TEST_ASSERT_EQUAL(3, __p_count);
}

void test_iscp_send_local()
{
   ina_iscp_ctx_t *ctx = NULL;

   __send_count = 0;

   /*INA_TRACE_MSG("test_iscp_send_local");
   INA_TEST_ASSERT_SUCCEED(ina_iscp_destroy(&ctx));
   INA_TEST_ASSERT_SUCCESS(ina_iscp_create(&ctx, INA_ISCP_INET));
   INA_TEST_ASSERT_FAILURE(ina_iscp_send(ctx, 1, 
                            INA_ISCP_TYPE_INT64, 300,
                            INA_ISCP_TYPE_DBL, 3.2,
                            INA_ISCP_TYPE_STR, "test"));
   INA_TEST_ASSERT_EQUAL(0, __send_count);
   INA_TEST_ASSERT_SUCCESS(ina_iscp_create(&ctx, INA_ISCP_INET));
   INA_TEST_ASSERT_SUCCEED(ina_iscp_set_callbacks(ctx, __null_open_cb, 
                                                   __null_clse_cb, 
                                                   __null_send_cb,
                                                   __null_recv_cb, 
                                                   __null_retn_cb));
   INA_TEST_ASSERT_FAILURE(ina_iscp_send(ctx, 1, 
                           INA_ISCP_TYPE_INT64, 300,
                           INA_ISCP_TYPE_DBL, 3.2,
                           INA_ISCP_TYPE_STR, "test"));
   INA_TEST_ASSERT_SUCCEED(ina_iscp_register(ctx, 1, 3, NULL));
   INA_TEST_ASSERT_SUCCEED(ina_iscp_send(ctx, 1, 
                           INA_ISCP_TYPE_INT64, 20,
                           INA_ISCP_TYPE_DBL, 5.2,
                           INA_ISCP_TYPE_STR, "test-2"));
   INA_TEST_ASSERT_FAILURE(ina_iscp_send(ctx, 1, 
                           INA_ISCP_TYPE_INT64, 20,
                           INA_ISCP_TYPE_STR, "test-2"));
   INA_TEST_ASSERT_EQUAL(1, __send_count);*/
}

void test_iscp_setup()
{
    ina_iscp_ctx_t *ctx = NULL;

    INA_TRACE_MSG("test_iscp_setup");
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
    INA_TEST_ASSERT_SUCCEED(ina_iscp_register(ctx, 1, 3, __null_handler));
    INA_TEST_ASSERT_SUCCEED(ina_iscp_register(ctx, 1, 3, __null_handler));
    INA_TEST_ASSERT_SUCCEED(ina_iscp_register(ctx, 1, 3, __null_handler2));
    /*INA_TEST_ASSERT_FAILURE(ina_iscp_register(ctx, 1, 2, __null_handler));*/
    /*INA_TEST_ASSERT_FAILURE(ina_iscp_register(ctx, 1, 4, __null_handler2));*/
    INA_TEST_ASSERT_SUCCEED(ina_iscp_destroy(&ctx));
    INA_TEST_ASSERT_SUCCESS(ina_iscp_create(&ctx, INA_ISCP_INET));
    INA_TEST_ASSERT_SUCCEED(ina_iscp_register(ctx, 1, 4, __null_handler2));
    INA_TEST_ASSERT_SUCCEED(ina_iscp_set_callbacks(ctx, __null_open_cb, 
                                                    __null_clse_cb, 
                                                    __null_send_cb,
                                                    __null_recv_cb, 
                                                    __null_retn_cb));
    INA_TEST_ASSERT_SUCCEED(ina_iscp_register(ctx, 1, 4, __null_handler2));
}

void test_iscp_regsiter_ex()
{
    ina_iscp_ctx_t *ctx = NULL;
    ina_iscp_cmd_t cmds[] = {
         INA_ISCP_SEND_CMD(1,1),
         INA_ISCP_SENDRECV_CMD(2,3,__null_handler2),
     };
     INA_ISCP_CMDS(cmds2,
          INA_ISCP_SENDRECV_CMD(4,1,__null_handler2),
          INA_ISCP_SENDRECV_CMD(5,3, __null_handler2));
     
     
    INA_TEST_ASSERT_SUCCESS(ina_iscp_create(&ctx, INA_ISCP_NONE));
    INA_TEST_ASSERT_NOTNULL(ctx);
    INA_TEST_ASSERT_SUCCEED(ina_iscp_register_ex(ctx, cmds));
    INA_TEST_ASSERT_SUCCEED(ina_iscp_register_ex(ctx, cmds2));
}