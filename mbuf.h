/*
 * Memory buffers management interface
 *
 * Copyright (c) 2015 Alexei A. Smekalkine
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef SYS_MBUF_H
#define SYS_MBUF_H  1

#include <stddef.h>

struct mbuf *mbuf_alloc (int how, int type);
void mbuf_free (struct mbuf *m);

/*
 * Append data to the end of the buffer. Returns the number of bytes
 * actually written.
 */
size_t mbuf_append (struct mbuf *m, int how, const void *data, size_t size);

/*
 * Apply function fn to the each buffer in the buffer chain until fn return
 * zero. Returns result code of last function applied.
 */
int mbuf_apply (const struct mbuf *m,
		int (*fn) (void *cookie, const void *data, size_t size),
		void *cookie);

/*
 * Try to add a continuous region to the top (or the end) of the buffer
 * chain. Returns a pointer to the allocated region or NULL if memory cannot
 * be allocated. In the latter case, the buffer chain is freed.
 */
void *mbuf_add_head (struct mbuf **m, int how, size_t size);
void *mbuf_add_tail (struct mbuf *m,  int how, size_t size);

/*
 * Arrange that the first (or last) size bytes of the buffer chain are
 * contiguous. Returns a pointer to the requested continuous region or
 * NULL if memory cannot be allocated. In the latter case, the buffer
 * chain is freed.
 */
void *mbuf_get_head (struct mbuf **m, int how, size_t size);
void *mbuf_get_tail (struct mbuf *m , int how, size_t size);

/*
 * Cut off the first (or last) size bytes of the buffer chain.
 */
int mbuf_del_head (struct mbuf **m, size_t size);
int mbuf_del_tail (struct mbuf *m,  size_t size);

#endif  /* SYS_MBUF_H */
