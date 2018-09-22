/*
 * Copyright (c) 2016-2018, INAOS GmbH
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
#include "config.h"

#ifndef INA_OS_WIN32
#include <dirent.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>
#endif

struct ina_dir_walker_s {
    ina_str_t basedir;
    ina_dir_entry_t *first;
    ina_dir_entry_t *last;
    ina_dir_entry_t *current;
    ina_dir_entry_t *head;
    ina_mempool_t *mp;
    ina_mempool_t *smp;
    ina_dir_sort_order_t sort_order;
    ina_dir_sort_attrib_t sort_attrib;
    int recursive;
    int (*sort_cb)(const void*,const void*);
};

struct ina_dir_stat_s {
    ina_str_t dir;
#ifdef INA_OS_WIN32
    ULARGE_INTEGER free_bytes_available;
    ULARGE_INTEGER total_number_of_bytes;
    ULARGE_INTEGER total_numof_free_bytes;
#else
    uint64_t free_bytes;
    uint64_t total_bytes;
#endif
};

static int __ina_dir_walker_cmp_name_ascend(const void* a, const void* b)
{
    return INA_CSTR_CASECMP(((ina_dir_entry_t*)a)->name,
                            ((ina_dir_entry_t*)b)->name);
}

static int __ina_dir_walker_cmp_name_descend(const void* a, const void* b)
{
    return INA_CSTR_CASECMP(((ina_dir_entry_t*)b)->name,
                            ((ina_dir_entry_t*)a)->name);
}

static int __ina_dir_walker_cmp_type_ascend(const void* a, const void* b)
{
    if (((ina_dir_entry_t*)a)->type < ((ina_dir_entry_t*)b)->type) return -1;
    if (((ina_dir_entry_t*)a)->type == ((ina_dir_entry_t*)b)->type) return 0;
    if (((ina_dir_entry_t*)a)->type >  ((ina_dir_entry_t*)b)->type) return 1;
    return 0;
}

static int __ina_dir_walker_cmp_type_descend(const void* a, const void* b)
{
    if (((ina_dir_entry_t*)b)->type < ((ina_dir_entry_t*)a)->type) return -1;
    if (((ina_dir_entry_t*)b)->type == ((ina_dir_entry_t*)a)->type) return 0;
    if (((ina_dir_entry_t*)b)->type >  ((ina_dir_entry_t*)a)->type) return 1;
    return 0;
}

static void __ina_dir_walker_set_sort_cb(ina_dir_walker_t *walker)
{
    switch (walker->sort_order) {
        case INA_DIR_SORT_ORDER_NONE:
            walker->sort_cb = NULL;
            break;
        case INA_DIR_SORT_ORDER_ASCEND:
            switch (walker->sort_attrib) {
                case INA_DIR_SORT_ATTRIB_DFT:
                case INA_DIR_SORT_ATTRIB_NAME:
                    walker->sort_cb = __ina_dir_walker_cmp_name_ascend;
                    break;
                case INA_DIR_SORT_ATTRIB_TYPE:
                    walker->sort_cb = __ina_dir_walker_cmp_type_ascend;
                    break;
            }
            break;
        case INA_DIR_SORT_ORDER_DESCEND:
            switch (walker->sort_attrib) {
                case INA_DIR_SORT_ATTRIB_DFT:
                case INA_DIR_SORT_ATTRIB_NAME:
                    walker->sort_cb = __ina_dir_walker_cmp_name_descend;
                    break;
                case INA_DIR_SORT_ATTRIB_TYPE:
                    walker->sort_cb = __ina_dir_walker_cmp_type_descend;
                    break;
            }
    }
}


INA_API(ina_rc_t) ina_dir_walker_new(const char *basedir,
                                      ina_dir_walker_t **walker)
{
    INA_VERIFY_NOT_NULL(walker);
    INA_VERIFY_NOT_NULL(basedir);
    INA_VERIFY(strlen(basedir));

    *walker = (ina_dir_walker_t*)ina_mem_alloc(sizeof(ina_dir_walker_t));
    INA_RETURN_IF_NULL(*walker);
    INA_RETURN_IF_FAILED(ina_mempool_new(1024 * sizeof(ina_dir_entry_t), NULL, 0, &(*walker)->mp));
    INA_RETURN_IF_FAILED(ina_mempool_new(1024 * 1024, NULL, INA_MEM_DYNAMIC, &(*walker)->smp));
    (*walker)->head = ina_mempool_dalloc((*walker)->mp,1024*sizeof(ina_dir_entry_t));
    (*walker)->basedir = ina_str_new_fromcstr(basedir);
    (*walker)->sort_order = INA_DIR_SORT_ORDER_NONE;
    (*walker)->sort_attrib = INA_DIR_SORT_ATTRIB_DFT;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_dir_walker_enable_recursive(ina_dir_walker_t *walker)
{
    INA_VERIFY_NOT_NULL(walker);
    walker->recursive = INA_YES;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_dir_walker_disable_recursive(ina_dir_walker_t *walker)
{
    INA_VERIFY_NOT_NULL(walker);
    walker->recursive = INA_NO;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_dir_walker_get_sort_order(const ina_dir_walker_t *walker,
                                                ina_dir_sort_order_t *sort_order)
{
    INA_VERIFY_NOT_NULL(walker);
    INA_VERIFY_NOT_NULL(sort_order);
    *sort_order = walker->sort_order;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_dir_walker_set_sort_order(ina_dir_walker_t *walker,
                                                ina_dir_sort_order_t sort_order)
{
    INA_VERIFY_NOT_NULL(walker);
    walker->sort_order = sort_order;
    __ina_dir_walker_set_sort_cb(walker);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_dir_walker_get_sort_attrib(const ina_dir_walker_t *walker,
                                                 ina_dir_sort_attrib_t *sort_attrib)
{
    INA_VERIFY_NOT_NULL(walker);
    INA_VERIFY_NOT_NULL(sort_attrib);
    *sort_attrib = walker->sort_attrib;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_dir_walker_set_sort_attrib(ina_dir_walker_t *walker,
                                                 ina_dir_sort_attrib_t sort_attrib)
{
    INA_VERIFY_NOT_NULL(walker);
    walker->sort_attrib = sort_attrib;
    __ina_dir_walker_set_sort_cb(walker);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_dir_walker_get_next_entry(ina_dir_walker_t *walker,
                                                 const ina_dir_entry_t **entry)
{
    INA_VERIFY_NOT_NULL(walker);
    INA_VERIFY_NOT_NULL(entry);
    *entry = NULL;

    if (walker->first == NULL) {
        DIR *dir;
        struct dirent *f;

        INA_MUST_SUCCEED(ina_mempool_reset(walker->smp));
        walker->last = NULL;

        if ((dir = opendir(walker->basedir)) != NULL) {
            while ((f = readdir(dir)) != NULL) {
                if (walker->first == NULL) {
                    walker->first = walker->head;
                    walker->last = walker->first;
                } else {
                    walker->last++;
                }
                walker->last->name = ina_str_new_fromcstr_using_pool(f->d_name,
                                                                     walker->smp);
                switch (f->d_type) {
                    case DT_UNKNOWN:
                    case DT_FIFO:
                    case DT_SOCK:
                    case DT_CHR:
                    case DT_BLK:
                    case DT_LNK:
                        walker->last->type = INA_DIR_ENTRY_TYPE_UNKNOWN;
                        break;
                    case DT_REG:
                        walker->last->type = INA_DIR_ENTRY_TYPE_FILE;
                        break;
                    case DT_DIR:
                        walker->last->type = INA_DIR_ENTRY_TYPE_DIR;
                        break;
                }
            }
            walker->current = walker->first;
            closedir(dir);
        }

        if (walker->first != NULL && walker->sort_cb != NULL) {
            qsort(walker->first,
                  (walker->last - walker->first) + 1,
                  sizeof(ina_dir_entry_t),
                  walker->sort_cb);
        }
        walker->current = walker->first;
        if (walker->current == NULL) {
            return INA_ERROR(INA_NN_ENUMERATION|INA_ERR_EMPTY);
        }
    } else if (walker->current == NULL) {
        walker->current = walker->first;
    } else if (walker->current == walker->last) {
        *entry = NULL;
        return INA_ERROR(INA_ERR_END_OF|INA_NN_ENUMERATION);
    } else {
        walker->current++;
    }

    *entry = walker->current;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_dir_walker_reset(ina_dir_walker_t *walker)
{
    INA_VERIFY_NOT_NULL(walker);
    walker->current = NULL;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_dir_walker_reload(ina_dir_walker_t *walker)
{
    INA_VERIFY_NOT_NULL(walker);
    walker->first = NULL;
    return INA_SUCCESS;
}

INA_API(void) ina_dir_walker_free(ina_dir_walker_t **walker) {

    INA_FREE_CHECK(walker);
    ina_mempool_free(&(*walker)->mp);
    ina_mempool_free(&(*walker)->smp);
    INA_STR_FREE_SAFE((*walker)->basedir);
    INA_MEM_FREE_SAFE(*walker);
}

INA_API(ina_rc_t) ina_dir_stat_new(ina_dir_stat_t **stat, const char *dir)
{
    INA_VERIFY_NOT_NULL(stat);
    INA_VERIFY_NOT_NULL(dir);

    *stat = (ina_dir_stat_t*)ina_mem_alloc(sizeof(ina_dir_stat_t));
    INA_RETURN_IF_NULL(*stat);
    (*stat)->dir = ina_str_new_fromcstr(dir);
#ifdef INA_OS_WIN32
    if (GetDiskFreeSpaceEx(dir, &(*stat)->free_bytes_available, 
        &(*stat)->total_number_of_bytes, 
        &(*stat)->total_numof_free_bytes) == 0) {
            ina_dir_stat_free(stat);
            return INA_ERROR(INA_NN_OPERATION|INA_ERR_FAILED);
    }
#else
    struct statvfs sfs;
    if (statvfs(dir, &sfs) != 0) {
        ina_dir_stat_free(stat);
        return INA_ERROR(INA_NN_OPERATION|INA_ERR_FAILED);
    }
    (*stat)->free_bytes = sfs.f_bsize * sfs.f_bavail;
    (*stat)->total_bytes = sfs.f_blocks * sfs.f_bsize;
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_dir_stat_bytes_capacity(ina_dir_stat_t *stat, uint64_t *capacity_bytes)
{
    INA_VERIFY_NOT_NULL(stat);
    INA_VERIFY_NOT_NULL(capacity_bytes);
#ifdef INA_OS_WIN32
    *capacity_bytes = stat->total_number_of_bytes.QuadPart;
#else
    *capacity_bytes = stat->total_bytes;
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_dir_stat_bytes_free(ina_dir_stat_t *stat, uint64_t *free_bytes)
{
    INA_VERIFY_NOT_NULL(stat);
    INA_VERIFY_NOT_NULL(free_bytes);
#ifdef INA_OS_WIN32
    *free_bytes = stat->free_bytes_available.QuadPart;
#else
    *free_bytes = stat->free_bytes;
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_dir_stat_pct_used(ina_dir_stat_t *stat, int *pct_used)
{
    uint64_t b_total = 0;
    uint64_t b_free = 0;
    double free_pct;
    double used_pct;
    INA_VERIFY_NOT_NULL(stat);
    INA_VERIFY_NOT_NULL(pct_used);
    INA_RETURN_IF_FAILED(ina_dir_stat_bytes_capacity(stat, &b_total));
    INA_RETURN_IF_FAILED(ina_dir_stat_bytes_free(stat, &b_free));
    free_pct = (((double)(b_free/1024/1024))*100.0)/((double)(b_total/1024/1024));
    used_pct = 100-free_pct;
    *pct_used = (int)used_pct;
    return INA_SUCCESS;
}

INA_API(void) ina_dir_stat_free(ina_dir_stat_t **stat)
{
    INA_FREE_CHECK(stat);
    INA_STR_FREE_SAFE((*stat)->dir);
    INA_MEM_FREE_SAFE(*stat);
}

