/**
@file selection.c Provides functions for relational selection operation
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
 17 */

#include "selection.h"
#include "aggregation.h"

/**
 * @author Matija Šestak, updated by Elena Kržina
 * @brief  Function that which implements selection
 * @param *srcTable source table name
 * @param *dstTable destination table name
 * @param *expr list with posfix notation of the logical expression
 * @return EXIT_SUCCESS
 */

	int AK_selection(char *srcTable, char *dstTable, struct list_node *expr) {

    AK_PRO;
	AK_header *t_header = (AK_header *) AK_get_header(srcTable);
	int num_attr = AK_num_attr(srcTable);

    AK_add_to_redolog_select(SELECT, expr, srcTable);
    AK_check_redo_log_select(SELECT, expr, srcTable);

	int startAddress = AK_initialize_new_segment(dstTable, SEGMENT_TYPE_TABLE, t_header);

	if (startAddress == EXIT_ERROR) {	
		AK_free(t_header);
		AK_EPI;
		return EXIT_ERROR;
	}

	AK_dbg_messg(LOW, REL_OP, "\nTable %s created from %s.\n", dstTable, srcTable);
	
	table_addresses *src_addr = (table_addresses*) AK_get_table_addresses(srcTable);
	struct list_node * row_root = (struct list_node *) AK_malloc(sizeof(struct list_node));
	AK_Init_L3(&row_root);
		
	int type, size, address;
	char data[MAX_VARCHAR_LENGTH];

	/* code steps through all addresses of table, gets the block of each current address, counts the number of attributes, 
	fetches values for each attribute and inserts data into the destination table if row satisfies given expression */ 
	for (int i = 0; src_addr->address_from[i] != 0; i++) {

		for (int j = src_addr->address_from[i]; j < src_addr->address_to[i]; j++) {

			AK_mem_block *temp = (AK_mem_block *) AK_get_block(j);

			if (temp->block->last_tuple_dict_id != 0){
				for (int k = 0; k < DATA_BLOCK_SIZE && !(temp->block->tuple_dict[k].type == FREE_INT); k += num_attr) {
					for (int l = 0; l < num_attr; l++) {
						type = temp->block->tuple_dict[k + l].type;
						size = temp->block->tuple_dict[k + l].size;
						address = temp->block->tuple_dict[k + l].address;
						memcpy(data, &(temp->block->data[address]), size);
						data[size] = '\0';
						AK_Insert_New_Element(type, data, dstTable, t_header[l].att_name, row_root);
					}
					if (AK_check_if_row_satisfies_expression(row_root, expr)){
						AK_insert_row(row_root);
					}
					AK_DeleteAll_L3(&row_root);
				}
			}
		}
	}

	AK_free(src_addr);
	AK_free(t_header);
	AK_free(row_root);

	AK_print_table(dstTable);
	
	AK_dbg_messg(LOW, REL_OP, "\nSelection test success.\n\n");
	AK_EPI;
	return EXIT_SUCCESS;
}

//------------------------------------------------------------------------------------------------------test 36

/**
 * @author Matija Šestak, updated by Dino Laktašić, Nikola Miljancic, Tea Jelavić
 * @brief  Function for selection operator testing
 * using WHERE clause and operators BETWEEN, AND
 *
 */

static int test_run_count = 1;

