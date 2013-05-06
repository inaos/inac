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

const char *test_xml =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
"<?xml-stylesheet type=\"text/xsl\" href=\"./ClientHTML.xsl\"?><protocol>"
"<version>5.0 for US Messages</version>"
"<revisions>"
"    <revision type=\"Draft\" version=\"5.0\" author=\"Hans Muster\" date=\"07/16/2008\"/>"
"    <revision type=\"Draft\" version=\"5.0b ($Rev: $)\" author=\"Hans Muster\" date=\"07/20/2008\"/>"
"    <revision type=\"Draft\" version=\"5.0c\" author=\"John Doe\" date=\"10/21/2008\"/>"
"    <revision type=\"Release\" version=\"5.0d\" author=\"Hans Muster\" date=\"10/30/2008\"/>"
"    <revision type=\"Release\" version=\"5.0e\" author=\"Hans Muster\" date=\"11/3/2008\"/>"
"    <revision type=\"Release\" version=\"5.0f\" author=\"John Doe\" date=\"12/01/2008\"/>"
"    <revision type=\"Release\" version=\"5.0g\" author=\"John Doe\" date=\"12/08/2008\"/>"
"    <revision type=\"Release\" version=\"5.0h\" author=\"John Doe\" date=\"12/17/2008\"/>"
"    <revision type=\"Release\" version=\"5.0i\" author=\"John Doe\" date=\"01/09/2009\"/>"
"    <revision type=\"Release\" version=\"5.0j\" author=\"John Doe\" date=\"05/04/2009\"/>"
"    <revision type=\"Release\" version=\"5.0k\" author=\"John Doe\" date=\"06/22/2009\"/>"
"    <revision type=\"Beta\" version=\"5.0l\" author=\"Hans Muster\" date=\"02/15/2010\"/>"
"    <revision type=\"Beta\" version=\"5.0m\" author=\"Hans Muster\" date=\"02/25/2010\"/>"
"    <revision type=\"Beta\" version=\"5.0n\" author=\"Hans Muster\" date=\"03/25/2010\"/>"
"    <revision type=\"Beta\" version=\"5.0o\" author=\"Hans Muster\" date=\"04/14/2010\"/>"
"</revisions>";

INA_TEST(xml, simple_xml)
{
	ina_xml_ctx_t *ctx;
	ina_xml_parser_t *parser;
	ina_xml_elem_t *root;
	ina_xml_elem_t *itr;
	ina_str_t source = ina_str_fromcstr(test_xml);
	const char *name;
	const char *value;
	size_t len;	

	INA_TEST_ASSERT_SUCCEED(ina_xml_init(&ctx, 16));
	INA_TEST_ASSERT_SUCCEED(ina_xml_parser_borrow(ctx, &parser));

	INA_TEST_ASSERT_SUCCEED(ina_xml_parser_execute(parser, source, &root)); 

	INA_TEST_ASSERT_SUCCEED(ina_xml_elem_name(root, &name, &len));
	INA_TEST_ASSERT(strncmp("version", name, len) == 0);
	INA_TEST_ASSERT_SUCCEED(ina_xml_elem_value(root, &value, &len));
	INA_TEST_ASSERT(strncmp("5.0 for US Messages", value, len) == 0);

	
	INA_TEST_ASSERT_SUCCEED(ina_xml_elem_next(root, &itr));
	while (itr != NULL) {

		ina_xml_elem_next(itr, &itr);
	}

	INA_TEST_ASSERT_SUCCEED(ina_xml_parser_release(ctx, &parser));
	INA_TEST_ASSERT_SUCCEED(ina_xml_destory(&ctx));	
	
	ina_str_destroy(source);
}
