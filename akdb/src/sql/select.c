/**
@file select.c Provides functions for SELECT relational operator
 *
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
#include "select.h"
#include "../mm/memoman.h"
#include "../rec/redo_log.h"
#include "../file/table.h"
#include "../file/files.h"
#include "../auxi/constants.h"
#include "../auxi/auxiliary.h"
#include <time.h>
#include <stdio.h>
#include <string.h>

#ifndef OPERATION_SELECT
#define OPERATION_SELECT 3
#endif

/**
 * @author Filip Žmuk, Edited by: Marko Belusic
 * @brief Helper function in SELECT clause which filters by condition
 * @param src_table - original table that is used for selection
 * @param selection_table - table in which result of applied condition is stored
 * @param condition - condition for selection
 * @return EXIT_SUCCESS if there was no error applying condition 
 */
int AK_apply_select_by_condition(char *src_table, char *selection_table, struct list_node *condition){
    AK_PRO;
    strcpy(selection_table, src_table);
    if(condition != NULL)
    {
        strcat(selection_table, "__selection");
        return AK_selection(src_table, selection_table, condition);
    }
    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Filip Žmuk, Edited by: Marko Belusic
 * @brief Helper function in SELECT clause which does the ordering
 * @param ordering - condition on which to order
 * @param sorted_table - table in which result of applied ordering is stored
 * @param selection_table - table in which result of applied condition is stored
 * @return EXIT_SUCCESS if there was no error ordering
 */
int AK_apply_select_by_sorting(char *sorted_table, char *selection_table, struct list_node *ordering){
    AK_PRO;
    strcpy(sorted_table, selection_table);
    if (ordering != NULL)
    {
        strcat(sorted_table, "__sorted");
        return AK_sort_segment(selection_table, sorted_table, ordering);
    }
    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Filip Žmuk, Edited by: Marko Belusic
 * @brief Function that clears temporary tables
 * @param sorted_table - table in which result of applied ordering is stored
 * @param selection_table - table in which result of applied condition is stored
 * @param src_table - original table that is used for selection
 */
void AK_apply_select_free_temp_tables(char *src_table, char *selection_table, char *sorted_table){
    AK_PRO;
    if(strcmp(src_table, selection_table) != 0)
    {
        AK_delete_segment(selection_table, SEGMENT_TYPE_TABLE);
        if(strcmp(selection_table, sorted_table) != 0)
        {
            AK_delete_segment(sorted_table, SEGMENT_TYPE_TABLE);
        }
    }
    else
    {
        if(strcmp(src_table, sorted_table) != 0)
        {
            AK_delete_segment(sorted_table, SEGMENT_TYPE_TABLE);
        }
    }
    AK_EPI;
}

/**
 * @author Emma Uđbinac
 * @brief Helper function that create copy of attributes
 * @param attributes - atributes to be selected
 * @param projection_attributes - projected atributes
 */
void AK_create_copy_of_attributes(struct list_node *attributes, struct list_node *projection_attributes){
    AK_PRO;
    struct list_node *attribute;
    if(!projection_attributes) return;
    


    for (attribute = AK_First_L2(attributes); attribute; attribute = AK_Next_L2(attribute))
    {
        AK_InsertAtEnd_L3(TYPE_ATTRIBS, attribute->data, strlen(attribute->data), projection_attributes);
    }
    AK_EPI;
}

/**
 * @author Emma Uđbinac
 * @brief Helper function that clears projection of attributes
 * @param projection_attributes - projected atributes for delete
 */
void AK_clear_projection_attributes(struct list_node *projection_attributes){
    AK_PRO;
    if(projection_attributes) {
        AK_DeleteAll_L3(&projection_attributes);
        AK_free(projection_attributes);
    }
    AK_EPI;
}

/**
 * @author Emma Uđbinac
 * @brief Helper function that apply select by condition or by sorting
 * @param srcTable - original table that is used for selection
 * @param selection_table - temp table tfor selection
 * @param condition - condition for selection
 * @param attributes - atributes to be selected
 * @param projection_attributes - projected attributes
 * @param sorted_table - temp table for sorting
 * @param ordering - atributes for result sorting
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_apply_select(char *srcTable, char *selection_table, struct list_node *condition, struct list_node *attributes, struct list_node *projection_attributes, char *sorted_table, struct list_node *ordering){
    AK_PRO;
    if (AK_apply_select_by_condition(srcTable, selection_table, condition) != EXIT_SUCCESS){
        return EXIT_ERROR;
    }

    if (attributes) {
        AK_create_copy_of_attributes(attributes, projection_attributes);
    }

    if (AK_apply_select_by_sorting(sorted_table, selection_table, ordering) != EXIT_SUCCESS){
        return EXIT_ERROR;
    }
    AK_EPI;
    return EXIT_SUCCESS;
}


/**
 * @author AI Assistant for redo-log #143
 * @brief Helper function to materialize a cached result into a specified destination table.
 * @param res The cached result to materialize.
 * @param destTableName The name of the destination table to create/populate.
 * @return EXIT_SUCCESS on success, EXIT_ERROR on failure.
 */
int AK_materialize_cached_results_to_table(AK_results* res, const char* destTableName) {
    AK_PRO;
    if (!res || !destTableName || res->header[0].type == TYPE_INTERNAL || !res->result_rows) {
        return EXIT_ERROR;
    }
    if (AK_table_exist((char*)destTableName)) {
        AK_delete_segment((char*)destTableName, SEGMENT_TYPE_TABLE);
    }
    AK_temp_create_table((char*)destTableName, res->header, SEGMENT_TYPE_TABLE);

    if (res->result_rows && res->num_rows > 0 && res->num_cols > 0) {
        struct list_node* current_cell = AK_First_L2(res->result_rows);
        for(int i = 0; i < res->num_rows; ++i) {
           struct list_node* single_row_for_insert = NULL;
           AK_Init_L3(&single_row_for_insert);

           for (int j = 0; j < res->num_cols; ++j) {
               if (!current_cell) {
                   AK_DeleteAll_L3(&single_row_for_insert); AK_free(single_row_for_insert);
                   AK_EPI; return EXIT_ERROR;
               }
               AK_Insert_New_Element(current_cell->type, current_cell->data, (char*)destTableName, res->header[j].att_name, single_row_for_insert);
               current_cell = AK_Next_L2(current_cell);
           }
           
           if (AK_First_L2(single_row_for_insert)) {
               AK_insert_row(single_row_for_insert);
           }
           AK_DeleteAll_L3(&single_row_for_insert); 
           AK_free(single_row_for_insert);
        }
    }
    AK_EPI;
    return EXIT_SUCCESS;
}


/**
 * @author Filip Žmuk, Edited by: Marko Belusic, Emma Uđbinac
 * @brief Function that implements SELECT relational operator.
 * @param src_table - original table that is used for selection
 * @param dest_table - table that contains the result
 * @param attributes - atributes to be selected
 * @param condition - condition for selection
 * @param ordering - atributes for result sorting
 * @return EXIT_SUCCESS if select is successful else break 
 */
int AK_select(char *src_table, char *dest_table, struct list_node *attributes, struct list_node *condition, struct list_node *ordering)
{
    AK_PRO;
    
    char query_identifier[MAX_VARCHAR_LENGTH * 4]; 
    AK_generate_select_query_identifier(src_table, attributes, condition, query_identifier, sizeof(query_identifier));

    AK_results *cached_result = NULL;
    time_t log_timestamp = 0;

    if (AK_check_redo_log_select_for_caching(src_table, attributes, condition, query_identifier, &cached_result, &log_timestamp)) {
        time_t table_mod_timestamp = AK_get_table_timestamp(src_table);
        if (log_timestamp > 0 && (table_mod_timestamp == 0 || log_timestamp >= table_mod_timestamp) && cached_result) {
            if (AK_materialize_cached_results_to_table(cached_result, dest_table) == EXIT_SUCCESS) {
                AK_EPI;
                return EXIT_SUCCESS;
            }
        }
    }

    char selection_table[MAX_ATT_NAME] = "";
    struct list_node *projectionAttributes = (struct list_node *)AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&projectionAttributes); 
    char sorted_table[ MAX_ATT_NAME ] = "";

    if (AK_apply_select(src_table, selection_table, condition, attributes, projectionAttributes, sorted_table, ordering) != EXIT_SUCCESS){
        AK_clear_projection_attributes(projectionAttributes);
        AK_EPI;
        return EXIT_ERROR;
    }
    
    if (AK_projection(sorted_table, dest_table, projectionAttributes, NULL) != EXIT_SUCCESS) {
        AK_clear_projection_attributes(projectionAttributes);
        AK_apply_select_free_temp_tables(src_table, selection_table, sorted_table); 
        AK_EPI;
        return EXIT_ERROR;
    }
    
    AK_results* executed_result_ptr = AK_create_results_from_table(dest_table);
    if (executed_result_ptr) {
        AK_add_to_redolog_select_with_result(OPERATION_SELECT, src_table, attributes, condition, executed_result_ptr, query_identifier);
        AK_free_results_struct(executed_result_ptr); 
    }
    
    AK_apply_select_free_temp_tables(src_table, selection_table, sorted_table);
    AK_clear_projection_attributes(projectionAttributes);

    AK_EPI;
    return EXIT_SUCCESS;
}


/**
 * @author Renata Mesaros, updated by Filip Žmuk and Josip Susnjara
 * @brief Function for testing the implementation
 */
TestResult AK_select_test(){
    int succesful_tests = 0;
    int failed_tests = 0;
    AK_PRO;
	
	struct list_node *attributes = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&attributes);
	struct list_node *condition = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&condition);
	struct list_node *ordering = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&ordering);

	char *src_table="student";
	char *dest_table1="select_result1";
    char *dest_table2="select_result2";

	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight+year", sizeof("weight+year"), attributes);

	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), condition);
    int year = 2008;
	AK_InsertAtEnd_L3(TYPE_INT, (char *)&year, sizeof(int), condition);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, "<", sizeof("<"), condition);
    
	AK_InsertAtBegin_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), ordering);

    if (AK_select(src_table, dest_table1, attributes, condition, ordering) == EXIT_SUCCESS) {
        succesful_tests++;
    } else {
        failed_tests++;
    }
    
	AK_DeleteAll_L3(&attributes);
    AK_DeleteAll_L3(&ordering);
	AK_DeleteAll_L3(&condition);

    AK_Init_L3(&attributes);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes);

    if (AK_select(dest_table1, dest_table2, attributes, NULL, NULL) == EXIT_SUCCESS) {
        succesful_tests++;
    } else {
        failed_tests++;
    }
    
    AK_DeleteAll_L3(&attributes);
	
    AK_print_table(src_table);
	printf("\n SELECT firstname, year, weight, weight+year FROM student WHERE year < 2008 ORDER BY firstname;\n\n");
    AK_print_table(dest_table1);
    printf("\n SELECT firstname, year FROM select_result1;\n\n");
    AK_print_table(dest_table2);
	
	AK_free(attributes);
	AK_free(condition);
	AK_free(ordering);

    AK_delete_segment(dest_table1, SEGMENT_TYPE_TABLE);
    AK_delete_segment(dest_table2, SEGMENT_TYPE_TABLE);
	AK_EPI;

	return TEST_result(succesful_tests, failed_tests);
}