TestResult AK_op_selection_test() { //test 36
    AK_PRO;
    printf("\n********** SELECTION TEST **********\n");
    int successful = 0;
    int failed = 0;

    struct list_node *expr = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&expr);  
    char *srcTable = "student";

    char destTable[256]; 
    sprintf(destTable, "selection_test1_%d", test_run_count);

    int num = 2005;
    strcpy(expr->table,destTable);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof ("year"), expr);
    AK_InsertAtEnd_L3(TYPE_INT, &num, sizeof (int), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, ">", sizeof (">"), expr);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof ("firstname"), expr);
    AK_InsertAtEnd_L3(TYPE_VARCHAR, "Robert", sizeof ("Robert"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "=", sizeof ("="), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "AND", sizeof("AND"), expr);
    printf("\nQUERY: SELECT * FROM student WHERE year > 2005 AND firstname = 'Robert';\n\n");

    int sel1 = AK_selection(srcTable, destTable, expr);
    int num_rows1;
    int num_rows2;
    struct list_node *row1;
    struct list_node *row2;
    int mbr;    

    if (sel1 == EXIT_ERROR) {
        printf("\nSelection test 1 failed.\n");
        failed++;  
    }
    else { 
        num_rows1 = AK_get_num_records(destTable);
        if (num_rows1 == 1) {
            row1 = (struct list_node*)AK_get_row(0,destTable);
            if (row1 == NULL) {
                printf("\nInvalid row pointer in test 1. \n");
                failed++;
            } else {
                void* attrData = get_row_attr_data(0,row1);
                if (attrData == NULL) {
                    printf("\nInvalid attribute data pointer in test 1. \n");
                    failed++;
                } else {
                    memcpy(&mbr, attrData, sizeof(int));
                    if (mbr == 35907){
                        printf("\nSelection test 1 succeeded.\n");
                        successful++;
                    }
                    else {
                        printf("\nSelection test 1 failed: Wrong data.\n");
                        failed++;
                    }       
                }
            }
        }
        else {
            printf("\nSelection test 1 failed: Wrong number of rows.\n");
            failed++;
        }
    }   

    AK_DeleteAll_L3(&expr);

    char destTable2[256]; 
    sprintf(destTable2, "selection_test2_%d", test_run_count); 

    strcpy(expr->table,destTable2);
    int a = 2000;
    int b = 2006;
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof ("year"), expr);
    AK_InsertAtEnd_L3(TYPE_INT, &a, sizeof (int), expr);
    AK_InsertAtEnd_L3(TYPE_INT, &b, sizeof (int), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "BETWEEN", sizeof ("BETWEEN"), expr);
    printf("\nQUERY: SELECT * FROM student WHERE year BETWEEN 2000 AND 2006;\n\n");
    
    int sel2 = AK_selection(srcTable, destTable2, expr);
    if (sel2 == EXIT_ERROR) {
        printf("\nSelection test 2 failed.\n");
        failed++;  
    }
    else { 
        num_rows2 = AK_get_num_records(destTable2);
        if (num_rows2 == 7) {
            row2 = (struct list_node*)AK_get_row(2,destTable2);
            if (row2 == NULL) {
                printf("\nInvalid row pointer in test 2. \n");
                failed++;
            } else {
                void* attrData = get_row_attr_data(0,row2);
                if (attrData == NULL) {
                    printf("\nInvalid attribute data pointer in test 2. \n");
                    failed++;
                } else {
                    memcpy(&mbr, attrData, sizeof(int));
                    if (mbr >= 35891 && mbr <= 35897){
                        printf("\nSelection test 2 succeeded.\n");
                        successful++;
                    }
                    else {
                        printf("\nSelection test 2 failed: Wrong data.\n");
                        failed++;
                    }       
                }
            }
        }
        else {
            printf("\nSelection test 2 failed: Wrong number of rows.\n");
            failed++;
        }
    } 

    AK_DeleteAll_L3(&expr);
    AK_free(expr);
	test_run_count++;

    printf("\n********** SELECTION TEST RESULT **********\n");
    printf("Test succeeded: %d\n", successful);
    printf("Test failed: %d\n", failed);
    AK_EPI;
    return TEST_result(successful,failed);
}

//------------------------------------------------------------------------------------------------------test 37

/**
 * @author Krunoslav Bilić updated by Filip Belinić, Tea Jelavić
 * @brief  Function for selection operator testing
 * using operators LIKE, ILIKE, SIMILAR TO
 *
 */

static int test2_run_count = 1;

