/*
 * Copyright (c) 2004-2005 Sergey Lyubka <valenok@gmail.com>
 * All rights reserved
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Sergey Lyubka wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */

#ifndef STD_HEADERS_INCLUDED
#define	STD_HEADERS_INCLUDED

/* we use ../include/ trick to avoid including kdewin32's include/msvc/ wrapper headers */

#ifndef _WIN32_WCE /* Some ANSI #includes are not available on Windows CE */
#include <../include/sys/types.h>
#include <../include/sys/stat.h>
#include <../include/time.h>
#include <../include/errno.h>
#include <../include/signal.h>
#include <../include/fcntl.h>
#endif /* _WIN32_WCE */

#include <../include/stdlib.h>
#include <../include/stdarg.h>
#include <../include/assert.h>
#include <../include/string.h>
#include <../include/ctype.h>
#include <../include/limits.h>
#include <../include/stddef.h>
#include <../include/stdio.h>
#include <../include/wchar.h>

#if defined(_WIN32)		/* Windows specific	*/
#include "compat_win32.h"
#elif defined(__rtems__)	/* RTEMS specific	*/
#include "compat_rtems.h"
#else				/* UNIX  specific	*/
#include "compat_unix.h"
#endif /* _WIN32 */

#endif /* STD_HEADERS_INCLUDED */
