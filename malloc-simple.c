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

struct mem_domain *mem_domain_alloc (const char *name)
{
	return NULL + 1;
}

void mem_domain_free (struct mem_domain *domain)
{
	/* do nothing */
}

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
	void *p = malloc (size);

	if ((how & MEM_ZERO) != 0 && p != NULL)
		memset (p, 0, size);

	return p;
}

void mem_free (void *p, size_t size, struct mem_domain *domain)
{
	free (p);
}
