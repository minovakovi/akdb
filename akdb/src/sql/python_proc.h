/**
 @file python_proc.h Header file for Python procedural language support
*/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#ifndef PYTHON_PROC
#define PYTHON_PROC

#include "../auxi/test.h"
#include "../file/table.h"
#include "../file/fileio.h"
#include "../file/id.h"
#include "../sql/function.h"
#include "../auxi/mempro.h"

// Language types for functions
#define LANG_C          0
#define LANG_PYTHON     1


// Function execution context
typedef struct {
    int function_id;
    char *function_name;
    int language;
    char *code;
    struct list_node *arguments;
    struct list_node *old_values;  // For triggers - OLD row data
    struct list_node *new_values;  // For triggers - NEW row data
    char *table_name;               // For triggers - affected table
    char *trigger_event;            // For triggers - INSERT/UPDATE/DELETE
} AK_function_context;

// Function prototypes
int AK_python_proc_init();
int AK_python_proc_cleanup();
int AK_function_add_procedural(char *name, int return_type, struct list_node *arguments_list, int language, char *code);
int AK_function_execute_python(AK_function_context *context);
int AK_function_execute_procedural(int function_id, struct list_node *arguments, struct list_node *old_vals, struct list_node *new_vals, char *table, char *event);
int AK_trigger_execute_procedural(int trigger_id, struct list_node *old_row, struct list_node *new_row, char *table, char *event);
char* AK_function_get_code(int function_id);
int AK_function_get_language(int function_id);
TestResult AK_python_proc_test();

// Python bridge functions
int AK_python_execute_string(char *code);
int AK_python_execute_function(char *function_name, struct list_node *args);
struct list_node* AK_python_get_result();
int AK_python_set_context(AK_function_context *context);

#endif
