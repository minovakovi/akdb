/**
@file query_optimization.c Provides functions for general query optimization
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

#include "query_optimization.h"

int error_message=0;

/**
 * @author Dino Laktašić.
 * @brief Function that prints optimization table for testing purposes
 * @param *list_query optimized RA expresion list 
 * @result list output
 */
void AK_print_optimized_query(struct list_node *list_query) {
    AK_PRO;
    struct list_node *list_elem = (struct list_node *) AK_First_L2(list_query);

    int length;
    int len[] = {strlen("Type"), strlen("Size"), strlen("Data")};
    list_elem = (struct list_node *) AK_First_L2(list_query);

    printf("==>");
    while (list_elem != NULL) {
        length = AK_chars_num_from_number(list_elem->type, 10);
        if (len[0] < length) {
            len[0] = length;
        }

        length = AK_chars_num_from_number(list_elem->size, 10);
        if (len[1] < length) {
            len[1] = length;
        }

        if (len[2] < strlen(list_elem->data)) {
            len[2] = strlen(list_elem->data);
        }

        if (list_elem->type == TYPE_ATTRIBS || list_elem->type == TYPE_CONDITION) {
            printf("(%s)", list_elem->data);
        } else {
            printf(" %s", list_elem->data);
        }
        list_elem = list_elem->next;
    }

    //7 is number of char | + space in printf
    //set offset to change the box size
    length = len[0] + len[1] + len[2] + 3 * TBL_BOX_OFFSET + 7;

    printf("\n%i REL_EQ OPTIMIZED QUERY TABLE", (length / 2) - (strlen("REL_EQ OPTIMIZED QUERY TABLE") / 2));
    printf("\n| %-*s| %-*s| %-*s|\n", len[0] + TBL_BOX_OFFSET, "Type",
            len[1] + TBL_BOX_OFFSET, "Size", len[2] + TBL_BOX_OFFSET, "Data");
    AK_print_row_spacer(len, length);

    list_elem = (struct list_node *) AK_First_L2(list_query);

    while (list_elem != NULL) {
        printf("\n| %-*i| %-*i| %-*s|\n", len[0] + TBL_BOX_OFFSET, list_elem->type,
                len[1] + TBL_BOX_OFFSET, list_elem->size, len[2] + TBL_BOX_OFFSET, list_elem->data);
        AK_print_row_spacer(len, length);
        list_elem = list_elem->next;
    }
    AK_EPI;
}

/**
 * @author Dino Laktašić.
 * @brief Function that calls and executes relation equivalence
 * RELATION EQUIVALENCE RULES FLAGS
 * c - commutation
 * a - associativity
 * p - projection
 * s - selection	
 * @param *list_query RA expresion list where we need to apply relational equivalences rules
 * @param rel_eq rel_eq to execute 
 * @param *FLAGS flags for relation equivalences (execute rel_eq for given flags) 
 * @return returns struct list_node (RA expresion list) optimized by given relational equivalence rule 
*/ 
struct list_node *AK_execute_rel_eq(struct list_node *list_query, const char rel_eq, const char *FLAGS) {
    AK_PRO;
    AK_dbg_messg(LOW, REL_EQ, "\nATTEMPT TO EXECUTE '%c' AS RELATIONAL EQUIVALENCE\n", rel_eq);
    AK_dbg_messg(LOW, REL_EQ, "=================================================\n");

    if (strchr(FLAGS, rel_eq) != NULL) {
        switch (rel_eq) {
            case 'c':
                AK_dbg_messg(LOW, REL_EQ, "\napply rel_eq_commute.\n");
                AK_EPI;
                return (struct list_node *)AK_rel_eq_comut(list_query);
                break;

            case 'a':
                AK_dbg_messg(LOW, REL_EQ, "\napply rel_eq_assoc.\n");
                AK_EPI;
                return (struct list_node *) AK_rel_eq_assoc(list_query);
                break;

            case 'p':
                AK_dbg_messg(LOW, REL_EQ, "\napply rel_eq_projection.\n");
                AK_EPI;
                return (struct list_node *) AK_rel_eq_projection(list_query);
                break;

            case 's':
                AK_dbg_messg(LOW, REL_EQ, "\napply rel_eq_selection.\n");
                AK_EPI;
                return (struct list_node *) AK_rel_eq_selection(list_query);
                break;

            default:
                AK_dbg_messg(LOW, REL_EQ, "Invalid relational equivalence flag: %c", rel_eq);
				error_message++;
                AK_EPI;
                return list_query;
                break;
        }
    }
    AK_EPI;
}
    
