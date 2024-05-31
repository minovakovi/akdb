/**
@file sequence.c Provides functions for sequences
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

#include "sequence.h"

/**
 * @author Boris Kišić
 * @brief Function for adding sequence.
 * @param name name of the sequence
 * @param start_value start value of the sequence
 * @param increment increment of the sequence
 * @param max_value maximium value of the sequence
 * @param min_value minimum value of the sequence
 * @param cycle 0:non-cyclic sequence, 1:cyclic sequence
 * @return sequence_id or EXIT_ERROR
 */
int AK_sequence_add(char *name, int start_value, int increment, int max_value, int min_value, int cycle){
    AK_PRO;
    printf("\n***Add sequence***");

    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row_root);
    
    int sequence_id = AK_get_id(); //get sequence_id
    AK_Insert_New_Element(TYPE_INT, &sequence_id, "AK_sequence", "obj_id", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, name, "AK_sequence", "name", row_root);
    AK_Insert_New_Element(TYPE_INT, &start_value, "AK_sequence", "current_value", row_root);
    AK_Insert_New_Element(TYPE_INT, &increment, "AK_sequence", "increment", row_root);
    AK_Insert_New_Element(TYPE_INT, &max_value, "AK_sequence", "max", row_root);
    AK_Insert_New_Element(TYPE_INT, &min_value, "AK_sequence", "min", row_root);
    AK_Insert_New_Element(TYPE_INT, &cycle, "AK_sequence", "cycle", row_root);
    
    int result = AK_insert_row(row_root); 
    
    if (result == EXIT_ERROR || sequence_id == EXIT_ERROR) {
      AK_dbg_messg(HIGH, SEQUENCES, "AK_sequence_add: Could not add sequence.\n");
      AK_EPI;
      return EXIT_ERROR;
    }
    
    AK_DeleteAll_L3(&row_root);
    
    AK_free(row_root);
    AK_EPI;
    return sequence_id;
}

/**
 * @author Boris Kišić
 * @brief Function for removing sequence.
 * @param name name of the sequence
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_sequence_remove(char *name){
    AK_PRO;
    printf("\n***Remove sequence***");

    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row_root);
    
    AK_Update_Existing_Element(TYPE_VARCHAR, name, "AK_sequence", "name", row_root);
    int result =  AK_delete_row(row_root); 
   
    if (result == EXIT_ERROR) {
      AK_dbg_messg(HIGH, SEQUENCES, "AK_sequence_delete: Could not delete sequence.\n");
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
 * @brief Function that returns the current value of the sequence.
 * @param name name of the sequence
 * @return current_value or EXIT_ERROR
 */
int AK_sequence_current_value(char *name){
    int i = 0;
    int current_value = -1;
    int num_rows = AK_get_num_records("AK_sequence");
    
    struct list_node *row;
    AK_PRO;

    while ((row = (struct list_node *)AK_get_row(i, "AK_sequence")) != NULL){
        if (strcmp(get_row_attr_data(1,row), name) == 0) {
            memcpy(&current_value, get_row_attr_data(2,row), sizeof (int));
	    break;
        }
        i++;
    }
    
    if (current_value == -1){
	AK_EPI;
        return EXIT_ERROR;
    }
    AK_EPI;
    return current_value;
}

/**
 * @author Boris Kišić
 * @brief Function that returns the next value of the sequence and writes it in a system table as current value.
 * @param name name of the sequence
 * @return next_value or EXIT_ERROR
 */
int AK_sequence_next_value(char *name){
    int next_value ;
    int i = 0;
    int obj_id;
    int current_value = -1;
    int increment;
    int max_value;
    int min_value;
    int cycle;
    
    struct list_node *row;
    AK_PRO;

    while ((row = (struct list_node *)AK_get_row(i, "AK_sequence")) != NULL){
        if(strcmp( get_row_attr_data(1,row) ,name) == 0) {
        memcpy(&obj_id, get_row_attr_data(0,row), sizeof (int));
        memcpy(&current_value, get_row_attr_data(2,row), sizeof (int));
        memcpy(&increment, get_row_attr_data(3,row), sizeof (int));
        memcpy(&max_value, get_row_attr_data(4,row), sizeof (int));
        memcpy(&min_value, get_row_attr_data(5,row), sizeof (int));
        memcpy(&cycle, get_row_attr_data(6,row), sizeof (int));
        break;
        }
        i++;
    }
   

    if (current_value == -1){
    AK_EPI;
        return EXIT_ERROR;
    }
 
    next_value = current_value + increment;
    
    if(next_value < min_value){
    if(cycle==0){
        printf("\nNon-cyclic sequence can not go below its minimum value.");
            AK_EPI;
        return EXIT_ERROR;
    }
    else{
        next_value=max_value;
    } 
    }
    
    if(next_value > max_value){
    if(cycle==0){
        printf("\nNon-cyclic sequence can not go over its maximum value.");
        AK_EPI;
        return EXIT_ERROR;
    }
    else{
        next_value=min_value;
    } 
    }
    
 
    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row_root);
    
    AK_Update_Existing_Element(TYPE_INT, &obj_id, "AK_sequence", "obj_id", row_root);
    AK_Insert_New_Element(TYPE_INT, &next_value, "AK_sequence", "current_value", row_root);
    int result =  AK_update_row(row_root); 
    AK_DeleteAll_L3(&row_root);
    AK_free(row_root);
   
   if (result == EXIT_ERROR) {
      AK_dbg_messg(HIGH, SEQUENCES, "AK_sequence_next_value: Could not retrieve next value.\n");
      AK_EPI;
      return EXIT_ERROR;
    }
   
    AK_EPI;
    return next_value;
}

