/**
 * @file api_functions.h
 * @brief Header file that provides functions for arithmetic operations and standard string functions.
 */

#ifndef API_FUNCTIONS_H
#define API_FUNCTIONS_H

#include "../auxi/test.h"



/* Arithmetic operations for integers */
int sql_add_int(int a, int b);
int sql_subtract_int(int a, int b);
int sql_multiply_int(int a, int b);
int sql_divide_int(int a, int b);
int sql_modulo_int(int a, int b);
int sql_power_int(int a, int b);

/**

 * @author Mateja Žunec
 * @brief Returns the amount of successful and failed tests.
 * @param successfulAmount amount of successful tests
 * @param failedAmount amount of failed tests
 * @return TestResult
 */


/* Arithmetic operations for floats */
float sql_add_float(float a, float b);
float sql_subtract_float(float a, float b);
float sql_multiply_float(float a, float b);
float sql_divide_float(float a, float b);
float sql_power_float(float a, float b);

/**

 * @author Mateja Žunec
 * @brief Returns the amount of successful and failed tests.
 * @param successfulAmount amount of successful tests
 * @param failedAmount amount of failed tests
 * @return TestResult
 */

TestResult AK_api_functions_test();

#endif /* API_FUNCTIONS_H */
