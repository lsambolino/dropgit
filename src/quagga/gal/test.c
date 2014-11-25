
#include <assert.h>
#include <stdio.h>

#include "gal.h"

void test_power_state(const char* id, bool committed, int ps)
{
	int res;
	assert(monitor_state(id, committed, &res) == GAL_SUCCESS);
	assert(res == ps);
}

int main()
{
	int ps;

	test_power_state("0.eth0", false, 0);
	test_power_state("0.eth0", true, 0);

	test_power_state("0.eth1", false, 0);
	test_power_state("0.eth1", true, 0);

	assert(provisioning("0", 0) == GAL_NOT_IMPLEMENTED);

	assert(provisioning("1", 0) == GAL_RESOURCE_NOT_FOUND);
	assert(provisioning("1.eth0", 0) == GAL_RESOURCE_NOT_FOUND);

	assert(provisioning("0.eth0", 0) == GAL_SUCCESS);
	assert(provisioning("0.eth0", 1) == GAL_SUCCESS);

	assert(provisioning("0.eth1", 0) == GAL_SUCCESS);
	assert(provisioning("0.eth1", 1) == GAL_SUCCESS);

	test_power_state("0.eth0", false, 1);
	test_power_state("0.eth0", true, 0);

	test_power_state("0.eth1", false, 1);
	test_power_state("0.eth1", true, 0);

	assert(rollback("0.eth0") == GAL_SUCCESS);
	assert(rollback("0.eth1") == GAL_SUCCESS);

	test_power_state("0.eth0", false, 0);
	test_power_state("0.eth0", true, 0);

	test_power_state("0.eth1", false, 0);
	test_power_state("0.eth1", true, 0);

	assert(provisioning("0.eth0", 1) == GAL_SUCCESS);
	assert(provisioning("0.eth1", 1) == GAL_SUCCESS);

	assert(commit("0.eth0") == GAL_SUCCESS);
	assert(commit("0.eth1") == GAL_SUCCESS);

	test_power_state("0.eth0", false, 1);
	test_power_state("0.eth0", true, 1);

	test_power_state("0.eth1", false, 1);
	test_power_state("0.eth1", true, 1);

	assert(release("0.eth0") == GAL_SUCCESS);
	assert(release("0.eth1") == GAL_SUCCESS);

	test_power_state("0.eth0", false, 0);
	test_power_state("0.eth0", true, 0);

	test_power_state("0.eth1", false, 0);
	test_power_state("0.eth1", true, 0);

	printf("All test passed successfully\n");
}
