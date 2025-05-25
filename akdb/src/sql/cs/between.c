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
    struct list_node *row;
    struct list_node *constraint_table;
    struct list_node *constraint_attribute;
    struct list_node *start_value;
    struct list_node *end_value;
    int result = EXIT_SUCCESS;

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
            
            if(strcmp(constraint_table->data, tableName) == 0) {
                constraint_attribute = AK_GetNth_L2(4, row);
                if(strcmp(constraint_attribute->data, attNamePar) == 0) {
                    start_value = AK_GetNth_L2(5, row);
                    end_value = AK_GetNth_L2(6, row);

                    if(strcmp(start_value->data, newValue) > 0) {
                        printf("\nFAILURE: Value '%s' is smaller than minimal allowed value: '%s' \n",newValue,start_value->data);
                        result = EXIT_FAILURE;
                    }
                    else if(strcmp(end_value->data, newValue) < 0) {
                        printf("\nFAILURE: Value '%s' is bigger than maximum allowed value: '%s' \n",newValue,end_value->data);
                        result = EXIT_FAILURE;
                    }
                }
            }
            
            AK_DeleteAll_L3(&row); // Properly free the row structure
            AK_free(row);
        }
    }

    AK_EPI;
    return result;
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

    if (tableName == NULL || constraintNamePar == NULL) {
        printf("ERROR: Invalid parameters passed to AK_delete_constraint_between\n");
        AK_EPI;
        return EXIT_ERROR;
    }

    if (strcmp(tableName, AK_CONSTRAINTS_BEWTEEN) != 0) {
        printf("ERROR: Invalid table name. Expected %s\n", AK_CONSTRAINTS_BEWTEEN);
        AK_EPI;
        return EXIT_ERROR;
    }

    if(AK_check_constraint_name(constraintNamePar, AK_CONSTRAINTS_BEWTEEN) == EXIT_SUCCESS){
        printf("ERROR: Constraint with name %s doesn't exist in table %s\n", constraintNamePar, tableName);
        AK_EPI;
        return EXIT_ERROR;
    }

    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
    if (row_root == NULL) {
        printf("ERROR: Memory allocation failed\n");
        AK_EPI;
        return EXIT_ERROR;
    }

    AK_Init_L3(&row_root);
    
    // Delete constraint by its name
    AK_Update_Existing_Element(TYPE_VARCHAR, constraintNamePar, tableName, "constraintName", row_root);
    int result = AK_delete_row(row_root);

    AK_DeleteAll_L3(&row_root);
    AK_free(row_root);    

    if (result == EXIT_SUCCESS) {
        printf("SUCCESS: Constraint %s successfully deleted\n", constraintNamePar);
    } else {
        printf("ERROR: Failed to delete constraint %s\n", constraintNamePar);
    }

    AK_EPI;
    return result;
}

/**
  * @author Saša Vukšić, updated by Mislav Jurinić, updated by Blaž Rajič, updated by Vilim Trakoštanec
  * @brief Function that tests the functionality of implemented between constraint.
  * @return No return value
  */
 TestResult AK_constraint_between_test(void) {
    AK_PRO;
    int passed=0,failed=0;

    /* Names & values */
    const char*tbl="department";
    const char*c1="dep_name_between";
    const char*a1="dep_name";
    const char*s1="Department of Economy";
    const char*e1="Department of Organization";
    const char*v1="Department of Zoo";

    const char*c2="manager_surname_between";
    const char*a2="manager";
    const char*s2="Hutinski";
    const char*e2="Redep";
    const char*v2="Kero";

    const char*c3="id_department_between";
    const char*a3="id_department";
    const char*s3="1";
    const char*e3="5";
    const char*v3="3";

    const char*v4="-1.2";

    typedef enum{OP_SET_READ,OP_READ,OP_DELETE_ALL}Op;
    typedef struct{Op op;const char*cn;const char*tbl;const char*at;const char*start;const char*end;const char*val;int exp;}T;

    T tests[]={
        {OP_SET_READ,c1,tbl,a1,s1,e1,v1,0},
        {OP_SET_READ,c2,tbl,a2,s2,e2,v2,1},
        {OP_SET_READ,c3,tbl,a3,s3,e3,v3,1},
        {OP_READ,NULL,tbl,a3,NULL,NULL,v4,0},
        {OP_DELETE_ALL,NULL,NULL,NULL,NULL,NULL,NULL,1}
    };
    size_t n=sizeof tests/sizeof *tests;

    for(size_t i=0;i<n;i++){
        T*t=&tests[i];
        printf("\n==== Running Test #%zu ====""\n",i+1);
        int r=EXIT_ERROR;
        if(t->op==OP_SET_READ){
            printf("Setting BETWEEN %s on %s.%s [%s..%s]...\n",t->cn,t->tbl,t->at,t->start,t->end);
            AK_set_constraint_between((char*)t->tbl,(char*)t->cn,(char*)t->at,(char*)t->start,(char*)t->end);
            AK_print_table(AK_CONSTRAINTS_BEWTEEN);
            printf("Testing %s on %s.%s expected %s...\n",t->val,t->tbl,t->at,t->exp?"pass":"fail");
            r=AK_read_constraint_between((char*)t->tbl,(char*)t->val,(char*)t->at);
            if((r==EXIT_SUCCESS)==t->exp){passed++;printf("SUCCESS\n");}else{failed++;printf("FAILED\n");}
        } else if(t->op==OP_READ){
            printf("Testing %s on %s.%s expected %s...\n",t->val,t->tbl,t->at,t->exp?"pass":"fail");
            r=AK_read_constraint_between((char*)t->tbl,(char*)t->val,(char*)t->at);
            if((r==EXIT_SUCCESS)==t->exp){passed++;printf("SUCCESS\n");}else{failed++;printf("FAILED\n");}
        } else {
            printf("Deleting all BETWEEN constraints...\n");
            const char*arr[]={c1,c2,c3};
            int ok=1;
            for(int j=0;j<3;j++)if(AK_delete_constraint_between(AK_CONSTRAINTS_BEWTEEN,(char*)arr[j])!=EXIT_SUCCESS)ok=0;
            AK_print_table(AK_CONSTRAINTS_BEWTEEN);
            if(ok){passed++;printf("SUCCESS\n");}else{failed++;printf("FAILED\n");}
        }
    }

    AK_EPI;
    return TEST_result(passed,failed);
}