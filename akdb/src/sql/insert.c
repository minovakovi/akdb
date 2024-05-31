/**
 * @file insert.h Implementation of SQL insert command.
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


#include "insert.h"

/**
 * @author Filip Žmuk
 * @brief Function creates headers based on entered columns in SQL command.
 * If no columns are entered it will use table header.
 * @param size pointer to integer in which size of header will be saved
 * @param tblName table in which rows will be inserted
 * @param columns list of columns in SQL command
 * @return header for values to be inserted or EXIT_ERROR
 */
AK_header *AK_get_insert_header(int *size, char *tblName, struct list_node *columns) {
    AK_PRO;
    int num_attr = AK_num_attr(tblName);
    AK_header *tblHeader = AK_get_header(tblName);
    
    if(!columns) {
        *size = num_attr;
        AK_EPI;
        return tblHeader;
    }

    int columnsNumber = AK_Size_L2(columns);

    AK_header *header = (AK_header *) AK_malloc(columnsNumber * sizeof(AK_header));

    int j = 0;
    struct list_node* column = AK_First_L2(columns);
    while(column) {
        // search for duplicate column
        for(int i = 0; i < j; i++) {
            if(strcmp(column->attribute_name, header[i].att_name) == 0) {
                AK_free(header);
                AK_free(tblHeader);
                AK_EPI;
                return EXIT_ERROR;
            }
        }

        // search for column in table atributes 
        int found = 0;
        for(int i = 0; i < num_attr; i++) {
            if(strcmp(column->attribute_name, tblHeader[i].att_name) == 0) {
                header[j].type = tblHeader[i].type;
                strcpy(header[j].att_name, column->attribute_name);
                found = 1;
                j++;
                break;
            }
        }

        if(!found) {
            AK_free(header);
            AK_free(tblHeader);
            AK_EPI;
            return EXIT_ERROR;
        }

        column = AK_Next_L2(column);
    }

    AK_free(tblHeader);
    *size = columnsNumber;
    AK_EPI;
    return header;
}

