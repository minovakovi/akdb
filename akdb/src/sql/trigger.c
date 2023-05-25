/**
@file trigger.c Provides functions for triggers
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

#include "trigger.h"

/**
 * @author Unknown, updated by Mario Peroković, fixed by Josip Susnjara
 * @brief Function that saves conditions for a trigger.
 * @param trigger obj_id of the trigger in question
 * @param *condition AK_list list of conditions
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_trigger_save_conditions(int trigger, struct list_node *condition) {
    int condition_index = 0;
    char tempData[MAX_VARCHAR_LENGTH];
    AK_PRO;

    // Prepare the row root for the AK database
    struct list_node *current_condition = AK_First_L2(condition);
    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&row_root);
    // Delete any existing conditions for this trigger.
    AK_Update_Existing_Element(TYPE_INT, &trigger, "AK_trigger_conditions", "trigger", row_root);
    if (AK_delete_row(row_root) == EXIT_ERROR){
	AK_EPI;
        return EXIT_ERROR;
    }

    while (current_condition != NULL) {
        memcpy(tempData, current_condition->data, current_condition->size);
        tempData[current_condition->size] = '\0';
        AK_Insert_New_Element(TYPE_INT, &trigger, "AK_trigger_conditions", "trigger", row_root);
        AK_Insert_New_Element(TYPE_INT, &condition_index, "AK_trigger_conditions", "id", row_root);
        if(current_condition->type == TYPE_INT){
            AK_Insert_New_Element(TYPE_INT, &current_condition->data, "AK_trigger_conditions", "data", row_root);
        } else{
            AK_Insert_New_Element(TYPE_VARCHAR, tempData, "AK_trigger_conditions", "data", row_root);
        }
        AK_Insert_New_Element(TYPE_INT, &current_condition->type, "AK_trigger_conditions", "type", row_root);
        if (AK_insert_row(row_root) == EXIT_ERROR) {
            AK_free(row_root);
	    AK_EPI;
            return EXIT_ERROR;
        }

	current_condition = AK_Next_L2(current_condition);
        condition_index++;
        AK_DeleteAll_L3(&row_root);
    }

    AK_free(row_root);
    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Unknown updated by Aleksandra Polak, fixed by Josip Susnjara
 * @brief Function that adds a trigger to the system table.
 * @param *name name of the trigger
 * @param *event event that calls the trigger - this should perhaps be an integer with defined constants...
 * @param *condition AK_list list of conditions in postfix
 * @param *table name of the table trigger is hooked on
 * @param *function function that is being called by the trigger
 * @return trigger id or EXIT_ERROR
 */
int AK_trigger_add(char *name, char* event, struct list_node *condition, char* table, char* function, struct list_node *arguments_list) {
    int funk_id = -1, table_id = -1, trigg_id;
    AK_PRO;
    table_id = AK_get_table_obj_id(table);

    if(arguments_list == NULL) {
	printf ("\nAK_trigger_add: Argument list is empty. Can not add this trigger: %s \n\n", name);
	AK_EPI;
	return EXIT_ERROR;
    }

    if (table_id == EXIT_ERROR) {
        printf ("\nAK_trigger_add: No such table upon which to create a trigger. Table name: %s \n\n", table);
	AK_EPI;
        return EXIT_ERROR;
    }

    funk_id = AK_get_function_obj_id(function, arguments_list);

    if (funk_id == EXIT_ERROR) {
        printf ("\nAK_trigger_add: No such function to execute upon activation of trigger. Function name: %s \n\n", function);
	AK_EPI;
        return EXIT_ERROR;
    }

    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));

    if (row_root == NULL){
	printf ("\nRow root is NULL.\n");
	AK_EPI;
       	return EXIT_ERROR;
    }


    AK_Init_L3(&row_root);

    trigg_id = AK_get_id();

    AK_Insert_New_Element(TYPE_INT, &trigg_id, "AK_trigger", "obj_id", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, name, "AK_trigger", "name", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, event, "AK_trigger", "event", row_root);

    if (condition == NULL || AK_IsEmpty_L2(condition) == 1) 
        AK_Insert_New_Element(TYPE_VARCHAR, "null", "AK_trigger", "condition", row_root);
    else
        AK_Insert_New_Element(TYPE_VARCHAR, "T", "AK_trigger", "condition", row_root);

    AK_Insert_New_Element(TYPE_INT, &funk_id, "AK_trigger", "action", row_root);
    AK_Insert_New_Element(TYPE_INT, &table_id, "AK_trigger", "on", row_root);
    AK_insert_row(row_root);

    AK_free(row_root);

    if (condition != NULL && AK_IsEmpty_L2(condition) == 0)
        AK_trigger_save_conditions(trigg_id, condition);
    AK_EPI;
    return trigg_id;
}

