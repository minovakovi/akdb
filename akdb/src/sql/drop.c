/*
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/**
 *@author Unknown, Jurica Hlevnjak - 
 drop table bugs fixed, reorganized code structure, 
 system catalog tables drop disabled, drop index added, 
 drop view added, drop sequence added, drop trigger added, drop_function added, 
 drop user added, drop group added, AK_drop_test updated
 *@file drop.c Provides DROP functions
*/

#include "drop.h"

// an array that stores the names of system catalog tables --- TEST BROJ 41 
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
 * @author Unknown, Jurica Hlevnjak, updated by Tomislav Ilisevic, Maja Vračan, Fran Turković
 * @brief Function for DROP table, index, view, sequence, trigger, function, user, group and constraint.
 * @param type drop type
 * @param drop_arguments arguments of DROP command
 */

int AK_drop(int type, AK_drop_arguments *drop_arguments) {
    AK_PRO;
    int result;
    switch (type) {
        case DROP_TABLE:
            result = AK_drop_table(drop_arguments);
            break;
        case DROP_INDEX:
            result = AK_drop_index(drop_arguments);
            break;
        case DROP_VIEW:
            result = AK_drop_view(drop_arguments);
            break;
        case DROP_SEQUENCE:
            result = AK_drop_sequence(drop_arguments);
            break;
        case DROP_TRIGGER:
            result = AK_drop_trigger(drop_arguments);
            break;
        case DROP_FUNCTION:
            result = AK_drop_function(drop_arguments);
            break;
        case DROP_USER:
            result = AK_drop_user(drop_arguments);
            break;
        case DROP_GROUP:
            result = AK_drop_group(drop_arguments);
            break;
        case DROP_CONSTRAINT:
            result = AK_drop_constraint(drop_arguments);
            break;
        default:
            break;
    }
    AK_EPI;
    return result;   
}

/**
 * @author Fran Turković, updated by Andrej Hrebak Pajk
 * @brief Drop function that deletes specific table
 * @param drop_arguments arguments of DROP command
 */
int AK_drop_table(AK_drop_arguments *drop_arguments){
    char *sys_table = "AK_relation";
    char* name = (char*) drop_arguments->value;

    if (AK_if_exist(name, sys_table) == 0) {
        printf("Table %s does not exist!\n", name);
        return EXIT_ERROR;
    }
        for (int i = 0; i < NUM_SYS_TABLES; i++) {
            if (strcmp(name, system_catalog[i]) == 0) {
                printf("Table %s is a System Catalog Table and can't be dropped!\n", name);
                return EXIT_ERROR;
            }
        }
        
        AK_drop_help_function(name, sys_table);
        printf("Table %s dropped!\n", name);
        return EXIT_SUCCESS;    
}

/**
 * @author Fran Turković, updated by Andrej Hrebak Pajk
 * @brief Drop function that deletes specific index
 * @param drop_arguments arguments of DROP command 
 */
// using define for constant variable instead of hardcoding it
#define AK_INDEX_SYS_TABLE "AK_index"
int AK_drop_index(AK_drop_arguments *drop_arguments){
    char* name = (char*) drop_arguments->value;

    if (AK_if_exist(name, AK_INDEX_SYS_TABLE) != 0) {
        AK_drop_help_function(name, AK_INDEX_SYS_TABLE);
        printf("Index %s dropped!\n", name);
        return EXIT_SUCCESS;
    } else {
        printf("Index %s does not exist!\n", name);
        return EXIT_ERROR;
    }
}

/**
 * @author Fran Turković, updated by Andrej Hrebak Pajk
 * @brief Drop function that deletes specific view
 * @param drop_arguments arguments of DROP command 
 */
#define AK_VIEW_SYS_TABLE "AK_view"
int AK_drop_view(AK_drop_arguments *drop_arguments){
    char* name = (char*) drop_arguments->value;

    if (AK_if_exist(name, AK_VIEW_SYS_TABLE) != 0 && !AK_table_empty(AK_VIEW_SYS_TABLE)) {
        AK_view_remove_by_name(name);
        printf("View %s dropped!\n", name);
        return EXIT_SUCCESS;
    } else {
        printf("View %s does not exist!\n", name);
        return EXIT_ERROR;
    }
}

