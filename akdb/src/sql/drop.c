/**
 *@author Unknown, Jurica Hlevnjak - 
 drop table bugs fixed, reorganized code structure, 
 system catalog tables drop disabled, drop index added, 
 drop view added, drop sequence added, drop trigger added, drop_function added, 
 drop user added, drop group added, AK_drop_test updated
 * 
 *@file drop.c Provides DROP functions
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

#include "drop.h"

// list of system catalog tables
char *system_catalog[NUM_SYS_TABLES] = {
    "AK_relation",
    "AK_attribute",
    "AK_index",
    "AK_view",
    "AK_sequence",
    "AK_function",
    "AK_function_arguments",
    "AK_trigger",
    "AK_trigger_conditions",
    "AK_db",
    "AK_db_obj",
    "AK_user",
    "AK_group",
    "AK_user_group",
    "AK_user_right",
    "AK_group_right",
    "AK_constraints_between",
    "AK_constraints_not_null",
    AK_CONSTRAINTS_CHECK_CONSTRAINT,
    "AK_constraints_unique",
    "AK_reference"
};

/**
 * @author Unknown, Jurica Hlevnjak, updated by Tomislav Ilisevic, Maja Vračan
 * @brief Function for DROP table, index, view, sequence, trigger, function, user, group and constraint.
 * @param type drop type
 * @param drop_arguments arguments of DROP command
 */
