/**
@file unique.c Provides functions for unique constraint
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

#include "unique.h"

/**
 * @author Domagoj Tuličić, updated by Nenad Makar 
 * @brief Function that sets unique constraint on attribute(s)
 * @param char* tableName name of table
 * @param char attName[] name(s) of attribute(s), if you want to set UNIQUE constraint on combination of attributes seperate their names with constant SEPARATOR (see test)
 * @param char constraintName[] name of constraint
 * @return EXIT_ERROR or EXIT_SUCCESS
 **/

int AK_set_constraint_unique(char* tableName, char attName[], char constraintName[]){
	int i;
	int j;
	int numRows;
	int newConstraint;
	int uniqueConstraintName;
	char attributeName[MAX_VARCHAR_LENGTH]="";
	char values[MAX_VARCHAR_LENGTH]="";
	struct list_node *row;
	struct list_node *row2;
	struct list_node *attribute;
	struct list_node *attribute2;
	dictionary* dict;
	AK_PRO;

	strcat(attributeName, "tableName");
	strcat(attributeName, SEPARATOR);
	strcat(attributeName, "attributeName");

	strcat(values, tableName);
	strcat(values, SEPARATOR);
	strcat(values, attName);

	newConstraint = AK_read_constraint_unique("AK_constraints_unique", attributeName, values);

	if(newConstraint == EXIT_ERROR)
	{
		printf("\nFAILURE!\nUNIQUE constraint already exists on (combination of) attribute(s): %s\nof table: %s\n\n", attName, tableName);
		AK_EPI;
		return EXIT_ERROR;
	}
	
	numRows = AK_get_num_records(tableName);
	
	if(numRows > 0)
	{
		row = AK_get_row(0, tableName);
		int numOfAttsInTable = AK_Size_L2(row);
		int positionsOfAtts[numOfAttsInTable];
		int numOfImpAttPos = 0;
		char attNameCopy[MAX_VARCHAR_LENGTH];
		char *nameOfOneAtt;
		char namesOfAtts[numOfAttsInTable][MAX_VARCHAR_LENGTH];
		char *key, *val;
		AK_DeleteAll_L3(&row);
		AK_free(row);
		strncpy(attNameCopy, attName, sizeof(attNameCopy));

		nameOfOneAtt = strtok(attNameCopy, SEPARATOR);
		while(nameOfOneAtt != NULL)
		{
			positionsOfAtts[numOfImpAttPos] = AK_get_attr_index(tableName, nameOfOneAtt) + 1;
			strncpy(namesOfAtts[numOfImpAttPos], nameOfOneAtt, sizeof(namesOfAtts[numOfImpAttPos]));

			numOfImpAttPos++;

			nameOfOneAtt = strtok(NULL, SEPARATOR);
		}
		
		int match;
		int impoIndexInArray;
		char *tuple_to_string_return;
		

		dict = dictionary_new(0);
		if(!dict) {
			printf("ERROR: Dictionary initialization failed.");
			AK_EPI;
			return EXIT_ERROR;
		}
		for(i=0; i<numRows-1; i++)
		{
			row = AK_get_row(i, tableName);
				
			match = 1;
			for(impoIndexInArray=0; (impoIndexInArray<numOfImpAttPos)&&(match==1); impoIndexInArray++)
			{
				attribute = AK_GetNth_L2(positionsOfAtts[impoIndexInArray], row);
				tuple_to_string_return = AK_tuple_to_string(attribute);
				if(tuple_to_string_return==NULL)
				{
					match = 0;
				}
				else if(dictionary_get(dict, tuple_to_string_return, NULL) == NULL)
				{
					key=AK_tuple_to_string(attribute);
					val=AK_tuple_to_string(attribute);
					dictionary_set(dict, key, val);
					match = 0;
					AK_free(key);
					AK_free(val);
				}
				AK_free(tuple_to_string_return);
			}
				
			if(match == 1)
			{
				printf("\nFAILURE!\nExisting values in table: %s\nwould violate UNIQUE constraint which You would like to set on (combination of) attribute(s): %s\n\n", tableName, attName);
				dictionary_del(dict);
				AK_DeleteAll_L3(&row);
				AK_free(row);
				AK_EPI;
				return EXIT_ERROR;
			}
			AK_DeleteAll_L3(&row);
			AK_free(row);
		}
	dictionary_del(dict);
	}

	

	uniqueConstraintName = AK_check_constraint_name(constraintName, AK_CONSTRAINTS_UNIQUE);

	if(uniqueConstraintName == EXIT_ERROR)
	{
		printf("\nFAILURE!\nConstraint name: %s\nalready exists in database\n\n", constraintName);
		AK_EPI;
		return EXIT_ERROR;
	}

	struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
	AK_Init_L3(&row_root);

	int obj_id = AK_get_id();
	AK_Insert_New_Element(TYPE_INT, &obj_id, "AK_constraints_unique", "obj_id", row_root);
	AK_Insert_New_Element(TYPE_VARCHAR, tableName, "AK_constraints_unique", "tableName", row_root);
	AK_Insert_New_Element(TYPE_VARCHAR, constraintName, "AK_constraints_unique", "constraintName", row_root);
	AK_Insert_New_Element(TYPE_VARCHAR, attName, "AK_constraints_unique", "attributeName", row_root);
	AK_insert_row(row_root);
	AK_DeleteAll_L3(&row_root);
	AK_free(row_root);
	printf("\nUNIQUE constraint is set on (combination of) attribute(s): %s\nof table: %s\n\n", attName, tableName);
	AK_EPI;
	return EXIT_SUCCESS;
}

