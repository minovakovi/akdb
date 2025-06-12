/**
@file python_proc.c Provides Python procedural language support for AKDB
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

#include "python_proc.h"
#include <Python.h>
#include <string.h>

static int python_initialized = 0;

/**
 * @author Frane Suman
 * @brief Initialize Python procedural language support
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_python_proc_init() {
    AK_PRO;
    
    if (python_initialized) {
        AK_EPI;
        return EXIT_SUCCESS;
    }
    
    // Initialize Python interpreter
    Py_Initialize();
    if (!Py_IsInitialized()) {
        AK_dbg_messg(HIGH, FUNCTIONS, "AK_python_proc_init: Failed to initialize Python interpreter\n");
        AK_EPI;
        return EXIT_ERROR;
    }
    
    // Import required modules and setup AKDB bridge
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("import traceback");
    
    // Add AKDB module to Python path
    char python_setup[] = 
        "class AKDBContext:\n"
        "    def __init__(self):\n"
        "        self.old_values = {}\n"
        "        self.new_values = {}\n"
        "        self.table_name = ''\n"
        "        self.trigger_event = ''\n"
        "        self.function_args = []\n"
        "    \n"
        "    def get_old(self, column):\n"
        "        return self.old_values.get(column)\n"
        "    \n"
        "    def get_new(self, column):\n"
        "        return self.new_values.get(column)\n"
        "    \n"
        "    def get_arg(self, index):\n"
        "        if 0 <= index < len(self.function_args):\n"
        "            return self.function_args[index]\n"
        "        return None\n"
        "\n"
        "akdb = AKDBContext()\n";
    
    if (PyRun_SimpleString(python_setup) != 0) {
        AK_dbg_messg(HIGH, FUNCTIONS, "AK_python_proc_init: Failed to setup AKDB Python context\n");
        Py_Finalize();
        AK_EPI;
        return EXIT_ERROR;
    }
    
    python_initialized = 1;
    AK_dbg_messg(LOW, FUNCTIONS, "AK_python_proc_init: Python procedural language initialized\n");
    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Frane Suman
 * @brief Cleanup Python procedural language support
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_python_proc_cleanup() {
    AK_PRO;
    
    if (!python_initialized) {
        AK_EPI;
        return EXIT_SUCCESS;
    }
    
    Py_Finalize();
    python_initialized = 0;
    
    AK_dbg_messg(LOW, FUNCTIONS, "AK_python_proc_cleanup: Python procedural language cleaned up\n");
    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Frane Suman
 * @brief Add a procedural function to the system
 * @param *name name of the function
 * @param return_type data type returned from function
 * @param *arguments_list list of function arguments
 * @param language language of the function (LANG_PYTHON, LANG_R, etc.)
 * @param *code the function code
 * @return function id or EXIT_ERROR
 */
int AK_function_add_procedural(char *name, int return_type, struct list_node *arguments_list, int language, char *code) {
    AK_PRO;
    
    if (name == NULL || code == NULL) {
        AK_dbg_messg(HIGH, FUNCTIONS, "AK_function_add_procedural: Invalid parameters\n");
        AK_EPI;
        return EXIT_ERROR;
    }
    
    if (language == LANG_PYTHON && !python_initialized) {
        if (AK_python_proc_init() != EXIT_SUCCESS) {
            AK_EPI;
            return EXIT_ERROR;
        }
    }
    
    // Create the function entry
    struct list_node *row_root = (struct list_node *)AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row_root);
    
    int function_id = AK_get_id();
    int num_args = AK_Size_L2(arguments_list) / 2;
    
    AK_Insert_New_Element(TYPE_INT, &function_id, "AK_function", "obj_id", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, name, "AK_function", "name", row_root);
    AK_Insert_New_Element(TYPE_INT, &num_args, "AK_function", "arg_num", row_root);
    AK_Insert_New_Element(TYPE_INT, &return_type, "AK_function", "return_type", row_root);
    AK_Insert_New_Element(TYPE_INT, &language, "AK_function", "language", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, code, "AK_function", "code", row_root);
    
    if (AK_insert_row(row_root) == EXIT_ERROR) {
        AK_free(row_root);
        AK_EPI;
        return EXIT_ERROR;
    }
    
    // Add function arguments if any
    if (num_args > 0) {
        int i;
        struct list_node *current_elem = AK_First_L2(arguments_list);
        
        for (i = 1; i <= num_args; i++) {
            char *argname = AK_Retrieve_L2(current_elem, arguments_list);
            current_elem = AK_Next_L2(current_elem);
            char *argtype = AK_Retrieve_L2(current_elem, arguments_list);
            int argtype_int = atoi(argtype);
            current_elem = AK_Next_L2(current_elem);
            
            AK_function_arguments_add(function_id, i, argtype_int, argname);
        }
    }
    
    AK_free(row_root);
    AK_dbg_messg(LOW, FUNCTIONS, "AK_function_add_procedural: Added procedural function %s (ID: %d, Language: %d)\n", name, function_id, language);
    AK_EPI;
    return function_id;
}