/**
 * @author Fran Turković, updated by Andrej Hrebak Pajk
 * @brief Drop function that deletes specific sequence
 * @param drop_arguments arguments of DROP command 
 */
#define AK_SEQUENCE_SYS_TABLE "AK_sequence"
int AK_drop_sequence(AK_drop_arguments *drop_arguments){
    char* name = (char*) drop_arguments->value;

    if (AK_if_exist(name, AK_SEQUENCE_SYS_TABLE) != 0 && !AK_table_empty(AK_SEQUENCE_SYS_TABLE)) {
        AK_sequence_remove(name);
        printf("Sequence %s dropped!\n", name);
        return EXIT_SUCCESS;
    } else {
        printf("Sequence %s does not exist!\n", name);
        return EXIT_ERROR;
    }
}

/**
 * @author Fran Turković, updated by Andrej Hrebak Pajk
 * @brief Drop function that deletes specific trigger
 * @param drop_arguments arguments of DROP command 
 */
#define AK_TRIGGER_SYS_TABLE "AK_trigger"
#define AK_RELATION_SYS_TABLE "AK_relation"
int AK_drop_trigger(AK_drop_arguments *drop_arguments){
    char* name = (char*) drop_arguments->value;
    char* tableName = (char*) drop_arguments->next->value;

    if (AK_if_exist(name, AK_TRIGGER_SYS_TABLE) != 0 && AK_if_exist(tableName, AK_RELATION_SYS_TABLE) != 0 && !AK_table_empty(AK_TRIGGER_SYS_TABLE) && !AK_table_empty(AK_RELATION_SYS_TABLE)) {
        AK_trigger_remove_by_name(name, tableName);
        printf("Trigger %s dropped!\n", name);
        return EXIT_SUCCESS;
    } else {
        if (AK_if_exist(name, AK_TRIGGER_SYS_TABLE) == 0) {
            printf("Trigger %s does not exist!\n", name);
        } else if (AK_if_exist(tableName, AK_RELATION_SYS_TABLE) == 0) {
            printf("Table %s does not exist!\n", tableName);
        } else {
            printf("Trigger %s does not exist or table %s is empty!\n", name, tableName);
        }
        return EXIT_ERROR;
    }
}

/**
 * @author Fran Turković, updated by Andrej Hrebak Pajk
 * @brief Drop function that deletes specific function
 * @param drop_arguments arguments of DROP command 
 */
#define AK_FUNCTION_SYS_TABLE "AK_function"
int AK_drop_function(AK_drop_arguments *drop_arguments){
    char* name = (char*) drop_arguments->value;
    struct list_node* argList = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&argList);

    while (drop_arguments != NULL && drop_arguments->next != NULL) {
        drop_arguments = drop_arguments->next;
        AK_InsertAtEnd_L3(TYPE_VARCHAR, drop_arguments->value, sizeof(drop_arguments), argList);
    }

    if (AK_if_exist(name, AK_FUNCTION_SYS_TABLE) != 0) {
        if (AK_function_remove_by_name(name, argList) == EXIT_SUCCESS) {
            printf("Function %s dropped!\n", name);
            AK_DeleteAll_L3(&argList);
            AK_free(argList);
            return EXIT_SUCCESS;
        } else {
            printf("Function with specified arguments does not exist!\n");
        }
    } else {
        printf("Function %s does not exist!\n", name);
    }

    AK_DeleteAll_L3(&argList);
    // used to free allocated memory
    AK_free(argList);
    return EXIT_ERROR;
}

/**
 * @author Fran Turković, updated by Andrej Hrebak Pajk
 * @brief Drop function that deletes specific user
 * @param drop_arguments arguments of DROP command 
 */
