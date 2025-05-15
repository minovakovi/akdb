/**
@file nnull.c Provides functions for not null constraint
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

#include "nnull.h"
#include <stdbool.h>

/**
 * @author Saša Vukšić, updated by Nenad Makar
 * @brief Function that sets NOT NULL constraint on an attribute
 * @param char* tableName name of table
 * @param char* attName name of attribute
 * @param char* constraintName name of constraint
 * @return EXIT_ERROR or EXIT_SUCCESS
 **/
int AK_set_constraint_not_null(char* tableName, char* attName, char* constraintName) {
	int i;
	int numRows;
	int newConstraint;
	int uniqueConstraintName;
	int check;
	struct list_node *row;
	struct list_node *attribute;
	char *tupple_to_string_return;

	AK_PRO;

	// Validate input parameters
	if (tableName == NULL || attName == NULL || constraintName == NULL) {
		printf("ERROR: NULL parameter passed to AK_set_constraint_not_null\n");
		AK_EPI;
		return EXIT_ERROR;
	}

	newConstraint = AK_read_constraint_not_null(tableName, attName, NULL);
	if(newConstraint == EXIT_ERROR) {
		printf("ERROR: NOT NULL constraint already exists on attribute '%s' of table '%s'\n", attName, tableName);
		AK_EPI;
		return EXIT_ERROR;
	}
   
	check =  AK_check_constraint_not_null(tableName, attName, constraintName);
	if(check == EXIT_ERROR) {
		printf("ERROR: NOT NULL constraint cannot be set\n");
		AK_EPI;
		return EXIT_ERROR;
	}	
	

	struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
	AK_Init_L3(&row_root);
	int obj_id = AK_get_id();

	AK_Insert_New_Element(TYPE_INT, &obj_id, "AK_constraints_not_null", "obj_id", row_root);
	AK_Insert_New_Element(TYPE_VARCHAR, tableName, "AK_constraints_not_null", "tableName", row_root);
	AK_Insert_New_Element(TYPE_VARCHAR, constraintName, "AK_constraints_not_null", "constraintName", row_root);
	AK_Insert_New_Element(TYPE_VARCHAR, attName, "AK_constraints_not_null", "attributeName", row_root);
	AK_insert_row(row_root);
	AK_DeleteAll_L3(&row_root);
	AK_free(row_root);
	printf("SUCCESS: NOT NULL constraint successfully set on attribute '%s' of table '%s'\n", attName, tableName);
	
	AK_EPI;
	
	return EXIT_SUCCESS;
}


/**
 * @author Saša Vukšić, updated by Nenad Makar
 * @brief Function that checks if constraint name is unique and in violation of NOT NULL constraint
 * @param char* tableName name of table
 * @param char* attName name of attribute
 * @param char* constraintName name of constraint
 * @return EXIT_ERROR or EXIT_SUCCESS
 **/
int AK_check_constraint_not_null(char* tableName, char* attName, char* constraintName) {
	int i;
	int numRows;
	int newConstraint;
	int uniqueConstraintName;
	struct list_node *row;
	struct list_node *attribute;
	
	char *tupple_to_string_return;

	AK_PRO;

	// Validate input parameters
	if (tableName == NULL || attName == NULL || constraintName == NULL) {
		printf("ERROR: NULL parameter passed to AK_check_constraint_not_null\n");
		AK_EPI;
		return EXIT_ERROR;
	}

	numRows = AK_get_num_records(tableName);
	
	if(numRows > 0) {
		int positionOfAtt = AK_get_attr_index(tableName, attName) + 1;
		
		for(i=0; i<numRows; i++) {
			row = AK_get_row(i, tableName);
			attribute = AK_GetNth_L2(positionOfAtt, row);
			
			if((tupple_to_string_return=AK_tuple_to_string(attribute)) == NULL) {
				printf("ERROR: Table '%s' contains NULL value that would violate NOT NULL constraint on attribute '%s'\n", 
                       tableName, attName);
				AK_DeleteAll_L3(&row);
				AK_free(row);
				AK_EPI;
				return EXIT_ERROR;
			}
			else
				AK_free(tupple_to_string_return);
			AK_DeleteAll_L3(&row);
			AK_free(row);
		}
	}

	uniqueConstraintName = AK_check_constraint_name(constraintName, AK_CONSTRAINTS_NOT_NULL);
	if(uniqueConstraintName == EXIT_ERROR) {
		printf("ERROR: Constraint name '%s' is not unique - already exists in database\n", constraintName);
		AK_EPI;
		return EXIT_ERROR;
	}
	
	AK_EPI;

	return EXIT_SUCCESS;
}

