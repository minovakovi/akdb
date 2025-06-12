/**
@file aggregation.c Provides functions for aggregation and grouping
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

#include "aggregation.h"

extern search_result AK_search_unsorted(char *szRelation, search_params *aspParams, int iNum_search_params);

/**
 @author Dejan Frankovic
 @brief  Function that calculates how many attributes there are in the header with a while loop.
 @param  header A header array
 @return Number of attributes defined in header array

 */
int AK_header_size(AK_header *header) {
    int counter = 0;
    AK_PRO;
    while (*((char*) & header[counter]) != '\0') {
        counter++;
    }
    AK_EPI;
    return counter;
}

/**
  @author Dejan Frankovic
  @brief  Function that initializes the input object for aggregation with init values
  @param  input the input object
  @return No return value
 */
void AK_agg_input_init(AK_agg_input *input) {
    int i;
    AK_PRO;
    for (i = 0; i < MAX_ATTRIBUTES; i++) {
        memcpy(&(*input).attributes[i], "\0", sizeof ("\0"));
        (*input).tasks[i] = -1;
        (*input).counter = 0;
    }
    AK_EPI;
}

/**
  @author Dejan Frankovic
  @brief  Function that adds a header with a task in input object for aggregation
  @param header a header that is being aggregated
  @param agg_task the task which is to be done on the header
  @param input the input object
  @return On success, returns EXIT_SUCCESS, otherwise EXIT_FAILURE
  */

int AK_agg_input_add(AK_header header, int agg_task, AK_agg_input *input) {
    AK_PRO;
    if ((char*) & header == '\0' || agg_task < 0 || (*input).counter == MAX_ATTRIBUTES){
        AK_EPI;
        return EXIT_FAILURE;
    }
    (*input).attributes[(*input).counter] = header;
    (*input).tasks[(*input).counter] = agg_task;
    (*input).counter++;
    AK_EPI;
    return EXIT_SUCCESS;
}
/**
  @author Dejan Frankovic
  @brief Function that adds a header with a task on the beginning of the input object for aggregation.
		 With the use of for loop existing attributes and tasks are moved from one place forward in input object
  @param header a header that is being aggregated
  @param agg_task  the task which is to be done on the header
  @param input the input object
  @return On success, returns EXIT_SUCCESS, otherwise EXIT_FAILURE

 */
int AK_agg_input_add_to_beginning(AK_header header, int agg_task, AK_agg_input *input) {
    AK_PRO;
    if ((char*) & header == '\0' || agg_task < 0 || (*input).counter == MAX_ATTRIBUTES){
        AK_EPI;
        return EXIT_FAILURE;
    }

    int i;
    for (i = (*input).counter; i > 0; i--) {
        (*input).attributes[i] = (*input).attributes[i - 1];
        (*input).tasks[i] = (*input).tasks[i - 1];
    }

    (*input).attributes[0] = header;
    (*input).tasks[0] = agg_task;
    (*input).counter++;

    AK_EPI;
    return EXIT_SUCCESS;
}

/**
  @author Dejan Frankovic
  @brief  function that handles AVG (average) aggregation. It  goes through array of tasks in input
          object until it comes to task with a value of -1. While loop examines whether the task in array is equal to
          AGG_TASK_AVG. If so, AGG_TASK_AVG_COUNT is put on the beginning of input object. After that,
          AGG_TASK_AVG_SUM is put on the beginning of input object.
  @param input the input object
  @return No return value
 */
void AK_agg_input_fix(AK_agg_input *input) {
    int i = 0;
    AK_PRO;
    while ((*input).tasks[i] != -1) {
        if ((*input).tasks[i] == AGG_TASK_AVG) {
            AK_agg_input_add_to_beginning((*input).attributes[i], AGG_TASK_AVG_COUNT, input); // it's important that AVG_COUNT and AVG_SUM are added to the begining so that they are calculated before the AVG
            i++;
            AK_agg_input_add_to_beginning((*input).attributes[i], AGG_TASK_AVG_SUM, input);
            i++;
        }
        i++;
    }
    AK_EPI;
}

/**
   @author Dejan Frankovic
   @brief Function that aggregates a given table by given attributes. Firstly, AGG_TASK_AVG_COUNT and
          AGG_TASK_AVG_SUM are put on the beginning of the input object. Then for loop iterates through
          input tasks and assignes the type of aggregation operation according to aggregation operation.
	  New table has to be created. For loop goes through given table. GROUP operation is executed separately
	  from other operations. Addresses of records are put in needed_values array and
	  results are put in new table.
   @param input input object with list of atributes by which we aggregate and types of aggregations
   @param source_table - table name for the source table
   @param agg_table  table name for aggregated table
   @return EXIT_SUCCESS if continues succesfuly, when not EXIT_ERROR

 */

 // === Pomoćne funkcije ===

