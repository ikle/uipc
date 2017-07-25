/*
 * Memory management interface, simple variant
 *
 * Copyright (c) 2015-2017 Alexei A. Smekalkine
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>
#include <string.h>

#include "malloc.h"

/*
 * The mem_alloc function allocates memory for an object whose size is
 * specified by size in the specified domain and returns a pointer to
 * the allocated memory that is suitably aligned for storage of any
 * kind of object, or NULL if the request could not be satisfied.
 *
 * The how argument specifies operational flags: it may be bitwise-or
 * of zero or more constants of enum mem_flags.
 *
 * The mem_free function releases memory region pointed to by p whose
 * size is specified by size. This region must previously be allocated
 * in the specified domain.
 */
void *mem_alloc (size_t size, struct mem_domain *domain, int how)
{
	void *p;

	if ((p = domain->alloc (domain, size, how)) == NULL)
		return NULL;

	domain->size += size;
	return p;
}

void mem_free (void *p, size_t size, struct mem_domain *domain)
{
	domain->free (domain, p, size);
	domain->size -= size;
}

static void *mem_simple_alloc (struct mem_domain *domain, size_t size, int how)
{
	void *p;

	if ((p = malloc (size)) == NULL)
		return NULL;

	if ((how & MEM_ZERO) != 0)
		memset (p, 0, size);

	return p;
}

static void mem_simple_free (struct mem_domain *domain, void *p, size_t size)
{
	free (p);
}

struct mem_domain mem_core = {
	.alloc	= mem_simple_alloc,
	.free	= mem_simple_free,
	.name	= "simple domain",
};