/**
 * @author Unknown, fixed by Josip Susnjara
 * @brief Function that gets obj_id of a trigger defined by name and table.
 * @param *name name of the trigger
 * @param *table name of the table on which the trigger is hooked
 * @return obj_id of the trigger or EXIT_ERROR
 */
int AK_trigger_get_id(char *name, char *table) {
    int trigger_index = 0, table_id = -1;
    
    struct list_node *row;
    AK_PRO;
    table_id = AK_get_table_obj_id(table);
    if (table_id == EXIT_ERROR){
	AK_EPI;
        return EXIT_ERROR;
    }

    while ((row = (struct list_node *)AK_get_row(trigger_index, "AK_trigger")) != NULL) {
        struct list_node *name_elem = AK_GetNth_L2(2,row);
        struct list_node *table_elem = AK_GetNth_L2(6,row);
        if (strcmp(name_elem->data, name) == 0 && table_id == (int) * table_elem->data) {
            trigger_index = (int) * row->next->data;
            AK_free(row);
	    AK_EPI;
            return trigger_index;
        }
        trigger_index++;
    }

    AK_free(row);
    AK_EPI;
    return EXIT_ERROR;
}

/**
 * @author Unknown
 * @brief Function that removes a trigger from the system table by name
 * @param *name name of the trigger
 * @param *table name of the table
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_trigger_remove_by_name(char *name, char *table) {
	int trigg_id;
    	AK_PRO;
    	trigg_id = AK_trigger_get_id(name, table);
	AK_EPI;
	return AK_trigger_remove_by_obj_id(trigg_id);
}

/**
 * @author Unknown
 * @brief Function that removes a trigger by its obj_id
 * @param obj_id obj_id of the trigger
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_trigger_remove_by_obj_id(int obj_id) {
    AK_PRO;

    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&row_root);

    AK_Update_Existing_Element(TYPE_INT, &obj_id, "AK_trigger", "obj_id", row_root);

    int result = AK_delete_row(row_root);

    if (result == EXIT_ERROR) {
        AK_dbg_messg(HIGH, TRIGGERS, "AK_trigger_remove_by_name: Could not delete trigger.\n");
	AK_EPI;
        return EXIT_ERROR;
    }

    // the following can be avoided if foreign key is declared...
    
    AK_DeleteAll_L3(&row_root);
    AK_Update_Existing_Element(TYPE_INT, &obj_id, "AK_trigger_conditions", "trigger", row_root);
    AK_EPI;
    return AK_delete_row(row_root);
}


/**
 * @author Unknown, fixed by Josip Susnjara
 * @brief Function that edits information about the trigger in system table. In order to identify the trigger,
 *	  either obj_id or table and name parameters should be defined. The other options should be set to NULL.
 *   	  Values of parameters that aren't changing can be left NULL. If conditions are to be removed,
 * 	  condition parameter should hold an empty list.
 * @param *name name of the trigger (or NULL if using obj_id)
 * @param *event event of the trigger (or NULL if it isn't changing)
 * @param *condition list of conditions for trigger (or NULL if it isn't changing; empty list if all conditions are to be removed)
 * @param *table name of the connected table (or NULL id using obj_id)
 * @param *function name of the connected function (or NULL if it isn't changing)
 * @param *arguments_list arguments of the function (without arguments can't find passed function)
 * @return EXIT_SUCCESS or EXIT_ERROR
 */

