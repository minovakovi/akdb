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


#include <string.h>  
/* Provides:
   - memcpy() for copying header and tuple data  
   - strcmp() for key comparisons  
   - strdup() for duplicating string keys  
*/

#include <stdlib.h>  
/* Provides:
   - malloc()/free() for dynamic memory  
   - EXIT_SUCCESS/EXIT_ERROR constants  
   - strdup() fallback if not in <string.h>  
   - unlink()/access() on POSIX systems  
*/

//array to keep track of each row’s “key” (first column value) 
//used for duplicate detection during the union:
#define MAX_UNION_KEYS 1024  
static char *union_keys[MAX_UNION_KEYS];
static int union_key_count;

/**
 * @author Tea Radić 
 * @brief  Perform union across multiple tables, handling duplicates and schema checks.
 * @param  count      Number of source tables
 * @param  srcTables  Array of source table names
 * @param  dstTable   Destination table name
 * @return EXIT_SUCCESS on success, EXIT_ERROR otherwise
 */
int AK_union_multiple(int count, char *srcTables[], char *dstTable) {
    AK_PRO;
    if (count < 1) {
        AK_dbg_messg(LOW, REL_OP, "AK_union_multiple: nema tablica za uniju\n");
        AK_EPI;
        return EXIT_ERROR;
    }
    union_key_count = 0;

    table_addresses **addrs = malloc(count * sizeof *addrs);
    for (int i = 0; i < count; i++) {
        addrs[i] = (table_addresses*) AK_get_table_addresses(srcTables[i]);
        if (!addrs[i] || addrs[i]->address_from[0] == 0) {
            AK_dbg_messg(LOW, REL_OP, "AK_union_multiple: tablica %s ne postoji\n", srcTables[i]);
            for (int j = 0; j <= i; j++) if (addrs[j]) AK_free(addrs[j]);
            free(addrs);
            AK_EPI;
            return EXIT_ERROR;
        }
    }

    AK_mem_block *first_block = AK_get_block(addrs[0]->address_from[0]);
    int num_att = num_att = AK_check_tables_scheme(first_block, first_block, "Union");

    if (num_att == EXIT_ERROR) {
        for (int i = 0; i < count; i++) AK_free(addrs[i]);
        free(addrs);
        AK_EPI;
        return EXIT_ERROR;
    }
    for (int i = 1; i < count; i++) {
        AK_mem_block *blk = AK_get_block(addrs[i]->address_from[0]);
        if (AK_check_tables_scheme(first_block, blk, "Union") == EXIT_ERROR) {
            AK_dbg_messg(LOW, REL_OP, "AK_union_multiple: mismatch shema %s\n", srcTables[i]);
            for (int j = 0; j < count; j++) AK_free(addrs[j]);
            free(addrs);
            AK_EPI;
            return EXIT_ERROR;
        }
    }

    AK_header *hdr = AK_malloc(num_att * sizeof *hdr);
    memcpy(hdr, first_block->block->header, num_att * sizeof *hdr);
    AK_initialize_new_segment(dstTable, SEGMENT_TYPE_TABLE, hdr);
    AK_free(hdr);

    struct list_node *row_root = AK_malloc(sizeof *row_root);
    memset(row_root, 0, sizeof *row_root);

    for (int i = 0; i < count; i++) {
        int start = addrs[i]->address_from[0];
        AK_Write_Segments(dstTable, num_att, addrs[i], start, AK_get_block(start), row_root);
    }

    for (int i = 0; i < count; i++) AK_free(addrs[i]);
    free(addrs);
    AK_free(row_root);

    AK_dbg_messg(LOW, REL_OP, "UNION_MULTIPLE_SUCCESS\n");
    AK_EPI;
    return EXIT_SUCCESS;
}



/**
 * @author Dino Laktašić; updated by Elena Kržina
 * @brief  Function that makes a union of two tables. Union is implemented for working with multiple sets of data, i.e. duplicate tuples can be written in same table (union) 
 * @param srcTable1 name of the first table
 * @param srcTable2 name of the second table
 * @param dstTable name of the new table
 * @return if success returns EXIT_SUCCESS, else returns EXIT_ERROR
 */
 /*int AK_union(char *srcTable1, char *srcTable2, char *dstTable) {
    AK_PRO;
    //reset the key counter before processing    
    union_key_count = 0; 

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
}*/