void aggregate_value(void* src, void* dst, int type, int agg_task) {
    switch (type) {
        case TYPE_INT: {
            int val_new = *((int*) src);
            int val_old = *((int*) dst);
            int result = val_old;

            if (agg_task == AGG_TASK_SUM || agg_task == AGG_TASK_AVG_SUM)
                result += val_new;
            else if (agg_task == AGG_TASK_MIN && val_new < val_old)
                result = val_new;
            else if (agg_task == AGG_TASK_MAX && val_new > val_old)
                result = val_new;

            memcpy(dst, &result, sizeof(int));
            break;
        }
        case TYPE_FLOAT: {
            float val_new = *((float*) src);
            float val_old = *((float*) dst);
            float result = val_old;

            if (agg_task == AGG_TASK_SUM || agg_task == AGG_TASK_AVG_SUM)
                result += val_new;
            else if (agg_task == AGG_TASK_MIN && val_new < val_old)
                result = val_new;
            else if (agg_task == AGG_TASK_MAX && val_new > val_old)
                result = val_new;

            memcpy(dst, &result, sizeof(float));
            break;
        }
        case TYPE_DOUBLE: {
            double val_new = *((double*) src);
            double val_old = *((double*) dst);
            double result = val_old;

            if (agg_task == AGG_TASK_SUM || agg_task == AGG_TASK_AVG_SUM)
                result += val_new;
            else if (agg_task == AGG_TASK_MIN && val_new < val_old)
                result = val_new;
            else if (agg_task == AGG_TASK_MAX && val_new > val_old)
                result = val_new;

            memcpy(dst, &result, sizeof(double));
            break;
        }
        default:
            fprintf(stderr, "Unsupported type for aggregation.\n");
    }
}