int AK_trigger_edit(char *name, char* event, struct list_node *condition, char* table, char* function, struct list_node *arguments_list) {  
    AK_PRO;

    if (name == NULL || table == NULL) {
        AK_dbg_messg(HIGH, TRIGGERS, "AK_trigger_edit: Not enough data to identify the trigger.\n");
	AK_EPI;
        return EXIT_ERROR;
    }
    int table_id = AK_get_table_obj_id(table); 
    int trigger_id = AK_trigger_get_id(name, table);
    int function_id = AK_get_function_obj_id(function, arguments_list);
 
   if (function_id == EXIT_ERROR) {
        AK_dbg_messg(HIGH, TRIGGERS, "AK_trigger_edit: Could not update trigger. Function does not exist.\n");
	AK_EPI;
        return EXIT_ERROR;
    }

    if(table_id == EXIT_ERROR){
	AK_dbg_messg(HIGH, TRIGGERS, "\nAK_trigger_edit: Could not update trigger. Table %s does not exist.\n", table);
	AK_EPI;
	return EXIT_ERROR;
    }
    
    if(trigger_id == EXIT_ERROR){
	AK_dbg_messg(HIGH, TRIGGERS, "\nAK_trigger_edit: Could not update trigger. Trigger %s does not exist.\n", name);
	AK_EPI;
	return EXIT_ERROR;
    }

    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&row_root);

    AK_Update_Existing_Element(TYPE_INT, &trigger_id, "AK_trigger", "obj_id", row_root);

    if (event != NULL)
        AK_Insert_New_Element(TYPE_VARCHAR, event, "AK_trigger", "event", row_root);
    if (condition != NULL){
        AK_Insert_New_Element(TYPE_VARCHAR, "T", "AK_trigger", "condition", row_root);
        AK_trigger_save_conditions(trigger_id, condition);
    }
    else
        AK_Insert_New_Element(0, "", "AK_trigger", "condition", row_root);
    if (function != NULL)
        AK_Insert_New_Element(TYPE_INT, &function_id, "AK_trigger", "action", row_root);

    int result = AK_update_row(row_root);
    AK_free(row_root);

    if (result == EXIT_ERROR) {
        AK_dbg_messg(HIGH, TRIGGERS, "AK_trigger_edit: Could not update trigger.\n");
	AK_EPI;
        return EXIT_ERROR;
    }
    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Unknown, updated by Mario Peroković
 * @brief Function that fetches postfix list of conditions for the trigger (compatible with selection)
 * @param trigger obj_id of the trigger
 * @return list of conditions for the trigger
 */
//AK_list *AK_trigger_get_conditions(int trigger) {
struct list_node *AK_trigger_get_conditions(int trigger) {
    char *endPtr;
    AK_PRO;

    struct list_node *expr = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&expr);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "trigger", strlen("trigger"), expr);
    AK_InsertAtEnd_L3(TYPE_INT, (char*)(&trigger), sizeof (int), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "=", 1, expr);
    
    AK_selection("AK_trigger_conditions", "AK_trigger_conditions_temp", expr);
    
    struct list_node *result = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&result);
    
    int i = 0;

    struct list_node *row;

    while((row = (struct list_node *)AK_get_row(i, "AK_trigger_conditions_temp")) != NULL){
        struct list_node *first_arg_elem = AK_GetNth_L2(4,row);
        struct list_node *second_arg_elem = AK_GetNth_L2(3,row);
        AK_InsertAtEnd_L3(strtol(first_arg_elem->data, &endPtr, 10), second_arg_elem->data, second_arg_elem->size, result);
        i++;
    }

    AK_delete_segment("AK_trigger_conditions_temp", SEGMENT_TYPE_TABLE);
    AK_free(row);
    AK_EPI;
    return result;
}

/**
* @author Ljubo Barać
* @brief Function that renames the trigger
* @param old_name Name of the trigger to be renamed
* @param new_name New name of the trigger
* @return EXIT_SUCCESS or EXIT_ERROR
*/
int AK_trigger_rename(char *old_name, char *new_name, char *table){
	AK_PRO;
	printf("\n***Rename trigger***\n");

	int trig_id = AK_trigger_get_id(old_name, table);

	struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
	AK_Init_L3(&row_root);


	AK_Update_Existing_Element(TYPE_INT, &trig_id, "AK_trigger", "obj_id", row_root);
	AK_Insert_New_Element(TYPE_VARCHAR, new_name, "AK_trigger", "name", row_root);

	int result =  AK_update_row(row_root);
	
	AK_DeleteAll_L3(&row_root);
	AK_free(row_root);

	if (result == EXIT_ERROR) {
	   AK_dbg_messg(HIGH, SEQUENCES, "AK_trigger_rename: Could not rename trigger.\n");
	   AK_EPI;
	   return EXIT_ERROR;
	   }
	AK_EPI;
	return EXIT_SUCCESS;
}



/**
 * @author Unknown updated by Aleksandra Polak and Josip Susnjara
 * @brief Function for trigger testing
 */