/**
 * @author Dino Laktašić.
 * @brief Function that executes all relational equivalences provided by FLAGS (one or more), 
 * if DIFF_PLANS turned on execute permutations without repetition on given RA list from SQL parser output
 * @param *list_query RA expresion list where we need to apply relational equivalences rules
 * @param *FLAGS flags for relation equivalences (execute rel_eq for given flags) 
 * @return returns AK_list (RA expresion list) optimized by all relational equivalence rules provided by FLAGS 
 * (commented code can be edited so AK_list can return the list of lists (lists of different optimization plans),
 * with permutation switched on (DIFF_PLANS = 1) time for execution will be significantly increased
 * Current implementation without uncommenting code doesn't produce list of list, 
 * it rather apply all permutations on the same list
 * 
 * For futher development consider to implement cost estimation for given plan based on 
 * returned heuristicly optimized list
 */
struct list_node *AK_query_optimization(struct list_node *list_query, const char *FLAGS, const int DIFF_PLANS) {
    int num_perms = 1;
    int div;
    int next_perm,sum, next_flag;
    AK_PRO;
    int len_flags = strlen(FLAGS);

    struct list_node *temp = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&temp);
    
    //change view name for its rel_exp in list_query if exists
    //@author Danko Sacer
    struct list_node *list_elem = (struct list_node *) AK_First_L2(list_query);
    list_elem = (struct list_node *) AK_First_L2(list_query);
    
    while (list_elem != NULL) {
 	//suposed that views have name prefiks or sufics 'view' (viewname_view or view_viewname)
    	if (list_elem->type == TYPE_OPERAND && strstr(list_elem->data,"view")!=NULL){
		//list_elem->data change value from view name to view query
		
		char* record = AK_get_relation_expression(list_elem->data);
		strcpy(list_elem->data, record);
	}
	
    	list_elem = list_elem->next;
    }

    temp = list_query;

    //Get total permutation without repetition number
    for (next_flag = 1; next_flag <= len_flags; num_perms *= next_flag++);

    struct list_node *temps[num_perms];

    if (num_perms > MAX_PERMUTATION) {
        AK_dbg_messg(LOW, REL_EQ, "ERROR: max four flags are allowed!\n");
		error_message++;
        AK_EPI;
        return temp;
    }

    for (next_perm = 0; next_perm < num_perms; next_perm++) {
        char *perm = (char *) AK_calloc(len_flags, sizeof (char));
        memcpy(perm, FLAGS, len_flags);

        AK_dbg_messg(LOW, REL_EQ, "\n\t==============================\n\t\t%i. LOGIC PLAN\n\t==============================\n", next_perm + 1 );
        for (next_flag = len_flags, div = num_perms; next_flag > 0; next_flag--) {
            div /= next_flag;
            int index = (next_perm / div) % next_flag;

            if (DIFF_PLANS) {
            temps[next_perm] = (struct list_node *)AK_malloc(sizeof(struct list_node));
            AK_Init_L3(&temps[next_perm]);
            temps[next_perm] = AK_execute_rel_eq(temp, perm[index], FLAGS);
            AK_print_optimized_query(temps[next_perm]);
	    
            } else {
            temp = AK_execute_rel_eq(temp, perm[index], FLAGS);
            AK_print_optimized_query(temp);
            }

            memmove(perm + index, perm + index + 1, len_flags - index);
        }

        if (!DIFF_PLANS) {
            break;
        }
    }

    if (DIFF_PLANS) {
        AK_dbg_messg(LOW, REL_EQ, "\nTOTAL REL_EQ OPTIMIZED PLANS: %i\n", num_perms);
	
        temp = (struct list_node *)AK_realloc(temp, num_perms * sizeof(temps));
	sum=num_perms * sizeof(temps);
        memcpy(temp,temps,sum);
    }

    AK_DeleteAll_L3(&list_query);
    AK_EPI;
    return temp;
}
/**
  * @author Dino Laktašić
  * @param Function for testing *list_query query to be optimized
  * @return No return value
  */