#define AK_USER_SYS_TABLE "AK_user"
int AK_drop_user(AK_drop_arguments *drop_arguments){
    char* name = (char*) drop_arguments->value;
    int cascade = 0;

    if (strcmp(drop_arguments->next->value, "CASCADE") == 0) {
        cascade = 1;
    }

    if (AK_if_exist(name, AK_USER_SYS_TABLE) != 0) {
        if (cascade) {
            // revoke all user privileges
            AK_revoke_all_privileges_user(name);
            // remove user from group(s)
            AK_remove_user_from_all_groups(name);
        } else {
            // check if user has any privilege or belongs to a group
            if (AK_check_user_privilege(name) == EXIT_SUCCESS) {
                printf("User %s cannot be dropped because it has related objects!\n", name);
                return EXIT_ERROR;
            }
        }
        // remove the user
        AK_user_remove_by_name(name);
        printf("User %s dropped!\n", name);
        return EXIT_SUCCESS;
    } else {
        printf("User %s does not exist!\n", name);
    }

    return EXIT_ERROR;
}

/**
 * @author Fran Turković, updated by Andrej Hrebak Pajk
 * @brief Drop function that deletes specific group
 * @param drop_arguments arguments of DROP command 
 */
#define AK_GROUP_SYS_TABLE "AK_group"
int AK_drop_group(AK_drop_arguments *drop_arguments){
    char* name = (char*) drop_arguments->value;
    int cascade = 0;

    if (strcmp(drop_arguments->next->value, "CASCADE") == 0) {
        cascade = 1;
    }

    if (AK_if_exist(name, AK_GROUP_SYS_TABLE) != 0) {
        if (cascade) {
            // revoke group privileges
            AK_revoke_all_privileges_group(name);
            // remove group users
            AK_remove_all_users_from_group(name);
        } else {
            // check if group has any privilege or users
            if (AK_check_group_privilege(name) == EXIT_SUCCESS) {
                printf("Group %s cannot be dropped because it has related objects!\n", name);
                return EXIT_ERROR;
            }
        }
        // remove the group
        AK_group_remove_by_name(name);
        printf("Group %s dropped!\n", name);
        return EXIT_SUCCESS;
    } else {
        printf("Group %s does not exist!\n", name);
    }

    return EXIT_ERROR;
}

/**
 * @author Fran Turković, updated by Andrej Hrebak Pajk
 * @brief Drop function that deletes specific group
 * @param drop_arguments arguments of DROP command 
 */
#define AK_CONSTRAINT_UNIQUE_SYS_TABLE "AK_constraints_unique"
#define AK_CONSTRAINT_NOT_NULL_SYS_TABLE "AK_constraints_not_null"
#define AK_CONSTRAINT_BETWEEN_SYS_TABLE "AK_constraints_between"
#define AK_CONSTRAINT_CHECK_SYS_TABLE "AK_constraints_check_constraint"
int AK_drop_constraint(AK_drop_arguments *drop_arguments){
    char* table_name = (char*) drop_arguments->value;
    char* constraint_name = (char*) drop_arguments->next->next->value;
    char* constraint_type = drop_arguments->next->next->next->value;
    
    int result = EXIT_ERROR;

    if (strcmp(constraint_type, "unique") == 0) {
        result = AK_delete_constraint_unique(AK_CONSTRAINT_UNIQUE_SYS_TABLE, constraint_name);
        if (result == EXIT_SUCCESS) {
            printf("Unique constraint %s dropped from table %s.\n", constraint_name, table_name);
        }
    } else if (strcmp(constraint_type, "not_null") == 0) {
        result = AK_delete_constraint_not_null(AK_CONSTRAINT_NOT_NULL_SYS_TABLE, constraint_name);
        if (result == EXIT_SUCCESS) {
            printf("Not null constraint %s dropped from table %s.\n", constraint_name, table_name);
        }
    } else if (strcmp(constraint_type, "between") == 0) {
        result = AK_delete_constraint_between(AK_CONSTRAINT_BETWEEN_SYS_TABLE, constraint_name);
        if (result == EXIT_SUCCESS) {
            printf("Between constraint %s dropped from table %s.\n", constraint_name, table_name);
        }
    } else if (strcmp(constraint_type, "check") == 0) {
        result = AK_delete_check_constraint(AK_CONSTRAINT_CHECK_SYS_TABLE, constraint_name);
        if (result == EXIT_SUCCESS) {
            printf("Check constraint %s dropped from table %s.\n", constraint_name, table_name);
        }
    } else {
        printf("Constraint '%s' in table '%s' does not exist.\n", constraint_name, table_name);
    }

    return result;
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

    #define MAX_EXTENTS 100

    for (j = 0; j < MAX_EXTENTS; j++) {
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

       if (strncmp(name_sys, sys_table, MAX_ATT_NAME) == 0) {
           break;
       }
    }

    mem_block2 = (AK_mem_block *) AK_get_block(address_sys);
    table_addresses *addresses2;

    addresses2 = (table_addresses*) AK_get_table_addresses(tblName);

    memset(addresses2->address_from, 0, sizeof(addresses2->address_from));
    memset(addresses2->address_to, 0, sizeof(addresses2->address_to));

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

    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row_root);
    AK_DeleteAll_L3(&row_root);
    
    AK_Update_Existing_Element(TYPE_VARCHAR, tblName, sys_table, "name", row_root);

    AK_delete_row(row_root);
    AK_free(row_root);
    AK_free(addresses);
    AK_free(addresses2);

    AK_EPI;
}

