/**
@file rel_eq_selection.c Provides functions for for relational equivalences in selection
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

#include "rel_eq_selection.h"
#include "../auxi/auxiliary.h"

/**
 * @author Dino Laktašić.
 * @brief Function that checks if some set of attributes is subset of larger set
 * <ol>
 * <li>Tokenize set and subset of projection attributes and store each of them to it's own array</li>
 * <li>Check if the size of subset array is larger than the size of set array</li>
 * <li>if the subset array is larger return 0</li>
 * <li>else sort both arrays ascending</li>
 * <li>Compare the subset and set items at the same positions, starting from 0</li>
 * <li>if there is an item in the subset array that doesn't match attribute at the same position in the set array return 0</li>
 * <li>else continue comparing until final item in the subset array is ritched</li>
 * <li>on loop exit return EXIT_SUCCESS</li>
 * </ol> 
 * @param *set set array 
 * @param *subset subset array 
 * @return EXIT_SUCCESS if some set of attributes is subset of larger set, else returns EXIT_FAILURE
 */
int AK_rel_eq_is_attr_subset(char *set, char *subset) {
    int len_set, len_subset;
    int set_id = 0;
    int subset_id = 0;
    char *temp_set, *temp_subset;
    char *token_set, *token_subset;
    char *save_token_set, *save_token_subset;
    char *tokens_set[MAX_TOKENS] = {NULL};
    char *tokens_subset[MAX_TOKENS] = {NULL};
    AK_PRO;
    if (set == NULL || subset == NULL) {
        AK_EPI;
        return EXIT_FAILURE;
    }

    len_set = len_subset = 0;

    temp_set = (char *) AK_calloc(strlen(set), sizeof (char));
    temp_subset = (char *) AK_calloc(strlen(subset), sizeof (char));

    memcpy(temp_set, set, strlen(set));
    memcpy(temp_subset, subset, strlen(subset));

    AK_dbg_messg(HIGH, REL_EQ, "RULE - is (%s) subset of set (%s) in rel_eq_selection\n", subset, set);

    for ((token_set = strtok_r(temp_set, ATTR_DELIMITER, &save_token_set)); token_set;
            (token_set = strtok_r(NULL, ATTR_DELIMITER, &save_token_set)), set_id++) {
        if (set_id < MAX_TOKENS - 1) {
            tokens_set[set_id] = token_set;
            len_set++;
        }
    }

    for ((token_subset = strtok_r(temp_subset, ATTR_DELIMITER, &save_token_subset)); token_subset;
            (token_subset = strtok_r(NULL, ATTR_DELIMITER, &save_token_subset)), subset_id++) {
        if (subset_id < MAX_TOKENS - 1) {
            tokens_subset[subset_id] = token_subset;
            len_subset++;
        }
    }

    if (len_set < len_subset) {
        AK_dbg_messg(HIGH, REL_EQ, "RULE - failed (%s) isn't subset of set (%s)!\n", subset, set);
        AK_EPI;
        return EXIT_FAILURE;
    }

    qsort(tokens_set, len_set, sizeof (char *), AK_strcmp);
    qsort(tokens_subset, len_subset, sizeof (char *), AK_strcmp);

    len_set = 0;

    for (subset_id = 0; tokens_subset[subset_id] != NULL; subset_id++) {
        for (set_id = 0; tokens_set[set_id] != NULL; set_id++) {
            if (strcmp(tokens_set[set_id], tokens_subset[subset_id]) == 0) {
                len_set++;
            }
        }
    }

    if (len_set != len_subset) {
        AK_dbg_messg(HIGH, REL_EQ, "RULE - failed (%s) isn't subset of set (%s)!\n", subset, set);
        AK_EPI;
        return EXIT_FAILURE;
    }

    AK_free(temp_set);
    AK_free(temp_subset);

    AK_dbg_messg(HIGH, REL_EQ, "RULE - succeed (%s) is subset of set (%s).\n", subset, set);
    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Dino Laktašić.
 * @brief Function that fetches attributes for a given table and store them to the char array 
 * <ol>
 * <li>Get the number of attributes in a given table</li>
 * <li>If there is no attributes return NULL</li>
 * <li>Get the table header for a given table</li>
 * <li>Initialize struct list_node</li>
 * <li>For each attribute in table header, insert attribute in the array</li>
 * <li>Delimit each new attribute with ";" (ATTR_DELIMITER)</li>
 * <li>return pointer to char array</li>
 * </ol>
 * @param *tblName name of the table
 * @result pointer to char array 
 */
char *AK_rel_eq_get_atrributes_char(char *tblName) {
    int len_attr, num_attr, next_attr;
    int next_address = 0;
    char *attr_name;
    AK_PRO;
    num_attr = AK_num_attr(tblName);

    if (num_attr == EXIT_WARNING) {
        AK_EPI;
        return NULL;
    }

    char *attr = (char *) AK_calloc(1, sizeof (char));
    AK_header *table_header = (AK_header *) AK_get_header(tblName);

    for (next_attr = 0; next_attr < num_attr; next_attr++) {
        attr_name = (table_header + next_attr)->att_name;
        len_attr = strlen(attr_name);

        attr = (char *) AK_realloc(attr, len_attr + next_address + 1);
        memcpy(attr + next_address, attr_name, len_attr);
        next_address += len_attr;

        if (next_attr < num_attr - 1) {
            memcpy(attr + next_address, ATTR_DELIMITER, 1);
            next_address++;
        } else {
            memcpy(attr + next_address, "\0", 1);
        }
    }

    AK_free(table_header);

    if (next_address > 0) {
        AK_EPI;
        return attr;
    } else {
        AK_free(attr);
        AK_EPI;
        return NULL;
    }
}

/**
 * @author Dino Laktašić.
 * @brief Function for filtering and returning attributes from condition 
 * @param *cond condition array that contains condition data
 * @result pointer to array that contains attributes for a given condition
 */
char *AK_rel_eq_cond_attributes(char *cond) {
    int next_chr = 0;
    int next_address = 0;
    int attr_end = -1;
    AK_PRO;
    if (cond == NULL) {
        AK_EPI;
        return NULL;
    }

    char *temp_cond = (char *) AK_malloc(strlen(cond));
    memcpy(temp_cond, cond, strlen(cond));
    char *attr = (char *) AK_malloc(sizeof (char));

    while (next_chr < strlen(cond)) {
        if (temp_cond[next_chr] == ATTR_ESCAPE) {
            next_chr++;
            if (++attr_end) {
                attr_end = -1;
            } else {
                if (next_address > 0) {
                    memcpy(attr + next_address++, ATTR_DELIMITER, 1);
                    attr = (char *) AK_realloc(attr, next_address + 1);
                }
            }
        }

        if (!attr_end) {
            memcpy(attr + next_address++, &temp_cond[next_chr], 1);
            attr = (char *) AK_realloc(attr, next_address + 1);
        }
        next_chr++;
    }

    AK_free(temp_cond);

    if (next_address > 0) {
        memcpy(attr + next_address, "\0", 1);
        AK_EPI;
        return attr;
    } else {
        AK_free(attr);
        AK_EPI;
        return NULL;
    }
}

/**
 * @author Dino Laktašić.
 * @brief Function that checks if two sets share one or more of it's attributes
 * <ol>
 * <li>If is empty set or subset returns EXIT_FAILURE</li>
 * <li>For each attribute in one set check if there is same attribute in the second set</li>
 * <li>If there is the same attribute return EXIT_SUCCESS</li>
 * <li>else remove unused pointers and return EXIT_FAILURE</li>
 * </ol>
 * @param *set first set of attributes delimited by ";" (ATTR_DELIMITER)
 * @param *subset second set of attributes delimited by ";" (ATTR_DELIMITER)
 * @result EXIT_SUCCESS if set and subset share at least one attribute, else returns EXIT_FAILURE 
 */
int AK_rel_eq_share_attributes(char *set, char *subset) {
    AK_PRO;
    if (set == NULL || subset == NULL) {
        AK_EPI;
        return EXIT_FAILURE;
    }

    char *temp, *temp_set, *temp_subset;
    char *token_set, *token_subset;
    char *save_token_set, *save_token_subset;

    temp = (char *) AK_calloc(strlen(subset), sizeof (char));
    temp_set = (char *) AK_calloc(strlen(set), sizeof (char));
    temp_subset = (char *) AK_calloc(strlen(subset), sizeof (char));

    memcpy(temp, subset, strlen(subset));
    memcpy(temp_set, set, strlen(set));
    memcpy(temp_subset, subset, strlen(subset));

    for ((token_set = strtok_r(temp_set, ATTR_DELIMITER, &save_token_set)); token_set;
            (token_set = strtok_r(NULL, ATTR_DELIMITER, &save_token_set))) {
        for ((token_subset = strtok_r(temp_subset, ATTR_DELIMITER, &save_token_subset)); token_subset;
                (token_subset = strtok_r(NULL, ATTR_DELIMITER, &save_token_subset))) {
            if (memcmp(token_set, token_subset, strlen(token_set)) == 0) {
                AK_free(temp);
                AK_EPI;
                return EXIT_SUCCESS;
            }
        }
        temp_subset = temp;
    }

    AK_free(temp);
    AK_EPI;
    return EXIT_FAILURE;
}



/**
 * @author Dino Laktašić.
 * @brief Function that checks if selection can commute with theta-join or product 
 * (if working with conditions in infix format use this function instead - also remember to change code at the other places)
 * <ol>
 * <li>For each token (delimited by " ") in selection condition first check if token represents attribute/s and is subset in the given table</li>
 * <li>If token is a subset set variable id to 1</li>
 * <li>else check if token differs from "OR", and if so, set id to 0, else make no changes to variable id</li>
 * <li>if token equals to "AND" and id equals to 1 append collected conds to result condition</li>
 * <li>else if token equals to "AND" and id equals to 0 discarge collected conds</li>
 * <li>else append token to collected data</li>
 * <li>When exits from loop if id greater then 0, append the last collected data to result</li>
 * <li>return pointer to char array that contains new condition for a given table
 * </ol>
 * @param *cond condition array that contains condition data 
 * @param *tblName name of the table
 * @return pointer to char array that contains new condition for a given table
 */
/*
char *AK_rel_eq_commute_with_theta_join(char *cond, char *tblName) {
       int id = 0;
       int token_id = 0;
       int attr_address = 0;
       int ret_address = 0;
       int len_token;
	
       char *token_cond, *save_token_cond;
       char *temp_attr = (char *)AK_calloc(1, sizeof(char));
       char *ret_attributes = (char *)AK_calloc(MAX_VARCHAR_LENGHT, sizeof(char));
       char *temp_cond = (char *)AK_calloc(strlen(cond) + 1, sizeof(char));
	
       memcpy(temp_cond, cond, strlen(cond));
       memcpy(temp_cond + strlen(cond) + 1, "\0", 1);
	
       dbg_messg(HIGH, REL_EQ, "RULE - commute selection (%s) with theta-join, table name (%s)\n", temp_cond, tblName);
	
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
                                               //if (strcmp(token_cond, "OR") != 0) {
                                                       id = 0;
                                               //}
                                       }
                               }
                               AK_free(tbl);
                               AK_free(attr);
                       }
			
                       len_token = strlen(token_cond);
			
                       if (strcmp(token_cond, "AND") == 0 && id > 0) {
                               memcpy(ret_attributes + ret_address, temp_attr, attr_address);
                               ret_address += attr_address;
                               memset(ret_attributes + ret_address, '\0', 1);
                               AK_free(temp_attr);
                               temp_attr = (char *)AK_calloc(len_token + 1, sizeof(char));
                               attr_address  = 1;
                               memcpy(temp_attr, " ", 1);
                               memcpy(temp_attr + attr_address, token_cond, len_token);
                               attr_address += len_token;
                       } else if (strcmp(token_cond, "AND") == 0 && id == 0) {
                               if (ret_address > 0) {
                                       AK_free(temp_attr);
                                       temp_attr = (char *)AK_calloc(len_token + 1, sizeof(char));
                                       attr_address  = 1;
                                       memcpy(temp_attr, " ", 1);
                                       memcpy(temp_attr + attr_address, token_cond, len_token);
                                       attr_address += len_token;
                               } else {
                                       AK_free(temp_attr);
                                       temp_attr = (char *)AK_calloc(1, sizeof(char));
                                       attr_address = 0;
                               }
                       } else {
                               if (attr_address > 0) {
                                       temp_attr = (char *)AK_realloc(temp_attr, attr_address + len_token + 1);
                                       memcpy(temp_attr + attr_address++, " ", 1);
                               } else {
                                       temp_attr = (char *)AK_realloc(temp_attr, attr_address + len_token);
                               }
				
                               memcpy(temp_attr + attr_address, token_cond, len_token);
                               attr_address += len_token;
                       }
               }
       }

       if (id > 0) {
               memcpy(ret_attributes + ret_address, temp_attr, attr_address);
               ret_address += attr_address;
               memcpy(ret_attributes + ret_address, "\0", 1);
       }
	
       AK_free(temp_attr);

       if (ret_attributes > 0) {
               dbg_messg(HIGH, REL_EQ, "RULE - commute selection with theta-join succeed.\n");
               return ret_attributes;
       } else {
               dbg_messg(HIGH, REL_EQ, "RULE - commute selection with theta-join failed!\n");
               //AK_free(ret_attributes);
               return NULL;
       }
}*/

/**
 * @author Dino Laktašić.
 * @brief Break conjunctive conditions to individual conditions 
 * (currently not used - commented in main AK_rel_eq_selection function), it can be usefull in some optimization cases
 * <ol>
 * <li>For each delimited item (' AND ') insert item to the struct list_node</li>
 * <li>Remove unused pointers and return the conditions list</li>
 * </ol>
 * @param *cond condition expression
 * @result conditions list
 */
//TODO: Move to .h
struct list_node *AK_rel_eq_split_condition(char *cond) {
    AK_PRO;
    struct list_node *list_attr = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&list_attr);

    int token_id = 0;
    int attr_address = 0;
    int len_token;
    char *token_cond, *save_token_cond;

    //it's much safe to allocate MAX_VARCHAR_LENGHT, and remove all AK_realloc from function
    char *temp_attr = (char *) AK_calloc(1, sizeof (char));

    char *temp_cond = (char *) AK_calloc(strlen(cond), sizeof (char));
    memcpy(temp_cond, cond, strlen(cond));

    for ((token_cond = strtok_r(temp_cond, " ", &save_token_cond)); token_cond;
            (token_cond = strtok_r(NULL, " ", &save_token_cond)), token_id++) {
        if (token_id < MAX_TOKENS - 1) {
            len_token = strlen(token_cond);

            if (strcmp(token_cond, "AND") == 0) {
            	AK_InsertAtEnd_L3(TYPE_CONDITION, temp_attr, strlen(temp_attr), list_attr);

                attr_address = 0;
                AK_free(temp_attr);
                temp_attr = (char *) AK_calloc(1, sizeof (char));
            } else {
                if (attr_address > 0) {
                    temp_attr = (char *) AK_realloc(temp_attr, attr_address + len_token + 2);
                    strcpy(temp_attr + attr_address, " ");
                    attr_address++;
                } else {
                    temp_attr = (char *) AK_realloc(temp_attr, attr_address + len_token + 1);
                }

                strcpy(temp_attr + attr_address, token_cond);
                attr_address += len_token;
            }
        }
    }

    AK_InsertAtEnd_L3(TYPE_CONDITION, temp_attr, strlen(temp_attr), list_attr);

    AK_free(temp_cond);
    AK_free(temp_attr);
    AK_EPI;
    return list_attr;
}

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
