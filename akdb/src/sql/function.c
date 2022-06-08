/**
@file function.c Provides functions for functions
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
 * 
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#include "function.h"

/**
 * @author Unknown, updated by Jurica Hlevnjak - check function arguments included for drop purpose, updated by Tomislav Ilisevic
 * @brief Function that gets obj_id of a function by name and arguments list (transferred from trigger.c/drop.c).
 * @param *function name of the function
 * @param *arguments_list list of arguments
 * @return obj_id of the function or EXIT_ERROR
 */
int AK_get_function_obj_id(char *function, struct list_node *arguments_list)
{
    int i = 0, id = -1, result, arg_num;
    struct list_node *row;

    int num_args;
    AK_PRO;
    num_args = AK_Size_L2(arguments_list) / 2; // u paru "naziv" - "vrsta" argumenta pa / 2

    while ((row = (struct list_node *)AK_get_row(i, "AK_function")) != NULL)
    {
        struct list_node *elem_in_memcpy = AK_GetNth_L2(3, row);
        memcpy(&arg_num, elem_in_memcpy->data, sizeof(int));
        struct list_node *elem_in_strcmp = AK_GetNth_L2(2, row);
        if ((strcmp(elem_in_strcmp->data, function) == 0) && (arg_num == num_args))
        {
            struct list_node *elem_in_memcpy2 = AK_GetNth_L2(1, row);
            memcpy(&id, elem_in_memcpy2->data, sizeof(int));

            if (num_args > 0)
            {
                result = AK_check_function_arguments_type(id, arguments_list); // check_type je za drop
            }
            else
            {
                result = EXIT_SUCCESS;
            }

            if (result != EXIT_ERROR)
            {
                AK_EPI;
                return id;
            }
        }
        i++;
    }

    AK_EPI;
    return EXIT_ERROR;
}

/**
 * @author Boris Kišić
 * @brief Function that checks whether arguments belongs to a function
 * @param *function_id id of the function
 * @param *arguments_list list of arguments
 * @return EXIT_SUCCESS of the function or EXIT_ERROR
 */
