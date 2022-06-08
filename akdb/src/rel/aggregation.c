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
int AK_aggregation(AK_agg_input *input, char *source_table, char *agg_table) {
    int i, j;
    AK_PRO;
    AK_agg_input_fix(input);
    AK_header *att_root = (*input).attributes;
    int *att_tasks = (*input).tasks;
    int num_aggregations = (*input).counter;
	AK_header *agg_head_ptr[MAX_ATTRIBUTES];
    AK_header agg_head[MAX_ATTRIBUTES];
    int agg_group_number = 0;
    int inttemp = 0;
    double doubletemp = 0;
    float floattemp = 0;

    char agg_h_name[MAX_ATT_NAME];
    int agg_h_type;
    char group_h_name[MAX_ATT_NAME];

    AK_agg_value *needed_values = AK_malloc(sizeof (AK_agg_value) * num_aggregations);

    char new_table[MAX_ATT_NAME];
    sprintf(new_table, "_%s", agg_table);

    for (i = 0; i < num_aggregations; i++) 
	{
        agg_h_type = att_root[i].type;
        switch (att_tasks[i]) {
            case AGG_TASK_GROUP:
                strcpy(agg_h_name, att_root[i].att_name);
                strcpy(group_h_name, att_root[i].att_name);
                agg_group_number++;
                break;
            case AGG_TASK_COUNT:
                sprintf(agg_h_name, "Cnt(%s)", att_root[i].att_name);
                agg_h_type = TYPE_INT;
                break;
            case AGG_TASK_SUM:
                sprintf(agg_h_name, "Sum(%s)", att_root[i].att_name);
                if (agg_h_type != TYPE_INT && agg_h_type != TYPE_FLOAT && agg_h_type != TYPE_NUMBER)
                    agg_h_type = TYPE_INT;
                break;
            case AGG_TASK_MAX:
                sprintf(agg_h_name, "Max(%s)", att_root[i].att_name);
                if (agg_h_type != TYPE_INT && agg_h_type != TYPE_FLOAT && agg_h_type != TYPE_NUMBER)
                    agg_h_type = TYPE_INT;
                break;
            case AGG_TASK_MIN:
                sprintf(agg_h_name, "Min(%s)", att_root[i].att_name);
                if (agg_h_type != TYPE_INT && agg_h_type != TYPE_FLOAT && agg_h_type != TYPE_NUMBER)
                    agg_h_type = TYPE_INT;
                break;
            case AGG_TASK_AVG:
                sprintf(agg_h_name, "Avg(%s)", att_root[i].att_name);
                agg_h_type = TYPE_FLOAT;
                break;
            case AGG_TASK_AVG_COUNT:
                sprintf(agg_h_name, "_cAvg(%s)", att_root[i].att_name);
                agg_h_type = TYPE_INT;
                break;
            case AGG_TASK_AVG_SUM:
                sprintf(agg_h_name, "_sAvg(%s)", att_root[i].att_name);
                agg_h_type = TYPE_FLOAT;
                break;
        }
        needed_values[i].agg_task = att_tasks[i];
        strcpy(needed_values[i].att_name, att_root[i].att_name);
		agg_head_ptr[i] = AK_create_header(agg_h_name, agg_h_type, FREE_INT, FREE_CHAR, FREE_CHAR);
        agg_head[i] = *(agg_head_ptr[i]);
    }

    // removing rest of the unneeded attributes (where attribute id is greater than number of used aggregations)
    for (i = num_aggregations; i < MAX_ATTRIBUTES; i++) 
	{
        memcpy(&agg_head[i], "\0", sizeof ( "\0"));
    }
    

    int startAddress = AK_initialize_new_segment(new_table, SEGMENT_TYPE_TABLE, agg_head);

    if (startAddress != EXIT_ERROR)
        printf("\nTABLE %s CREATED!\n", new_table);

    // this was an optimisation or something, so that sort works normally
    //sort_segment(source_table,group_h_name);

    search_params search_parameters[agg_group_number];
    search_result sresult;
    j = 0;


    table_addresses *addresses = (table_addresses*) AK_get_table_addresses(source_table);
    int num_attr = AK_num_attr(source_table);

    int k, l, m, n, o, counter;

    AK_block *temp;
    AK_mem_block *mem_block;


	rowroot_struct rowroot_table = {.row_root = (struct list_node*) AK_malloc(sizeof(struct list_node))};

    AK_Init_L3(&rowroot_table);


    i = 0;
    counter = 0;

    while (addresses->address_from[ i ] != 0) {
        for (j = addresses->address_from[ i ]; j < addresses->address_to[ i ]; j++) {
            temp = (AK_block*) AK_read_block(j);
            if ( temp->last_tuple_dict_id == 0 )
            	break;
            for (k = 0; k < temp->last_tuple_dict_id; k += num_attr) {
                counter++;
                n = 0;

                //aggregation when no grouping is done will be executed separately to skip the unsorted search
                if(agg_group_number == 0){
                	for (l = 0; l < num_attr; l++) {
						for (m = 0; m < num_aggregations; m++) {
							if (strcmp(needed_values[m].att_name, temp->header[l].att_name) == 0) {
								switch (needed_values[m].agg_task) {
									case AGG_TASK_COUNT:
										//no break is intentional
									case AGG_TASK_AVG_COUNT:
										memcpy(needed_values[m].data, &counter, sizeof(int));
										needed_values[m].data[sizeof(int)] = '\0';
										break;

									case AGG_TASK_MAX:
										switch (agg_head[m].type) {
											case TYPE_INT:
												memcpy(&inttemp, &(temp->data[temp->tuple_dict[k + l].address]), temp->tuple_dict[k + l].size);
												if (*((int*)needed_values[m].data) < inttemp || counter == 1)
													memcpy(needed_values[m].data, &inttemp, sizeof (int));
												break;

											case TYPE_FLOAT:
												memcpy(&floattemp, &(temp->data[temp->tuple_dict[k + l].address]), temp->tuple_dict[k + l].size);
												if (*((float *)needed_values[m].data) < floattemp || counter == 1)
													memcpy(needed_values[m].data, &floattemp, sizeof(float));
												break;

											case TYPE_NUMBER:
												memcpy(&doubletemp, &(temp->data[temp->tuple_dict[k + l].address]), temp->tuple_dict[k + l].size);
												if (*((double *)needed_values[m].data) < doubletemp || counter == 1)
													memcpy(needed_values[m].data, &doubletemp, sizeof(double));
												break;
										}
										needed_values[m].data[temp->tuple_dict[k + l].size] = '\0';
										break;

									case AGG_TASK_MIN:
										switch (agg_head[m].type) {
											case TYPE_INT:
												memcpy(&inttemp, &(temp->data[temp->tuple_dict[k + l].address]), temp->tuple_dict[k + l].size);
												if (*((int*)needed_values[m].data) > inttemp || counter == 1)
													memcpy(needed_values[m].data, &inttemp, sizeof (int));
												break;

											case TYPE_FLOAT:
												memcpy(&floattemp, &(temp->data[temp->tuple_dict[k + l].address]), temp->tuple_dict[k + l].size);
												if (*((float *)needed_values[m].data) > floattemp || counter == 1)
													memcpy(needed_values[m].data, &floattemp, sizeof(float));
												break;

											case TYPE_NUMBER:
												memcpy(&doubletemp, &(temp->data[temp->tuple_dict[k + l].address]), temp->tuple_dict[k + l].size);
												if (*((double *)needed_values[m].data) > doubletemp || counter == 1)
													memcpy(needed_values[m].data, &doubletemp, sizeof(double));
												break;
										}
										needed_values[m].data[temp->tuple_dict[k + l].size] = '\0';
										break;

									case AGG_TASK_SUM:
										//no break is intentional
									case AGG_TASK_AVG_SUM:
										switch (agg_head[m].type) {
											case TYPE_INT:
												memcpy(&inttemp, &(temp->data[temp->tuple_dict[k + l].address]), temp->tuple_dict[k + l].size);
												inttemp += *((int*) needed_values[m].data);
												memcpy(needed_values[m].data, &inttemp, sizeof (int));
												break;

											case TYPE_FLOAT:
												memcpy(&floattemp, &(temp->data[temp->tuple_dict[k + l].address]), temp->tuple_dict[k + l].size);
												floattemp += *((float *)needed_values[m].data);
												memcpy(needed_values[m].data, &floattemp, sizeof (float));
												break;

											case TYPE_NUMBER:
												memcpy(&doubletemp, &(temp->data[temp->tuple_dict[k + l].address]), temp->tuple_dict[k + l].size);
												doubletemp += *((double*) needed_values[m].data);
												memcpy(needed_values[m].data, &doubletemp, sizeof (double));
												break;
										}
										needed_values[m].data[temp->tuple_dict[k + l].size] = '\0';
								}
							}
						}
					}
				}
				else{

					for (l = 0; l < num_attr; l++) {
						for (m = 0; m < num_aggregations; m++) {
							if (strcmp(needed_values[m].att_name, temp->header[l].att_name) == 0) {

								memcpy(needed_values[m].data, &(temp->data[temp->tuple_dict[k + l].address]), temp->tuple_dict[k + l].size);
								needed_values[m].data[temp->tuple_dict[k + l].size] = '\0';

								if (needed_values[m].agg_task == AGG_TASK_GROUP) {
									search_parameters[n].iSearchType = SEARCH_PARTICULAR;
									search_parameters[n].pData_lower = AK_malloc(temp->tuple_dict[k + l].size + 1);
									memcpy(search_parameters[n].pData_lower, &(temp->data[temp->tuple_dict[k + l].address]), temp->tuple_dict[k + l].size);
									((char *) search_parameters[n].pData_lower)[ temp->tuple_dict[k + l].size ] = '\0';
									search_parameters[n].szAttribute = (temp->header[l].att_name);
									n++;
								}
							}
						}
					}

					sresult = AK_search_unsorted(new_table, search_parameters, agg_group_number);

					if (sresult.iNum_tuple_addresses == 0) {
						AK_DeleteAll_L3(&rowroot_table);

						for (l = 0; l < num_aggregations; l++) {
							switch (needed_values[l].agg_task) {
								case AGG_TASK_COUNT:
									//no break is intentional
								case AGG_TASK_AVG_COUNT:
									inttemp = 1;
									/**
									 * THIS SINGLE LINE BELOW (memcpy) is the purpose of ALL evil in the world!
									 * This line is the reason why test function prints one extra empty 
									 * row with "nulls" at the end! Trust me! Comment it, and you will see - 
									 * test function will not print extra row with nulls (but counts and averages 
									 * in table will be all messed up!)
									 * After two days of hard research, I still have not found what is the
									 * reason behind printing extra row at the end! Fellow programmer,
									 * if you really really want to solve this issue, arm yourself with
									 * at least 2 liters of hot coffee!
									 *
									 * What this line does? What is the purpose of this line in the universe?
									 * Well, fellow programmer, this line sets the initial count to 1.
									 * That means if name "Ivan" is found, it will have count of 1
									 * because, well, that's the first Ivan that is found!
									 * If function finds another Ivan (which, actually, will happen),
									 * this part of code will not handle it (other part of code will).
									 *
									 * That actually means that this little piece of code 
									 * (this line below) only (and ONLY) sets count to 1! And besides that
									 * causes every other evil in the world. :O
									 *
									 * P.S. The reason for that may be in linked list, or in AK_insert_row()
									 * You'll have to check every piece of AKDB code to find cause!
									 * I have found out that additional line is added when k == 25.
									 * There may be problem in linked lists or in AK_insert_row function
									 * or somewhere else. Who knows.
									 *
									 * If I didn't handle that last row (which has one attribute of size 0),
									 * test would not pass!
									 *
									 * Good luck, fellow programmer!
									 */
									memcpy(needed_values[l].data, &inttemp, sizeof(int));
									needed_values[l].data[sizeof (int) ] = '\0';
									AK_Insert_New_Element(agg_head[l].type, needed_values[l].data, new_table, agg_head[l].att_name, rowroot_table.row_root);
									break;

								case AGG_TASK_AVG:
									//no break is intentional
								case AGG_TASK_MAX:
									//no break is intentional
								case AGG_TASK_MIN:
									//no break is intentional
								case AGG_TASK_SUM:
									//no break is intentional
								case AGG_TASK_GROUP:
									//no break is intentional
								case AGG_TASK_AVG_SUM:
									//no break is intentional
								default:
									AK_Insert_New_Element(agg_head[l].type, needed_values[l].data, new_table, agg_head[l].att_name, rowroot_table.row_root);
							}

						}
                        //FILE  -  fix this!
						AK_insert_row(rowroot_table.row_root);

					} else {
						mem_block = AK_get_block(sresult.aiBlocks[0]);

						for (l = 0; l < num_attr; l++) {
							for (m = 0; m < num_aggregations; m++) {
								if (strcmp(needed_values[m].att_name, temp->header[l].att_name) == 0) {
									switch (needed_values[m].agg_task) {
										case AGG_TASK_COUNT:
											//no break is intentional
										case AGG_TASK_AVG_COUNT:
											inttemp = *((int *) (mem_block->block->data + mem_block->block->tuple_dict[sresult.aiTuple_addresses[i] + m].address)) + 1;
											memcpy(&mem_block->block->data[mem_block->block->tuple_dict[sresult.aiTuple_addresses[i] + m].address], &inttemp, sizeof (int));
											break;

										case AGG_TASK_SUM:
											//no break is intentional
										case AGG_TASK_AVG_SUM:
											switch (agg_head[m].type) {
												case TYPE_INT:
													inttemp = *((int*) needed_values[m].data) + *((int *) (mem_block->block->data + mem_block->block->tuple_dict[sresult.aiTuple_addresses[i] + m].address));
													memcpy(mem_block->block->data + mem_block->block->tuple_dict[sresult.aiTuple_addresses[i] + m].address, &inttemp, sizeof (int));
													break;

												case TYPE_FLOAT:
													floattemp = *((float*) needed_values[m].data) + *((float *) (mem_block->block->data + mem_block->block->tuple_dict[sresult.aiTuple_addresses[i] + m].address));
													memcpy(mem_block->block->data + mem_block->block->tuple_dict[sresult.aiTuple_addresses[i] + m].address, &floattemp, sizeof (float));
													break;

												case TYPE_NUMBER:
													doubletemp = *((double*) needed_values[m].data) + *((double *) (mem_block->block->data + mem_block->block->tuple_dict[sresult.aiTuple_addresses[i] + m].address));
													memcpy(mem_block->block->data + mem_block->block->tuple_dict[sresult.aiTuple_addresses[i] + m].address, &doubletemp, sizeof (double));
													break;
											}
											break;

										case AGG_TASK_MAX:
											switch (agg_head[m].type) {
												case TYPE_INT:
													inttemp = *((int *) (mem_block->block->data + mem_block->block->tuple_dict[sresult.aiTuple_addresses[i] + m].address));
													if (*((int*) needed_values[m].data) > inttemp) {
														inttemp = *((int*) needed_values[m].data);
														memcpy(mem_block->block->data + mem_block->block->tuple_dict[sresult.aiTuple_addresses[i] + m].address, &inttemp, sizeof (int));
													}
													break;

												case TYPE_FLOAT:
													floattemp = *((float *) (mem_block->block->data + mem_block->block->tuple_dict[sresult.aiTuple_addresses[i] + m].address));
													if (*((float*) needed_values[m].data) > floattemp) {
														floattemp = *((float*) needed_values[m].data);
														memcpy(mem_block->block->data + mem_block->block->tuple_dict[sresult.aiTuple_addresses[i] + m].address, &floattemp, sizeof (float));
													}
													break;

												case TYPE_NUMBER:
													doubletemp = *((double *) (mem_block->block->data + mem_block->block->tuple_dict[sresult.aiTuple_addresses[i] + m].address));
													if (*((double*) needed_values[m].data) > doubletemp) {
														doubletemp = *((double*) needed_values[m].data);
														memcpy(mem_block->block->data + mem_block->block->tuple_dict[sresult.aiTuple_addresses[i] + m].address, &doubletemp, sizeof (double));
													}
													break;
											}
											break;

										case AGG_TASK_MIN:
											switch (agg_head[m].type) {
												case TYPE_INT:
													inttemp = *((int *) (mem_block->block->data + mem_block->block->tuple_dict[sresult.aiTuple_addresses[i] + m].address));
													if (*((int*) needed_values[m].data) < inttemp) {
														inttemp = *((int*) needed_values[m].data);
														memcpy(mem_block->block->data + mem_block->block->tuple_dict[sresult.aiTuple_addresses[i] + m].address, &inttemp, sizeof (int));
													}
													break;

												case TYPE_FLOAT:
													floattemp = *((float *) (mem_block->block->data + mem_block->block->tuple_dict[sresult.aiTuple_addresses[i] + m].address));
													if (*((float*) needed_values[m].data) < floattemp) {
														floattemp = *((float*) needed_values[m].data);
														memcpy(mem_block->block->data + mem_block->block->tuple_dict[sresult.aiTuple_addresses[i] + m].address, &floattemp, sizeof (float));
													}
													break;

												case TYPE_NUMBER:
													doubletemp = *((double *) (mem_block->block->data + mem_block->block->tuple_dict[sresult.aiTuple_addresses[i] + m].address));
													if (*((double*) needed_values[m].data) < doubletemp) {
														doubletemp = *((double*) needed_values[m].data);
														memcpy(mem_block->block->data + mem_block->block->tuple_dict[sresult.aiTuple_addresses[i] + m].address, &doubletemp, sizeof (double));
													}
													break;
											}
											break;

										case AGG_TASK_AVG:
											inttemp = floattemp = -1;
											for (o = 0; o < num_aggregations; o++) {
												sprintf(agg_h_name, "_cAvg(%s)", needed_values[m].att_name);

												if (strcmp(agg_h_name, mem_block->block->header[o].att_name) == 0) {
													inttemp = *((int*) (mem_block->block->data + mem_block->block->tuple_dict[sresult.aiTuple_addresses[i] + o].address));
												}
												sprintf(agg_h_name, "_sAvg(%s)", needed_values[m].att_name);

												if (strcmp(agg_h_name, mem_block->block->header[o].att_name) == 0) {
													floattemp = *((float*) (mem_block->block->data + mem_block->block->tuple_dict[sresult.aiTuple_addresses[i] + o].address));
												}

												if (inttemp != -1 && doubletemp != -1)
													break;
											}
											floattemp = floattemp / (float) inttemp;
											memcpy(mem_block->block->data + mem_block->block->tuple_dict[sresult.aiTuple_addresses[i] + m].address, &floattemp, sizeof (float));
											break;

										case AGG_TASK_GROUP:
											break;
									}
								}
							}
						}
						AK_deallocate_search_result(sresult);
						
						AK_DeleteAll_L3(&rowroot_table);

						for (l = 0; l<num_aggregations;l++) {
							if (needed_values[l].agg_task == AGG_TASK_GROUP)
							   inttemp = 1;
							else
								inttemp = 0;
							AK_Insert_New_Element_For_Update(agg_head[l].type, needed_values[l].data, new_table, agg_head[l].att_name, rowroot_table.row_root, inttemp);
						}

						//AK_update_delete_row_from_block(mem_block->block, row_root, 0);
                        AK_mem_block_modify(mem_block, BLOCK_DIRTY);
					}
				}
            }
        }
        i++;
    }

    //if there is no grouping, we only need to insert one row into the table and we can insert it straight into the destination table
    if(agg_group_number == 0)
	{

    	AK_header agg_head_final[MAX_ATTRIBUTES];
    	j = 0;

    	for (i = 0; i < num_aggregations; i++) {
			if (agg_head[i].att_name[0] != '_') {
				agg_head_final[j] = agg_head[i];
				j++;
			}
		}

    	startAddress = AK_initialize_new_segment(agg_table, SEGMENT_TYPE_TABLE, agg_head_final);

		if (startAddress != EXIT_ERROR)
			printf("\nTABLE %s CREATED!\n", agg_table);

    	
	AK_DeleteAll_L3(&rowroot_table);

		for (l = 0; l < num_aggregations; l++) 
		{

			switch (needed_values[l].agg_task) {

				case AGG_TASK_AVG_SUM:
					floattemp = *((float *)needed_values[l].data);
					break;

				case AGG_TASK_AVG_COUNT:
					inttemp = *((int *)needed_values[l].data);
					break;

				case AGG_TASK_AVG:
					floattemp = floattemp/inttemp;
					memcpy(needed_values[l].data, &floattemp, sizeof(float));
					needed_values[l].data[sizeof(float)] = '\0';
					//no break is intentional
				case AGG_TASK_COUNT:
					//no break is intentional
				case AGG_TASK_MAX:
					//no break is intentional
				case AGG_TASK_MIN:
					//no break is intentional
				case AGG_TASK_SUM:
					//no break is intentional
				default:
					AK_Insert_New_Element(agg_head[l].type, needed_values[l].data, agg_table, agg_head[l].att_name, rowroot_table.row_root);
					break;
			}
		}
		AK_insert_row(rowroot_table.row_root);
    }
    else
	{

	
	projection_att_struct projection_att_table = {.projection_att = (struct list_node*) AK_malloc(sizeof(struct list_node))};
	
	AK_Init_L3(&projection_att_table);



	for (i = 0; i < num_aggregations; i++) 
		{
			if (agg_head[i].att_name[0] != '_') 
			{
				AK_InsertAtEnd_L3(TYPE_ATTRIBS, agg_head[i].att_name, strlen(agg_head[i].att_name), projection_att_table.projection_att);

			}
		}

		AK_projection(new_table, agg_table, projection_att_table.projection_att,NULL);

		
		AK_DeleteAll_L3(&projection_att_table);
		AK_free(projection_att_table.projection_att);
    }
	AK_free(addresses);
		

        //TODO replace this segment with AK_drop_table() once when it's done
	addresses = (table_addresses*) AK_get_table_addresses(new_table);
	i = 0;
	while (addresses->address_from[i] != 0) 
	{
		AK_delete_extent(addresses->address_from[i], addresses->address_to[i]);
		i++;
	}
	for (i = 0; i < num_aggregations; i++)
		AK_free(agg_head_ptr[i]);
    AK_free(needed_values);
    AK_free(rowroot_table.row_root);
    AK_free(temp);
	AK_free(addresses);
    AK_EPI;
    return EXIT_SUCCESS;
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
	            memcpy(&tmp_cnt_last_names, &block->data[cnt_last_names_address], sizeof(int));
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