TestResult AK_query_optimization_test() { 
    AK_PRO;
    printf("query_optimization.c: Present!\n");
    printf("\n********** QUERY OPTIMIZATION TEST by Dino Laktašić **********\n");

    if (DEBUG_ALL) {
        printf("\n------------------> TEST_REL_EQ_FUNCTIONS <------------------\n\n");

        char *test_attribs;
        char *test_cond1, *test_cond2;
        char *test_table;
        char *cond_attr1, *cond_attr2;

        test_table = "professor";
        test_attribs = "id_prof;firstname";
        test_cond1 = "`id_prof` 35897 < `category` '10' = AND";
        test_cond2 = "`mbr` 35891 > `id_prof` 35895 < OR";

        cond_attr1 = (char *) AK_rel_eq_cond_attributes(test_cond1);
        cond_attr2 = (char *) AK_rel_eq_cond_attributes(test_cond2);

        //Initialize list elements...
        struct list_node *list_elem_set, *list_elem_subset;
        struct list_node *list_elem_cond, *list_elem_attr;

        printf("Test table: (%s) | Test attributes: (%s)\n", test_table, test_attribs);
        printf("Test first condition       : (%s)\n", test_cond1);
        printf("Test second condition      : (%s)\n\n", test_cond2);
        printf("First condition attributes : (%s)\n", cond_attr1);
        printf("Second condition attributes: (%s)\n", cond_attr2);
        printf("-------------------------------------------------------------\n\n");

        printf("IS_SET_SUBSET_OF_LARGER_SET_TEST  : (%i)\n\n", AK_rel_eq_is_subset(list_elem_set, list_elem_subset));
        printf("COMMUTE_PROJECTION_SELECTION_TEST : (%i)\n\n", AK_rel_eq_can_commute(list_elem_attr, list_elem_cond));
        printf("GET_TABLE_ATTRIBUTES_TEST       : (%s)\n\n", (AK_rel_eq_get_attributes(test_table))->data);
        printf("GET_PROJECTION_ATTRIBUTES_TEST    : (%s)\n\n", AK_rel_eq_projection_attributes(test_attribs, test_table));
        printf("GET_ATTRIBUTES_FROM_CONDITION_TEST: (%s)\n\n", AK_rel_eq_collect_cond_attributes(list_elem_cond));
        printf("REMOVE_DUPLICATE_ATTRIBUTES_TEST  : (%s)\n", AK_rel_eq_remove_duplicates(test_attribs));

        printf("IS_SET_SUBSET_OF_LARGER_SET_TEST: (%i)\n\n", AK_rel_eq_is_attr_subset(cond_attr1, cond_attr2));
        printf("GET_ALL_TABLE_ATTRIBUTES_TEST   : (%s)\n\n", (char *) AK_rel_eq_get_atrributes_char(test_table));
        printf("GET_CONDITION_ATTRIBUTES_TEST   : (%s)\n\n", (char *) AK_rel_eq_cond_attributes(test_cond1));
        printf("COMMUTE_WITH_THETA_JOIN_TEST    : (%s)\n\n", (char *) AK_rel_eq_commute_with_theta_join(test_cond1, test_table));
        printf("CONDITION_SHARE_ATTRIBUTES_TEST : (%i)\n", AK_rel_eq_share_attributes(cond_attr1, cond_attr2));
        /**/
    } else {
        printf("\n...\n");
    }

    
    
    //testing a query representation of view in relation operators
    printf("\n\n---------Integration of view query test by Danko Sacer-----------\n\n");
    AK_view_add("view1", "SELECT firstname FROM student", "", 0);

    char* query = AK_get_view_query("view1");
    printf("View query: %s\n", query);


    struct list_node *mylist = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&mylist);

  
    struct list_node *mylist2 = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&mylist2);

    struct list_node *mylist3 = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&mylist3);

    struct list_node *mylist4 = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&mylist4);

    struct list_node *mylist5 = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&mylist5);

    struct list_node *mylist6 = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&mylist6);

    struct list_node *mylist7 = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&mylist7);

    struct list_node *mylist8 = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&mylist8);

    struct list_node *mylist9 = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&mylist9);

    struct list_node *mylist10 = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&mylist10);


    //*Associativity of union and intersection
    AK_InsertAtEnd_L3(TYPE_OPERAND, "professor", sizeof ("professor"), mylist);

    AK_InsertAtEnd_L3(TYPE_OPERATOR, "u", sizeof ("u"), mylist);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "course", sizeof ("course"), mylist);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "i", sizeof ("i"), mylist);

   

    //*Commutativity of Selection and Projection
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "s", sizeof ("s"), mylist2);
    AK_InsertAtEnd_L3(TYPE_CONDITION, "`firstname` = 'Dino'", sizeof ("`firstname` = 'Dino'"), mylist2);
    //...

    //*Cascade of Projection p[L1](p[L2](...p[Ln](R)...)) = p[L1](R)
    //[L1,...] < [L2,...] < [...,Ln-1,Ln]
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "p", sizeof ("p"), mylist3);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname;lastname", sizeof ("firstname;lastname"), mylist3);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "p", sizeof ("p"), mylist3);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname;lastname;year", sizeof ("firstname;lastname;year"),mylist3);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "student", sizeof ("student"), mylist3);
    //---------------------------------------------------------------------------------------------------------
    //*/


   
    //*Commutativity of Selection and set operations (Union, Intersection, and Set difference)
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "s", sizeof ("s"), mylist4);
    AK_InsertAtEnd_L3(TYPE_CONDITION, "`mbr` 35895 < `id_prof` 35897 < OR", sizeof ("`mbr` 35895 < `id_prof` 35897 < OR"), mylist4);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "professor", sizeof ("professor"), mylist4);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "student", sizeof ("student"), mylist4);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "u", sizeof ("u"), mylist4); //u, i, e
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "i", sizeof ("i"), mylist4); //u, i, e
    //---------------------------------------------------------------------------------------------------------
    //*/

    //*Commutativity of Selection and Theta join (or Cartesian product)
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "s", sizeof ("s"), mylist5);
    AK_InsertAtEnd_L3(TYPE_CONDITION, "`name` 'FOBP' = `firstname` 'Alen' = AND `lastname` 'Lovrencic' = OR", sizeof ("`name` 'FOBP' = `firstname` 'Alen' = AND `lastname` 'Lovrencic' = OR"),mylist5);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "professor", sizeof ("professor"), mylist5);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "course", sizeof ("course"),mylist5);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "t", sizeof ("t"), mylist5);
    AK_InsertAtEnd_L3(TYPE_CONDITION, "`name` 'FOBP' = `firstname` 'Alen' = AND `lastname` 'Lovrencic' = OR", sizeof ("`name` 'FOBP' = `firstname` 'Alen' = AND `lastname` 'Lovrencic' = OR"), mylist5);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "e", sizeof ("e"), mylist5); //u, i, e
    //*/

    //*Cascade of Selection and Commutativity of Selection
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "i", sizeof ("u"), mylist6); //u, i, e
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "s", sizeof ("s"), mylist6);
    AK_InsertAtEnd_L3(TYPE_CONDITION, "`year` 2008 <", sizeof ("`year` 2008 <"), mylist6);
    //...

    //*Commutativity of Projection and Theta join (or Cartesian product)
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "p", sizeof ("p"), mylist7);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "mbr;id_prof", sizeof ("mbr;id_prof"), mylist7);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "professor", sizeof ("professor"), mylist7);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "student", sizeof ("student"), mylist7);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "t", sizeof ("t"), mylist7);
    AK_InsertAtEnd_L3(TYPE_CONDITION, "`mbr` 35891 > `id_prof` 35897 < AND", sizeof ("`mbr` 35891 > `id_prof` 35897 < AND"), mylist7);
    //*/

    //*Associativity of natural joins
    AK_InsertAtEnd_L3(TYPE_OPERAND, "professor", sizeof ("professor"), mylist8);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "student", sizeof ("student"),  mylist8);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "n", sizeof ("n"),  mylist8);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "mbr;id_prof", sizeof ("mbr;id_prof"),  mylist8);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "course", sizeof ("course"),  mylist8);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "n", sizeof ("n"),  mylist8);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "id_course", sizeof ("id_course"),  mylist8);
    //*/

    //*Associativity of union and intersection
    AK_InsertAtEnd_L3(TYPE_OPERAND, "professor", sizeof ("professor"),  mylist9);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "student", sizeof ("student"),  mylist9);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "u", sizeof ("u"),  mylist9);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "course", sizeof ("course"),  mylist9);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "u", sizeof ("u"),  mylist9);
    //*/

    //*Associativity of theta-joins
    AK_InsertAtEnd_L3(TYPE_OPERAND, "professor", sizeof ("professor"), mylist10);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "student", sizeof ("student"), mylist10);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "t", sizeof ("t"), mylist10);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "`mbr` 35891 > `id_prof` 35897 <", sizeof ("`mbr` 35891 > `id_prof` 35897 <"), mylist10);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "course", sizeof ("course"), mylist10);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "t", sizeof ("t"), mylist10);
    AK_InsertAtEnd_L3(TYPE_CONDITION, "`id_course` 7 < `mbr` 35891 > AND", sizeof ("`id_course` 7 < `mbr` 35891 > AND"), mylist10);
    //*/

    AK_print_optimized_query(AK_query_optimization(mylist, "a", 1));
    AK_print_optimized_query(AK_query_optimization(mylist2, "c", 1));
    AK_print_optimized_query(AK_query_optimization(mylist3, "p", 1));
    AK_print_optimized_query(AK_query_optimization(mylist4, "cs", 1));
    AK_print_optimized_query(AK_query_optimization(mylist5, "c", 1));
    AK_print_optimized_query(AK_query_optimization(mylist6, "cs", 1));
    AK_print_optimized_query(AK_query_optimization(mylist7, "cp", 2));
    AK_print_optimized_query(AK_query_optimization(mylist8, "a", 1));
    AK_print_optimized_query(AK_query_optimization(mylist9, "a", 1));
    AK_print_optimized_query(AK_query_optimization(mylist10, "a", 1));

	int success=0;
    int failed=0;
	if (error_message==0){
	printf("\n\n\nTest succeeded!\n");
    success++;
    }
    else{
	printf("\n\n\nTest failed!\n");
    failed++;
    }
	
   AK_EPI;
   return TEST_result(success,failed);
}
