#include "cmocka_include.h"
#include "osal.h"
#include "osal_sem.h"
#include "osal_time.h"

static void test_sem_loop(void)
{
	int res;
	int i;
	uint32_t use;
	uint32_t avail;
	osal_sem_t *sem;
	uint64_t ts1, ts2, diff, wait_nsec;

	osal_sem_deinit();
	sem = osal_sem_create();
	assert_null(sem);

	res = osal_sem_init();
	assert_int_equal(res, OSAL_E_OK);

	use = osal_sem_use();
	assert_int_equal(use, 0);

	avail = osal_sem_avail();
	assert_int_equal(avail, OSAL_SEM_NUM_MAX);

	/* only create, not delete */
	for (i = 0; i < OSAL_SEM_NUM_MAX; i++) {
		use = osal_sem_use();
		assert_int_equal(use, i);

		avail = osal_sem_avail();
		assert_int_equal(avail, OSAL_SEM_NUM_MAX-i);

		sem = osal_sem_create();
		assert_non_null(sem);

		res = osal_sem_post(sem);
		assert_int_equal(res, OSAL_E_OK);

		osal_sem_wait(sem);
	}
	/* no more sem */
	sem = osal_sem_create();
	assert_null(sem);

	osal_sem_deinit();
	res = osal_sem_init();
	assert_int_equal(res, OSAL_E_OK);

	/* create and then delete */
	for (i = 0; i < OSAL_SEM_NUM_MAX; i++) {
		use = osal_sem_use();
		assert_int_equal(use, 0);

		avail = osal_sem_avail();
		assert_int_equal(avail, OSAL_SEM_NUM_MAX);

		sem = osal_sem_create();
		assert_non_null(sem);

		res = osal_sem_post(sem);
		assert_int_equal(res, OSAL_E_OK);

		osal_sem_wait(sem);

		osal_sem_delete(sem);
	}
	sem = osal_sem_create();
	assert_non_null(sem);

	wait_nsec = OSAL_MSEC_NSEC;
	for (i = 0; i < 10; i++) {
		res = osal_clock_time(&ts1);
		assert_int_equal(res, OSAL_E_OK);
		wait_nsec += i*100000;
		res = osal_sem_waittime(sem, wait_nsec/OSAL_USEC_NSEC);
		assert_int_equal(res, OSAL_E_TIMEOUT);

		res = osal_clock_time(&ts2);
		assert_int_equal(res, OSAL_E_OK);

		diff = ts2-ts1;
		assert_in_range(diff, wait_nsec, wait_nsec+OSAL_USEC_NSEC*300);
	}

	osal_sem_deinit();
}

static void test_sem(void **state)
{
	int i;
	for (i = 0; i < 10; i++) {
		test_sem_loop();
	}
}

static int setup(void **state)
{
	osal_init();
	return 0;
}

static int teardown(void **state)
{
	osal_deinit();
	return 0;
}

int main(int argc, char **argv)
{
	setenv("CMOCKA_TEST_ABORT", "1", 1);

	const struct CMUnitTest tests[] = {
		cmocka_unit_test_setup_teardown(test_sem, setup, teardown),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