int AK_check_function_arguments(int function_id, struct list_node *arguments_list)
{
    //int AK_check_function_arguments(int function_id, AK_list *arguments_list) {

    struct list_node *row;
    int i = 0, fid;
    AK_PRO;

    struct list_node *arguments_list_current = arguments_list->next;

    char *argtype_catalog;
    char *argname_catalog;
    char *arguments_list_argname;
    char *arguments_list_argtype;

    while ((row = (struct list_node *)AK_get_row(i, "AK_function_arguments")) != NULL)
    {
        struct list_node *current_elem = AK_First_L2(row); //set current_elem to first element in a list
        memcpy(&fid, current_elem->data, sizeof(int));
        printf("\n %d  %d function id: ", function_id, fid);

        if (fid == function_id)
        {
            current_elem = AK_Next_L2(current_elem);
            argtype_catalog = current_elem->data;
            current_elem = AK_Next_L2(current_elem);
            argname_catalog = current_elem->data;

            printf("\n %s %s", argtype_catalog, argname_catalog);

            arguments_list_argname = arguments_list_current->data;
            arguments_list_current = arguments_list_current->next;
            arguments_list_argtype = arguments_list_current->data;
            arguments_list_current = arguments_list_current->next;

            printf("\n %s %s %s %s", argtype_catalog, arguments_list_argtype, argname_catalog, arguments_list_argname);

            if (strcmp(argtype_catalog, arguments_list_argtype) != 0 || strcmp(argname_catalog, arguments_list_argname) != 0)
            {
                AK_EPI;
                return EXIT_ERROR;
            }
        }
        i++;
    }
    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Jurica Hlevnjak updated by Aleksandra Polak
 * @brief Function that checks whether arguments belongs to a function but only checks argument type (not name). Used for drop function.
 * @param function_id id of the function
 * @param args function arguments 
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_check_function_arguments_type(int function_id, struct list_node *args)
{
    struct list_node *row;
    int i = 0, tip = 0, fid;
    AK_PRO;
    struct list_node *arguments_list_current = args->next;

    char *argtype;
    char *args_argtype;
    while ((row = (struct list_node *)AK_get_row(i, "AK_function_arguments")) != NULL)
    {
        struct list_node *current_elem = AK_First_L2(row);

        memcpy(&fid, current_elem->data, sizeof(int));

        if (fid == function_id)
        {
            current_elem = AK_Next_L2(current_elem);
            current_elem = AK_Next_L2(current_elem);

            argtype = current_elem->data;

            arguments_list_current = arguments_list_current->next;
            args_argtype = arguments_list_current->data;

            argtype = AK_convert_type(argtype);

            arguments_list_current = arguments_list_current->next;

            if (strcmp(argtype, args_argtype) != 0)
            {
                AK_EPI;
                return EXIT_ERROR;
            }
        }
        i++;
    }
    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Boris Kišić, updated by Tomislav Ilisevic
 * @brief Function that adds a function to system table.
 * @param *name name of the function
 * @param *return_type data type returned from a function - values from 0 to 13 - defined in constants.h
 * @param *arguments_list list of function arguments
 * @return function id or EXIT_ERROR
 */
int AK_function_add(char *name, int return_type, struct list_node *arguments_list)
{
    AK_PRO;
    if (return_type < 0 || return_type > 13)
    {
        AK_dbg_messg(HIGH, FUNCTIONS, "AK_function_add: Invalid parameter value for return type.\n");
        AK_EPI;
        return EXIT_ERROR;
    }

    struct list_node *row_root = (struct list_node *)AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row_root);
    int function_id = AK_get_id();                 //get function_id
    int num_args = AK_Size_L2(arguments_list) / 2; //get number of arguments
    AK_Insert_New_Element(TYPE_INT, &function_id, "AK_function", "obj_id", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, name, "AK_function", "name", row_root);
    AK_Insert_New_Element(TYPE_INT, &num_args, "AK_function", "arg_num", row_root);
    AK_Insert_New_Element(TYPE_INT, &return_type, "AK_function", "return_type", row_root);
    AK_insert_row(row_root);

    if (num_args > 0)
    {
        int i;
        struct list_node *current_elem = AK_First_L2(arguments_list); //set current_elem to first element in a list
        //add arguments list to arguments system table
        for (i = 1; i <= num_args; i++)
        {
            char *argname = AK_Retrieve_L2(current_elem, arguments_list);
            current_elem = AK_Next_L2(current_elem);
            char *argtype = AK_Retrieve_L2(current_elem, arguments_list);
            int argtype_int = atoi(argtype);
            current_elem = AK_Next_L2(current_elem);
            AK_function_arguments_add(function_id, i, argtype_int, argname);
        }
    }
    AK_EPI;
    return function_id;
}

/**
 * @author Boris Kišić
 * @brief Function that adds a function argument to system table.
 * @param *function_id id of the function to which the argument belongs
 * @param *arg_number number of the argument
 * @param *arg_type data type of the argument
 * @param *argname name of the argument
 * @return function argument id or EXIT_ERROR
 */
int AK_function_arguments_add(int function_id, int arg_number, int arg_type, char *argname)
{
    AK_PRO;

    struct list_node *row_root = (struct list_node *)AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row_root);
    char *arg_type_name;
    switch (arg_type)
    {
    case 0:
        arg_type_name = "internal";
        break;
    case 1:
        arg_type_name = "int";
        break;
    case 2:
        arg_type_name = "float";
        break;
    case 3:
        arg_type_name = "number";
        break;
    case 4:
        arg_type_name = "varchar";
        break;
    case 5:
        arg_type_name = "date";
        break;
    case 6:
        arg_type_name = "datetime";
        break;
    case 7:
        arg_type_name = "time";
        break;
    case 8:
        arg_type_name = "blob";
        break;
    case 9:
        arg_type_name = "bool";
        break;
    case 10:
        arg_type_name = "operand";
        break;
    case 11:
        arg_type_name = "operator";
        break;
    case 12:
        arg_type_name = "attribs";
        break;
    case 13:
        arg_type_name = "condition";
        break;
    default:
        arg_type_name = "unknown";
        break;
    }
    AK_Insert_New_Element(TYPE_INT, &function_id, "AK_function_arguments", "func_id", row_root);
    AK_Insert_New_Element(TYPE_INT, &arg_number, "AK_function_arguments", "att_num", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, arg_type_name, "AK_function_arguments", "att_type", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, argname, "AK_function_arguments", "att_name", row_root);
    AK_insert_row(row_root);

    AK_EPI;
    return function_id;
}