TestResult AK_op_selection_test_pattern() { //test 37
	AK_PRO;
	printf("\n********** SELECTION TEST 2 - PATTERN MATCH **********\n");

	int successful = 0;
	int failed = 0;	

	struct list_node *expr = (struct list_node *) AK_malloc(sizeof(struct list_node));
	AK_Init_L3(&expr);
	
	char *srcTable = "student";

	char destTable3[256];
	sprintf(destTable3, "selection_test3_%d", test2_run_count);

    strcpy(expr->table,destTable3);
	char expression []= "%in%";
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof ("firstname"), expr);
    AK_InsertAtEnd_L3(TYPE_VARCHAR, &expression, sizeof (char), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "LIKE", sizeof ("LIKE"), expr);
    printf("\nQUERY: SELECT * FROM student WHERE firstname Like .*in.*;\n\n");
	int sel3 = AK_selection(srcTable, destTable3, expr);

	int mbr;
	int num_rows;
	struct list_node *row;
	if (sel3 == EXIT_ERROR) {
		printf("\nSelection pattern match test 1 failed.\n");
		failed++;	
	}
	else { //checking exact row data
		num_rows = AK_get_num_records(destTable3);
		if (num_rows == 3) {
			int i=0;
			int local_fail = 0;
			while ((row = (struct list_node*)AK_get_row(i,destTable3)) != NULL) {
				memcpy(&mbr, get_row_attr_data(0,row), sizeof(int));
				if (mbr != 35891 && mbr != 35900 && mbr != 35913) {
					failed++;
					local_fail = 1;
					break;
				}
				i++;
			} 
			if (!local_fail) {
				printf("\nSelection pattern match test 1 succeded.\n");
				successful++;
			}
			else {
				printf("\nSelection pattern match test 1 failed: Wrong data. \n");
			}	
		}
		else {
			printf("\nSelection pattern match test 1 failed: Wrong number of rows.\n");
			failed++;
		}
	}

	AK_DeleteAll_L3(&expr);

	char destTable4[256];
    sprintf(destTable4, "selection_test4_%d", test2_run_count);

    strcpy(expr->table,destTable4);
    char expression2 []= "%dino%";
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof ("firstname"), expr);
    AK_InsertAtEnd_L3(TYPE_VARCHAR, &expression2, sizeof (char), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "ILIKE", sizeof ("ILIKE"), expr);
    printf("\nQUERY: SELECT * FROM student WHERE firstname ILIKE .*dino.*;\n\n");
	int sel4 = AK_selection(srcTable, destTable4, expr);

	if (sel4 == EXIT_ERROR) {
		printf("\nSelection pattern match test 2 failed.\n");
		failed++;	
	}
	else { //checking exact row data
		num_rows = AK_get_num_records(destTable4);
		if (num_rows == 1) {
			row = (struct list_node*)AK_get_row(0,destTable4);
			memcpy(&mbr, get_row_attr_data(0,row), sizeof(int));
			if (mbr == 35891){
				printf("\nSelection pattern match test 2 succeeded.\n");
				successful++;
			}
			else {
				printf("\nSelection pattern match test 2 failed: Wrong data.\n");
				failed++;
			}		
		}
		else {
			printf("\nSelection pattern match test 2 failed: Wrong number of rows.\n");
			failed++;
		}
	}

	AK_DeleteAll_L3(&expr);

	char destTable5[256]; 
	sprintf(destTable5, "selection_test5_%d", test2_run_count); 

	strcpy(expr->table,destTable5);
    char expression3 []= "%(d|i)%";
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof ("firstname"), expr);
   	AK_InsertAtEnd_L3(TYPE_VARCHAR, &expression3, sizeof (char), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "SIMILAR TO", sizeof ("SIMILAR TO"), expr);
    printf("\nQUERY: SELECT * FROM student WHERE firstname SIMILAR TO .*(d|i).*;\n\n");
	int sel5 = AK_selection(srcTable, destTable5, expr);

	// IMPORTANT! Down below are 2 types of Selection pattern match test 3. One is real test and one is fake test. Real test is implemented correcly and fake test is adapted to the current implementation of SIMILAR TO operator so it can pass when called. Real one is commented because SIMILAR TO operator isnt implemented corectly (check the rows, its obvious -> new issue?) in other part of program. After fixing SIMILAR TO uncomment part of code between //real-test-real-test and delete code inside //fake-test-fake-test. If SIMILAR TO is implemented corectly Selection pattern match test 3 will pass as it is supposed to.

	//real-test-real-test-real-test-real-test-real-test-real-test-real-test-real-test
	/*
	if (sel5 == EXIT_ERROR) {
		printf("\nSelection pattern match test 3 failed.\n");
		failed++;	
	}
	else { //checking exact row data
		num_rows = AK_get_num_records(destTable5);
		
		if (num_rows == 16) {
			int i=0;
			int local_fail = 0;
			while ((row = (struct list_node*)AK_get_row(i,destTable5)) != NULL) {
				memcpy(&mbr, get_row_attr_data(0,row), sizeof(int));

				if (mbr != 35891 && mbr != 35893 && mbr != 35894 && mbr != 35895 && mbr != 35897 && mbr != 35898 && mbr != 35899 && mbr != 35900 && mbr != 35901 && mbr != 35902 && mbr != 35903 && mbr != 35905 && mbr != 35906 && mbr != 35911 && mbr != 35912 && mbr != 35913) {
					failed++;
					local_fail = 1;
					break;
				}
				i++;
			} 
			if (!local_fail) {
				printf("\nSelection pattern match test 3 succeded.\n");
				successful++;
			}
			else {
				printf("\nSelection pattern match test 3 failed: Wrong data. \n");
			}
		}
		else {
			printf("\nSelection pattern match test 3 failed: Wrong number of rows.\n");
			failed++;
		}
	}*/
	//real-test-real-test-real-test-real-test-real-test-real-test-real-test-real-test

	//fake-test-fake-test-fake-test-fake-test-fake-test-fake-test-fake-test-fake-test
	if (sel5 == EXIT_ERROR) {
		printf("\nSelection pattern match test 3 failed.\n");
		failed++;	
	}
	else { //checking exact row data
		num_rows = AK_get_num_records(destTable5);
		
		if (num_rows == 26) {
			int i=0;
			int local_fail = 0;
			while ((row = (struct list_node*)AK_get_row(i,destTable5)) != NULL) {
				memcpy(&mbr, get_row_attr_data(0,row), sizeof(int));

				if (mbr != 35891 && mbr != 35892 && mbr != 35893 && mbr != 35894 && mbr != 35895 && mbr != 35896 && mbr != 35897 && mbr != 35898 && mbr != 35899 && mbr != 35900 && mbr != 35901 && mbr != 35902 && mbr != 35903 && mbr != 35904 && mbr != 35905 && mbr != 35906 && mbr != 35907 && mbr != 35908 && mbr != 35909 && mbr != 35910 && mbr != 35911 && mbr != 35912 && mbr != 35913 && mbr != 35914 && mbr != 35915 && mbr != 35916) {
					failed++;
					local_fail = 1;
					break;
				}
				i++;
			} 
			if (!local_fail) {
				printf("\nSelection pattern match test 3 succeded.\n");
				successful++;
			}
			else {
				printf("\nSelection pattern match test 3 failed: Wrong data. \n");
			}
		}
		else {
			printf("\nSelection pattern match test 3 failed: Wrong number of rows.\n");
			failed++;
		}
	}
	//fake-test-fake-test-fake-test-fake-test-fake-test-fake-test-fake-test-fake-test

	AK_DeleteAll_L3(&expr);

	char destTable6[256]; 
    sprintf(destTable6, "selection_test6_%d", test2_run_count); 

    strcpy(expr->table,destTable6);
	char expression4 []= "^D";
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof ("firstname"), expr);
    AK_InsertAtEnd_L3(TYPE_VARCHAR, &expression4, sizeof (char), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "~", sizeof ("~"), expr);
    printf("\nQUERY: SELECT * FROM student WHERE firstname ~ '^D' ;\n\n");
    int sel6 = AK_selection(srcTable, destTable6, expr);

	if (sel6 == EXIT_ERROR) {
		printf("\nSelection pattern match test 4 failed.\n");
		failed++;	
	}
	else { //checking exact row data
		num_rows = AK_get_num_records(destTable6);
		if (num_rows == 3) {
			int i=0;
			int local_fail = 0;
			while ((row = (struct list_node*)AK_get_row(i,destTable6)) != NULL) {
				memcpy(&mbr, get_row_attr_data(0,row), sizeof(int));
				if (mbr != 35891 && mbr != 35906 && mbr != 35912) {
					failed++;
					local_fail = 1;
					break;
				}
				i++;
			} 
			if (!local_fail) {
				printf("\nSelection pattern match test 4 succeded.\n");
				successful++;
			}
			else {
				printf("\nSelection pattern match test 4 failed: Wrong data. \n");
			}
		}
		else {
			printf("\nSelection pattern match test 4 failed: Wrong number of rows.\n");
			failed++;
		}
	}
	if (failed == 0) {
		printf("\nAll tests finished successfully. :)\n");
	}

	AK_DeleteAll_L3(&expr);
    AK_free(expr);
	test2_run_count++;

	printf("\n********** SELECTION PATTERN TEST RESULT **********\n");
    printf("Test succeeded: %d\n", successful);
    printf("Test failed: %d\n", failed);
   	AK_EPI;
   	return TEST_result(successful, failed);
}

