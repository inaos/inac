
#include "rapidxml.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

/* Enumeration listing all node types produced by the parser. */
typedef enum __rapidxml_node_type_e
{
    RAPIDXML_NODE_TYPE_DOCUMENT,      /* A document node. Name and value are empty. */
    RAPIDXML_NODE_TYPE_ELEMENT,       /* An element node. Name contains element name. Value contains text of first data node. */
    RAPIDXML_NODE_TYPE_DATA,          /* A data node. Name is empty. Value contains data text. */
    RAPIDXML_NODE_TYPE_CDATA,         /* A CDATA node. Name is empty. Value contains data text. */
    RAPIDXML_NODE_TYPE_COMMENT,       /* A comment node. Name is empty. Value contains comment text. */
    RAPIDXML_NODE_TYPE_DECLARATION,   /* A declaration node. Name and value are empty. Declaration parameters (version, encoding and standalone) are in node attributes. */
    RAPIDXML_NODE_TYPE_DOCTYPE,       /* A DOCTYPE node. Name is empty. Value contains DOCTYPE text. */
    RAPIDXML_NODE_TYPE_PI             /* A PI node. Name contains target. Value contains instructions.*/
} __rapidxml_node_type_t;

typedef unsigned char (*test_func)(char c);
typedef unsigned char (*test_func2)(char c1, char c2);

typedef struct __rapidxml_mempool_s {
	rapidxml_parse_error_handler err_handler;
	char *begin;                                      /* Start of raw memory making up current pool */
    char *ptr;                                        /* First free byte in current pool */
    char *end;                                        /* One past last available byte in current pool */
    char static_memory[RAPIDXML_STATIC_POOL_SIZE];    /* Static raw memory */
    rapidxml_alloc_func alloc_func;                   /* Allocator function, or 0 if default is to be used */
    rapidxml_free_func free_func;                     /* Free function, or 0 if default is to be used */
} __rapidxml_mempool_t;

typedef struct __rapidxml_mempool_header_s {
	char *previous_begin;
} __rapidxml_mempool_header_t;

struct rapidxml_attr_s {
	rapidxml_node_t *parent;
	const char *name;
	size_t name_size;
	const char *value;
	size_t value_size;
	rapidxml_attr_t *prev;
	rapidxml_attr_t *next;
} rapidxml_attr_s;

struct rapidxml_node_s {
	rapidxml_node_t *parent;
	__rapidxml_node_type_t type;
	const char *name;
	size_t name_size;
	const char *value;
	size_t value_size;
	rapidxml_attr_t *first_attr;
	rapidxml_attr_t *last_attr;
	rapidxml_node_t *first;
	rapidxml_node_t *last;
	rapidxml_node_t *prev;
	rapidxml_node_t *next;
} rapidxml_node_s;

struct rapidxml_doc_s {
	int flags;
	rapidxml_parse_error_handler err_handler;
	__rapidxml_mempool_t mempool;
	rapidxml_node_t *root;
} rapidxml_doc_s;

/* forward decls */

const unsigned char __lookup_whitespace[256];
const unsigned char __lookup_upcase[256];
const unsigned char __lookup_node_name[256];
const unsigned char __lookup_text[256];
const unsigned char __lookup_attribute_name[256];
const unsigned char __lookup_attribute_data_1[256];
const unsigned char __lookup_attribute_data_2[256];

static rapidxml_node_t *__document_parse_node(rapidxml_doc_t *doc, char *text);

/*
 * Find length of the string
 */
static size_t __measure(const char *p)
{
	const char *tmp = p;
	while (*tmp) {
		++tmp;
	}
	return tmp - p;
}
/*
 * Compare strings for equality
 */
static int __compare(const char *p1, size_t size1, const char *p2, size_t size2, int case_sensitive)
{
	if (size1 != size2) {
		return 0;
	}
	if (case_sensitive) {
		const char *end;
		for (end = p1 + size1; p1 < end; ++p1, ++p2) {
			if (*p1 != *p2) {
				return 0;
			}
		}
    }
	else {
		const char *end;
		for (end = p1 + size1; p1 < end; ++p1, ++p2) {
			if (__lookup_upcase[(unsigned char)(*p1)] != __lookup_upcase[(unsigned char)(*p2)]) {
				return 0;
            }
		}
    }
	return 1;
}
/*
 * 
 */
static char *__mempool_align(char *ptr)
{
	size_t alignment = ((RAPIDXML_ALIGNMENT - ((size_t)(ptr) & (RAPIDXML_ALIGNMENT - 1))) & (RAPIDXML_ALIGNMENT - 1));
	return ptr + alignment;
}
/*
 * 
 */
static void __mempool_init(__rapidxml_mempool_t *pool_ptr, rapidxml_parse_error_handler err_handler)
{
	pool_ptr->err_handler = err_handler;
	pool_ptr->begin = pool_ptr->static_memory;
	pool_ptr->ptr = __mempool_align(pool_ptr->begin);
	pool_ptr->end = pool_ptr->static_memory + sizeof(pool_ptr->static_memory);
}
/*
 * 
 */