/**
 * @author Boris Kišić
 * @brief Function that removes a function by its obj_id.
 * @param obj_id obj_id of the function
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_function_remove_by_obj_id(int obj_id)
{
    AK_PRO;

    struct list_node *row_root = (struct list_node *)AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row_root);

    AK_Update_Existing_Element(TYPE_INT, &obj_id, "AK_function", "obj_id", row_root);

    int result = AK_delete_row(row_root);
    if (result == EXIT_ERROR)
    {
        AK_dbg_messg(HIGH, FUNCTIONS, "AK_function_remove_by_obj_id: Could not delete function.\n");
        AK_EPI;
        return EXIT_ERROR;
    }

    //delete function arguments
    AK_DeleteAll_L3(&row_root);
    AK_function_arguments_remove_by_obj_id(&obj_id);
    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Boris Kišić
 * @brief Function that removes function arguments by function id.
 * @param obj_id obj_id of the function
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_function_arguments_remove_by_obj_id(int *obj_id)
{
    // printf("***Remove function arguments***\n");
    int oid;
    AK_PRO;
    memcpy(&oid, obj_id, sizeof(int));

    struct list_node *row_root = (struct list_node *)AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row_root);

    AK_Update_Existing_Element(TYPE_INT, &oid, "AK_function_arguments", "func_id", row_root);
    int result = AK_delete_row(row_root);
    if (result == EXIT_ERROR)
    {
        AK_dbg_messg(HIGH, FUNCTIONS, "AK_function_arguments_remove_by_obj_id: Could not delete arguments.\n");
        AK_EPI;
        return EXIT_ERROR;
    }
    AK_DeleteAll_L3(&row_root);
    AK_free(row_root);
    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Boris Kišić
 * @brief Function that removes a function from system table by name and arguments.
 * @param *name name of the function
 * @param *arguments_list list of arguments
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_function_remove_by_name(char *name, struct list_node *arguments_list)
{
    AK_PRO;
    int func_id = AK_get_function_obj_id(name, arguments_list);
    int result = AK_function_remove_by_obj_id(func_id);

    if (result == EXIT_ERROR || func_id == EXIT_ERROR)
    {
        AK_dbg_messg(HIGH, FUNCTIONS, "AK_function_remove_by_name: Could not delete function.\n");
        AK_EPI;
        return EXIT_ERROR;
    }
    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Boris Kišić
 * @brief Function that changes the function name.
 * @param *name name of the function to be modified
 * @param *arguments_list list of arguments to be modified
 * @param *new_name new name of the function
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_function_rename(char *name, struct list_node *arguments_list, char *new_name)
{
    AK_PRO;
    printf("***Function rename***\n");

    int func_id = AK_get_function_obj_id(name, arguments_list);

    struct list_node *row_root = (struct list_node *)AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row_root);

    AK_Update_Existing_Element(TYPE_INT, &func_id, "AK_function", "obj_id", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, new_name, "AK_function", "name", row_root);

    int result = AK_update_row(row_root);

    if (result == EXIT_ERROR || func_id == EXIT_ERROR)
    {
        AK_dbg_messg(HIGH, FUNCTIONS, "AK_function_rename: Could not rename function.\n");
        AK_EPI;
        return EXIT_ERROR;
    }

    AK_DeleteAll_L3(&row_root);
    AK_free(row_root);
    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Boris Kišić
 * @brief Function that changes the return type.
 * @param *name name of the function to be modified
 * @param *arguments_list list of function arguments
 * @param *new_return_type new return type
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
//int AK_function_change_return_type(char *name, AK_list *arguments_list, int new_return_type) {
int AK_function_change_return_type(char *name, struct list_node *arguments_list, int new_return_type)
{
    AK_PRO;
    printf("***Change function return type***\n");

    int func_id = AK_get_function_obj_id(name, arguments_list);

    printf("%d %d", func_id, new_return_type);

    struct list_node *row_root = (struct list_node *)AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row_root);

    AK_Update_Existing_Element(TYPE_INT, &func_id, "AK_function", "obj_id", row_root);
    AK_Insert_New_Element(TYPE_INT, &new_return_type, "AK_function", "return_type", row_root);

    int result = AK_update_row(row_root);

    if (result == EXIT_ERROR || func_id == EXIT_ERROR)
    {
        AK_dbg_messg(HIGH, FUNCTIONS, "AK_function_change_return_type: Could not change return type.\n");
        AK_EPI;
        return EXIT_ERROR;
    }

    AK_DeleteAll_L3(&row_root);
    AK_free(row_root);
    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Boris Kišić, updated by Tomislav Ilisevic
 * @brief Function for functions testing.
 * @return No return value
 */
