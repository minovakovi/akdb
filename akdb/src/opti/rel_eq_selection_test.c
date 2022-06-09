
//THIS FILE IS PART 2 OF REL_EQ_SELECTION.C   ORIGINALLY IT WAS PART OF IT, BUT IN ORDER TO SPLIT THE CODE AND MAKE IT MORE READABLE IT MADE INTO 2 PARTS


#include "rel_eq_selection.h"
#include "../auxi/auxiliary.h"
#include "rel_eq_comut_theta.c"

/**
 * @author Dino Laktašić.
 * @brief Main function for generating RA expresion according to selection equivalence rules 
 * @param *list_rel_eq RA expresion as the struct list_node
 * @return optimised RA expresion as the struct list_node
 */
struct list_node *AK_rel_eq_selection(struct list_node *list_rel_eq) {
    int step; //, exit_cond[5] = {0};
    AK_PRO;
    //Initialize temporary linked list
    struct list_node *temp = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&temp);

    struct list_node *tmp, *temp_elem, *temp_elem_prev, *temp_elem_next;
    struct list_node *list_elem_next, *list_elem = (struct list_node *) AK_First_L2(list_rel_eq);

    //Iterate through all the elements of RA linked list
    while (list_elem != NULL) {

        switch (list_elem->type) {

            case TYPE_OPERATOR:
                AK_dbg_messg(LOW, REL_EQ, "\nOPERATOR '%c' SELECTED\n", list_elem->data[0]);
                AK_dbg_messg(LOW, REL_EQ, "----------------------\n");
                temp_elem = (struct list_node *) AK_End_L2(temp);
                temp_elem_prev = (struct list_node *) AK_Previous_L2(temp_elem, temp);
                list_elem_next = (struct list_node *) AK_Next_L2(list_elem);

                switch (list_elem->data[0]) {
                    //Commutativity of Selection and Projection.
                    case RO_PROJECTION:
                        step = -1;

                        if (temp_elem != NULL) {
                            while (temp_elem != NULL) {
                                if (temp_elem->type == TYPE_OPERAND || temp_elem->type == TYPE_CONDITION) {
                                    if (temp_elem->type == TYPE_CONDITION) {
                                        temp_elem_prev = (struct list_node *) AK_Previous_L2(temp_elem, temp);

                                        if ((AK_rel_eq_can_commute(list_elem_next, temp_elem) == EXIT_FAILURE) &&
                                                (temp_elem_prev->data[0] == RO_SELECTION) && (temp_elem_prev->type == TYPE_OPERATOR)) {
                                        	AK_InsertAtEnd_L3(list_elem->type, list_elem->data, list_elem->size, temp);
                                            AK_InsertAtEnd_L3(list_elem_next->type, list_elem_next->data, list_elem_next->size, temp);
                                            AK_dbg_messg(MIDDLE, REL_EQ, "::operator %s inserted with condition (%s) in temp list\n", list_elem->data, list_elem_next->data);
                                            step++;
                                            break;
                                        } else if ((AK_rel_eq_can_commute(list_elem_next, temp_elem) == EXIT_SUCCESS) &&
                                                (temp_elem_prev->data[0] == RO_SELECTION) && (temp_elem_prev->type == TYPE_OPERATOR)) {
                                        	AK_InsertBefore_L2(list_elem->type, list_elem->data, list_elem->size, &temp_elem_prev, &temp);
                                        	AK_InsertBefore_L2(list_elem_next->type, list_elem_next->data, list_elem_next->size, &temp_elem_prev, &temp);
                                            AK_dbg_messg(MIDDLE, REL_EQ, "::operator %s inserted with condition (%s) in temp list\n", list_elem->data, list_elem_next->data);
                                            step++;
                                            break;
                                        }
                                    }
                                }
                                temp_elem = (struct list_node *) AK_Previous_L2(temp_elem, temp);
                            }
                        }

                        if (temp_elem == NULL || step != 0) {
                        	AK_InsertAtEnd_L3(list_elem->type, list_elem->data, list_elem->size, temp);
                            AK_InsertAtEnd_L3(list_elem_next->type, list_elem_next->data, list_elem_next->size, temp);
                            AK_dbg_messg(MIDDLE, REL_EQ, "::operator %s inserted with condition (%s) in temp list\n", list_elem->data, list_elem_next->data);
                        }

                        list_elem = list_elem->next;
                        break;

                        //Cascade of Selection and Commutativity of Selection
                    case RO_SELECTION:

                        //Join cascade selection conditions to one
                        if (temp_elem != NULL && temp_elem_prev != NULL && temp_elem->type == TYPE_CONDITION &&
                                temp_elem_prev->data[0] == RO_SELECTION && temp_elem_prev->type == TYPE_OPERATOR) {
                            temp_elem->size = temp_elem->size + list_elem_next->size + strlen(" AND") + 1; //edit to (" AND ")
                            //strcat(temp_elem->data, " AND "); //uncomment for infix use
                            strcat(temp_elem->data, " "); //remove for infix
                            strcat(temp_elem->data, list_elem_next->data);
                            strcat(temp_elem->data, " AND"); //comment if using infix format
                            memcpy(temp_elem->data, temp_elem->data, temp_elem->size);
                            AK_dbg_messg(MIDDLE, REL_EQ, "::selection cascade - condition changed to (%s) in temp list\n", temp_elem->data);
                        } else {
                        	AK_InsertAtEnd_L3(list_elem->type, list_elem->data, list_elem->size, temp);
                            AK_InsertAtEnd_L3(list_elem_next->type, list_elem_next->data, list_elem_next->size, temp);
                            AK_dbg_messg(MIDDLE, REL_EQ, "::operator %s inserted with attributes (%s) in temp list\n", list_elem->data, list_elem_next->data);
                        }

                        /* Divide selection condition (slower than upper solution but can be useful in certain cases)
                        list_split_sel = AK_rel_eq_split_condition(list_elem_next->data);
                        struct list_node *list_elem_split = (struct list_node *)FirstL(list_split_sel);
						
                        if (temp_elem != NULL) {
                                tmp = temp_elem;
						
                                while (list_elem_split != NULL) {
                                        step = 0;
								
                                        while (temp_elem != NULL) {
                                                if (temp_elem->type == TYPE_CONDITION || (temp_elem->data[0] == RO_SELECTION && temp_elem->type == TYPE_OPERATOR)) {
                                                        if (temp_elem->type == TYPE_CONDITION && strcmp(list_elem_next->data, temp_elem->data) == 0) {
                                                                step = 1;
                                                        }
                                                } else if (!step){
                                                        InsertAtEndL(list_elem->type , list_elem->data, list_elem->size, temp);
                                                        InsertAtEndL(list_elem_next->type, list_elem_split->data, list_elem_split->size, temp);
                                                        break;
                                                }
                                                temp_elem = (struct list_node *)PreviousL(temp_elem, temp);
                                        }
								
                                        list_elem_split = list_elem_split->next;
                                        temp_elem = tmp;
                                }
                        } else {
                                while (list_elem_split != NULL) {
                                        InsertAtEndL(list_elem->type , list_elem->data, list_elem->size, temp);
                                        InsertAtEndL(list_elem_next->type, list_elem_split->data, list_elem_split->size, temp);
                                        list_elem_split = list_elem_split->next;
                                }
                        }
                        DeleteAllL(list_split_sel);*/
                        list_elem = list_elem->next;
                        break;

                        //Commutativity of Selection and set operations (Union, Intersection, and Set difference)
                    case RO_UNION:
                    case RO_INTERSECT:
                    case RO_EXCEPT:
                        step = -1;

                        while (temp_elem != NULL) {
                            if (temp_elem->type == TYPE_OPERAND || temp_elem->type == TYPE_CONDITION) {
                                step++;
                                temp_elem_prev = (struct list_node *) AK_Previous_L2(temp_elem, temp);

                                if (temp_elem_prev->data[0] == RO_SELECTION && temp_elem_prev->type == TYPE_OPERATOR) {
                                    if (step > 1) {
                                        tmp = temp_elem;
                                        while (tmp->type != TYPE_OPERAND) {
                                            tmp = tmp->next;
                                        }
                                        AK_InsertAfter_L2(temp_elem->type, temp_elem->data, temp_elem->size, &tmp, &temp);
                                        AK_InsertAfter_L2(temp_elem_prev->type, temp_elem_prev->data, temp_elem_prev->size, &tmp, &temp);
                                        AK_dbg_messg(MIDDLE, REL_EQ, "::operator %s inserted with attributes (%s) in temp list\n", temp_elem_prev->data, temp_elem->data);
                                    }
                                    break;
                                }
                            } else {
                                break;
                            }
                            temp_elem = (struct list_node *) AK_Previous_L2(temp_elem, temp);
                        }
                        AK_InsertAtEnd_L3(list_elem->type, list_elem->data, list_elem->size, temp);
                        AK_dbg_messg(MIDDLE, REL_EQ, "::operator %s inserted in temp list\n", list_elem->data);
                        break;

                    case RO_NAT_JOIN:
                    	AK_InsertAtEnd_L3(list_elem->type, list_elem->data, list_elem->size, temp);
                        AK_InsertAtEnd_L3(list_elem_next->type, list_elem_next->data, list_elem_next->size, temp);
                        AK_dbg_messg(MIDDLE, REL_EQ, "::operator %s inserted in temp list\n", list_elem->data);
                        list_elem = list_elem->next;
                        break;

                        //Commutativity of Selection and Theta join (or Cartesian product)
                    case RO_THETA_JOIN:
                        step = -1;

                        while (temp_elem != NULL) {
                            if (temp_elem->type == TYPE_OPERAND || temp_elem->type == TYPE_CONDITION) {
                                step++;
                                temp_elem_prev = (struct list_node *) AK_Previous_L2(temp_elem, temp);

                                if (temp_elem_prev->data[0] == RO_SELECTION && temp_elem_prev->type == TYPE_OPERATOR) {
                                    if (step > 1) {
                                        tmp = temp_elem;
                                        temp_elem_next = temp_elem->next;

                                        char *data1, *data2;
                                        char *cond_attr1, *cond_attr2;
                                        char op_selected[2];
                                        memcpy(op_selected, temp_elem_prev->data, 2);

                                        data1 = AK_rel_eq_commute_with_theta_join(temp_elem->data, temp_elem_next->data);
                                        cond_attr1 = AK_rel_eq_cond_attributes(data1);

                                        data2 = AK_rel_eq_commute_with_theta_join(temp_elem->data, (temp_elem_next->next)->data);
                                        cond_attr2 = AK_rel_eq_cond_attributes(data2);

                                        if (AK_rel_eq_share_attributes(cond_attr1, cond_attr2)) {
                                            if (cond_attr1 != NULL) {
                                                temp_elem->size = strlen(data1) + 1;
                                                memcpy(temp_elem->data, data1, temp_elem->size);
                                                memset(temp_elem->data + temp_elem->size, '\0', MAX_VARCHAR_LENGTH - temp_elem->size);
                                                AK_dbg_messg(MIDDLE, REL_EQ, "::operator %s inserted with attributes (%s) in temp list\n", temp_elem_prev->data, temp_elem->data);
                                            } else {
                                                struct list_node *temp_elem_prevprev = (struct list_node *) AK_Previous_L2(temp_elem_prev, temp);
                                                temp_elem_prevprev->next = temp_elem;
                                                AK_free(temp_elem_prev);
                                                struct list_node *temp_elem_prev = temp_elem_prevprev;

                                                temp_elem_prev->next = temp_elem_next;
                                                AK_free(temp_elem);
                                                struct list_node *temp_elem = temp_elem_next;
                                                temp_elem_next = temp_elem->next;
                                                tmp = temp_elem;
                                            }

                                            while (tmp->type != TYPE_OPERAND) {
                                                tmp = tmp->next;
                                            }

                                            if (cond_attr2 != NULL) {
                                                memset(data2 + strlen(data2), '\0', 1);
                                                AK_InsertAfter_L2(temp_elem->type, data2, strlen(data2) + 1, &tmp, &temp);
                                                AK_InsertAfter_L2(TYPE_OPERATOR, op_selected, 2, &tmp, &temp);
                                                AK_dbg_messg(MIDDLE, REL_EQ, "::operator %s inserted with attributes (%s) in temp list\n", op_selected, data2);
                                            }
                                        }

                                        AK_free(data1);
                                        AK_free(data2);
                                        AK_free(cond_attr1);
                                        AK_free(cond_attr2);
                                        break;
                                    }
                                }
                            } else {
                                break;
                            }
                            temp_elem = (struct list_node *) AK_Previous_L2(temp_elem, temp);
                        }

                        AK_InsertAtEnd_L3(list_elem->type, list_elem->data, list_elem->size, temp);
                        AK_InsertAtEnd_L3(list_elem_next->type, list_elem_next->data, list_elem_next->size, temp);
                        AK_dbg_messg(MIDDLE, REL_EQ, "::operator %s inserted with condition (%s) in temp list\n", list_elem->data, list_elem_next->data);
                        list_elem = list_elem->next;
                        break;

                    case RO_RENAME:
                    	AK_InsertAtEnd_L3(list_elem->type, list_elem->data, list_elem->size, temp);
                        AK_dbg_messg(MIDDLE, REL_EQ, "::operator %s inserted in temp list\n", list_elem->data);
                        break;

                    default:
                        AK_dbg_messg(LOW, REL_EQ, "Invalid operator: %s", list_elem->data);
                        break;
                }
                break;

                //additional type definition included to distinguish beetween table name and attribute/s
            case TYPE_ATTRIBS:
                //printf("::attribute '%s' inserted in the temp list\n", list_elem->data);
                break;

                //additional type definition included to distinguish beetween attribute/s and condition
            case TYPE_CONDITION:
                //printf("::condition '%s' inserted in the temp list\n", list_elem->data);
                break;

            case TYPE_OPERAND:
                AK_dbg_messg(MIDDLE, REL_EQ, "::table_name (%s) inserted in the temp list\n", list_elem->data);
                AK_InsertAtEnd_L3(TYPE_OPERAND, list_elem->data, list_elem->size, temp);
                break;

            default:
                AK_dbg_messg(LOW, REL_EQ, "Invalid type: %s", list_elem->data);
                break;
        }

        list_elem = list_elem->next;
    }

    //====================================> IMPROVEMENTS <=======================================
    //Recursive RA optimization (need to implement exit condition in place of each operator, ...)
    //If there is no new changes on the list return generated struct list_nodes
    //int iter_cond;
    //for (iter_cond = 0; iter_cond < sizeof(exit_cond); iter_cond++) {
    //	if (exit_cond[iter_cond] == 0) {
    ////	Edit function to return collection of the struct list_nodes
    ////	Generate next RA expr. (new plan)
    ////	temp += remain from the list_rel_eq
    //		AK_rel_eq_selection(temp);
    //	}
    //}

    AK_DeleteAll_L3(&list_rel_eq);
    AK_EPI;
    return temp;
}