static char *__mempool_allocate_raw(__rapidxml_mempool_t *pool_ptr, size_t size)
{
	/* Allocate */
	void *memory;   
	if (pool_ptr->alloc_func) {  /* Allocate memory using either user-specified allocation function or global operator new[] */
		memory = pool_ptr->alloc_func(size);
		assert(memory); /* Allocator is not allowed to return 0, on failure it must either throw, stop the program or use longjmp */
	}
	else {
		memory = malloc(sizeof(char)*size);

        if (!memory) {
			pool_ptr->err_handler("out of memory", 0);
		}

    }
	return (char*)memory;
}
/*
 * 
 */
static void * __mempool_allocate_aligned(__rapidxml_mempool_t *pool_ptr, size_t size)
{
	/* Calculate aligned pointer */
	char *result = __mempool_align(pool_ptr->ptr);

	/* If not enough memory left in current pool, allocate a new pool */
	if (result + size > pool_ptr->end) {
		char *pool;
		size_t alloc_size;
		char *raw_memory;
		__rapidxml_mempool_header_t *new_header;

		/* Calculate required pool size (may be bigger than RAPIDXML_DYNAMIC_POOL_SIZE) */
		size_t pool_size = RAPIDXML_DYNAMIC_POOL_SIZE;
		if (pool_size < size) {
			pool_size = size;
		}

		/* Allocate */
		alloc_size = sizeof(__rapidxml_mempool_header_t) + (2 * RAPIDXML_ALIGNMENT - 2) + pool_size; /* 2 alignments required in worst case: one for header, one for actual allocation */
		raw_memory = __mempool_allocate_raw(pool_ptr, alloc_size);
                    
		/* Setup new pool in allocated memory */
		pool = __mempool_align(raw_memory);
		new_header = (__rapidxml_mempool_header_t*)pool;
		new_header->previous_begin = pool_ptr->begin;
		pool_ptr->begin = raw_memory;
		pool_ptr->ptr = pool + sizeof(__rapidxml_mempool_header_t);
		pool_ptr->end = raw_memory + alloc_size;
		
		/* Calculate aligned pointer again using new pool */
		result = __mempool_align(pool_ptr->ptr);
	}

    /* Update pool and return aligned pointer */
    pool_ptr->ptr = result + size;
    return result;
}
/*
 * 
 */
static rapidxml_node_t *__mempool_allocate_node(__rapidxml_mempool_t *pool_ptr, __rapidxml_node_type_t type, 
										 const char *name, const char *value, size_t name_size, size_t value_size)
{
	void *memory = __mempool_allocate_aligned(pool_ptr, sizeof(rapidxml_node_t));
    rapidxml_node_t *node = (rapidxml_node_t*)memory;
	node->type = type;
    if (name) {
        if (name_size > 0) {
            node->name = name;
			node->name_size = name_size;
		}
		else {
            node->name = name;
			node->name_size = __measure(name);
		}
    }
    if (value) {
        if (value_size > 0) {
			node->value = value;
			node->value_size = value_size;
		}
        else {
            node->value = value;
			node->value_size = __measure(value);
		}
    }
    return node;
}
/*
 * 
 */
static rapidxml_attr_t *__mempool_allocate_attribute(__rapidxml_mempool_t *pool_ptr, const char *name, const char *value, 
													 size_t name_size, size_t value_size)
{
	void *memory = __mempool_allocate_aligned(pool_ptr, sizeof(rapidxml_attr_t));
    rapidxml_attr_t *attribute = (rapidxml_attr_t*)memory;
    if (name) {
        if (name_size > 0) {
            attribute->name = name;
			attribute->name_size = name_size;
			
		}
        else {
            attribute->name = name;
			attribute->name_size = __measure(name);
		}
    }
    if (value)
    {
        if (value_size > 0) {
            attribute->value = value;
			attribute->value_size = value_size;
		}
        else {
            attribute->value = value;
			attribute->value_size = __measure(value);
		}
    }
    return attribute;
}
/*
 * 
 */
static void __mempool_clear(__rapidxml_mempool_t *pool_ptr)
{
	while (pool_ptr->begin != pool_ptr->static_memory) {
		char *previous_begin = ((__rapidxml_mempool_header_t*)__mempool_align(pool_ptr->begin))->previous_begin;
		if (pool_ptr->free_func) {
			pool_ptr->free_func(pool_ptr->begin);
		}
        else {
			free(pool_ptr->begin);
		}
        pool_ptr->begin = previous_begin;
    }
	__mempool_init(pool_ptr, pool_ptr->err_handler);
}
/*
 * 
 */
