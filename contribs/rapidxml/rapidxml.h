#ifndef __RAPIDXML_H__
#define __RAPIDXML_H__

#ifdef WIN32
    #define RAPIDXML_EXPORT_SHARED __declspec(dllexport)
#else
    #ifdef __cplusplus
        #define RAPIDXML_EXPORT_SHARED extern "C"
    #else
        #define RAPIDXML_EXPORT_SHARED
    #endif
    #include <sys/types.h>
#endif

/* opaque */
typedef struct rapidxml_attr_s rapidxml_attr_t;

/* opaque */
typedef struct rapidxml_node_s rapidxml_node_t;

/* opaque */
typedef struct rapidxml_doc_s rapidxml_doc_t;

RAPIDXML_EXPORT_SHARED int rapidxml_parser_init(rapidxml_doc_t **doc);

RAPIDXML_EXPORT_SHARED int rapidxml_parser_exec(rapidxml_doc_t *doc, const char *data);

RAPIDXML_EXPORT_SHARED int rapidxml_parser_reset(rapidxml_doc_t *doc);

RAPIDXML_EXPORT_SHARED int rapidxml_parser_root(rapidxml_doc_t *doc, rapidxml_node_t **root);

RAPIDXML_EXPORT_SHARED int rapidxml_node_next(rapidxml_node_t *node, rapidxml_node_t **next);

RAPIDXML_EXPORT_SHARED int rapidxml_node_get_name(rapidxml_node_t *node, const char **name, size_t *len);

RAPIDXML_EXPORT_SHARED int rapidxml_node_get_value(rapidxml_node_t *node, const char **value, size_t *len);

RAPIDXML_EXPORT_SHARED int rapidxml_node_first_attribute(rapidxml_node_t *node, rapidxml_attr_t **attr);

RAPIDXML_EXPORT_SHARED int rapidxml_attribute_next(rapidxml_attr_t *attr, rapidxml_attr_t **next);

RAPIDXML_EXPORT_SHARED int rapidxml_attribute_get_name(rapidxml_attr_t *, const char **name, size_t *len);

RAPIDXML_EXPORT_SHARED int rapidxml_attribute_get_value(rapidxml_attr_t *, const char **value, size_t *len);

RAPIDXML_EXPORT_SHARED int rapidxml_parser_destroy(rapidxml_doc_t **doc);

#endif