TestResult AK_function_test()
{
    AK_PRO;

    printf("function.c: Present!\n");

    int success = 0, failed = 0;

    struct list_node *arguments_list1 = (struct list_node *)AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&arguments_list1);
    AK_InsertAtEnd_L3(TYPE_VARCHAR, "argument1", sizeof("argument1"), arguments_list1);
    AK_InsertAtEnd_L3(TYPE_INT, "5", sizeof(int), arguments_list1);
    AK_InsertAtEnd_L3(TYPE_VARCHAR, "argument2", sizeof("argument2"), arguments_list1);
    AK_InsertAtEnd_L3(TYPE_INT, "3", sizeof(int), arguments_list1);

    int fun_add1 = AK_function_add("test_funkcija", 1, arguments_list1);
    int fun_add2 = AK_function_add("test_funkcija2", 3, arguments_list1);
    int fun_add3 = AK_function_add("test_funkcija3", 4, arguments_list1);

    if (fun_add1 != EXIT_ERROR || fun_add2 != EXIT_ERROR || fun_add3 != EXIT_ERROR)
    {
        printf("\n\nSuccessfully added 'arguments_list1' to 'test_funkcija', 'test_funkcija2' and 'test_funkcija3'\n\n");
        success++;
    }
    else
    {
        failed++;
    }

    AK_print_table("AK_function");
    AK_print_table("AK_function_arguments");

    AK_DeleteAll_L3(&arguments_list1);
    AK_free(arguments_list1);

    struct list_node *arguments_list3 = (struct list_node *)AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&arguments_list3);

    AK_InsertAtEnd_L3(TYPE_VARCHAR, "argument1", sizeof("argument1"), arguments_list3);
    AK_InsertAtEnd_L3(TYPE_INT, "5", sizeof(int), arguments_list3);
    AK_InsertAtEnd_L3(TYPE_VARCHAR, "argument2", sizeof("argument2"), arguments_list3);
    AK_InsertAtEnd_L3(TYPE_INT, "3", sizeof(int), arguments_list3);
    AK_InsertAtEnd_L3(TYPE_VARCHAR, "argument3", sizeof("argument3"), arguments_list3);
    AK_InsertAtEnd_L3(TYPE_INT, "1", sizeof(int), arguments_list3);

    int fun_add4 = AK_function_add("test_funkcija", 1, arguments_list3);
    int fun_add5 = AK_function_add("test_funkcija2", 2, arguments_list3);
    int fun_add6 = AK_function_add("test_funkcija3", 1, arguments_list3);

    if (fun_add4 != EXIT_ERROR || fun_add5 != EXIT_ERROR || fun_add6 != EXIT_ERROR)
    {
        printf("\n\nSuccessfully added 'arguments_list3' to 'test_funkcija', 'test_funkcija2' and 'test_funkcija3'\n\n");
        success++;
    }
    else
    {
        failed++;
    }

    AK_print_table("AK_function");
    AK_print_table("AK_function_arguments");

    AK_DeleteAll_L3(&arguments_list3);
    AK_free(arguments_list3);

    struct list_node *arguments_list4 = (struct list_node *)AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&arguments_list4);

    AK_InsertAtEnd_L3(TYPE_VARCHAR, "argument1", sizeof("argument1"), arguments_list4);
    AK_InsertAtEnd_L3(TYPE_INT, "5", sizeof(int), arguments_list4);
    AK_InsertAtEnd_L3(TYPE_VARCHAR, "argument2", sizeof("argument2"), arguments_list4);
    AK_InsertAtEnd_L3(TYPE_INT, "3", sizeof(int), arguments_list4);
    AK_InsertAtEnd_L3(TYPE_VARCHAR, "argument3", sizeof("argument3"), arguments_list4);
    AK_InsertAtEnd_L3(TYPE_INT, "2", sizeof(int), arguments_list4);

    int fun_add7 = AK_function_add("test_funkcija", 2, arguments_list4);
    int fun_add8 = AK_function_add("test_funkcija2", 3, arguments_list4);
    int fun_add9 = AK_function_add("test_funkcija3", 1, arguments_list4);

    if (fun_add7 != EXIT_ERROR || fun_add8 != EXIT_ERROR || fun_add9 != EXIT_ERROR)
    {
        printf("\n\nSuccessfully added 'arguments_list4' to 'test_funkcija', 'test_funkcija2' and 'test_funkcija3'\n\n");
        success++;
    }
    else
    {
        failed++;
    }

    // Test bez argumenata
    struct list_node *arguments_list5 = (struct list_node *)AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&arguments_list5);

    AK_function_add("test_without_arg", 1, arguments_list5);

    int get = AK_get_function_obj_id("test_without_arg", arguments_list5);
    printf("ID test_without_arg: %i \n", get);

    AK_print_table("AK_function");
    AK_print_table("AK_function_arguments");

    AK_DeleteAll_L3(&arguments_list4);
    AK_free(arguments_list4);
    AK_DeleteAll_L3(&arguments_list5);

    if (failed == 0)
    {
        printf("\n\nAll tests has successfully completed!!\n\n");
    }

    AK_free(arguments_list5);
    AK_EPI;
    return TEST_result(success, failed);
}