int AK_drop(int type, AK_drop_arguments *drop_arguments) {

    char *sys_table;
    char *name;
    int result;
    AK_PRO;
    switch (type) {
        case DROP_TABLE:

            sys_table = "AK_relation";
            name = (char*) drop_arguments->value;

            int status = 0;
            int x = 0;

            if (AK_if_exist(name, sys_table) == 0) {
                printf("Table %s does not exist!\n", name);
            } else {
                for (x = 0; x < NUM_SYS_TABLES; x++) {
                    if (strcmp(name, system_catalog[x]) == 0) {
                        status = 1;
                        printf("Table %s is a System Catalog Table and can't be DROPPED!\n", name);
                        AK_EPI;
                        return EXIT_ERROR;
                        break;
                    }
                }
                if (status != 1) {
                    AK_drop_help_function(name, sys_table);
                    printf("TABLE %s DROPPED!\n", name);
                    AK_EPI;
                    return EXIT_SUCCESS;
                }

            }

            break;

        case DROP_INDEX:

            sys_table = "AK_index";
            name = (char*) drop_arguments->value;

            if (!AK_if_exist(name, sys_table) == 0) {
                AK_drop_help_function(name, sys_table);
                printf("INDEX %s DROPPED!\n", name);
                AK_EPI;
                return EXIT_SUCCESS;
            } else {
                printf("Index %s does not exist!\n", name);
                AK_EPI;
                return EXIT_ERROR;
            }

            break;

        case DROP_VIEW:

            sys_table = "AK_view";
            name = (char*) drop_arguments->value;

            if ((AK_table_empty(sys_table)) || (AK_if_exist(name, sys_table) == 0)) {
                printf("View %s does not exist!\n", name);
                AK_EPI;
                return EXIT_ERROR;
            } else {
                AK_view_remove_by_name(name);
                printf("VIEW %s DROPPED!\n", name);
                AK_EPI;
                return EXIT_SUCCESS;
            }

            break;

        case DROP_SEQUENCE:

            sys_table = "AK_sequence";
            name = (char*) drop_arguments->value;

            if ((AK_table_empty(sys_table)) || (AK_if_exist(name, sys_table) == 0)) {
                printf("Sequence %s does not exist!\n", name);
                AK_EPI;
                return EXIT_ERROR;
            } else {
                AK_sequence_remove(name);
                printf("SEQUENCE %s DROPPED!\n", name);
                AK_EPI;
                return EXIT_SUCCESS;
            }

            break;

        case DROP_TRIGGER:

            sys_table = "AK_trigger";
            char *sys_tbl2 = "AK_relation";

            name = (char*) drop_arguments->value;
            char *tbl = (char*) drop_arguments->next->value;

            if ((AK_table_empty(sys_table)) || (AK_if_exist(name, sys_table) == 0)) {
                printf("Trigger %s does not exist!\n", name);
                AK_EPI;
                return EXIT_ERROR;
            } else if ((AK_table_empty(sys_tbl2)) || (AK_if_exist(tbl, sys_tbl2) == 0)) {
                printf("Table %s does not exist!\n", tbl);
                AK_EPI;
                return EXIT_ERROR;
            } else {
                AK_trigger_remove_by_name(name, tbl);
                printf("TRIGGER %s DROPPED!\n", name);
                AK_EPI;
                return EXIT_SUCCESS;
            }

            break;

        case DROP_FUNCTION:

            sys_table = "AK_function";
            name = (char*) drop_arguments->value;
	    

	    struct list_node *args = (struct list_node *) AK_malloc(sizeof (struct list_node));
            AK_Init_L3(&args);

            if (drop_arguments != NULL) {
                while (drop_arguments != NULL) {
                    // printf("Argument: %s\n", drop_arguments->value);
                    drop_arguments = drop_arguments->next;
                    if (drop_arguments != NULL) {
                        
			AK_InsertAtEnd_L3(TYPE_VARCHAR, drop_arguments->value, sizeof (drop_arguments), args);
                    }
                }
            }

            if ((AK_table_empty(sys_table)) || (AK_if_exist(name, sys_table) == 0)) {
                printf("Function %s does not exist!\n", name);
                AK_EPI;
                return EXIT_ERROR;
            } else {
                if (AK_function_remove_by_name(name, args) == EXIT_SUCCESS) {
                    printf("FUNCTION %s DROPPED!\n", name);
                    AK_EPI;
                    return EXIT_SUCCESS;
                } else {
                    printf("Function does not exist!\n"); // returned EXIT_ERROR because the funcion with the given args was not found
                    AK_EPI;
                    return EXIT_ERROR;
                }
            }
            AK_free(args);
            break;

        case DROP_USER:

            sys_table = "AK_user";
            name = (char*) drop_arguments->value;

            int status1 = 0;

            if (strcmp(drop_arguments->next->value, "CASCADE") == 0) {
                status1 = 1;
            }

            if ((AK_table_empty(sys_table)) || (AK_if_exist(name, sys_table) == 0)) {
                printf("User %s does not exist!\n", name);
                AK_EPI;
                return EXIT_ERROR;
            } else {
                // if CASCADE
                if (status1 == 1) {
                    // revokes all user privileges
                    AK_revoke_all_privileges_user(name);
                    // remove user from group(s)
                    AK_remove_user_from_all_groups(name);

                    AK_user_remove_by_name(name);
                    printf("USER %s DROPPED!\n", name);
                    AK_EPI;
                    return EXIT_SUCCESS;

                } else { // if not CASCADE
                    // check if user has any privilege or belong to group
                    if (AK_check_user_privilege(name) == EXIT_SUCCESS) {
                        printf("User %s can not be dropped because it has related objects!\n", name);
                        AK_EPI;
                        return EXIT_ERROR;
                    } else {
                        AK_user_remove_by_name(name);
                        printf("USER %s DROPPED!\n", name);
                        AK_EPI;
                        return EXIT_SUCCESS;
                    }
                }
            }

            break;

        case DROP_GROUP:

            sys_table = "AK_group";
            name = (char*) drop_arguments->value;

            int status2 = 0;

            if (strcmp(drop_arguments->next->value, "CASCADE") == 0) {
                status2 = 1;
            }

            if ((AK_table_empty(sys_table)) || (AK_if_exist(name, sys_table) == 0)) {
                printf("Group %s does not exist!\n", name);
                AK_EPI;
                return EXIT_ERROR;
            } else {
                // if CASCADE
                if (status2 == 1) {
                    // revoke group privileges
                    AK_revoke_all_privileges_group(name);
                    // remove group users
                    AK_remove_all_users_from_group(name);

                    AK_group_remove_by_name(name);
                    printf("GROUP %s DROPPED!\n", name);
                    AK_EPI;
                    return EXIT_SUCCESS;
                } else { // if not CASCADE
                    // check if group has any privilege or have users
                    if (AK_check_group_privilege(name) == EXIT_SUCCESS) {
                        printf("Group %s can not be dropped because it has related objects!\n", name);
                        AK_EPI;
                        return EXIT_ERROR;
                    } else {
                        AK_group_remove_by_name(name);
                        printf("GROUP %s DROPPED!\n", name);
                        AK_EPI;
                        return EXIT_SUCCESS;
                    }
                }
            }

            break;

        case DROP_CONSTRAINT:

            sys_table = (char*) drop_arguments->value; // table
            name = (char*) drop_arguments->next->value; // atribute
            char *constName = (char*) drop_arguments->next->next->value; // constraint name
            char * newValue = drop_arguments->next->next->next->value; //new value
            int deleted = 0;

            if(AK_read_constraint_unique(sys_table, constName, name) == EXIT_SUCCESS){
                result = AK_delete_constraint_unique(sys_table, name, constName);
                if(result == EXIT_SUCCESS) {
                    printf("\nUnique constraint %s dropped", constName);
                    AK_EPI;
                    return EXIT_SUCCESS;
                }
                deleted = 1;
            }
            if(AK_read_constraint_not_null(sys_table, constName, name) == EXIT_SUCCESS){
                result = AK_delete_constraint_not_null(sys_table, name, constName);
                if(result == EXIT_SUCCESS) {
                    printf("\nNot null constraint %s dropped", constName);
                    AK_EPI;
                    return EXIT_SUCCESS;
                }
                
                deleted = 1;
            }
            if(AK_read_constraint_between(sys_table, newValue, name) == EXIT_SUCCESS){
                result = AK_delete_constraint_between(sys_table, constName, name);
                if(result == EXIT_SUCCESS) {
                    printf("\nBetween constraint %s dropped", constName);
                    AK_EPI;
                    return EXIT_SUCCESS;
                }
                deleted = 1;
            }
            AK_EPI;
            return EXIT_ERROR;
            // TODO: delete "check" constraints

            printf("Constraint '%s' in table '%s' %s!\n", (constName==""?name:constName), sys_table,(deleted==0?"does not exist":"DROPPED"));

            break;

        default:
            break;
    }
    
}

