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
#include "rel_eq_selection_test.c"
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
