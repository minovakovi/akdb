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

/* Standard functions */
char* sql_now() {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    static char buffer[25];
    strftime(buffer, 25, "%Y-%m-%d %H:%M:%S", tm_info);
    return buffer;
}

int sql_len(const char *str) {
    return strlen(str);
}

char* sql_left(const char *str, int n) {
    if (n <= 0) return "";
    char *result = (char*)malloc(n + 1);
    if (!result) return NULL; // Check for malloc failure
    strncpy(result, str, n);
    result[n] = '\0';
    return result;
}

char* sql_lower(char *str) {
    for (char *p = str; *p; ++p) *p = tolower(*p);
    return str;
}

char* sql_upper(char *str) {
    for (char *p = str; *p; ++p) *p = toupper(*p);
    return str;
}

char* sql_replace(char *str, char old_char, char new_char) {
    char *result = strdup(str);
    if (!result) return NULL; // Check for strdup failure
    for (char *p = result; *p; ++p) {
        if (*p == old_char) *p = new_char;
    }
    return result;
}

TestResult AK_api_functions_test() {
    int successful = 0, failed = 0;

    // Testiranje aritmetičkih operacija za cjelobrojne vrijednosti
    if (sql_add_int(2, 3) == 5 && sql_subtract_int(5, 3) == 2 &&
        sql_multiply_int(2, 3) == 6 && sql_divide_int(6, 3) == 2 &&
        sql_modulo_int(5, 3) == 2 && sql_power_int(2, 3) == 8) {
        successful++;
    } else {
        failed++;
    }

    // Testiranje aritmetičkih operacija za decimalne vrijednosti
    if (sql_add_float(2.5f, 3.5f) == 6.0f && sql_subtract_float(5.5f, 3.5f) == 2.0f &&
        sql_multiply_float(2.5f, 3.0f) == 7.5f && sql_divide_float(7.5f, 3.0f) == 2.5f &&
        sql_power_float(2.0f, 3.0f) == 8.0f) {
        successful++;
    } else {
        failed++;
    }

    // Testiranje standardnih funkcija
    if (strcmp(sql_now(), "") != 0 && sql_len("hello") == 5 &&
        strcmp(sql_left("hello", 3), "hel") == 0 && strcmp(sql_lower("HELLO"), "hello") == 0 &&
        strcmp(sql_upper("hello"), "HELLO") == 0 && strcmp(sql_replace("hello", 'l', 'r'), "herro") == 0) {
        successful++;
    } else {
        failed++;
    }

    // Vraćanje rezultata testiranja
    return TEST_result(successful, failed);
}
