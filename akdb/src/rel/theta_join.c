/**
@file theta_join.c Provides functions for relational theta join operation
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

#include "theta_join.h"

/**
 * @author Tomislav Mikulček
 * @brief  Function that creates a header of the new table for theta join
 * @param srcTable1 name of the first table
 * @param srcTable2 name of the second table
 * @param new_table name of the destination table
 * @return EXIT_SUCCESS if the header was successfully created and EXIT_ERROR if the renamed headers are too long
 */
int AK_create_theta_join_header(char *srcTable1, char * srcTable2, char *new_table) {
	AK_PRO;
	table_addresses *src_addr1 = (table_addresses *) AK_get_table_addresses(srcTable1);
	table_addresses *src_addr2 = (table_addresses *) AK_get_table_addresses(srcTable2);

	int table_address1 = src_addr1->address_from[0];
	int table_address2 = src_addr2->address_from[0];

    AK_block *temp_block_tbl1 = (AK_block *) AK_read_block(table_address1);
    AK_block *temp_block_tbl2 = (AK_block *) AK_read_block(table_address2);

	// The same header is written in all allocated table blocks.
	//This is wrong and needs to be corrected.
	//If header doesn't fit in the first block than system must write the remain attributes from header to the new block.
	//Correction must be handled in all functions that write, read or count header attributes.
    
    
    int head1 = 0; 		//counter of the heads
    int head2 = 0; 		//counter of the heads
    while (strcmp(temp_block_tbl1->header[head1].att_name, "") != 0) head1++;
    while (strcmp(temp_block_tbl2->header[head2].att_name, "") != 0) head2++;
    int header_size = head1+head2+1;
    AK_header header[header_size];
    memset(header, 0, sizeof( AK_header ) * header_size);
   
    head1 = 0;
    head2 = 0;
    int new_head = 0; 	//counter of heads to write
    int rename = 0;		//indicate if we copy these header or not
    int length_tbl1 = strlen(srcTable1);
    int length_tbl2 = strlen(srcTable2);
    char *renamed_att;
    //first we copy all of the column names from the first table
    while (strcmp(temp_block_tbl1->header[head1].att_name, "") != 0) {

    	AK_dbg_messg(HIGH, REL_OP, "Theta join: Copying attribute header: %s from first table", temp_block_tbl1->header[head1].att_name);
    	memcpy(&header[head1], &temp_block_tbl1->header[head1], sizeof (temp_block_tbl1->header[head1]));
    	head1++;
    }

    new_head = head1;

    //we start copying column names from the second table
    while (strcmp(temp_block_tbl2->header[head2].att_name, "") != 0) {

    	head1 = 0;

    	//we need to check if the names of the columns from the second table overlap with the names from the first table
    	while (strcmp(temp_block_tbl1->header[head1].att_name, "") != 0) {

			if (strcmp(temp_block_tbl1->header[head1].att_name, temp_block_tbl2->header[head2].att_name) == 0){
				rename = 1;
				break;
			}

			head1++;
    	}

    	AK_dbg_messg(HIGH, REL_OP, "Theta join: Copying attribute header: %s from second table", temp_block_tbl2->header[head2].att_name);
    	memcpy(&header[new_head], &temp_block_tbl2->header[head2], sizeof(temp_block_tbl2->header[head2]));


    	//if an overlap is found, the column is renamed by prepending its table name and a dot.
    	if (rename){

    		AK_dbg_messg(HIGH, REL_OP, "Theta join: renaming attribute: %s", temp_block_tbl2->header[head2].att_name);

    		renamed_att = AK_malloc(length_tbl1 + strlen(temp_block_tbl1->header[head1].att_name) + 2);
    		memcpy(renamed_att, srcTable1, length_tbl1);
    		memcpy(renamed_att + length_tbl1, ".", strlen("."));
    		memcpy(renamed_att + length_tbl1 + 1, temp_block_tbl1->header[head1].att_name, strlen(temp_block_tbl1->header[head1].att_name) + 1);

    		if (strlen(renamed_att) > MAX_ATT_NAME){
    			AK_dbg_messg(HIGH, REL_OP, "Theta join: renaming failed for attribute: %s (name is too long)", renamed_att);
			AK_EPI;
    			return EXIT_ERROR;
    		}
    		else
    			memcpy(header[head1].att_name, renamed_att, strlen(renamed_att) + 1);

    		AK_free(renamed_att);

    		renamed_att = AK_malloc(length_tbl2 + strlen(temp_block_tbl2->header[head2].att_name) + 2);
    		memcpy(renamed_att, srcTable2, length_tbl2);
			memcpy(renamed_att + length_tbl2, ".", strlen("."));
			memcpy(renamed_att + length_tbl2 + 1, temp_block_tbl2->header[head2].att_name, strlen(temp_block_tbl2->header[head2].att_name) + 1);

			if (strlen(renamed_att) > MAX_ATT_NAME){
				AK_dbg_messg(HIGH, REL_OP, "Theta join: renaming failed for attribute: %s (name is too long)", renamed_att);
				AK_EPI;
				return EXIT_ERROR;
			}
			else
				memcpy(header[new_head].att_name, renamed_att, strlen(renamed_att) + 1);

			AK_free(renamed_att);
			rename = 0;
    	}

    	new_head++;
		head2++;
    }

    AK_free(temp_block_tbl1);
    AK_free(temp_block_tbl2);

    AK_temp_create_table(new_table, header, SEGMENT_TYPE_TABLE);
    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Tomislav Mikulček
 * @brief Function that iterates through blocks of the two tables and copies the rows which pass the constraint check into the new table
 * @param tbl1_temp_block block of the first table
 * @param tbl2_temp_block block of the second join table
 * @param tbl1_num_att number of attributes in the first table
 * @param tbl2_num_att number of attributes in the second table
 * @param constraints list of attributes, (in)equality and logical operators which are the conditions for the join in postfix notation
 * @param new_table name of the theta_join table
 * @return No return value
 */
void AK_check_constraints(AK_block *tbl1_temp_block, AK_block *tbl2_temp_block, int tbl1_num_att, int tbl2_num_att, struct list_node *constraints, char *new_table) {
    AK_PRO;
    AK_dbg_messg(HIGH, REL_OP, "\n COPYING THETA JOIN");

    int tbl1_att, tbl2_att, tbl1_row, tbl2_row;
    int address, size, type;
    char data[MAX_VARCHAR_LENGTH];

    struct list_node *row_root_init = (struct list_node *) AK_malloc(sizeof (struct list_node));
    struct list_node *row_root_full;

    AK_header *t_header = (AK_header *) AK_get_header(new_table);

    AK_Init_L3(&row_root_init);

    for (tbl1_row = 0; tbl1_row < DATA_BLOCK_SIZE; tbl1_row += tbl1_num_att){

    	if (tbl1_temp_block->tuple_dict[tbl1_row].type == FREE_INT)
			break;

		for (tbl1_att = 0; tbl1_att < tbl1_num_att; tbl1_att++){
			address = tbl1_temp_block->tuple_dict[tbl1_row + tbl1_att].address;
			size = tbl1_temp_block->tuple_dict[tbl1_row + tbl1_att].size;
			type = tbl1_temp_block->tuple_dict[tbl1_row + tbl1_att].type;
			memset(data, 0, MAX_VARCHAR_LENGTH);
			memcpy(data, &(tbl1_temp_block->data[address]), size);
			AK_Insert_New_Element(type, data, new_table, t_header[tbl1_att].att_name, row_root_init);
		}


    	for (tbl2_row = 0; tbl2_row < DATA_BLOCK_SIZE; tbl2_row += tbl2_num_att){

    		if (tbl2_temp_block->tuple_dict[tbl2_row].type == FREE_INT)
				break;

    		row_root_full = row_root_init;

    		for (tbl2_att = 0; tbl2_att < tbl2_num_att; tbl2_att++){
				address = tbl2_temp_block->tuple_dict[tbl2_row + tbl2_att].address;
				size = tbl2_temp_block->tuple_dict[tbl2_row + tbl2_att].size;
				type = tbl2_temp_block->tuple_dict[tbl2_row + tbl2_att].type;
				memset(data, 0, MAX_VARCHAR_LENGTH);
				memcpy(data, &(tbl2_temp_block->data[address]), size);
				AK_Insert_New_Element(type, data, new_table, t_header[tbl1_att + tbl2_att].att_name, row_root_full);
			}

			if (AK_check_if_row_satisfies_expression(row_root_full, constraints)){
    			AK_insert_row(row_root_full);
			}
    	}

    	
    	AK_DeleteAll_L3(&row_root_init);
    }

    AK_free(row_root_init);
    AK_EPI;
}

/**
 * @author Tomislav Mikulček,updated by Nikola Miljancic
 * @brief Function that creates a theta join betwen two tables on specified conditions. Names of the attibutes in the constraints parameter must be prefixed
 *         with the table name followed by a dot if and only if they exist in both tables. This is left for the preprocessing. Also, for now the constraints  
 *	   must come from the two source tables and not from a third.
 * @param srcTable1 name of the first table to join
 * @param srcTable2 name of the second table to join
 * @param constraints list of attributes, (in)equality and logical operators which are the conditions for the join in postfix notation
 * @param dstTable name of the theta join table
 * @return if successful returns EXIT_SUCCESS and EXIT_ERROR otherwise
 *   
 */
//int AK_theta_join(char *srcTable1, char * srcTable2, char * dstTable, AK_list *constraints) {
int AK_theta_join(char *srcTable1, char * srcTable2, char * dstTable, struct list_node *constraints) {
	AK_PRO;
	table_addresses *src_addr1 = (table_addresses *) AK_get_table_addresses(srcTable1);
    table_addresses *src_addr2 = (table_addresses *) AK_get_table_addresses(srcTable2);

    int startAddress1 = src_addr1->address_from[0];
    int startAddress2 = src_addr2->address_from[0];
    int tbl1_num_att = AK_num_attr(srcTable1);
    int tbl2_num_att = AK_num_attr(srcTable2);

    if ((startAddress1 != 0) && (startAddress2 != 0)) {
        if (AK_create_theta_join_header(srcTable1, srcTable2, dstTable) == EXIT_ERROR){
		AK_EPI;
        	return EXIT_ERROR;
	}

        AK_dbg_messg(LOW, REL_OP, "\nTABLE %s CREATED from %s and %s\n", dstTable, srcTable1, srcTable2);
		AK_dbg_messg(MIDDLE, REL_OP, "\nAK_theta_join: start copying data\n");

        AK_mem_block *tbl1_temp_block, *tbl2_temp_block;

        int i, j, k, l;
        i = j = k = l = 0;

        //for each extent in table1 that contains blocks needed for join
        for (i = 0; (i < src_addr1->address_from[i]) != 0; i++) {
            startAddress1 = src_addr1->address_from[i];

            if (startAddress1 != 0) {
                AK_dbg_messg(MIDDLE, REL_OP, "\nTheta join: copying extent of table 1: %d\n", i);

                //for each block in table1 extent
                for (j = startAddress1; j < src_addr1->address_to[i]; j++) {
                    AK_dbg_messg(MIDDLE, REL_OP, "Theta join: copying block of table 1: %d\n", j);

                    tbl1_temp_block = (AK_mem_block *) AK_get_block(j);

                    //if there is data in the block
                    if (tbl1_temp_block->block->AK_free_space != 0) {

                        //for each extent in table2 that contains blocks needed for join
                        for (k = 0; (k < src_addr2->address_from[k]) != 0; k++) {
                            startAddress2 = src_addr2->address_from[k];

                            if (startAddress2 != 0) {
                                AK_dbg_messg(MIDDLE, REL_OP, "Theta join: copying extent of table 2: %d\n", k);

                                //for each block in table2 extent
                                for (l = startAddress2; l < src_addr2->address_to[k]; l++) {
                                    AK_dbg_messg(MIDDLE, REL_OP, "Theta join: copying block of table 2: %d\n", l);

                                    tbl2_temp_block = (AK_mem_block *) AK_get_block(l);

                                    //if there is data in the block
                                    if (tbl2_temp_block->block->AK_free_space != 0) {

                                    		AK_check_constraints(tbl1_temp_block->block, tbl2_temp_block->block, tbl1_num_att, tbl2_num_att, constraints, dstTable);
                                    }
                                }
                            } else break;
                        }
                    }
                }
            } else break;
        }

        AK_free(src_addr1);
        AK_free(src_addr2);

		AK_dbg_messg(LOW, REL_OP, "THETA_JOIN_SUCCESS\n\n");
	AK_EPI;
        return EXIT_SUCCESS;
    } else {

        AK_dbg_messg(LOW, REL_OP, "\n AK_theta_join: Table doesn't exist!");

        AK_free(src_addr1);
        AK_free(src_addr2);
	AK_EPI;
        return EXIT_ERROR;
    }
}

/**
 * @author Tomislav Mikulček
 * @brief Function for testing the theta join
 * @return No return value
 */
TestResult AK_op_theta_join_test() {
    AK_PRO;
    printf("\n********** THETA JOIN TEST **********\n\n");

    struct list_node *constraints = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&constraints);
    
    //test where no column names overlap
    printf("SELECT * FROM department, professor WHERE manager = lastname;\n");
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "manager", sizeof ("manager"), constraints);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "lastname", sizeof ("lastname"), constraints);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "=", sizeof ("="), constraints);
    
    AK_theta_join("department", "professor", "theta_join_test", constraints);
    AK_print_table("theta_join_test");

    AK_DeleteAll_L3(&constraints);
    
    printf("SELECT * FROM student, professor2 WHERE id_prof=mbr;\n");

    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "id_prof", sizeof ("id_prof"), constraints);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "mbr", sizeof ("mbr"), constraints);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "=", sizeof ("="), constraints);
 
   
    
    AK_theta_join("student", "professor2", "theta_join_test2", constraints);
    AK_print_table("theta_join_test2");
    AK_DeleteAll_L3(&constraints);
    
    //test where overlaping columns are a part of the constraints
    printf("SELECT * FROM employee, department WHERE employee.id_department = department.id_department;\n");   
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "employee.id_department", sizeof ("employee.id_department"), constraints);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "department.id_department", sizeof ("department.id_department"), constraints);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "=", sizeof ("="), constraints);

    AK_theta_join("employee", "department", "theta_join_test3", constraints);
    AK_print_table("theta_join_test3");
    AK_DeleteAll_L3(&constraints);
    char num = 102;
    printf("SELECT * FROM student, professor2 WHERE year + id_prof > 37895;\n");
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof ("year"), constraints);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "id_prof", sizeof ("id_prof"), constraints);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "+", sizeof ("+"), constraints);
    AK_InsertAtEnd_L3(TYPE_INT, &num, sizeof (int), constraints);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, ">", sizeof (">"), constraints);

    AK_theta_join("student", "professor2", "theta_join_test4", constraints);
    AK_print_table("theta_join_test4");
    printf("Test is successful :) \n");
    AK_DeleteAll_L3(&constraints);
    
    AK_free(constraints);
    AK_EPI;
    return TEST_result(0,0);
}