/**
 * @author Saša Vukšić, updated by Nenad Makar
 * @brief Function checks if NOT NULL constraint is already set 
 * @param char* tableName name of table
 * @param char* attName name of attribute
 * @param char* newValue new value
 * @return EXIT_ERROR or EXIT_SUCCESS
 **/

int AK_read_constraint_not_null(char* tableName, char* attName, char* newValue) {
	int numRecords = AK_get_num_records("AK_constraints_not_null");
	int result = EXIT_SUCCESS;
	struct list_node *row = NULL;
	struct list_node *attribute = NULL;
	struct list_node *table = NULL;
	int i;
	
	AK_PRO;

	// Validate required parameters (newValue is allowed to be NULL)
	if (tableName == NULL || attName == NULL) {
		printf("ERROR: NULL parameter passed to AK_read_constraint_not_null\n");
		AK_EPI;
		return EXIT_ERROR;
	}

	if(newValue == NULL) {
		if(numRecords != 0) {
			for (i = 0; i < numRecords; i++) 
			{
				row = AK_get_row(i, "AK_constraints_not_null");
				if (row == NULL) {
					continue;
				}
				
				attribute = AK_GetNth_L2(4, row);
				if(attribute != NULL && strcmp(attribute->data, attName) == 0) 
				{
					table = AK_GetNth_L2(2, row);
					if(table != NULL && strcmp(table->data, tableName) == 0) 
					{
						result = EXIT_ERROR;
						AK_DeleteAll_L3(&row);
						AK_free(row);
						break;
					}
				}
				AK_DeleteAll_L3(&row);
				AK_free(row);
				row = NULL;
			}			
		}
	}

	AK_EPI;
	return result;		
}


/**
 * @author Bruno Pilošta
 * @brief Function for deleting not null constraints
 * @param tableName System table where constraint will be deleted from
 * @param constraintName Name of constraint that will be deleted 
 * @return EXIT_SUCCESS if the constraint is deleted, EXIT_ERROR otherwise
 * **/
int AK_delete_constraint_not_null(char* tableName, char* constraintName){
    AK_PRO;

    // Validate input parameters
	if (tableName == NULL || constraintName == NULL) {
		printf("ERROR: NULL parameter passed to AK_delete_constraint_not_null\n");
		AK_EPI;
		return EXIT_ERROR;
	}

    char* constraint_attr = "constraintName";

    if(AK_check_constraint_name(constraintName, AK_CONSTRAINTS_NOT_NULL) == EXIT_SUCCESS) {
        printf("ERROR: Constraint '%s' does not exist in table '%s'\n", constraintName, tableName);
        AK_EPI;
        return EXIT_ERROR;
    }

    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&row_root);
    
    AK_Update_Existing_Element(TYPE_VARCHAR, constraintName, tableName, constraint_attr, row_root);
    int result = AK_delete_row(row_root);
    AK_DeleteAll_L3(&row_root);
	AK_free(row_root);    

    if (result == EXIT_SUCCESS) {
        printf("SUCCESS: Constraint '%s' successfully deleted from table '%s'\n", constraintName, tableName);
    } else {
        printf("ERROR: Failed to delete constraint '%s' from table '%s'\n", constraintName, tableName);
    }

    AK_EPI;

    return result;
}

typedef struct {
    char *table;
    char *attribute;
    char *constraint;
    bool shouldSucceed;
    char *description;
} NnullTest;

/**
  * @author Saša Vukšić, updated by Nenad Makar, updated by Tea Jelavić, updated by Vilim Trakoštanec
  * @brief Function for testing NOT NULL constraint
  * @return Test result containing number of passed and failed tests
  */
