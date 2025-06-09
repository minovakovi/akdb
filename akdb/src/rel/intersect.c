/**
@file intersect.c Provides functions for relational intersect operation
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


#include "intersect.h"

/**
 * @author Dino Laktašić; updated by Elena Kržina
 * @brief  Function that makes an intersect of two tables. Intersect is implemented for working with multiple sets of data, i.e. duplicate 
          tuples can be written in same table (intersect)
 * @param srcTable1 name of the first table
 * @param srcTable2 name of the second table
 * @param dstTable name of the new table
 * @return if success returns EXIT_SUCCESS, else returns EXIT_ERROR
 */

int AK_intersect(char *srcTable1, char *srcTable2, char *dstTable) {
    AK_PRO;
    table_addresses *src_addr1 = (table_addresses*) AK_get_table_addresses(srcTable1);
    table_addresses *src_addr2 = (table_addresses*) AK_get_table_addresses(srcTable2);

    int startAddress1 = src_addr1->address_from[0];
    int startAddress2 = src_addr2->address_from[0];

    if ((startAddress1 != 0) && (startAddress2 != 0)) 
	{
		//register int used for faster processing
        register int extend1, blockExtend1, extend2, blockExtend2;
        extend1 = blockExtend1 = extend2 = blockExtend2 = 0;

        AK_mem_block *tbl1_temp_block = (AK_mem_block *) AK_get_block(startAddress1);
        AK_mem_block *tbl2_temp_block = (AK_mem_block *) AK_get_block(startAddress2);
        
        int num_att = AK_check_tables_scheme(tbl1_temp_block, tbl2_temp_block, "Intersect");

        if (num_att == EXIT_ERROR) {

			AK_free(src_addr1);
       		AK_free(src_addr2);
			
			AK_EPI;
			return EXIT_ERROR;
		}

        int touple1, touple2, column;
		int address, type, size,thesame;
		thesame = 0;
		
        char data1[MAX_VARCHAR_LENGTH];
        char data2[MAX_VARCHAR_LENGTH];

        //initialize new segment
        AK_header *header = (AK_header *) AK_malloc(num_att * sizeof (AK_header));
        memcpy(header, tbl1_temp_block->block->header, num_att * sizeof (AK_header));
        AK_initialize_new_segment(dstTable, SEGMENT_TYPE_TABLE, header);
        AK_free(header);

		struct list_node *row_root = (struct list_node * ) AK_malloc(sizeof(struct list_node));
        AK_Init_L3(&row_root);

        //TABLE1: for each extent in table1
        for (extend1 = 0; src_addr1->address_from[extend1] != 0; extend1++) 
		{
            startAddress1 = src_addr1->address_from[extend1];

                //BLOCK: for each block in table1 extent
                for (blockExtend1 = startAddress1; blockExtend1 < src_addr1->address_to[extend1]; blockExtend1++) 
				{
                    tbl1_temp_block = (AK_mem_block *) AK_get_block(blockExtend1);

                    //if there is data in the block
                    if (tbl1_temp_block->block->AK_free_space != 0) 
					{

                        //TABLE2: for each extent in table2
                        for (extend2 = 0; extend2 < (src_addr2->address_from[extend2] != 0); extend2++) 
						{
                            startAddress2 = src_addr2->address_from[extend2];

                            if (startAddress2 != 0) 
							{
                                //BLOCK: for each block in table2 extent
                                for (blockExtend2 = startAddress2; blockExtend2 < src_addr2->address_to[extend2]; blockExtend2++) 
								{
                                    tbl2_temp_block = (AK_mem_block *) AK_get_block(blockExtend2);

                                    //if there is data in the block
                                    if (tbl2_temp_block->block->AK_free_space != 0) 
									{
                                        //TUPLE_DICTS: for each tuple_dict in the block
                                        for (touple1 = 0; touple1 < DATA_BLOCK_SIZE; touple1 += num_att) 
										{
                                            if (tbl1_temp_block->block->tuple_dict[touple1 + 1].type == FREE_INT)
                                                break;

                                            for (touple2 = 0; touple2 < DATA_BLOCK_SIZE; touple2 += num_att) 
											{
                                                if (tbl2_temp_block->block->tuple_dict[touple2 + 1].type == FREE_INT)
                                                    break;

                                                for (column = 0; column < num_att; column++) 
												{
                                                    type = tbl1_temp_block->block->tuple_dict[touple1 + column].type;
                                                    size = tbl1_temp_block->block->tuple_dict[touple1 + column].size;
                                                    address = tbl1_temp_block->block->tuple_dict[touple1 + column].address;
													
                                                    memcpy(data1, &(tbl1_temp_block->block->data[address]), size);
                                                    data1[size] = '\0';

                                                    type = tbl2_temp_block->block->tuple_dict[touple2 + column].type;
                                                    size = tbl2_temp_block->block->tuple_dict[touple2 + column].size;
                                                    address = tbl2_temp_block->block->tuple_dict[touple2 + column].address;
													
                                                    memcpy(data2, &(tbl2_temp_block->block->data[address]), size);
                                                    data2[size] = '\0';

                                                    //if two attributes are different, stop searching that row
													if(strcmp(data1,data2)!=0)break;
													else if(strcmp(data1, data2) == 0 && column ==(num_att-1)) thesame=1;
                                                }

                                                if (thesame == 1) 
												{
                                                    for (column = 0; column < num_att; column++) 
													{
                                                        type = tbl1_temp_block->block->tuple_dict[touple1 + column].type;
                                                        size = tbl1_temp_block->block->tuple_dict[touple1 + column].size;
                                                        address = tbl1_temp_block->block->tuple_dict[touple1 + column].address;
														
                                                        memcpy(data1, &(tbl1_temp_block->block->data[address]), size);
                                                        data1[size] = '\0';
														
                                                        AK_Insert_New_Element(type, data1, dstTable, tbl1_temp_block->block->header[column].att_name, row_root);
                                                    }
                                                    AK_insert_row(row_root);

                                                    AK_DeleteAll_L3(&row_root);
                                                    thesame=0;
                                                }
                                            }
                                        }
                                    }
                                }
                            } else break;
                        }
                    }
                }
        }

        AK_free(src_addr1);
        AK_free(src_addr2);
		AK_free(row_root);
		
		AK_dbg_messg(LOW, REL_OP, "INTERSECT_TEST_SUCCESS\n\n");
	
		AK_EPI;
		return EXIT_SUCCESS;
		} 

	else 
	{
        AK_dbg_messg(LOW, REL_OP, "\nAK_intersect: Table/s doesn't exist!");
        AK_free(src_addr1);
        AK_free(src_addr2);
		
		AK_EPI;
		return EXIT_ERROR;
    }
    AK_EPI;
}

