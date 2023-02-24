//THIS FILE IS PART 2 OF REL_EQ_PROJECTION.C   ORIGINALLY IT WAS PART OF IT, BUT IN ORDER TO SPLIT THE CODE AND MAKE IT MORE READABLE IT MADE INTO 2 PARTS

#include "rel_eq_projection.h"
#include "../auxi/auxiliary.h"

/**
 * @author Dino Laktašić.
 * @brief Main function for generating RA expresion according to projection equivalence rules 
 * @param *list_rel_eq RA expresion as the AK_list
 * @return optimised RA expresion as the AK_list
 */
struct list_node *AK_rel_eq_projection(struct list_node *list_rel_eq) {
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
                        //Cascade of Projection p[L1](p[L2](...p[Ln](R)...)) = p[L1](R)
                        //[L1,...] < [L2,...] < [...,Ln-1,Ln]
                    case RO_PROJECTION:
                        if (temp_elem != NULL && temp_elem->type == TYPE_ATTRIBS) {
                            if (AK_rel_eq_is_subset(list_elem_next, temp_elem) == EXIT_FAILURE) {
                            	AK_InsertAtEnd_L3(list_elem->type, list_elem->data, list_elem->size, temp);
                                AK_InsertAtEnd_L3(list_elem_next->type, list_elem_next->data, list_elem_next->size, temp);
                                AK_dbg_messg(MIDDLE, REL_EQ, "::operator %s inserted with attributes (%s) in temp list\n", list_elem->data, list_elem_next->data);
                            }

                        } else {
                        	AK_InsertAtEnd_L3(list_elem->type, list_elem->data, list_elem->size, temp);
                            AK_InsertAtEnd_L3(list_elem_next->type, list_elem_next->data, list_elem_next->size, temp);
                            AK_dbg_messg(MIDDLE, REL_EQ, "::operator %s inserted with attributes (%s) in temp list\n", list_elem->data, list_elem_next->data);
                        }

                        list_elem = list_elem->next;
                        break;

                        //Commuting Selection with Projection p[L](s[L1](R)) = s[L1](p[L](R))
                    case RO_SELECTION:
                        //step = -1;
                        //check if selection uses only attributes retained by the projection before commuting
                        if (temp_elem != NULL) {
                            while (temp_elem != NULL) {
                                if (temp_elem->type == TYPE_OPERAND || temp_elem->type == TYPE_ATTRIBS) {
                                    //step++;
                                    temp_elem_prev = (struct list_node *) AK_Previous_L2(temp_elem, temp);

                                    if (temp_elem->type == TYPE_ATTRIBS) {
                                        if ((AK_rel_eq_can_commute(temp_elem, list_elem_next) == EXIT_FAILURE) &&
                                                (temp_elem_prev->data[0] == RO_PROJECTION) && (temp_elem_prev->type == TYPE_OPERATOR)) {
                                        	AK_InsertAtEnd_L3(list_elem->type, list_elem->data, list_elem->size, temp);
                                        	AK_InsertAtEnd_L3(list_elem_next->type, list_elem_next->data, list_elem_next->size, temp);
                                            AK_dbg_messg(MIDDLE, REL_EQ, "::operator %s inserted with condition (%s) in temp list\n", list_elem->data, list_elem_next->data);
                                            break;
                                        }
                                    }

                                    if (temp_elem_prev->data[0] == RO_PROJECTION && temp_elem_prev->type == TYPE_OPERATOR) {
                                        //if (step == 0) {
                                    	AK_InsertBefore_L2(list_elem->type, list_elem->data, list_elem->size, &temp_elem_prev, &temp);
                                    	AK_InsertBefore_L2(list_elem_next->type, list_elem_next->data, list_elem_next->size, &temp_elem_prev, &temp);
                                        AK_dbg_messg(MIDDLE, REL_EQ, "::operator %s inserted with condition (%s) in temp list\n", list_elem->data, list_elem_next->data);
                                        break;
                                        //}
                                    }
                                } else {
                                	AK_InsertAtEnd_L3(list_elem->type, list_elem->data, list_elem->size, temp);
                                	AK_InsertAtEnd_L3(list_elem_next->type, list_elem_next->data, list_elem_next->size, temp);
                                    AK_dbg_messg(MIDDLE, REL_EQ, "::operator %s inserted with condition (%s) in temp list\n", list_elem->data, list_elem_next->data);
                                    break;
                                }
                                temp_elem = (struct list_node *) AK_Previous_L2(temp_elem, temp);
                            }

                        } else {
                        	AK_InsertAtEnd_L3(list_elem->type, list_elem->data, list_elem->size, temp);
                        	AK_InsertAtEnd_L3(list_elem_next->type, list_elem_next->data, list_elem_next->size, temp);
                            AK_dbg_messg(MIDDLE, REL_EQ, "::operator %s inserted with condition (%s) in temp list\n", list_elem->data, list_elem_next->data);
                        }
                        list_elem = list_elem->next;
                        break;

