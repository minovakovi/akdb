/**
@file aggregation.h Header file that provides data structures, functions and defines for aggregation and grouping
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

#ifndef AGGREGATION
#define AGGREGATION

#include "../auxi/test.h"
#include "selection.h"
#include "projection.h"
#include "../file/filesearch.h"
#include "../auxi/mempro.h"
#include "../sql/drop.h"

#define AGG_TASK_GROUP 1
#define AGG_TASK_COUNT 2
#define AGG_TASK_SUM 3
#define AGG_TASK_MAX 4
#define AGG_TASK_MIN 5
#define AGG_TASK_AVG 6
#define AGG_TASK_AVG_COUNT 10 //used internaly
#define AGG_TASK_AVG_SUM 11 //used internaly

/**
  * @author Unknown
  * @struct AK_agg_value
  * @brief Structure that contains atribute name, date and aggregation task associated
  */
typedef struct {
    char att_name[MAX_ATT_NAME];
    char data[ MAX_VARCHAR_LENGTH ];
    int agg_task;
} AK_agg_value;
/**
  * @author Unknown
  * @struct AK_agg_input
  * @brief Structure that contains attributes from table header, tasks for this table and counter value
  */
typedef struct {
    AK_header attributes[MAX_ATTRIBUTES];
    int tasks[MAX_ATTRIBUTES];
    int counter;
} AK_agg_input;

/**
  * @author Ena Dujak
  * @struct rowroot_struct
  * @brief Structure that defines a new row in table using list_node
  */
typedef struct{
    struct list_node * row_root;
} rowroot_struct;
/**
  * @author Ena Dujak
  * @struct projection_att_struct
  * @brief Structure that defines projection_att which is a new list_node
  */
typedef struct{
    struct list_node * projection_att;
} projection_att_struct;

/**
 @author Dejan Frankovic
 @brief  Function that calculates how many attributes there are in the header with a while loop.
 @param  header A header array
 @return Number of attributes defined in header array

 */
int AK_header_size(AK_header *);

/**
  @author Dejan Frankovic
  @brief  Function that initializes the input object for aggregation with init values
  @param  input the input object
  @return No return value
 */
void AK_agg_input_init(AK_agg_input *input);

/**
  @author Dejan Frankovic
  @brief  Function that adds a header with a task in input object for aggregation
  @param header a header that is being aggregated
  @param agg_task the task which is to be done on the header
  @param input the input object
  @return On success, returns EXIT_SUCCESS, otherwise EXIT_FAILURE
  */
int AK_agg_input_add(AK_header header, int agg_task, AK_agg_input *input);

/**
  @author Dejan Frankovic
  @brief Function that adds a header with a task on the beginning of the input object for aggregation.
		 With the use of for loop existing attributes and tasks are moved from one place forward in input object
  @param header a header that is being aggregated
  @param agg_task  the task which is to be done on the header
  @param input the input object
  @return On success, returns EXIT_SUCCESS, otherwise EXIT_FAILURE

 */
int AK_agg_input_add_to_beginning(AK_header header, int agg_task, AK_agg_input *input);

/**
  @author Dejan Frankovic
  @brief  function that handles AVG (average) aggregation. It  goes through array of tasks in input
          object until it comes to task with a value of -1. While loop examines whether the task in array is equal to
          AGG_TASK_AVG. If so, AGG_TASK_AVG_COUNT is put on the beginning of input object. After that,
          AGG_TASK_AVG_SUM is put on the beginning of input object.
  @param input the input object
  @return No return value
 */
void AK_agg_input_fix(AK_agg_input *input);

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
int AK_aggregation(AK_agg_input *input, char *source_table, char *agg_table);
TestResult AK_aggregation_test();

#endif