/**
 * @author unknown, Jurica Hlevnjak - fix bugs and reorganize code in this function
 * @brief Help function for the drop command. Delete memory blocks and addresses of table 
 * and removes table or index from system table.
 * @param tblName name of table or index
 * @param sys_table name of system catalog table
 */
void AK_drop_help_function(char *tblName, char *sys_table) {

    table_addresses *addresses;
    AK_PRO;
    addresses = (table_addresses*) AK_get_table_addresses(tblName);

    AK_mem_block *mem_block;

    int from = 0, to = 0, i = 0, j = 0, c;

    for (j = 0; j < MAX_EXTENTS_IN_SEGMENT; j++) {
        if (addresses->address_from != 0) {
            from = addresses->address_from[j];
            to = addresses->address_to[j];

            if (from == 0 || to == 0) break;
            for (i = from; i <= to; i++) {
                mem_block = (AK_mem_block *) AK_get_block(i);
                mem_block->block->type = BLOCK_TYPE_FREE;

                for (c = 0; c < DATA_BLOCK_SIZE; c++) {
                    mem_block->block->tuple_dict[c].type = FREE_INT;
                    mem_block->block->data[c] = FREE_CHAR;
                }
            }
        } else break;
    }

    int data_adr = 0;
    int data_size = 0;
    //int data_type = 0;
    int address_sys;
    char name_sys[MAX_ATT_NAME];

    AK_mem_block *mem_block2 = (AK_mem_block *) AK_get_block(0);

    for (i = 0; i < DATA_BLOCK_SIZE; i++) {
        memset(name_sys, 0, MAX_ATT_NAME);

        if (mem_block2->block->tuple_dict[i].address == FREE_INT) {
            break;
        }

        data_adr = mem_block2->block->tuple_dict[i].address;
        data_size = mem_block2->block->tuple_dict[i].size;

        memcpy(name_sys, mem_block2->block->data + data_adr, data_size);

        i++;
        data_adr = mem_block2->block->tuple_dict[i].address;
        data_size = mem_block2->block->tuple_dict[i].size;

        memcpy(&address_sys, mem_block2->block->data + data_adr, data_size);

        if (strcmp(name_sys, sys_table) == 0) {
            break;
        }
    }

    mem_block2 = (AK_mem_block *) AK_get_block(address_sys);
    table_addresses *addresses2;

    addresses2 = (table_addresses*) AK_get_table_addresses(tblName);

    for (i = 0; i < MAX_EXTENTS_IN_SEGMENT; i++) {
        addresses2->address_from[i] = 0;
        addresses2->address_to[i] = 0;
    }

    char name[MAX_VARCHAR_LENGTH];

    for (i = 0; i < DATA_BLOCK_SIZE; i++) {
        if (mem_block2->block->tuple_dict[i].type == FREE_INT)
            break;
        i++;
        memcpy(name, &(mem_block2->block->data[mem_block2->block->tuple_dict[i].address]), mem_block2->block->tuple_dict[i].size);
        name[ mem_block2->block->tuple_dict[i].size] = '\0';
        if (strcmp(name, tblName) == 0) {
            i++;
            mem_block2->block->data[mem_block2->block->tuple_dict[i].address] = 0;
            i++;
            mem_block2->block->data[mem_block2->block->tuple_dict[i].address] = 0;
        }
    }


    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node ));
    AK_Init_L3(&row_root);
    AK_DeleteAll_L3(&row_root);
    
    AK_Update_Existing_Element(TYPE_VARCHAR, tblName, sys_table, "name", row_root);

    AK_delete_row(row_root);
    AK_free(addresses);
    AK_free(addresses2);

    AK_EPI;
}

