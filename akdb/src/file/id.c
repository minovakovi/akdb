/**
   @file id.c Provides functions for creating id of objects
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

#include "id.h"

/**
 * @author Saša Vukšić, updated by Mislav Čakarić, changed by Mario Peroković, now uses AK_update_row, updated by Nenad Makar
 * @brief Function that fetches unique ID for any object, stored in a sequence
 * @return objectID
 */
int AK_get_id() {
    int obj_id = 0;
    int current_value;
    AK_PRO;
    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&row_root); 
	
	/*Assumption was that objectID is always in the first row of table AK_sequence. If in future, for some reason, that won't be the case
	 * then check all rows of table AK_sequence (for(i=0; i<num_rec; i++)) and update a row which contains objectID (AK_GetNth_L2(2, row), value in column 
	 * name must be objectID) or create a row which will contain objectID*/
	
	int num_rec = AK_get_num_records("AK_sequence");
	
    if(num_rec > 0) {
    	struct list_node *row = AK_get_row(0, "AK_sequence");
		struct list_node *attribute = AK_GetNth_L2(3, row);
		memcpy(&current_value, &attribute->data, attribute->size);
		AK_DeleteAll_L3(&row);
		AK_free(row);
		
        current_value++;
		AK_Update_Existing_Element(TYPE_INT, &obj_id, "AK_sequence", "obj_id", row_root);
        AK_Insert_New_Element(TYPE_INT, &current_value, "AK_sequence", "current_value", row_root);
        int result = AK_update_row(row_root);
		AK_DeleteAll_L3(&row_root);
        AK_free(row_root);
		
        if(result != EXIT_SUCCESS){
            AK_EPI;
            return EXIT_ERROR;
        }
        AK_EPI;
        return current_value;
    }
    else
    {
		AK_Insert_New_Element(TYPE_INT, &obj_id, "AK_sequence", "obj_id", row_root);
		AK_Insert_New_Element(TYPE_VARCHAR, "objectID", "AK_sequence", "name", row_root);
		current_value = ID_START_VALUE;
		AK_Insert_New_Element(TYPE_INT, &current_value, "AK_sequence", "current_value", row_root);
		int increment = 1;
		AK_Insert_New_Element(TYPE_INT, &increment, "AK_sequence", "increment", row_root);
		AK_insert_row(row_root);
		AK_DeleteAll_L3(&row_root);
		AK_free(row_root);
		AK_EPI;
		return current_value;
    }
        AK_EPI;
        //return 100;
}

/**
 * @author Lovro Predovan
 * @brief Function that fetches unique ID for any object, stored in sequence based on table name
 * @return objectID in string(char) format
 */
char AK_get_table_id(char *tableName) {
    AK_PRO;
    char *table = "AK_relation";
    int result = 0;

    int num_rows = AK_get_num_records(table);
    int a;

    if(num_rows==0){
        return result;
    }

    for (a = 0; a < num_rows; a++) {
        struct list_node *el;
        el = AK_get_tuple(a, 1,table);
        if (strcmp(tableName, el->data)==0) {
            result = AK_tuple_to_string(AK_get_tuple(a, 0,table));
            break;
        }
    }
    AK_EPI;
    return result;
}


/**
 * @author Mislav Čakarić, updated by Nenad Makar
 * @brief  Function for testing getting ID's
 * @return No retun value
 */
TestResult AK_id_test() {
    AK_PRO;
    int result;
    int failed=0;
    int success=0;
    printf("\nCurrent value of objectID (depends on number of AK_get_id() calls (when objects are created...) before call of AK_id_test()):\n\n");
    AK_print_table("AK_sequence");
    result=AK_get_id();
    if(result==EXIT_ERROR)
    {
        failed++;
    }
    else
    {
        success++;
    }
    
    printf("\nIncremented value of objectID:\n\n");
    AK_print_table("AK_sequence");
    result=AK_get_id();
    if(result==EXIT_ERROR)
    {
        failed++;
    }
    else
    {
        success++;
    }
    printf("\nIncremented value of objectID:\n\n");
    AK_print_table("AK_sequence");
    printf("\nTest succeeded.\nIt's clear that objectID was created after first call of AK_get_id() function (when ./akdb test created first DB object) then incremented after other calls.\n");
    AK_EPI;
    return TEST_result(success,failed);
}
