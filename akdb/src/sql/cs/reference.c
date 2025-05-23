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
        if (strcmp(list->next->data, tableName) == 0 &&
                strcmp(list->next->next->data, constraintName) == 0) {
            strcpy(reference.table, tableName);
            strcpy(reference.constraint, constraintName);
            strcpy(reference.attributes[reference.attributes_number], list->next->next->next->data);
            strcpy(reference.parent, list->next->next->next->next->data);
            strcpy(reference.parent_attributes[reference.attributes_number], list->next->next->next->next->next->data);
            memcpy(&reference.type, list->next->next->next->next->next->next->data, sizeof (int));
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
    AK_PRO;

    // Scan system table for the FK constraint on (tableName, attribute)
    struct list_node *r;
    while ((r = AK_get_row(i++, "AK_reference")) != NULL) {
        if (strcmp(r->next->data, tableName) == 0 &&
            strcmp(r->next->next->next->data, attribute) == 0)
        {
            // Found the constraint row; look in the parent table
            char * parentTable = r->next->next->next->next->data;
            char * parentAttr  = r->next->next->next->next->next->data;
            int idx = AK_get_attr_index(parentTable, parentAttr);
            struct list_node *c = AK_get_column(idx, parentTable);

            // Walk the parent‐column until we find a match (or run out)
            while (c) {
                // Handle different data types appropriately
                if (c->type == TYPE_INT) {
                    int parent_val;
                    memcpy(&parent_val, c->data, sizeof(int));
                    int check_val = atoi(value);
                    if (parent_val == check_val) {
                        AK_EPI;
                        return EXIT_SUCCESS;
                    }
                }
                else if (c->type == TYPE_VARCHAR) {
                    if (strcmp(c->data, value) == 0) {
                        AK_EPI;
                        return EXIT_SUCCESS;
                    }
                }
                c = c->next;
            }
            AK_EPI;
            return EXIT_ERROR;
        }
    }

    // No FK constraint found on that attribute => error
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
        if (strcmp(row->next->next->next->next->data, lista->next->table) == 0) {
	    temp = AK_First_L2(lista);
            while (temp != NULL) {
                if (action == UPDATE && temp->constraint == 0 && strcmp(row->next->next->next->next->next->data, temp->attribute_name) == 0){
		    AK_EPI;
                    return EXIT_SUCCESS;
		}
                else if (action == DELETE && strcmp(row->next->next->next->next->next->data, temp->attribute_name) == 0){
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
        if (strcmp(row->next->next->next->next->data, lista->next->table) == 0) {

	    temp = AK_First_L2(lista);
            while (temp != NULL) {
                if (action == UPDATE && temp->constraint == 0 && memcmp(row->next->next->next->next->next->data, temp->attribute_name, row->next->next->next->next->next->size) == 0 && (int) * row->next->next->next->next->next->next->data == REF_TYPE_RESTRICT){
		    AK_EPI;
                    return EXIT_ERROR;
		}
                else if (action == DELETE && memcmp(row->next->next->next->next->next->data, temp->attribute_name, row->next->next->next->next->next->size) == 0 && (int) * row->next->next->next->next->next->next->data == REF_TYPE_RESTRICT){
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
    int parent_i = 0;
    int ref_i    = 0;
    int i        = 0;
    int j        = 0;
    int con_num  = 0;

    AK_PRO;

    // 1) Collect all child constraints where lista->next->table is the PARENT
    struct list_node *ref_row = NULL;
    char constraints[MAX_CHILD_CONSTRAINTS][MAX_VARCHAR_LENGTH];
    char child_tables[MAX_CHILD_CONSTRAINTS][MAX_VARCHAR_LENGTH];

    while ((ref_row = AK_get_row(ref_i++, "AK_reference")) != NULL) {
        if (strcmp(ref_row->next->next->next->next->data,
                   lista->next->table) == 0) {
            // dedupe on (constraint name + child table)
            for (j = 0; j < con_num; j++) {
                if (strcmp(constraints[j], ref_row->next->next->data) == 0 &&
                    strcmp(child_tables[j], ref_row->next->data) == 0) {
                    break;
                }
            }
            if (j == con_num) {
                strcpy(constraints[con_num], ref_row->next->next->data);
                strcpy(child_tables[con_num], ref_row->next->data);
                con_num++;
            }
        }
    }

    // 2) If no child constraints apply, short-circuit:
    //    UPDATE: nothing to do -> success
    //    DELETE: cannot enforce cascade -> error
    if (con_num == 0) {
        AK_EPI;
        return (action == UPDATE ? EXIT_SUCCESS : EXIT_ERROR);
    }

    // 3) Allocate and init lists for selection predicate and row batching
    struct list_node *expr = (struct list_node *) AK_malloc(sizeof *expr);
    AK_Init_L3(&expr);

    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof *row_root);
    AK_Init_L3(&row_root);

    // 4) Build predicate: find affected parent rows
    struct list_node *temp = AK_First_L2(lista);
    while (temp) {
        if (action == DELETE || temp->constraint == 1) {
            AK_InsertAtEnd_L3(TYPE_OPERAND, temp->attribute_name,
                              strlen(temp->attribute_name), expr);
            AK_InsertAtEnd_L3(temp->type, temp->data,
                              AK_type_size(temp->type, temp->data), expr);
            AK_InsertAtEnd_L3(TYPE_OPERATOR, "=", 1, expr);
            i++;
        }
        temp = AK_Next_L2(temp);
    }
    for (j = 0; j < i - 1; j++) {
        AK_InsertAtEnd_L3(TYPE_OPERAND, "AND", 3, expr);
    }

    // 5) Materialize into a temporary table
    char tempTable[MAX_ATT_NAME];
    sprintf(tempTable, "ref_tmp_%s", lista->next->table);
    AK_selection(lista->next->table, tempTable, expr);

    AK_DeleteAll_L3(&expr);
    AK_free(expr);

    AK_print_table(tempTable);

    // 6) For each affected parent row, apply updates/deletes to children
    struct list_node *parent_row = NULL;
    char tempData[MAX_VARCHAR_LENGTH];
    for (parent_i = 0; (parent_row = AK_get_row(parent_i, tempTable)) != NULL; parent_i++) {
        for (i = 0; i < con_num; i++) {
            AK_ref_item reference = AK_get_reference(child_tables[i], constraints[i]);
            AK_DeleteAll_L3(&row_root);

            for (j = 0; j < reference.attributes_number; j++) {
                struct list_node *tempcell = AK_GetNth_L2(
                    AK_get_attr_index(reference.parent, reference.parent_attributes[j]),
                    parent_row
                );
                memcpy(tempData, tempcell->data, tempcell->size);
                tempData[tempcell->size] = '\0';

                // preserve existing child PK/FK
                AK_Update_Existing_Element(
                    tempcell->type, tempData,
                    reference.table, reference.attributes[j],
                    row_root
                );

                switch (reference.type) {
                    case REF_TYPE_CASCADE:
                        if (action == UPDATE) {
                            struct list_node *tt = AK_First_L2(lista);
                            while (tt) {
                                if (strcmp(tt->attribute_name,
                                           reference.parent_attributes[j]) == 0 &&
                                    tt->constraint == 0) {
                                    memcpy(tempData, tempcell->data, tempcell->size);
                                    tempData[tempcell->size] = '\0';
                                    AK_Insert_New_Element(
                                        tempcell->type,
                                        tempData,
                                        reference.table,
                                        reference.attributes[j],
                                        row_root
                                    );
                                    break;
                                }
                                tt = AK_Next_L2(tt);
                            }
                        }
                        break;

                    case REF_TYPE_NO_ACTION:
                    case REF_TYPE_SET_DEFAULT:
                        break;

                    case REF_TYPE_SET_NULL:
                        if (action == DELETE) {
                            AK_Insert_New_Element(0, "",
                                                  reference.table,
                                                  reference.attributes[j],
                                                  row_root);
                        } else {
                            struct list_node *tt = AK_First_L2(lista);
                            while (tt) {
                                if (strcmp(tt->attribute_name,
                                           reference.parent_attributes[j]) == 0 &&
                                    tt->constraint == 0) {
                                    AK_Insert_New_Element(0, "",
                                                          reference.table,
                                                          reference.attributes[j],
                                                          row_root);
                                    break;
                                }
                                tt = AK_Next_L2(tt);
                            }
                        }
                        break;
                }
            }

            // execute update or delete on child rows
            if (action == UPDATE || reference.type == REF_TYPE_SET_NULL) {
                AK_update_row(row_root);
            } else {
                AK_delete_row(row_root);
            }
        }
    }

    // 7) Cleanup
    AK_delete_segment(tempTable, SEGMENT_TYPE_TABLE);
    AK_DeleteAll_L3(&row_root);
    AK_free(row_root);

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
        if (strcmp(row->next->data, lista->next->table) == 0) 
		{
            for (j = 0; j < con_num; j++) 
			{
                if (strcmp(constraints[j], row->next->next->data) == 0) 
				{
                    break;
                }
            }
            if (j == con_num) 
			{
                strcpy(constraints[con_num], row->next->next->data);
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
        reference = AK_get_reference(lista->next->table, constraints[i]);

        // fetching relevant attributes from entry list...
        // attributes = AK_malloc(sizeof(char)*MAX_VARCHAR_LENGHT*reference.attributes_number);
        for (j = 0; j < reference.attributes_number; j++) {
            temp = lista->next;
            while (temp != NULL) {

                if (temp->constraint == 0 && strcmp(temp->attribute_name, reference.attributes[j]) == 0) {
                    strcpy(attributes[j], temp->data);
                    if (reference.type == REF_TYPE_SET_NULL && strcmp(temp->data, "\0") == 0) //if type is 0, the value is PROBABLY null
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
                  if (is_att_null[k] || strcmp(temp1->data, attributes[k]) != 0) {
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
    int passing_tests = 0;
    int failing_tests = 0;
    printf("reference.c: Starting tests!\n");

    // Test setup - create test tables
    AK_header t_header[4] = {
        {TYPE_INT, {"FK"}, {0}, {{'\0'}}, {{'\0'}}},
        {TYPE_VARCHAR, {"Value"}, {0}, {{'\0'}}, {{'\0'}}},
        {TYPE_VARCHAR, {"Rnd"}, {0}, {{'\0'}}, {{'\0'}}},
        {0, {'\0'}, {0}, {{'\0'}}, {{'\0'}}}
    };

    // Create test table
    int startAddress = AK_initialize_new_segment("ref_test", SEGMENT_TYPE_TABLE, t_header);
    if (startAddress != EXIT_ERROR) {
        printf("\nTest table 'ref_test' created successfully\n");
        passing_tests++;
    } else {
        printf("\nFAILED: Could not create test table 'ref_test'\n");
        failing_tests++;
        return TEST_result(passing_tests, failing_tests);
    }

    // Test 1: AK_add_reference - Valid case
    char *att1[2];
    att1[0] = AK_malloc(sizeof(char) * 20);
    strcpy(att1[0], "FK");
    att1[1] = AK_malloc(sizeof(char) * 20);
    strcpy(att1[1], "Value");

    char *patt1[2];
    patt1[0] = AK_malloc(sizeof(char) * 20);
    strcpy(patt1[0], "mbr");
    patt1[1] = AK_malloc(sizeof(char) * 20);
    strcpy(patt1[1], "firstname");

    if (AK_add_reference("ref_test", att1, "student", patt1, 2, "constraint1", REF_TYPE_SET_NULL) == EXIT_SUCCESS) {
        printf("Test 1 PASSED: Successfully added valid reference\n");
        passing_tests++;
    } else {
        printf("Test 1 FAILED: Could not add valid reference\n");
        failing_tests++;
    }

    // Test 2: AK_add_reference - Invalid case (invalid reference type)
    char *att2[2];
    att2[0] = AK_malloc(sizeof(char) * 20);
    strcpy(att2[0], "FK");
    att2[1] = AK_malloc(sizeof(char) * 20);
    strcpy(att2[1], "Value");

    char *patt2[2];
    patt2[0] = AK_malloc(sizeof(char) * 20);
    strcpy(patt2[0], "id");
    patt2[1] = AK_malloc(sizeof(char) * 20);
    strcpy(patt2[1], "name");

    if (AK_add_reference("ref_test", att2, "employee", patt2, 2, "constraint2", 999) == 0) {
        printf("Test 2 PASSED: Invalid reference type correctly rejected\n");
        passing_tests++;
    } else {
        printf("Test 2 FAILED: Invalid reference type not rejected\n");
        failing_tests++;
    }

    // Test 3: AK_get_reference - Valid case
    AK_ref_item ref3 = AK_get_reference("ref_test", "constraint1");
    if (ref3.attributes_number > 0 && strcmp(ref3.table, "ref_test") == 0) {
        printf("Test 3 PASSED: Successfully retrieved reference\n");
        passing_tests++;
    } else {
        printf("Test 3 FAILED: Could not retrieve existing reference\n");
        failing_tests++;
    }

    // Test 4: AK_get_reference - Invalid case
    AK_ref_item ref4 = AK_get_reference("non_existent", "non_existent");
    if (ref4.attributes_number == 0) {
        printf("Test 4 PASSED: Non-existent reference returns empty result\n");
        passing_tests++;
    } else {
        printf("Test 4 FAILED: Non-existent reference returned data\n");
        failing_tests++;
    }

    // Test 5: AK_reference_check_attribute - Valid case
    int test5_val = 35891;
    char value5[20];
    sprintf(value5, "%d", test5_val);
    
    struct list_node *row_root5 = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row_root5);
    AK_DeleteAll_L3(&row_root5);
    AK_Insert_New_Element(TYPE_INT, &test5_val, "ref_test", "FK", row_root5);
    AK_Insert_New_Element(TYPE_VARCHAR, "Test5", "ref_test", "Value", row_root5);
    AK_Insert_New_Element(TYPE_VARCHAR, "Test5Rnd", "ref_test", "Rnd", row_root5);
    AK_insert_row(row_root5);

    if (AK_reference_check_attribute("ref_test", "FK", value5) == EXIT_SUCCESS) {
        printf("Test 5 PASSED: Valid attribute check passed\n");
        passing_tests++;
    } else {
        printf("Test 5 FAILED: Valid attribute check failed\n");
        failing_tests++;
    }

    // Test 6: AK_reference_check_attribute - Invalid case
    const char *invalid_value6 = "88888";
    if (AK_reference_check_attribute("ref_test", "FK", invalid_value6) == EXIT_ERROR) {
        printf("Test 6 PASSED: Invalid attribute correctly rejected\n");
        passing_tests++;
    } else {
        printf("Test 6 FAILED: Invalid attribute not rejected\n");
        failing_tests++;
    }

    // Test 7: AK_reference_check_if_update_needed - Valid case
    int test7_val = 45678;
    struct list_node *update_list7 = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&update_list7);
    AK_DeleteAll_L3(&update_list7);
    AK_Insert_New_Element(TYPE_INT, &test7_val, "student", "mbr", update_list7);
    
    if (AK_reference_check_if_update_needed(update_list7, UPDATE) == EXIT_SUCCESS) {
        printf("Test 7 PASSED: Update check successful\n");
        passing_tests++;
    } else {
        printf("Test 7 FAILED: Update check failed\n");
        failing_tests++;
    }

    // Test 8: AK_reference_check_restricion - SET_NULL case
    int test8_val = 56789;
    struct list_node *update_list8 = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&update_list8);
    AK_DeleteAll_L3(&update_list8);
    AK_Insert_New_Element(TYPE_INT, &test8_val, "student", "mbr", update_list8);
    
    if (AK_reference_check_restricion(update_list8, UPDATE) == EXIT_SUCCESS) {
        printf("Test 8 PASSED: Restriction check passed for SET_NULL\n");
        passing_tests++;
    } else {
        printf("Test 8 FAILED: Restriction check failed for SET_NULL\n");
        failing_tests++;
    }

    // Test 9: AK_reference_check_entry - Valid case
    struct list_node *stu9 = (struct list_node*)AK_malloc(sizeof *stu9);
    AK_Init_L3(&stu9);
    int student_id9 = 67890;
    AK_Insert_New_Element(TYPE_INT, &student_id9, "student", "mbr", stu9);
    AK_Insert_New_Element(TYPE_VARCHAR, "Test9", "student", "firstname", stu9);
    AK_insert_row(stu9);

    if (AK_reference_check_entry(stu9) == EXIT_SUCCESS) {
        printf("Test 9 PASSED: Entry check passed for valid data\n");
        passing_tests++;
    } else {
        printf("Test 9 FAILED: Entry check failed for valid data\n");
        failing_tests++;
    }

    // Test 10: AK_reference_check_entry - Invalid case
    struct list_node *row_root10 = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row_root10);
    AK_DeleteAll_L3(&row_root10);
    int test10_val = 77777;
    AK_Insert_New_Element(TYPE_INT, &test10_val, "ref_test", "FK", row_root10);
    AK_Insert_New_Element(TYPE_VARCHAR, "Invalid10", "ref_test", "Value", row_root10);
    
    if (AK_reference_check_entry(row_root10) == EXIT_ERROR) {
        printf("Test 10 PASSED: Invalid entry correctly rejected\n");
        passing_tests++;
    } else {
        printf("Test 10 FAILED: Invalid entry not rejected\n");
        failing_tests++;
    }

    // Test 11: AK_reference_update - Valid case
    struct list_node *update_test_list11 = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&update_test_list11);
    int test11_val = 88888;
    AK_Insert_New_Element(TYPE_INT, &test11_val, "ref_test", "FK", update_test_list11);
    AK_Insert_New_Element(TYPE_VARCHAR, "UpdateTest11", "ref_test", "Value", update_test_list11);

    if(AK_reference_update(update_test_list11, UPDATE) == EXIT_SUCCESS) {
        printf("Test 11 PASSED: Update operation successful\n");
        passing_tests++;
    } else {
        printf("Test 11 FAILED: Update operation failed\n");
        failing_tests++;
    }

    // Test 12: AK_reference_update - Invalid case
    struct list_node *update_test_list12 = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&update_test_list12);
    int test12_val = 99999;
    AK_Insert_New_Element(TYPE_INT, &test12_val, "ref_test", "FK", update_test_list12);
    AK_Insert_New_Element(TYPE_VARCHAR, "UpdateTest12", "ref_test", "Value", update_test_list12);

    if(AK_reference_update(update_test_list12, DELETE) == EXIT_ERROR) {
        printf("Test 12 PASSED: Delete operation correctly rejected\n");
        passing_tests++;
    } else {
        printf("Test 12 FAILED: Delete operation not rejected\n");
        failing_tests++;
    }

    // Cleanup
    AK_free(att1[0]); AK_free(att1[1]);
    AK_free(patt1[0]); AK_free(patt1[1]);
    AK_free(att2[0]); AK_free(att2[1]);
    AK_free(patt2[0]); AK_free(patt2[1]);
    AK_free(row_root5);
    AK_DeleteAll_L3(&update_list7);
    AK_free(update_list7);
    AK_DeleteAll_L3(&update_list8);
    AK_free(update_list8);
    AK_DeleteAll_L3(&stu9);
    AK_free(stu9);
    AK_DeleteAll_L3(&row_root10);
    AK_free(row_root10);
    AK_DeleteAll_L3(&update_test_list11);
    AK_free(update_test_list11);
    AK_DeleteAll_L3(&update_test_list12);
    AK_free(update_test_list12);

    printf("\nTEST SUMMARY:\n");
    printf("Passing tests: %d\n", passing_tests);
    printf("Failing tests: %d\n", failing_tests);
    
    AK_EPI;
    return TEST_result(passing_tests, failing_tests);
}
