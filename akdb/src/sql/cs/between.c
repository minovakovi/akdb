/**
@file between.c Provides functions for between constaint
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

#include "between.h"
//string restrictions must be in lowercase

/**
 * @author Mislav Jurinić
 * @brief Function that returns system tables addresses by name
 * @param _systemTableName table name
 * @return int
 **/
int AK_find_table_address(char* _systemTableName) {
    AK_block *tempBlock = (AK_block *) AK_read_block(0);
    int i = -1;
    int ret;
    int tuple_dict_size = sizeof(tempBlock->tuple_dict) / sizeof(AK_tuple_dict);
    char systemTableName[50];

    AK_PRO;

    while (++i + 1 <= tuple_dict_size) {
        memset(systemTableName, FREE_CHAR, sizeof(systemTableName));
        memmove(systemTableName, tempBlock->data + tempBlock->tuple_dict[i].address, strlen(tempBlock->data + tempBlock->tuple_dict[i].address));

        systemTableName[strlen(tempBlock->data + tempBlock->tuple_dict[i].address) - 1] = FREE_CHAR;

        if (strcmp(systemTableName, _systemTableName) == 0) {
            memmove(&ret, tempBlock->data + tempBlock->tuple_dict[i + 1].address, tempBlock->tuple_dict[i + 1].size);

            AK_EPI;

            return ret;
        }
    }

    AK_free(tempBlock);
    AK_EPI;

    return -1;
}

/**
 * @author Saša Vukšić, updated by Mislav Jurinić, updated by Blaž Rajič
 * @brief Function that sets between constraints on particular attribute, string constraint should be writen in lowercase.
      It searches for AK_free space. Then it inserts id, name of table, name of constraint, name of attribute, start and
       end value in temporary block.      
 * @param tableName table name
 * @param constraintName name of constraint
 * @param attName name of attribute
 * @param startValue initial constraint
 * @param endValue final constraint
 * @return No return value
 **/
void AK_set_constraint_between(char* tableName, char* constraintName, char* attName, char* startValue, char* endValue) {
    /*char* systemTableName = AK_CONSTRAINTS_BEWTEEN;

    int systemTableAddress = AK_find_table_address(systemTableName);
    int AK_freeSpaceFound = 0;
    int tupleDictID = -1;
    int id;

    AK_block *tempBlock;*/

    AK_PRO;
    /*
    if (systemTableAddress != -1) {
        AK_dbg_messg(HIGH, CONSTRAINTS, "System table to insert: %s, address: %i\n", systemTableName, systemTableAddress);

        tempBlock = (AK_block *) AK_read_block(systemTableAddress);

        while (AK_freeSpaceFound == 0) {
            ++tupleDictID;

            if (tempBlock->tuple_dict[tupleDictID].size == FREE_INT) {
                AK_freeSpaceFound = 1;
            }
        }

        if (AK_freeSpaceFound == 1) {
            id = AK_get_table_id(systemTableName);

            AK_insert_entry(tempBlock, TYPE_INT, &id, tupleDictID);
            AK_insert_entry(tempBlock, TYPE_VARCHAR, tableName, tupleDictID + 1);
            AK_insert_entry(tempBlock, TYPE_VARCHAR, constraintName, tupleDictID + 2);
            AK_insert_entry(tempBlock, TYPE_VARCHAR, attName, tupleDictID + 3);
            AK_insert_entry(tempBlock, TYPE_VARCHAR, startValue, tupleDictID + 4);
            AK_insert_entry(tempBlock, TYPE_VARCHAR, endValue, tupleDictID + 5);

            AK_write_block(tempBlock);
        }
    }*/

    if (AK_check_constraint_name(constraintName, AK_CONSTRAINTS_BEWTEEN) == EXIT_ERROR) {
        printf("\n*** ERROR ***\nFailed to add 'between constraint' on TABLE: %s\nConstrait '%s' already exists in the database!\n\n", tableName, constraintName);

        AK_EPI;

        return EXIT_ERROR;
    }

    struct list_node *constraint_row = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&constraint_row);

    int obj_id = AK_get_id();
    //int type_param = type;
    int value_param;

    AK_Insert_New_Element(TYPE_INT, &obj_id, AK_CONSTRAINTS_BEWTEEN, "obj_id", constraint_row); // #1
    AK_Insert_New_Element(TYPE_VARCHAR, tableName, AK_CONSTRAINTS_BEWTEEN, "tableName", constraint_row); // #2
    AK_Insert_New_Element(TYPE_VARCHAR, constraintName, AK_CONSTRAINTS_BEWTEEN, "constraintName", constraint_row); // #3
    AK_Insert_New_Element(TYPE_VARCHAR, attName, AK_CONSTRAINTS_BEWTEEN, "attributeName", constraint_row); // #4
    //AK_Insert_New_Element(TYPE_INT, &type_param, AK_CONSTRAINTS_CHECK_CONSTRAINT, "value_type", constraint_row); // #5
    AK_Insert_New_Element(TYPE_VARCHAR, startValue, AK_CONSTRAINTS_BEWTEEN, "start_value", constraint_row); // #6
    AK_Insert_New_Element(TYPE_VARCHAR, endValue, AK_CONSTRAINTS_BEWTEEN, "end_value", constraint_row);

   /* if (type == TYPE_INT) { // #7
        value_param = value;
        AK_Insert_New_Element(TYPE_INT, &value_param, AK_CONSTRAINTS_CHECK_CONSTRAINT, "constraint_value", constraint_row);
    }
    else if (type == TYPE_FLOAT) {
        AK_Insert_New_Element(TYPE_FLOAT, value, AK_CONSTRAINTS_CHECK_CONSTRAINT, "constraint_value", constraint_row);
    }
    else {
        AK_Insert_New_Element(TYPE_VARCHAR, value, AK_CONSTRAINTS_CHECK_CONSTRAINT, "constraint_value", constraint_row);
    }*/

    AK_insert_row(constraint_row);

    AK_DeleteAll_L3(&constraint_row);
    AK_free(constraint_row);

    printf("\nCHECK CONSTRAINT set on attribute: '%s' on TABLE %s!\n\n", attName, tableName);

    AK_EPI;
}