                        //Distributing Projection over Union and Intersect p[L](R1 u R2) = (p[L](R1)) u (p[L](R2))
                    case RO_UNION:
                    case RO_INTERSECT:
                        step = -1;

                        while (temp_elem != NULL) {
                            if (temp_elem->type == TYPE_OPERAND || temp_elem->type == TYPE_ATTRIBS) {
                                step++;
                                temp_elem_prev = (struct list_node *) AK_Previous_L2(temp_elem, temp);

                                if (temp_elem_prev->data[0] == RO_PROJECTION && temp_elem_prev->type == TYPE_OPERATOR) {
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

                        //Commuting Projection with Join and Cartesian Product
                    case RO_NAT_JOIN:
                    	AK_InsertAtEnd_L3(list_elem->type, list_elem->data, list_elem->size, temp);
                        AK_InsertAtEnd_L3(list_elem_next->type, list_elem_next->data, list_elem_next->size, temp);
                        AK_dbg_messg(MIDDLE, REL_EQ, "::operator %s inserted in temp list\n", list_elem->data);
                        list_elem = list_elem->next;
                        break;

                    case RO_THETA_JOIN:
                        step = -1;

                        while (temp_elem != NULL) {
                            if (temp_elem->type == TYPE_OPERAND || temp_elem->type == TYPE_ATTRIBS) {
                                step++;
                                temp_elem_prev = (struct list_node *) AK_Previous_L2(temp_elem, temp);

                                if (temp_elem_prev->data[0] == RO_PROJECTION && temp_elem_prev->type == TYPE_OPERATOR) {
                                    if (step > 1) {
                                        // If projection list is of form L = L1 u L2, where L1 only has attributes of R,
                                        // and L2 only has attributes of S, provided join condition only contains attributes
                                        // of L, Projection and Theta join commute:
                                        // p[a](R Join S) = (p[a1]R) n (p[a2]R)
                                        int has_attributes = AK_rel_eq_can_commute(temp_elem, list_elem_next);

                                        //1. Get operator and its attributes
                                        tmp = temp_elem;
                                        temp_elem_next = temp_elem->next;

                                        //2. For each attribute in projection attributes that belongs to operand attributes, append
                                        //   attribute to new projection atributes list;
                                        char *data1 = AK_rel_eq_projection_attributes(temp_elem->data, temp_elem_next->data);
                                        char *data2 = AK_rel_eq_projection_attributes(temp_elem->data, (temp_elem_next->next)->data);

                                        if (data1 != NULL && data2 != NULL) {
                                            if (!has_attributes) {
                                                temp_elem->size = strlen(data1) + 1;
                                                memcpy(temp_elem->data, data1, temp_elem->size);
                                                memset(temp_elem->data + temp_elem->size, '\0', MAX_VARCHAR_LENGTH - temp_elem->size);
                                                AK_dbg_messg(MIDDLE, REL_EQ, "::operator %s inserted with attributes (%s) in temp list\n", temp_elem_prev->data, temp_elem->data);
                                            } else {
                                                //3. Insert new projection
                                                strcat(data1, ATTR_DELIMITER);
                                                strcat(data1, AK_rel_eq_projection_attributes(AK_rel_eq_collect_cond_attributes(list_elem_next), temp_elem_next->data));
                                                data1 = AK_rel_eq_remove_duplicates(data1);

                                                AK_InsertAfter_L2(temp_elem->type, data1, strlen(data1) + 1, &tmp, &temp);
                                                AK_InsertAfter_L2(temp_elem_prev->type, temp_elem_prev->data, temp_elem_prev->size, &tmp, &temp);
                                                AK_dbg_messg(MIDDLE, REL_EQ, "::operator %s inserted with attributes (%s) in temp list\n", temp_elem_prev->data, data1);
                                            }

                                            AK_free(data1);

                                            while (tmp->type != TYPE_OPERAND) {
                                                tmp = tmp->next;
                                            }

                                            if (has_attributes) {
                                                strcat(data2, ATTR_DELIMITER);
                                                strcat(data2, AK_rel_eq_projection_attributes(AK_rel_eq_collect_cond_attributes(list_elem_next), (tmp->next)->data));
                                                data2 = AK_rel_eq_remove_duplicates(data2);
                                            }
                                            memset(data2 + strlen(data2), '\0', 1);
                                            AK_InsertAfter_L2(temp_elem->type, data2, strlen(data2) + 1, &tmp, &temp);
                                            AK_InsertAfter_L2(temp_elem_prev->type, temp_elem_prev->data, temp_elem_prev->size, &tmp, &temp);
                                            AK_dbg_messg(MIDDLE, REL_EQ, "::operator %s inserted with attributes (%s) in temp list\n", temp_elem_prev->data, data2);
                                        }

                                        AK_free(data2);
                                    }
                                    break;
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

                    case RO_EXCEPT:
                    	AK_InsertAtEnd_L3(list_elem->type, list_elem->data, list_elem->size, temp);
                        AK_dbg_messg(MIDDLE, REL_EQ, "::operator %s inserted in temp list\n", list_elem->data);
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
    //If there is no new changes on the list return generated AK_lists
    //int iter_cond;
    //for (iter_cond = 0; iter_cond < sizeof(exit_cond); iter_cond++) {
    //	if (exit_cond[iter_cond] == 0) {
    ////	Edit function to return collection of the AK_lists
    ////	Generate next RA expr. (new plan)
    ////	temp += remain from the list_rel_eq
    //		AK_rel_eq_projection(temp);
    //	}
    //}

    AK_DeleteAll_L3(&list_rel_eq);
    AK_EPI;
    return temp;
}

/**
 * @author Dino Laktašić.
 * @brief Function for printing AK_list to the screen 
 * @param *list_rel_eq RA expresion as the AK_list
 * @return No return value
 */
void AK_print_rel_eq_projection(struct list_node *list_rel_eq) {
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
TestResult AK_rel_eq_projection_test() {
    AK_PRO;
    printf("rel_eq_projection.c: Present!\n");
    printf("\n********** REL_EQ_PROJECTION TEST by Dino Laktašić **********\n");

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
    temp = (AK_header*) AK_create_header("tezina", TYPE_FLOAT, FREE_INT, FREE_CHAR, FREE_CHAR);
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

    printf("rel_eq_projection_test: After segment initialization: %d\n", AK_num_attr(tblName));

    struct list_node *expr = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&expr);

    AK_InsertAtEnd_L3(TYPE_OPERATOR, "p", sizeof ("p"), expr);
    
    //The projection is only made up of a one or more of attributes
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "L1;L2;L3;L4", sizeof ("L1;L2;L3;L4"), expr); //projection attribute
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "p", sizeof ("p"), expr);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "L1;L4;L3;L2;L5", sizeof ("L1;L4;L3;L2;L5"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "s", sizeof ("s"), expr);

    /* The selection condition is made up of a number of clauses of the form
     * <attribute name> <comparison op> <constant value> OR
     * <attribute name 1> <comparison op> <attribute name 2>
     * In the clause, the comparison operations could be one of the following: ≤, ≥, ≠, =, >, < .
     * Clauses are connected by Boolean operators : and, or , not
     */
    AK_InsertAtEnd_L3(TYPE_CONDITION, "`L1` 100 > `L2` 50 < OR", sizeof ("`L1` 100 > `L2` 50 < OR"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "R", sizeof ("R"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "S", sizeof ("S"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "u", sizeof ("u"), expr);

    AK_InsertAtEnd_L3(TYPE_OPERATOR, "p", sizeof ("p"), expr);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "mbr;firstname;job", sizeof ("mbr;firstname;job"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "student", sizeof ("student"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "profesor", sizeof ("profesor"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "t", sizeof ("t"), expr);
    AK_InsertAtEnd_L3(TYPE_CONDITION, "`mbr` `job` =", sizeof ("`mbr` `job` ="), expr); //theta join attribute


    AK_print_rel_eq_projection(AK_rel_eq_projection(expr));

    if (DEBUG_ALL) {
        printf("\n------------------> TEST_PROJECTION_FUNCTIONS <------------------\n\n");

        //Initialize list elements...
        AK_list_elem list_elem_set, list_elem_subset;
        AK_list_elem list_elem_cond, list_elem_attr;

        char *test_cond1, *test_cond2;
        char *test_table;
        char *test_attribs;

        test_table = "profesor";
        test_cond1 = "`mbr` 100 > `firstname` 'Dino' = AND `id` 1000 > OR";
        test_cond2 = "`id` 100 > `firstname` 50 < AND `job` 'teacher' = AND";
        test_attribs = "id;mbr";

        printf("IS_SET_SUBSET_OF_LARGER_SET_TEST  : (%i)\n\n", AK_rel_eq_is_subset(list_elem_set, list_elem_subset));
        printf("COMMUTE_PROJECTION_SELECTION_TEST : (%i)\n\n", AK_rel_eq_can_commute(list_elem_attr, list_elem_cond));
        printf("GET_TABLE_ATTRIBUTES_TEST       : (%s)\n\n", (AK_rel_eq_get_attributes(test_table))->data);
        printf("GET_PROJECTION_ATTRIBUTES_TEST    : (%s)\n\n", AK_rel_eq_projection_attributes(test_attribs, test_table));
        printf("GET_ATTRIBUTES_FROM_CONDITION_TEST: (%s)\n\n", AK_rel_eq_collect_cond_attributes(list_elem_cond));
        printf("REMOVE_DUPLICATE_ATTRIBUTES_TEST  : (%s)\n", AK_rel_eq_remove_duplicates(test_attribs));
        
    } else {
        printf("...\n");
    }

    AK_DeleteAll_L3(&expr);
    AK_EPI;
    return TEST_result(success, failed);
}