#include <stdio.h>
#include <stdlib.h>
#include "os.h"


/* Test if the update of a VPN mapping has in fact changed the mapping */
static test_update_coherency();

/* Test is removing the mapping has in fact removed it */
static void test_removal_coherency();

/* Test if a change in one VPN mapping doesn't affect the other */
static void test_dual_mapping_coherency();

/* Test if two tables at once are manageable */
static void test_two_tables();

/* Test if the logic cares for the valid bit */
static void test_valid_bit();




int main() {
	/* Add test-function calls */
	return 0;
}