static rapidxml_attr_t *__attr_previous_attribute(rapidxml_attr_t *attr, const char *name, size_t name_size, int case_sensitive)
{
	if (name) {
		rapidxml_attr_t *attribute;
        if (name_size == 0) {
            name_size = __measure(name);
		}
		for (attribute = attr->prev; attribute; attribute = attribute->prev) {
            if (__compare(attribute->name, attribute->name_size, name, name_size, case_sensitive)) {
                return attribute;
			}
		}
        return NULL;
    }
    else {
		if (attr->parent) {
			return attr->prev;
		}
		else {
			return NULL;
		}
	}
}
/*
 * 
 */
static rapidxml_attr_t *__attr_next_attribute(rapidxml_attr_t *attr, const char *name, size_t name_size, int case_sensitive)
{
	if (name) {
		rapidxml_attr_t *attribute;
        if (name_size == 0) {
            name_size = __measure(name);
		}
		for (attribute = attr->next; attribute; attribute = attribute->next) {
            if (__compare(attribute->name, attribute->name_size, name, name_size, case_sensitive)) {
                return attribute;
			}
		}
        return NULL;
    }
    else {
        if (attr->parent) {
			return attr->next;
		}
		else {
			return NULL;
		}
	}
}
/*
 * 
 */
static rapidxml_node_t *__node_next_sibling(rapidxml_node_t *node, const char *name, size_t name_size, int case_sensitive)
{
	assert(node->parent);	/* Cannot query for siblings if node has no parent */
	if (name) {
		rapidxml_node_t *sibling;
		if (name_size == 0) {
            name_size = __measure(name);
		}
		for (sibling = node->next; sibling; sibling = sibling->next) {
            if (__compare(sibling->name, sibling->name_size, name, name_size, case_sensitive)) {
                return sibling;
			}
		}
        return NULL;
    }
    else {
		return node->next;
	}
}
/*
 * 
 */
static rapidxml_node_t *__node_previous_sibling(rapidxml_node_t *node, const char *name, size_t name_size, int case_sensitive)
{
	assert(node->parent);	/* Cannot query for siblings if node has no parent */
    if (name) {
		rapidxml_node_t *sibling;
        if (name_size == 0) {
            name_size = __measure(name);
		}
		for (sibling = node->prev; sibling; sibling = sibling->prev) {
            if (__compare(sibling->name, sibling->name_size, name, name_size, case_sensitive)) {
                return sibling;
			}
		}
        return  NULL;
    }
    else {
		return node->prev;
	}
}
/*
 * 
 */
static rapidxml_node_t *__node_first_node(rapidxml_node_t *node, const char *name, size_t name_size, int case_sensitive)
{
	if (name) {
		rapidxml_node_t *child;
        if (name_size == 0) {
            name_size = __measure(name);
		}
		for (child = node->first; child; child = __node_next_sibling(child, NULL, 0, 1)) {
            if (__compare(child->name, child->name_size, name, name_size, case_sensitive)) {
                return child;
			}
		}
        return NULL;
    }
    else {
		return node->first;
	}
}
/*
 * 
 */
static rapidxml_node_t *__node_last_node(rapidxml_node_t *node, const char *name, size_t name_size, int case_sensitive)
{
    assert(node->first);  /* Cannot query for last child if node has no children */
    if (name) {
		rapidxml_node_t *child;
        if (name_size == 0) {
            name_size = __measure(name);
		}
		for (child = node->last; child; child = __node_previous_sibling(child, NULL, 0, 1)) {
            if (__compare(child->name, child->name_size, name, name_size, case_sensitive)) {
                return child;
			}
		}
        return NULL;
    }
    else {
		return node->last;
	}
}
/*
 * 
 */
static rapidxml_attr_t *__node_first_attribute(rapidxml_node_t *node, const char *name, size_t name_size, int case_sensitive)
{
	if (name) {
		rapidxml_attr_t *attribute;
        if (name_size == 0) {
            name_size = __measure(name);
		}
		for (attribute = node->first_attr; attribute; attribute = attribute->next) {
            if (__compare(attribute->name, attribute->name_size, name, name_size, case_sensitive)) {
                return attribute;
			}
		}
        return NULL;
    }
    else {
		return node->first_attr;
	}
}
/*
 * 
 */
static rapidxml_attr_t *__node_last_attribute(rapidxml_node_t *node, const char *name, size_t name_size, int case_sensitive)
{
	if (name) {
		rapidxml_attr_t *attribute;
        if (name_size == 0) {
            name_size = __measure(name);
		}
        for (attribute = node->last_attr; attribute; attribute = attribute->prev) {
            if (__compare(attribute->name, attribute->name_size, name, name_size, case_sensitive)) {
                return attribute;
			}
		}
        return NULL;
    }
    else {
		if (node->first_attr) {
			return node->last_attr;
		}
		else {
			return NULL;
		}
	}
}
static void __node_append_node(rapidxml_node_t *ptr, rapidxml_node_t *child)
{
	assert(child && !child->parent && child->type != RAPIDXML_NODE_TYPE_DOCUMENT);
    if (__node_first_node(ptr, NULL, 0, 1)) {
		child->prev = ptr->last;
		ptr->last->next = child;
    }
    else {
		child->prev = NULL;
		ptr->first = child;
    }
	ptr->last = child;
	child->parent = ptr;
    child->next = NULL;
}
/*
 * 
 */
