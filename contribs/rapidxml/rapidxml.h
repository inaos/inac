#ifndef __RAPIDXML_H__
#define __RAPIDXML_H__

#include <stddef.h>

#ifndef RAPIDXML_STATIC_POOL_SIZE
    /* 
	 * Size of static memory block of memory_pool.
     * Define RAPIDXML_STATIC_POOL_SIZE before including rapidxml.h if you want to override the default value.
     * No dynamic memory allocations are performed by memory_pool until static memory is exhausted.
	 */
    #define RAPIDXML_STATIC_POOL_SIZE (64 * 1024)
#endif

#ifndef RAPIDXML_DYNAMIC_POOL_SIZE
    /*
	 * Size of dynamic memory block of memory_pool.
     * Define RAPIDXML_DYNAMIC_POOL_SIZE before including rapidxml.hpp if you want to override the default value.
     * After the static block is exhausted, dynamic blocks with approximately this size are allocated by memory_pool.
	 */
    #define RAPIDXML_DYNAMIC_POOL_SIZE (64 * 1024)
#endif

#ifndef RAPIDXML_ALIGNMENT
    /*
	 * Memory allocation alignment.
     * Define RAPIDXML_ALIGNMENT before including rapidxml.hpp if you want to override the default value, which is the size of pointer.
     * All memory allocations for nodes, attributes and strings will be aligned to this value.
     * This must be a power of 2 and at least 1, otherwise memory_pool will not work.
	 */
    #define RAPIDXML_ALIGNMENT sizeof(void *)
#endif

typedef enum rapidxml_parse_flags_e {
	RAPIDXML_PARSE_FLAG_NO_UTF8 = 0x1,
	RAPIDXML_PARSE_FLAG_DECLARATION_NODE = 0x2,
	RAPIDXML_PARSE_FLAG_COMMENT_NODES = 0x4,
	RAPIDXML_PARSE_FLAG_DOCTYPE_NODES = 0x8,
	RAPIDXML_PARSE_FLAG_PI_NODES = 0x10,
	RAPIDXML_PARSE_FLAG_VALIDATE_CLOSING_TAGS = 0x20,
	RAPIDXML_PARSE_FLAG_NO_DATA_NODES = 0x40
} rapidxml_parse_flags_t;

typedef void (*rapidxml_parse_error_handler)(const char *what, const char *where);
typedef void *(*rapidxml_alloc_func)(size_t);
typedef void (*rapidxml_free_func)(void *);

/* opaque */
typedef struct rapidxml_attr_s rapidxml_attr_t;

/* opaque */
typedef struct rapidxml_node_s rapidxml_node_t;

/* opaque */
typedef struct rapidxml_doc_s rapidxml_doc_t;

int rapidxml_parser_init(rapidxml_doc_t **doc, int flags, rapidxml_parse_error_handler err_handler, 
						 rapidxml_alloc_func alloc_fun, rapidxml_free_func free_fun);

int rapidxml_parser_exec(rapidxml_doc_t *doc, const char *data);

int rapidxml_parser_reset(rapidxml_doc_t *doc);

int rapidxml_parser_root(rapidxml_doc_t *doc, rapidxml_node_t **root);

int rapidxml_node_first(rapidxml_node_t *node, rapidxml_node_t **first);

int rapidxml_node_next(rapidxml_node_t *node, rapidxml_node_t **next);

int rapidxml_node_get_name(rapidxml_node_t *node, const char **name, size_t *len);

int rapidxml_node_get_value(rapidxml_node_t *node, const char **value, size_t *len);

int rapidxml_node_first_attribute(rapidxml_node_t *node, rapidxml_attr_t **attr);

int rapidxml_attribute_next(rapidxml_attr_t *attr, rapidxml_attr_t **next);

int rapidxml_attribute_get_name(rapidxml_attr_t *attr, const char **name, size_t *len);

int rapidxml_attribute_get_value(rapidxml_attr_t *attr, const char **value, size_t *len);

int rapidxml_parser_destroy(rapidxml_doc_t **doc);

#endif
