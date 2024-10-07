/*
 * Author: Carter Williams
 * Email: carterww@hotmail.com
 * Date: 2024-10-06
 * License: MIT
 */

#ifndef _CUNIT_CASSERT_H
#define _CUNIT_CASSERT_H

#include <stdio.h>
#include <stdlib.h>

#define c_assert(e)                                                  \
	((e) ? (1) :                                                 \
	       (fprintf(stderr, "'%s' FAILED %s:%d\n", #e, __FILE__, \
			__LINE__),                                   \
		0))

#define c_assert_fail(e)              \
	do {                          \
		if (!(c_assert(e))) { \
			exit(1);      \
		}                     \
	} while (0)

#endif // _CUNIT_CASSERT_H
