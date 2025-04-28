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


 TestResult AK_check_constraint_test(void) {
     AK_PRO;
     int passed = 0, failed = 0;
 
     // Constraint names
     char cn_year[]     = "check_student_year";
     char cn_weight[]   = "check_student_weight";
     char cn_lastname[] = "check_student_lastname";
 
     // Test values
     int   i1 = 2005, i2 = 1900, iv1 = 1901, iv2 = 1899;
     float w1 = 105.5f, w2 = 85.5f, w3 = 106.0f;
 
     typedef enum { OP_SET, OP_READ, OP_DELETE_ALL } Op;
     typedef union { int i; float *f; const char *s; } Val;
     typedef struct { Op op; const char *cn; const char *tbl; const char *at; const char *opstr; int type; Val val; int exp; } T;
 
     T tests[] = {
         {OP_SET,     cn_year,   "student", "year",     ">", TYPE_INT,     {.i = i1},  0},
         {OP_SET,     cn_year,   "student", "year",     ">", TYPE_INT,     {.i = i2},  1},
         {OP_READ,    NULL,      "student", "year",     NULL,  TYPE_INT,     {.i = iv1}, 1},
         {OP_READ,    NULL,      "student", "year",     NULL,  TYPE_INT,     {.i = iv2}, 0},
         {OP_SET,     cn_weight, "student", "weight",   "<=", TYPE_FLOAT,   {.f = &w1}, 1},
         {OP_READ,    NULL,      "student", "weight",   NULL,  TYPE_FLOAT,   {.f = &w1}, 1},
         {OP_READ,    NULL,      "student", "weight",   NULL,  TYPE_FLOAT,   {.f = &w3}, 0},
         {OP_SET,     cn_weight, "student", "weight",   ">", TYPE_FLOAT,   {.f = &w2}, 0},
         {OP_SET,     cn_lastname,"student","lastname",">", TYPE_VARCHAR, {.s = "Anic"},    1},
         {OP_READ,    NULL,      "student", "lastname", NULL, TYPE_VARCHAR, {.s = "Baric"},  1},
         {OP_READ,    NULL,      "student", "lastname", NULL, TYPE_VARCHAR, {.s = "Abdullah"},0},
         {OP_SET,     cn_lastname,"student","lastname","<", TYPE_VARCHAR, {.s = "Yeager"}, 0},
         {OP_SET,     "check_student_lastname_fail","student","lastname",">", TYPE_VARCHAR, {.s = "Yeager"}, 0},
         {OP_DELETE_ALL,NULL,    NULL,       NULL,       NULL, TYPE_VARCHAR, {.i = 0},      1}
     };
     size_t n = sizeof tests / sizeof *tests;
 
     for (size_t i = 0; i < n; ++i) {
         T *t = &tests[i];
         printf("\n---- TEST %zu ----\n", i + 1);
         int res = EXIT_ERROR;
 
         if (t->op == OP_SET) {
             // Perform SET operation
             if (t->type == TYPE_INT) {
                 res = AK_set_check_constraint((char*)t->tbl, (char*)t->cn, (char*)t->at,
                                               (char*)t->opstr, t->type,
                                               t->val.i);
             } else if (t->type == TYPE_FLOAT) {
                 res = AK_set_check_constraint((char*)t->tbl, (char*)t->cn, (char*)t->at,
                                               (char*)t->opstr, t->type,
                                               t->val.f);
             } else {
                 res = AK_set_check_constraint((char*)t->tbl, (char*)t->cn, (char*)t->at,
                                               (char*)t->opstr, t->type,
                                               (void*)t->val.s);
             }
             AK_print_table(AK_CONSTRAINTS_CHECK_CONSTRAINT);
         } else if (t->op == OP_READ) {
             // Perform READ operation
             if (t->type == TYPE_INT) {
                 res = AK_check_constraint((char*)t->tbl, (char*)t->at,
                                           t->val.i);
             } else if (t->type == TYPE_FLOAT) {
                 res = AK_check_constraint((char*)t->tbl, (char*)t->at,
                                           t->val.f);
             } else {
                 res = AK_check_constraint((char*)t->tbl, (char*)t->at,
                                           (void*)t->val.s);
             }
         } else {
             // Delete all constraints
             int d1 = AK_delete_check_constraint(AK_CONSTRAINTS_CHECK_CONSTRAINT, cn_year);
             int d2 = AK_delete_check_constraint(AK_CONSTRAINTS_CHECK_CONSTRAINT, cn_weight);
             int d3 = AK_delete_check_constraint(AK_CONSTRAINTS_CHECK_CONSTRAINT, cn_lastname);
             AK_print_table(AK_CONSTRAINTS_CHECK_CONSTRAINT);
             res = (d1 == EXIT_SUCCESS && d2 == EXIT_SUCCESS && d3 == EXIT_SUCCESS) ? EXIT_SUCCESS : EXIT_ERROR;
         }
 
         // Evaluate result
         if ((res == EXIT_SUCCESS) == t->exp) {
             passed++;
             printf("Test passed!\n");
         } else {
             failed++;
             printf("Test FAILED!\n");
         }
     }
 
     AK_EPI;
     return TEST_result(passed, failed);
 }
 