/**
 * @author Domagoj Tuličić, updated by Nenad Makar 
 * @brief Function that checks if the insertion of some value(s) would violate the UNIQUE constraint
 * @param char* tableName name of table
 * @param char attName[] name(s) of attribute(s), if you want to check combination of values of more attributes seperate names of attributes with constant SEPARATOR (see test)
 * @param char newValue[] new value(s), if you want to check combination of values of more attributes seperate their values with constant SEPARATOR (see test),
	if some value(s) which you want to check isn't stored as char (string) convert it to char (string) using AK_tuple_to_string(struct list_node *tuple) or with sprintf 
	in a similiar way it's used in that function (if value isn't part of a *tuple),
	to concatenate more values in newValue[] use strcat(destination, source) and put constant SEPARATOR between them (see test)
	if newValue[] should contain NULL sign pass it as " " (space)
 * @return EXIT_ERROR or EXIT_SUCCESS
 **/

int AK_read_constraint_unique(char* tableName, char attName[], char newValue[]){
	char *value;
	//Because strtok could create problems for newValue...
	char newValueCopy[MAX_VARCHAR_LENGTH];
	int numOfValues = 0;
	AK_PRO;

	int strcmpTableName = strcmp(tableName, "AK_constraints_unique");
	int strcmpAttName = strcmp(attName, "obj_id");
	
	//If there's check if combination of values of attributes tableName and attributeName of table AK_constraints_unique are UNIQUE numOfValues is 2
	//First value is value of attribute tableName and second value is value of attribute attributeName which could be made of more SEPARATORS and then it would seem like
	//there are more than two values in newValue[]
	//That combination is used to check if there's some UNIQUE constraint set on table AK_constraints_unique
	//Combination of values of attributes tableName and attributeName is only needed UNIQUE combination in table AK_constraints_unique
	//Values of attribute obj_id (alone) will be UNIQUE when PRIMARY KEY will be created on table AK_constraints_unique
	//UNIQUENESS of value of attribute constraintName is checked with AK_check_constraint_name among all constraint names in database so there's
	//no need to set UNIQUE constraint on that attribute
	if(strcmpTableName==0 && strcmpAttName!=0)
	{
		numOfValues = 2;
	}
	else
	{
		strncpy(newValueCopy, newValue, sizeof(newValueCopy));

		value = strtok(newValueCopy, SEPARATOR);
		while(value != NULL)
		{
			//Every combination which contains NULL sign ( ) is UNIQUE
			//http://www.postgresql.org/docs/current/interactive/ddl-constraints.html#DDL-CONSTRAINTS-UNIQUE-CONSTRAINTS
			if(strcmp(value, " ") == 0)
			{
				return EXIT_SUCCESS;
				AK_EPI;	
			}
			numOfValues++;
			value = strtok(NULL, SEPARATOR);
		}
	}

	int numRecords = AK_get_num_records("AK_constraints_unique");
	
	if(numRecords!=0 && (strcmpTableName!=0 || (strcmpTableName==0 && strcmpAttName==0)))
	{
		struct list_node *row;
		struct list_node *attribute;
		struct list_node *table;
		int i;
		
		for(i=0; i<numRecords; i++)
		{
			row = AK_get_row(i, "AK_constraints_unique");
			attribute = AK_GetNth_L2(4, row);
			
			if(strcmp(attribute->data, attName) == 0)
			{
				table = AK_GetNth_L2(2, row);
				
				if(strcmp(table->data, tableName) == 0)
				{
					int numRows = AK_get_num_records(table->data);
					
					if(numRows == 0)
					{
						AK_EPI;
						return EXIT_SUCCESS;
					}
					
					struct list_node *row2 = AK_get_row(0, table->data);
					int numOfAttsInTable = AK_Size_L2(row2);
					int positionsOfAtts[numOfAttsInTable];
					int numOfImpAttPos = 0;
					char attNameCopy[MAX_VARCHAR_LENGTH];
					char *nameOfOneAtt;
					char namesOfAtts[numOfAttsInTable][MAX_VARCHAR_LENGTH];
					struct list_node *attribute2;
					
					strncpy(attNameCopy, attName, sizeof(attNameCopy));

					nameOfOneAtt = strtok(attNameCopy, SEPARATOR);
					while(nameOfOneAtt != NULL)
					{
						positionsOfAtts[numOfImpAttPos] = AK_get_attr_index(table->data, nameOfOneAtt) + 1;
						strncpy(namesOfAtts[numOfImpAttPos], nameOfOneAtt, sizeof(namesOfAtts[numOfImpAttPos]));
						numOfImpAttPos++;

						nameOfOneAtt = strtok(NULL, SEPARATOR);
					}
					
					int h;
					int impoIndexInArray;
					int match;
					int index = 0;
					char *value2;
					char newValueCopy2[MAX_VARCHAR_LENGTH];
					char values[numOfValues][MAX_VARCHAR_LENGTH];

					strncpy(newValueCopy2, newValue, sizeof(newValueCopy2));

					value2 = strtok(newValueCopy2, SEPARATOR);
					while(value2 != NULL)
					{
						strncpy(values[index], value2, sizeof(values[index]));
						index++;
						value2 = strtok(NULL, SEPARATOR);
					}

					
					for(h=0; h<numRows; h++)
					{
						row2 = AK_get_row(h, table->data);

						match = 1;
						
						for(impoIndexInArray=0; (impoIndexInArray<numOfImpAttPos)&&(match==1); impoIndexInArray++)
						{
							attribute2 = AK_GetNth_L2(positionsOfAtts[impoIndexInArray], row2);
							if(AK_tuple_to_string(attribute2) == NULL)
							{
								match = 0;
							}
							else if(strcmp(values[impoIndexInArray], AK_tuple_to_string(attribute2)) != 0)
							{
								match = 0 ;
							}

						}
						
						if(match == 1)
						{
							AK_EPI;
							return EXIT_ERROR;
						}
					}
					
					AK_EPI;
					return EXIT_SUCCESS;
				}
			}
		}
		
		AK_EPI;
		return EXIT_SUCCESS;
	}
	else if(numRecords !=0 && strcmpTableName==0 && strcmpAttName!=0)
	{
		struct list_node *row = AK_get_row(0, tableName);
		int numOfAttsInTable = AK_Size_L2(row);
		int positionsOfAtts[numOfAttsInTable];
		int numOfImpAttPos = 0;
		char attNameCopy[MAX_VARCHAR_LENGTH];
		char *nameOfOneAtt;
		char namesOfAtts[numOfAttsInTable][MAX_VARCHAR_LENGTH];
		struct list_node *attribute2;
		AK_DeleteAll_L3(&row);
		AK_free(row);
		
		strncpy(attNameCopy, attName, sizeof(attNameCopy));

		nameOfOneAtt = strtok(attNameCopy, SEPARATOR);
		while(nameOfOneAtt != NULL)
		{
			positionsOfAtts[numOfImpAttPos] = AK_get_attr_index(tableName, nameOfOneAtt) + 1;
			strncpy(namesOfAtts[numOfImpAttPos], nameOfOneAtt, sizeof(namesOfAtts[numOfImpAttPos]));
			numOfImpAttPos++;

			nameOfOneAtt = strtok(NULL, SEPARATOR);
		}
		
		int h;
		int impoIndexInArray;
		int match;
		int index = 0;
		char *value2;
		char newValueCopy2[MAX_VARCHAR_LENGTH];
		char values[numOfValues][MAX_VARCHAR_LENGTH];
		
		char *tuple_to_string_return;

		strncpy(newValueCopy2, newValue, sizeof(newValueCopy2));

		value2 = strtok(newValueCopy2, SEPARATOR);
		strncpy(values[index], value2, sizeof(values));
		index++;
		value2 = strtok(NULL, "");
		strncpy(values[index], value2+strlen(SEPARATOR)-1, sizeof(values[index]));

		int numRows = AK_get_num_records(tableName);

		for(h=0; h<numRows; h++)
		{
			row = AK_get_row(h, tableName);

			match = 1;
			
			for(impoIndexInArray=0; (impoIndexInArray<numOfImpAttPos)&&(match==1); impoIndexInArray++)
			{
				attribute2 = AK_GetNth_L2(positionsOfAtts[impoIndexInArray], row);
				tuple_to_string_return = AK_tuple_to_string(attribute2);
				if(tuple_to_string_return == NULL)
				{
					match = 0;
				}
				else if(strcmp(values[impoIndexInArray], tuple_to_string_return) != 0)
				{
					match = 0 ;
					AK_free(tuple_to_string_return);
				}
				else
					AK_free(tuple_to_string_return);
			}
			AK_DeleteAll_L3(&row);
			AK_free(row);
			if(match == 1)
			{
				AK_EPI;
				return EXIT_ERROR;
			}
		}
		
		AK_EPI;
		return EXIT_SUCCESS;
	}
	else
	{
		AK_EPI;
		return EXIT_SUCCESS;
	}
}
 