/**
 * @author Saša Vukšić, updated by Mislav Jurinić, updated by Blaž Rajič
 * @brief Function that checks if the given value is between lower and upper bounds of the "between" constraint.
 * @param tableName table name
 * @param newValue value we want to insert
 * @param attNamePar attribute name in table
 * @return EXIT_SUCCESS or EXIT_ERROR
 **/
int AK_read_constraint_between(char* tableName, char* newValue, char* attNamePar) {

    int systemTableAddress = AK_find_table_address(AK_CONSTRAINTS_BEWTEEN);
    int num_rows = AK_get_num_records(AK_CONSTRAINTS_BEWTEEN);
    int loop_count;
    AK_block *tempBlock;
    struct list_node *row;
    struct list_node *constraint_table;
    struct list_node *constraint_attribute;
    struct list_node *start_value;
    struct list_node *end_value;

    AK_PRO;

    if (systemTableAddress == -1) {
        printf("\nERROR: Table for between_constraints not found.\n");

        AK_EPI;
        return EXIT_ERROR;
    }

    if (num_rows != 0) {
        for (loop_count = 0; loop_count < num_rows; loop_count++) {
            row = AK_get_row(loop_count, AK_CONSTRAINTS_BEWTEEN);
            constraint_table = AK_GetNth_L2(2, row);
            
            if(strcmp(constraint_table->data, tableName) == 0){

                constraint_attribute = AK_GetNth_L2(4, row);
                if(strcmp(constraint_attribute->data, attNamePar) == 0){

                    start_value = AK_GetNth_L2(5, row);
                    end_value = AK_GetNth_L2(6, row);

                    if(strcmp(start_value->data,newValue) > 0){
                        printf("\nFAILURE: Value '%s' is smaller than minimal allowed value: '%s' \n",newValue,start_value->data);

                        AK_EPI;
                        return EXIT_FAILURE;
                    }

                    if(strcmp(end_value->data,newValue) < 0){

                        printf("\nFAILURE: Value '%s' is bigger than maximum allowed value: '%s' \n",newValue,end_value->data);

                        AK_EPI;
                        return EXIT_FAILURE;
                    }
                }
            }
        }
    }

    AK_free(tempBlock);
    AK_free(constraint_table);
    AK_free(constraint_attribute);
    AK_free(start_value);
    AK_free(end_value);
    AK_EPI;

    return EXIT_SUCCESS;
}

/**
 * @author Maja Vračan
 * @brief Function for printing tables
 * @param tableName name of table 
 */