/**
 * @author Jurica Hlevnjak, updated by Tomislav Ilisevic, updated by AN
 * @brief Help function for checking if the element(view, function, sequence, user ...) exist in system catalog table
 * @param tblName name of table, index view, function, trigger, sequence, user, group or constraint
 * @param sys_table name of system catalog table
 * @return if element exist in system catalog returns 1, if not returns 0
 */
int AK_if_exist(char *tblName, char *sys_table) {
    AK_PRO;
    int num_rows = AK_get_num_records(sys_table);
    int rowIndex;
	struct list_node *el;

    for (rowIndex = 0; rowIndex < num_rows; rowIndex++) 
	{
        el = AK_get_tuple(rowIndex, 1, sys_table);
        if (strcmp(tblName, el->data) == 0) {
            AK_DeleteAll_L3(&el);
            AK_free(el);
            AK_EPI;
            return 1; // exists
        }
		AK_DeleteAll_L3(&el);
		AK_free(el);
    }
    AK_EPI;
    return 0; // not exist
}

/**
 * @author unknown, Jurica Hlevnjak - added all tests except drop table test, updated by Tomislav Ilisevic, Maja Vračan, Fran Turković
 * @brief Function for testing all DROP functions
 */
TestResult AK_drop_test() {
    AK_PRO;
    printf("=========================================================\n");
    printf("========================DROP_TEST========================\n");

    int results[14] = {0};
    AK_drop_arguments *drop_arguments = (AK_drop_arguments *)AK_malloc(sizeof(AK_drop_arguments));
    drop_arguments->next = (AK_drop_arguments *)AK_malloc(sizeof(AK_drop_arguments));
    drop_arguments->next->next = (AK_drop_arguments *)AK_malloc(sizeof(AK_drop_arguments));
    drop_arguments->next->next->next = NULL;
    drop_arguments->value = NULL;
    drop_arguments->next->value = NULL;
    drop_arguments->next->next->value = NULL;


    printf("\n-----DROP TABLE-----\n");
    AK_print_table("AK_relation");
    drop_arguments->value = "department";
    results[0] = AK_drop(DROP_TABLE, drop_arguments);
    AK_print_table("AK_relation");
    AK_print_table("department");

    printf("\n-----DROP CATALOG TABLE-----\n");
    drop_arguments->value = "AK_attribute";
    results[1] = !AK_drop(DROP_TABLE, drop_arguments);
    AK_print_table("AK_relation");

    printf("\n-----DROP VIEW-----\n");
    AK_view_add("view_drop_test","SELECT firstname FROM profesor", "profesor;firstname;", 0);
    AK_print_table("AK_view");
    drop_arguments->value = "view_drop_test";
    results[2] = AK_drop(DROP_VIEW, drop_arguments);
    AK_print_table("AK_view");

    // Sql index (create index) is not implemented yet
    printf("\n-----DROP HASH INDEX-----\n");
    drop_arguments->value = "student_hash_index";
    AK_print_table("AK_index");
    results[3] = AK_drop(DROP_INDEX, drop_arguments);
    AK_print_table("AK_index");

    // Sql index (create index) is not implemented yet
    printf("\n-----DROP BITMAP INDEX-----\n");
    drop_arguments->value = "assistantfirstname_bmapIndex";
    AK_print_table("AK_index");
    results[4] = AK_drop(DROP_INDEX, drop_arguments);
    AK_print_table("AK_index");

    // "AK_sequence_add" function is called before attempting to drop the sequence. This ensures that the sequence with the name "seq_drop_test" exists before attempting to drop it.
    printf("\n-----DROP SEQUENCE-----\n");
    AK_print_table("AK_sequence");
    AK_sequence_add("seq_drop_test", 0, 5, 200, 0, 0);
    AK_print_table("AK_sequence");
    drop_arguments->value = "seq_drop_test"; 
    results[5] = AK_drop(DROP_SEQUENCE, drop_arguments);
    AK_print_table("AK_sequence");

    printf("\n-----DROP TRIGGER-----\n");

    struct list_node *argument_list_function = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&argument_list_function);
    AK_function_add("function_drop_trigger", 1, argument_list_function);
    AK_print_table("AK_function");
    struct list_node *expr = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&expr);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), expr);
    AK_InsertAtEnd_L3(TYPE_INT, "2002", sizeof(int), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, ">", sizeof(">"), expr);
    AK_print_table("AK_trigger_conditions"); //Print trigger conditions before adding the trigger
    AK_trigger_add("trigger_drop_test", "insert", expr, "AK_reference", "function_drop_trigger", argument_list_function);
    AK_print_table("AK_trigger");
    AK_print_table("AK_trigger_conditions");
    drop_arguments->value = "trigger_drop_test";
    drop_arguments->next->value = "AK_reference";
    results[6] = AK_drop(DROP_TRIGGER, drop_arguments);
    AK_print_table("AK_trigger");
    AK_print_table("AK_trigger_conditions");
    AK_DeleteAll_L3(&argument_list_function);
    AK_DeleteAll_L3(&expr);
    AK_free(argument_list_function);
    AK_free(expr);


    printf("\n-----DROP FUNCTION-----\n");
    AK_print_table("AK_function");
    AK_print_table("AK_function_arguments");
    drop_arguments->value = "function_drop_trigger";
    drop_arguments->next = NULL;
    results[7] = AK_drop(DROP_FUNCTION, drop_arguments);
    AK_print_table("AK_function");
    AK_print_table("AK_function_arguments");

    printf("\n-----DROP USER-----\n");
    AK_user_add("user_drop_test",1234,1);
    drop_arguments->value = "user_drop_test";
    drop_arguments->next = (AK_drop_arguments *)AK_malloc(sizeof (AK_drop_arguments));
    drop_arguments->next->value = "CASCADE";
    AK_print_table("AK_user");
    AK_print_table("AK_user_group");
    AK_print_table("AK_user_right");
    results[8] = AK_drop(DROP_USER, drop_arguments);
    AK_print_table("AK_user");
    AK_print_table("AK_user_group");
    AK_print_table("AK_user_right");

    // The value is set for the drop group to an empty string to avoid any potential issues
    drop_arguments->next->value = "";
    printf("\n-----DROP GROUP-----\n");
    AK_group_add("group_drop_test", 1);
    AK_print_table("AK_group");
    AK_print_table("AK_user_group");
    AK_print_table("AK_group_right");
    results[9] = AK_drop(DROP_GROUP, drop_arguments);
    AK_print_table("AK_group");
    AK_print_table("AK_user_group");
    AK_print_table("AK_group_right");


    printf("\n-----DROP GROUP-----\n");
    AK_group_add("group_drop_test",1);
    drop_arguments->value = "group_drop_test";
    AK_print_table("AK_group");
    AK_print_table("AK_user_group");
    AK_print_table("AK_group_right");
    results[9] = AK_drop(DROP_GROUP, drop_arguments);
    AK_print_table("AK_group");
    AK_print_table("AK_user_group");
    AK_print_table("AK_group_right");

    printf("\n-----DROP CONSTRAINT-----\n");
    
    printf("\n-----UNIQUE-----\n");
    char* tableName_1 = "student";
    char* attName_1 = "year";
    char* constraintName_1 = "yearUnique";
    char* constrainType_1 = "unique";
    AK_set_constraint_unique(tableName_1, attName_1, constraintName_1);
    AK_print_table("AK_constraints_unique");
    
    drop_arguments->value = tableName_1;
    drop_arguments->next->value = attName_1;
    drop_arguments->next->next = (AK_drop_arguments *)AK_malloc(sizeof (AK_drop_arguments));
    drop_arguments->next->next->value = constraintName_1;
    drop_arguments->next->next->next = (AK_drop_arguments *)AK_malloc(sizeof (AK_drop_arguments));
    drop_arguments->next->next->next->value = constrainType_1;
    results[10] = AK_drop(DROP_CONSTRAINT, drop_arguments);
    AK_print_table("AK_constraints_unique");
    
    
    printf("\n-----NOT NULL-----\n");
    char* tableName_2 = "student";
    char* attName_2 = "firstname";
    char* constraintName_2 = "firstnameNotNull";
    char* constrainType_2 = "not_null";
    AK_set_constraint_not_null(tableName_2, attName_2, constraintName_2);
    AK_print_table("AK_constraints_not_null");
    
    drop_arguments->value = tableName_2;
    drop_arguments->next->value = attName_2;
    drop_arguments->next->next->value = constraintName_2;
    drop_arguments->next->next->next->value = constrainType_2;

    results[11] = AK_drop(DROP_CONSTRAINT, drop_arguments);
    AK_print_table("AK_constraints_not_null");
    
    
    printf("\n-----BETWEEN-----\n");
    char* tableName_3 = "department";
    char* attName_3 = "manager";
    char* constraintName_3 = "manager_between";
    char* constrainType_3 = "between";
    char* startValue_3 = "Hutinski";    
    char* endValue_3 = "Redep";
    AK_set_constraint_between(tableName_3, constraintName_3, attName_3, startValue_3, endValue_3);
    AK_print_table("AK_constraints_between");
    
    drop_arguments->value = tableName_3;
    drop_arguments->next->value = attName_3;
    drop_arguments->next->next->value = constraintName_3;
    drop_arguments->next->next->next->value = constrainType_3;
    
    results[12] = AK_drop(DROP_CONSTRAINT, drop_arguments);
    AK_print_table("AK_constraints_between");

    printf("\n-----CHECK-----\n");
    char* tableName_4 = "department";
    char* attName_4 = "id_department";
    char* constraintName_4 = "check_id_test";
    char* constrainType_4 = "check";
    AK_set_check_constraint(tableName_4,constraintName_4,attName_4,">",TYPE_INT,0);
    AK_print_table("AK_constraints_check_constraint");
    
    drop_arguments->value = tableName_4;
    drop_arguments->next->value = attName_4;
    drop_arguments->next->next->value = constraintName_4;
    drop_arguments->next->next->next->value = constrainType_4;
    
    results[13] = AK_drop(DROP_CONSTRAINT, drop_arguments);
    AK_print_table("AK_constraints_check_constraint");
    

    int success=0;
    int failed=0;
    printf("======================END_DROP_TEST======================\n");

    printf("Test results: \n");
    for (int x = 0; x<=13; x++){
        if(results[x] == EXIT_SUCCESS){
            printf("Test %d: EXIT_SUCCESS \n", x);
            success++;
        }
        else{
            printf("Test %d: EXIT_ERROR \n", x);
            failed++;
        }
    }
    AK_EPI;
    return TEST_result(success, failed);
}
