#include "api_functions.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <ctype.h>

/* Arithmetic operations for integers */
int sql_add_int(int a, int b) { return a + b; }
int sql_subtract_int(int a, int b) { return a - b; }
int sql_multiply_int(int a, int b) { return a * b; }
int sql_divide_int(int a, int b) { return b != 0 ? a / b : 0; } // Avoid division by zero
int sql_modulo_int(int a, int b) { return a % b; }
int sql_power_int(int a, int b) { return (int)pow(a, b); }

/* Arithmetic operations for floats */
float sql_add_float(float a, float b) { return a + b; }
float sql_subtract_float(float a, float b) { return a - b; }
float sql_multiply_float(float a, float b) { return a * b; }
float sql_divide_float(float a, float b) { return b != 0.0f ? a / b : 0.0f; } // Avoid division by zero
float sql_power_float(float a, float b) { return powf(a, b); }

TestResult AK_api_functions_test() {
    int successful = 0, failed = 0;

    // Testiranje aritmetičkih operacija za cjelobrojne vrijednosti
    if (sql_add_int(2, 3) == 5 && sql_subtract_int(5, 3) == 2 &&
        sql_multiply_int(2, 3) == 6 && sql_divide_int(6, 3) == 2 &&
        sql_modulo_int(5, 3) == 2 && sql_power_int(2, 3) == 8) {
        printf("\n\nArithmetic integer operations test passed.\n\n");   
        successful++;
    } else {
        printf("\n\nArithmetic integer operations test failed.\n\n");  
        failed++;
    }

    // Testiranje aritmetičkih operacija za decimalne vrijednosti
    if (sql_add_float(2.5f, 3.5f) == 6.0f && sql_subtract_float(5.5f, 3.5f) == 2.0f &&
        sql_multiply_float(2.5f, 3.0f) == 7.5f && sql_divide_float(7.5f, 3.0f) == 2.5f &&
        sql_power_float(2.0f, 3.0f) == 8.0f) {
        printf("\n\nArithmetic float operations test passed.\n\n");  
        successful++;
    } else {
        printf("\n\nArithmetic float operations test failed.\n\n");  
        failed++;
    }

    
    if (failed == 0)
    {
        printf("\n\nAll tests has successfully completed!!\n\n");
    }
    // Vraćanje rezultata testiranja
    return TEST_result(successful, failed);
}