static void __node_append_attribute(rapidxml_node_t *node, rapidxml_attr_t *attribute)
{
    assert(attribute && !attribute->parent);
    if (__node_first_attribute(node, NULL, 0, 1)) {
		attribute->prev = node->last_attr;
		node->last_attr->next = attribute;
    }
    else {
		attribute->prev = NULL;
		node->first_attr = attribute;
    }
	node->last_attr = attribute;
	attribute->parent = node;
	attribute->next = NULL;
}
/*
 * 
 */
static void __node_remove_all_nodes(rapidxml_node_t *ptr)
{
	rapidxml_node_t *node;
	for (node = __node_first_node(ptr, NULL, 0, 1); node; node = node->next) {
		node->parent = NULL;
	}
	ptr->first = NULL;
}
/*
 * 
 */
static void __node_remove_all_attributes(rapidxml_node_t *node)
{
	rapidxml_attr_t *attribute;
	for (attribute = __node_first_attribute(node, NULL, 0, 1); attribute; attribute = attribute->next) {
		attribute->parent = NULL;
	}
	node->first_attr = NULL;
}
/*
 *
 */
static void __document_parse_bom(char *text)
{
    /* UTF-8? */
    if ((unsigned char)text[0] == 0xEF && 
        (unsigned char)text[1] == 0xBB && 
        (unsigned char)text[2] == 0xBF)
    {
        *text += 3;      /* Skip utf-8 bom */
    }
}
/*
 *
 */
static unsigned char __test_whitespace(char c)
{
	return __lookup_whitespace[c];
}
/*
 *
 */
static unsigned char __test_node_name_pred(char c)
{
	return __lookup_node_name[c];
}
/*
 *
 */
static unsigned char __test_attr_name_pred(char c)
{
	return __lookup_attribute_name[c];
}
/*
 *
 */
static unsigned char __test_attr_value_pred(char quote, char c)
{
	if (quote == '\'') {
		return __lookup_attribute_data_1[c];
	}
	else {
		return __lookup_attribute_data_2[c];
	}
}
/*
 *
 */
static unsigned char __test_text_pred(char c)
{
	return __lookup_text[c];
}
/*
 *
 */
static void __document_skip(test_func test, char *text)
{
    char *tmp = text;
    while (test(*tmp)) {
        ++tmp;
	}
}
/*
 *
 */
static void __document_skip2(test_func2 test, char arg, char *text)
{
    char *tmp = text;
    while (test(arg, *tmp)) {
        ++tmp;
	}
}
/*
 *
 */
static void __document_parse_node_attributes(rapidxml_doc_t *doc, rapidxml_node_t *node, char *text)
{
	rapidxml_attr_t *attribute;

	/* For all attributes */
	while (__test_attr_name_pred(*text))
    {
		char quote;
		char *value, *end;

        /* Extract attribute name */
        char *name = text;
        ++text;     /* Skip first character of attribute name */
		__document_skip(__test_attr_name_pred, text);
        if (text == name) {
			doc->err_handler("expected attribute name", name);
		}

        /* Create new attribute */
        attribute = __mempool_allocate_attribute(&doc->mempool, NULL, NULL, 0, 0);
        attribute->name = name;
		attribute->name_size = text - name;
		
		__node_append_attribute(node, attribute);

        /* Skip whitespace after attribute name */
		__document_skip(__test_whitespace, text);

        /* Skip = */
        if (*text != '=') {
			doc->err_handler("expected =", text);
		}
        ++text;

        /* Skip whitespace after = */
        __document_skip(__test_whitespace, text);

        /* Skip quote and remember if it was ' or " */
        quote = *text;
        if (quote != '\'' && quote != '"') {
			doc->err_handler("expected ' or \"", text);
		}
        ++text;

        /* Extract attribute value and expand char refs in it */
        value = text;
		end = text;
        if (quote == '\'') {
			__document_skip2(__test_attr_value_pred, '\'', text);
		}
        else {
			__document_skip2(__test_attr_value_pred, '"', text);
		}
                
        /* Set attribute value */
        attribute->value = value;
		attribute->value_size = end - value;
                
        /* Make sure that end quote is present */
        if (*text != quote) {
            doc->err_handler("expected ' or \"", text);
		}
        ++text;     /* Skip quote */

        /* Skip whitespace after attribute value */
        __document_skip(__test_whitespace, text);
    }
}
/*
 *
 */
