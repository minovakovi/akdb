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

	newConstraint = AK_read_constraint_not_null(tableName, attName, NULL);
	if(newConstraint == EXIT_ERROR)
	{
		printf("\n\nFAILURE!\nNOT NULL constraint already exists on attribute: %s\nof table: %s\n\n", attName, tableName);
		AK_EPI;
		return EXIT_ERROR;
	}
   
	check =  AK_check_constraint_not_null(tableName, attName, constraintName);
	if(check == EXIT_ERROR)
	{
		printf("\n****FAILURE! NOT NULL constraint cannot be set!****\n\n");
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
	printf("\nNOT NULL constraint is successfully set on attribute: %s\nof table: %s\n\n", attName, tableName);
	
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

	numRows = AK_get_num_records(tableName);
	
	if(numRows > 0)
	{
		int positionOfAtt = AK_get_attr_index(tableName, attName) + 1;
		
		for(i=0; i<numRows; i++)
		{
			row = AK_get_row(i, tableName);
			attribute = AK_GetNth_L2(positionOfAtt, row);
			
			if((tupple_to_string_return=AK_tuple_to_string(attribute)) == NULL)
			{
				printf("\nFAILURE!\nTable: %s\ncontains NULL sign and that would violate NOT NULL constraint which You would like to set on attribute: %s\n\n", tableName, attName);
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

	if(uniqueConstraintName == EXIT_ERROR)
	{
		printf("\nFAILURE!\nConstraint name is not unique! Constraint name: %s\nalready exists in database!!\n\n", constraintName);
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

	struct list_node *row;
	struct list_node *attribute;
	struct list_node *table;
	int i;
	
	AK_PRO;

	if(newValue == NULL) {
		if(numRecords != 0) {
			for (i = 0; i < numRecords; i++) 
			{
				row = AK_get_row(i, "AK_constraints_not_null");
				attribute = AK_GetNth_L2(4, row);
				
				if(strcmp(attribute->data, attName) == 0) 
				{
					table = AK_GetNth_L2(2, row);
					
					if(strcmp(table->data, tableName) == 0) 
					{
						AK_DeleteAll_L3(&row);
						AK_free(row);
						AK_EPI;
						return EXIT_ERROR;
					}
				}
				AK_DeleteAll_L3(&row);
				AK_free(row);
			}			
		}
	}

	AK_EPI;
	return EXIT_SUCCESS;		
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

    char* constraint_attr = "constraintName";

    if(AK_check_constraint_name(constraintName, AK_CONSTRAINTS_NOT_NULL) == EXIT_SUCCESS){
        printf("FAILURE! -- CONSTRAINT with name %s doesn't exist in TABLE %s", constraintName, tableName);
        AK_EPI;
        return EXIT_ERROR;
    }

    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&row_root);
    
    AK_Update_Existing_Element(TYPE_VARCHAR, constraintName, tableName, constraint_attr, row_root);
    int result = AK_delete_row(row_root);
    AK_DeleteAll_L3(&row_root);
	AK_free(row_root);    

    AK_EPI;

    return result;
}

/**
  * @author Saša Vukšić, updated by Nenad Makar, updated by Tea Jelavić
  * @brief Function for testing NOT NULL constraint
  * @return No return value
  */
TestResult AK_nnull_constraint_test() {
	//TABLES
	char* tableName = "student";
	char* tableName2 = "professor";
	char* tableName3 = "professor2";
	char* tableName4 = "assistant";
	char* tableName5 = "employee";
	char* tableName6 = "department";
	
	//ATTRIBUTES
	char* attMbr = "mbr";
	char* attIdProf = "id_prof";
	char* attIdProf2 = "id_prof";
	char* attIdAssistant = "id_prof";
	char* attIdEmployeeProf = "id_prof";
	char* attIdEmployeeDept = "id_department";
	char* attIdDepartment = "id_department";
	char* attName = "firstname";
	
	//CONSTRAINTS
	char* constraintName = "firstnameNotNull";
	char* constraintName1 = "mbrNotNull";
	char* constraintName2 = "id_prof_NotNull";
	char* constraintName3 = "id_prof_2_NotNull";
	char* constraintName4 = "id_assistant_NotNull";
	char* constraintName5 = "id_employee_prof_NotNull";
	char* constraintName6 = "id_employee_dept_NotNull";
	char* constraintName7 = "id_department_NotNull";
	char* newValue = NULL;
	int passed=0;
	int failed=0;

	AK_PRO;
	printf("\nList of existing NOT NULL constraints:\n\n");
	AK_print_table(AK_CONSTRAINTS_NOT_NULL);
	printf("\nTest table:\n\n");	
	AK_print_table(tableName);

	printf("\n\n");
	printf("---------------------------------------------------------------------------------");
	printf("\n\n");

	printf("\nTEST 1 - Trying to set NOT NULL constraint on attribute %s of table %s...\n\n", attName, tableName);
	int resultTest1 = AK_set_constraint_not_null(tableName, attName, constraintName);
	AK_print_table(AK_CONSTRAINTS_NOT_NULL);
	if(resultTest1 == EXIT_SUCCESS)
	{	
		passed++;
		printf("\nTest successful!");
	}

	else
	{
		failed++;
		printf("\nTest failed!");
	}

	printf("\n\n-------------------------------------------------------------------------------------\n\n");
	
	printf("\nTEST 2 - Deleting NOT NULL constraint %s...\n", constraintName);
    int resultTest2 = AK_delete_constraint_not_null(AK_CONSTRAINTS_NOT_NULL, constraintName);
    AK_print_table("AK_constraints_not_null");
    if(resultTest2 == EXIT_SUCCESS) 
	{
		passed++;
        printf("\nTest successful!");
    }
	else
	{
		failed++;
		printf("\nTest failed!");
	}

	printf("\n\n-------------------------------------------------------------------------------------\n\n");

	printf("\nTEST 3 - Trying to set NOT NULL constraint on attribute %s of table %s AGAIN...\n\n", attName, tableName);
	int resultTest3 = AK_set_constraint_not_null(tableName, attName, constraintName);
	if(resultTest3 == EXIT_SUCCESS) 
		{
			passed++;
            printf("\nTest successful!");
        }
		else
		{
			failed++;
			printf("\nTest failed!");
		}
	printf("\n");
	AK_print_table("AK_constraints_not_null");

	printf("\n\n-------------------------------------------------------------------------------------\n\n");

	printf("\nTEST 4 - Trying to set NOT NULL constraint on attribute %s of table %s AGAIN...\n\n", attName, tableName);
	int resultTest4 = AK_set_constraint_not_null(tableName, attName, constraintName);
	//this constraint is already in the AK_constaints_not_null table. Test is supossed to fail, so it will succeed if it fails
	if(resultTest4 != EXIT_SUCCESS) 
		{
			passed++;
            printf("\nTest successful!");
        }
		else
		{
			failed++;
			printf("\nTest failed!");
		}
	printf("\n");
	AK_print_table("AK_constraints_not_null");
	
	printf("\n\n-------------------------------------------------------------------------------------\n\n");

	//deleting constraint added with test3
	AK_delete_constraint_not_null(AK_CONSTRAINTS_NOT_NULL, constraintName);
	printf("\nConstraint from test3 deleted");
	printf("\n");
	AK_print_table("AK_constraints_not_null");

	printf("\n\n-------------------------------------------------------------------------------------\n\n");

	printf("\nTEST 5 - Trying to set NOT NULL constraint on attribute %s of table %s...\n\n", attMbr, tableName);
	int resultTest5 = AK_set_constraint_not_null(tableName, attMbr, constraintName1);
	if(resultTest5 == EXIT_SUCCESS) 
		{
			passed++;
            printf("\nTest successful!");
        }
		else
		{
			failed++;
			printf("\nTest failed!");
		}
	printf("\n");
	AK_print_table("AK_constraints_not_null");

	printf("\n\n-------------------------------------------------------------------------------------\n\n");

	//deleting constraint added with test5
	AK_delete_constraint_not_null(AK_CONSTRAINTS_NOT_NULL, constraintName1);
	printf("\nConstraint from test5 deleted");
	printf("\n");
	AK_print_table("AK_constraints_not_null");

	printf("\n\n-------------------------------------------------------------------------------------\n\n");

	printf("\nTEST 6 - Trying to set NOT NULL constraint on attribute %s of table %s...\n\n", attIdProf, tableName2);
	int resultTest6 = AK_set_constraint_not_null(tableName2, attIdProf, constraintName2);
	if(resultTest6 == EXIT_SUCCESS) 
		{
			passed++;
            printf("\nTest successful!");
        }
		else
		{
			failed++;
			printf("\nTest failed!");
		}
	printf("\n");
	AK_print_table("AK_constraints_not_null");

	printf("\n\n-------------------------------------------------------------------------------------\n\n");

	//deleting constraint added with test6
	AK_delete_constraint_not_null(AK_CONSTRAINTS_NOT_NULL, constraintName2);
	printf("\nConstraint from test6 deleted");
	printf("\n");
	AK_print_table("AK_constraints_not_null");

	printf("\n\n-------------------------------------------------------------------------------------\n\n");

	printf("\nTEST 7 - Trying to set NOT NULL constraint on attribute %s of table %s...\n\n", attIdProf2, tableName3);
	int resultTest7 = AK_set_constraint_not_null(tableName3, attIdProf2, constraintName3);
	if(resultTest7 == EXIT_SUCCESS) 
		{
			passed++;
            printf("\nTest successful!");
        }
		else
		{
			failed++;
			printf("\nTest failed!");
		}
	printf("\n");
	AK_print_table("AK_constraints_not_null");

	printf("\n\n-------------------------------------------------------------------------------------\n\n");

	//deleting constraint added with test7
	AK_delete_constraint_not_null(AK_CONSTRAINTS_NOT_NULL, constraintName3);
	printf("\nConstraint from test7 deleted");
	printf("\n");
	AK_print_table("AK_constraints_not_null");

	printf("\n\n-------------------------------------------------------------------------------------\n\n");

	printf("\nTEST 8 - Trying to set NOT NULL constraint on attribute %s of table %s...\n\n", attIdAssistant, tableName4);
	int resultTest8 = AK_set_constraint_not_null(tableName4, attIdAssistant, constraintName4);
	if(resultTest8 == EXIT_SUCCESS) 
		{
			passed++;
            printf("\nTest successful!");
        }
		else
		{
			failed++;
			printf("\nTest failed!");
		}
	printf("\n");
	AK_print_table("AK_constraints_not_null");

	printf("\n\n-------------------------------------------------------------------------------------\n\n");

	//deleting constraint added with test8
	AK_delete_constraint_not_null(AK_CONSTRAINTS_NOT_NULL, constraintName4);
	printf("\nConstraint from test8 deleted");
	printf("\n");
	AK_print_table("AK_constraints_not_null");

	printf("\n\n-------------------------------------------------------------------------------------\n\n");

	printf("\nTEST 9 - Trying to set NOT NULL constraint on attribute %s of table %s...\n\n", attIdEmployeeProf, tableName5);
	int resultTest9 = AK_set_constraint_not_null(tableName5, attIdEmployeeProf, constraintName5);
	if(resultTest9 == EXIT_SUCCESS) 
		{
			passed++;
            printf("\nTest successful!");
        }
		else
		{
			failed++;
			printf("\nTest failed!");
		}
	printf("\n");
	AK_print_table("AK_constraints_not_null");

	printf("\n\n-------------------------------------------------------------------------------------\n\n");

	//deleting constraint added with test9
	AK_delete_constraint_not_null(AK_CONSTRAINTS_NOT_NULL, constraintName5);
	printf("\nConstraint from test9 deleted");
	printf("\n");
	AK_print_table("AK_constraints_not_null");

	printf("\n\n-------------------------------------------------------------------------------------\n\n");

	printf("\nTEST 10 - Trying to set NOT NULL constraint on attribute %s of table %s...\n\n", attIdEmployeeDept, tableName5);
	int resultTest10 = AK_set_constraint_not_null(tableName5, attIdEmployeeDept, constraintName6);
	if(resultTest10 == EXIT_SUCCESS) 
		{
			passed++;
            printf("\nTest successful!");
        }
		else
		{
			failed++;
			printf("\nTest failed!");
		}
	printf("\n");
	AK_print_table("AK_constraints_not_null");

	printf("\n\n-------------------------------------------------------------------------------------\n\n");

	//deleting constraint added with test10
	AK_delete_constraint_not_null(AK_CONSTRAINTS_NOT_NULL, constraintName6);
	printf("\nConstraint from test10 deleted");
	printf("\n");
	AK_print_table("AK_constraints_not_null");

	printf("\n\n-------------------------------------------------------------------------------------\n\n");

	printf("\nTEST 11 - Trying to set NOT NULL constraint on attribute %s of table %s...\n\n", attIdDepartment, tableName6);
	int resultTest11 = AK_set_constraint_not_null(tableName6, attIdDepartment, constraintName7);
	if(resultTest11 == EXIT_SUCCESS) 
		{
			passed++;
            printf("\nTest successful!");
        }
		else
		{
			failed++;
			printf("\nTest failed!");
		}
	printf("\n");
	AK_print_table("AK_constraints_not_null");

	printf("\n\n-------------------------------------------------------------------------------------\n\n");

	//deleting constraint added with test11
	AK_delete_constraint_not_null(AK_CONSTRAINTS_NOT_NULL, constraintName7);
	printf("\nConstraint from test11 deleted");
	printf("\n");
	AK_print_table("AK_constraints_not_null");

	printf("\n\n-------------------------------------------------------------------------------------\n\n");


	printf("\nFinal AK_contraints_not_null table should be the same as it was before doing any of these tests");
	AK_print_table("AK_constraints_not_null");
	
	
	AK_EPI;

	return TEST_result(passed,failed);
}
