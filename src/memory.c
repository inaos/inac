/*
* Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
*
* This software is the confidential and proprietary information of INAOS GmbH
* ("Confidential Information"). You shall not disclose such Confidential
* Information and shall use it only in accordance with the terms of the
* license agreement you entered into with INAOS GmbH.
*/
#include <sys/stat.h>
#include <libinac/lib.h>
#include "config.h"


/* Child pool (internal used) */
#define INA_MEM_CHILD (8)

struct ina_mempool_s {
	ina_handle_t shm_handle;
	uint32_t cf;
	size_t size;
	size_t pos;
	size_t end;
	size_t alignment;
	unsigned char *m;
	ina_str_t label;
	struct ina_mempool_s *current;
	struct ina_mempool_s *parent;
	struct ina_mempool_s *child;
};

static ina_rc_t __ina_shm_open(ina_mempool_t *);
static ina_rc_t __ina_shm_close(ina_mempool_t *);


INA_API(void *) ina_mem_alloc_aligned(size_t alignment, size_t size)
{
	/*
	* Same behavior as in c-runtime
	*/
	if (INA_UNLIKELY(size == 0)) {
		ina_err_reset();
		return NULL;
	}

	if (INA_UNLIKELY(alignment == 0)) {
		INA_ERROR(INA_ERR_INVALID_ARGUMENT);
		return NULL;
	}

	/* Allocate necessary memory area
	* client request - size parameter -
	* plus area to store the address
	* of the memory returned by standard
	* malloc().
	*/
	void *p = INA_MEM_MALLOC(size + alignment - 1 + sizeof(void*));

	if (INA_UNLIKELY(p != NULL)) {
		void *ptr;
		/* Address of the aligned memory according to the align parameter*/
		ptr = (void*)(((size_t)p + sizeof(void*) + alignment - 1) & ~(alignment - 1));
		/* store the address of the malloc() above
		* at the beginning of our total memory area.
		* You can also use *((void **)ptr-1) = p
		* instead of the one below.
		*/
		*((void**)((size_t)ptr - sizeof(void*))) = p;
		/* Return the address of aligned memory */
		return ptr;
	}
	INA_ERROR(INA_ERR_OUT_OF_MEMORY);
	return NULL;
}

INA_API(void*) ina_mem_realloc(void *ptr, size_t nb)
{
	void *p;
	INA_ASSERT_NOT_NULL(ptr);
	p = *((void**)((size_t)ptr - sizeof(void*)));

	if (nb == 0) {
		ina_mem_free(ptr);
		return NULL;
	}

	p = INA_MEM_REALLOC(p, nb + sizeof(void*) - 1 + sizeof(void*));
	ptr = (void*)(((size_t)p + sizeof(void*) + sizeof(void*) - 1) & ~(sizeof(void*) - 1));
	*((void**)((size_t)ptr - sizeof(void*))) = p;
	return ptr;
}