static char __document_parse_and_append_data(rapidxml_node_t *node, char *text)
{
    /* Skip until end of data */
    char *value = text, *end;
    
	__document_skip(__test_text_pred, text);
	end = text;

	if (node->value == NULL) {
		node->value = value;
		node->value_size = end - value;
	}     
    
    /* Return character that ends data */
    return *text;
}
/*
 *
 */
static void __document_parse_node_content(rapidxml_doc_t *doc, rapidxml_node_t *node, char *text)
{
	/* For all children and text */
    while (1)
    {
		char next_char;

        /* Skip whitespace between > and node contents */
        __document_skip(__test_whitespace, text);
        next_char = *text;

    /*
	 * After data nodes, instead of continuing the loop, control jumps here.
     * This is because zero termination inside parse_and_append_data() function
     * would wreak havoc with the above code.
     * Also, skipping whitespace after data nodes is unnecessary.
	 */
    after_data_node: 
                
        /* Determine what comes next: node closing, child node, data node, or 0? */
        switch (next_char)
        {
                
        /* Node closing or child node */
        case '<':
            if (text[1] == '/') {
                /* Node closing */
                text += 2;      /* Skip '</' */
				if (doc->flags & RAPIDXML_PARSE_FLAG_VALIDATE_CLOSING_TAGS)
                {
                    /* Skip and validate closing tag name */
                    char *closing_name = text;
					__document_skip(__test_node_name_pred, text);
                    if (!__compare(node->name, node->name_size, closing_name, text - closing_name, 1)) {
						doc->err_handler("invalid closing tag name", text);
					}
                }
                else
                {
                    /* No validation, just skip name */
                    __document_skip(__test_node_name_pred, text);
                }
                /* Skip remaining whitespace after node name */
                __document_skip(__test_whitespace, text);
                if (*text != '>') {
					doc->err_handler("expected >", text);
				}
                ++text;     /* Skip '>' */
                return;     /* Node closed, finished parsing contents */
            }
            else {
				rapidxml_node_t *child;
                /* Child node */
                ++text;     /* Skip '<' */
                if (child = __document_parse_node(doc, text)) {
                    __node_append_node(node, child);
				}
            }
            break;

        /* End of data - error */
        case '\0':
			doc->err_handler("unexpected end of data", text);

        /* Data node */
        default:
            next_char = __document_parse_and_append_data(node, text);
            goto after_data_node;   /* Bypass regular processing after data nodes */

        }
    }
}
/*
 *
 */
static rapidxml_node_t *__document_parse_element(rapidxml_doc_t *doc, char *text)
{
	/* Create element node */
	rapidxml_node_t *element = __mempool_allocate_node(&doc->mempool, RAPIDXML_NODE_TYPE_ELEMENT, NULL, NULL, 0, 0);

    /* Extract element name */
    char *name = text;
	__document_skip(__test_node_name_pred, text);
    if (text == name) {
		doc->err_handler("expected element name", text);
	}
    element->name = name;
	element->name_size = text - name;
            
    /* Skip whitespace between element name and attributes or > */
	__document_skip(__test_whitespace, text);

    /* Parse attributes, if any */
    __document_parse_node_attributes(doc, element, text);

    /* Determine ending type */
    if (*text == '>') {
        ++text;
		__document_parse_node_content(doc, element, text);
    }
    else if (*text == '/') {
        ++text;
        if (*text != '>') {
			doc->err_handler("expected >", text);
		}
        ++text;
    }
    else {
		doc->err_handler("expected >", text);
	}

    /* Return parsed element */
    return element;
}
/*
 *
 */
static rapidxml_node_t *__document_parse_declaration(rapidxml_doc_t *doc, char *text)
{
	rapidxml_node_t *declaration;
	/* If parsing of declaration is disabled */
	if (!(doc->flags & RAPIDXML_PARSE_FLAG_DECLARATION_NODE))
    {
        /* Skip until end of declaration */
        while (text[0] != '?' || text[1] != '>') {
            if (!text[0]) {
				doc->err_handler("unexpected end of data", text);
			}
            ++text;
        }
        text += 2;    /* Skip '?>' */
        return 0;
    }

    /* Create declaration */
    declaration = __mempool_allocate_node(&doc->mempool, RAPIDXML_NODE_TYPE_DECLARATION, NULL, NULL, 0, 0);

    /* Skip whitespace before attributes or ?> */
	__document_skip(__test_whitespace, text);

    /* Parse declaration attributes */
	__document_parse_node_attributes(doc, declaration, text);
            
    /* Skip ?> */
    if (text[0] != '?' || text[1] != '>') {
		doc->err_handler("expected ?>", text);
	}
    text += 2;
            
    return declaration;
}
/*
 *
 */