/**
 * @author Frane Suman
 * @brief Execute a Python function
 * @param *context function execution context
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_function_execute_python(AK_function_context *context) {
    AK_PRO;
    
    if (!python_initialized) {
        if (AK_python_proc_init() != EXIT_SUCCESS) {
            AK_EPI;
            return EXIT_ERROR;
        }
    }
    
    if (context == NULL || context->code == NULL) {
        AK_dbg_messg(HIGH, FUNCTIONS, "AK_function_execute_python: Invalid context\n");
        AK_EPI;
        return EXIT_ERROR;
    }
    
    // Set up context in Python
    PyObject *main_module = PyImport_AddModule("__main__");
    PyObject *main_dict = PyModule_GetDict(main_module);
    PyObject *akdb_obj = PyDict_GetItemString(main_dict, "akdb");
    
    if (akdb_obj == NULL) {
        AK_dbg_messg(HIGH, FUNCTIONS, "AK_function_execute_python: AKDB context not found\n");
        AK_EPI;
        return EXIT_ERROR;
    }
    
    // Set function arguments
    if (context->arguments != NULL) {
        PyObject *args_list = PyList_New(0);
        struct list_node *current = AK_First_L2(context->arguments);
        
        while (current != NULL) {
            PyObject *arg_val = NULL;
            
            switch (current->type) {
                case TYPE_INT:
                    arg_val = PyLong_FromLong(*(int*)current->data);
                    break;
                case TYPE_FLOAT:
                    arg_val = PyFloat_FromDouble(*(float*)current->data);
                    break;
                case TYPE_VARCHAR:
                    arg_val = PyUnicode_FromString((char*)current->data);
                    break;
                default:
                    arg_val = PyUnicode_FromString((char*)current->data);
                    break;
            }
            
            if (arg_val != NULL) {
                PyList_Append(args_list, arg_val);
                Py_DECREF(arg_val);
            }
            
            current = AK_Next_L2(current);
        }
        
        PyObject_SetAttrString(akdb_obj, "function_args", args_list);
        Py_DECREF(args_list);
    }
    
    // Set trigger context if available
    if (context->old_values != NULL) {
        PyObject *old_dict = PyDict_New();
        struct list_node *current = AK_First_L2(context->old_values);
        
        while (current != NULL) {
            const char *attr_name = current->attribute_name;
            PyObject *val = NULL;
            
            switch (current->type) {
                case TYPE_INT:
                    val = PyLong_FromLong(*(int*)current->data);
                    break;
                case TYPE_FLOAT:
                    val = PyFloat_FromDouble(*(float*)current->data);
                    break;
                case TYPE_VARCHAR:
                    val = PyUnicode_FromString((char*)current->data);
                    break;
                default:
                    val = PyUnicode_FromString((char*)current->data);
                    break;
            }
            
            if (val != NULL && attr_name != NULL) {
                PyDict_SetItemString(old_dict, attr_name, val);
                Py_DECREF(val);
            }
            
            current = AK_Next_L2(current);
        }
        
        PyObject_SetAttrString(akdb_obj, "old_values", old_dict);
        Py_DECREF(old_dict);
    }
    
    // Set NEW values for triggers
    if (context->new_values != NULL) {
        PyObject *new_dict = PyDict_New();
        struct list_node *current = AK_First_L2(context->new_values);
        
        while (current != NULL) {
            const char *attr_name = current->attribute_name;
            PyObject *val = NULL;
            
            switch (current->type) {
                case TYPE_INT:
                    val = PyLong_FromLong(*(int*)current->data);
                    break;
                case TYPE_FLOAT:
                    val = PyFloat_FromDouble(*(float*)current->data);
                    break;
                case TYPE_VARCHAR:
                    val = PyUnicode_FromString((char*)current->data);
                    break;
                default:
                    val = PyUnicode_FromString((char*)current->data);
                    break;
            }
            
            if (val != NULL && attr_name != NULL) {
                PyDict_SetItemString(new_dict, attr_name, val);
                Py_DECREF(val);
            }
            
            current = AK_Next_L2(current);
        }
        
        PyObject_SetAttrString(akdb_obj, "new_values", new_dict);
        Py_DECREF(new_dict);
    }
    
    // Set table name and trigger event
    if (context->table_name != NULL) {
        PyObject *table_str = PyUnicode_FromString(context->table_name);
        PyObject_SetAttrString(akdb_obj, "table_name", table_str);
        Py_DECREF(table_str);
    }
    
    if (context->trigger_event != NULL) {
        PyObject *event_str = PyUnicode_FromString(context->trigger_event);
        PyObject_SetAttrString(akdb_obj, "trigger_event", event_str);
        Py_DECREF(event_str);
    }
    
    // Execute the Python code
    int result = PyRun_SimpleString(context->code);
    
    if (result != 0) {
        AK_dbg_messg(HIGH, FUNCTIONS, "AK_function_execute_python: Python execution failed for function %s\n", 
                     context->function_name ? context->function_name : "unknown");
        
        // Print Python traceback
        PyRun_SimpleString("traceback.print_exc()");
        AK_EPI;
        return EXIT_ERROR;
    }
    
    AK_dbg_messg(LOW, FUNCTIONS, "AK_function_execute_python: Successfully executed Python function %s\n", 
                 context->function_name ? context->function_name : "unknown");
    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Frane Suman  
 * @brief Execute a procedural function
 * @param function_id ID of the function to execute
 * @param *arguments function arguments
 * @param *old_vals OLD row values (for triggers)
 * @param *new_vals NEW row values (for triggers)
 * @param *table table name (for triggers)
 * @param *event trigger event (for triggers)
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_function_execute_procedural(int function_id, struct list_node *arguments, struct list_node *old_vals, struct list_node *new_vals, char *table, char *event) {
    AK_PRO;
    
    // Get function details
    char *function_name = NULL;
    char *code = AK_function_get_code(function_id);
    int language = AK_function_get_language(function_id);
    
    if (code == NULL) {
        AK_dbg_messg(HIGH, FUNCTIONS, "AK_function_execute_procedural: Function code not found for ID %d\n", function_id);
        AK_EPI;
        return EXIT_ERROR;
    }
    
    // Get function name
    int i = 0;
    struct list_node *row;
    while ((row = (struct list_node *)AK_get_row(i, "AK_function")) != NULL) {
        struct list_node *id_elem = AK_GetNth_L2(1, row);
        int stored_id;
        memcpy(&stored_id, id_elem->data, sizeof(int));
        
        if (stored_id == function_id) {
            struct list_node *name_elem = AK_GetNth_L2(2, row);
            function_name = AK_malloc(strlen((char*)name_elem->data) + 1);
            strcpy(function_name, (char*)name_elem->data);
            break;
        }
        i++;
    }
    
    // Create execution context
    AK_function_context context;
    context.function_id = function_id;
    context.function_name = function_name;
    context.language = language;
    context.code = code;
    context.arguments = arguments;
    context.old_values = old_vals;
    context.new_values = new_vals;
    context.table_name = table;
    context.trigger_event = event;
    
    int result = EXIT_ERROR;
    
    switch (language) {
        case LANG_PYTHON:
            result = AK_function_execute_python(&context);
            break;
        case LANG_C:
            // Traditional C function execution would go here
            AK_dbg_messg(MIDDLE, FUNCTIONS, "AK_function_execute_procedural: C function execution not implemented in procedural context\n");
            break;
        case LANG_R:
        case LANG_PROLOG:
            AK_dbg_messg(MIDDLE, FUNCTIONS, "AK_function_execute_procedural: Language %d not yet implemented\n", language);
            break;
        default:
            AK_dbg_messg(HIGH, FUNCTIONS, "AK_function_execute_procedural: Unknown language %d\n", language);
            break;
    }
    
    // Cleanup
    if (function_name) AK_free(function_name);
    if (code) AK_free(code);
    
    AK_EPI;
    return result;
}

/**
 * @author Frane Suman
 * @brief Execute a procedural function from a trigger
 * @param trigger_id ID of the trigger
 * @param *old_row OLD row data
 * @param *new_row NEW row data  
 * @param *table table name
 * @param *event trigger event
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_trigger_execute_procedural(int trigger_id, struct list_node *old_row, struct list_node *new_row, char *table, char *event) {
    AK_PRO;
    
    // Get trigger details
    int function_id = -1;
    int i = 0;
    struct list_node *row;
    
    while ((row = (struct list_node *)AK_get_row(i, "AK_trigger")) != NULL) {
        struct list_node *id_elem = AK_GetNth_L2(1, row);
        int stored_id;
        memcpy(&stored_id, id_elem->data, sizeof(int));
        
        if (stored_id == trigger_id) {
            struct list_node *action_elem = AK_GetNth_L2(5, row); // action column contains function_id
            memcpy(&function_id, action_elem->data, sizeof(int));
            break;
        }
        i++;
    }
    
    if (function_id == -1) {
        AK_dbg_messg(HIGH, TRIGGERS, "AK_trigger_execute_procedural: Function ID not found for trigger %d\n", trigger_id);
        AK_EPI;
        return EXIT_ERROR;
    }
    
    // Execute the function with trigger context
    int result = AK_function_execute_procedural(function_id, NULL, old_row, new_row, table, event);
    
    AK_EPI;
    return result;
}

/**
 * @author Frane Suman
 * @brief Get function code by function ID
 * @param function_id ID of the function
 * @return function code or NULL
 */