/**
 * @author Jurica Hlevnjak, updated by Tomislav Ilisevic
 * @brief Help function for checking if the element(view, function, sequence, user ...) exist in system catalog table
 * @param tblName name of table, index view, function, trigger, sequence, user, group or constraint
 * @param sys_table name of system catalog table
 * @return if element exist in system catalog returns 1, if not returns 0
 */
int AK_if_exist(char *tblName, char *sys_table) {
    AK_PRO;
    int num_rows = AK_get_num_records(sys_table);
    int a;
	struct list_node *el;
    for (a = 0; a < num_rows; a++) 
	{
        el = AK_get_tuple(a, 1, sys_table);
        if (!strcmp(tblName, el->data)) 
		{
			AK_DeleteAll_L3(&el);
			AK_free(el);
            return 1; // exist
        }
		AK_DeleteAll_L3(&el);
		AK_free(el);
    }
    AK_EPI;
    return 0; // not exist
}

/**
 * @author unknown, Jurica Hlevnjak - added all tests except drop table test, updated by Tomislav Ilisevic, Maja Vračan
 * @brief Function for testing all DROP functions
 */
TestResult AK_drop_test() {
    AK_PRO;
    printf("=========================================================\n");
    printf("========================DROP_TEST========================\n");
    int results[114];
    AK_drop_arguments *drop_arguments = (AK_drop_arguments *)AK_malloc(sizeof (AK_drop_arguments));
    drop_arguments->next = (AK_drop_arguments *)AK_malloc(sizeof (AK_drop_arguments));
    drop_arguments->value=(char*)"\0";

    drop_arguments->next->next = (AK_drop_arguments *)AK_malloc(sizeof (AK_drop_arguments));
    drop_arguments->next->value=(char*)"\0";

    drop_arguments->next->next->next = (AK_drop_arguments *)AK_malloc(sizeof (AK_drop_arguments));
    drop_arguments->next->next->value=(char*)"\0";

    drop_arguments->next->next->next->next =NULL;

    printf("\n-----DROP TABLE-----\n");
    AK_print_table("AK_relation");
    drop_arguments->value = "department";
    results[1] = AK_drop(DROP_TABLE, drop_arguments);

    AK_print_table("AK_relation");

    printf("\n-----DROP CATALOG TABLE-----\n");
    drop_arguments->value = "AK_attribute";
    results[2] = AK_drop(DROP_TABLE, drop_arguments);
    AK_print_table("AK_attribute");
    AK_print_table("AK_relation");

    printf("\n-----DROP VIEW-----\n");
    AK_print_table("AK_view");
    drop_arguments->value = "view300";
    results[3] = AK_drop(DROP_VIEW, drop_arguments);
    AK_print_table("AK_view");

    printf("\n-----DROP HASH INDEX-----\n");
    drop_arguments->value = "student_hash_index";
    AK_print_table("AK_index");

    results[4] = AK_drop(DROP_INDEX, drop_arguments);

    printf("\n-----DROP BITMAP INDEX-----\n");
    drop_arguments->value = "assistantfirstname_bmapIndex";
    AK_print_table("AK_index");

    results[5] = AK_drop(DROP_INDEX, drop_arguments);

    AK_print_table("AK_index");

    printf("\n-----DROP SEQUENCE-----\n");
    drop_arguments->value = "sekvenca5";
    AK_print_table("AK_sequence");
    results[6] = AK_drop(DROP_SEQUENCE, drop_arguments);
    AK_print_table("AK_sequence");

    printf("\n-----DROP TRIGGER-----\n");
    AK_print_table("AK_trigger");
    AK_print_table("AK_trigger_conditions");
    drop_arguments->value = "trigg4";
    drop_arguments->next->value = "AK_reference";
    results[7] = AK_drop(DROP_TRIGGER, drop_arguments);
    AK_print_table("AK_trigger");
    AK_print_table("AK_trigger_conditions");

    printf("\n-----DROP FUNCTION-----\n");
    AK_print_table("AK_function");
    AK_print_table("AK_function_arguments");
    drop_arguments->value = "test_funkcija";
    drop_arguments->next->value = "date";
    drop_arguments->next->next->value = "number";
    drop_arguments->next->next->next->value = "int";
    drop_arguments->next->next->next->next = NULL;
    results[8] = AK_drop(DROP_FUNCTION, drop_arguments);
    AK_print_table("AK_function");
    AK_print_table("AK_function_arguments");

    printf("\n-----DROP USER-----\n");
    drop_arguments->value = "kritakac";
    drop_arguments->next->value = "CASCADE";
    AK_print_table("AK_user");
    AK_print_table("AK_user_group");
    AK_print_table("AK_user_right");
    results[9] = AK_drop(DROP_USER, drop_arguments);
    AK_print_table("AK_user");
    AK_print_table("AK_user_group");
    AK_print_table("AK_user_right");

    // to avoid problems with transfering the previous value for the drop group
    drop_arguments->next->value = "asdfghj"; // so that cascade doesn't stay

    printf("\n-----DROP GROUP-----\n");
    drop_arguments->value = "grupa1";

    AK_print_table("AK_group");
    AK_print_table("AK_user_group");
    AK_print_table("AK_group_right");
    results[10] = AK_drop(DROP_GROUP, drop_arguments);
    AK_print_table("AK_group");
    AK_print_table("AK_user_group");
    AK_print_table("AK_group_right");

    printf("\n-----DROP CONSTRAINT-----\n");
    
    printf("\n-----UNIQUE-----\n");
    char* tableName_1 = "student";
    char* attName_1 = "year";
    char* constraintName_1 = "yearUnique";
    AK_set_constraint_unique(tableName_1, attName_1, constraintName_1);
    AK_print_table("AK_constraints_unique");
    
    drop_arguments->value = tableName_1;
    drop_arguments->next->value = attName_1;
    drop_arguments->next->next->value = constraintName_1;
    
    results[11] = AK_drop(DROP_CONSTRAINT, drop_arguments);
    AK_print_table("AK_constraints_unique");
    
    
    printf("\n-----NOT NULL-----\n");
    char* tableName_2 = "student";
    char* attName_2 = "firstname";
    char* constraintName_2 = "firstnameNotNull";
    AK_set_constraint_not_null(tableName_2, attName_2, constraintName_2);
    AK_print_table("AK_constraints_not_null");
    
    drop_arguments->value = tableName_2;
    drop_arguments->next->value = attName_2;
    drop_arguments->next->next->value = constraintName_2;
    
    results[12] = AK_drop(DROP_CONSTRAINT, drop_arguments);
    AK_print_table("AK_constraints_not_null");
    
    
    printf("\n-----BETWEEN-----\n");
    char* tableName_3 = "department";
    char* attName_3 = "manager";
    char* constraintName_3 = "manager_between";
    char* newValue_3 = "Kero";
    char* startValue_3 = "Hutinski";    
    char* endValue_3 = "Redep";
    AK_set_constraint_between(tableName_3, constraintName_3, attName_3, startValue_3, endValue_3);
    AK_print_constraints(tableName_3);
    
    drop_arguments->value = tableName_3;
    drop_arguments->next->value = attName_3;
    drop_arguments->next->next->value = constraintName_3;
    drop_arguments->next->next->next->value = newValue_3;
    
    results[13] = AK_drop(DROP_CONSTRAINT, drop_arguments);
    AK_print_constraints(tableName_3);
    

    int success=0;
    int failed=0;
    printf("======================END_DROP_TEST======================\n");
    printf("Test results: \n");
    int x=1;
    for (x; x<14; x++){
        if(results[x]== EXIT_SUCCESS){
            printf("Test %d: EXIT_SUCCESS \n", x);
            success++;
        }
        else{
            printf("Test %d: EXIT_ERROR \n", x);
            failed++;
        }
    }
    AK_EPI;
    return TEST_result(success,failed);
}