static rapidxml_node_t *__document_parse_pi(rapidxml_doc_t *doc, char *text)
{
	/* If creation of PI nodes is enabled */
	if (doc->flags & RAPIDXML_PARSE_FLAG_PI_NODES) {
		char *value;
        /* Create pi node */
        rapidxml_node_t *pi = __mempool_allocate_node(&doc->mempool, RAPIDXML_NODE_TYPE_PI, NULL, NULL, 0, 0);

        /* Extract PI target name */
        char *name = text;
		__document_skip(__test_node_name_pred, text);
        if (text == name) {
			doc->err_handler("expected PI target", text);
		}
        pi->name = name;
		pi->name_size = text - name;
                
        /* Skip whitespace between pi target and pi */
		__document_skip(__test_whitespace, text);

        /* Remember start of pi */
        value = text;
                
        /* Skip to '?>' */
        while (text[0] != '?' || text[1] != '>') {
            if (*text == '\0') {
				doc->err_handler("unexpected end of data", text);
			}
            ++text;
        }

        /* Set pi value (verbatim, no entity expansion or whitespace normalization) */
        pi->value = value;
		pi->value_size = text - value;
                
        text += 2; /* Skip '?>' */
        return pi;
    }
    else {
        /* Skip to '?>' */
        while (text[0] != '?' || text[1] != '>') {
            if (*text == '\0') {
				doc->err_handler("unexpected end of data", text);
			}
            ++text;
        }
        text += 2;  /* Skip '?>' */
        return NULL;
    }
}
/*
 *
 */
static rapidxml_node_t *__document_parse_comment(rapidxml_doc_t *doc, char *text)
{
	char *value;
	rapidxml_node_t *comment;

	/* If parsing of comments is disabled */
	if (!(doc->flags & RAPIDXML_PARSE_FLAG_COMMENT_NODES)) {
        /* Skip until end of comment */
        while (text[0] != '-' || text[1] != '-' || text[2] != '>') {
            if (!text[0]) {
				doc->err_handler("unexpected end of data", text);
			}
            ++text;
        }
        text += 3;     /* Skip '-->' */
        return NULL;   /* Do not produce comment node */
    }

    /* Remember value start */
    value = text;

    /* Skip until end of comment */
    while (text[0] != '-' || text[1] != '-' || text[2] != '>') {
        if (!text[0]) {
			doc->err_handler("unexpected end of data", text);
		}
        ++text;
    }

    /* Create comment node */
    comment = __mempool_allocate_node(&doc->mempool, RAPIDXML_NODE_TYPE_COMMENT, NULL, NULL, 0, 0);
    comment->value = value;
	comment->value_size = text - value;
            
    text += 3;     /* Skip '-->' */

    return comment;
}
/*
 *
 */
static rapidxml_node_t *__document_parse_cdata(rapidxml_doc_t *doc, char *text)
{
	char *value;
	rapidxml_node_t *cdata;

	/* If CDATA is disabled */
	if (doc->flags & RAPIDXML_PARSE_FLAG_NO_DATA_NODES) {
        /* Skip until end of cdata */
        while (text[0] != ']' || text[1] != ']' || text[2] != '>') {
            if (!text[0]) {
				doc->err_handler("unexpected end of data", text);
			}
            ++text;
        }
        text += 3;      /* Skip ]]> */
        return 0;       /* Do not produce CDATA node */
    }

    /* Skip until end of cdata */
    value = text;
    while (text[0] != ']' || text[1] != ']' || text[2] != '>') {
        if (!text[0]) {
			doc->err_handler("unexpected end of data", text);
		}
        ++text;
    }

    /* Create new cdata node */
    cdata = __mempool_allocate_node(&doc->mempool, RAPIDXML_NODE_TYPE_CDATA, NULL, NULL, 0, 0);
	cdata->value = value;
	cdata->value_size = text - value;

    text += 3;      /* Skip ]]> */
    return cdata;
}
/*
 *
 */
static rapidxml_node_t *__document_parse_doctype(rapidxml_doc_t *doc, char *text)
{
	/* Remember value start */
    char *value = text;
	int depth;

    /* Skip to > */
    while (*text != '>') {
        /* Determine character type */
        switch (*text)
        {
                
        /* If '[' encountered, scan for matching ending ']' using naive algorithm with depth */
        /* This works for all W3C test files except for 2 most wicked */
        case '[':
        {
            ++text;     /* Skip '[' */
            depth = 1;
            while (depth > 0) {
                switch (*text)
                {
                    case '[': ++depth; break;
                    case ']': --depth; break;
					case 0: doc->err_handler("unexpected end of data", text);
                }
                ++text;
            }
            break;
        }
                
        /* Error on end of text */
        case '\0':
			doc->err_handler("unexpected end of data", text);
                
        /* Other character, skip it */
        default:
            ++text;

        }
    }
            
    /* If DOCTYPE nodes enabled */
    if (doc->flags & RAPIDXML_PARSE_FLAG_DOCTYPE_NODES) {
        /* Create a new doctype node */
        rapidxml_node_t *doctype = __mempool_allocate_node(&doc->mempool, RAPIDXML_NODE_TYPE_DOCTYPE, NULL, NULL, 0, 0);
        doctype->value = value;
		doctype->value_size = text - value;

        text += 1;      /* skip '>' */
        return doctype;
    }
    else {
        text += 1;      /* skip '>' */
        return 0;
    }
}
/*
 *
 */
