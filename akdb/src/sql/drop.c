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
 *@updated by Antonio Brković
 *@file drop.c Provides DROP functions
*/

#include "drop.h"

// --- Definicije ---

#define MAX_EXTENTS 100
#define AK_INDEX_SYS_TABLE "AK_index"
#define AK_VIEW_SYS_TABLE "AK_view"
#define AK_SEQUENCE_SYS_TABLE "AK_sequence"
#define AK_TRIGGER_SYS_TABLE "AK_trigger"
#define AK_RELATION_SYS_TABLE "AK_relation"
#define AK_FUNCTION_SYS_TABLE "AK_function"
#define AK_USER_SYS_TABLE "AK_user"
#define AK_GROUP_SYS_TABLE "AK_group"
#define AK_CONSTRAINT_UNIQUE_SYS_TABLE "AK_constraints_unique"
#define AK_CONSTRAINT_NOT_NULL_SYS_TABLE "AK_constraints_not_null"
#define AK_CONSTRAINT_BETWEEN_SYS_TABLE "AK_constraints_between"
#define AK_CONSTRAINT_CHECK_SYS_TABLE "AK_constraints_check_constraint"

// --- System kataloge koji se ne smiju brisati ---
char *system_catalog[NUM_SYS_TABLES] = {
    "AK_relation", "AK_attribute", "AK_index", "AK_view", "AK_sequence", "AK_function",
    "AK_function_arguments", "AK_trigger", "AK_trigger_conditions", "AK_db", "AK_db_obj",
    "AK_user", "AK_group", "AK_user_group", "AK_user_right", "AK_group_right",
    "AK_constraints_between", "AK_constraints_not_null", "AK_constraints_check_constraint",
    "AK_constraints_unique", "AK_reference"
};

/**
 * @author Antonio Brković
 * @brief Pomoćna funkcija za alokaciju lančanih AK_drop_arguments čvorova
 */
AK_drop_arguments *allocate_drop_args_chain(int depth) {
    AK_drop_arguments *head = NULL, *current = NULL;
    for (int i = 0; i < depth; i++) {
        AK_drop_arguments *new_node = (AK_drop_arguments *)AK_malloc(sizeof(AK_drop_arguments));
        new_node->value = NULL;
        new_node->next = NULL;
        if (!head) {
            head = new_node;
            current = head;
        } else {
            current->next = new_node;
            current = current->next;
        }
    }
    return head;
}

/**
 * @author Antonio Brković
 * @brief Generic DROP function handler for simple catalog entities
 * @param drop_arguments pointer to arguments
 * @param sys_table name of system catalog table
 * @param remove_fn function to call for removal (can be NULL if not used)
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_generic_drop(AK_drop_arguments *drop_arguments, const char *sys_table, void (*remove_fn)(char *)) {
    if (!drop_arguments || !drop_arguments->value) return EXIT_ERROR;
    char *name = (char*) drop_arguments->value;

    if (AK_if_exist(name, (char *)sys_table) == 0) {
        printf("%s %s does not exist!\n", sys_table, name);
        return EXIT_ERROR;
    }

    // Provjera ako je u system_catalog
    for (int i = 0; i < NUM_SYS_TABLES; i++) {
        if (strcmp(name, system_catalog[i]) == 0) {
            printf("%s is a System Catalog Table and can't be dropped!\n", name);
            return EXIT_ERROR;
        }
    }

    if (remove_fn) remove_fn(name);
    else AK_drop_help_function(name, (char *)sys_table);

    printf("%s %s dropped!\n", sys_table, name);
    return EXIT_SUCCESS;
}

/**
 * @author Fran Turković
 * @updated by Antonio Brković
 * @brief Drop function for table
 */
int AK_drop_table(AK_drop_arguments *drop_arguments) {
    return AK_generic_drop(drop_arguments, "AK_relation", NULL);
}

/**
 * @author Fran Turković
 * @updated by Antonio Brković
 * @brief Drop function for index
 */
int AK_drop_index(AK_drop_arguments *drop_arguments) {
    return AK_generic_drop(drop_arguments, AK_INDEX_SYS_TABLE, NULL);
}

/**
 * @author Fran Turković
 * @updated by Antonio Brković
 * @brief Drop function for view
 */