char* AK_function_get_code(int function_id) {
    AK_PRO;
    
    int i = 0;
    struct list_node *row;
    char *code = NULL;
    
    while ((row = (struct list_node *)AK_get_row(i, "AK_function")) != NULL) {
        struct list_node *id_elem = AK_GetNth_L2(1, row);
        int stored_id;
        memcpy(&stored_id, id_elem->data, sizeof(int));
        
        if (stored_id == function_id) {
            // Check if function has code column (procedural function)
            if (AK_Size_L2(row) >= 6) {
                struct list_node *code_elem = AK_GetNth_L2(6, row);
                if (code_elem && code_elem->data) {
                    code = AK_malloc(strlen((char*)code_elem->data) + 1);
                    strcpy(code, (char*)code_elem->data);
                }
            }
            break;
        }
        i++;
    }
    
    AK_EPI;
    return code;
}

/**
 * @author Frane Suman
 * @brief Get function language by function ID
 * @param function_id ID of the function
 * @return language code or LANG_C (default)
 */
int AK_function_get_language(int function_id) {
    AK_PRO;
    
    int i = 0;
    struct list_node *row;
    int language = LANG_C; // default
    
    while ((row = (struct list_node *)AK_get_row(i, "AK_function")) != NULL) {
        struct list_node *id_elem = AK_GetNth_L2(1, row);
        int stored_id;
        memcpy(&stored_id, id_elem->data, sizeof(int));
        
        if (stored_id == function_id) {
            // Check if function has language column
            if (AK_Size_L2(row) >= 5) {
                struct list_node *lang_elem = AK_GetNth_L2(5, row);
                if (lang_elem && lang_elem->data) {
                    memcpy(&language, lang_elem->data, sizeof(int));
                }
            }
            break;
        }
        i++;
    }
    
    AK_EPI;
    return language;
}