/**
 * @author Ljubo Barać
 * @brief Function that fetches sequence id
 * @param name Name of the sequence
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_sequence_get_id(char *name){
    int i = 0;
    
    struct list_node * row;
    AK_PRO;
	
	while ((row = (struct list_node *)AK_get_row(i, "AK_sequence")) != NULL) {
		if (strcmp( get_row_attr_data(1,row) ,name) == 0) {
			i = (int) * get_row_attr_data(0,row);
			AK_free(row);
			AK_EPI;
			return i;
		}
		i++;
	}
	AK_free(row);
	AK_EPI;
	return EXIT_ERROR;
}


/**
 * @author Boris Kišić
 * @brief Function that renames the sequence.
 * @param old_name Name of the sequence to be renamed
 * @param new_name New name of the sequence
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_sequence_rename(char *old_name, char *new_name){
    AK_PRO;
    printf("\n***Rename sequence***");
    int i = 0;
    
    int seq_id = AK_sequence_get_id(old_name);

    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&row_root);
    
    AK_Update_Existing_Element(TYPE_INT, &seq_id, "AK_sequence", "obj_id", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, new_name, "AK_sequence", "name", row_root);
    int result =  AK_update_row(row_root); 
    
    AK_DeleteAll_L3(&row_root);
    AK_free(row_root);
    
    if (result == EXIT_ERROR || seq_id == -1) {
      AK_dbg_messg(HIGH, SEQUENCES, "AK_sequence_rename: Could not rename sequence.\n");
      AK_EPI;
      return EXIT_ERROR;
    }
    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Boris Kišić fixed by Ljubo Barać
 * @brief Function for modifying a sequence
 * @param name Name of the sequence
 * @param start_value start value of the sequence
 * @param increment increment of the sequence
 * @param max_value maximium value of the sequence
 * @param min_value minimum value of the sequence
 * @param cycle 0:non-cyclic sequence, 1:cyclic sequence
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_sequence_modify(char *name, int start_value, int increment, int max_value, int min_value, int cycle){
    AK_PRO;
    printf("\n***Edit sequence***");

    int seq_id = AK_sequence_get_id(name);
    
    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row_root);
    
    AK_Update_Existing_Element(TYPE_INT, &seq_id, "AK_sequence", "obj_id", row_root);
    AK_Insert_New_Element(TYPE_INT, &start_value, "AK_sequence", "current_value", row_root);
    AK_Insert_New_Element(TYPE_INT, &increment, "AK_sequence", "increment", row_root);
    AK_Insert_New_Element(TYPE_INT, &max_value, "AK_sequence", "max", row_root);
    AK_Insert_New_Element(TYPE_INT, &min_value, "AK_sequence", "min", row_root);
    AK_Insert_New_Element(TYPE_INT, &cycle, "AK_sequence", "cycle", row_root);
    int result =  AK_update_row(row_root); 
    
    AK_DeleteAll_L3(&row_root);
    AK_free(row_root);
    
    if (result == EXIT_ERROR || seq_id == -1) {
      AK_dbg_messg(HIGH, SEQUENCES, "AK_sequence_modify: Could not modify sequence.\n");
      AK_EPI;
      return EXIT_ERROR;
    }
    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Boris Kišić fixed by Ljubo Barać
 * @brief Function used for sequences testing.
 * @return No return value
 */
TestResult AK_sequence_test() {
    AK_PRO;
    printf("sequence.c: Present!\n");
    printf("\n***Adding sequences***\n");
    int add1 = AK_sequence_add("sequence1", 100, 5, 200, 100, 1);     
    int add2 = AK_sequence_add("sequence2", 200, 10, 205, 100, 1);
    AK_print_table("AK_sequence");

    printf("\n***Getting sequence id***\n");
    int id = AK_sequence_get_id("sequence1");
    printf("\nValue of 'sequence1' id: %d\n", id);

    printf("\n***Current & next value check***\n");
    int currval = AK_sequence_current_value("sequence1");
    printf("\nCurrent value of sequence sequence1: %d", currval);
    int nextval = AK_sequence_next_value("sequence1");
    printf("\nNext value of sequence sequence1: %d\n", nextval);
    
    int rename = AK_sequence_rename("sequence1", "sequence3");
    printf("\nRenaming 'sequence 1' to 'sequence2'.\n");
    AK_print_table("AK_sequence"); 

    int modify = AK_sequence_modify("sequence3", 200, 20, 300, 150, 0);
    printf("\nModifying 'sequence3.'\n");
    AK_print_table("AK_sequence"); 

    int remove = AK_sequence_remove("sequence2");
    printf("\nRemoving 'sequence2'.\n");
    AK_print_table("AK_sequence");

    int failed = 0;
    if (add1 == EXIT_ERROR) {
    	printf("\nError while adding 'sequence1'.\n");
        failed++;
    }
    if (add2 == EXIT_ERROR) {
    	printf("\nError while adding 'sequence2'.\n");
	    failed++;
    }
    if (id == EXIT_ERROR) {
    	printf("\nError while getting id value 'sequence1'.\n");
        failed++;
    }
    if (currval == EXIT_ERROR || currval != 100) {
    	printf("\nError while getting current value of 'sequence1'.\n");
        failed++;
    }
    if (nextval == EXIT_ERROR || nextval != 105) {
    	printf("\nError while getting next value 'sequence1'.\n");
        failed++;
    }
    if (rename == EXIT_ERROR) {
    	printf("\nError while renaming 'sequence1'.\n");
        failed++;
    }
    if (modify == EXIT_ERROR) {
    	printf("\nError while modifying 'sequence3'.\n");
        failed++;
    }   
    if (remove == EXIT_ERROR) {
    	printf("\nError removing 'sequence2'.\n");
        failed++;
    } 
    AK_EPI;
    return TEST_result(8-failed,failed);
}