int AK_drop_view(AK_drop_arguments *drop_arguments) {
    char *name = (char*) drop_arguments->value;

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
 * @author Fran Turković
 * @updated by Antonio Brković
 * @brief Drop function for sequence
 */
int AK_drop_sequence(AK_drop_arguments *drop_arguments) {
    char *name = (char*) drop_arguments->value;

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
 * @author Unknown, Jurica Hlevnjak
 * @updated by Antonio Brković
 * @brief Main dispatcher for DROP commands
 */
int AK_drop(int type, AK_drop_arguments *drop_arguments) {
    AK_PRO;

    typedef int (*DropFn)(AK_drop_arguments*);
    DropFn drop_functions[] = {
        AK_drop_table,     // DROP_TABLE = 0
        AK_drop_index,     // DROP_INDEX = 1
        AK_drop_view,      // DROP_VIEW = 2
        AK_drop_sequence,  // DROP_SEQUENCE = 3
        AK_drop_trigger,   // DROP_TRIGGER = 4
        AK_drop_function,  // DROP_FUNCTION = 5
        AK_drop_user,      // DROP_USER = 6
        AK_drop_group,     // DROP_GROUP = 7
        AK_drop_constraint // DROP_CONSTRAINT = 8
    };

    const int num_drop_types = sizeof(drop_functions) / sizeof(drop_functions[0]);
    int result = EXIT_ERROR;

    if (type >= 0 && type < num_drop_types) {
        result = drop_functions[type](drop_arguments);
    } else {
        printf("Invalid DROP type: %d\n", type);
    }

    AK_EPI;
    return result;
}

/**
 * @author Fran Turković
 * @updated by Antonio Brković
 */
int AK_drop_user(AK_drop_arguments *drop_arguments) {
    if (!drop_arguments || !drop_arguments->value) return EXIT_ERROR;
    char *name = (char*) drop_arguments->value;
    int cascade = 0;

    if (drop_arguments->next && drop_arguments->next->value &&
        strcmp(drop_arguments->next->value, "CASCADE") == 0) {
        cascade = 1;
    }

    if (AK_if_exist(name, AK_USER_SYS_TABLE) == 0) {
        printf("User %s does not exist!\n", name);
        return EXIT_ERROR;
    }

    if (cascade) {
        AK_revoke_all_privileges_user(name);
        AK_remove_user_from_all_groups(name);
    } else if (AK_check_user_privilege(name) == EXIT_SUCCESS) {
        printf("User %s cannot be dropped because it has related objects!\n", name);
        return EXIT_ERROR;
    }

    AK_user_remove_by_name(name);
    printf("User %s dropped!\n", name);
    return EXIT_SUCCESS;
}

/**
 * @author Fran Turković
 * @updated by Antonio Brković
 */
int AK_drop_group(AK_drop_arguments *drop_arguments) {
    if (!drop_arguments || !drop_arguments->value) return EXIT_ERROR;
    char *name = (char*) drop_arguments->value;
    int cascade = 0;

    if (drop_arguments->next && drop_arguments->next->value &&
        strcmp(drop_arguments->next->value, "CASCADE") == 0) {
        cascade = 1;
    }

    if (AK_if_exist(name, AK_GROUP_SYS_TABLE) == 0) {
        printf("Group %s does not exist!\n", name);
        return EXIT_ERROR;
    }

    if (cascade) {
        AK_revoke_all_privileges_group(name);
        AK_remove_all_users_from_group(name);
    } else if (AK_check_group_privilege(name) == EXIT_SUCCESS) {
        printf("Group %s cannot be dropped because it has related objects!\n", name);
        return EXIT_ERROR;
    }

    AK_group_remove_by_name(name);
    printf("Group %s dropped!\n", name);
    return EXIT_SUCCESS;
}

/**
 * @author Fran Turković
 * @updated by Antonio Brković
 */
int AK_drop_function(AK_drop_arguments *drop_arguments) {
    if (!drop_arguments || !drop_arguments->value) return EXIT_ERROR;
    char *name = (char*) drop_arguments->value;

    struct list_node* argList = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&argList);

    while (drop_arguments && drop_arguments->next) {
        drop_arguments = drop_arguments->next;
        AK_InsertAtEnd_L3(TYPE_VARCHAR, drop_arguments->value, sizeof(drop_arguments->value), argList);
    }

    if (AK_if_exist(name, AK_FUNCTION_SYS_TABLE) != 0 &&
        AK_function_remove_by_name(name, argList) == EXIT_SUCCESS) {
        printf("Function %s dropped!\n", name);
        AK_DeleteAll_L3(&argList);
        AK_free(argList);
        return EXIT_SUCCESS;
    }

    printf("Function %s does not exist or arguments mismatch!\n", name);
    AK_DeleteAll_L3(&argList);
    AK_free(argList);
    return EXIT_ERROR;
}

/**
 * @author Fran Turković
 * @updated by Antonio Brković
 */
int AK_drop_trigger(AK_drop_arguments *drop_arguments) {
    if (!drop_arguments || !drop_arguments->value || !drop_arguments->next || !drop_arguments->next->value)
        return EXIT_ERROR;

    char *name = (char*) drop_arguments->value;
    char *tableName = (char*) drop_arguments->next->value;

    if (AK_if_exist(name, AK_TRIGGER_SYS_TABLE) &&
        AK_if_exist(tableName, AK_RELATION_SYS_TABLE) &&
        !AK_table_empty(AK_TRIGGER_SYS_TABLE) &&
        !AK_table_empty(AK_RELATION_SYS_TABLE)) {
        AK_trigger_remove_by_name(name, tableName);
        printf("Trigger %s dropped!\n", name);
        return EXIT_SUCCESS;
    }

    printf("Trigger %s or table %s is invalid or empty!\n", name, tableName);
    return EXIT_ERROR;
}

/**
 * @author Fran Turković
 * @updated by Antonio Brković
 */
int AK_drop_constraint(AK_drop_arguments *drop_arguments){
    if (!drop_arguments || !drop_arguments->value || !drop_arguments->next || 
        !drop_arguments->next->next || !drop_arguments->next->next->next)
        return EXIT_ERROR;

    char *table_name = (char*) drop_arguments->value;
    char *constraint_name = (char*) drop_arguments->next->next->value;
    char *constraint_type = (char*) drop_arguments->next->next->next->value;

    int result = EXIT_ERROR;

    if (strcmp(constraint_type, "unique") == 0) {
        result = AK_delete_constraint_unique(AK_CONSTRAINT_UNIQUE_SYS_TABLE, constraint_name);
        if (result == EXIT_SUCCESS)
            printf("Unique constraint %s dropped from table %s.\n", constraint_name, table_name);
    } else if (strcmp(constraint_type, "not_null") == 0) {
        result = AK_delete_constraint_not_null(AK_CONSTRAINT_NOT_NULL_SYS_TABLE, constraint_name);
        if (result == EXIT_SUCCESS)
            printf("Not null constraint %s dropped from table %s.\n", constraint_name, table_name);
    } else if (strcmp(constraint_type, "between") == 0) {
        result = AK_delete_constraint_between(AK_CONSTRAINT_BETWEEN_SYS_TABLE, constraint_name);
        if (result == EXIT_SUCCESS)
            printf("Between constraint %s dropped from table %s.\n", constraint_name, table_name);
    } else if (strcmp(constraint_type, "check") == 0) {
        result = AK_delete_check_constraint(AK_CONSTRAINT_CHECK_SYS_TABLE, constraint_name);
        if (result == EXIT_SUCCESS)
            printf("Check constraint %s dropped from table %s.\n", constraint_name, table_name);
    } else {
        printf("Unknown constraint type '%s' on table '%s'.\n", constraint_type, table_name);
    }

    return result;
}

/**
 * @author Jurica Hlevnjak
 * @updated by Antonio Brković
 */
int AK_if_exist(char *tblName, char *sys_table) {
    AK_PRO;

    if (!tblName || !sys_table) return 0;

    int num_rows = AK_get_num_records(sys_table);
    struct list_node *el;

    for (int i = 0; i < num_rows; i++) {
        el = AK_get_tuple(i, 1, sys_table);
        if (el && strcmp(tblName, el->data) == 0) {
            AK_DeleteAll_L3(&el);
            AK_free(el);
            AK_EPI;
            return 1;
        }
        AK_DeleteAll_L3(&el);
        AK_free(el);
    }

    AK_EPI;
    return 0;
}

void AK_drop_help_function(char *tblName, char *sys_table) {
    if (!tblName || !sys_table) return;

    AK_PRO;

    // Dohvat adresa iz adresne tablice
    table_addresses *addresses = (table_addresses*) AK_get_table_addresses(tblName);
    if (!addresses) {
        AK_EPI;
        return;
    }

    // Oslobađanje memorijskih blokova (extents)
    for (int j = 0; j < MAX_EXTENTS; j++) {
        int from = addresses->address_from[j];
        int to = addresses->address_to[j];

        if (from == 0 || to == 0) break;

        for (int i = from; i <= to; i++) {
            AK_mem_block *mem_block = (AK_mem_block *) AK_get_block(i);
            if (!mem_block || !mem_block->block) continue;

            mem_block->block->type = BLOCK_TYPE_FREE;
            for (int c = 0; c < DATA_BLOCK_SIZE; c++) {
                mem_block->block->tuple_dict[c].type = FREE_INT;
                mem_block->block->data[c] = FREE_CHAR;
            }
        }
    }

    // Dohvat adrese system katalog tablice
    int address_sys = 0;
    char name_sys[MAX_ATT_NAME] = {0};
    AK_mem_block *mem_block2 = (AK_mem_block *) AK_get_block(0);

    for (int i = 0; i < DATA_BLOCK_SIZE; i += 2) {
        if (mem_block2->block->tuple_dict[i].address == FREE_INT) break;

        int data_adr = mem_block2->block->tuple_dict[i].address;
        int data_size = mem_block2->block->tuple_dict[i].size;
        memcpy(name_sys, mem_block2->block->data + data_adr, data_size);
        name_sys[data_size] = '\0';

        int addr_adr = mem_block2->block->tuple_dict[i + 1].address;
        int addr_size = mem_block2->block->tuple_dict[i + 1].size;
        memcpy(&address_sys, mem_block2->block->data + addr_adr, addr_size);

        if (strncmp(name_sys, sys_table, MAX_ATT_NAME) == 0) break;
    }

    // Očisti adrese
    table_addresses *addresses2 = (table_addresses*) AK_get_table_addresses(tblName);
    if (addresses2) {
        memset(addresses2->address_from, 0, sizeof(addresses2->address_from));
        memset(addresses2->address_to, 0, sizeof(addresses2->address_to));
    }

    // Pronađi i onesposobi zapis u katalogu
    AK_mem_block *sys_block = (AK_mem_block *) AK_get_block(address_sys);
    if (sys_block && sys_block->block) {
        for (int i = 0; i < DATA_BLOCK_SIZE; i++) {
            if (sys_block->block->tuple_dict[i].type == FREE_INT) break;
            i++; // preskoči tip

            char name[MAX_VARCHAR_LENGTH];
            memcpy(name, &sys_block->block->data[sys_block->block->tuple_dict[i].address], sys_block->block->tuple_dict[i].size);
            name[sys_block->block->tuple_dict[i].size] = '\0';

            if (strcmp(name, tblName) == 0) {
                i++;
                sys_block->block->data[sys_block->block->tuple_dict[i].address] = 0; // onesposobi zapis
                i++;
                sys_block->block->data[sys_block->block->tuple_dict[i].address] = 0;
            }
        }
    }

    // Briši redak iz system catalog tablice
    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row_root);
    AK_Update_Existing_Element(TYPE_VARCHAR, tblName, sys_table, "name", row_root);
    AK_delete_row(row_root);
    AK_DeleteAll_L3(&row_root);
    AK_free(row_root);

    // Oslobađanje svih adresnih struktura
    if (addresses) AK_free(addresses);
    if (addresses2) AK_free(addresses2);

    AK_EPI;
}

