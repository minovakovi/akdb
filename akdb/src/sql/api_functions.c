#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "function.h"
#include <math.h>
#include <ctype.h>

/**
 * @author Sara Sušac
 * @brief Function to add two integers.
 * @param a First integer operand.
 * @param b Second integer operand.
 * @return Result of the addition operation.
 */
int add_int(int a, int b) {
    return a + b;
}

/**
 * @author Sara Sušac
 * @brief Function to subtract two integers.
 * @param a First integer operand.
 * @param b Second integer operand.
 * @return Result of the subtraction operation.
 */
int subtract_int(int a, int b) {
    return a - b;
}

/**
 * @author Sara Sušac
 * @brief Function to multiply two integers.
 * @param a First integer operand.
 * @param b Second integer operand.
 * @return Result of the multiplication operation.
 */
int multiply_int(int a, int b) {
    return a * b;
}

/**
 * @author Sara Sušac
 * @brief Function to divide two integers.
 * @param a Numerator.
 * @param b Denominator.
 * @return Result of the division operation.
 * @note Returns 0 if division by zero occurs.
 */
int divide_int(int a, int b) {
    if (b != 0) {
        return a / b;
    } else {
        printf("Error: Division by zero\n");
        return 0;
    }
}

/**
 * @author Sara Sušac
 * @brief Function to calculate the power of an integer.
 * @param a The base number.
 * @param b The exponent.
 * @return The result of raising the base to the power of the exponent.
 */
int power_int(int a, int b) {
    int result = 1;
    for (int i = 0; i < b; i++) {
        result *= a;
    }
    return result;
}

/**
 * @author Sara Sušac
 * @brief Function to calculate the modulus of two integers.
 * @param a The dividend.
 * @param b The divisor.
 * @return The remainder of the division operation.
 * @note Returns 0 if division by zero occurs.
 */
int modulus_int(int a, int b) {
    if (b != 0) {
        return a % b;
    } else {
        printf("Error: Division by zero\n");
        return 0;
    }
}

/**
 * @author Sara Sušac
 * @brief Function to add two floats.
 * @param a The first float operand.
 * @param b The second float operand.
 * @return The sum of the two floats.
 */
float add_float(float a, float b) {
    return a + b;
}

/**
 * @author Sara Sušac
 * @brief Function to subtract two floats.
 * @param a The first float operand.
 * @param b The second float operand.
 * @return The difference between the two floats.
 */
float subtract_float(float a, float b) {
    return a - b;
}

/**
 * @author Sara Sušac
 * @brief Function to multiply two floats.
 * @param a The first float operand.
 * @param b The second float operand.
 * @return The product of the two floats.
 */
float multiply_float(float a, float b) {
    return a * b;
}

/**
 * @author Sara Sušac
 * @brief Function to divide two floats.
 * @param a The numerator.
 * @param b The denominator.
 * @return The result of the division operation.
 * @note Returns 0.0 if division by zero occurs.
 */
float divide_float(float a, float b) {
    if (b != 0.0f) {
        return a / b;
    } else {
        printf("Error: Division by zero\n");
        return 0.0f;
    }
}

/**
 * @author Sara Sušac
 * @brief Function to calculate the power of a float.
 * @param a The base number.
 * @param b The exponent.
 * @return The result of raising the base to the power of the exponent.
 * @note If the exponent is a non-negative float, the function performs multiplication;
 *       if the exponent is a negative float, the function performs division.
 */
float power_float(float a, float b) {
    float result = 1.0;
    
    if (b >= 0) {
        for (int i = 0; i < b; i++) {
            result *= a;
        }
    } else {
        for (int i = 0; i > b; i--) {
            result /= a;
        }
    }
    
    return result;
}

/**
 * @author Sara Sušac
 * @brief Function to calculate the modulus of two floats.
 * @param a The dividend.
 * @param b The divisor.
 * @return The result of the modulus operation.
 * @note The modulus operator is not defined for floats. This function returns 0.0f and prints an error message.
 */
float modulus_float(float a, float b) {
    printf("Error: Modulus operator not defined for float\n");
    return 0.0f;
}

/**
 * @author Sara Sušac
 * @brief Function to get the current date and time.
 * @return A string representing the current date and time.
 */
char* now() {
    time_t current_time;
    time(&current_time);
    return ctime(&current_time);
}

/**
 * @author Sara Sušac
 * @brief Function to get the length of a string.
 * @param str The input string.
 * @return The length of the string.
 */
int len(char* str) {
    return strlen(str);
}

/**
 * @author Sara Sušac
 * @brief Function to get the leftmost characters of a string.
 * @param str The input string.
 * @param length The desired length of the left substring.
 * @return The leftmost substring of the input string with the specified length.
 */
char* left(char* str, int length) {
    int str_length = strlen(str);
    int result_length = (length < str_length) ? length : str_length;
    char* result = malloc(result_length + 1);
    strncpy(result, str, result_length);
    result[result_length] = '\0';
    return result;
}

