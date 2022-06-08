/**
@file nat_join.c Provides functions for relational natural join operation
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

#include "nat_join.h"

/**
 * @author Matija Novak, optimized, and updated to work with AK_list by Dino Laktašić
 * @brief  Function that makes a header for the new table and call the function to create the segment
 * @param table_address1 address of the block of the first table
 * @param table_address2 address of the block of the second table
 * @param new_table name of the join table
 * @param att_root ttributes on which we make nat_join
 * @return No return value
 */
//void AK_create_join_block_header(int table_address1, int table_address2, char *new_table, AK_list *att) {
void AK_create_join_block_header(int table_address1, int table_address2, char *new_table, struct list_node *att) {
    AK_PRO;
    AK_block *temp_block = (AK_block *) AK_read_block(table_address1);
    
	//Currently it works with headers no longer then MAX_ATTRIBUTES. The same header is written in all allocated table blocks.
	//This is wrong and need to be corrected.
	//If header doesn't fit in the first block than system must write the remain attributes from header to the new block.
	//Correction must be handled in all functions that write, read or count header attributes.
    
    AK_header header[MAX_ATTRIBUTES];
    memset(header, 0, sizeof( AK_header ) * MAX_ATTRIBUTES);

    int head = 0; 		//counter of the heads
    int new_head = 0; 	//counter of heads to write
    int s_copy; 		//indicate if we copy these header or not

    struct list_node *list_elem;
	
    while (strcmp(temp_block->header[head].att_name, "") != 0) {
        s_copy = 1;
	list_elem = AK_First_L2(att);
	
        while (list_elem != NULL) {
            //if is element on wich we make join skip it
            if (strcmp(list_elem->data, temp_block->header[head].att_name) == 0) {
                s_copy = 0;
                break;
            }
            list_elem = list_elem->next;
        }

        //Copy table1 header
        if (s_copy) {
            memcpy(&header[new_head], &temp_block->header[head], sizeof (temp_block->header[head]));
            AK_dbg_messg(HIGH, REL_OP, "Natural join: Copy attribute header: %s", header[new_head].att_name);
            new_head++;
        }
        head++;
    }

    AK_free(temp_block);
    temp_block = (AK_block *) AK_read_block(table_address2);
    head = 0;

    //add table2 headers
    while (strcmp(temp_block->header[head].att_name, "") != 0) {
        //copy all headers if some exist
        memcpy(&header[new_head], &temp_block->header[head], sizeof (temp_block->header[head]));

        AK_dbg_messg(HIGH, REL_OP, "Natural join: Copy attribute header2: %s", header[new_head].att_name);
        new_head++;
        head++;
    }

    AK_temp_create_table(new_table, header, SEGMENT_TYPE_TABLE);
    AK_free(temp_block);
    AK_EPI;
}

/**
 * @author Matija Novak, updated by Dino Laktašić
 * @brief  Function that searches the second block and when found matches with the first one makes a join and writes a row to join the tables
 * @param row_root - list of values from the first table to be marged with table2
 * @param row_root_insert - list of values from the first table to be inserted into nat_join table
 * @param temp_block - block from the second table to be merged
 * @param new_table - name of the nat_join table
 * @return No return value
 */
void AK_merge_block_join(struct list_node *row_root, struct list_node *row_root_insert, AK_block *temp_block, char *new_table) {
//void AK_merge_block_join(AK_list *row_root, AK_list *row_root_insert, AK_block *temp_block, char *new_table) {
    AK_PRO;
    AK_dbg_messg(HIGH, REL_OP, "\n MERGE NAT JOIN...");

    struct list_node *some_element;

    int i; //counter of tuple_dicts
    int head; //counter of the headers
    int something_to_copy = 1; //boolean variable to indicate if there is data to copy in these set of tuple_dicts
    int size;
    int overflow;
    int not_in_list = 1;

    char data[MAX_VARCHAR_LENGTH];

    //going through tuple_dicts of the second table
    for (i = 0; i < DATA_BLOCK_SIZE;) {
        head = 0;
        something_to_copy = 1;

        //make a copy of insert row list of the first table
        //some_element = (AK_list_elem) AK_First_L2(row_root_insert);
	some_element = AK_First_L2(row_root_insert);

        //going through headers of the second table
        while (strcmp(temp_block->header[head].att_name, "") != 0) {
            //going through list of elements to compare
            //some_element = (AK_list_elem) AK_First_L2(row_root);
	    some_element = AK_First_L2(row_root);
	    
            while (some_element != NULL) {
                size = temp_block->tuple_dict[i].size;
                overflow = size + temp_block->tuple_dict[i].address;

                //if isn't element in the list, and if data is correct, and size is not null
                if ((strcmp(some_element->attribute_name, temp_block->header[head].att_name) == 0) && (size != 0)
                        && (overflow < (temp_block->AK_free_space + 1)) && (overflow > -1)) {
                    
                    memset(data, '\0', MAX_VARCHAR_LENGTH);
                    memcpy(data, temp_block->data + temp_block->tuple_dict[i].address, temp_block->tuple_dict[i].size);

                    //if merge data is not equal
                    if (strcmp(some_element->data, data) != 0) {
                        //dont copy these set of tuple_dicts
                        something_to_copy = 0;
			break;
                    }
                    not_in_list = 0;
                } else if ((size == 0) || (overflow > (temp_block->AK_free_space + 1)) || (overflow < -1)) {
                    something_to_copy = 0;
                }
                //if there are more elements to compare for merge
                some_element = some_element->next;
            }

            //if it isn't header which is in compare list then I can copy these tuple dicts becouse it not exist in the insert_row_list2
            if ((not_in_list == 1) && (size != 0) && (overflow < temp_block->AK_free_space + 1) && (overflow > -1)) {
                memset(data, '\0', MAX_VARCHAR_LENGTH);
                //data[MAX_VARCHAR_LENGHT] = '\0';
                memcpy(data, temp_block->data + temp_block->tuple_dict[i].address, temp_block->tuple_dict[i].size);

                //insert data from second table to insert_list
                //AK_Insert_New_Element(temp_block->tuple_dict[i].type, data, new_table, temp_block->header[head].att_name, &row_root_insert);
                AK_Insert_New_Element(temp_block->tuple_dict[i].type, data, new_table, temp_block->header[head].att_name, row_root_insert);

            }
            not_in_list = 1;
            head++; //next header
            i++; //next tuple dict
        ;}

        //if these set is one that is passes merge then insert the list of data to join table
        if (something_to_copy) {
            AK_insert_row(row_root_insert);
        }
    }
    AK_EPI;
}

