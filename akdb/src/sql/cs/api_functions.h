/*
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/**
 @file api_functions.h Header file that provides basic SQL operations and standard functions.
*/

#include "../auxi/test.h"
#ifndef API_FUNCTIONS_H
#define API_FUNCTIONS_H

#include <time.h>
#include <string.h>
#include <ctype.h>


/**
 * Basic arithmetic operations
 */
int sql_add_int(int a, int b);
int sql_subtract_int(int a, int b);
int sql_multiply_int(int a, int b);
int sql_divide_int(int a, int b);
int sql_modulo_int(int a, int b);
int sql_power_int(int a, int b);

float sql_add_float(float a, float b);
float sql_subtract_float(float a, float b);
float sql_multiply_float(float a, float b);
float sql_divide_float(float a, float b);
float sql_power_float(float a, float b);

/**
 * Standard functions
 */
char* sql_now();
int sql_len(const char *str);
char* sql_left(const char *str, int n);
char* sql_lower(char *str);
char* sql_upper(char *str);
char* sql_replace(char *str, char old_char, char new_char);

#endif // API_FUNCTIONS_H