TestResult AK_trigger_test() {
	
	int successfulTests = 0;
	int failedTests = 0;
    AK_PRO;

    printf("trigger.c: Present!\n");

    struct list_node *arguments_list1 = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&arguments_list1);
    struct list_node *arguments_list2 = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&arguments_list2);

  
    
    AK_InsertAtEnd_L3(TYPE_VARCHAR, "argument123", sizeof ("argument123"), arguments_list1);
    AK_InsertAtEnd_L3(TYPE_INT, "5", sizeof (int), arguments_list1);
    AK_InsertAtEnd_L3(TYPE_VARCHAR, "argument2", sizeof ("argument2"), arguments_list1);
    AK_InsertAtEnd_L3(TYPE_INT, "3", sizeof (int), arguments_list1);
  
    if (AK_function_add("dummy_funk_1", 1, arguments_list1) != EXIT_ERROR) {
		printf("\n Successful added arguments_list1 to dummy_funk_1.\n\n");
		successfulTests++;
    }
    else {
    	failedTests++;
    }


    AK_InsertAtEnd_L3(TYPE_VARCHAR, "argument7", sizeof ("argument7"), arguments_list2);
    AK_InsertAtEnd_L3(TYPE_INT, "5", sizeof (int), arguments_list2);

    if (AK_function_add("dummy_funk_2", 1, arguments_list2) != EXIT_ERROR) {
		printf("\n Successful added arguments_list2 to dummy_funk_2.\n\n");
		successfulTests++;
    }
    else {
    	failedTests++;
    }
    
    AK_print_table("AK_function");

    struct list_node *expr = (struct list_node *) AK_malloc(sizeof (struct list_node));
    struct list_node *dummyExpression = (struct list_node *) AK_malloc(sizeof(struct list_node));
    strcpy(dummyExpression->data, "");
    
    AK_Init_L3(&expr);
   
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), expr);
    AK_InsertAtEnd_L3(TYPE_INT, "2002", sizeof(int), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, ">", sizeof(">"), expr);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), expr);
    AK_InsertAtEnd_L3(TYPE_VARCHAR, "Matija", sizeof("Matija"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "=", sizeof("="), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "OR", sizeof("OR"), expr);
   

    if(AK_trigger_add("trigg1", "insert", expr, "AK_reference", "dummy_funk_1", arguments_list1) != EXIT_ERROR){
    	successfulTests++;
    }
    else{
    	failedTests++;
    }
    
    if(AK_trigger_add("trigg2", "update", expr, "dummy_table", "dummy_funk_1", arguments_list1) == EXIT_ERROR){
    	//must fail because table "dummy_table" doesn't exist
    	successfulTests++;
    }
    else{
    	failedTests++;
    }
    
    if(AK_trigger_add("trigg3", "delete", dummyExpression, "AK_reference", "dummy_funk", NULL) == EXIT_ERROR){
    	//must fail because arguments list is empty
    	successfulTests++;
    }
    else{
    	failedTests++;
    }
    
    if(AK_trigger_add("trigg4", "insert", NULL, "AK_reference", "dummy_funk_2", arguments_list2) != EXIT_ERROR){
    	successfulTests++;
    }
    else{
    	failedTests++;
    }

    AK_print_table("AK_trigger");
    AK_print_table("AK_trigger_conditions");

    printf("\n\nAK_trigger_get_conditions test!\n\n");
    AK_trigger_get_conditions(121);

    AK_DeleteAll_L3(&expr);

    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "not null", sizeof("not null"), expr);

    printf("\n\nUpdate trigger:\n\n");


    //look for 'trigg1' on table 'AK_reference' and change it's event to drop and it's function to 'dummy_funk_1'
    if (AK_trigger_edit("trigg1", "drop", expr, "AK_reference", "dummy_funk_2", arguments_list2) != EXIT_ERROR){
		printf("\n\nSuccessfully edited trigger.\n\n");
		successfulTests++;
    }
    else {
    	failedTests++;
    }

    AK_print_table("AK_trigger");

    printf("\n\nDelete trigg1 by name.\n\n");

    if(AK_trigger_remove_by_name("trigg1", "AK_reference") != EXIT_ERROR){
    	printf("\n\nSuccessfully deleted trigger.\n\n");
		successfulTests++;
    }
    else{
    	failedTests++;
    }

    AK_print_table("AK_trigger");

    AK_DeleteAll_L3(&arguments_list1);
    AK_DeleteAll_L3(&arguments_list2);
    AK_free(arguments_list1);
    AK_free(arguments_list2);
    AK_free(dummyExpression);

    AK_free(expr);
    AK_EPI;

    return TEST_result(successfulTests, failedTests);
}