/**
 * @author Matija Novak, optimized, and updated to work with AK_list by Dino Laktašić
 * @brief Function that iterates through block of the first table and copies data that needs for join, then it calls a merge function to
        merge with the second table
 * @param tbl1_temp_block block of the first table
 * @param tbl2_temp_block block of the second join table
 * @param att attributes on which we make nat_join
 * @param new_table name of the nat_join table
 * @return No return value
 */
void AK_copy_blocks_join(AK_block *tbl1_temp_block, AK_block *tbl2_temp_block, struct list_node *att, char *new_table) {
    AK_PRO;
    AK_dbg_messg(HIGH, REL_OP, "\n COPYING NAT JOIN");
    
    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof(struct list_node));
    struct list_node *row_root_insert = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row_root);
    AK_Init_L3(&row_root_insert);
    struct list_node *list_elem;
    
    int i;
    int head;
    int something_to_copy = 0;
    int size;
    int overflow;
    int not_in_list = 1;
    char data[MAX_VARCHAR_LENGTH];

    //going through tuple_dict of the table block
    for (i = 0; i < DATA_BLOCK_SIZE;) {
        head = something_to_copy = 0;

        while (strcmp(tbl1_temp_block->header[head].att_name, "") != 0) {
            
	  list_elem = AK_First_L2(att);
            //going through list of elements on which we merge
            while (list_elem != NULL) {
                
                size = tbl1_temp_block->tuple_dict[i].size;
                overflow = size + tbl1_temp_block->tuple_dict[i].address;

                //if there is an element that we need, and it's correct we copy it
                if ((strcmp(list_elem->data, tbl1_temp_block->header[head].att_name) == 0) && (size != 0)
                        && (overflow < (tbl1_temp_block->AK_free_space + 1)) && (overflow > -1)) {
                    memset(data, '\0', MAX_VARCHAR_LENGTH);

                    memcpy(data, tbl1_temp_block->data + tbl1_temp_block->tuple_dict[i].address, tbl1_temp_block->tuple_dict[i].size);
                    //insert element into list on which we compare
                    AK_Insert_New_Element(tbl1_temp_block->tuple_dict[i].type, data, new_table, list_elem->data, row_root);
                    //insert element into list which we insert into join_table together with second table data
                    AK_Insert_New_Element(tbl1_temp_block->tuple_dict[i].type, data, new_table, list_elem->data, row_root_insert);
                    

                    something_to_copy = 1;
                    not_in_list = 0;
                    
                    break; //check for break here, to speed up execution  
                }
                list_elem = list_elem->next;
            }

            //copy element which is not for merge only for insert
            if ((not_in_list == 1) && (size != 0) && (overflow < tbl1_temp_block->AK_free_space + 1) && (overflow > -1)) {
                    memset(data, '\0', MAX_VARCHAR_LENGTH);
                    //data[MAX_VARCHAR_LENGHT] = '\0';
                    memcpy(data, tbl1_temp_block->data + tbl1_temp_block->tuple_dict[i].address, tbl1_temp_block->tuple_dict[i].size);
                    AK_Insert_New_Element(tbl1_temp_block->tuple_dict[i].type, data, new_table, tbl1_temp_block->header[head].att_name, row_root_insert);

            }
            not_in_list = 1; //reset not_in_list
            head++; //next header
            i++; //next tuple dict
        }

        //if there is a data to copy
        if (something_to_copy) {
            //merge data with second table
            AK_merge_block_join(row_root, row_root_insert, tbl2_temp_block, new_table);
            
            
	    AK_DeleteAll_L3(&row_root);
	    AK_DeleteAll_L3(&row_root_insert);
        }
    }
    AK_free(row_root);
    AK_free(row_root_insert);
    AK_EPI;
}