static rapidxml_node_t *__document_parse_node(rapidxml_doc_t *doc, char *text)
{
	/* Parse proper node type */
    switch (text[0])
    {

    /* <... */
    default: 
        /* Parse and append element node */
		return __document_parse_element(doc, text);

    /* <?... */
    case '?': 
        ++text;     /* Skip ? */
        if ((text[0] == 'x' || text[0] == 'X') &&
            (text[1] == 'm' || text[1] == 'M') && 
            (text[2] == 'l' || text[2] == 'L') &&
            __test_whitespace(text[3]))
        {
            /* '<?xml ' - xml declaration */
            text += 4;      /* Skip 'xml ' */
            return __document_parse_declaration(doc, text);
        }
        else
        {
            /* Parse PI */
            return __document_parse_pi(doc, text);
        }
            
    /* <!... */
    case '!': 

        /* Parse proper subset of <! node */
        switch (text[1])    
        {
                
        /* <!- */
        case '-':
            if (text[2] == '-')
            {
                /* '<!--' - xml comment */
                text += 3;     /* Skip '!--' */
                return __document_parse_comment(doc, text);
            }
            break;

        /* <![ */
        case '[':
            if (text[2] == 'C' && text[3] == 'D' && text[4] == 'A' && 
                text[5] == 'T' && text[6] == 'A' && text[7] == '[')
            {
                /* '<![CDATA[' - cdata */
                text += 8;     /* Skip '![CDATA[' */
                return __document_parse_cdata(doc, text);
            }
            break;

        /* <!D */
        case 'D':
            if (text[2] == 'O' && text[3] == 'C' && text[4] == 'T' && 
                text[5] == 'Y' && text[6] == 'P' && text[7] == 'E' && 
                __test_whitespace(text[8]))
            {
                /* '<!DOCTYPE ' - doctype */
                text += 9;      /* skip '!DOCTYPE ' */
                return __document_parse_doctype(doc, text);
            }

        }   /* switch */

        /* Attempt to skip other, unrecognized node types starting with <! */
        ++text;     /* Skip ! */
        while (*text != '>')
        {
            if (*text == 0) {
				doc->err_handler("unexpected end of data", text);
			}
            ++text;
        }
        ++text;     /* Skip '>' */
        return NULL;   /* No node recognized */

    }
}
/*
 *
 */
static void __document_parse(rapidxml_doc_t *doc, char *text)
{
	assert(text);
            
    /* Remove current contents */
    __node_remove_all_nodes(doc->root);
	__node_remove_all_attributes(doc->root);
            
    /* Parse BOM, if any */
    __document_parse_bom(text);

    /* Parse children */
    while (1) {

        /* Skip whitespace before node */
        __document_skip(__test_whitespace, text);
        if (*text == 0) {
            break;
		}

        /* Parse and append new child */
        if (*text == '<')
        {
			rapidxml_node_t *node;
            ++text;     /* Skip '<' */
            if (node = __document_parse_node(doc, text)) {
				__node_append_node(doc->root, node);
			}
        }
        else {
			doc->err_handler("expected <", text);
		}
    }
}
/*
 *
 */
static void __document_clean(rapidxml_doc_t *doc)
{
	__node_remove_all_nodes(doc->root);
	__node_remove_all_attributes(doc->root);
	__mempool_clear(&doc->mempool);
}
/*
 *
 */
static void __default_error_handler(const char *what, const char *where)
{
	printf("Parse error: %s\n", what);
	abort();
}

/* PUBLIC API */

int rapidxml_parser_init(rapidxml_doc_t **doc, int flags, rapidxml_parse_error_handler err_handler, 
						 rapidxml_alloc_func alloc_fun, rapidxml_free_func free_fun)
{
	if (alloc_fun) {
		*doc = (rapidxml_doc_t*)alloc_fun(sizeof(rapidxml_doc_t));
		(*doc)->mempool.alloc_func = alloc_fun;
	}
	else {
		*doc = (rapidxml_doc_t*)malloc(sizeof(rapidxml_doc_t));
		(*doc)->mempool.alloc_func = NULL;
	}
	if (free_fun) {
		(*doc)->mempool.free_func = free_fun;
	}
	else {
		(*doc)->mempool.free_func = NULL;
	}
	if (err_handler) {
		(*doc)->err_handler = err_handler;
	}
	else {
		(*doc)->err_handler = __default_error_handler;
	}
	(*doc)->root = NULL;
	(*doc)->flags = flags;
	__mempool_init(&(*doc)->mempool, (*doc)->err_handler);

	return 0;
}