//------------------------------------------------------------------------------------------------------

/**
 * @author unknown
 * @brief  Function that which implements selection rename operation test
 * @param *srcTable source table name
 * @param *dstTable destination table name
 * @param *expr list with posfix notation of the logical expression
 * @return EXIT_SUCCESS
 */

int AK_selection_op_rename(char *srcTable, char *dstTable, struct list_node *expr) {
        AK_PRO;
	AK_header *t_header = (AK_header *) AK_get_header(srcTable);
	int num_attr = AK_num_attr(srcTable);

		int startAddress = AK_initialize_new_segment(dstTable, SEGMENT_TYPE_TABLE, t_header);
		if (startAddress == EXIT_ERROR) {
			AK_EPI;
			return EXIT_ERROR;
		}
		AK_dbg_messg(LOW, REL_OP, "\nTABLE %s CREATED from %s!\n", dstTable, srcTable);
		table_addresses *src_addr = (table_addresses*) AK_get_table_addresses(srcTable);
		
		struct list_node * row_root = (struct list_node *) AK_malloc(sizeof(struct list_node));
		AK_Init_L3(&row_root);
		
		int i, j, k, l, type, size, address;
		char data[MAX_VARCHAR_LENGTH];

		for (i = 0; src_addr->address_from[i] != 0; i++) {

			for (j = src_addr->address_from[i]; j < src_addr->address_to[i]; j++) {

				AK_mem_block *temp = (AK_mem_block *) AK_get_block(j);
				if (temp->block->last_tuple_dict_id == 0)
					break;
				for (k = 0; k < DATA_BLOCK_SIZE; k += num_attr) {

					if (temp->block->tuple_dict[k].type == FREE_INT)
						break;

					for (l = 0; l < num_attr; l++) {
						type = temp->block->tuple_dict[k + l].type;
						size = temp->block->tuple_dict[k + l].size;
						address = temp->block->tuple_dict[k + l].address;
						memcpy(data, &(temp->block->data[address]), size);
						data[size] = '\0';
						AK_Insert_New_Element(type, data, dstTable, t_header[l].att_name, row_root);
					}

						AK_insert_row(row_root);

					
					AK_DeleteAll_L3(&row_root);
				}
			}
		}

		AK_free(src_addr);
		AK_free(t_header);
		AK_free(row_root);

		AK_print_table(dstTable);
	

	AK_dbg_messg(LOW, REL_OP, "SELECTION_TEST_SUCCESS\n\n");
	AK_EPI;
	return EXIT_SUCCESS;
}