/**
 * @author Blaž Rajič, updated by Bruno Pilošta
 * @brief Function for deleting specific unique constraint
 * @param tableName name of table on which constraint refers
 * @param constraintName name of constraint 
 * @return EXIT_SUCCESS when constraint is deleted, else EXIT_ERROR
 */
int AK_delete_constraint_unique(char* tableName, char* constraintName){
    AK_PRO;

    char* constraint_attr = "constraintName";

    if(AK_check_constraint_name(constraintName, AK_CONSTRAINTS_UNIQUE) == EXIT_SUCCESS){
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
  * @author Domagoj Tuličić, updated by Nenad Makar , updated by Vilim Trkaoštanec
  * @brief Function for testing UNIQUE constraint
  * @return No return value
  */



TestResult AK_unique_test(void) {
    AK_PRO;
    int passed = 0, failed = 0;

    // Setup names
    char* table1 = "student";
    char* table2 = "professor";
    char* table3 = "professor2";
    char* table4 = "assistant";
    char* table5 = "employee";
    char* table6 = "department";

    char attr1[] = "year";
    char attr2[] = "firstname";
    char names1[MAX_VARCHAR_LENGTH] = "";
    char names2[MAX_VARCHAR_LENGTH] = "";
    char names3[MAX_VARCHAR_LENGTH] = "mbr";
    char names4[MAX_VARCHAR_LENGTH] = "id_prof";
    char names5[MAX_VARCHAR_LENGTH] = "id_prof";
    char names6[MAX_VARCHAR_LENGTH] = "id_prof";
    char names7[MAX_VARCHAR_LENGTH] = "id_prof";
    char names8[MAX_VARCHAR_LENGTH] = "id_department";
    char names9[MAX_VARCHAR_LENGTH] = "";

    char cons1[] = "yearUnique";
    char cons2[] = "firstnameUnique";
    char cons3[] = "mbrUnique";
    char cons4[] = "profIdUnique";
    char cons5[] = "profId2Unique";
    char cons6[] = "assistantId2Unique";
    char cons7[] = "emp_profId2Unique";
    char cons8[] = "emp_deptId2Unique";
    char cons9[] = "deptId2Unique";
    char c1[MAX_VARCHAR_LENGTH] = "";
    char c2[MAX_VARCHAR_LENGTH] = "";

    // Build combined names
    strcat(names1, "mbr"); strcat(names1, SEPARATOR); strcat(names1, "lastname");
    strcat(names2, "mbr"); strcat(names2, SEPARATOR); strcat(names2, "lastname"); strcat(names2, SEPARATOR); strcat(names2, "weight");
    strcat(names9, "id_department"); strcat(names9, SEPARATOR); strcat(names9, "dep_name");
    strcat(c1, "mbr"); strcat(c1, SEPARATOR); strcat(c1, "lastnameUnique");
    strcat(c2, "mbr"); strcat(c2, SEPARATOR); strcat(c2, "lastname"); strcat(c2, SEPARATOR); strcat(c2, "weightUnique");

    typedef enum { SET, DEL, DEL_ALL } Op;
    typedef struct { Op op; const char *tbl; const char *at; const char *cn; int exp; } T;

    T tests[] = {
        { SET, table1, attr1,  cons1, 1 },
        { DEL, "AK_constraints_unique", NULL, cons1, 1 },
        { SET, table1, attr1,  cons1, 1 },
        { SET, table1, attr2,  cons2, 0 },
        { SET, table1, names1, c1, 1 },
        { SET, table1, names2, c2, 1 },
        { SET, table1, names2, c2, 0 },
        { SET, table1, names3, cons3, 1 },
        { SET, table2, names4, cons4, 1 },
        { SET, table3, names5, cons5, 1 },
        { SET, table4, names6, cons6, 1 },
        { SET, table5, names7, cons7, 1 },
        { SET, table5, names8, cons8, 1 },
        { SET, table6, names9, cons9, 1 },
        { DEL_ALL, NULL, NULL, NULL, 1 }
    };
    size_t n = sizeof tests / sizeof *tests;

    for (size_t i = 0; i < n; ++i) {
        T *t = &tests[i];
        printf("\n---- TEST %zu ----\n", i + 1);
        int res = EXIT_ERROR;
        if (t->op == SET) {
            printf("Setting UNIQUE on %s.%s as %s...\n", t->tbl, t->at, t->cn);
            res = AK_set_constraint_unique((char *)t->tbl, (char *)t->at, (char *)t->cn);
        } else if (t->op == DEL) {
            printf("Deleting constraint %s...\n", t->cn);
            res = AK_delete_constraint_unique((char *)t->tbl, (char *)t->cn);
        } else {
            printf("\n============== Running Test DELETE ==============\n");
            printf("\nTrying to set delete all existing UNIQUE constraints ...\n\n");
            int d1 = AK_delete_constraint_unique("AK_constraints_unique", cons3);
            int d2 = AK_delete_constraint_unique("AK_constraints_unique", cons1);
            int d3 = AK_delete_constraint_unique("AK_constraints_unique", c1);
            int d4 = AK_delete_constraint_unique("AK_constraints_unique", c2);
            int d5 = AK_delete_constraint_unique("AK_constraints_unique", cons4);
            int d6 = AK_delete_constraint_unique("AK_constraints_unique", cons5);
            int d7 = AK_delete_constraint_unique("AK_constraints_unique", cons6);
            int d8 = AK_delete_constraint_unique("AK_constraints_unique", cons7);
            int d9 = AK_delete_constraint_unique("AK_constraints_unique", cons8);
            int d10= AK_delete_constraint_unique("AK_constraints_unique", cons9);
            if (d1 == EXIT_SUCCESS && d2 == EXIT_SUCCESS && d3 == EXIT_SUCCESS && d4 == EXIT_SUCCESS) {
                passed++;
                printf("\nSUCCESS\n\nAll existing UNIQUE constraints deleted successfully\n");
                res = EXIT_SUCCESS;
            } else {
                failed++;
                printf("\nFAILED\n\nOne or two UNIQUE constraints not deleted successfully\n");
                res = EXIT_ERROR;
            }
            AK_print_table("AK_constraints_unique");
        }
        if (t->op != DEL_ALL) {
            AK_print_table("AK_constraints_unique");
            int passed_flag = (res == EXIT_SUCCESS);
            if (passed_flag == t->exp) {
                passed++;
                printf("Test passed!\n");
            } else {
                failed++;
                printf("Test FAILED!\n");
            }
        }
    }

    AK_EPI;
    return TEST_result(passed, failed);
}