TestResult AK_nnull_constraint_test() {
    AK_PRO;
    int passed = 0, failed = 0;

    /* Define test cases in a table */
    NnullTest tests[] = {
        { "student", "firstname", "firstnameNotNull", true, "Initial constraint set" },
        { AK_CONSTRAINTS_NOT_NULL, NULL, "firstnameNotNull", true, "Delete existing constraint" },
        { "student", "firstname", "firstnameNotNull", true, "Re-add deleted constraint" },
        { "student", "firstname", "firstnameNotNull", false, "Attempt to add duplicate constraint" },
        { "student", "mbr", "mbrNotNull", true, "Add constraint on different attribute" },
        { AK_CONSTRAINTS_NOT_NULL, NULL, "mbrNotNull", true, "Delete second constraint" },
        { "professor", "id_prof", "id_prof_NotNull", true, "Add constraint to different table" },
        { AK_CONSTRAINTS_NOT_NULL, NULL, "id_prof_NotNull", true, "Clean up professor constraint" },
        { "professor2", "id_prof", "id_prof_2_NotNull", true, "Add constraint to professor2" },
        { AK_CONSTRAINTS_NOT_NULL, NULL, "id_prof_2_NotNull", true, "Clean up professor2 constraint" },
        { "assistant", "id_prof", "id_assistant_NotNull", true, "Add constraint to assistant" },
        { AK_CONSTRAINTS_NOT_NULL, NULL, "id_assistant_NotNull", true, "Clean up assistant constraint" }
    };
    size_t nTests = sizeof(tests)/sizeof(*tests);

    printf("\nInitial state of NOT NULL constraints:\n");
    AK_print_table(AK_CONSTRAINTS_NOT_NULL);

    for (size_t i = 0; i < nTests; i++) {
        NnullTest *t = &tests[i];
        printf("\n---------------- TEST %zu: %s ----------------\n", i+1, t->description);

        if (t->table == AK_CONSTRAINTS_NOT_NULL && t->attribute == NULL) {
            /* This is a delete test */
            printf("Deleting constraint '%s'...\n", t->constraint);
            int res = AK_delete_constraint_not_null(t->table, t->constraint);
            if ((res == EXIT_SUCCESS) == t->shouldSucceed) {
                passed++;
                printf("Delete test successful!\n");
            } else {
                failed++;
                printf("Delete test failed! Expected %s but got %s\n", 
                       t->shouldSucceed ? "success" : "failure",
                       res == EXIT_SUCCESS ? "success" : "failure");
            }
        } else {
            /* This is a set-constraint test */
            printf("Setting NOT NULL constraint on %s.%s as '%s'...\n",
                   t->table, t->attribute, t->constraint);
            int res = AK_set_constraint_not_null(t->table, t->attribute, t->constraint);
            if ((res == EXIT_SUCCESS) == t->shouldSucceed) {
                passed++;
                printf("Set constraint test successful!\n");
            } else {
                failed++;
                printf("Set constraint test failed! Expected %s but got %s\n",
                       t->shouldSucceed ? "success" : "failure",
                       res == EXIT_SUCCESS ? "success" : "failure");
            }
        }

        printf("\nCurrent state of constraints:\n");
        AK_print_table(AK_CONSTRAINTS_NOT_NULL);
        printf("\n");
    }

    /* Final cleanup - make sure we've removed all test constraints */
    const char *cleanup_constraints[] = {
        "firstnameNotNull", "mbrNotNull", "id_prof_NotNull", 
        "id_prof_2_NotNull", "id_assistant_NotNull"
    };
    size_t nCleanup = sizeof(cleanup_constraints) / sizeof(*cleanup_constraints);
    
    printf("\nPerforming final cleanup...\n");
    for (size_t i = 0; i < nCleanup; i++) {
        AK_delete_constraint_not_null(AK_CONSTRAINTS_NOT_NULL, cleanup_constraints[i]);
    }

    printf("\nFinal state of AK_constraints_not_null table:\n");
    AK_print_table(AK_CONSTRAINTS_NOT_NULL);

    AK_EPI;
    return TEST_result(passed, failed);
}