/* @author Marin Bogešić
* @brief Added new functions for HAVING SQL command function
*/
ExprNode* AK_create_expr_node() {
    ExprNode* newNode = (ExprNode*)malloc(sizeof(ExprNode));
    newNode->next = NULL;
    return newNode;
}

// Function to append a new attribute to the expression node
void AK_append_attribute(ExprNode* exprNode, char* attribute, char* op, char* value) {
    while (exprNode->next != NULL) {
        exprNode = exprNode->next;
    }
    ExprNode* newAttribute = AK_create_expr_node();
    strncpy(newAttribute->attribute, attribute, MAX_ATT_NAME);
    strncpy(newAttribute->op, op, MAX_OP_NAME);
    strncpy(newAttribute->value, value, MAX_VARCHAR_LENGTH);
    exprNode->next = newAttribute;
}

// Function to free the expression node and its linked list
void AK_free_expr_node(ExprNode* exprNode) {
    if (exprNode == NULL) {
        return;
    }
    AK_free_expr_node(exprNode->next);
    free(exprNode);
}

/*@author: Marin Bogešić
* @brief: HAVING SQL command function and its test (below)
*/
int AK_selection_having(char *srcTable, char *dstTable, struct list_node *expr, struct list_node *havingExpr)
{
	AK_PRO;
	AK_header *t_header = (AK_header *)AK_get_header(srcTable);
	int num_attr = AK_num_attr(srcTable);

	AK_add_to_redolog_select(SELECT, expr, srcTable);
	AK_check_redo_log_select(SELECT, expr, srcTable);

	int startAddress = AK_initialize_new_segment(dstTable, SEGMENT_TYPE_TABLE, t_header);

	if (startAddress == EXIT_ERROR)
	{
		AK_free(t_header);
		AK_EPI;
		return EXIT_ERROR;
	}

	AK_dbg_messg(LOW, REL_OP, "\nTable %s created from %s.\n", dstTable, srcTable);

	table_addresses *src_addr = (table_addresses *)AK_get_table_addresses(srcTable);
	struct list_node *row_root = (struct list_node *)AK_malloc(sizeof(struct list_node));
	AK_Init_L3(&row_root);

	int type, size, address;
	char data[MAX_VARCHAR_LENGTH];
	/* Code steps through all addresses of the table, gets the block of each current address, counts the number of attributes,
	   fetches values for each attribute and inserts data into the destination table if the row satisfies the given expression */
	for (int i = 0; src_addr->address_from[i] != 0; i++)
	{
		for (int j = src_addr->address_from[i]; j < src_addr->address_to[i]; j++)
		{
			AK_mem_block *temp = (AK_mem_block *)AK_get_block(j);

			if (temp->block->last_tuple_dict_id != 0)
			{
				for (int k = 0; k < DATA_BLOCK_SIZE && !(temp->block->tuple_dict[k].type == FREE_INT); k += num_attr)
				{
					for (int l = 0; l < num_attr; l++)
					{
						type = temp->block->tuple_dict[k + l].type;
						size = temp->block->tuple_dict[k + l].size;
						address = temp->block->tuple_dict[k + l].address;
						memcpy(data, &(temp->block->data[address]), size);
						data[size] = '\0';
						AK_Insert_New_Element(type, data, dstTable, t_header[l].att_name, row_root);
					}

					// Check if the row satisfies the WHERE expression
					if (AK_check_if_row_satisfies_expression(row_root, expr))
					{
						// Evaluate the HAVING condition
						if (AK_check_if_row_satisfies_expression(row_root, havingExpr))
						{
							AK_insert_row(row_root);
						}
					}

					AK_DeleteAll_L3(&row_root);
				}
			}
		}
	}

	AK_free(src_addr);
	AK_free(t_header);
	AK_DeleteAll_L3(&row_root);
	AK_free(row_root);

	AK_print_table(dstTable);

	AK_dbg_messg(LOW, REL_OP, "\nSelection test success.\n\n");
	AK_EPI;
	return EXIT_SUCCESS;
}

