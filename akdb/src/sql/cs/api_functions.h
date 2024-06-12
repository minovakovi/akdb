/**
 * @file api_functions.h
 * @brief Header file that provides functions for arithmetic operations and standard string functions.
 */

#ifndef API_FUNCTIONS_H
#define API_FUNCTIONS_H

#include "../../auxi/test.h"

/* Arithmetic operations for integers */
int sql_add_int(int a, int b);
int sql_subtract_int(int a, int b);
int sql_multiply_int(int a, int b);
int sql_divide_int(int a, int b);
int sql_modulo_int(int a, int b);
int sql_power_int(int a, int b);

/* Arithmetic operations for floats */
float sql_add_float(float a, float b);
float sql_subtract_float(float a, float b);
float sql_multiply_float(float a, float b);
float sql_divide_float(float a, float b);
float sql_power_float(float a, float b);

/* Standard functions */
char* sql_now();
int sql_len(const char *str);
char* sql_left(const char *str, int n);
char* sql_lower(char *str);
char* sql_upper(char *str);
char* sql_replace(char *str, char old_char, char new_char);

TestResult AK_api_functions_test();

#endif /* API_FUNCTIONS_H */
