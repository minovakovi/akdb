/**
 * @file constraint_names.c Provides functions for checking if constraint name is unique in database
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

#include "constraint_names.h"

/**
 * @author Nenad Makar, updated by Matej Lipovača
 * @brief Function that checks if constraint name would be unique in database 
 * @param char constraintName name which you want to give to constraint which you are trying to create
 * @return EXIT_ERROR or EXIT_SUCCESS
 **/
int AK_check_constraint_name(char *constraintName, char *constraintTable) {
	int i, j;
	int num_rows;

	/**
	 * Updated by Matej Lipovača
	 * Added other constraint names from catalog, aswell in "constants.h"
	 */
	char constraint_table_names[][50] = {
		AK_CONSTRAINTS_BEWTEEN,
		AK_CONSTRAINTS_UNIQUE,
		AK_CONSTRAINTS_CHECK_CONSTRAINT,
		AK_CONSTRAINTS_NOT_NULL,
		AK_CONSTRAINTS_INDEX,
		AK_CONSTRAINTS_PRIMARY_KEY,
		AK_CONSTRAINTS_FOREIGN_KEY,
		AK_CONSTRAINTS_DEFAULT,
		AK_REFERENCE
	};

	size_t constraint_table_names_size = sizeof(constraint_table_names) / sizeof(constraint_table_names[0]);

	struct list_node *row;
	struct list_node *attribute;

	// if constraintTable is not null only search through given table, else search every constraint table
	if(constraintTable != NULL){
		constraint_table_names_size = 1;
		strcpy(constraint_table_names[0], constraintTable);
	}

	AK_PRO;

	for (i = 0; i < constraint_table_names_size; ++i)
	{
		num_rows = AK_get_num_records(constraint_table_names[i]);

		for (j = 0; j < num_rows; ++j)
		{
			row = AK_get_row(j, constraint_table_names[i]);
			attribute = AK_GetNth_L2(3, row);
			
			if (strcmp(attribute->data, constraintName) == 0)
			{
				AK_DeleteAll_L3(&row);
				AK_free(row);
				AK_EPI;
				return EXIT_ERROR;
			}
			AK_DeleteAll_L3(&row);
			AK_free(row);
		}
	}
	
	AK_EPI;
	return EXIT_SUCCESS;
}
 
/**
  * @author Nenad Makar 
  * @brief Function that tests if constraint name would be unique in database
  * @return No return value
  */
TestResult AK_constraint_names_test() {
	char* tableName = "student";
	char *constraintName1 = "nameUnique";
	char *constraintName2 = "nameUNIQUE";
	char attYear[] = "year";
	char constraintYear[] = "yearUnique";
	int result;

	int success=0;
	int failed=0;
	AK_PRO;
	
	printf("\nExisting constraints:\n\n");
	AK_print_table("AK_constraints_not_null");
	AK_print_table("AK_constraints_unique");
	AK_print_table("AK_reference");
	AK_print_table("AK_constraints_between");
	AK_print_table("AK_constraints_index");
	AK_print_table("AK_constraints_foreign_key");
	AK_print_table("AK_constraints_primary_key");
	AK_print_table("AK_constraints_default");
	
	printf("\nChecking if constraint name %s would be unique in database...\n", constraintName1);
	result = AK_check_constraint_name(constraintName1, AK_CONSTRAINTS_UNIQUE);
	printf("Yes (0) No (-1): %d\n\n", result);
	
	printf("\nChecking if constraint name %s would be unique in database...\n", constraintName2);
	result = AK_check_constraint_name(constraintName2, AK_CONSTRAINTS_UNIQUE);
	printf("Yes (0) No (-1): %d\n\n", result);


	printf("\nSetting year attribute to UNIQUE in table student\n");
	AK_set_constraint_unique(tableName,  attYear, constraintYear);
	printf("\nChecking if constraint name %s would be unique in database...\n", constraintYear);
	result = AK_check_constraint_name(constraintYear, AK_CONSTRAINTS_UNIQUE);
	if(result==EXIT_ERROR){
		success++;
		printf("\nSUCCESS\n\n");
	}else{
		failed++;
		printf("\nFAILED\n\n");
	}

	printf("\nDeleting the UNIQUE constraint on atribute year in table student\n");
	AK_delete_constraint_unique("AK_constraints_unique", constraintYear);
	result = AK_check_constraint_name(constraintYear, AK_CONSTRAINTS_UNIQUE);
	if(result==EXIT_SUCCESS){
		success++;
		printf("\nSUCCESS\n\n");
	}else{
		failed++;
		printf("\nFAILED\n\n");
	}

	AK_EPI;
	return TEST_result(success,failed);
}
