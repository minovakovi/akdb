//THIS FILE IS PART 2 OF REL_EQ_COMUT.C   ORIGINALLY IT WAS PART OF IT, BUT IN ORDER TO SPLIT THE CODE AND MAKE IT MORE READABLE IT MADE INTO 2 PARTS

#include "rel_eq_comut.h"

/**
 * @author Dino Laktašić.
 * @brief Function that checks if the selection can commute with theta-join or product
 * <ol>
 * <li>For each token (delimited by " ") in selection condition first check if token represents attribute/s and is subset in the given table</li>
 * <li>If token is a subset set variable id to 1</li>
 * <li>else set id to 0, else make no changes to variable id</li>
 * <li>if token differs from "AND" and "OR" and id equals to 1 append current token to result condition</li>
 * <li>else if token equals to "AND" or "OR" and id equals to 1 and there are two added tokens add "AND" or "OR" to condition string</li>
 * <li>When exits from loop, return pointer to char array that contains new condition for a given table
 * </ol>
 * @param *cond condition array that contains condition data 
 * @param *tblName name of the table
 * @result pointer to char array that contains new condition for a given table
 */

char *AK_rel_eq_commute_with_theta_join(char *cond, char *tblName) {
    int id = 0;
    int token_id = 0;
    int next_cond = 0;
    AK_PRO;
    char *token_cond, *save_token_cond;
    char *ret_attributes = (char *) AK_calloc(MAX_VARCHAR_LENGTH, sizeof (char));
    char *temp_cond = (char *) AK_calloc(strlen(cond) + 1, sizeof (char));

    memcpy(temp_cond, cond, strlen(cond));
    memcpy(temp_cond + strlen(cond) + 1, "\0", 1);

    AK_dbg_messg(HIGH, REL_EQ, "RULE - commute selection (%s) with theta-join, table name (%s)\n", temp_cond, tblName);

    for ((token_cond = strtok_r(temp_cond, " ", &save_token_cond)); token_cond;
            (token_cond = strtok_r(NULL, " ", &save_token_cond)), token_id++) {
        if (token_id < MAX_TOKENS - 1) {
            if (*token_cond == ATTR_ESCAPE) {
                char *tbl = AK_rel_eq_get_atrributes_char(tblName);
                char *attr = AK_rel_eq_cond_attributes(token_cond);

                if (attr != NULL && tbl != NULL) {
                    if (!AK_rel_eq_is_attr_subset(tbl, attr)) { //if is subset set id to 1
                        id = 1;
                    } else {
                        id = 0;
                    }
                }
                AK_free(tbl);
                AK_free(attr);
            }
            //'category' 'teacher' = 'firstname' 'Dino' = AND 'lastname' 'Laktasic' = OR
            if ((strcmp(token_cond, "AND") != 0 && strcmp(token_cond, "OR") != 0) && id > 0) {
                if (strlen(ret_attributes)) {
                    strcat(ret_attributes, " ");
                }
                strcat(ret_attributes, token_cond);
                next_cond++;
            } else if ((strcmp(token_cond, "AND") == 0 || strcmp(token_cond, "OR") == 0) && next_cond % 2 == 0 && id > 0) {
                strcat(ret_attributes, " ");
                strcat(ret_attributes, token_cond);
            }
        }
    }

    if (ret_attributes > 0) {
        AK_dbg_messg(HIGH, REL_EQ, "RULE - commute selection with theta-join succeed.\n");
        AK_EPI;
        return ret_attributes;
    } else {
        AK_dbg_messg(HIGH, REL_EQ, "RULE - commute selection with theta-join failed!\n");
        AK_free(ret_attributes);
        AK_EPI;
        return NULL;
    }
}

/**
 * @author Dino Laktašić (AK_rel_eq_commute_with_theta_join), Davor Tomala (AK_rel_eq_comut)
 * @brief Function that tests relational equivalences regarding commutativity
 * @return No return vlaue
 */