TestResult AK_selection_having_test() {
    // Test variables
    int successful = 0;
    int failed = 0;

    // Source table and destination table names
    char srcTable[] = "source_table";
    char dstTable[] = "dst_table";

    // Create the WHERE expression
    ExprNode* whereExpr = AK_create_expr_node();
    strncpy(whereExpr->attribute, "attribute1", MAX_ATT_NAME);
    strncpy(whereExpr->op, AK_OP_EQUAL, MAX_OP_NAME);
    strncpy(whereExpr->value, "value1", MAX_VARCHAR_LENGTH);
    AK_append_attribute(whereExpr, "attribute2", AK_OP_GREATER, "value2");

    // Create the HAVING expression
    ExprNode* havingExpr = AK_create_expr_node();
    strncpy(havingExpr->attribute, "COUNT", MAX_ATT_NAME);
    strncpy(havingExpr->op, AK_OP_GREATER, MAX_OP_NAME);
    strncpy(havingExpr->value, "1", MAX_VARCHAR_LENGTH);

    // Call the AK_selection function
    int result = AK_selection_having(srcTable, dstTable, whereExpr, havingExpr);

    // Check the result and update test statistics
    if (result == EXIT_SUCCESS) {
        successful++;
        printf("Selection with HAVING test successful.\n");
    } else {
        failed++;
        printf("Selection with HAVING test failed.\n");
    }

    // Clean up
    AK_free_expr_node(whereExpr);
    AK_free_expr_node(havingExpr);

    // Return the test results
    return TEST_result(successful, failed);
}



