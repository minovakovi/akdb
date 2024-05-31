/**
 * @file check_constraint.c Check constraint implementation file.
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

#include "check_constraint.h"
#include "../drop.h"

/**
 * @author Mislav Jurinić
 * @brief Function that for a given value, checks if it satisfies the "check" constraint.
 * @param condition logical operator ['<', '>', '!=', ...]
 * @param type data type [int, float, varchar, datetime, ...]
 * @param value condition to be set
 * @param row_data data in table
 * @return 1 - result, 0 - failure 
 */
int condition_passed(char *condition, int type, void *value, void *row_data) {
    float float_value;
    float float_row_data;

    AK_PRO;

    switch (type) {
        case TYPE_INT:
            value = (int) value;
            row_data = *((int *) row_data);

            // DEBUG
            // printf("XCondition: %s\n", condition);
            // printf("Existing Entry: %d\n", row_data);
            // printf("Condition Value: %d\n", value);

            if (!strcmp(condition, "=")) {
                return value == row_data;
            }
            else if (!strcmp(condition, ">")) {
                return row_data > value;
            }
            else if (!strcmp(condition, "<")) {
                return row_data < value;
            }
            else if (!strcmp(condition, ">=")) {
                return row_data >= value;
            }
            else if (!strcmp(condition, "<=")) {
                return row_data <= value;
            }
            else if (!strcmp(condition, "!=")) {
                return row_data != value;
            }
	    else if (!strcmp(condition, "&&")) {
                return row_data && value;
            }
            else if (!strcmp(condition, "||")) {
                return row_data || value;
            }

            break;

        case TYPE_FLOAT:
            float_value = *(float *) value;
            float_row_data = *((float *) row_data);

            // DEBUG
            // printf("YCondition: %s\n", condition);
            // printf("Condition Value: %f\n", float_value);
            // printf("Existing Entry: %f\n", float_row_data);

            if (!strcmp(condition, "=")) {
                return float_value == float_row_data;
            }
            else if (!strcmp(condition, ">")) {
                return float_row_data > float_value;
            }
            else if (!strcmp(condition, "<")) {
                return float_row_data < float_value;
            }
            else if (!strcmp(condition, ">=")) {
                return float_row_data >= float_value;
            }
            else if (!strcmp(condition, "<=")) {
                return float_row_data <= float_value;
            }
            else if (!strcmp(condition, "!=")) {
                return float_row_data != float_value;
            }
	    else if (!strcmp(condition, "&&")) {
                return float_row_data && float_value;
            }
            else if (!strcmp(condition, "||")) {
                return float_row_data || float_value;
            }

            break;

        default:
            value = (char *) value;
            row_data = (char *) row_data;

            // DEBUG
            // printf("ZCondition: %s\n", condition);
            // printf("Condition Value: %s\n", value);
            // printf("Existing Entry: %s\n", row_data);

            if (!strcmp(condition, "=")) {
                return !strcmp(value, row_data);
            }
            else if (!strcmp(condition, ">")) {
                return strcmp(value, row_data) < 0;
            }
            else if (!strcmp(condition, "<")) {
                return strcmp(value, row_data) > 0;
            }
            else if (!strcmp(condition, "!=")) {
                return strcmp(value, row_data) != 0;
            }
	    else if (!strcmp(condition, "&&")) {
                return strcmp(value, row_data) && 0;
            }
            else if (!strcmp(condition, "||")) {
                return strcmp(value, row_data) || 0;
            }

            break;
    }

    AK_EPI;

    return EXIT_ERROR;
}

/**
 * @author Mislav Jurinić
 * @brief Function that adds a new "check" constraint into the system table.
 * @param table_name target table for "check" constraint evaluation
 * @param constraint_name new "check" constraint name that will be visible in the system table
 * @param attribute_name target attribute for "check" constraint evaluation
 * @param condition logical operator ['<', '>', '!=', ...]
 * @param type data type [int, float, varchar, datetime, ...]
 * @param value condition to be set
 * @return 1 - result, 0 - failure 
 */