void AK_process_block_with_grouping(AK_agg_input *input, AK_agg_value *needed_values, AK_header *agg_head, char *group_name, char *source_table) {
    table_addresses *addresses = (table_addresses*) AK_get_table_addresses(source_table);
    int num_attr = AK_num_attr(source_table);
    AK_block *temp = NULL;

    rowroot_struct rowroot_table;
    rowroot_table.row_root = (struct list_node*) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&rowroot_table);

    for (int i = 0; addresses->address_from[i] != 0; i++) {
        for (int j = addresses->address_from[i]; j < addresses->address_to[i]; j++) {
            temp = (AK_block*) AK_read_block(j);
            if (temp->last_tuple_dict_id == 0) break;

            for (int k = 0; k < temp->last_tuple_dict_id; k += num_attr) {
                char *group_value = NULL;

                for (int l = 0; l < num_attr; l++) {
                    if (temp->tuple_dict[k + l].is_null) continue;
                    if (strcmp(temp->header[l].att_name, group_name) == 0) {
                        group_value = (char*) &temp->data[temp->tuple_dict[k + l].address];
                        break;
                    }
                }

                struct list_node *existing_row = AK_Find_Row_By_Value(rowroot_table.row_root, group_value);

                if (existing_row == NULL) {
                    for (int l = 0; l < input->counter; l++) {
                        for (int m = 0; m < num_attr; m++) {
                            if (strcmp(needed_values[l].att_name, temp->header[m].att_name) == 0) {
                                if (temp->tuple_dict[k + m].is_null) continue;
                                void *src = &(temp->data[temp->tuple_dict[k + m].address]);
                                AK_Insert_New_Element(agg_head[l].type, src, source_table, agg_head[l].att_name, rowroot_table.row_root);
                            }
                        }
                    }
                    AK_insert_row(rowroot_table.row_root);
                } else {
                    for (int l = 0; l < input->counter; l++) {
                        for (int m = 0; m < num_attr; m++) {
                            if (strcmp(needed_values[l].att_name, temp->header[m].att_name) == 0) {
                                void *src = &(temp->data[temp->tuple_dict[k + m].address]);
                                struct list_node *target = AK_Find_Column_In_Row(existing_row, agg_head[l].att_name);
                                if (target != NULL) {
                                    if (needed_values[l].agg_task == AGG_TASK_COUNT || needed_values[l].agg_task == AGG_TASK_AVG_COUNT) {
                                        int prev = *((int*) target->value);
                                        prev++;
                                        memcpy(target->value, &prev, sizeof(int));
                                    } else {
                                        aggregate_value(src, target->value, needed_values[l].type, needed_values[l].agg_task);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    AK_free(temp);
    AK_free(addresses);
    AK_free(rowroot_table.row_root);
}

void AK_prepare_aggregation_headers(AK_agg_input *input, AK_header *agg_head, AK_header **agg_head_ptr, AK_agg_value *needed_values, int *agg_group_number, char *group_h_name) {
    for (int i = 0; i < input->counter; i++) {
        int attr_type = input->attributes[i].type;
        int agg_task = input->tasks[i];
        int agg_h_type = attr_type;  // defaultno postavi isti tip
        char agg_h_name[MAX_ATT_NAME];

        switch (agg_task) {
            case AGG_TASK_GROUP:
                strcpy(agg_h_name, input->attributes[i].att_name);
                strcpy(group_h_name, input->attributes[i].att_name);
                (*agg_group_number)++;
                break;

            case AGG_TASK_COUNT:
                sprintf(agg_h_name, "Cnt(%s)", input->attributes[i].att_name);
                agg_h_type = TYPE_INT;
                break;

            case AGG_TASK_SUM:
            case AGG_TASK_MAX:
            case AGG_TASK_MIN:
            case AGG_TASK_AVG:
            case AGG_TASK_AVG_SUM:
                //Validacija: podržani tipovi
                if (!(attr_type == TYPE_INT || attr_type == TYPE_FLOAT || attr_type == TYPE_DOUBLE)) {
                    fprintf(stderr, "Error: Unsupported type (%d) for aggregation task %d on attribute '%s'\n", 
                            attr_type, agg_task, input->attributes[i].att_name);
                    exit(EXIT_FAILURE);
                }

                // Dodjela imena i ciljnog tipa
                if (agg_task == AGG_TASK_SUM) {
                    sprintf(agg_h_name, "Sum(%s)", input->attributes[i].att_name);
                } else if (agg_task == AGG_TASK_MAX) {
                    sprintf(agg_h_name, "Max(%s)", input->attributes[i].att_name);
                } else if (agg_task == AGG_TASK_MIN) {
                    sprintf(agg_h_name, "Min(%s)", input->attributes[i].att_name);
                } else if (agg_task == AGG_TASK_AVG) {
                    sprintf(agg_h_name, "Avg(%s)", input->attributes[i].att_name);
                    agg_h_type = TYPE_FLOAT;  // AVG rezultat je uvijek float
                } else if (agg_task == AGG_TASK_AVG_SUM) {
                    sprintf(agg_h_name, "_sAvg(%s)", input->attributes[i].att_name);
                    agg_h_type = TYPE_FLOAT;
                }
                break;

            case AGG_TASK_AVG_COUNT:
                sprintf(agg_h_name, "_cAvg(%s)", input->attributes[i].att_name);
                agg_h_type = TYPE_INT;
                break;

            default:
                fprintf(stderr, "Error: Unknown aggregation task %d\n", agg_task);
                exit(EXIT_FAILURE);
        }

        // Postavljanje vrijednosti
        needed_values[i].agg_task = agg_task;
        needed_values[i].type = attr_type;
        strcpy(needed_values[i].att_name, input->attributes[i].att_name);
        agg_head_ptr[i] = AK_create_header(agg_h_name, agg_h_type, FREE_INT, FREE_CHAR, FREE_CHAR);
        agg_head[i] = *(agg_head_ptr[i]);
    }
}

void AK_clean_unused_headers(AK_header *agg_head, int from) {
    for (int i = from; i < MAX_ATTRIBUTES; i++) {
        memcpy(&agg_head[i], "\0", sizeof("\0"));
    }
}

int AK_initialize_aggregation_table(char *table_name, AK_header *header) {
    int addr = AK_initialize_new_segment(table_name, SEGMENT_TYPE_TABLE, header);
    if (addr != EXIT_ERROR) {
        printf("\nTABLE %s CREATED!\n", table_name);
    }
    return addr;
}

void AK_process_block_without_grouping(AK_agg_input *input, AK_agg_value *needed_values, AK_header *agg_head, char *source_table) {
    table_addresses *addresses = (table_addresses*) AK_get_table_addresses(source_table);
    int num_attr = AK_num_attr(source_table);
    int counter = 0;
    AK_block *temp = NULL;

    for (int i = 0; addresses->address_from[i] != 0; i++) {
        for (int j = addresses->address_from[i]; j < addresses->address_to[i]; j++) {
            temp = (AK_block*) AK_read_block(j);
            if (temp->last_tuple_dict_id == 0) break;

            for (int k = 0; k < temp->last_tuple_dict_id; k += num_attr) {
                counter++;
                for (int l = 0; l < num_attr; l++) {
                    for (int m = 0; m < input->counter; m++) {
                        if (strcmp(needed_values[m].att_name, temp->header[l].att_name) == 0) {
                            void *src = &(temp->data[temp->tuple_dict[k + l].address]);

                            if (needed_values[m].agg_task == AGG_TASK_COUNT || needed_values[m].agg_task == AGG_TASK_AVG_COUNT) {
                                *((int*) needed_values[m].data) = counter;
                                ((char*) needed_values[m].data)[sizeof(int)] = '\0';
                            } else {
                                aggregate_value(src, needed_values[m].data, needed_values[m].type, needed_values[m].agg_task);
                            }
                        }
                    }
                }
            }
        }
    }

    AK_free(temp);
    AK_free(addresses);
}

void AK_finalize_aggregation_result(AK_agg_input *input, AK_header *agg_head, AK_agg_value *needed_values, char *agg_table) {
    AK_header agg_head_final[MAX_ATTRIBUTES];
    rowroot_struct rowroot_table;
    rowroot_table.row_root = (struct list_node*) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&rowroot_table);

    int j = 0;
    for (int i = 0; i < input->counter; i++) {
        if (agg_head[i].att_name[0] != '_') {
            agg_head_final[j++] = agg_head[i];
        }
    }

    int startAddress = AK_initialize_new_segment(agg_table, SEGMENT_TYPE_TABLE, agg_head_final);
    if (startAddress != EXIT_ERROR)
        printf("\nTABLE %s CREATED!\n", agg_table);

    for (int i = 0; i < input->counter; i++) {
        AK_Insert_New_Element(agg_head[i].type, needed_values[i].data,
                              agg_table, agg_head[i].att_name,
                              rowroot_table.row_root);
    }

    AK_insert_row(rowroot_table.row_root);
    AK_free(rowroot_table.row_root);
}

void AK_cleanup_aggregation(AK_agg_value *needed_values, AK_header **agg_head_ptr, int num_aggregations) {
    for (int i = 0; i < num_aggregations; i++) {
        AK_free(agg_head_ptr[i]);
    }
    AK_free(needed_values);
}

// === Testne funkcije ===

void AK_aggregation_test_print_result(char *table) {
    printf("\n=== Rezultat za tablicu: %s ===\n", table);
    AK_print_table(table);
    printf("===============================\n");
}

void AK_aggregation_test_sum_and_count(char *source_table) {
    AK_agg_input input;
    input.counter = 2;
    strcpy(input.attributes[0].att_name, "amount");
    strcpy(input.attributes[1].att_name, "amount");
    input.tasks[0] = AGG_TASK_SUM;
    input.tasks[1] = AGG_TASK_COUNT;

    AK_aggregation(&input, source_table, "agg_sum_count");
    AK_aggregation_test_print_result("agg_sum_count");
}

void AK_aggregation_test_group_by(char *source_table) {
    AK_agg_input input;
    input.counter = 3;
    strcpy(input.attributes[0].att_name, "category");
    strcpy(input.attributes[1].att_name, "amount");
    strcpy(input.attributes[2].att_name, "amount");
    input.tasks[0] = AGG_TASK_GROUP;
    input.tasks[1] = AGG_TASK_SUM;
    input.tasks[2] = AGG_TASK_COUNT;

    AK_aggregation(&input, source_table, "agg_grouped");
    AK_aggregation_test_print_result("agg_grouped");
}

void AK_aggregation_test_avg(char *source_table) {
    AK_agg_input input;
    input.counter = 3;
    strcpy(input.attributes[0].att_name, "amount");
    strcpy(input.attributes[1].att_name, "amount");
    strcpy(input.attributes[2].att_name, "amount");
    input.tasks[0] = AGG_TASK_AVG;
    input.tasks[1] = AGG_TASK_AVG_SUM;
    input.tasks[2] = AGG_TASK_AVG_COUNT;

    AK_aggregation(&input, source_table, "agg_avg");
    AK_aggregation_test_print_result("agg_avg");
}

// === HAVING, projection i cleanup ===

void AK_apply_having_clause(char *agg_table, char *condition) {
    char *tmp_table = "_tmp_having";
    AK_selection(agg_table, tmp_table, condition);
    AK_delete_table(agg_table);
    AK_rename_table(tmp_table, agg_table);
}

void AK_projection_and_cleanup(char *agg_table, char **attributes, int attr_count) {
    char *tmp_table = "_tmp_projected";
    AK_projection(agg_table, tmp_table, attributes, attr_count);
    AK_delete_table(agg_table);
    AK_rename_table(tmp_table, agg_table);
}

// === Glavna funkcija ===

int AK_aggregation(AK_agg_input *input, char *source_table, char *agg_table) {
    AK_PRO;
    AK_agg_input_fix(input);

    int num_aggregations = input->counter;
    AK_header *att_root = input->attributes;
    int *att_tasks = input->tasks;

    AK_header *agg_head_ptr[MAX_ATTRIBUTES];
    AK_header agg_head[MAX_ATTRIBUTES];
    AK_agg_value *needed_values = AK_malloc(sizeof(AK_agg_value) * num_aggregations);

    int agg_group_number = 0;
    char group_h_name[MAX_ATT_NAME];
    char new_table[MAX_ATT_NAME];
    sprintf(new_table, "_%s", agg_table);

    AK_prepare_aggregation_headers(input, agg_head, agg_head_ptr, needed_values, &agg_group_number, group_h_name);
    AK_clean_unused_headers(agg_head, num_aggregations);

    int startAddress = AK_initialize_aggregation_table(new_table, agg_head);

    if (agg_group_number == 0) {
        AK_process_block_without_grouping(input, needed_values, agg_head, source_table);
        AK_finalize_aggregation_result(input, agg_head, needed_values, agg_table);
    } else {
        AK_process_block_with_grouping(input, needed_values, agg_head, group_h_name, source_table);
    }

    AK_cleanup_aggregation(needed_values, agg_head_ptr, num_aggregations);

    AK_EPI;
    return EXIT_SUCCESS;
}


/*
*@author Marin Bogešić
*@brief SQL group by function and test (below)
*/

void groupBy(Table *table, GroupByAttribute *groupByAttributes, int numGroupByAttributes) {
    // Initialize the input object for aggregation
    AK_agg_input input;
    AK_agg_input_init(&input);

    // Add group by attributes to the input object
    for (int i = 0; i < numGroupByAttributes; i++) {
        AK_header header;
        strncpy(header.att_name, groupByAttributes[i].att_name, MAX_ATT_NAME);
        input.attributes[i] = header;
        input.tasks[i] = AGG_TASK_GROUP;
        input.counter++;
    }

    // Aggregate the table based on the group by attributes
    AK_aggregation(&input, "source_table", "agg_table");
}

TestResult test_groupBy() {
    int successful = 0;
    int failed = 0;

    // Create a sample table
    Table table;
    table.count = 3;

    strncpy(table.records[0].att_name, "attribute1", MAX_ATT_NAME);
    strncpy(table.records[0].data, "value1", MAX_VARCHAR_LENGTH);

    strncpy(table.records[1].att_name, "attribute2", MAX_ATT_NAME);
    strncpy(table.records[1].data, "value2", MAX_VARCHAR_LENGTH);

    strncpy(table.records[2].att_name, "attribute1", MAX_ATT_NAME);
    strncpy(table.records[2].data, "value3", MAX_VARCHAR_LENGTH);

    // Create group by attributes
    GroupByAttribute groupByAttributes[1];
    strncpy(groupByAttributes[0].att_name, "attribute1", MAX_ATT_NAME);
    groupByAttributes[0].agg_task = AGG_TASK_GROUP;

    // Call the groupBy function
    groupBy(&table, groupByAttributes, 1);

    // TODO: Add assertions or checks to verify the correctness of the groupBy function

    // Example of a successful test
    successful++;

    // Example of a failed test
    // failed++;

    return TEST_result(successful, failed);
}

//TODO: Needs description
TestResult AK_aggregation_test() {
    AK_PRO;
    printf("aggregation.c: Present!\n");
 
    char *sys_table = "AK_relation";
    char *destTable = "agg";
    char *tblName = "student";
    
    if (AK_if_exist(destTable, sys_table) == 0) {
	    printf("Table %s does not exist!\n", destTable);
	    AK_header *t_header = (AK_header *) AK_get_header(tblName);  // header is array of attributes

	    AK_agg_input aggregation;
	    AK_agg_input_init(&aggregation);
	    AK_agg_input_add(t_header[1], AGG_TASK_GROUP, &aggregation);  // group by second column (first name)
	    AK_agg_input_add(t_header[4], AGG_TASK_AVG, &aggregation);  // average by last (5th) column (weight)
	    AK_agg_input_add(t_header[2], AGG_TASK_COUNT, &aggregation);  // count of last names (for the same first name)
	    AK_agg_input_add(t_header[4], AGG_TASK_SUM, &aggregation);  // sum of weights by student's first name
	    AK_agg_input_add(t_header[4], AGG_TASK_MAX, &aggregation);  // max weight grouped by student's first name
	    AK_agg_input_add(t_header[4], AGG_TASK_MIN, &aggregation);  // min weight grouped by student's first name
	    AK_free(t_header);

	    AK_aggregation(&aggregation, tblName, "agg");
    }
    else {
    	printf("Table %s already exists!\n", destTable);
    }

    AK_print_table("agg");

    printf("\n\n\n");

    /**
     * checking results
     */
    AK_block *block;
    int i, j, k, l, current_row = 0;  // counters

    int first_name_address, first_name_size,  // addresses for data in block
    	last_name_address, last_name_size,
    	weight_address;

    char tmp_first_name[MAX_VARCHAR_LENGTH];  // placeholders for temp data loaded from the block
   	float tmp_weight;

    int student_numrows = AK_get_num_records("student");
    int student_numattr = AK_num_attr("student");
    /* initializing arrays which will hold aggregated data */
    int *counts = AK_malloc(student_numrows * sizeof(int));
    float *sum_weights = AK_malloc(student_numrows * sizeof(float));
    float *min_weights = AK_malloc(student_numrows * sizeof(float));
    float *max_weights = AK_malloc(student_numrows * sizeof(float));
    char **first_names = AK_malloc(student_numrows * sizeof(char *));
    for(i=0; i < student_numrows; i++) {
    	first_names[i] = AK_malloc(MAX_VARCHAR_LENGTH);
    	first_names[i][0] = '\0';
    }

    /* Reading table data and aggregating it within allocated arrays */
    i = 0;
    table_addresses *addresses = (table_addresses*) AK_get_table_addresses("student");
    while (addresses->address_from[ i ] != 0) {
        for (j = addresses->address_from[ i ]; j < addresses->address_to[ i ]; j++) {
            block = (AK_block*) AK_read_block(j);
            if (block->last_tuple_dict_id == 0) { continue; }

            for (k = 0; k < block->last_tuple_dict_id; k += student_numattr) {
	            first_name_address = block->tuple_dict[k + 1].address;
	            first_name_size = block->tuple_dict[k + 1].size;
	            weight_address = block->tuple_dict[k + 4].address;

	            memcpy(tmp_first_name, &block->data[first_name_address], first_name_size);
	            tmp_first_name[first_name_size] = '\0';
	            memcpy(&tmp_weight, &block->data[weight_address], sizeof(float));

	            // checking if that first name has already been added
	            int added_name_index = -1;
	            for (l = 0; l < student_numrows; l++) {
	            	if ( strcmp(first_names[l], tmp_first_name) == 0 ) {
	            		added_name_index = l;
	            		break;
	            	}
	            }

	            if ( added_name_index != -1 ) {  // if that name is already added to first names array
	            	counts[added_name_index]++;
	            	sum_weights[added_name_index] += tmp_weight;
	            	if ( tmp_weight < min_weights[added_name_index] ) { min_weights[added_name_index] = tmp_weight; }
	            	if ( tmp_weight > max_weights[added_name_index] ) { max_weights[added_name_index] = tmp_weight; }
	            } else {
	            	strcpy(first_names[current_row], tmp_first_name);
	            	counts[current_row] = 1;
	            	sum_weights[current_row] = tmp_weight;
	            	min_weights[current_row] = tmp_weight;
	            	max_weights[current_row] = tmp_weight;
	            	current_row++;
	            }
        	}
		}
		i++;
	}

	/* Reading data from aggregated table and comparing results to the data in arrays! */
	i = 0;
	int agg_numcol = AK_num_attr("agg"),
		current_aggregated_row = 0;

	int avg_weight_address, cnt_last_names_address, // addresses for data in block
		sum_weights_address, max_weight_address,
		min_weight_address;

	int num_errors = 0;  // this will count number of errors

	float tmp_avg_weight, tmp_sum_weights,  // placeholders for temp data loaded from the block
		  tmp_max_weight, tmp_min_weight;
	int   tmp_cnt_last_names;

	/* Reading aggregated table data and comparing it to the data from aggregated arrays */
    addresses = (table_addresses*) AK_get_table_addresses("agg");
    while (addresses->address_from[ i ] != 0) {
        for (j = addresses->address_from[ i ]; j < addresses->address_to[ i ]; j++) {
            block = (AK_block*) AK_read_block(j);
            if (block->last_tuple_dict_id == 0) { continue; }

            for (k = 0; k < block->last_tuple_dict_id; k += agg_numcol) {
            	/** 
            	 * This variable was added to handle bug described in this file.
            	 */
            	int is_row_empty = 0;
            	for (l=0; l<6; l++) {
            		if ( block->tuple_dict[k + l].size == 0 ) {
            			is_row_empty = 1;
            			break;
            		}
				}

				if ( is_row_empty == 1 ) { break; }

            	first_name_address = block->tuple_dict[k].address;
            	first_name_size = block->tuple_dict[k].size;
            	avg_weight_address = block->tuple_dict[k + 1].address;
            	cnt_last_names_address = block->tuple_dict[k + 2].address;
            	sum_weights_address = block->tuple_dict[k + 3].address;
            	max_weight_address = block->tuple_dict[k + 4].address;
            	min_weight_address = block->tuple_dict[k + 5].address;

            	memcpy(tmp_first_name, &block->data[first_name_address], first_name_size);
	            tmp_first_name[first_name_size] = '\0';
	            memcpy(&tmp_avg_weight, &block->data[avg_weight_address], sizeof(float));
				tmp_cnt_last_names = 1;
	            memcpy(&tmp_sum_weights, &block->data[sum_weights_address], sizeof(float));
	            memcpy(&tmp_max_weight, &block->data[max_weight_address], sizeof(float));
	            memcpy(&tmp_min_weight, &block->data[min_weight_address], sizeof(float));

	            if ( strcmp( tmp_first_name, first_names[current_aggregated_row] ) != 0 ) {
	            	num_errors++;
	            	printf("Error in aggregated table, row: %d! Wrong first name (1st column)!\n", current_aggregated_row+1);
	            	printf("Table showed value: '%s', but it should show: '%s'\n\n", 
	            		tmp_first_name, first_names[current_aggregated_row]);
	            }
	            if ( tmp_avg_weight != sum_weights[current_aggregated_row] / counts[current_aggregated_row] ) {
	            	num_errors++;
	            	printf("Error in aggregated table, row: %d! Wrongly calculated avg(weight)!\n", current_aggregated_row+1);
	            	printf("Table showed value: '%f', but it should show: '%f'\n\n", 
	            		tmp_avg_weight, sum_weights[current_aggregated_row] / counts[current_aggregated_row]);
	            }
	            if ( tmp_cnt_last_names != counts[current_aggregated_row] ) {
	            	num_errors++;
	            	printf("Error in aggregated table, row: %d! Wrongly calculated cnt(lastname)!\n", current_aggregated_row+1);
	            	printf("Table showed value: '%d', but it should show: '%d'\n\n", 
	            		tmp_cnt_last_names, counts[current_aggregated_row]);
	            }
	            if ( tmp_sum_weights != sum_weights[current_aggregated_row] ) {
	            	num_errors++;
	            	printf("Error in aggregated table, row: %d! Wrongly calculated sum(weight)!\n", current_aggregated_row+1);
	            	printf("Table showed value: '%f', but it should show: '%f'\n\n", 
	            		tmp_sum_weights, sum_weights[current_aggregated_row]);
	            }
	            if ( tmp_max_weight != max_weights[current_aggregated_row] ) {
	            	num_errors++;
	            	printf("Error in aggregated table, row: %d! Wrongly calculated max(weight)!\n", current_aggregated_row+1);
	            	printf("Table showed value: '%f', but it should show: '%f'\n\n", 
	            		tmp_max_weight, max_weights[current_aggregated_row]);
	            }
	            if ( tmp_min_weight != min_weights[current_aggregated_row] ) {
	            	num_errors++;
	            	printf("Error in aggregated table, row: %d! Wrongly calculated min(weight)!\n", current_aggregated_row+1);
	            	printf("Table showed value: '%f', but it should show: '%f'\n\n", 
	            		tmp_min_weight, min_weights[current_aggregated_row]);
	            }

	            current_aggregated_row++;
            }
        }
        i++;
    }
    
    AK_free(counts);
    AK_free(sum_weights);
    AK_free(min_weights);
    AK_free(max_weights);
    for(i=0; i < student_numrows; i++) {
        AK_free( first_names[i] );
    }
    AK_free(first_names);

    if ( num_errors == 0 ) {
    	printf("\nTEST PASSED!\n");
    } else {
    	printf("\nTEST FAILED! Number of errors: %d\n", num_errors);
    }

    AK_EPI;
}

int AK_aggregation_having(AK_agg_input *input, char *source_table, char *agg_having_table, struct list_node *having_expr) {

	char *sys_table = "AK_relation";
    char *temp_agg_table = "temp_agg_table";
	struct list_node *where_expr;
    
    if (AK_if_exist(temp_agg_table, sys_table)) {
		printf("Table %s already exists!\n", temp_agg_table);
	} else {
	    printf("Table %s does not exist!\n", temp_agg_table);
	
		// Deconstruct HAVING as AGGREGATION + SELECTION
	
	    AK_header *t_header = (AK_header *) AK_get_header(source_table);

		struct list_node *where_expr = (struct list_node *) AK_malloc(sizeof (struct list_node));
		AK_Init_L3(&where_expr);

    	struct list_node *el = AK_First_L2(having_expr);
	    while (el) {
			if (el->type != TYPE_AGGREGATOR) {
				AK_InsertAtEnd_L3(el->type, el->data, el->size, where_expr);
				if (el->type == TYPE_ATTRIBS) {
					struct list_node *aggregator_node = el->next;
					// Add aggregators form HAVING to input
					AK_agg_input_add(t_header[AK_get_attr_index(source_table, el->data)], aggregator_node->data, input);
				}
			}
			el = el->next;
		}
	    AK_free(t_header);

		// Preform aggregation
	    int aggregation_result = AK_aggregation(input, source_table, temp_agg_table);
		if (aggregation_result != EXIT_SUCCESS) {
			return aggregation_result;
		}
    }

    AK_print_table(temp_agg_table);

	// Filter rows based on HAVING requirements (now moved to WHERE)
	int selection_result = AK_selection(temp_agg_table, agg_having_table, where_expr);
	if (selection_result != EXIT_SUCCESS) {
		return selection_result;
	}

	// Delete aggregator columns used only for HAVING, if ever implemented

	return EXIT_SUCCESS;

}
TestResult AK_aggregation_having_test() {
	AK_PRO;

    printf("\n********** AGGREGATION HAVING TEST **********\n");
 
    char *sys_table = "AK_relation";
    char *dest_table = "agg_having";
    char *src_table = "student";
	static int test_run_count = 0;
    
    if (AK_if_exist(dest_table, sys_table)) {
		printf("Table %s already exists!\n", dest_table);
	} else {
	    printf("Table %s does not exist!\n", dest_table);
	    AK_header *t_header = (AK_header *) AK_get_header(src_table);  // header is array of attributes

	    AK_agg_input aggregation;
	    AK_agg_input_init(&aggregation);
	    AK_agg_input_add(t_header[1], AGG_TASK_GROUP, &aggregation);  // group by second column (first name)
	    AK_agg_input_add(t_header[4], AGG_TASK_AVG, &aggregation);  // average by last (5th) column (weight)
	    AK_agg_input_add(t_header[2], AGG_TASK_COUNT, &aggregation);  // count of last names (for the same first name)
	    AK_agg_input_add(t_header[4], AGG_TASK_SUM, &aggregation);  // sum of weights by student's first name
	    AK_agg_input_add(t_header[4], AGG_TASK_MAX, &aggregation);  // max weight grouped by student's first name
	    AK_agg_input_add(t_header[4], AGG_TASK_MIN, &aggregation);  // min weight grouped by student's first name
	    AK_free(t_header);

		struct list_node *having_expr = (struct list_node *) AK_malloc(sizeof (struct list_node));
		AK_Init_L3(&having_expr);

		char destTable[256]; 
		sprintf(destTable, "selection_test1_%d", test_run_count);

		int avg_weight_threshold_gt = 100;
		int count_threshold_gt = 1;
		strcpy(having_expr->table, dest_table);
		AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof ("weight"), having_expr);
		AK_InsertAtEnd_L3(TYPE_AGGREGATOR, AGG_TASK_AVG, sizeof (int), having_expr);
		AK_InsertAtEnd_L3(TYPE_INT, &avg_weight_threshold_gt, sizeof (int), having_expr);
		AK_InsertAtEnd_L3(TYPE_OPERATOR, ">", sizeof (">"), having_expr);
		AK_InsertAtEnd_L3(TYPE_ATTRIBS, "id_student", sizeof ("id_student"), having_expr);
		AK_InsertAtEnd_L3(TYPE_AGGREGATOR, AGG_TASK_COUNT, sizeof (int), having_expr);
		AK_InsertAtEnd_L3(TYPE_INT, &count_threshold_gt, sizeof (int), having_expr);
		AK_InsertAtEnd_L3(TYPE_OPERATOR, ">", sizeof (">"), having_expr);
		AK_InsertAtEnd_L3(TYPE_OPERATOR, "AND", sizeof("AND"), having_expr);
		printf("\nQUERY: SELECT * FROM student GROUP BY year HAVING AVG(weight) > 100 AND COUNT(id_student) > 1;\n\n");

	    AK_aggregation_having(&aggregation, src_table, "agg_haivng", having_expr);
    }

    AK_print_table("agg_having");

    printf("\n\n\n");

	return TEST_result(0 , 0);
}
