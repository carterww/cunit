#include "cunit.h"

int will_be_five = 0;

CUNIT_SUITE(suite1)

CUNIT_SETUP()
{
	CU_ASSERT_EQ(will_be_five, 0);
	will_be_five = 5;
}

CUNIT_TEARDOWN()
{
	will_be_five = 0;
}

CUNIT_TEST(suite1_test1)
{
	CU_ASSERT_EQ(will_be_five, 5);
}

CUNIT_TEST(suite1_test2)
{
	// Will fail
	CU_ASSERT_EQ(will_be_five, 3);
}

CUNIT_TEST(suite1_test3)
{
	// Seg fault
	char *variable = NULL;
	variable[5] = 'c';
}
