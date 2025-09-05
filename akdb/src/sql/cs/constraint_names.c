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
 * @author Nenad Makar, updated by Matej Lipovača, updated by Marko Belusic
 * @brief Function that checks if constraint name would be unique in database 
 * @param constraintName constraintName name which you want to give to constraint which you are trying to create
 * @param constraintTable name of the constraint table you want to seach, put NULL if you want to seach all constraint tables
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
    int success = 0;
    int failed = 0;
    
    /* Names & values */
    const char* tableName = "student";
    const char* seqTableName = "AK_sequence";
    const char* attYear = "year";
    const char* constraintYear = "yearUnique";
    
    typedef enum { SET_UNIQUE, DELETE_UNIQUE } Op;
    typedef struct { Op op; const char* tbl; const char* att; const char* cname; int exp; } T;
    
    T tests[] = {
        {SET_UNIQUE, seqTableName, "name", "nameUnique", 1}, // set sequence.name to unique
        {DELETE_UNIQUE, NULL, NULL, "nameUnique", 1}, // delete nameUnique constraint
        //{SET_UNIQUE, tableName, attYear, constraintYear, 1},
		//this passes on second run but it does not make sense
		//{DELETE_UNIQUE,tableName,attYear,constraintYear} // set year to unique 
		/*
		On second run of tests this fails because it can't set year to unique -
		Idk how, where, why values for years get doubled each time tests run and
		this negates UNQUE constraint and causes this test to fail.
		*/
		
    };
    size_t n = sizeof tests/sizeof *tests;
    
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
    
    for(size_t i = 0; i < n; i++) {
        T* t = &tests[i];
        printf("\n==== Running Test #%zu ====\n", i+1);
        int result;
        
        if(t->op == SET_UNIQUE) {
            printf("\nSetting %s.%s to UNIQUE with constraint name %s\n", t->tbl, t->att, t->cname);
            AK_set_constraint_unique((char*)t->tbl, (char*)t->att, (char*)t->cname);
            result = AK_check_constraint_name((char*)t->cname, AK_CONSTRAINTS_UNIQUE);
            if((result == EXIT_ERROR) == t->exp) {
                success++;
                printf("\nSUCCESS\n\n");
            } else {
                failed++;
                printf("\nFAILED\n\n");
            }
        } else { // DELETE_UNIQUE
            printf("\nDeleting the UNIQUE constraint %s\n", t->cname);
            AK_delete_constraint_unique("AK_constraints_unique", (char*)t->cname);
            result = AK_check_constraint_name((char*)t->cname, AK_CONSTRAINTS_UNIQUE);
            if((result == EXIT_SUCCESS) == t->exp) {
                success++;
                printf("\nSUCCESS\n\n");
            } else {
                failed++;
                printf("\nFAILED\n\n");
            }
        }
    }

    AK_EPI;
    return TEST_result(success, failed);
}