void AK_print_constraints(char* tableName) {  //currently not used, maybe delete?
    char* systemTableName = AK_CONSTRAINTS_BEWTEEN;
    char attName[50] = {FREE_CHAR};
    char constraintName[50] = {FREE_CHAR};
    char inMemoryTable[50] = {FREE_CHAR};
    char lowerBoundary[50] = {FREE_CHAR};
    char upperBoundary[50] = {FREE_CHAR};

    int systemTableAddress = AK_find_table_address(systemTableName);
    int tupleDictID = -1;
    int flag = EXIT_SUCCESS;

    AK_block *tempBlock;

    AK_PRO;

    printf("\n============= CONSTRAINTS LISTING =============\n");
    if (systemTableAddress != -1) {
        tempBlock = (AK_block *) AK_read_block(systemTableAddress);

        while (tempBlock->tuple_dict[++tupleDictID].size != FREE_INT) {
            // clear all vars  
            memset(attName, FREE_CHAR, sizeof(attName));
            memset(constraintName, FREE_CHAR, sizeof(constraintName));
            memset(inMemoryTable, FREE_CHAR, sizeof(inMemoryTable));
            memset(lowerBoundary, FREE_CHAR, sizeof(lowerBoundary));
            memset(upperBoundary, FREE_CHAR, sizeof(upperBoundary));

            memmove(inMemoryTable, tempBlock->data + tempBlock->tuple_dict[tupleDictID].address, tempBlock->tuple_dict[tupleDictID].size);
            inMemoryTable[strlen(tempBlock->data + tempBlock->tuple_dict[tupleDictID].address) - 1] = FREE_CHAR;

            if (strcmp(inMemoryTable, tableName) == 0) {                
                AK_dbg_messg(HIGH, CONSTRAINTS, "--------------------------------\n");                
                AK_dbg_messg(HIGH, CONSTRAINTS, "Table name: %s\n", tableName);

                memmove(attName, tempBlock->data + tempBlock->tuple_dict[tupleDictID + 2].address, tempBlock->tuple_dict[tupleDictID + 2].size);
                AK_dbg_messg(HIGH, CONSTRAINTS, "Attribute name: %s\n", attName);

                memmove(constraintName, tempBlock->data + tempBlock->tuple_dict[tupleDictID + 1].address, tempBlock->tuple_dict[tupleDictID + 1].size);
                AK_dbg_messg(HIGH, CONSTRAINTS, "Constraint name: %s\n", constraintName);

                memmove(lowerBoundary, tempBlock->data + tempBlock->tuple_dict[tupleDictID + 3].address, tempBlock->tuple_dict[tupleDictID + 3].size);
                AK_dbg_messg(HIGH, CONSTRAINTS, "Low boundary: %s\n", lowerBoundary);

                memmove(upperBoundary, tempBlock->data + tempBlock->tuple_dict[tupleDictID + 4].address, tempBlock->tuple_dict[tupleDictID + 4].size);
                AK_dbg_messg(HIGH, CONSTRAINTS, "High boundary: %s\n", upperBoundary);
                AK_dbg_messg(HIGH, CONSTRAINTS, "--------------------------------\n");

                // DEBUG
                printf("\ninMemoryTable: %s\n", inMemoryTable);
                printf("Table name: %s\n", tableName);
                printf("Attribute name: %s\n", attName);
                printf("Constraint name: %s\n", constraintName);
                printf("Low boundary: %s\n", lowerBoundary);
                printf("High boundary: %s\n", upperBoundary);
            }
        }
    }

    AK_free(tempBlock);
    AK_EPI;
}

/**
 * @author Maja Vračan
 * @brief Function for deleting specific between constraint
 * @param tableName name of table on which constraint refers
 * @param attName name of attribute on which constraint is declared
 * @param constraintName name of constraint 
 * @return EXIT_SUCCESS when constraint is deleted, else EXIT_ERROR
 */

int AK_delete_constraint_between(char* tableName, char* constraintNamePar){
    AK_PRO;

    char* constraint_attr = "constraintName";

    if(AK_check_constraint_name(constraintNamePar, AK_CONSTRAINTS_BEWTEEN) == EXIT_SUCCESS){
        printf("FAILURE! -- CONSTRAINT with name %s doesn't exist in TABLE %s", constraintNamePar, tableName);
        AK_EPI;
        return EXIT_ERROR;
    }

    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&row_root);
    
    AK_Update_Existing_Element(TYPE_VARCHAR, constraintNamePar, tableName, constraint_attr, row_root);
    int result = AK_delete_row(row_root);
    AK_DeleteAll_L3(&row_root);
	AK_free(row_root);    

    AK_EPI;

    return result;
}

/**
  * @author Saša Vukšić, updated by Mislav Jurinić, updated by Blaž Rajič
  * @brief Function that tests the functionality of implemented between constraint.
  * @return No return value
  */