/**
 * @author Frane Suman
 * @brief Test Python procedural language functionality
 * @return test result
 */
TestResult AK_python_proc_test() {
    AK_PRO;
    
    int successful_tests = 0;
    int failed_tests = 0;
    
    printf("python_proc.c: Testing Python procedural language support\n");
    
    // Test 1: Initialize Python
    printf("\nTest 1: Initialize Python procedural language\n");
    if (AK_python_proc_init() == EXIT_SUCCESS) {
        printf("PASSED: Python initialization successful\n");
        successful_tests++;
    } else {
        printf("FAILED: Python initialization failed\n");
        failed_tests++;
    }
    
    // Test 2: Add Python function
    printf("\nTest 2: Add Python function\n");
    struct list_node *args = (struct list_node *)AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&args);
    
    AK_InsertAtEnd_L3(TYPE_VARCHAR, "x", sizeof("x"), args);
    AK_InsertAtEnd_L3(TYPE_INT, "5", sizeof(int), args);
    AK_InsertAtEnd_L3(TYPE_VARCHAR, "y", sizeof("y"), args);
    AK_InsertAtEnd_L3(TYPE_INT, "5", sizeof(int), args);
    
    char python_code[] = 
        "def test_add():\n"
        "    x = akdb.get_arg(0)\n"
        "    y = akdb.get_arg(1)\n"
        "    result = int(x) + int(y)\n"
        "    print('Python function executed: {} + {} = {}'.format(x, y, result))\n"
        "    return result\n"
        "\n"
        "test_add()\n";
    
    int func_id = AK_function_add_procedural("python_add", TYPE_INT, args, LANG_PYTHON, python_code);
    if (func_id != EXIT_ERROR) {
        printf("PASSED: Python function added with ID %d\n", func_id);
        successful_tests++;
        
        // Test 3: Execute Python function
        printf("\nTest 3: Execute Python function\n");
        struct list_node *test_args = (struct list_node *)AK_malloc(sizeof(struct list_node));
        AK_Init_L3(&test_args);
        AK_InsertAtEnd_L3(TYPE_INT, "10", sizeof(int), test_args);
        AK_InsertAtEnd_L3(TYPE_INT, "20", sizeof(int), test_args);
        
        if (AK_function_execute_procedural(func_id, test_args, NULL, NULL, NULL, NULL) == EXIT_SUCCESS) {
            printf("PASSED: Python function execution successful\n");
            successful_tests++;
        } else {
            printf("FAILED: Python function execution failed\n");
            failed_tests++;
        }
        
        AK_DeleteAll_L3(&test_args);
        AK_free(test_args);
        
    } else {
        printf("FAILED: Python function addition failed\n");
        failed_tests++;
    }
    
    // Test 4: Add Python trigger function
    printf("\nTest 4: Add Python trigger function\n");
    char trigger_code[] = 
        "def trigger_function():\n"
        "    print('Trigger executed on table:', akdb.table_name)\n"
        "    print('Trigger event:', akdb.trigger_event)\n"
        "    \n"
        "    if akdb.old_values:\n"
        "        print('OLD values:', akdb.old_values)\n"
        "    if akdb.new_values:\n"
        "        print('NEW values:', akdb.new_values)\n"
        "    \n"
        "    # Example: Log the change\n"
        "    if akdb.trigger_event == 'INSERT':\n"
        "        print('New row inserted')\n"
        "    elif akdb.trigger_event == 'UPDATE':\n"
        "        print('Row updated')\n"
        "    elif akdb.trigger_event == 'DELETE':\n"
        "        print('Row deleted')\n"
        "    \n"
        "    return True\n"
        "\n"
        "trigger_function()\n";
    
    struct list_node *trigger_args = (struct list_node *)AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&trigger_args);
    
    int trigger_func_id = AK_function_add_procedural("python_trigger", TYPE_INT, trigger_args, LANG_PYTHON, trigger_code);
    if (trigger_func_id != EXIT_ERROR) {
        printf("PASSED: Python trigger function added with ID %d\n", trigger_func_id);
        successful_tests++;
    } else {
        printf("FAILED: Python trigger function addition failed\n");
        failed_tests++;
    }
    
    // Cleanup
    AK_DeleteAll_L3(&args);
    AK_free(args);
    AK_DeleteAll_L3(&trigger_args);
    AK_free(trigger_args);
    
    // Test 5: Cleanup Python
    printf("\nTest 5: Cleanup Python procedural language\n");
    if (AK_python_proc_cleanup() == EXIT_SUCCESS) {
        printf("PASSED: Python cleanup successful\n");
        successful_tests++;
    } else {
        printf("FAILED: Python cleanup failed\n");
        failed_tests++;
    }
    
    printf("\nPython Procedural Language Tests Summary:\n");
    printf("Successful tests: %d\n", successful_tests);
    printf("Failed tests: %d\n", failed_tests);
    
    AK_EPI;
    return TEST_result(successful_tests, failed_tests);
}