int AK_set_check_constraint(char *table_name, char *constraint_name, char *attribute_name, char *condition, int type, void *value) {
    int i;
    int attribute_position;
    int num_rows = AK_get_num_records(table_name);
    struct list_node *row;
    struct list_node *attribute;
    void *data = (void *) AK_calloc(MAX_VARCHAR_LENGTH, sizeof (void));

    AK_PRO;

    if (num_rows > 0) {
        attribute_position = AK_get_attr_index(table_name, attribute_name) + 1;

        for (i = 0; i < num_rows; ++i) {
            row = AK_get_row(i, table_name);
            attribute = AK_GetNth_L2(attribute_position, row);

            memmove(data, attribute->data, attribute->size);

            if (!condition_passed(condition, type, value, data)) {
                printf("\n*** ERROR ***\nFailed to add 'check constraint' on TABLE: %s\nEntry in table caused 'constraint violation'!\n\n", table_name);

                AK_EPI;

                return EXIT_ERROR;
            }
        }
    }

    if (AK_check_constraint_name(constraint_name, AK_CONSTRAINTS_CHECK_CONSTRAINT) == EXIT_ERROR) {
        printf("\n*** ERROR ***\nFailed to add 'check constraint' on TABLE: %s\nConstrait '%s' already exists in the database!\n\n", table_name, constraint_name);

        AK_EPI;

        return EXIT_ERROR;
    }

    struct list_node *constraint_row = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&constraint_row);

    int obj_id = AK_get_id();
    int type_param = type;
    int value_param;

    AK_Insert_New_Element(TYPE_INT, &obj_id, AK_CONSTRAINTS_CHECK_CONSTRAINT, "obj_id", constraint_row); // #1
    AK_Insert_New_Element(TYPE_VARCHAR, table_name, AK_CONSTRAINTS_CHECK_CONSTRAINT, "table_name", constraint_row); // #2
    AK_Insert_New_Element(TYPE_VARCHAR, constraint_name, AK_CONSTRAINTS_CHECK_CONSTRAINT, "constraint_name", constraint_row); // #3
    AK_Insert_New_Element(TYPE_VARCHAR, attribute_name, AK_CONSTRAINTS_CHECK_CONSTRAINT, "attribute_name", constraint_row); // #4
    AK_Insert_New_Element(TYPE_INT, &type_param, AK_CONSTRAINTS_CHECK_CONSTRAINT, "constraint_value_type", constraint_row); // #5
    AK_Insert_New_Element(TYPE_VARCHAR, condition, AK_CONSTRAINTS_CHECK_CONSTRAINT, "constraint_condition", constraint_row); // #6

    if (type == TYPE_INT) { // #7
        value_param = value;
        AK_Insert_New_Element(TYPE_INT, &value_param, AK_CONSTRAINTS_CHECK_CONSTRAINT, "constraint_value", constraint_row);
    }
    else if (type == TYPE_FLOAT) {
        AK_Insert_New_Element(TYPE_FLOAT, value, AK_CONSTRAINTS_CHECK_CONSTRAINT, "constraint_value", constraint_row);
    }
    else {
        AK_Insert_New_Element(TYPE_VARCHAR, value, AK_CONSTRAINTS_CHECK_CONSTRAINT, "constraint_value", constraint_row);
    }

    AK_insert_row(constraint_row);

    AK_DeleteAll_L3(&constraint_row);
    AK_free(constraint_row);

    printf("\nCHECK CONSTRAINT set on attribute: '%s' on TABLE %s!\n\n", attribute_name, table_name);

    AK_EPI;

    return EXIT_SUCCESS;
}

/**
 * @author Mislav Jurinić
 * @brief Function that verifies if the value we want to insert satisfies the "check" constraint.
 * @param table target table name
 * @param attribute target attribute name
 * @param value data we want to insert
 * @return 1 - result, 0 - failure 
 */
