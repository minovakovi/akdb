/**
@file reference.c Provides functions for referential integrity
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
 17 */

#include "reference.h"
#include <string.h>

// Added helper function for pointer party
struct list_node* AK_nth_next(struct list_node* node, int n) {
	while (node != NULL && n-- > 0) {
		node = node->next;
	}
	return node;
}

void safe_string_copy(char *dest, const char *src, size_t size) {
	strncpy(dest, src, size - 1);
	dest[size - 1] = '\0';
}

/**
 * @author Dejan Frankovic
 * @brief Function that adds a reference for a group of attributes over a given table to a group of attributes over another table with a given constraint name.
 * @param name of the child table
 * @param array of child table attribute names (foreign key attributes)
 * @param name of the parent table
 * @param array of parent table attribute names (primary key attributes)
 * @param number of attributes in foreign key
 * @param name of the constraint
 * @param type of the constraint, constants defined in 'reference.h'
 * @return EXIT_SUCCESS
 */
int AK_add_reference(char *childTable, char *childAttNames[], char *parentTable, char *parentAttNames[], int attNum, char *constraintName, int type) {
    int i;
    AK_PRO;
    if (type != REF_TYPE_CASCADE && type != REF_TYPE_NO_ACTION && type != REF_TYPE_RESTRICT && type != REF_TYPE_SET_DEFAULT && type != REF_TYPE_SET_NULL){
	AK_EPI;
	return 0;
    }

    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
    if (!row_root) {
    	AK_EPI;
	return 0;
    }
    AK_Init_L3(&row_root);

    for (i = 0; i < attNum; i++) {

	AK_DeleteAll_L3(&row_root);
        AK_Insert_New_Element(TYPE_VARCHAR, childTable, "AK_reference", "table", row_root);
        AK_Insert_New_Element(TYPE_VARCHAR, constraintName, "AK_reference", "constraint", row_root);
        AK_Insert_New_Element(TYPE_VARCHAR, childAttNames[i], "AK_reference", "attribute", row_root);
        AK_Insert_New_Element(TYPE_VARCHAR, parentTable, "AK_reference", "parent", row_root);
        AK_Insert_New_Element(TYPE_VARCHAR, parentAttNames[i], "AK_reference", "parent_attribute", row_root);
        AK_Insert_New_Element(TYPE_INT, &type, "AK_reference", "type", row_root);
        AK_insert_row(row_root);
    }

    AK_free(row_root);
    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Dejan Frankovic
 * @brief Function that reads a reference entry from system table.
 * @param name of the table with reference (with foreign key)
 * @param name of the reference constraint
 * @return AK_ref_item object with all neccessary information about the reference
 */
AK_ref_item AK_get_reference(char *tableName, char *constraintName) {
    int i = 0;

    struct list_node *list;
    AK_ref_item reference;
    AK_PRO;
    reference.attributes_number = 0;

    while ((list = AK_get_row(i, "AK_reference")) != NULL) {
        if (strncmp(AK_nth_next(list, 1)->data, tableName, MAX_ATT_NAME) == 0 &&
                strncmp(AK_nth_next(list, 2)->data, constraintName, MAX_ATT_NAME) == 0) {
            
	    safe_string_copy(reference.table, tableName, MAX_ATT_NAME);
	    safe_string_copy(reference.constraint, constraintName, MAX_ATT_NAME);
            safe_string_copy(reference.attributes[reference.attributes_number], AK_nth_next(list, 3)->data, MAX_ATT_NAME);
            safe_string_copy(reference.parent, AK_nth_next(list, 4)->data, MAX_ATT_NAME);
            safe_string_copy(reference.parent_attributes[reference.attributes_number], AK_nth_next(list, 5)->data, MAX_ATT_NAME);
            memmove(&reference.type, AK_nth_next(list, 6)->data, sizeof (int));
            reference.attributes_number++;
        }
        i++;
    }
    AK_EPI;
    return reference;
}

/**
 * @author Dejan Frankovic
 * @brief Function that checks referential integrity for one attribute
 * @param child table name
 * @param attribute name (foreign key attribute)
 * @param value of the attribute we're checking
 * @return EXIT ERROR if check failed, EXIT_SUCCESS if referential integrity is ok
 */
int AK_reference_check_attribute(char *tableName, char *attribute, char *value) {
    int i = 0;
    int att_index;

    struct list_node *list_row = NULL, *list_col = NULL;
    AK_PRO;

    while ((list_row = AK_get_row(i, "AK_reference")) != NULL) {
        if (strncmp(AK_nth_next(list_row, 1)->data, tableName, MAX_ATT_NAME) == 0 &&
                strncmp(AK_nth_next(list_row, 3)->data, attribute, MAX_ATT_NAME) == 0) {
            att_index = AK_get_attr_index(AK_nth_next(list_row, 4)->data, AK_nth_next(list_row, 5)->data);
            if (att_index < 0) {
                printf("ERROR: Attribute index not found for '%s' in table '%s'\n",
                       AK_nth_next(list_row, 5)->data, AK_nth_next(list_row, 4)->data);
                AK_EPI;
                return EXIT_ERROR;
            }
            list_col = AK_get_column(att_index, AK_nth_next(list_row, 4)->data);
            if (list_col == NULL) {
                printf("ERROR: Could not retrieve column data for table '%s'\n",
                       AK_nth_next(list_row, 4)->data);
                AK_EPI;
                return EXIT_ERROR;
            }             
            while (list_col != 0) {
		if (strncmp(list_col->data, value, MAX_VARCHAR_LENGTH) == 0) {
			AK_free(list_row);
			AK_EPI;
			return EXIT_SUCCESS;
		}
                list_col = AK_nth_next(list_col, 1);
            }
	    AK_free(list_row);
	    AK_EPI;
            return EXIT_ERROR;
        }
	AK_free(list_row);
        i++;
    }
    AK_EPI;
    return EXIT_ERROR;
}

/**
 * @author Dejan Frankovic
 * @brief Funcction that quickly checks if there are any referential constraints that should be applied on a given list of changes.
 * @param list of elements for update
 * @param is action UPDATE or DELETE ?
 * @return EXIT_SUCCESS if update is needed, EXIT_ERROR if not
 */

int AK_reference_check_if_update_needed(struct list_node *lista, int action) {

    struct list_node *temp;
    int i = 0;

    struct list_node *row;
    AK_PRO;
    while ((row = AK_get_row(i, "AK_reference")) != NULL) {
        if (strncmp(AK_nth_next(row, 4)->data, AK_nth_next(lista, 1)->table, MAX_ATT_NAME) == 0) {
	    temp = AK_First_L2(lista);
            while (temp != NULL) {
                if (action == UPDATE && temp->constraint == 0 && strncmp(AK_nth_next(row, 5)->data, temp->attribute_name, MAX_ATT_NAME) == 0){
		    AK_EPI;
                    return EXIT_SUCCESS;
		}
                else if (action == DELETE && strncmp(AK_nth_next(row, 5)->data, temp->attribute_name, MAX_ATT_NAME) == 0){
		    AK_EPI;
                    return EXIT_SUCCESS;
		}
		temp = AK_Next_L2(temp);
            }
        }
        i++;
    }
    AK_EPI;
    return EXIT_ERROR;
}

/**
 * @author Dejan Franković
 * @brief Function that checks for a REF_TYPE_RESTRICT references appliable to the operation of updating or deleting a row in a table.
 * @param list of elements for update
 * @param is action UPDATE or DELETE?
 * @return EXIT_SUCCESS if there is no restriction on this action, EXIT_ERROR if there is
 */

int AK_reference_check_restricion(struct list_node *lista, int action) {    
    int i = 0;

    struct list_node *temp;
    struct list_node *row;
    AK_PRO;
    while ((row = AK_get_row(i, "AK_reference")) != NULL) {
        if (strncmp(AK_nth_next(row, 4)->data, AK_nth_next(lista, 1)->table, MAX_ATT_NAME) == 0) {

	    temp = AK_First_L2(lista);
            while (temp != NULL) {
                if (action == UPDATE && temp->constraint == 0 && strncmp(AK_nth_next(row, 5)->data, temp->attribute_name, MAX_ATT_NAME) == 0 && *((int *) AK_nth_next(row, 6)->data) == REF_TYPE_RESTRICT){
		    AK_EPI;
                    return EXIT_ERROR;
		}
                else if (action == DELETE && strncmp(AK_nth_next(row, 5)->data, temp->attribute_name, MAX_ATT_NAME) == 0 && *((int *) AK_nth_next(row, 6)->data) == REF_TYPE_RESTRICT){
		    AK_EPI;
                    return EXIT_ERROR;
		}
		temp = AK_Next_L2(temp);
            }
        }
        i++;
    }

    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Dejan Franković
 * @brief Function that updates child table entries according to ongoing update of parent table entries.
 * @param list of elements for update
 * @param is action UPDATE or DELETE ?
 * @return EXIT_SUCCESS
 */

int AK_reference_update(struct list_node *lista, int action) {
    int parent_i, i, j, ref_i, con_num = 0;

    struct list_node *parent_row;
    struct list_node *ref_row;
    struct list_node *temp;
    struct list_node *tempcell;
    AK_ref_item reference;
    char constraints[MAX_CHILD_CONSTRAINTS][MAX_VARCHAR_LENGTH];
    char child_tables[MAX_CHILD_CONSTRAINTS][MAX_VARCHAR_LENGTH];

    char tempData[MAX_VARCHAR_LENGTH];
    AK_PRO;

    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&row_root);

    while ((ref_row = AK_get_row(ref_i, "AK_reference")) != NULL) {
        if (strncmp(AK_nth_next(ref_row, 4)->data, AK_nth_next(lista, 1)->table, MAX_ATT_NAME) == 0) { // we're searching for PARENT table here
            for (j = 0; j < con_num; j++) {
                if (strncmp(constraints[j], AK_nth_next(ref_row, 2)->data, MAX_ATT_NAME) == 0 && strncmp(child_tables[j], AK_nth_next(ref_row, 1)->data, MAX_ATT_NAME) == 0) {
                    break;
                }
            }
            if (j == con_num) {
                safe_string_copy(constraints[con_num], AK_nth_next(ref_row, 2)->data, MAX_ATT_NAME);
                safe_string_copy(child_tables[con_num], AK_nth_next(ref_row, 1)->data, MAX_ATT_NAME);
                con_num++;
            }
        }
        ref_i++;
    }

    struct list_node *expr;
    AK_Init_L3(&expr);

    // selecting only affected rows in parent table..
    i = 0;

    temp = AK_First_L2(lista);
    while (temp != NULL) {
        if (action == DELETE || temp->constraint == 1) {
	  
	    AK_InsertAtEnd_L3(TYPE_OPERAND, temp->attribute_name, strlen(temp->attribute_name), expr);
            AK_InsertAtEnd_L3(temp->type, temp->data, AK_type_size(temp->type, temp->data), expr);
            AK_InsertAtEnd_L3(TYPE_OPERATOR, "=", 1, expr);
            i++;
        }
        
	temp = AK_Next_L2(temp);
    }
    for (j = 0; j < i - 1; j++) {
    
	AK_InsertAtEnd_L3(TYPE_OPERAND, "AND", 3, expr);
    }
    char tempTable[MAX_ATT_NAME];
    sprintf(tempTable, "ref_tmp_%s", AK_nth_next(lista, 1)->table);
    
    AK_selection(AK_nth_next(lista, 1)->table, tempTable, expr);
    
    AK_DeleteAll_L3(&expr);

    AK_print_table(tempTable);

    // browsing through affected rows..
    while ((parent_row = AK_get_row(parent_i, tempTable)) != NULL) {
        for (i = 0; i < con_num; i++) {
            reference = AK_get_reference(child_tables[i], constraints[i]);
            
	    AK_DeleteAll_L3(&row_root);

            for (j = 0; j < reference.attributes_number; j++) {
		 tempcell = AK_GetNth_L2(AK_get_attr_index(reference.parent, reference.parent_attributes[j]), parent_row); // from the row of parent table, take the value of attribute with name from parent_attribute
	      
                memmove(tempData, tempcell->data, tempcell->size);
                tempData[tempcell->size] = '\0';

                AK_Update_Existing_Element(tempcell->type, tempData, reference.table, reference.attributes[j], row_root);

                switch (reference.type) {
                    case REF_TYPE_CASCADE:
                        if (action == UPDATE) {
                            
			    temp = AK_First_L2(lista);
                            while (temp != NULL) {
                                if (strncmp(temp->attribute_name, reference.parent_attributes[j], MAX_ATT_NAME) == 0 && temp->constraint == 0) {
                                    memmove(tempData, tempcell->data, tempcell->size);
                                    //tempData[tempcell->size] == '\0';
                                    AK_Insert_New_Element(tempcell->type, tempData, reference.table, reference.attributes[j], row_root);
                                    break;
                                }

				temp = AK_Next_L2(temp);
                            }
                        }
                        break;
                    case REF_TYPE_NO_ACTION:
                        break;
                        // case REF_TYPE_RESTRICT: // handled by function AK_reference_check_restrict
                    case REF_TYPE_SET_DEFAULT: // default values are currently unsupported by this DBMS... reserved for future use
                        break;
                    case REF_TYPE_SET_NULL:
                        if (action == DELETE) {
                            AK_Insert_New_Element(0, "", reference.table, reference.attributes[j], row_root);
                        } else {

			  temp = AK_First_L2(lista);
                            while (temp != NULL) {
                                if (strncmp(temp->attribute_name, reference.parent_attributes[j], MAX_ATT_NAME) == 0 && temp->constraint == 0) {
                                    AK_Insert_New_Element(0, "", reference.table, reference.attributes[j], row_root);
                                    break;
                                }

				temp = AK_Next_L2(temp);
                            }
                        }
                        break;
                }
            }

            //InsertNewElementForUpdate(tempcell->type, "" ,ref_row->next->data, ref_row->next->next->next->data, row_root,0);
            if (action == UPDATE || reference.type == REF_TYPE_SET_NULL)
                AK_update_row(row_root);
            else
                AK_delete_row(row_root);

        }

        parent_i++;
    }

    AK_delete_segment(tempTable, SEGMENT_TYPE_TABLE);
    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Dejan Franković
 * @brief Function that checks a new entry for referential integrity.
 * @param list of elements for insert row
 * @return EXIT_SUCCESS if referential integrity is ok, EXIT_ERROR if it is compromised
 */
int AK_reference_check_entry(struct list_node *lista) {
    
    struct list_node *temp, *row, *temp1;
    int i = 0, j, k, con_num = 0, success;
    char constraints[10][MAX_VARCHAR_LENGTH]; // this 10 should probably be a constant... how many foreign keys can one table have..
    char attributes[MAX_REFERENCE_ATTRIBUTES][MAX_ATT_NAME];
    int is_att_null[MAX_REFERENCE_ATTRIBUTES]; //this is a workaround... when proper null value implementation is in place, this should be solved differently
    
    AK_ref_item reference;

    AK_PRO;

    temp = AK_First_L2(lista);
    while (temp != NULL) {
        if (temp->constraint == 1){
	    AK_EPI;
            return EXIT_SUCCESS;
        }

	temp = AK_Next_L2(temp);
    }

    while ((row = AK_get_row(i, "AK_reference")) != NULL)
        {
        if (strncmp(AK_nth_next(row, 1)->data, AK_nth_next(lista, 1)->table, MAX_ATT_NAME) == 0)
                {
            for (j = 0; j < con_num; j++)
                        {
                if (strncmp(constraints[j], AK_nth_next(row, 2)->data, MAX_ATT_NAME) == 0)
                                {
                    break;
                }
            }
            if (j == con_num)
                        {
                safe_string_copy(constraints[con_num], AK_nth_next(row, 2)->data, MAX_ATT_NAME);
                con_num++;
            }
        }
        i++;
                AK_free(row);
    }


    if (con_num == 0){
	AK_EPI;
        return EXIT_SUCCESS;
    }

    for (i = 0; i < con_num; i++) { // reference
        reference = AK_get_reference(AK_nth_next(lista, 1)->table, constraints[i]);

        // fetching relevant attributes from entry list...
        // attributes = AK_malloc(sizeof(char)*MAX_VARCHAR_LENGHT*reference.attributes_number);
        for (j = 0; j < reference.attributes_number; j++) {
            temp = AK_nth_next(lista, 1);
            while (temp != NULL) {

                if (temp->constraint == 0 && strncmp(temp->attribute_name, reference.attributes[j], MAX_ATT_NAME) == 0) {
                    safe_string_copy(attributes[j], temp->data, MAX_VARCHAR_LENGTH);
                    if (reference.type == REF_TYPE_SET_NULL && temp->data[0] == '\0') //if type is 0, the value is PROBABLY null
                        is_att_null[j] = 1;
                    else
                        is_att_null[j] = 0;
                    break;
                }
//                temp = AK_Next_L2(temp);
		temp = AK_Next_L2(temp);
            }
        }

        if (reference.attributes_number == 1) {
            if (AK_reference_check_attribute(reference.table, reference.attributes[0], attributes[0]) == EXIT_ERROR) {
		AK_EPI;
                return EXIT_ERROR;
            } else continue;
        }


        j = 0;
        while ((row = AK_get_row(j, reference.parent)) != NULL) { // rows in parent table
            success = 1;
            for (k = 0; k < reference.attributes_number; k++) { // attributes in reference
		temp1 = AK_GetNth_L2(AK_get_attr_index(reference.parent, reference.parent_attributes[k]), row);
                if (temp1 != 0x0) {
                  if (is_att_null[k] || strncmp(temp1->data, attributes[k], MAX_VARCHAR_LENGTH) != 0) {
                      success = 0;
                      break;
                  }
                }
            }
            if (success == 1) {
		AK_EPI;
                return EXIT_SUCCESS;
            }
            j++;
        }
    }
    AK_EPI;
    return EXIT_ERROR;
}

/**
 * @author Dejan Franković
 * @brief Function for testing referential integrity.
 * @return No return value
 */
TestResult AK_reference_test() {
    AK_PRO;
    printf("reference.c: Present!\n");

    AK_header t_header[4] ={
        {TYPE_INT, {"FK"}, {0}, {{'\0'}}, {{'\0'}}},
        {TYPE_VARCHAR, {"Value"}, {0}, {{'\0'}}, {{'\0'}}},
        {TYPE_VARCHAR, {"Rnd"}, {0}, {{'\0'}}, {{'\0'}}},
        {0, {'\0'}, {0}, {{'\0'}}, {{'\0'}}}
    };

    int startAddress = AK_initialize_new_segment("ref_test", SEGMENT_TYPE_TABLE, t_header);

    if (startAddress != EXIT_ERROR)
        printf("\nTABLE %s CREATED!\n", "ref_test");

    int a;

    char *att[2];
    att[0] = AK_malloc(sizeof (char) *20);
    safe_string_copy(att[0], "FK", MAX_ATT_NAME);
    att[1] = AK_malloc(sizeof (char) *20);
    safe_string_copy(att[1], "Value", MAX_ATT_NAME);


    char *patt[2];
    patt[0] = AK_malloc(sizeof (char) *20);
    safe_string_copy(patt[0], "mbr", MAX_ATT_NAME);
    patt[1] = AK_malloc(sizeof (char) *20);
    safe_string_copy(patt[1], "firstname", MAX_ATT_NAME);

    AK_add_reference("ref_test", att, "student", patt, 2, "constraint", REF_TYPE_SET_NULL);
    AK_print_table("AK_reference");
    AK_print_table("student");

    a = 35891;
    
    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&row_root);
    //AK_DeleteAll_L3(&row_root);
    AK_Insert_New_Element(TYPE_INT, &a, "ref_test", "FK", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "Dude", "ref_test", "Value", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "TheRippah", "ref_test", "Rnd", row_root);
    AK_insert_row(row_root);

    a = 35891;
    
      
    AK_DeleteAll_L3(row_root);
    AK_Insert_New_Element(TYPE_INT, &a, "ref_test", "FK", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "Mislav", "ref_test", "Value", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "TheMutilator", "ref_test", "Rnd", row_root);
    AK_insert_row(row_root);

    a = 35893;
      
    AK_DeleteAll_L3(row_root);
    AK_Insert_New_Element(TYPE_INT, &a, "ref_test", "FK", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "Mislav", "ref_test", "Value", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "TheMutilator", "ref_test", "Rnd", row_root);
    AK_insert_row(row_root);

    AK_print_table("ref_test");

    AK_print_table("student");
    AK_print_table("ref_test");

    // Free dynamically allocated attribute arrays and delete all temporary rows
    // to ensure memory is properly released after running the reference integrity test.    
    AK_DeleteAll_L3(&row_root);
    AK_free(att[0]);
    AK_free(att[1]);
    AK_free(patt[0]);
    AK_free(patt[1]);	

    AK_EPI;
    return TEST_result(0,1);
}

// ====================== TEST FUNCTIONS =========================
/**

TestResult test_AK_reference_check_attribute() {
    AK_PRO;
    printf("Running test: AK_reference_check_attribute...\n");

    int broj = 35891;
    AK_Insert_New_Element(TYPE_INT, &broj, "student", "mbr", NULL);

    int result = AK_reference_check_attribute("student", "mbr", "35891");
    if (result == EXIT_SUCCESS) {
        printf("Test passed: Value found.\n");
    } else {
        printf("Test failed: Value not found (should exist).\n");
        return TEST_result(1, 1);  // fail
    }

    result = AK_reference_check_attribute("student", "mbr", "99999");
    if (result == EXIT_ERROR) {
        printf("Test passed: Value not found as expected.\n");
    } else {
        printf("Test failed: Unexpected success for non-existent value.\n");
        return TEST_result(1, 1);  // fail
    }

    AK_EPI;
    return TEST_result(0, 2); // all passed
}


TestResult test_AK_reference_check_entry() {
    AK_PRO;
    printf("Running test: AK_reference_check_entry...\n");
        
    char *child_attrs[] = {"FK"};
    char *parent_attrs[] = {"mbr"};
    AK_add_reference("ref_test", child_attrs, "student", parent_attrs, 1, "FK_constraint", REF_TYPE_RESTRICT);

    int a = 35891;
    AK_Insert_New_Element(TYPE_INT, &a, "student", "mbr", NULL);

    struct list_node *row = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row);

    AK_Insert_New_Element(TYPE_INT, &a, "ref_test", "FK", row);
    AK_Insert_New_Element(TYPE_VARCHAR, "Dude", "ref_test", "Value", row);
    AK_Insert_New_Element(TYPE_VARCHAR, "Something", "ref_test", "Rnd", row);

    int result = AK_reference_check_entry(row);
    if (result == EXIT_SUCCESS) {
        printf("Test passed: Referential integrity OK.\n");
    } else {
        printf("Test failed: Referential integrity check failed (should pass).\n");
        return TEST_result(1, 1);
    }
    AK_DeleteAll_L3(&row);

    struct list_node *row2 = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row2);    

    int wrong = 99999;
    AK_Insert_New_Element(TYPE_INT, &wrong, "ref_test", "FK", row2);
    AK_Insert_New_Element(TYPE_VARCHAR, "Wrong", "ref_test", "Value", row2);
    AK_Insert_New_Element(TYPE_VARCHAR, "Nope", "ref_test", "Rnd", row2);

    result = AK_reference_check_entry(row2);
    if (result == EXIT_ERROR) {
        printf("Test passed: Referential integrity violated as expected.\n");
    } else {
        printf("Test failed: Referential integrity falsely passed.\n");
        return TEST_result(1, 1);
    }

    AK_DeleteAll_L3(&row2);
    AK_EPI;
    return TEST_result(0, 2);
}

TestResult test_AK_reference_check_restriction() {
    AK_PRO;
    printf("Running test: AK_reference_check_restriction...\n");
    
    char *child_attrs[] = {"FK"};
    char *parent_attrs[] = {"mbr"};
    AK_add_reference("ref_test", child_attrs, "student", parent_attrs, 1, "FK_constraint", REF_TYPE_RESTRICT);

    int a = 35891;
    AK_Insert_New_Element(TYPE_INT, &a, "student", "mbr", NULL);
    AK_insert_row(NULL);
    struct list_node *row = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row);

    AK_Insert_New_Element(TYPE_INT, &a, "ref_test", "FK", row);
    AK_Insert_New_Element(TYPE_VARCHAR, "Dude", "ref_test", "Value", row);
    AK_Insert_New_Element(TYPE_VARCHAR, "X", "ref_test", "Rnd", row);
    
    AK_insert_row(row);

    int result = AK_reference_check_restricion(row, DELETE);
    if (result == EXIT_ERROR) {
        printf("Test passed: DELETE restriction correctly detected.\n");
    } else {
        printf("Test failed: DELETE restriction not detected.\n");
        return TEST_result(1, 1);
    }

    result = AK_reference_check_restricion(row, UPDATE);
    if (result == EXIT_ERROR || result == EXIT_SUCCESS) {
        printf("Test passed: UPDATE restriction checked without crash.\n");
    } else {
        printf("Test failed: Unexpected result for UPDATE.\n");
        return TEST_result(1, 1);
    }

    AK_DeleteAll_L3(&row);
    AK_EPI;
    return TEST_result(0, 2);
}

TestResult test_AK_reference_check_if_update_needed() {
    AK_PRO;
    printf("Running test: AK_reference_check_if_update_needed...\n");
    
    char *child_attrs[] = {"FK"};
    char *parent_attrs[] = {"mbr"};
    AK_add_reference("ref_test", child_attrs, "student", parent_attrs, 1, "FK_constraint", REF_TYPE_RESTRICT);

    int a = 35891;
    AK_Insert_New_Element(TYPE_INT, &a, "student", "mbr", NULL);
    
    struct list_node *row = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row);
    row->constraint = 1;
    AK_Insert_New_Element(TYPE_INT, &a, "ref_test", "FK", row); 
    AK_Insert_New_Element(TYPE_VARCHAR, "Dude", "ref_test", "Value", row);
    AK_Insert_New_Element(TYPE_VARCHAR, "Test", "ref_test", "Rnd", row);

    int result = AK_reference_check_if_update_needed(row, UPDATE);

    if (result == EXIT_SUCCESS || result == EXIT_ERROR) {
        printf("Test passed: update check completed (result = %d)\n", result);
    } else {
        printf("Test failed: Unexpected result (%d)\n", result);
        return TEST_result(1, 1);
    }

    AK_DeleteAll_L3(&row);
    AK_EPI;
    return TEST_result(0, 1);
}


TestResult test_AK_reference_update() {
    AK_PRO;
    printf("Running test: AK_reference_update...\n");
   
    char *child_attrs[] = {"FK"};
    char *parent_attrs[] = {"mbr"};
    AK_add_reference("ref_test", child_attrs, "student", parent_attrs, 1, "FK_constraint", REF_TYPE_RESTRICT);

    int a = 1;
    AK_Insert_New_Element(TYPE_INT, &a, "parent", "id", NULL);

    struct list_node *lista = AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&lista);
    AK_Insert_New_Element(TYPE_INT, &a, "ref_test", "FK", lista);
    lista->constraint = 0;

    int result = AK_reference_update(lista, UPDATE);

    AK_DeleteAll_L3(&lista);

    if (result == EXIT_SUCCESS) {
        printf("Test passed: AK_reference_update returned EXIT_SUCCESS.\n");
    } else {
        printf("Test failed: AK_reference_update returned %d.\n", result);
    }

    AK_EPI;
    return TEST_result(result == EXIT_SUCCESS, 1);
}


TestResult test_AK_get_reference() {
    AK_PRO;
    printf("Running test: AK_get_reference...\n");

    char *child_attrs[] = {"FK"};
    char *parent_attrs[] = {"mbr"};
    AK_add_reference("ref_test", child_attrs, "student", parent_attrs, 1, "FK_constraint", REF_TYPE_RESTRICT);


    AK_ref_item ref = AK_get_reference("ref_test", "constraint");

    int valid = (strncmp(ref.table, "ref_test", MAX_ATT_NAME) == 0) &&
                (strncmp(ref.constraint, "constraint", MAX_ATT_NAME) == 0) &&
                (ref.attributes_number > 0);

    if (valid) {
        printf("Test passed: AK_get_reference returned valid structure.\n");
    } else {
        printf("Test failed: Invalid reference structure.\n");
    }

    AK_EPI;
    return TEST_result(valid, 1);
}


void AK_run_all_reference_tests() {
   // test_AK_reference_check_if_update_needed();
   // test_AK_reference_check_restriction();
    test_AK_reference_check_entry();
    test_AK_reference_check_attribute();
    test_AK_get_reference();
    test_AK_reference_update();
}
**/



