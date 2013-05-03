#include "rapidxml.h"
#include "rapidxml.hpp"

using namespace rapidxml;

typedef struct rapidxml_attr_s {
	xml_attribute<> *attr;
} rapidxml_attr_t;

typedef struct rapidxml_node_s {
	xml_node<> *node;
	rapidxml_attr_t attr_ptr;
} rapidxml_node_t;

typedef struct rapidxml_doc_s {
	xml_document<> *doc;
	rapidxml_node_t node_ptr;
} rapidxml_doc_t;

int rapidxml_parser_init(rapidxml_doc_t **doc)
{
	*doc = (rapidxml_doc_t*)malloc(sizeof(rapidxml_doc_t));
	
	(*doc)->doc = new xml_document<>();
	
	return 0;
}

int rapidxml_parser_exec(rapidxml_doc_t *doc, const char *data)
{
	/* 
	 * since we know that we parse in non-destructive more 
	 * we can cast from the const char to char - we want 
	 * const char in the api because its nicer.
	 */
	doc->doc->parse<parse_fastest>((char*)data);
	return 0;
}

int rapidxml_parser_reset(rapidxml_doc_t *doc)
{
	doc->doc->clear();
	return 0;
}

int rapidxml_parser_first_child(rapidxml_doc_t *doc, rapidxml_node_t **node)
{
	doc->node_ptr.node = doc->doc->first_node();
	*node = &doc->node_ptr;
	return 0;
}

int rapidxml_parser_next_child(rapidxml_doc_t *doc, rapidxml_node_t **node)
{
	doc->node_ptr.node = doc->node_ptr.node->next_sibling();
	*node = &doc->node_ptr;
	return 0;
}

int rapidxml_node_get_name(rapidxml_node_t *node, const char **name, size_t *len)
{
	*name = node->node->name();
	*len = node->node->name_size();
	return 0;
}

int rapidxml_node_get_value(rapidxml_node_t *node, const char **value, size_t *len)
{
	*value = node->node->value();
	*len = node->node->value_size();
	return 0;
}

int rapidxml_node_first_attribute(rapidxml_node_t *node, rapidxml_attr_t **attr)
{
	node->attr_ptr.attr = node->node->first_attribute();
	*attr = &node->attr_ptr;
	return 0;
}

int rapidxml_node_next_attribute(rapidxml_node_t *node, rapidxml_attr_t **attr)
{
	node->attr_ptr.attr = node->attr_ptr.attr->next_attribute();
	*attr = &node->attr_ptr;
	return 0;
}

int rapidxml_attribute_get_name(rapidxml_attr_t *attr, const char **name, size_t *len)
{
	*name = attr->attr->name();
	*len = attr->attr->name_size();
	return 0;
}

int rapidxml_attribute_get_value(rapidxml_attr_t *attr, const char **value, size_t *len)
{
	*value = attr->attr->value();
	*len = attr->attr->value_size();
	return 0;
}

int rapidxml_parser_destroy(rapidxml_doc_t **doc)
{
	delete (*doc)->doc;
	free(*doc);
	return 0;
}
