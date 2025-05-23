/**
@file select.c Provides functions for SELECT relational operator
 *
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

/**
 * @author Filip Žmuk, Edited by: Marko Belusic
 * @brief Helper function in SELECT clause which filters by condition
 * @param src_table - original table that is used for selection
 * @param selection_table - table in which result of applied condition is stored
 * @param condition - condition for selection
 * @return EXIT_SUCCESS if there was no error applying condition 
 */
int AK_apply_select_by_condition(char *src_table, char *selection_table, struct list_node *condition){
    strcat(selection_table, src_table);
    if(condition != NULL)
    {
        strcat(selection_table, "__selection");
        //select required rows
        return AK_selection(src_table, selection_table, condition);
    }
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
    strcat(sorted_table, selection_table);
    //sort required rows
    if (ordering != NULL)
    {
        strcat(sorted_table, "__sorted");
        return AK_sort_segment(selection_table, sorted_table, ordering);
    }
    return EXIT_SUCCESS;
}


/**
 * @author Matija Karaula
 * @brief Structure to hold column name and its alias
 */

 /*
struct column_alias {
    char original_name[MAX_ATT_NAME];
    char alias_name[MAX_ATT_NAME];
};

struct column_alias_collection {
    struct column_alias *aliases;
    int count;
};
*/
/**
 * @author Filip Žmuk, Edited by: Marko Belusic
 * @brief Function that clears temporary tables
 * @param sorted_table - table in which result of applied ordering is stored
 * @param selection_table - table in which result of applied condition is stored
 * @param src_table - original table that is used for selection
 */
void AK_apply_select_free_temp_tables(char *src_table, char *selection_table, char *sorted_table){
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
}

/**
 * @author Emma Uđbinac
 * @brief Helper function that create copy of attributes
 * @param attributes - atributes to be selected
 * @param projection_attributes - projected atributes
 */
void AK_create_copy_of_attributes(struct list_node *attributes, struct list_node *projection_attributes, struct column_alias_collection *aliases){
    struct list_node *attribute;

    AK_Init_L3(&projection_attributes);

    for (attribute = AK_First_L2(attributes); attribute; attribute = AK_Next_L2(attribute))
    {
        char *original_name = attribute->data;
        char *final_name = original_name;
        
        if(aliases != NULL)
        {
            for (int i = 0; i < aliases->count; i++)
            {
                if (strcmp(original_name, aliases->aliases[i].original_name) == 0)
                {
                    final_name = aliases->aliases[i].alias_name;
                    break;
                }
            }
        }

        AK_InsertAtEnd_L3(TYPE_ATTRIBS, final_name, strlen(final_name), projection_attributes);
    }
}

/**
 * @author Emma Uđbinac
 * @brief Helper function that clears projection of attributes
 * @param projection_attributes - projected atributes for delete
 */
