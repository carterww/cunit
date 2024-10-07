#include <stdlib.h>

#include "cunit.h"

CUNIT_SUITE(suite2)

CUNIT_TEST(suite1_test1)
{
	char *s = malloc(5);
	CU_ASSERT_NOT_NULL(s);
	s[4] = 'a';
	CU_ASSERT_EQ('a', s[4]);
	free(s);
}

CUNIT_TEST(suite1_test2)
{
	fprintf(stderr, "Output this in summary\n");
}