int rapidxml_parser_destroy(rapidxml_doc_t **doc)
{
	__mempool_clear(&(*doc)->mempool);
	if ((*doc)->mempool.free_func) {
		(*doc)->mempool.free_func(*doc);
	}
	else {
		free(*doc);
	}
	return 0;
}

int rapidxml_parser_exec(rapidxml_doc_t *doc, const char *data)
{
	/* we're not changing any data but something we need to reset the pointers */
	__document_parse(doc, (char*)data);

	return 0;
}

int rapidxml_parser_reset(rapidxml_doc_t *doc)
{
	__document_clean(doc);

	return 0;
}

int rapidxml_parser_root(rapidxml_doc_t *doc, rapidxml_node_t **root)
{
	*root = doc->root;
	return 0;
}

int rapidxml_node_next(rapidxml_node_t *node, rapidxml_node_t **next)
{
	*next = __node_next_sibling(node, NULL, 0, 1);
	return 0;
}

int rapidxml_node_get_name(rapidxml_node_t *node, const char **name, size_t *len)
{
	*name = node->name;
	*len = node->name_size;
	return 0;
}

int rapidxml_node_get_value(rapidxml_node_t *node, const char **value, size_t *len)
{
	*value = node->value;
	*len = node->value_size;
	return 0;
}

int rapidxml_node_first_attribute(rapidxml_node_t *node, rapidxml_attr_t **attr)
{
	*attr = __node_first_attribute(node, NULL, 0, 1);
	return 0;
}

int rapidxml_attribute_next(rapidxml_attr_t *attr, rapidxml_attr_t **next)
{
	*next = __attr_next_attribute(attr, NULL, 0, 1);
	return 0;
}

int rapidxml_attribute_get_name(rapidxml_attr_t *attr, const char **name, size_t *len)
{
	*name = attr->name;
	*len = attr->name_size;
	return 0;
}

int rapidxml_attribute_get_value(rapidxml_attr_t *attr, const char **value, size_t *len)
{
	*value = attr->value;
	*len = attr->value_size;
	return 0;
}

/* Whitespace table */
const unsigned char __lookup_whitespace[256] = {
 /* 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  0,  0,  1,  0,  0,  /* 0 */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 1 */
    1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 2 */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 3 */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 4 */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 5 */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 6 */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 7 */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 8 */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 9 */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* A */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* B */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* C */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* D */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* E */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0   /* F */
};
/* Upper case conversion */
const unsigned char __lookup_upcase[256] = 
{
	/* 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  A   B   C   D   E   F */
	0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,   /* 0 */
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,   /* 1 */
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,   /* 2 */
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,   /* 3 */
	64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,   /* 4 */
	80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,   /* 5 */
	96, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,   /* 6 */
	80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 123,124,125,126,127,  /* 7 */
	128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,  /* 8 */
	144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,  /* 9 */
	160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,  /* A */
	176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,  /* B */
	192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,  /* C */
	208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,  /* D */
	224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,  /* E */
	240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255   /* F */
};
/* Node name (anything but space \n \r \t / > ? \0) */
const unsigned char __lookup_node_name[256] = 
{
 /* 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F */
    0,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  1,  1,  0,  1,  1,  /* 0 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 1 */
    0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  /* 2 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  /* 3 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 4 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 5 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 6 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 7 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 8 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 9 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* A */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* B */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* C */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* D */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* E */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1   /* F */
};
/* Text (i.e. PCDATA) (anything but < \0) */
const unsigned char __lookup_text[256] = 
{
 /* 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F */
    0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 0 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 1 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 2 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  1,  1,  1,  /* 3 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 4 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 5 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 6 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 7 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 8 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 9 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* A */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* B */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* C */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* D */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* E */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1   /* F */
};
/* Attribute name (anything but space \n \r \t / < > = ? ! \0) */
const unsigned char __lookup_attribute_name[256] = 
{
 /* 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F */
    0,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  1,  1,  0,  1,  1,  /* 0 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 1 */
    0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  /* 2 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  /* 3 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 4 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 5 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 6 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 7 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 8 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 9 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* A */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* B */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* C */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* D */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* E */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1   /* F */
};
/* Attribute data with single quote (anything but ' \0) */
const unsigned char __lookup_attribute_data_1[256] = 
{
 /* 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F */
    0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 0 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 1 */
    1,  1,  1,  1,  1,  1,  1,  0,  1,  1,  1,  1,  1,  1,  1,  1,  /* 2 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 3 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 4 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 5 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 6 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 7 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 8 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 9 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* A */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* B */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* C */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* D */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* E */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1   /* F */
};
/* Attribute data with double quote (anything but " \0) */
const unsigned char __lookup_attribute_data_2[256] = 
{
 /* 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F */
    0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 0 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 1 */
    1,  1,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 2 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 3 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 4 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 5 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 6 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 7 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 8 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 9 */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* A */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* B */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* C */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* D */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* E */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1   /* F */
};