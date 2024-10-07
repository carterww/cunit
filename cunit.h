/*
 * Author: Carter Williams
 * Email: carterww@hotmail.com
 * Date: 2024-10-06
 * License: MIT
 */

#ifndef _CUNIT_CUNIT_H
#define _CUNIT_CUNIT_H

#ifdef __has_attribute
#if __has_attribute(constructor)
#define attr_constructor __attribute__((constructor))
#endif // __has_attribute(constructor)
#endif // defined __has_attribute

#ifndef attr_constructor
#error "cunit requires the constructor attribute to be defined."
#endif

#include <stddef.h>

#include "cassert.h"

typedef void (*cunit_func)(void);

struct cunit_test {
	const char *name;
	void (*test)(void);
	struct cunit_test *next;
};

struct cunit_suite {
	const char *name;
	void (*setup)(void);
	void (*teardown)(void);
	struct cunit_test *tests;
	struct cunit_suite *next;
};

void __cunit_add_suite(struct cunit_suite *suite);

#define CUNIT_SUITE(suite_name)                            \
	static struct cunit_suite suite = {                \
		.name = #suite_name,                       \
		.setup = NULL,                             \
		.teardown = NULL,                          \
		.tests = NULL,                             \
		.next = NULL,                              \
	};                                                 \
	attr_constructor void add_suite_##suite_name(void) \
	{                                                  \
		__cunit_add_suite(&suite);                 \
	}

#define CUNIT_SETUP()                                \
	static void setup(void);                     \
	attr_constructor static void add_setup(void) \
	{                                            \
		suite.setup = setup;                 \
	}                                            \
	static void setup(void)

#define CUNIT_TEARDOWN()                                \
	static void teardown(void);                     \
	attr_constructor static void add_teardown(void) \
	{                                               \
		suite.teardown = teardown;              \
	}                                               \
	static void teardown(void)

#define CUNIT_TEST(test_name)                                   \
	static void test_func_##test_name(void);                \
	static struct cunit_test test_##test_name = {           \
		.name = #test_name,                             \
		.test = test_func_##test_name,                  \
		.next = NULL,                                   \
	};                                                      \
	attr_constructor static void add_test_##test_name(void) \
	{                                                       \
		if (NULL == suite.tests) {                      \
			suite.tests = &test_##test_name;        \
			return;                                 \
		}                                               \
		struct cunit_test *next = suite.tests;          \
		while (NULL != next->next) {                    \
			next = next->next;                      \
		}                                               \
		next->next = &test_##test_name;                 \
	}                                                       \
	static void test_func_##test_name(void)

#define CU_ASSERT(e) c_assert_fail(e)
#define CU_ASSERT_NULL(e) c_assert_fail(NULL == e)
#define CU_ASSERT_NOT_NULL(e) c_assert_fail(NULL != e)

#define CU_ASSERT_EQ(v1, v2) c_assert_fail(v1 == v2)

#define CU_ASSERT_LT(v1, v2) c_assert_fail(v1 < v2)
#define CU_ASSERT_LTE(v1, v2) c_assert_fail(v1 <= v2)

#define CU_ASSERT_GT(v1, v2) c_assert_fail(v1 > v2)
#define CU_ASSERT_GTE(v1, v2) c_assert_fail(v1 >= v2)

#endif // _CUNIT_CUNIT_H