TestResult AK_constraint_between_test() {
    // TEST #1
    char* tableName_1 = "department";
    char* attName_1 = "dep_name";
    char* constraintName_1 = "dep_name_between";
    char* newValue_1 = "Department of Zoo";
    char* startValue_1 = "Department of Economy";
    char* endValue_1 = "Department of Organization";
    int failed=0;
    int passed=0;
    AK_PRO;

    printf("============= Running Test #1 =============\n");

    printf("\nTrying to insert constraint %s and then testing values.\n\n",constraintName_1);
    AK_print_table(tableName_1);
    AK_set_constraint_between(tableName_1, constraintName_1, attName_1, startValue_1, endValue_1);
    AK_print_table(AK_CONSTRAINTS_BEWTEEN);
    printf("\nTrying to set '%s' as attribute '%s' in table '%s' which should fail.\n",newValue_1,attName_1,tableName_1);
    if (AK_read_constraint_between(tableName_1, newValue_1, attName_1) == EXIT_FAILURE) {
        printf("\nSUCCESS\n\n");
        passed++;
    } 
    else {
        printf("\nFAILED\n\n");
        failed++;
    }
    
    // TEST #2
    char* tableName_2 = "department";
    char* attName_2 = "manager";
    char* constraintName_2 = "manager_surname_between";
    char* newValue_2 = "Kero";
    char* startValue_2 = "Hutinski";    
    char* endValue_2 = "Redep";
    AK_PRO;

    printf("\n============== Running Test #2 ==============\n");

    printf("\nTrying to insert constraint %s and then testing values.\n\n",constraintName_2);

    AK_set_constraint_between(tableName_2, constraintName_2, attName_2, startValue_2, endValue_2);
    AK_print_table(AK_CONSTRAINTS_BEWTEEN);
    printf("\nTrying to set '%s' as attribute '%s' in table '%s' which should pass.\n",newValue_2,attName_2,tableName_2);
    if (AK_read_constraint_between(tableName_2, newValue_2, attName_2) == EXIT_SUCCESS) {
        printf("\nSUCCESS\n\n");
        passed++;
    } 
    else {
        printf("\nFAILED\n\n");
        failed++;
    }

    //INTEGER TEST
    char* tableName_3 = "department";
    char* attName_3 = "id_department";
    char* constraintName_3 = "id_department_between";
    char* newValue_3 = "3";
    char* startValue_3 = "1";    
    char* endValue_3 = "5";

    AK_PRO;

    printf("\n============== Running Test #3 ==============\n");

    printf("\nTrying to insert constraint %s and then testing values.\n\n",constraintName_3);

    AK_set_constraint_between(tableName_3, constraintName_3, attName_3, startValue_3, endValue_3);
    AK_print_table(AK_CONSTRAINTS_BEWTEEN);
    printf("\nTrying to set '%s' as attribute '%s' in table '%s' which should pass.\n",newValue_3,attName_3,tableName_3);
    if (AK_read_constraint_between(tableName_3, newValue_3, attName_3) == EXIT_SUCCESS) {
        printf("\nSUCCESS\n\n");
        passed++;
    } 
    else {
        printf("\nFAILED\n\n");
        failed++;
    }

    //FLOAT TEST
    char* newValue_4 = "-1.2";
    printf("\n============== Running Test #4 ==============\n");

    printf("\nTrying to set '%s' as attribute '%s' in table '%s' which should fail.\n",newValue_4,attName_3,tableName_3);
    if (AK_read_constraint_between(tableName_3, newValue_4, attName_3) == EXIT_FAILURE) {
        printf("\nSUCCESS\n");
        passed++;
    } 
    else {
        printf("\nFAILED\n\n");
        failed++;
    }

    
    // DELETE TEST
    printf("\n============== Running Test #5 ==============\n");

    printf("\nTrying to delete constraint %s and %s from database.\n\n",constraintName_1,constraintName_2);

    if(AK_delete_constraint_between(AK_CONSTRAINTS_BEWTEEN, constraintName_2) == EXIT_SUCCESS && AK_delete_constraint_between(AK_CONSTRAINTS_BEWTEEN, constraintName_1)== EXIT_SUCCESS && AK_delete_constraint_between(AK_CONSTRAINTS_BEWTEEN, constraintName_3)== EXIT_SUCCESS) {
        AK_print_table(AK_CONSTRAINTS_BEWTEEN);
        printf("\nSUCCESS\n\n");
        passed++;
    } 
    else {
        AK_print_table(AK_CONSTRAINTS_BEWTEEN);
        printf("\nFAILED\n\n");
        failed++;
    }
    AK_EPI;
    return TEST_result(passed,failed);
}