void AK_clear_projection_attributes(struct list_node *projection_attributes){
    AK_DeleteAll_L3(&projection_attributes);
    AK_free(projection_attributes);
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
int AK_apply_select(char *srcTable, char *selection_table, struct list_node *condition, struct list_node *attributes, struct list_node *projection_attributes, char *sorted_table, struct list_node *ordering, struct column_alias_collection *aliases){
    if (AK_apply_select_by_condition(srcTable, selection_table, condition) != EXIT_SUCCESS){
        AK_clear_projection_attributes(projection_attributes);
        AK_apply_select_free_temp_tables(srcTable, selection_table, sorted_table);
        return EXIT_ERROR;
    }

    // create copy of attributes
    AK_create_copy_of_attributes(attributes, projection_attributes, aliases);

    //create help table name for sorting
    if (AK_apply_select_by_sorting(sorted_table, selection_table, ordering) != EXIT_SUCCESS){
        AK_clear_projection_attributes(projection_attributes);
        AK_apply_select_free_temp_tables(srcTable, selection_table, sorted_table);
        return EXIT_ERROR;
    }
    return EXIT_SUCCESS;
}

/**
 * @author Filip Žmuk, Edited by: Marko Belusic, Matija Karaula
 * @brief Function that implements SELECT relational operator
 * @param src_table - original table that is used for selection
 * @param dest_table - table that contains the result
 * @param condition - condition for selection
 * @param attributes - atributes to be selected
 * @param ordering - atributes for result sorting
 * @return EXIT_SUCCESS if cache result in memory and print table else break 
 */
int AK_select(char *src_table, 
              char *dest_table, 
              struct list_node *attributes,
              struct column_alias *aliases,
              int alias_count,
              struct list_node *condition,
              struct list_node *ordering) 
{
    AK_PRO;
    //create help table name for selection
    char selection_table[MAX_ATT_NAME] = "";
    struct list_node *projectionAttributes = (struct list_node *)AK_malloc(sizeof(struct list_node));
    char sorted_table[ MAX_ATT_NAME ] = "";

    struct column_alias_collection alias_collection;
    alias_collection.aliases = aliases;
    alias_collection.count = alias_count;

    if(AK_apply_select(src_table, selection_table, condition, attributes, projectionAttributes, sorted_table, ordering, aliases ? &alias_collection : NULL) != EXIT_SUCCESS){
        AK_apply_select_free_temp_tables(src_table, selection_table, sorted_table);
        AK_clear_projection_attributes(projectionAttributes);
        AK_EPI;
        return EXIT_ERROR;
    }

    //project required rows
    if (AK_projection(sorted_table, dest_table, projectionAttributes, NULL) != EXIT_SUCCESS)
    {
        AK_apply_select_free_temp_tables(src_table, selection_table, sorted_table);
        AK_clear_projection_attributes(projectionAttributes);
        AK_EPI;
        return EXIT_ERROR;
    }

    // free temp tables
    AK_apply_select_free_temp_tables(src_table, &selection_table, &sorted_table);
    AK_clear_projection_attributes(projectionAttributes);
    AK_EPI;
    return EXIT_SUCCESS;
}


/**
 * @author Matija Karaula
 * @brief Function that implements SELECT INTO relational operator
 * @param src_table original table that is used for selection
 * @param new_table name of the new table to create and insert into
 * @param attributes attributes to be selected
 * @param aliases column aliases array 
 * @param alias_count number of aliases
 * @param condition condition for selection
 * @param ordering attributes for result sorting
 * @return EXIT_SUCCESS if successful, EXIT_ERROR otherwise
 */
int AK_select_into(char *src_table, 
                   char *new_table,
                   struct list_node *attributes,
                   struct column_alias *aliases,
                   int alias_count, 
                   struct list_node *condition,
                   struct list_node *ordering) {
    
    AK_PRO;
    // First create a temporary table for the SELECT result
    char temp_table[MAX_ATT_NAME] = "";
    strcat(temp_table, new_table);
    strcat(temp_table, "_temp");

    // Execute the SELECT into the temporary table
    if (AK_select(src_table, temp_table, attributes, aliases, alias_count, condition, ordering) != EXIT_SUCCESS) {
        AK_delete_segment(temp_table, SEGMENT_TYPE_TABLE);
        AK_EPI;
        return EXIT_ERROR;
    }

    // Get the header info from the temporary table
    AK_header *header = AK_get_header(temp_table);
    if (header == NULL) {
        AK_delete_segment(temp_table, SEGMENT_TYPE_TABLE);
        AK_EPI;
        return EXIT_ERROR;
    }

    // Create the new table with the same structure
    int success = AK_initialize_new_segment(new_table, SEGMENT_TYPE_TABLE, header);
    if (success == EXIT_ERROR) {
        AK_delete_segment(temp_table, SEGMENT_TYPE_TABLE);
        AK_EPI;
        return EXIT_ERROR;
    }

    // Copy all rows from temporary table to new table
    if (AK_copy_table_content(temp_table, new_table) != EXIT_SUCCESS) {
        AK_delete_segment(temp_table, SEGMENT_TYPE_TABLE);
        AK_delete_segment(new_table, SEGMENT_TYPE_TABLE);
        AK_EPI;
        return EXIT_ERROR;
    }

    // Clean up temporary table
    AK_delete_segment(temp_table, SEGMENT_TYPE_TABLE);

    AK_EPI;
    return EXIT_SUCCESS;
}


/**
 * @author Renata Mesaros, updated by Filip Žmuk and Josip Susnjara, Matija Karaula
 * @brief Function for testing the implementation
 */
TestResult AK_select_test() {
    int succesful_tests = 0;
    int failed_tests = 0;
    AK_PRO;
    
    // list of attributes which will be in the result of selection
    struct list_node *attributes = (struct list_node *) AK_malloc(sizeof (struct list_node));
    struct list_node *condition = (struct list_node *) AK_malloc(sizeof (struct list_node));
    struct list_node *ordering = (struct list_node *) AK_malloc(sizeof (struct list_node));

    char *src_table="student";
    char *dest_table1="select_result1";
    char *dest_table2="select_result2";
    char *dest_table3="select_result_as";

    // Test 1: SELECT with condition and ordering
    AK_Init_L3(&attributes);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight+year", sizeof("weight+year"), attributes);

    AK_Init_L3(&condition);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), condition);
    int year = 2008;
    AK_InsertAtEnd_L3(TYPE_INT, (char *)&year, sizeof(int), condition);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "<", sizeof("<"), condition);

    AK_Init_L3(&ordering);
    AK_InsertAtBegin_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), ordering);

    if (AK_select(src_table, dest_table1, attributes, NULL, 0, condition, ordering) == EXIT_SUCCESS) {
        succesful_tests++;
    } else {
        failed_tests++;
    }
    
    // Test 2: Simple SELECT without conditions
    AK_DeleteAll_L3(&attributes);
    AK_DeleteAll_L3(&ordering);
    AK_DeleteAll_L3(&condition);
    AK_Init_L3(&attributes);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes);

    if (AK_select(dest_table1, dest_table2, attributes, NULL, 0, NULL, NULL) == EXIT_SUCCESS) {
        succesful_tests++;
    } else {
        failed_tests++;
    }
    
    // Test 3: SELECT AS test
    AK_DeleteAll_L3(&attributes);
    AK_Init_L3(&attributes);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes);

    // Define aliases
    struct column_alias aliases[2] = {
        {"firstname", "name"},
        {"year", "enrollment_year"}
    };

    if (AK_select(src_table, dest_table3, attributes, aliases, 2, NULL, NULL) == EXIT_SUCCESS) {
        succesful_tests++;
    } else {
        failed_tests++;
    }
    
    // Print results
    AK_print_table(src_table);
    printf("\n SELECT firstname, year, weight, weight+year FROM student WHERE year < 2008 ORDER BY firstname;\n\n");
    AK_print_table(dest_table1);
    printf("\n SELECT firstname, year FROM select_result1;\n\n");
    AK_print_table(dest_table2);
    printf("\n SELECT firstname AS name, year AS enrollment_year FROM student;\n\n");
    AK_print_table(dest_table3);
    
    // Cleanup
    AK_DeleteAll_L3(&attributes);
    AK_free(attributes);
    AK_free(condition);
    AK_free(ordering);

    

    // Reset all tables
    AK_delete_segment(dest_table1, SEGMENT_TYPE_TABLE);
    AK_delete_segment(dest_table2, SEGMENT_TYPE_TABLE);
    AK_delete_segment(dest_table3, SEGMENT_TYPE_TABLE);
    
    // Test 4: SELECT INTO test
    AK_DeleteAll_L3(&attributes);
    AK_Init_L3(&attributes);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes);

    char *new_table = "new_student_table";

    if (AK_select_into(src_table, new_table, attributes, NULL, 0, NULL, NULL) == EXIT_SUCCESS) {
        succesful_tests++;
        printf("\n SELECT firstname, year INTO new_student_table FROM student;\n\n");
        AK_print_table(new_table);
    } else {
        failed_tests++;
    }

    // Clean up the new table
    AK_delete_segment(new_table, SEGMENT_TYPE_TABLE);


    AK_EPI;
    return TEST_result(succesful_tests, failed_tests);
}