/**
 * @author Dino Laktašić.
 * @brief Function for printing struct list_node to the screen 
 * @param *list_rel_eq RA expresion as the struct list_node
 * @return void
 */
void AK_print_rel_eq_selection(struct list_node *list_rel_eq) {
    AK_PRO;
    struct list_node *list_elem = (struct list_node *) AK_First_L2(list_rel_eq);

    printf("\n");
    while (list_elem != NULL) {
        printf("Type: %i, size: %i, data: %s\n", list_elem->type, list_elem->size, list_elem->data);
        list_elem = list_elem->next;
    }
    AK_EPI;
}

/**
 * @author Dino Laktašić.
 * @brief Function for testing rel_eq_selection
 * @return No return value
 */
TestResult AK_rel_eq_selection_test() {
    AK_PRO;
    printf("rel_eq_selection.c: Present!\n");
    printf("\n********** REL_EQ_SELECTION TEST by Dino Laktašić **********\n");

    int success=0;
    int failed=0;
    int result;

    //create header
    AK_header t_header[MAX_ATTRIBUTES];
    AK_header *temp;

    temp = (AK_header*) AK_create_header("id", TYPE_INT, FREE_INT, FREE_CHAR, FREE_CHAR);
    memcpy(t_header, temp, sizeof (AK_header));
    temp = (AK_header*) AK_create_header("firstname", TYPE_VARCHAR, FREE_INT, FREE_CHAR, FREE_CHAR);
    memcpy(t_header + 1, temp, sizeof (AK_header));
    temp = (AK_header*) AK_create_header("job", TYPE_VARCHAR, FREE_INT, FREE_CHAR, FREE_CHAR);
    memcpy(t_header + 2, temp, sizeof (AK_header));
    temp = (AK_header*) AK_create_header("year", TYPE_INT, FREE_INT, FREE_CHAR, FREE_CHAR);
    memcpy(t_header + 3, temp, sizeof (AK_header));
    temp = (AK_header*) AK_create_header("tezina", TYPE_FLOAT, FREE_INT, FREE_CHAR, FREE_CHAR);
    memcpy(t_header + 4, temp, sizeof (AK_header));
    memset(t_header + 5, '\0', MAX_ATTRIBUTES - 5);

    //TEST 1
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

    printf("rel_eq_selection_test: After segment initialization: %d\n", AK_num_attr(tblName));

    //create header
    AK_header t_header2[MAX_ATTRIBUTES];
    AK_header *temp2;

    temp2 = (AK_header*) AK_create_header("mbr", TYPE_INT, FREE_INT, FREE_CHAR, FREE_CHAR);
    memcpy(t_header2, temp2, sizeof (AK_header));
    temp2 = (AK_header*) AK_create_header("firstname", TYPE_VARCHAR, FREE_INT, FREE_CHAR, FREE_CHAR);
    memcpy(t_header2 + 1, temp2, sizeof (AK_header));
    temp2 = (AK_header*) AK_create_header("lastname", TYPE_VARCHAR, FREE_INT, FREE_CHAR, FREE_CHAR);
    memcpy(t_header2 + 2, temp2, sizeof (AK_header));
    temp2 = (AK_header*) AK_create_header("year", TYPE_INT, FREE_INT, FREE_CHAR, FREE_CHAR);
    memcpy(t_header2 + 3, temp2, sizeof (AK_header));
    temp2 = (AK_header*) AK_create_header("weight", TYPE_FLOAT, FREE_INT, FREE_CHAR, FREE_CHAR);
    memcpy(t_header2 + 4, temp2, sizeof (AK_header));
    memset(t_header2 + 5, '\0', MAX_ATTRIBUTES - 5);

    //TEST 2
    //create table
    char *tblName2 = "student";

    int startAddress2 = AK_initialize_new_segment(tblName2, SEGMENT_TYPE_TABLE, t_header2);

    if (startAddress2 != EXIT_ERROR){
	    success++;
        printf("\nTABLE %s CREATED!\n", tblName2);
    }else{
        failed++;
        printf("\nTest failed!\n");
    }

    printf("rel_eq_selection_test: After segment initialization: %d\n", AK_num_attr(tblName2));
    //-----------------------------------------------------------------------------------------

    //Init list and insert elements (Query parser output)
    struct list_node *expr = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&expr);

    //Commutativity of Selection and Projection
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "s", sizeof ("s"), expr);
    AK_InsertAtEnd_L3(TYPE_CONDITION, "`L1` 100 >", sizeof ("`L1` 100 >"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "p", sizeof ("p"), expr);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "L1;L2;L3;L4", sizeof ("L1;L2;L3;L4"), expr); //projection attribute
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "p", sizeof ("p"), expr);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "L1;L4;L3;L2;L5", sizeof ("L1;L4;L3;L2;L5"), expr);

    //Cascade of Selection and Commutativity of Selection
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "s", sizeof ("s"), expr);
    AK_InsertAtEnd_L3(TYPE_CONDITION, "`L1` 100 >", sizeof ("`L1` 100 >"), expr);
    //
    //Commutativity of Selection and set operations (Union, Intersection, and Set difference)
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "s", sizeof ("s"), expr);
    AK_InsertAtEnd_L3(TYPE_CONDITION, "`L2` 100 > `L3` 50 < OR", sizeof ("`L2` 100 > `L3` 50 < OR"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "R", sizeof ("R"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "S", sizeof ("S"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "u", sizeof ("u"), expr); //u, i, e

    //Commutativity of Selection and Theta join (or Cartesian product)
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "s", sizeof ("s"), expr);
    AK_InsertAtEnd_L3(TYPE_CONDITION, "`job` 'teacher' = `mbr` 50 < AND", sizeof ("`job` 'teacher' = `mbr` 50 < AND"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "student", sizeof ("student"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "profesor", sizeof ("profesor"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "t", sizeof ("t"), expr);
    AK_InsertAtEnd_L3(TYPE_CONDITION, "`mbr` 50 = `job` 'teacher' = AND", sizeof ("`mbr` 50 = `job` 'teacher' = AND"), expr); //theta join attribute

    AK_print_rel_eq_selection(AK_rel_eq_selection(expr));

    if (DEBUG_ALL) {
        printf("\n------------------> TEST_SELECTION_FUNCTIONS <------------------\n\n");

        char *test_cond1, *test_cond2;
        char *test_table;
        char *cond_attr1, *cond_attr2;

        test_table = "profesor";
        test_cond1 = "`mbr` 100 > `firstname` 50 < AND `id` 'A' > OR";
        test_cond2 = "`id` 100 > `firstname` 50 < AND `job` 'teacher' = AND";

        cond_attr1 = AK_rel_eq_cond_attributes(test_cond1);
        cond_attr2 = AK_rel_eq_cond_attributes(test_cond2);

        printf("IS_SET_SUBSET_OF_LARGER_SET_TEST: (%i)\n\n", AK_rel_eq_is_attr_subset(cond_attr1, cond_attr2));
        printf("GET_ALL_TABLE_ATTRIBUTES_TEST   : (%s)\n\n", AK_rel_eq_get_atrributes_char(test_table));
        printf("GET_CONDITION_ATTRIBUTES_TEST   : (%s)\n\n", AK_rel_eq_cond_attributes(test_cond1));
        printf("COMMUTE_WITH_THETA_JOIN_TEST    : (%s)\n\n", AK_rel_eq_commute_with_theta_join(test_cond1, test_table));
        printf("CONDITION_SHARE_ATTRIBUTES_TEST : (%i)\n", AK_rel_eq_share_attributes(cond_attr1, cond_attr2));
        /**/
    } else {
        printf("...\n");
    }

    AK_DeleteAll_L3(&expr);

    AK_EPI;
    return TEST_result(success, failed);
}