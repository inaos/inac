/*
 * Copyright (c) 2017, INAOS GmbH
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

#include <contribs/miniz/miniz.h>

#ifdef INA_OS_WIN32
#include <direct.h>  
#endif

#define __INA_AAR_APP_META_KEY_MAX 255

typedef struct __ina_aar_app_meta_entry_s {
    char key[__INA_AAR_APP_META_KEY_MAX];
    ina_str_t value;
    UT_hash_handle hh;
} __ina_aar_app_meta_entry_t;

struct ina_aar_ctx_s {
    ina_str_t wd;
    ina_dir_stat_t *ds;
    ina_json_ctx_t *jctx;
    ina_json_parser_t *p;
};

struct ina_aar_app_s {
    ina_str_t id;
    ina_str_t path;
    ina_aar_ctx_t *ctx;
    __ina_aar_app_meta_entry_t *meta;
    mz_zip_archive zar;
};

INA_API(ina_rc_t) ina_aar_init(ina_aar_ctx_t **ctx, const char *working_directory)
{
    *ctx = (ina_aar_ctx_t*)ina_mem_alloc(sizeof(ina_aar_ctx_t));
    (*ctx)->wd = ina_str_new_fromcstr(working_directory);
    if (!INA_SUCCEED(ina_dir_stat_new(&(*ctx)->ds, working_directory))) {
        return INA_ERR_PUSH_LAST;
    }
    if (!INA_SUCCEED(ina_json_init(&(*ctx)->jctx, 1, 0))) {
        return INA_ERR_PUSH_LAST;
    }
    if (!INA_SUCCEED(ina_json_parser_borrow((*ctx)->jctx, &(*ctx)->p))) {
        return INA_ERR_PUSH_LAST;
    }
    
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_aar_app_new(ina_aar_ctx_t *ctx, const char *id, ina_aar_app_t **app)
{
    *app = (ina_aar_app_t*)ina_mem_alloc(sizeof(ina_aar_app_t));
    (*app)->id = ina_str_new_fromcstr(id);
    (*app)->ctx = ctx;

    (*app)->path = ina_str_sprintf("%s/%s", ina_str_cstr(ctx->wd), id);

#ifdef INA_OS_WIN32
    if (_mkdir(ina_str_cstr((*app)->path)) == -1) {
        return INA_FAILURE;
    }
#else
    if (!mkdir(ina_str_cstr((*app)->path), S_IWRITE)) {
        return INA_FAILURE;
    }
#endif

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_aar_app_archive_load(ina_aar_app_t *app, const char *full_path)
{
    mz_bool status;
    int i;

    status = mz_zip_reader_init_file(&app->zar, full_path, 0);
    if (!status) {
        /* FIXME: proper error handling */
        return INA_FAILURE;
    }

    for (i = 0; i < (int)mz_zip_reader_get_num_files(&app->zar); i++) {
        mz_zip_archive_file_stat file_stat;
        if (!mz_zip_reader_file_stat(&app->zar, i, &file_stat)) {
            mz_zip_reader_end(&app->zar);
            return INA_FAILURE;
        }
        
        if (mz_zip_reader_is_file_a_directory(&app->zar, i)) {
            ina_str_t path = ina_str_sprintf("%s/%s", ina_str_cstr(app->path), file_stat.m_filename);
#ifdef INA_OS_WIN32
            if (_mkdir(ina_str_cstr(path)) == -1) {
                return INA_EEXISTS;
            }
#else
            if (!mkdir(ina_str_cstr(path), S_IWRITE)) {
                return INA_EEXISTS;
            }
#endif
            ina_str_free(path);
        }
        else if (INA_CSTR_CASECMP(file_stat.m_filename, ".meta") == 0) {
            ina_rc_t rc = INA_SUCCESS;
            size_t buf_size = (size_t)file_stat.m_uncomp_size;
            unsigned char *buf = (unsigned char*)malloc(buf_size);
            if (!mz_zip_reader_extract_to_mem(&app->zar, file_stat.m_file_index, buf, buf_size, 0)) {
                free(buf);
                mz_zip_reader_end(&app->zar);
                return INA_FAILURE;
            }
            rc = ina_json_parser_execute(app->ctx->p, buf, buf_size, INA_YES);
            if (rc == INA_SUCCESS) {
                const ina_json_data_t *data = NULL;
                char key[__INA_AAR_APP_META_KEY_MAX];
                __ina_aar_app_meta_entry_t *e;
                while (INA_SUCCEED(ina_json_parser_try_data(app->ctx->p, &data))) {
                    switch (data->event) {
                        case INA_JSON_PARSE_EVENT_OBJECT_KEY:
                            ina_mem_set(key, 0, sizeof(char)*__INA_AAR_APP_META_KEY_MAX);
                            INA_ASSERT_TRUE(data->size < __INA_AAR_APP_META_KEY_MAX);
                            strncpy(key, data->value.s, data->size);
                            break;
                        case INA_JSON_PARSE_EVENT_DATA_STRING:
                            e = (__ina_aar_app_meta_entry_t*)ina_mem_alloc(sizeof(__ina_aar_app_meta_entry_t));
                            strncpy(e->key, key, __INA_AAR_APP_META_KEY_MAX);
                            e->value = ina_str_new_fromblk(data->value.s, data->size);
                            HASH_ADD_STR(app->meta, key, e);
                            break;
                    }
                }
            }
            free(buf);
            if (rc != INA_SUCCESS) {
                mz_zip_reader_end(&app->zar);
                return INA_FAILURE;
            }
        }
        else {
            ina_str_t path = ina_str_sprintf("%s/%s", ina_str_cstr(app->path), file_stat.m_filename);
            mz_zip_reader_extract_to_file(&app->zar, file_stat.m_file_index, ina_str_cstr(path), 0);
            ina_str_free(path);
        }
    }

    mz_zip_reader_end(&app->zar);

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_aar_app_meta_str_get(ina_aar_app_t *app, const char *key, ina_str_t *value)
{
    __ina_aar_app_meta_entry_t *e;
    HASH_FIND_STR(app->meta, key, e);
    if (e == NULL) {
        *value = NULL;
        return INA_EEXISTS;
    }
    *value = e->value;
    return INA_SUCCESS;
}

