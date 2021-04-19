/*
 * Memory buffers management interface, simple variant
 *
 * Copyright (c) 2015-2021 Alexei A. Smekalkine
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <string.h>

#include "malloc.h"
#include "mbuf.h"
#include "types.h"

static struct mem_domain *domain;

#define MSIZE  (1 << 8)

struct mbuf {
	struct mbuf *next;
	struct mbuf *next_chain;
	void *data;
	u32 size;
	u16 type;
	u16 flags;

	char buf[];
};

struct mbuf *mbuf_alloc (int how, int type)
{
	struct mbuf *o;

	if ((o = mem_alloc (domain, MSIZE, how)) == NULL)
		return NULL;

	o->next = NULL;
	o->next_chain = NULL;
	o->data = o->buf;
	o->size = 0;
	o->type = type;
	o->flags = 0;

	return o;
}

void mbuf_free (struct mbuf *o)
{
	struct mbuf *next;

	for (; o != NULL; o = next) {
		next = o->next;
		mem_free (domain, o, MSIZE);
	}
}

#define MLEN  (MSIZE - offsetof (struct mbuf, buf))

/*
 * Set the data pointer of a newly-allocated mbuf to place an object of
 * the specified size at the end of the mbuf, longword aligned.
 *
 * Returns non-zero on success.
 */
static int mbuf_align (struct mbuf *o, size_t size)
{
	if (size > MLEN || o->size != 0)
		return 0;

	o->data += (MLEN - (size)) & ~(sizeof (long) - 1);
	return 1;
}

static size_t mbuf_avail_at_head (struct mbuf *o)
{
	void *head = o->buf;

	return o->data - head;
}

static size_t mbuf_avail_at_tail (struct mbuf *o)
{
	void *tail = o->buf + MLEN;

	return tail - (o->data + o->size);
}

/*
 * Append data to the end of the buffer. Returns the number of bytes
 * actually written.
 */
size_t mbuf_append (struct mbuf *o, int how, const void *data, size_t size)
{
	size_t avail;

	for (; o->next != NULL; o = o->next) {}

	if ((avail = mbuf_avail_at_tail (o)) == 0) {
		if ((o->next = mbuf_alloc (how, o->type)) == NULL)
			return 0;

		o = o->next;
		avail = mbuf_avail_at_tail (o);
	}

	if (size > avail)
		size = avail;

	memcpy (o->data + o->size, data, size);
	o->size += size;

	return size;
}

/*
 * Apply function fn to the each buffer in the buffer chain until fn return
 * zero. Returns result code of last function applied.
 */
int mbuf_apply (const struct mbuf *o,
		int (*fn) (void *cookie, const void *data, size_t size),
		void *cookie)
{
	int ret;

	for (ret = 0; o != NULL; o = o->next)
		if ((ret = fn (cookie, o->data, o->size)) == 0)
			return 0;

	return ret;
}

/*
 * Try to add a continuous region to the top (or the end) of the buffer
 * chain. Returns a pointer to the allocated region or NULL if memory cannot
 * be allocated. In the latter case, the buffer chain is freed.
 */
void *mbuf_add_head (struct mbuf **m, int how, size_t size)
{
	struct mbuf *o = *m;

	if (mbuf_avail_at_head (o) >= size) {
		o->data -= size;
		o->size += size;

		return o->data;
	}

	if (size <= MLEN && (o = mbuf_alloc (how, o->type)) != NULL) {
		o->next = *m;
		o->size = size;
		mbuf_align (o, size);
		*m = o;

		return o->data;
	}

	mbuf_free (*m);
	return NULL;
}

void *mbuf_add_tail (struct mbuf *o, int how, size_t size)
{
	void *data;

	for (; o->next != NULL; o = o->next) {}

	if (mbuf_avail_at_tail (o) >= size) {
		data = o->data + o->size;
		o->size += size;

		return data;
	}

	if (size <= MLEN && (o->next = mbuf_alloc (how, o->type)) != NULL) {
		o = o->next;
		o->size = size;

		return o->data;
	}

	mbuf_free (o);
	return NULL;
}

/*
 * Arrange that the first (or last) size bytes of the buffer chain are
 * contiguous. Returns a pointer to the requested continuous region or
 * NULL if memory cannot be allocated. In the latter case, the buffer
 * chain is freed.
 */
void *mbuf_get_head (struct mbuf **m, int how, size_t size)
{
	struct mbuf *o = *m;
	char *p;

	if (o->size >= size)
		return o->data;

	if ((p = mbuf_add_head (m, how, size)) == NULL)
		return NULL;

	for (; o != NULL; o = (*m)->next) {
		if (o->size > size) {
			memcpy (p, o->data, size);
			o->size -= size;

			return (*m)->data;
		}

		memcpy (p, o->data, o->size);
		p += o->size, size -= o->size;

		(*m)->next = o->next;
		o->next = NULL; mbuf_free (o);  /* unlink and free */
	}

	mbuf_free (o);
	return NULL;
}

void *mbuf_get_tail (struct mbuf *m, int how, size_t size)
{
	/* TODO: implement */
	return NULL;
}

/*
 * Copy the first (or last) size bytes of the buffer chain into the line
 * buffer and cuf off it. If buf is NULL then just cut off data.
 */
int mbuf_pull_head (struct mbuf **m, void *buf, size_t size)
{
	struct mbuf *o;
	char *p;

	for (o = *m, p = buf; o != NULL; o = *m) {
		if (o->size > size) {
			if (buf != NULL)
				memcpy (p, o->data, size);

			o->data += size;
			o->size -= size;
			return 1;
		}

		if (buf != NULL) {
			memcpy (p, o->data, o->size);
			p += o->size;
		}

		*m = o->next;
		size -= o->size;

		o->next = NULL; mbuf_free (o);  /* unlink and free */
	}

	return size == 0;
}

static size_t pull_tail (struct mbuf *o, void *buf, size_t size)
{
	size_t rest = size;
	char *p = buf;

	if (o->next != NULL) {
		rest -= pull_tail (o->next, buf, size);

		if (o->next->size == 0) {
			mbuf_free (o->next);
			o->next = NULL;
		}
	}

	if (rest < o->size) {
		o->size -= rest;

		if (buf != NULL)
			memcpy (p, o->data + o->size, rest);

		return size;
	}

	rest -= o->size;

	if (buf != NULL)
		memcpy (p + rest, o->data, o->size);

	o->size = 0;
	return size - rest;
}

int mbuf_pull_tail (struct mbuf *m, void *buf, size_t size)
{
	return pull_tail (m, buf, size) == size;
}