int AK_check_constraint(char *table, char *attribute, void *value) {
    int i;
    int num_rows = AK_get_num_records(AK_CONSTRAINTS_CHECK_CONSTRAINT);
    int _row_data; // check constraint value
    struct list_node *row;
    struct list_node *constraint_attribute;
    void *row_data = (void *) AK_calloc(MAX_VARCHAR_LENGTH, sizeof (void)); // check constraint value

    AK_PRO;

    if (num_rows != 0) {
        for (i = 0; i < num_rows; ++i) {
            row = AK_get_row(i, AK_CONSTRAINTS_CHECK_CONSTRAINT);
            constraint_attribute = AK_GetNth_L2(7, row);

            memmove(row_data, constraint_attribute->data, AK_type_size(constraint_attribute->type, constraint_attribute->data));  

            // If table name and attribute name match, check value
            if (!strcmp(table, AK_GetNth_L2(2, row)->data) && !strcmp(attribute, AK_GetNth_L2(4, row)->data)) {
                if (AK_GetNth_L2(7, row)->type == TYPE_INT) {
                    _row_data = *((int *) row_data);

                    if (!condition_passed(AK_GetNth_L2(6, row)->data, AK_GetNth_L2(7, row)->type, _row_data, &value)) {
                        AK_EPI;

                        return EXIT_ERROR;
                    }
                    else {
                        break;
                    }
                }

                if (!condition_passed(AK_GetNth_L2(6, row)->data, AK_GetNth_L2(7, row)->type, row_data, value)) {
                    AK_EPI;

                    return EXIT_ERROR;
                }
                else {
                    break;
                }
            }
        }
    }

    AK_EPI;

    return EXIT_SUCCESS;
}

/**
 * @author Bruno Pilošta
 * @brief Function that deletes existing check constraint
 * @param tableName System table where constraint will be deleted from
 * @param constraintName Name of the constraint that will be deleted
 * @return 1 - result, 0 - failure 
 */