/**
 * @author Filip Žmuk
 * @brief Function that implements SQL insert command
 * @param tableName table in which rows will be inserted
 * @param columns list of columns
 * @param values values to be inserted
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_insert(char* tblName, struct list_node *columns, struct list_node *values) 
{
    AK_PRO;
    if(!AK_table_exist(tblName)) {
        AK_EPI;
        return EXIT_ERROR;
    }

    int num_values = AK_Size_L2(values);
    int num_columns = 0;

    AK_header *header = AK_get_insert_header(&num_columns, tblName, columns);

    if(header == EXIT_ERROR) {
        AK_EPI;
        return EXIT_ERROR;
    }

    if(num_values % num_columns) {
        AK_free(header);
        AK_EPI;
        return EXIT_ERROR;
    }

    struct list_node *row;
    int i = 0; // index of values
    struct list_node *value = AK_First_L2(values);

    while(value)
    {
        if(i == 0) {
            row = (struct list_node *) AK_malloc(sizeof (struct list_node));
            AK_Init_L3(&row);
        }

        if(header[i].type != value->type) {
            AK_DeleteAll_L3(&row);
            AK_free(row);
            AK_free(header);
            AK_EPI;
            return EXIT_ERROR;
        }

        AK_Insert_New_Element(value->type, value->data, tblName, header[i].att_name, row);

        i++;

        if(i >= num_columns) {
            i = 0;
            int result = AK_insert_row(row);
            AK_DeleteAll_L3(&row);
            AK_free(row);
            if(result != EXIT_SUCCESS) {
                AK_free(header);
                AK_EPI;
                return EXIT_ERROR;
            }
        }

        value = AK_Next_L2(value);
    }

    AK_free(header);
    AK_EPI;
    return EXIT_SUCCESS;
}

TestResult AK_insert_test() {
    struct list_node *row_root;
    struct list_node *col_root;
    int id_field;
    float size_field;
    int result;
    int failed_tests = 0;
    int passed_tests = 0;

    AK_PRO;
    char *testTable = "insertTestTable";

    AK_header header[4] = {
    {TYPE_INT, "id", {0}, {{'\0'}}, {{'\0'}}},
    {TYPE_VARCHAR, "name", {0}, {{'\0'}}, {{'\0'}}},
    {TYPE_FLOAT, "size", {0}, {{'\0'}}, {{'\0'}}},
    {0, {'\0'}, {0}, {{'\0'}}, {{'\0'}}}
    };

    if(AK_initialize_new_segment(testTable, SEGMENT_TYPE_TABLE, header) == EXIT_ERROR) {
        printf("\nCouldn't create test table\n");
        AK_EPI;
        return TEST_result(0, 1);
    }

    int num_table_rows = AK_get_num_records(testTable);

    printf("\nStarting table for test:\n");
    AK_print_table(testTable);

    row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&row_root);
    col_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&col_root);

    // Test 1
    printf("\nInsert two rows without specifing table columns\n");
    AK_DeleteAll_L3(&row_root);

    // Elements are in reverse order because AK_Insert_New_Element puts element on first place in list
    id_field = 2;
    size_field = 6.2;
    AK_Insert_New_Element(TYPE_FLOAT, &size_field, testTable, "", row_root); // size
    AK_Insert_New_Element(TYPE_VARCHAR, "Filip", testTable, "", row_root); // name
    AK_Insert_New_Element(TYPE_INT, &id_field, testTable, "", row_root); // id

    id_field = 1;
    size_field = 3.5;
    AK_Insert_New_Element(TYPE_FLOAT, &size_field, testTable, "", row_root); // size
    AK_Insert_New_Element(TYPE_VARCHAR, "Petrica", testTable, "", row_root); // name
    AK_Insert_New_Element(TYPE_INT, &id_field, testTable, "", row_root); // id

    result = AK_insert(testTable, NULL, row_root);

    if (
        result == EXIT_SUCCESS
        && ((int*)(AK_get_tuple(num_table_rows, 0, testTable)->data))[0] == 1
        && ((int*)(AK_get_tuple(num_table_rows + 1, 0, testTable)->data))[0] == 2
        && ((float*)(AK_get_tuple(num_table_rows, 2, testTable)->data))[0] > 3.495
        && ((float*)(AK_get_tuple(num_table_rows, 2, testTable)->data))[0] < 3.505
        && strcmp(AK_get_tuple(num_table_rows, 1, testTable)->data, "Petrica") == 0 
        && ( num_table_rows + 2 == AK_get_num_records(testTable) )
    ) {
        passed_tests++;
        printf("Test pased!\n");
    } else {
        failed_tests++;
        printf("Test failed!\n");
    }

    // Test 2
    printf("\nInsert should fail if data types don't match\n");
    num_table_rows = AK_get_num_records(testTable);

    AK_DeleteAll_L3(&row_root);
    // Elements are in reverse order because AK_Insert_New_Element puts element on first place in list
    AK_Insert_New_Element(TYPE_VARCHAR, "Test", testTable, "", row_root); // size
    AK_Insert_New_Element(TYPE_VARCHAR, "Test", testTable, "", row_root); // name
    AK_Insert_New_Element(TYPE_INT, &id_field, testTable, "", row_root); // id

    result = AK_insert(testTable, NULL, row_root);
    if (result == EXIT_ERROR && num_table_rows == AK_get_num_records(testTable)) {
        printf("Test pased!\n");
        passed_tests++;
    } else {
        failed_tests++;
        printf("Test failed!\n");
    }

    // Test 3
    printf("\nInsert with random column order\n"); 
    num_table_rows = AK_get_num_records(testTable);

    id_field = 3;
    size_field = 5.0;

    AK_DeleteAll_L3(&row_root);
    AK_Insert_New_Element(TYPE_FLOAT, &size_field, testTable, "", row_root); // size
    AK_Insert_New_Element(TYPE_INT, &id_field, testTable, "", row_root); // id
    AK_Insert_New_Element(TYPE_VARCHAR, "Ariana", testTable, "", row_root); // name

    AK_DeleteAll_L3(&col_root);
    AK_Insert_New_Element(TYPE_FLOAT, &size_field, testTable, "size", col_root); // size
    AK_Insert_New_Element(TYPE_INT, &id_field, testTable, "id", col_root); // id
    AK_Insert_New_Element(TYPE_VARCHAR, "", testTable, "name", col_root); // name

    result = AK_insert(testTable, col_root, row_root);

    if (
        result == EXIT_SUCCESS
        && ((int*)(AK_get_tuple(num_table_rows, 0, testTable)->data))[0] == 3
        && strcmp(AK_get_tuple(num_table_rows, 1, testTable)->data, "Ariana") == 0 
        && ( num_table_rows + 1 == AK_get_num_records(testTable) )
    ) {
        passed_tests++;
        printf("Test pased!\n");
    } else {
        failed_tests++;
        printf("Test failed!\n");
    }

    // Test 4 
    printf("\nInsert should fail if attribute doesn't exist\n");
    num_table_rows = AK_get_num_records(testTable);

    AK_DeleteAll_L3(&row_root);
    AK_Insert_New_Element(TYPE_FLOAT, &size_field, testTable, "", row_root);
    AK_Insert_New_Element(TYPE_INT, &id_field, testTable, "", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "Ariana", testTable, "", row_root);

    AK_DeleteAll_L3(&col_root);
    AK_Insert_New_Element(TYPE_FLOAT, &size_field, testTable, "nothing", col_root); 
    AK_Insert_New_Element(TYPE_INT, &id_field, testTable, "id", col_root); 
    AK_Insert_New_Element(TYPE_VARCHAR, "", testTable, "name", col_root);

    result = AK_insert(testTable, col_root, row_root);

    if (result == EXIT_ERROR && num_table_rows == AK_get_num_records(testTable)) {
        printf("Test pased!\n");
        passed_tests++;
    } else {
        failed_tests++;
        printf("Test failed!\n");
    }

    // Test 5
    printf("\nInsert should fail for duplicate attributes\n");
    num_table_rows = AK_get_num_records(testTable);

    AK_DeleteAll_L3(&row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "", testTable, "", row_root);
    AK_Insert_New_Element(TYPE_INT, &id_field, testTable, "", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "", testTable, "", row_root);

    AK_DeleteAll_L3(&col_root);
    AK_Insert_New_Element(TYPE_FLOAT, &size_field, testTable, "name", col_root); 
    AK_Insert_New_Element(TYPE_INT, &id_field, testTable, "id", col_root); 
    AK_Insert_New_Element(TYPE_VARCHAR, "", testTable, "name", col_root);

    result = AK_insert(testTable, col_root, row_root);

    if (result == EXIT_ERROR && num_table_rows == AK_get_num_records(testTable)) {
        printf("Test pased!\n");
        passed_tests++;
    } else {
        failed_tests++;
        printf("Test failed!\n");
    }

    printf("\nTable after tests:\n");
    AK_print_table(testTable);

    AK_DeleteAll_L3(&row_root);
    AK_free(row_root);
    AK_DeleteAll_L3(&col_root);
    AK_free(col_root);
    AK_EPI;
    return TEST_result(passed_tests, failed_tests);
}