/**
 * @author Dino Laktašić
 * @brief  Function for intersect operator testing
 * @return No return value
 */
/*TestResult AK_op_intersect_test() {
    AK_PRO;
    char *sys_table = "AK_relation";
    char *destTable = "intersect_test";
    char *tblName1 = "professor";
    char *tblName2 = "assistant";
	int test_intersect;
    
    printf("\n********** INTERSECT TEST **********\n\n");

    if (AK_if_exist(destTable, sys_table) == 0) {
    	printf("Table %s does not exist!\n", destTable);
		test_intersect = AK_intersect(tblName1, tblName2, destTable);
    }
    else {
	printf("Table %s already exists!\n", destTable);
	test_intersect = EXIT_SUCCESS;
    }

    AK_print_table(destTable);
	
    int success=0;
    int failed=0;
    if (test_intersect==EXIT_SUCCESS){
	    printf("\nTest succeeded!\n");
        success++;
    }
    else{
	    printf("\nTest failed!\n");
        failed++;
    }
    
    AK_EPI;
    return TEST_result(success,failed);
}*/


/**
 * @author Tea Radić
 * @brief  Runs intersect operator tests, including basic, schema‐mismatch and empty‐table scenarios
 */
TestResult AK_op_intersect_test() {
    AK_PRO;

    int passed = 0, failed = 0;

    AK_create_test_table_schema_mismatch();  
    AK_create_test_table_empty();           

    printf("\n********** INTERSECT TEST **********\n\n");

    if (AK_intersect("professor", "assistant", "intersect_test") == EXIT_SUCCESS) {
        AK_print_table("intersect_test");
        printf("Basic intersect test succeeded!\n\n");
        passed++;
    } else {
        printf("Basic intersect test failed!\n\n");
        failed++;
    }

    printf("\n********** SCHEMA MISMATCH TEST **********\n\n");
    if (AK_intersect("professor", "UT_mismatch", "intersect_mismatch_test") != EXIT_SUCCESS) {
        printf("OK: intersect s različitim shemama je ispravno odbijen.\n\n");
        passed++;
    } else {
        printf("ERROR: intersect s različitim shemama je neočekivano uspio.\n\n");
        failed++;
    }

    printf("\n********** INTERSECT WITH EMPTY TABLE **********\n\n");
    int rc1 = AK_intersect("UT_empty",   "assistant",      "intersect_empty1");
    int rc2 = AK_intersect("professor",  "UT_empty",       "intersect_empty2");

    if (rc1 == EXIT_SUCCESS) {
        printf("OK: intersect(empty, assistant) uspješan (treba biti prazan rezultat).\n");
        passed++;
    } else {
        printf("ERROR: intersect(empty, assistant) nije uspio.\n");
        failed++;
    }
    if (rc2 == EXIT_SUCCESS) {
        printf("OK: intersect(professor, empty) uspješan (treba biti prazan rezultat).\n\n");
        passed++;
    } else {
        printf("ERROR: intersect(professor, empty) nije uspio.\n\n");
        failed++;
    }

    AK_print_table("intersect_empty1");  
    AK_print_table("intersect_empty2");  

    AK_EPI;
    return TEST_result(passed, failed);
}