TestResult AK_rel_eq_comut_test() {
    AK_PRO;
    printf("AK_rel_eq_commute_with_theta_join: Present!\n");
    //printf(AK_rel_eq_commute_with_theta_join("'mbr' 100 > 'firstname' 50 < AND 'id' 'A' > OR", "profesor"));
    printf("\n");

    printf("rel_eq_comut.c: Present!\n");
    printf("\n********** REL_EQ_COMMUTATION TEST **********\n");

    int success=0;
    int failed=0;
    int result;

    //create header
    AK_header t_header[MAX_ATTRIBUTES];
    AK_header* temp;

    temp = (AK_header*) AK_create_header("id", TYPE_INT, FREE_INT, FREE_CHAR, FREE_CHAR);
    memcpy(t_header, temp, sizeof ( AK_header));
    temp = (AK_header*) AK_create_header("firstname", TYPE_VARCHAR, FREE_INT, FREE_CHAR, FREE_CHAR);
    memcpy(t_header + 1, temp, sizeof ( AK_header));
    temp = (AK_header*) AK_create_header("job", TYPE_VARCHAR, FREE_INT, FREE_CHAR, FREE_CHAR);
    memcpy(t_header + 2, temp, sizeof ( AK_header));
    temp = (AK_header*) AK_create_header("year", TYPE_INT, FREE_INT, FREE_CHAR, FREE_CHAR);
    memcpy(t_header + 3, temp, sizeof ( AK_header));
    temp = (AK_header*) AK_create_header("weight", TYPE_FLOAT, FREE_INT, FREE_CHAR, FREE_CHAR);
    memcpy(t_header + 4, temp, sizeof ( AK_header));
    memset(t_header + 5, '\0', MAX_ATTRIBUTES - 5);

    //create table
    char *tblName = "profesor";

    int startAddress = AK_initialize_new_segment(tblName, SEGMENT_TYPE_TABLE, t_header);

    if (startAddress != EXIT_ERROR){
	    success++;
        printf("\nTABLE %s CREATED!\n", tblName);
    }else{
        failed++;
        printf("\nTest failed!\n");
    }

    printf("rel_eq_comut_test: After segment initialization: %d\n", AK_num_attr(tblName));

    struct list_node *expr = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&expr);

    AK_InsertAtEnd_L3(TYPE_OPERATOR, "p", sizeof ("p"), expr);
    /*
     * The comut is only made up of a one or more of attributes
     */
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "L1;L2;L3;L4", sizeof ("L1;L2;L3;L4"), expr); //comut attribute
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "p", sizeof ("p"), expr);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "L1;L4;L3;L2;L5", sizeof ("L1;L4;L3;L2;L5"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "s", sizeof ("s"), expr);

    /* The selection condition is made up of a number of clauses of the form
     * <attribute name> <comparison op> <constant value> OR
     * <attribute name 1> <comparison op> <attribute name 2>
     * In the clause, the comparison operations could be one of the following: ≤, ≥, ≠, =, >, < .
     * Clauses are connected by Boolean operators : and, or , not
     */
    AK_InsertAtEnd_L3(TYPE_CONDITION, "'L1' 100 > 'L2' 50 < OR", sizeof ("'L1' 100 > 'L2' 50 < OR"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "R", sizeof ("R"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "S", sizeof ("S"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "u", sizeof ("u"), expr);

    AK_InsertAtEnd_L3(TYPE_OPERATOR, "p", sizeof ("p"), expr);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "mbr;firstname;job", sizeof ("mbr;firstname;job"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "student", sizeof ("student"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "profesor", sizeof ("profesor"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "t", sizeof ("t"), expr);
    AK_InsertAtEnd_L3(TYPE_CONDITION, "'mbr' 'job' =", sizeof ("'mbr' 'job' ="), expr); //theta join attribute

    AK_print_rel_eq_comut(AK_rel_eq_comut(expr));

    if (DEBUG_ALL) {
        printf("\n------------------> TEST_COMUT_FUNCTIONS <------------------\n\n");

        char *test_cond1;//, *test_cond2;
        char *test_table;

        test_table = "profesor";
        test_cond1 = "'mbr' 100 > 'firstname' 'Markus' = AND 'id' 1000 > OR";

        printf("GET_ALL_TABLE_ATTRIBUTES_TEST   : (%s)\n\n", AK_rel_eq_get_atrributes_char(test_table));
        printf("GET_CONDITION_ATTRIBUTES_TEST   : (%s)\n\n", AK_rel_eq_cond_attributes(test_cond1));
        printf("COMMUTE_WITH_THETA_JOIN_TEST    : (%s)\n\n", AK_rel_eq_commute_with_theta_join(test_cond1, test_table));
        
    } else {
        printf("...\n");
    }

    AK_DeleteAll_L3(&expr);

    AK_EPI;
    return TEST_result(success, failed);
}