static ina_rc_t __ina_aar_app_remove_dir(ina_str_t dir_path)
{
    ina_dir_walker_t *w = NULL;
    const ina_dir_entry_t *e;

    INA_MUST_SUCCEED(ina_dir_walker_new(dir_path, &w));
    INA_MUST_SUCCEED(ina_dir_walker_disable_recursive(w));

    while (INA_SUCCEED(ina_dir_walker_get_next_entry(w, &e))) {
        if (strcmp(e->name, ".") == 0 || strcmp(e->name, "..") == 0) {
            continue;
        }
        if (e->type == INA_DIR_ENTRY_TYPE_DIR) {
            ina_str_t path = ina_str_sprintf("%s/%s", ina_str_cstr(dir_path), e->name);
            __ina_aar_app_remove_dir(path);
#ifdef INA_OS_WIN32
            if (rmdir(ina_str_cstr(path)) == -1) {
                return INA_EEXISTS;
            }
#else
            if (!rmdir(ina_str_cstr(path))) {
                return INA_EEXISTS;
            }
#endif
            ina_str_free(path);
        }
        else if (e->type == INA_DIR_ENTRY_TYPE_FILE) {
            ina_str_t path = ina_str_sprintf("%s/%s", ina_str_cstr(dir_path), e->name);
            remove(ina_str_cstr(path));
            ina_str_free(path);
        }
    }
    INA_MUST_SUCCEED(ina_dir_walker_free(&w));
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_aar_app_free(ina_aar_ctx_t *ctx, ina_aar_app_t **app)
{
    ina_dir_walker_t *w = NULL;
    const ina_dir_entry_t *e;

    INA_MUST_SUCCEED(ina_dir_walker_new(ina_str_cstr((*app)->path), &w));
    INA_MUST_SUCCEED(ina_dir_walker_disable_recursive(w));
    while (INA_SUCCEED(ina_dir_walker_get_next_entry(w, &e))) {
        if (strcmp(e->name, ".") == 0 || strcmp(e->name, "..") == 0) {
            continue;
        }
        if (e->type == INA_DIR_ENTRY_TYPE_DIR) {
            ina_str_t path = ina_str_sprintf("%s/%s", ina_str_cstr((*app)->path), e->name);
            __ina_aar_app_remove_dir(path);
#ifdef INA_OS_WIN32
            if (rmdir(ina_str_cstr(path)) == -1) {
                return INA_EEXISTS;
            }
#else
            if (!rmdir(ina_str_cstr(path))) {
                return INA_EEXISTS;
            }
#endif
            ina_str_free(path);
        }
        else if (e->type == INA_DIR_ENTRY_TYPE_FILE) {
            ina_str_t path = ina_str_sprintf("%s/%s", ina_str_cstr((*app)->path), e->name);
            remove(ina_str_cstr(path));
            ina_str_free(path);
        }
    }
#ifdef INA_OS_WIN32
    if (rmdir(ina_str_cstr((*app)->path)) == -1) {
        return INA_FAILURE;
    }
#else
    if (!rmdir(ina_str_cstr((*app)->path))) {
        return INA_FAILURE;
    }
#endif
    INA_MUST_SUCCEED(ina_dir_walker_free(&w));
    ina_str_free((*app)->id);
    ina_mem_free(*app);
    *app = NULL;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_aar_destroy(ina_aar_ctx_t **ctx)
{
    ina_json_parser_release((*ctx)->jctx, &(*ctx)->p);
    ina_json_destroy(&(*ctx)->jctx);
    ina_dir_stat_free(&(*ctx)->ds);
    ina_str_free((*ctx)->wd);
    ina_mem_free(*ctx);
    *ctx = NULL;
    return INA_SUCCESS;
}