/**
 * @author Sara Sušac
 * @brief Function to convert a string to lowercase.
 * @param str The input string.
 * @return A new string with all characters converted to lowercase.
 */
char* lower(char* str) {
    int length = strlen(str);
    char* result = malloc(length + 1);
    for (int i = 0; i < length; i++) {
        result[i] = tolower(str[i]);
    }
    result[length] = '\0';
    return result;
}

/**
 * @author Sara Sušac
 * @brief Function to replace a substring with another substring in a string.
 * @param str The input string.
 * @param old_sub The substring to be replaced.
 * @param new_sub The replacement substring.
 * @return A new string with all occurrences of the old substring replaced by the new substring.
 */
char* replace(char* str, char* old_sub, char* new_sub) {
    char* result;
    char* pos;
    int old_sub_len = strlen(old_sub);
    int new_sub_len = strlen(new_sub);
    int count = 0;
    
    // Count the number of occurrences of old_sub in str
    pos = str;
    while ((pos = strstr(pos, old_sub)) != NULL) {
        count++;
        pos += old_sub_len;
    }
    // Allocate memory for the result string
    result = malloc(strlen(str) + count * (new_sub_len - old_sub_len) + 1);
    
    // Perform the replacement
    pos = str;
    while ((pos = strstr(pos, old_sub)) != NULL) {
        strncpy(result, str, pos - str);
        result[pos - str] = '\0';
        strcat(result, new_sub);
        strcat(result, pos + old_sub_len);
        pos += old_sub_len;
        str = pos;
    }
    
    strcat(result, str);
    
    return result;
}

/**
 * @author Sara Sušac
 * @brief Function to convert a string to uppercase.
 * @param str The input string.
 * @return A new string with all characters converted to uppercase.
 */
char* upper(char* str) {
    int length = strlen(str);
    char* result = malloc(length + 1);
    for (int i = 0; i < length; i++) {
        result[i] = toupper(str[i]);
    }
    result[length] = '\0';
    return result;
}

/**
 * @author Sara Sušac
 * @brief Function to calculate the absolute value of an integer.
 * @param num The input integer.
 * @return The absolute value of the input integer.
 */
int abs(int num) {
    return (num < 0) ? -num : num;
}


/**
 * @author Sara Sušac
 * @brief Function to perform the SQL functions based on the function name.
 * @param function_name The name of the SQL function.
 * @param data_type The data type of the operand.
 * @param operand The input operand.
 * @param result The output result of the SQL function.
 */
void perform_sql_function(char* function_name, int data_type, void* operand, void* result) {
    if (strcmp(function_name, "now") == 0) {
        if (data_type == 3) {
            *(char**)result = now();
        }
    }
}

/**
 * @author Sara Sušac
 * @brief Function to perform SQL operator based on data types.
 * @param operator_type The type of SQL operator to perform.
 * @param data_type The data type of the operands.
 * @param operand1 The first operand.
 * @param operand2 The second operand.
 * @param result The output result of the SQL operator.
 */
void perform_sql_operator(int operator_type, int data_type, void* operand1, void* operand2, void* result) {
    switch (operator_type) {
        case 0: // Addition
            if (data_type == 1) {
                *(int*)result = add_int(*(int*)operand1, *(int*)operand2);
            } else if (data_type == 2) {
                *(float*)result = add_float(*(float*)operand1, *(float*)operand2);
            }
            break;
        case 1: // Subtraction
            if (data_type == 1) {
                *(int*)result = subtract_int(*(int*)operand1, *(int*)operand2);
            } else if (data_type == 2) {
                *(float*)result = subtract_float(*(float*)operand1, *(float*)operand2);
            }
            break;
        case 2: // Multiplication
            if (data_type == 1) {
                *(int*)result = multiply_int(*(int*)operand1, *(int*)operand2);
            } else if (data_type == 2) {
                *(float*)result = multiply_float(*(float*)operand1, *(float*)operand2);
            }
            break;
        case 3: // Division
            if (data_type == 1) {
                *(int*)result = divide_int(*(int*)operand1, *(int*)operand2);
            } else if (data_type == 2) {
                *(float*)result = divide_float(*(float*)operand1, *(float*)operand2);
            }
            break;
        case 4: // Power
            if (data_type == 1) {
            *(int*)result = power_int(*(int*)operand1, *(int*)operand2);
            } else if (data_type == 2) {
            *(float*)result = power_float(*(float*)operand1, *(float*)operand2);
            }
        break;
        case 5: // Modulus
            if (data_type == 1) {
            *(int*)result = modulus_int(*(int*)operand1, *(int*)operand2);
            } else if (data_type == 2) {
            *(float*)result = modulus_float(*(float*)operand1, *(float*)operand2);
            }
            break;
        default:
        printf("Error: Invalid operator type\n");
        break;
            }
}
