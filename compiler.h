/*
 * Compiler Helper Module
 *
 * Copyright (c) 2011-2017 Alexei A. Smekalkine
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef SYS_COMPILER_H
#define SYS_COMPILER_H  1

#ifdef __GNUC__

#define noinline	__attribute__ ((noinline))
#define nonnull(...)	__attribute__ ((nonnull (__VA_ARGS__)))
#define sentinel	__attribute__ ((sentinel))

#else

#define noinline
#define nonnull(...)
#define sentinel

#endif

#endif  /* SYS_COMPILER_H */