/**
 * @author unknown, Jurica Hlevnjak
 * @updated by Antonio Brković
 * @brief Function for testing all DROP functions
 */
TestResult AK_drop_test() {
    AK_PRO;

    printf("=========================================================\n");
    printf("========================DROP_TEST========================\n");

    int success = 0, failed = 0;

    #define RUN_DROP_TEST(IDX, NAME, COND) \
    do { \
        int _res = (COND); \
        if (_res == EXIT_SUCCESS) { \
            printf("Test %2d [%-25s] PASSED\n", (IDX), NAME); \
            success++; \
        } else { \
            printf("Test %2d [%-25s] FAILED\n", (IDX), NAME); \
            failed++; \
        } \
    } while (0)



    AK_drop_arguments *drop_args = allocate_drop_args_chain(4);

    // DROP TABLE
    drop_args->value = "department";
    RUN_DROP_TEST(0, "DROP TABLE", AK_drop(DROP_TABLE, drop_args));

    // DROP SYSTEM CATALOG TABLE
    drop_args->value = "AK_attribute";
    RUN_DROP_TEST(1,"DROP TABLE", !AK_drop(DROP_TABLE, drop_args));

    // DROP VIEW
    AK_view_add("view_drop_test", "SELECT firstname FROM profesor", "profesor;firstname;", 0);
    drop_args->value = "view_drop_test";
    RUN_DROP_TEST(2,"DROP TABLE", AK_drop(DROP_VIEW, drop_args));

    // DROP INDEX (hash & bitmap)
    drop_args->value = "student_hash_index";
    RUN_DROP_TEST(3,"DROP TABLE", AK_drop(DROP_INDEX, drop_args));
    drop_args->value = "assistantfirstname_bmapIndex";
    RUN_DROP_TEST(4, "DROP TABLE",AK_drop(DROP_INDEX, drop_args));

    // DROP SEQUENCE
    AK_sequence_add("seq_drop_test", 0, 5, 200, 0, 0);
    drop_args->value = "seq_drop_test";
    RUN_DROP_TEST(5, "DROP TABLE",AK_drop(DROP_SEQUENCE, drop_args));

    // DROP TRIGGER
    struct list_node *arg_list = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&arg_list);
    AK_function_add("function_drop_trigger", 1, arg_list);

    struct list_node *expr = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&expr);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), expr);
    AK_InsertAtEnd_L3(TYPE_INT, "2002", sizeof(int), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, ">", sizeof(">"), expr);

    AK_trigger_add("trigger_drop_test", "insert", expr, "AK_reference", "function_drop_trigger", arg_list);

    drop_args->value = "trigger_drop_test";
    drop_args->next->value = "AK_reference";
    RUN_DROP_TEST(6, "DROP TABLE",AK_drop(DROP_TRIGGER, drop_args));

    AK_DeleteAll_L3(&arg_list); AK_DeleteAll_L3(&expr);
    AK_free(arg_list); AK_free(expr);

    // DROP FUNCTION
    drop_args->value = "function_drop_trigger";
    drop_args->next = NULL;
    RUN_DROP_TEST(7, "DROP TABLE",AK_drop(DROP_FUNCTION, drop_args));

    // Realokacija kako bi se izbjegao SIGSEGV
    AK_free(drop_args);
    drop_args = allocate_drop_args_chain(4);

    // DROP USER
    AK_user_add("user_drop_test", "Lozinka1234@A", 1);
    drop_args->value = "user_drop_test";
    drop_args->next->value = "CASCADE";
    RUN_DROP_TEST(8, "DROP TABLE",AK_drop(DROP_USER, drop_args));

    // DROP GROUP
    AK_group_add("group_drop_test", 1);
    drop_args->value = "group_drop_test";
    drop_args->next->value = "CASCADE";
    RUN_DROP_TEST(9, "DROP TABLE",AK_drop(DROP_GROUP, drop_args));

    // CONSTRAINTS
    AK_set_constraint_unique("student", "year", "yearUnique");
    drop_args->value = "student";
    drop_args->next->value = "year";
    drop_args->next->next->value = "yearUnique";
    drop_args->next->next->next->value = "unique";
    RUN_DROP_TEST(10, "DROP TABLE",AK_drop(DROP_CONSTRAINT, drop_args));

    AK_set_constraint_not_null("student", "firstname", "firstnameNotNull");
    drop_args->value = "student";
    drop_args->next->value = "firstname";
    drop_args->next->next->value = "firstnameNotNull";
    drop_args->next->next->next->value = "not_null";
    RUN_DROP_TEST(11, "DROP TABLE",AK_drop(DROP_CONSTRAINT, drop_args));

    AK_set_constraint_between("department", "manager_between", "manager", "Hutinski", "Redep");
    drop_args->value = "department";
    drop_args->next->value = "manager";
    drop_args->next->next->value = "manager_between";
    drop_args->next->next->next->value = "between";
    RUN_DROP_TEST(12, "DROP TABLE",AK_drop(DROP_CONSTRAINT, drop_args));

    int constraint_value = 0;
    AK_set_check_constraint("department", "check_id_test", "id_department", ">", TYPE_INT, &constraint_value);
    drop_args->value = "department";
    drop_args->next->value = "id_department";
    drop_args->next->next->value = "check_id_test";
    drop_args->next->next->next->value = "check";
    RUN_DROP_TEST(13, "DROP TABLE",AK_drop(DROP_CONSTRAINT, drop_args));

    AK_DeleteAll_L3(&drop_args);
    AK_free(drop_args);

    printf("======================END_DROP_TEST======================\n");
    AK_EPI;
    return TEST_result(success, failed);
}