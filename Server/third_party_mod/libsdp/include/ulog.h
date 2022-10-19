/**
 * Copyright (C) 2013 Parrot S.A.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * libulog: a minimalistic logging library derived from Android logger
 *
 */

#ifndef _PARROT_ULOG_H
#define _PARROT_ULOG_H

#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

/*----------------------------------------------------------------------------*/
/* ULOG API */

/**
 * ULOG priority levels: keep compatibility with a subset of syslog levels
 */
#define ULOG_CRIT    2       /* critical conditions */
#define ULOG_ERR     3       /* error conditions */
#define ULOG_WARN    4       /* warning conditions */
#define ULOG_NOTICE  5       /* normal but significant condition */
#define ULOG_INFO    6       /* informational message */
#define ULOG_DEBUG   7       /* debug-level message */

typedef void(*SDP_LOG_FUNC_IMP)(const char *file, int line, int level, const char *buffer);
void ulog_simple_log(const char *file, int line, int level, const char *fmt, ...);

#define ULOG_PRI(level, ...) ulog_simple_log(__FILE__, __LINE__, level, __VA_ARGS__)
#define ULOGC(...)      ULOG_PRI(ULOG_CRIT,   __VA_ARGS__)
#define ULOGE(...)      ULOG_PRI(ULOG_ERR,    __VA_ARGS__)
#define ULOGW(...)      ULOG_PRI(ULOG_WARN,   __VA_ARGS__)
#define ULOGN(...)      ULOG_PRI(ULOG_NOTICE, __VA_ARGS__)
#define ULOGI(...)      ULOG_PRI(ULOG_INFO,   __VA_ARGS__)
#define ULOGD(...)      ULOG_PRI(ULOG_DEBUG,  __VA_ARGS__)

/**
 * Log a message that will automatically be predended with the name of the
 * calling function and line number. It will also append the given error as
 * numerical + sting (it assumes an errno). The priority will be ERR.
 */
//#define ULOG_ERRNO(_fmt, _err, ...) \
//	ULOGE("%s:%d: " _fmt " err=%d(%s)", \
//		__func__, __LINE__, ##__VA_ARGS__, \
//			_err, strerror(_err))
#define ULOG_ERRNO(_fmt, _err, ...) \
	printf("%s:%d "_fmt", err=%d(%s) \n", __FILE__, __LINE__, ##__VA_ARGS__, _err, strerror(_err)); \
	fflush(stdout)

/**
 * Log an errno message (with the provided positive errno)
 * and return if the condition is true.
 * The priority will be ERR.
 */
#define ULOG_ERRNO_RETURN_IF(_cond, _err) \
	do { \
		if ((_cond)) { \
			ULOG_ERRNO("", (_err)); \
			return; \
		} \
	} while (0)

/**
 * Log an errno message (with the provided positive errno)
 * and return the negative errno if the condition is true.
 * The priority will be ERR.
 */
#define ULOG_ERRNO_RETURN_ERR_IF(_cond, _err) \
	do { \
		if ((_cond)) { \
			ULOG_ERRNO("", (_err)); \
			return -(_err); \
		} \
	} while (0)

/**
 * Log an errno message (with the provided positive errno)
 * and return the provided value if the condition is true.
 * The priority will be ERR.
 */
#define ULOG_ERRNO_RETURN_VAL_IF(_cond, _err, _val) \
	do { \
		if ((_cond)) { \
			ULOG_ERRNO("", (_err)); \
			/* codecheck_ignore[RETURN_PARENTHESES] */ \
			return (_val); \
		} \
	} while (0)

#endif /* _PARROT_ULOG_H */
