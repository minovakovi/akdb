/*
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/**
 @file union.c Provides functions for relational union operation
*/

#include "union.h"

/**
 * @author Dino Laktašić; updated by Elena Kržina
 * @brief  Function that makes a union of two tables. Union is implemented for working with multiple sets of data, i.e. duplicate tuples can be written in same table (union) 
 * @param srcTable1 name of the first table
 * @param srcTable2 name of the second table
 * @param dstTable name of the new table
 * @return if success returns EXIT_SUCCESS, else returns EXIT_ERROR
 */
 int AK_union(char *srcTable1, char *srcTable2, char *dstTable) {
    AK_PRO;
    table_addresses *src_addr1 = (table_addresses*) AK_get_table_addresses(srcTable1);
    table_addresses *src_addr2 = (table_addresses*) AK_get_table_addresses(srcTable2);
    
    int startAddress1 = src_addr1->address_from[0];
    int startAddress2 = src_addr2->address_from[0];
    
    if ((startAddress1 != 0) && (startAddress2 != 0)) {

        AK_mem_block *tbl1_temp_block = AK_get_block(startAddress1);
        AK_mem_block *tbl2_temp_block = AK_get_block(startAddress2);

        int num_att = AK_check_tables_scheme(tbl1_temp_block, tbl2_temp_block, "Union");

        if (num_att == EXIT_ERROR) {
            AK_EPI;              
            return EXIT_ERROR;
        }

        //initialize new segment
        AK_header *header = AK_malloc(num_att * sizeof(AK_header));
        memcpy(header, tbl1_temp_block->block->header, num_att * sizeof (AK_header));
        AK_initialize_new_segment(dstTable, SEGMENT_TYPE_TABLE, header);
        AK_free(header);

        struct list_node *row_root = AK_malloc(sizeof(struct list_node));
        memset(row_root, 0, sizeof(struct list_node));

        AK_Write_Segments(dstTable, num_att, src_addr1, startAddress1, tbl1_temp_block, row_root);
        AK_Write_Segments(dstTable, num_att, src_addr2, startAddress2, tbl2_temp_block,row_root);

        AK_free(src_addr1);
        AK_free(src_addr2);
        AK_free(row_root);

        AK_dbg_messg(LOW, REL_OP, "UNION_TEST_SUCCESS\n\n");
        AK_EPI;
        return EXIT_SUCCESS;
    } 
	else {
        AK_dbg_messg(LOW, REL_OP, "\nAK_union: Table/s doesn't exist!");
        AK_free(src_addr1);
        AK_free(src_addr2);
        AK_EPI;
        return EXIT_ERROR;
    }
AK_EPI;
}

/**
 * @author Dino Laktašić edited by Elena Kržina
 * @brief  Auxiliary function for writing blocks or tables into new segment, made by Dino Laktašić originally and separated and 
 * edited by Elena Kržina for code transparency
 * @param dstTable destination table of function
 * @param num_att number of attributes of table
 * @param src_addr1 source address
 * @param startAddress1 starting address
 * @param tbl1_temp_block table block that is accessed
 * @param row_root root of linked list
 * @return void
 */
void AK_Write_Segments(char *dstTable, int num_att, table_addresses *src_addr1, int startAddress1, AK_mem_block *tbl1_temp_block, struct list_node *row_root){
    AK_PRO;
    //registers used for faster working speed of variables
    register int i, j, k;
    i = j = k = 0;

    int address, type, size;
    char data[MAX_VARCHAR_LENGTH];

    for (i = 0; src_addr1->address_from[i] != 0; i++) {
        startAddress1 = src_addr1->address_from[i];

        //BLOCK: for each block in table1 extent
        for (j = startAddress1; j < src_addr1->address_to[i]; j++) {
            tbl1_temp_block = AK_get_block(j); //read block from first table

            //if there is data in the block
            if (tbl1_temp_block->block->AK_free_space != 0) {

                for (k = 0; k < DATA_BLOCK_SIZE; k++) {
                    if (tbl1_temp_block->block->tuple_dict[k].type == FREE_INT)
                        break;

                    address = tbl1_temp_block->block->tuple_dict[k].address;
                    size = tbl1_temp_block->block->tuple_dict[k].size;
                    type = tbl1_temp_block->block->tuple_dict[k].type;

                    memset(data, '\0', MAX_VARCHAR_LENGTH);
                    memcpy(data, tbl1_temp_block->block->data + address, size);

                    AK_Insert_New_Element(type, data, dstTable, tbl1_temp_block->block->header[k % num_att].att_name, row_root);

                    if ((k + 1) % num_att == 0 && k != 0) {
                        AK_insert_row(row_root);
                        AK_DeleteAll_L3(&row_root);
                    }
                }
            }
        }
    }
    AK_EPI;
}

/**
 * @author Dino Laktašić
 * @brief  Function for union operator testing
 * @return No return value
 * 
 */
TestResult AK_op_union_test() {
    AK_PRO;
    printf("\n********** UNION TEST **********\n\n");
    AK_print_table("professor");
    AK_print_table("assistant");
    int test = AK_union("professor", "assistant", "union_test");
    AK_print_table("union_test");
	
	AK_EPI;
	if (test == EXIT_SUCCESS){
		return TEST_result(1,0);
    }
    else{
		return TEST_result(0,1);
    }	
}