//Wrapper for backward compatibility: delegates two-table union to the new AK_union_multiple function
int AK_union(char *srcTable1, char *srcTable2, char *dstTable) {
    AK_PRO;
    char *srcs[2] = { srcTable1, srcTable2 };
    int rc = AK_union_multiple(2, srcs, dstTable);
    AK_EPI;
    return rc;
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

                    //Removed: original row-insertion code lacked any duplicate check
                    /*if ((k + 1) % num_att == 0 && k != 0) {
                        AK_insert_row(row_root);
                        AK_DeleteAll_L3(&row_root);
                    }*/
                   /*once a full tuple is buffered,
                     compare its key against stored keys;
                     insert only if the key is not yet recorded*/
                    if ((k + 1) % num_att == 0 && k != 0) {
                        struct list_node *first_elem = row_root->next;
                        char *new_key = first_elem ? first_elem->data : NULL;
                        
                        int exists = 0;
                        if (union_key_count > 0) {
                            for (int u = 0; u < union_key_count; u++) {
                                if (strcmp(union_keys[u], new_key) == 0) {
                                    exists = 1;
                                    break;
                                }
                            }
                        }
                        if (!exists) {
                        AK_insert_row(row_root);
                            if (union_key_count < MAX_UNION_KEYS && new_key) {
                                union_keys[union_key_count++] = strdup(new_key);
                            }
                        }
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

    int passed = 0;
    int failed = 0;

    AK_create_test_table_schema_mismatch();
    
    printf("\n********** UNION TEST **********\n\n");
    /*AK_print_table("professor");
    AK_print_table("assistant");
    int test = AK_union("professor", "assistant"    , "union_test");
    AK_print_table("union_test");*/

    if (AK_union("professor", "assistant", "union_test") == EXIT_SUCCESS) {
        AK_print_table("union_test");
        printf("Basic union test succeeded!\n\n");
        passed++;
    } else {
        printf("Basic union test failed!\n\n");
        failed++;
    }

    //Schema mismatch test: ensure union fails when input schemas differ
    printf("\n********** SCHEMA MISMATCH TEST **********\n\n");
    if (AK_union("professor", "UT_professor_mismatch", "union_mismatch_test") != EXIT_SUCCESS) {
        printf("Unija s različitim shemama je ispravno odbijena.\n\n");
        passed++;
    } else {
        printf("Unija s različitim shemama je neočekivano uspjela.\n\n");
        failed++;
    }

    AK_create_test_table_empty(); 

    //Empty-table tests: ensure union returns all rows from the non-empty table
    printf("\n********** UNION WITH EMPTY TABLE **********\n\n");
    if (AK_union("UT_empty", "assistant", "union_empty1") == EXIT_SUCCESS) {
        printf("OK: union(empty, assistant) uspješan.\n");
        passed++;
    } else {
        printf("ERROR: union(empty, assistant) nije uspio.\n");
        failed++;
    }

    if (AK_union("professor", "UT_empty", "union_empty2") == EXIT_SUCCESS) {
        printf("OK: union(professor, empty) uspješan.\n\n");
        passed++;
    } else {
        printf("ERROR: union(professor, empty) nije uspio.\n\n");
        failed++;
    }

    AK_print_table("union_empty1");
    AK_print_table("union_empty2");

    AK_create_test_table_union3();

    //Multiple-table union test: verify that AK_union_multiple merges three tables correctly without duplicates
    printf("\n********** MULTIPLE UNION TEST **********\n\n");
    {
        char *triple[] = { "professor", "assistant", "UT_union3" };
        if (AK_union_multiple(3, triple, "union_multiple_test") == EXIT_SUCCESS) {
            AK_print_table("union_multiple_test");
            printf("Multiple union test succeeded!\n\n");
            passed++;
        } else {
            printf("Multiple union test failed!\n\n");
            failed++;
        }
    }    
	
	AK_EPI;
	/*t == EXIT_SUCCESS){
		return TEST_result(1,0);
    }
    else{
		return TEST_result(0,1);
    }*/
   
    return TEST_result(passed, failed);
}