int AK_delete_check_constraint(char* tableName, char* constraintName){
    AK_PRO;

    char* constraint_attr = "constraint_name";

    if(AK_check_constraint_name(constraintName, AK_CONSTRAINTS_CHECK_CONSTRAINT) == EXIT_SUCCESS){
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
 * @author Mislav Jurinić, updated by Bruno Pilošta
 * @brief Test function for "check" constraint.
 * @return void
 */
TestResult AK_check_constraint_test() {

    char constraintYearName[50] = "check_student_year";
    char constraintWeightName[50] = "check_student_weight";
    char constraintLastnameName[50] = "check_student_lastname";

	int success = 0;
    int failed = 0;
    // Test 3 data
    float weight_one = 105.5, weight_three = 106.0;
    float *p_weight_one = &weight_one;
    float *p_weight_three = &weight_three;

    // Test 4 data
    float weight_two = 85.5;
    float *p_weight_two = &weight_two;

    int result;

    AK_PRO;
    
    AK_print_table("student");

    AK_print_table(AK_CONSTRAINTS_CHECK_CONSTRAINT);

    printf("\n\n");
	printf("-------------------------------------------------------------------------------------");
	printf("\n\n");

    printf("*** TEST 1 (INT) trying to set a 'year > 2005' constraint***\n");
    result = AK_set_check_constraint("student", constraintYearName, "year", ">", TYPE_INT, 2005);
    //Expecting EXIT_ERROR because there are values in student table for attribute year less than 2005
    if (result == EXIT_ERROR) {
        printf("*** TEST 1 Successful! ***\n");

        success++;
    } else {
        printf("*** TEST 1 Failed! ***\n");
        failed++;
    }
	printf("\n\n");
	printf("-------------------------------------------------------------------------------------");
	printf("\n\n");

    AK_print_table(AK_CONSTRAINTS_CHECK_CONSTRAINT);
    printf("\n\n*** TEST 2 (INT) trying to set a 'year > 1900' constraint ***\n");
    result = AK_set_check_constraint("student", constraintYearName, "year", ">", TYPE_INT, 1900);
    AK_print_table(AK_CONSTRAINTS_CHECK_CONSTRAINT);
    //Expecting EXIT_SUCCESS because there are only values in table "student" bigger than 1900
    if (result == EXIT_SUCCESS) {
        printf("*** TEST 2 Successful! ***\n");
        success++;
    } 
    else {
        printf("*** TEST 2 Failed! ***\n");
        failed++;
    }

    printf("\n\n");
	printf("-------------------------------------------------------------------------------------");
	printf("\n\n");

    printf("*** TEST 2.1 checking if the value 1901 we want to insert for attribute 'year' satisfies the check constraint. ***\n");
    result = AK_check_constraint("student", "year", 1901);
    //Expecting EXIT_SUCCESS because set up constraint checks if the value we want (1901) to insert is bigger than 1900
    if (result == EXIT_SUCCESS) {
        printf("*** TEST 2.1 Successful! ***\n");
        printf("*** Value 1901 for attribute 'year' can be inserted ***\n");
        success++;
    } else {
        printf("*** TEST 2.1 Failed! ***\n");
        failed++;
    }

    printf("\n\n");
	printf("-------------------------------------------------------------------------------------");
	printf("\n\n");

    printf("*** TEST 2.2 checking if the value 1899 we want to insert for attribute 'year' satisfies the check constraint. ***\n");
    result = AK_check_constraint("student", "year", 1899);
    //Expecting EXIT_ERROR because set up constraint checks if the value (1899) we want to insert is bigger than 1900
    if (result == EXIT_ERROR) {
        printf("*** TEST 2.2 Successful! ***\n");
        printf("*** Value 1899 for attribute 'year' can NOT be inserted ***\n");
	    success++;	
    } else {
        printf("*** TEST 2.2 Failed! ***\n");
        failed++;
    }

    printf("\n\n");
	printf("-------------------------------------------------------------------------------------");
	printf("\n\n");

    printf("\n\n*** TEST 3 (FLOAT) trying to set a 'weight <= 105.500' constraint ***\n");
    result = AK_set_check_constraint("student", constraintWeightName, "weight", "<=", TYPE_FLOAT, p_weight_one);
    AK_print_table(AK_CONSTRAINTS_CHECK_CONSTRAINT);
    //Expecting EXIT_SUCCESS because there are only values in table "student" for attribute weight less than 1900
    if (result == EXIT_SUCCESS) {
        printf("*** TEST 3 Successful! ***\n");
        success++;
    } else {
        printf("*** TEST 3 Failed! ***\n");
        failed++;;
    }

    printf("\n\n");
	printf("-------------------------------------------------------------------------------------");
	printf("\n\n");

    printf("*** TEST 3.1 checking if the value 105.5 we want to insert for attribute 'weight' satisfies the check constraint. ***\n");
    result = AK_check_constraint("student", "weight", p_weight_one);
    //Expecting EXIT_SUCCESS because 105.5 IS <= 105.5
    if (result == EXIT_SUCCESS) {
        printf("*** TEST 3.1 Successful! ***\n");
        printf("*** Value 105.5 for attribute 'weight' can be inserted ***\n");
        success++;
    } else {
        printf("*** TEST 3.1 Failed! ***\n");
        failed++;
    }

    printf("\n\n");
	printf("-------------------------------------------------------------------------------------");
	printf("\n\n");

    printf("*** TEST 3.2 checking if the value 106.0 we want to insert for attribute 'weight' satisfies the check constraint.***\n");
    result = AK_check_constraint("student", "weight", p_weight_three);
    //Expecting EXIT_ERROR because 106 IS NOT <= 105.5
    if (result == EXIT_ERROR) {
        printf("*** TEST 3.2 Successful! ***\n");
        printf("*** Value 106.0 for attribute 'weight' can NOT be inserted ***\n");
        success++;
    } else {
        failed++;
        printf("*** TEST 3.2 Failed! ***\n");
    }

	printf("\n\n");
	printf("-------------------------------------------------------------------------------------");
	printf("\n\n");

    printf("\n\n*** TEST 4 (FLOAT) trying to set a 'weight > 85.5' constraint  ***\n");
    result = AK_set_check_constraint("student", constraintWeightName, "weight", ">", TYPE_FLOAT, p_weight_two);
    //Expecting EXIT_ERROR because there are values in the table student for attribute weight less than 85.5
    AK_print_table(AK_CONSTRAINTS_CHECK_CONSTRAINT);
    if (result == EXIT_ERROR) {
        printf("*** TEST 4 Successful! ***\n");
        success++;
    } else {
        printf("*** TEST 4 Failed! ***\n");
        failed++;
    }
	printf("\n\n");
	printf("-------------------------------------------------------------------------------------");
	printf("\n\n");


    printf("\n\n*** TEST 5 (VARCHAR) *** trying to set a 'lastname > Anic' constraint \n");
    result = AK_set_check_constraint("student", constraintLastnameName, "lastname", ">", TYPE_VARCHAR, "Anic");
    //Expecting EXIT_SUCCESS because there are only values in table "student" for attribute lastname bigger than "Anic"
    AK_print_table(AK_CONSTRAINTS_CHECK_CONSTRAINT);
    if (result == EXIT_SUCCESS) {
        printf("*** TEST 5 Successful! ***\n");
        success++;
    } else {
        printf("*** TEST 5 Failed! ***\n");
        failed++;
    }

    printf("\n\n");
	printf("-------------------------------------------------------------------------------------");
	printf("\n\n");

    printf("*** TEST 5.1 *** checking if the value 'Baric' we want to insert for attribute 'lastname' satisfies the check constraint.\n");
    //Expecting EXIT_SUCCESS because Anic IS < Baric
    result = AK_check_constraint("student", "lastname", "Baric");
    if (result == EXIT_SUCCESS) {
        printf("*** TEST 5.1 Successful! ***\n");
        printf("*** Value 'Baric' can be inserted ***\n");
        success++;
    } else {
        printf("*** TEST 5.1 Failed! ***\n");
        failed++;
    }

    printf("\n\n");
	printf("-------------------------------------------------------------------------------------");
	printf("\n\n");

    printf("*** TEST 5.2 *** checking if the value 'Abdullah' we want to insert for attribute 'lastname' satisfies the check constraint. \n");
    //Expecting EXIT_ERROR because Anic IS NOT < Abdullah
    result = AK_check_constraint("student", "lastname", "Abdullah");
    if (result == EXIT_ERROR) {
        printf("*** TEST 5.2 Successful! ***\n");
        printf("Value 'Abdullah' can NOT be inserted\n");
        success++;
    } else {
        printf("*** TEST 5.2 Failed! ***\n");
        failed++;
    }
	printf("\n\n");
	printf("-------------------------------------------------------------------------------------");
	printf("\n\n");

    printf("\n\n*** TEST 6 (VARCHAR) *** trying to set a 'lastname < Yeager' constraint \n");
    result = AK_set_check_constraint("student", constraintLastnameName, "lastname", "<", TYPE_VARCHAR, "Yeager");
    //Expecting EXIT_ERROR because constraint 'check_student_lastname' already exists
    AK_print_table(AK_CONSTRAINTS_CHECK_CONSTRAINT);
    if (result == EXIT_ERROR) {
        printf("*** TEST 6 Successful! ***\n");
	    success++;
    } else {
        printf("*** TEST 6 Failed! ***\n");
        failed++;
    }

    printf("\n\n");
	printf("-------------------------------------------------------------------------------------");
	printf("\n\n");

    // Should fail
    printf("\n\n*** TEST 7 (VARCHAR) *** trying to set a 'lastname > Yeager' constraint \n");
    result = AK_set_check_constraint("student", "check_student_lastname_fail", "lastname", ">", TYPE_VARCHAR, "Yeager");
    //Expecting EXIT_ERROR because there are values in the table student for attribute 'lastname' less than 'Yeager'
    AK_print_table(AK_CONSTRAINTS_CHECK_CONSTRAINT);
    if (result == EXIT_ERROR) {
        printf("*** TEST 7 Successful! ***\n");
	    success++;
    } else {
        printf("*** TEST 7 Failed! ***\n");
        failed++;
    }


        AK_print_table(AK_CONSTRAINTS_CHECK_CONSTRAINT);

    printf("\n\n*** TEST 8 *** trying to delete all existing CHECK constraints \n");
    int delete1 =AK_delete_check_constraint(AK_CONSTRAINTS_CHECK_CONSTRAINT, constraintYearName);
    int delete2 =AK_delete_check_constraint(AK_CONSTRAINTS_CHECK_CONSTRAINT, constraintWeightName);
    int delete3 =AK_delete_check_constraint(AK_CONSTRAINTS_CHECK_CONSTRAINT, constraintLastnameName);
    AK_print_table(AK_CONSTRAINTS_CHECK_CONSTRAINT);
    if (delete1 == EXIT_SUCCESS && delete2 == EXIT_SUCCESS && delete3 == EXIT_SUCCESS)
	{
		success++;
        printf("*** TEST 8 Successful! ***\n");
	}
	else{
		failed++;
        printf("*** TEST 8 Failed! ***\n");
	}

    AK_EPI;

    return TEST_result(success,failed);
}
