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
 @file intersect.c Provides functions for relational intersect operation
*/

#include "intersect.h"

/**
 * @author Dino Laktašić; updated by Elena Kržina
 * @brief  Function that makes an intersect of two tables. Intersect is implemented for working with multiple sets of data, i.e. duplicate tuples can be written in same table (intersect)
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
    
    if ((startAddress1 != 0) && (startAddress2 != 0)) {
        AK_mem_block *tbl1_temp_block = AK_get_block(startAddress1);
        AK_mem_block *tbl2_temp_block = AK_get_block(startAddress2);

        int num_att = AK_check_tables_scheme(tbl1_temp_block, tbl2_temp_block, "Intersect");

        if (num_att == EXIT_ERROR) {
            AK_EPI;
            return EXIT_ERROR;
        }

        //initialize new segment
        AK_header *header = AK_malloc(num_att * sizeof(AK_header));
        memcpy(header, tbl1_temp_block->block->header, num_att * sizeof(AK_header));
        AK_initialize_new_segment(dstTable, SEGMENT_TYPE_TABLE, header);
        AK_free(header);

        struct list_node *row_root = AK_malloc(sizeof(struct list_node));
        memset(row_root, 0, sizeof(struct list_node));

        AK_Write_Intersect_Segments(dstTable, num_att, src_addr1, src_addr2, row_root);

        AK_free(src_addr1);
        AK_free(src_addr2);
        AK_free(row_root);

        AK_dbg_messg(LOW, REL_OP, "INTERSECT_TEST_SUCCESS\n\n");
        AK_EPI;
        return EXIT_SUCCESS;
    } 
    else {
        AK_dbg_messg(LOW, REL_OP, "\nAK_intersect: Table/s doesn't exist!");
        AK_free(src_addr1);
        AK_free(src_addr2);
        AK_EPI;
        return EXIT_ERROR;
    }
    AK_EPI;
}

/**
 * @brief  Auxiliary function for writing intersect blocks or tables into new segment
 * @param dstTable destination table of function
 * @param num_att number of attributes of table
 * @param src_addr1 source address 1
 * @param src_addr2 source address 2
 * @param row_root root of linked list
 * @return void
 */
void AK_Write_Intersect_Segments(char *dstTable, int num_att, table_addresses *src_addr1, table_addresses *src_addr2, struct list_node *row_root){
    AK_PRO;
    register int i, j, k, l;
    i = j = k = l = 0;

    int address1, address2, type1, type2, size1, size2, thesame;
    thesame = 0;
    char data1[MAX_VARCHAR_LENGTH];
    char data2[MAX_VARCHAR_LENGTH];

    for (i = 0; src_addr1->address_from[i] != 0; i++) {
        for (j = src_addr1->address_from[i]; j < src_addr1->address_to[i]; j++) {
            AK_mem_block *tbl1_temp_block = AK_get_block(j);

            if (tbl1_temp_block->block->AK_free_space != 0) {
                for (k = 0; src_addr2->address_from[k] != 0; k++) {
                    for (l = src_addr2->address_from[k]; l < src_addr2->address_to[k]; l++) {
                        AK_mem_block *tbl2_temp_block = AK_get_block(l);

                        if (tbl2_temp_block->block->AK_free_space != 0) {
                            for (int m = 0; m < DATA_BLOCK_SIZE; m += num_att) {
                                if (tbl1_temp_block->block->tuple_dict[m + 1].type == FREE_INT)
                                    break;

                                for (int o = 0; o < DATA_BLOCK_SIZE; o += num_att) {
                                    if (tbl2_temp_block->block->tuple_dict[o + 1].type == FREE_INT)
                                        break;

                                    thesame = 1;
                                    for (int n = 0; n < num_att; n++) {
                                        type1 = tbl1_temp_block->block->tuple_dict[m + n].type;
                                        size1 = tbl1_temp_block->block->tuple_dict[m + n].size;
                                        address1 = tbl1_temp_block->block->tuple_dict[m + n].address;
                                        memcpy(data1, &(tbl1_temp_block->block->data[address1]), size1);
                                        data1[size1] = '\0';

                                        type2 = tbl2_temp_block->block->tuple_dict[o + n].type;
                                        size2 = tbl2_temp_block->block->tuple_dict[o + n].size;
                                        address2 = tbl2_temp_block->block->tuple_dict[o + n].address;
                                        memcpy(data2, &(tbl2_temp_block->block->data[address2]), size2);
                                        data2[size2] = '\0';

                                        if (strcmp(data1, data2) != 0) {
                                            thesame = 0;
                                            break;
                                        }
                                    }

                                    if (thesame) {
                                        for (int n = 0; n < num_att; n++) {
                                            type1 = tbl1_temp_block->block->tuple_dict[m + n].type;
                                            size1 = tbl1_temp_block->block->tuple_dict[m + n].size;
                                            address1 = tbl1_temp_block->block->tuple_dict[m + n].address;
                                            memcpy(data1, &(tbl1_temp_block->block->data[address1]), size1);
                                            data1[size1] = '\0';

                                            AK_Insert_New_Element(type1, data1, dstTable, tbl1_temp_block->block->header[n].att_name, row_root);
                                        }
                                        AK_insert_row(row_root);
                                        AK_DeleteAll_L3(&row_root);
                                        thesame = 0;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    AK_EPI;
}

/**
 * @author Dino Laktašić
 * @brief  Function for intersect operator testing
 * @return No return value
 */
TestResult AK_op_intersect_test() {
    AK_PRO;
    printf("\n********** INTERSECT TEST **********\n\n");
    AK_print_table("professor");
    AK_print_table("assistant");
    int test = AK_intersect("professor", "assistant", "intersect_test");
    AK_print_table("intersect_test");
    
    AK_EPI;
    if (test == EXIT_SUCCESS){
        return TEST_result(1,0);
    }
    else{
        return TEST_result(0,1);
    }   
}