INA_API(ina_rc_t) ina_mem_get_pagesize(size_t *size)
{
#ifndef INA_OS_WINDOWS
	INA_VERIFY_NOT_NULL(size);
	*size = (size_t)sysconf(_SC_PAGESIZE);
#else
	SYSTEM_INFO si;
	INA_VERIFY_NOT_NULL(size);
	GetSystemInfo(&si);
	*size = (size_t)si.dwPageSize;
#endif
	return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_mempool_new(size_t size, const char *label, uint32_t cf, ina_mempool_t **pool)
{
	INA_VERIFY_NOT_NULL(pool);

	if (size < INA_MEM_MIN_POOL_SIZE) {
		size = INA_MEM_MIN_POOL_SIZE;
	}
	size = INA_MEM_ALIGN(size);

	*pool = (ina_mempool_t*)ina_mem_alloc(sizeof(ina_mempool_t));
	INA_RETURN_IF_NULL(*pool);
	INA_MEM_SET_ZERO(*pool, ina_mempool_t);
	if (cf^INA_MEM_BESTFIT) {
		(*pool)->alignment = INA_MEM_ALIGN_SIZE;
	} else {
		(*pool)->alignment = 1;
	}
	(*pool)->cf = cf;
	(*pool)->size = size;
	(*pool)->end = (*pool)->size;
	(*pool)->current = *pool;
	if (label != NULL) {
		(*pool)->label = ina_str_new_fromcstr(label);
	};
	if (cf&INA_MEM_SHARED) {
		INA_ASSERT_NOT_NULL((*pool)->label);

		if (INA_FAILED((__ina_shm_open(*pool)))) {
			ina_str_free((*pool)->label);
			ina_mem_free(*pool);
			*pool = NULL;
			return ina_err_get_rc();
		}
	}
	else {
		(*pool)->shm_handle = 0;
		(*pool)->m = (unsigned char*)ina_mem_alloc(size);
		if (0 == (cf&INA_MEM_NOZEROFILL)) {
			ina_mem_set((*pool)->m, 0, (*pool)->size);
		}
	}

	if ((*pool)->m == NULL) {
		ina_mem_free(*pool);
		ina_str_free((*pool)->label);
		*pool = NULL;
		return INA_ERROR(INA_ERR_OUT_OF_MEMORY);
	}
	INA_TRACE3("New memory pool: %p->%p size = %ld", *pool, (*pool)->m, (*pool)->size);
	return INA_SUCCESS;
}

INA_API(void) ina_mempool_free(ina_mempool_t **pool)
{
	ina_mempool_t *pm;
	ina_mempool_t *pn;

	INA_VERIFY_FREE(pool);

	/* Unlink parent */
	if ((*pool)->parent != NULL) {
		(*pool)->parent->child = NULL;
	}

	(*pool)->current = *pool;

	pn = *pool;
	while (pn != NULL) {
		pm = pn;
		if (pn != pn->child) {
			pn = pn->child;
		}
		if (pm->cf&INA_MEM_SHARED) {
			__ina_shm_close(pm);
		}
		else {
			ina_mem_free(pm->m);
		}
		ina_str_free(pm->label);
		ina_mem_free(pm);
	}
}

INA_API(ina_rc_t) ina_mempool_set_alignment(ina_mempool_t *pool, size_t alignment)
{
	INA_VERIFY_NOT_NULL(pool);
	INA_VERIFY(alignment > 0);
	pool->alignment = alignment;
	return INA_SUCCESS;
}

INA_API(size_t) ina_mempool_get_alignment(ina_mempool_t *pool)
{
	INA_ASSERT_NOT_NULL(pool);
	return pool->alignment;

}

INA_API(ina_rc_t) ina_mempool_merge(ina_mempool_t *dest, ina_mempool_t *src)
{
	ina_mempool_t *first_child;

	INA_VERIFY_NOT_NULL(dest);

	if (src == NULL) {
		return INA_SUCCESS;
	}
	if (src == dest) {
	    return INA_SUCCESS;
	}
	if (dest->cf&INA_MEM_SHARED || src->cf&INA_MEM_SHARED) {
		return INA_ERROR(INA_ERR_OPERATION_INVALID);
	}
	src->parent = dest;
	first_child = dest->child;
	dest->child = src;
	src->child = first_child;
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mempool_shrink(ina_mempool_t *pool, size_t chunks,
	ina_mempool_info_t *info)
{
	ina_mempool_t *pm;
	ina_mempool_t *pn;
	size_t c;

	INA_VERIFY_NOT_NULL(pool);
	INA_VERIFY_NOT_NULL(info);

	INA_RETURN_IF_FAILED(ina_mempool_info(pool, info));

	if (info->children <= chunks) {
		return INA_SUCCESS;
	}

	c = info->children;
	pn = pool;
	pm = NULL;
	while (pn != NULL) {
		pm = pn;
		if (pn != pn->child) {
			pn = pn->child;
		}
		if (c > chunks && pool->end == pool->size) {
			if (pm->cf&INA_MEM_SHARED) {
				__ina_shm_close(pm);
			}
			else {
				ina_mem_free(pm->m);
			}
			ina_mem_free(pm);
		}
		else {
			pm->pos = 0;
		}
		--c;
	}
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mempool_clear(ina_mempool_t *pool)
{
	ina_mempool_t *pm;
	ina_mempool_t *pn;

	INA_VERIFY_NOT_NULL(pool);

	pn = pool;
	pm = NULL;
	while (pn != NULL) {
		pm = pn;
		if (pn != pn->child) {
			pn = pn->child;
		}
		pm->pos = 0;
		ina_mem_set(pm->m, 0, pm->size);
	}
	pool->current = pool;
	return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_mempool_reset(ina_mempool_t *pool)
{
	ina_mempool_t *pm;
	ina_mempool_t *pn;

	INA_VERIFY_NOT_NULL(pool);

	pn = pool;
	pm = NULL;
	while (pn != NULL) {
		pm = pn;
		if (pn != pn->child) {
			pn = pn->child;
		}
		pm->pos = 0;
	}
	pool->current = pool;
	return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_mempool_info(ina_mempool_t *pool, ina_mempool_info_t *info)
{
	ina_mempool_t *pm;

	INA_VERIFY_NOT_NULL(pool);
	INA_VERIFY_NOT_NULL(info);

	pm = pool;

	info->size = 0;
	info->used = 0;
	info->children = 0;
	info->cf = pm->cf;
	while (pm != NULL) {
		info->size += pm->size;
		info->used += pm->pos + (pm->size - pm->end);
		++info->children;
		pm = pm->child;
	}
	--info->children;
	return INA_SUCCESS;
}

INA_API(void *) ina_mempool_dalloc_aligned(ina_mempool_t *pool, size_t size, size_t alignment)
{
	void *ret;

	INA_ASSERT_NOT_NULL(pool);
	INA_ASSERT_NOT_NULL(pool->current);
	INA_ASSERT_TRUE(alignment > 0);

	size = INA_MEM_ALIGN_N(size, alignment);

retry:

	if ((pool->current->pos + size > pool->current->end) ||
		(pool->current->pos + size < pool->current->pos)) {
		if (pool->cf&INA_MEM_DYNAMIC) {
			size_t nsize;
			if (pool->current->child != NULL) {
				pool->current = pool->current->child;
				goto retry;
			}

			if (pool->cf&INA_MEM_AUTOSIZE || size > pool->size) {
				nsize = size;
			}
			else {
				nsize = pool->size;
			}

			/* FXIME: shm can not handled in chunks ! */
			if (INA_FAILED(ina_mempool_new(nsize,
				pool->label,
				pool->cf | INA_MEM_CHILD, &pool->current->child))) {
				return NULL;
			}
			pool->current->child->parent = pool->current;
			pool->current = pool->current->child;
		}
		else {
			INA_ERROR(INA_ERR_POOL_FULL);
			return NULL;
		}
	}
	ret = &pool->current->m[pool->current->pos];
	pool->current->pos += size;
	return ret;
}


INA_API(void *) ina_mempool_nalloc_aligned(ina_mempool_t *pool, size_t size, size_t alignment)
{
	void *ret;

	INA_ASSERT_NOT_NULL(pool);
	INA_ASSERT_NOT_NULL(pool->current);
	INA_ASSERT_TRUE(alignment > 0);

	size = INA_MEM_ALIGN_N(size, alignment);

	/* bogus request */
	if (pool->end < size) {
		INA_ERROR(INA_ES_SIZE | INA_ERR_INVALID);
		return NULL;
	}

	if ((pool->current->pos + size > pool->current->end) ||
		(pool->current->pos + size < pool->current->pos)) {
		if (pool->cf&INA_MEM_DYNAMIC) {
			size_t nsize = 0;

			if (pool->cf&INA_MEM_AUTOSIZE || size > pool->size) {
				nsize = size;
			}
			else {
				nsize = pool->size;
			}

			/* FIXME: shm can not handled in chunks ! */
			ina_mempool_new(nsize,
				pool->label,
				pool->cf | INA_MEM_CHILD, &pool->current->child);
			pool->current->child->parent = pool->current;
			pool->current = pool->current->child;
		}
		else {
			INA_ERROR(INA_ERR_POOL_FULL);
			return NULL;
		}
	}
	ret = &pool->current->m[pool->current->end - size];
	pool->current->end -= size;
	return ret;
}

INA_API(void *) ina_mempool_ralloc_aligned(ina_mempool_t *pool, void *old,
	size_t old_size, size_t new_size, size_t alignment)
{
	void *ret;

	INA_ASSERT_NOT_NULL(pool);
	INA_ASSERT_NOT_NULL(old);
	INA_ASSERT(old_size > 0);
	INA_ASSERT(new_size > 0);
	INA_ASSERT_NOT_NULL(pool->current);
	INA_ASSERT_TRUE(alignment > 0);



	new_size = INA_MEM_ALIGN_N(new_size, alignment);
	old_size = INA_MEM_ALIGN_N(old_size, alignment);

	/* bogus request */
	if (pool->end < old_size) {
		INA_ERROR(INA_ES_SIZE | INA_ERR_INVALID);
		return NULL;
	}
	/* was the previous allocation - optimize! */
	if ((pool->pos >= old_size) && (&pool->m[pool->pos - old_size] == old)) {
		/* fits */
		if (pool->pos + new_size - old_size <= pool->end) {
			/* shrinking - zero again! */
			pool->pos += new_size - old_size;
			if (new_size < old_size) {
				ina_mem_set(&pool->m[pool->pos], 0, old_size - new_size);
			}
			return old;
		}
		/* does not fit */
		if (pool->cf&INA_MEM_DYNAMIC) {
			size_t nsize = 0;

			if (pool->cf&INA_MEM_AUTOSIZE || new_size > pool->size) {
				nsize = new_size;
			}
			else {
				nsize = pool->size;
			}

			/* FIXME: Push an error , if fails */
			/* FIXME: shm can not handled in chunks ! */
			ina_mempool_new(nsize,
				pool->label,
				pool->cf | INA_MEM_CHILD, &pool->current->child);
			pool->current->child->parent = pool->current;
			pool->current = pool->current->child;
			ret = &pool->m[pool->pos];
			ina_mem_cpy(ret, old, old_size);
			pool->pos += new_size;
			return ret;
		}
		INA_ERROR(INA_ERR_OUT_OF_MEMORY);
		return NULL;
	}
	/* cannot shrink, we need to move */
	if (new_size <= old_size) {
		return old;
	}
	/* fits */
	if ((pool->pos + new_size >= pool->pos) &&
		(pool->pos + new_size <= pool->end))
	{
		ret = &pool->m[pool->pos];
		ina_mem_cpy(ret, old, old_size);
		pool->pos += new_size;
		return ret;
	}
	if (pool->cf&INA_MEM_DYNAMIC) {
		size_t nsize = 0;

		if (pool->cf&INA_MEM_AUTOSIZE || new_size > pool->size) {
			nsize = new_size;
		}
		else {
			nsize = pool->size;
		}

		/* FIXME: Push an error , if fails */
		/* FIXME: shm can not handled in chunks ! */
		ina_mempool_new(nsize,
			pool->label,
			pool->cf | INA_MEM_CHILD, &pool->current->child);
		pool->current->child->parent = pool->current;
		pool->current = pool->current->child;
		ret = &pool->m[pool->pos];
		ina_mem_cpy(ret, old, old_size);
		pool->pos += new_size;
		return ret;
	}
	INA_ERROR(INA_ERR_POOL_FULL);
	return NULL;
}


#ifndef INA_OS_WINDOWS

static ina_rc_t
__ina_shm_open(ina_mempool_t *pool)
{
	int flags;

	INA_ASSERT_NOT_NULL(pool);
	INA_ASSERT_NOT_NULL(pool->label);
	INA_ASSERT(pool->size > 0);
	INA_ASSERT(pool->cf&INA_MEM_SHARED);
	INA_ASSERT_NULL(pool->m);

	pool->size = INA_MEM_ALIGN(pool->size + sizeof(int64_t));
	pool->end = pool->size;

	flags = O_RDWR;
	if (pool->cf&INA_MEM_SHARED_CREATE) {
		flags |= O_CREAT;
		if (pool->cf&INA_MEM_SHARED_EXCL) {
			flags |= O_EXCL;
		}
	}

	pool->shm_handle = shm_open(ina_str_cstr(pool->label), flags, S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP | S_IWOTH | S_IROTH);
	if (pool->shm_handle == -1) {
		return INA_OS_ERROR(INA_ES_HANDLE | INA_ERR_INVALID);
	}

	if (pool->cf&INA_MEM_SHARED_EXCL) {
		if (ftruncate(pool->shm_handle, pool->size) == -1) {
			INA_OS_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
			close(pool->shm_handle);
			pool->shm_handle = 0;
			shm_unlink(ina_str_cstr(pool->label));
			return ina_err_get_rc();
		}
	}

	pool->m = (void *)mmap(NULL, pool->size, PROT_READ | PROT_WRITE,
		MAP_SHARED,
		pool->shm_handle, 0);

	if (pool->m == MAP_FAILED) {
		INA_OS_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
		close(pool->shm_handle);
		pool->shm_handle = 0;
		if (pool->cf&INA_MEM_SHARED_EXCL) {
			shm_unlink(ina_str_cstr(pool->label));
		}
		return ina_err_get_rc();
	}
	/* Inc ref count */
	__sync_fetch_and_add((int64_t*)pool->m, 1);
	/* Inc start pos */
	pool->pos += sizeof(int64_t);
	INA_TRACE2("shared mem %s ref count =  %" INA_INT64_T_FMT, pool->label, *(int64_t*)pool->m);
	return INA_SUCCESS;
}

static ina_rc_t
__ina_shm_close(ina_mempool_t *pool)
{
	INA_ASSERT_NOT_NULL(pool);
	INA_ASSERT(pool->size > 0);
	int64_t cn;

	if (pool->m == NULL) {
		return INA_SUCCESS;
	}

	/* Dec an get ref count before unmap memory */
	cn = __sync_sub_and_fetch((int64_t*)pool->m, 1);

	/* Unmap memory */
	munmap(pool->m, pool->size);
	pool->m = NULL;
	pool->size = 0;
	pool->pos = 0;
	pool->end = 0;

	/* Close shared mem */
	close(pool->shm_handle);

	/* Dec ref count, unlink on last relase */
	if (cn == 0 || pool->cf&INA_MEM_SHARED_EXCL) {
		INA_TRACE2("unlinking shared mem %s", pool->label);
		shm_unlink(ina_str_cstr(pool->label));
	}
	INA_TRACE2("shared mem %s ref count =  %" INA_INT64_T_FMT, pool->label, cn);

	return INA_SUCCESS;
}
#else
static ina_rc_t
__ina_shm_open(ina_mempool_t *pool)
{
	INA_ASSERT_NOT_NULL(pool);
	INA_ASSERT_NOT_NULL(pool->label);
	INA_ASSERT(pool->size > 0);
	INA_ASSERT(pool->cf&INA_MEM_SHARED);
	INA_ASSERT_NULL(pool->m);

	pool->shm_handle = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		INA_HIGH32(pool->size),
		INA_LOW32(pool->size),
		ina_str_cstr(pool->label));

	if (pool->shm_handle == NULL) {
		return INA_OS_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
	}
	pool->m = (void*)MapViewOfFile(pool->shm_handle,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		pool->size);

	if (pool->m == NULL) {
		CloseHandle(pool->shm_handle);
		pool->shm_handle = NULL;
		return INA_OS_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
	}
	return INA_SUCCESS;
}

static ina_rc_t
__ina_shm_close(ina_mempool_t *pool)
{
	INA_ASSERT_NOT_NULL(pool);
	INA_ASSERT(pool->size > 0);
	INA_ASSERT_NOT_NULL(pool->label);
	INA_ASSERT_NOT_NULL(pool->shm_handle);

	if (pool->m == NULL) {
		return INA_SUCCESS;
	}

	/* TODO: Error handling */
	UnmapViewOfFile(pool->shm_handle);
	CloseHandle(pool->shm_handle);

	pool->m = NULL;
	pool->shm_handle = NULL;
	pool->size = 0;
	pool->pos = 0;
	pool->end = 0;

	return INA_SUCCESS;
}
#endif