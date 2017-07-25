/*
 * Memory management interface
 *
 * Copyright (c) 2015-2017 Alexei A. Smekalkine
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef SYS_MALLOC_H
#define SYS_MALLOC_H  1

#include <stddef.h>

struct mem_domain {
	void *(*alloc) (struct mem_domain *domain, size_t size, int how);
	void (*free)   (struct mem_domain *domain, void *p, size_t size);

	const char *name;
	size_t size;
};

struct mem_domain mem_core;  /* core domain */

enum mem_flags {
	MEM_NOWAIT	= 1,	/* do not wait for resources    */
	MEM_ZERO	= 2,	/* set allocated memory to zero */
};

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
void *mem_alloc (size_t size, struct mem_domain *domain, int how);
void mem_free (void *p, size_t size, struct mem_domain *domain);

#endif  /* SYS_MALLOC_H */
