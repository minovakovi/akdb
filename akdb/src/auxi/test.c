/**
@file test.c Provides functions for reporting test results for modules.
 */


#include "test.h"

/**
 * @author Igor Rinkovec
 * @brief Returns the amount of successful and failed tests.
 * @param successfulAmount amount of successful tests
 * @param failedAmount amount of failed tests
 * @return TestResult
 */
TestResult TEST_result(int successfulAmount, int failedAmount) {
	TestResult result;
	result.testSucceded = successfulAmount;
	result.testFailed = failedAmount;
	if(successfulAmount == 0 && failedAmount == 0) {
		result.implemented = 0;
		return result;
	}
	result.implemented = 1;
	return result;
}


/**
 * @author Igor Rinkovec
 * @brief Prints a beautiful string informing the user of test results in the terminal.
 * @return void
 */
void TEST_output_results(TestResult result) {
	if(result.implemented == 0) {
		printf( YELLOW "TEST: Test is still not reporting the results correctly. Read https://github.com/mschatten/akdb/wiki/Reporting-Test-Results\n" RESET );
		return;
	}
	if(result.testFailed == 0) {
		printf( GREEN "TEST: %i of %i tests successful.\n" RESET , result.testSucceded, result.testSucceded);
		return;
	}
	printf( RED "TEST: %i of %i tests FAILED.\n" RESET , result.testFailed, result.testFailed + result.testSucceded);
}