/**
 * @author Matija Novak, updated to work with AK_list and support cacheing by Dino Laktašić
 * @brief Function that makes a  nat_join betwen two tables on some attributes
 * @param srcTable1 name of the first table to join
 * @param srcTable2 name of the second table to join
 * @param att attributes on which we make nat_join
 * @param dstTable name of the nat_join table
 * @return if success returns EXIT_SUCCESS
 */
int AK_join(char *srcTable1, char * srcTable2, char * dstTable, struct list_node *att) {

    AK_PRO;
    table_addresses *src_addr1 = (table_addresses *) AK_get_table_addresses(srcTable1);
    table_addresses *src_addr2 = (table_addresses *) AK_get_table_addresses(srcTable2);

    int startAddress1 = src_addr1->address_from[0];
    int startAddress2 = src_addr2->address_from[0];

    if ((startAddress1 != 0) && (startAddress2 != 0)) {
        AK_create_join_block_header(startAddress1, startAddress2, dstTable, att);

        AK_dbg_messg(LOW, REL_OP, "\nTABLE %s CREATED from %s and %s\n", dstTable, srcTable1, srcTable2);
		AK_dbg_messg(MIDDLE, REL_OP, "\nAK_join: start copying data\n");

        AK_mem_block *tbl1_temp_block, *tbl2_temp_block;

        int i, j, k, l;
        i = j = k = l = 0;


        //for each extent in table1 that contains blocks needed for join
        for (i = 0; i < (src_addr1->address_from[i] != 0); i++) {
            startAddress1 = src_addr1->address_from[i];

            if (startAddress1 != 0) {
                AK_dbg_messg(MIDDLE, REL_OP, "\nNatural join: copy extent1: %d\n", i);

                //for each block in table1 extent
                for (j = startAddress1; j < src_addr1->address_to[i]; j++) {
                    AK_dbg_messg(MIDDLE, REL_OP, "Natural join: copy block1: %d\n", j);

                    tbl1_temp_block = (AK_mem_block *) AK_get_block(j);


                    //if there is data in the block
                    if (tbl1_temp_block->block->AK_free_space != 0) {
                        //for each extent in table2 that contains blocks needed for join
                        for (k = 0; k < (src_addr2->address_from[k] != 0); k++) {
                            startAddress2 = src_addr2->address_from[k];


                            if (startAddress2 != 0) {
                                AK_dbg_messg(MIDDLE, REL_OP, "Natural join: copy extent2: %d\n", k);

                                //for each block in table2 extent
                                for (l = startAddress2; l < src_addr2->address_to[k]; l++) {
                                    AK_dbg_messg(MIDDLE, REL_OP, "Natural join: copy block2: %d\n", l);

                                    tbl2_temp_block = (AK_mem_block *) AK_get_block(l);

                                    //if there is data in the block
                                    if (tbl2_temp_block->block->AK_free_space != 0) {
                                        AK_copy_blocks_join(tbl1_temp_block->block, tbl2_temp_block->block, att, dstTable);
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
		AK_dbg_messg(LOW, REL_OP, "NAT_JOIN_TEST_SUCCESS\n\n");
        AK_EPI;
        return EXIT_SUCCESS;
    } else {
        AK_dbg_messg(LOW, REL_OP, "\n AK_join: Table/s doesn't exist!");
        AK_free(src_addr1);
        AK_free(src_addr2);
        AK_EPI;
        return EXIT_ERROR;
    }
    AK_EPI;
    
}
/**
 * @author Matija Novak
 * @brief Function for natural join testing
 * @return No return value
 */
TestResult AK_op_join_test() {

    AK_PRO;
    char *sys_table = "AK_relation";
    char *destTable = "nat_join_test";
    char *tblName2 = "employee";
    char *tblName1 = "department";
    	int test;

    printf("\n********** NAT JOIN TEST **********\n\n");

    if (AK_if_exist(destTable, sys_table) == 0) {
    	printf("Table %s does not exist!\n", destTable);
	struct list_node *att = (struct list_node *) AK_malloc(sizeof(struct list_node));
    	AK_Init_L3(&att);
    	AK_InsertAtBegin_L3(TYPE_ATTRIBS, "id_department", sizeof ("id_department"), att);

    	test = AK_join(tblName1, tblName2, destTable, att);
	    AK_DeleteAll_L3(&att);
    }

    else {
	printf("Table %s already exists!\n", destTable);
    AK_print_table(destTable);
    return TEST_result(1, 0);
    }

    AK_EPI;
    if (test == EXIT_SUCCESS)
            {
                AK_print_table("employee");
                AK_print_table("department");
                AK_print_table(destTable);
                return TEST_result(1, 0);
            }
            else
            {
                AK_print_table("product_test");
                return TEST_result(0, 1);
            }

    }

