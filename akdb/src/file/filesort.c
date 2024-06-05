/**
@file filsort.c Provides functions for file sorting
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
#include "filesort.h"

/**
 * @author Unknown
 * @brief Function that returns the total number of headers in the block
 * @return number of attribute in header (0 - MAX_ATTRIBUTES). USE in tuple_dict[num]...
 */
int AK_get_total_headers(AK_block *iBlock) {
    register int i;
    AK_PRO;
    for (i = 0; i < MAX_ATTRIBUTES; i++) {
        if (strcmp((char *) iBlock->header[i].att_name, "") == 0){ //if there is no more attributes
		AK_EPI;
		return i;
	}
    }
    AK_EPI;
    return i;
}

/**
 * @author Unknown
 * @brief Function that returns the number of header in the block which to sort
 * @return number of attribute in header (0 - MAX_ATTRIBUTES). USE in tuple_dict[num]...
 */
int AK_get_header_number(AK_block *iBlock, char *attribute_name) {
    register int i;
    AK_PRO;
    for (i = 0; i < MAX_ATTRIBUTES; i++) {
        if (!strcmp((char *) iBlock->header[i].att_name, attribute_name)) {
	    AK_EPI;
            return i;
        } else if (!strcmp((char *) iBlock->header[i].att_name, "/0")) {
            printf("get_header_number: ERROR: in this table does not exist atribute: %s", attribute_name);
            AK_EPI;
            return EXIT_ERROR;
        }
    }
    printf("get_header_number: ERROR: in this table does not exist atribute: %s", attribute_name);
    AK_EPI;
    return EXIT_ERROR;
}

/**
 * @author Unknown
 * @brief Function that returns tuples number in block
 * @return tuples number in block
 */
int AK_get_num_of_tuples(AK_block *iBlock) {
    int i = 0;
    int kraj = 1;
    AK_PRO;
    while (kraj) {
        i++;
        if (iBlock->tuple_dict[ i ].size == 0) {
            kraj = 0;
        }
    }

    if (i == 0){
	AK_EPI;
        return 0;
    }

    int max_header_num = AK_get_total_headers(iBlock);
    AK_EPI;
    return (i / max_header_num);
}

/**
 * @author Tomislav Bobinac, updated by Filip Žmuk
 * @todo Make it to suport multiple sort atributes and ASC|DESC ordering
 * @brief Function that sorts a segment
 * @return No return value.
 */
int AK_sort_segment(char *srcTable, char *destTable, struct list_node* attributes) {
	//initialize variables
	register int i, j;
	int blocks_addr[MAX_NUM_OF_BLOCKS];	//array with block addresses
	int num_blocks = 0;	//number of blocks in segment

	AK_PRO;

	//Get number of blocks for given table
	table_addresses *addresses = (table_addresses *) AK_get_table_addresses(srcTable);
	for (i = 0; addresses->address_from[i] != 0; i++) {
		for (j = addresses->address_from[i]; j <= addresses->address_to[i]; j++) {
			blocks_addr[num_blocks] = j;
			num_blocks++;
		}
	}

	AK_header *head = AK_get_header(srcTable);
	AK_initialize_new_segment(destTable, SEGMENT_TYPE_TABLE, head);

	//SORTING ...
	char x[DATA_ROW_SIZE]; 		//needed data
	char y[DATA_ROW_SIZE]; 		//data needed for comparison
	char data[DATA_TUPLE_SIZE]; //data to be copied

	AK_mem_block * real_table = (AK_mem_block*) AK_get_block(blocks_addr[0]);

	//get total number of headers and the number of header used to sort segment
	int num_headers = AK_get_total_headers(real_table->block);
	int num_sort_header = AK_get_header_number(real_table->block, AK_First_L2(attributes)->data);
	int num_records = AK_get_num_records(srcTable);

	int type, temp, address, size, temp_field[num_records];
	for (i = 0; i<num_records; i++) {
		temp_field[i]=i;
	}

	for (i = 0; i < num_records - 1; i++) {
		for (j = i + 1; j < num_records; j++) {
			//first data -> x
			address = real_table->block->tuple_dict[(temp_field[i] * num_headers) + num_sort_header].address;
			size = real_table->block->tuple_dict[(temp_field[i] * num_headers) + num_sort_header].size;
			memset(x, '\0', MAX_VARCHAR_LENGTH);
			memcpy(x, real_table->block->data + address, size);
			//second data -> y
			address = real_table->block->tuple_dict[(temp_field[j] * num_headers) + num_sort_header].address;
			size = real_table->block->tuple_dict[(temp_field[j] * num_headers) + num_sort_header].size;
			memset(y, '\0', MAX_VARCHAR_LENGTH);
			memcpy(y, real_table->block->data + address, size);

			//comparison (x > y) ASC
			if (strcmp(x, y) >= 0) {
				temp=temp_field[i];
				temp_field[i]=temp_field[j];
				temp_field[j]=temp;
			}//end: if(strcmp(x,y) >= 0)
		}
	}

	for (i = 0; i<num_records; i++) {
		//initialize a new row
		struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
		AK_Init_L3(&row_root);
		for (j = 0; j < num_headers; j++) {
			//get data from column 'j' orginal table -> data
			address = real_table->block->tuple_dict[j + (temp_field[i] * num_headers)].address;
			size = real_table->block->tuple_dict[j + (temp_field[i] * num_headers)].size;
			type = real_table->block->tuple_dict[j + (temp_field[i] * num_headers)].type;
			memset(data, '\0', MAX_VARCHAR_LENGTH);
			memcpy(data, real_table->block->data + address, size);
			//add column 'j' data into struct row_root
			AK_Insert_New_Element(type, data, destTable, real_table->block->header[j].att_name, row_root);
		}
		//add row to new sorted table
		AK_insert_row(row_root);
		AK_DeleteAll_L3(&row_root);
	}

	AK_EPI;
	return EXIT_SUCCESS;
}

/**
 * @author Unknown
 * @brief Function that resets block
 * @param block block to be resetted
 * @return No return value
 */
void AK_reset_block(AK_block * block) {
    register int i, j, k;

    AK_header empty_header[ MAX_ATTRIBUTES ];
    AK_tuple_dict empty_tuple_dict[ DATA_BLOCK_SIZE ];
    unsigned char empty_data[ DATA_BLOCK_SIZE * DATA_ENTRY_SIZE ];
    AK_PRO;

    for (i = 0; i < MAX_ATTRIBUTES; i++) {
        empty_header[i].type = FREE_INT;
        for (j = 0; j < MAX_ATT_NAME; j++) {
            empty_header[i].att_name[j] = FREE_CHAR;
        }
        for (j = 0; j < MAX_CONSTRAINTS; j++) {
            empty_header[i].integrity[j] = FREE_INT;
            for (k = 0; k < MAX_CONSTR_NAME; k++) {
                empty_header[i].constr_name[j][k] = FREE_CHAR;
            }
            for (k = 0; k < MAX_CONSTR_CODE; k++) {
                empty_header[i].constr_code[j][k] = FREE_CHAR;
            }
        }
    }

    for (i = 0; i < DATA_BLOCK_SIZE; i++) {
        empty_tuple_dict[i].type = FREE_INT;
        empty_tuple_dict[i].address = FREE_INT;
        empty_tuple_dict[i].size = FREE_INT;
    }

    for (i = 0; i < DATA_BLOCK_SIZE * DATA_ENTRY_SIZE; i++) {
        empty_data[i] = FREE_CHAR;
    }

    block->type = BLOCK_TYPE_FREE;
    block->chained_with = NOT_CHAINED;
    block->AK_free_space = 0;

    memcpy(block->header, empty_header, sizeof (*empty_header));
    memcpy(block->tuple_dict, empty_tuple_dict, sizeof ( *empty_tuple_dict));
    memcpy(block->data, empty_data, sizeof (*empty_data));
    AK_EPI;
}

/**
  * @author Bakoš Nikola
  * @version v1.0
  * @brief Function that sorts the given block
  * @param iBlock block to be sorted
  * @return No return value
 */
void AK_block_sort(AK_block * iBlock, char* attribute_name) {
    register int i, j, k, n, t, q;
    char x[DATA_ROW_SIZE]; //data which interests us
    char y[DATA_ROW_SIZE]; //data used for comparison
    AK_PRO;



    AK_block * cTemp1 = (AK_block*) AK_malloc(sizeof (AK_block));
    cTemp1 = (AK_block *) AK_read_block(15);

    int tip = 0;
    int broj_td;

    AK_block *cTemp2 = (AK_block*) AK_malloc(sizeof (AK_block));
    cTemp2 = (AK_block *) AK_read_block(16);

    AK_header *block_header = (AK_header *) AK_malloc(sizeof (AK_header));
    memcpy(block_header, iBlock->header, sizeof (AK_header));

    int num_sort_header = AK_get_header_number(iBlock, attribute_name); //number of headers which are used for sort

    int max_header_num = AK_get_total_headers(iBlock); //total number of headers for one record
    int num_tuples = AK_get_num_of_tuples(iBlock);

    AK_dbg_messg(HIGH, FILE_MAN, "\n tu sam: %i, %i", num_sort_header, num_tuples);

    unsigned char data[MAX_VARCHAR_LENGTH]; //it was 2000 before MAX_VARCHAR_LENGHT
    int ubr1 = num_tuples / 2; //total number of first half
    int ubr2 = num_tuples - ubr1;

    for (n = 1; n < num_tuples; n = n * 2) {
        AK_reset_block(cTemp1);
        AK_reset_block(cTemp2);
        //n=1;

        broj_td = 0;
        //write a half in the first temp block
        for (t = 0; t < ubr1; t++) {
            AK_dbg_messg(HIGH, FILE_MAN, "block_sort: 1) sada sam na %i / %i\n", t, num_tuples);

            for (i = 0; i < max_header_num; i++) {
                memset(data, '\0', MAX_VARCHAR_LENGTH);
                memcpy(data, iBlock->data + iBlock->tuple_dict[i + (t * max_header_num)].address,
                        iBlock->tuple_dict[ i + (t * max_header_num) ].size);
                AK_insert_entry(cTemp1, iBlock->tuple_dict[i + (t * max_header_num)].type, data, broj_td);
                broj_td++;
            }
        }

        memcpy(cTemp1->header, block_header, sizeof (AK_header));
        AK_write_block(cTemp1); 
        printf("   \n\n               in cTemp1        data, brtd: %i\n", broj_td);

        for (i = 0; i < broj_td; i++) {
            memset(data, '\0', MAX_VARCHAR_LENGTH);
            memcpy(data, cTemp1->data + cTemp1->tuple_dict[i].address, cTemp1->tuple_dict[i].size);
            printf(" %s ", data);
        }
        printf("\n\n");

        //writing other half in second temp block
        broj_td = 0;
        for (; t < num_tuples; t++) {
            AK_dbg_messg(HIGH, FILE_MAN, "block_sort: 2) sada sam na %i / %i\n", t, num_tuples);

            for (i = 0; i < max_header_num; i++) {
                memset(data, '\0', MAX_VARCHAR_LENGTH);
                memcpy(data, iBlock->data + iBlock->tuple_dict[i + (t * max_header_num)].address,
                        iBlock->tuple_dict[i + (t * max_header_num)].size);
                //printf ("                          podatak: %s      na: %i  adresa: %i\n", podatak, broj_td, iBlock->tuple_dict[ i + (t * max_header_num) ].address);
                AK_insert_entry(cTemp2, iBlock->tuple_dict[i + (t * max_header_num)].type, data, broj_td);
                broj_td++;
            }
        }
        memcpy(cTemp2->header, block_header, sizeof (AK_header));
        if (AK_write_block(cTemp2) == EXIT_ERROR) //TO DO
            printf("cTemp2 writing error\n");

        printf("   \n\n               in cTemp2        data, brtd: %i\n", broj_td);
        for (i = 0; i < broj_td; i++) {
            memset(data, '\0', MAX_VARCHAR_LENGTH);
            memcpy(data, cTemp2->data + cTemp2->tuple_dict[i].address, cTemp2->tuple_dict[i].size);
            printf(" %s ", data);
        }
        printf("\n\n");

        //Start sorting
        AK_reset_block(iBlock);

        AK_dbg_messg(HIGH, FILE_MAN, "                      , size: %i\n", iBlock->AK_free_space);

        memcpy(iBlock->header, block_header, sizeof (AK_header));

        int br1, br2;
        broj_td = br1 = br2 = j = k = 0;

        for (q = 0; q < ubr1; q += n) {
            printf("\nfor (q = 0; q < num_tuples/2; q += n ) q=%i , j=%i   ,  k=%i  , br1=%i  , br2=%i\n", q, j, k, br1, br2);
            for (j = k = 0;;) {
                memset(x, '\0', DATA_ROW_SIZE);
                memcpy(x, cTemp1->data + cTemp1->tuple_dict[br1 * max_header_num + num_sort_header].address,
                        cTemp1->tuple_dict[br1 * max_header_num + num_sort_header].size);
                //x[cTemp1->tuple_dict[br1 * max_header_num + num_sort_header].size - 1]="\0";

                memset(y, '\0', DATA_ROW_SIZE);
                memcpy(y, cTemp2->data + cTemp2->tuple_dict[br2 * max_header_num + num_sort_header ].address,
                        cTemp2->tuple_dict[br2 * max_header_num + num_sort_header].size);
                //y[cTemp2->tuple_dict[br2 * max_header_num + num_sort_header].size - 1]="\0";

                AK_dbg_messg(HIGH, FILE_MAN, "slogovi: %s , %s   , head: %i \n", x, y, num_sort_header);

                //comparison
                if (strcmp(x, y) <= 0) {
                    AK_dbg_messg(HIGH, FILE_MAN, "manji je: %s\n", x);

                    //insert data
                    for (i = 0; i < max_header_num; i++) {
                        memset(data, '\0', MAX_VARCHAR_LENGTH);
                        memcpy(data, cTemp1->data + cTemp1->tuple_dict[i + (br1 * max_header_num)].address,
                                cTemp1->tuple_dict[i + (br1 * max_header_num)].size);

                        tip = cTemp1->tuple_dict[i + (br1 * max_header_num)].type;
                        AK_insert_entry(iBlock, tip, data, broj_td);
                        broj_td += 1;
                    }
                    br1++;
                    j++;

                    if (j < n && br1 < ubr1) { //još ima podataka u cTemp1
                        printf("\nif(j<n){    //još ima podataka u cTemp1  j=%i , n=%i\n", j, n);
                        memset(x, '\0', DATA_ROW_SIZE);
                        memcpy(x, cTemp1->data + cTemp1->tuple_dict[br1 * max_header_num + num_sort_header].address,
                                cTemp1->tuple_dict[br1 * max_header_num + num_sort_header].size);
                        //x[cTemp1->tuple_dict[br1 * max_header_num + num_sort_header].size]="\0";
                    } else {
                        printf("\n\nbreak j<n\n\n");
                        break;
                    }
                } else {
                    AK_dbg_messg(HIGH, FILE_MAN, "manji je: %s\n", y);

                    //insert data
                    for (i = 0; i < max_header_num; i++) {
                        memset(data, '\0', MAX_VARCHAR_LENGTH);
                        memcpy(data, cTemp2->data + cTemp2->tuple_dict[i + (br2 * max_header_num)].address,
                                cTemp2->tuple_dict[i + (br2 * max_header_num)].size);
                        tip = cTemp2->tuple_dict[i + (br2 * max_header_num)].type;
                        AK_insert_entry(iBlock, tip, data, broj_td);
                        broj_td += 1;
                    }
                    br2++;
                    k++;

                    if (k < n && br2 < ubr2) { //data left in cTemp2
                        printf("\nif(k<n){    //još ima podataka u cTemp1  k=%i , n=%i\n", k, n);
                        memset(y, '\0', DATA_ROW_SIZE);
                        memcpy(y, cTemp2->data + cTemp2->tuple_dict[br2 * max_header_num + num_sort_header].address,
                                cTemp2->tuple_dict[ br2 * max_header_num + num_sort_header].size);
                        //y[cTemp1->tuple_dict[br2 * max_header_num + num_sort_header].size]="\0";
                    } else {
                        printf("\n\nbreak k<n\n\n");
                        break;
                    }
                }
            }

            printf("\n\nmore data if(j<n) j=%i , n=%i, br1=%i, ubr1=%i\n\n", k, n, br1, ubr1);
            if (j < n && br1 < ubr1) { //data left in cTemp1
                if (!(br1 < ubr1))
                    break;

                //insert data
                for (i = 0; i < max_header_num; i++) {
                    memset(data, '\0', MAX_VARCHAR_LENGTH);
                    memcpy(data, cTemp1->data + cTemp1->tuple_dict[i + (br1 * max_header_num)].address,
                            cTemp1->tuple_dict[i + (br1 * max_header_num)].size);
                    tip = cTemp1->tuple_dict[i + (br1 * max_header_num)].type;
                    AK_insert_entry(iBlock, tip, data, broj_td);
                    broj_td += 1;
                }
                br1++;
                j++;
            }

            printf("\n\nmore data if(k<n) k=%i , n=%i, br2=%i, ubr2=%i\n\n", k, n, br2, ubr2);
            if (k < n && br2 < ubr2) { //data left in cTemp2
                if (!(br2 < ubr2))
                    break;
                //insert data
                for (i = 0; i < max_header_num; i++) {
                    memset(data, '\0', MAX_VARCHAR_LENGTH);
                    memcpy(data, cTemp2->data + cTemp2->tuple_dict[i + (br2 * max_header_num)].address,
                            cTemp2->tuple_dict[i + (br2 * max_header_num)].size);
                    tip = cTemp2->tuple_dict[i + (br2 * max_header_num)].type;
                    AK_insert_entry(iBlock, tip, data, broj_td);
                    broj_td += 1;
                }
                br2++;
                k++;
            }

            //if one of the files is empty
            for (; j < n; j++) {
                if (!(br1 < ubr1))
                    break;
                //insert data
                printf("\n\n for(; j<n; j++)  br1=%i    //insert data brtd=%i\n\n", br1, broj_td);
                for (i = 0; i < max_header_num; i++) {
                    memset(data, '\0', MAX_VARCHAR_LENGTH);
                    memcpy(data, cTemp1->data + cTemp1->tuple_dict[i + (br1 * max_header_num)].address,
                            cTemp1->tuple_dict[i + (br1 * max_header_num)].size);
                    tip = cTemp1->tuple_dict[i + (br1 * max_header_num)].type;
                    AK_insert_entry(iBlock, tip, data, broj_td);
                    broj_td += 1;
                }
                br1++;
            }

            for (; k < n; k++) {
                if (!(br2 < ubr2))
                    break;
                //insert data
                printf("\n\n for(; k<n; k++)  br2=%i   //insert data brtd=%i\n\n", br2, broj_td);
                for (i = 0; i < max_header_num; i++) {
                    memset(data, '\0', MAX_VARCHAR_LENGTH);
                    memcpy(data, cTemp2->data + cTemp2->tuple_dict[i + (br2 * max_header_num)].address,
                            cTemp2->tuple_dict[i + (br2 * max_header_num)].size);
                    tip = cTemp2->tuple_dict[i + (br2 * max_header_num)].type;
                    AK_insert_entry(iBlock, tip, data, broj_td);
                    broj_td += 1;
                }
                br2++;
            }
        }
        printf("\n\n KORAK ZAVRSIO \n\n");
        AK_write_block(iBlock); 
        printf("   \n\n               in iBlock        data, brtd: %i\n", broj_td);

        for (i = 0; i < broj_td; i++) {
            memset(data, '\0', MAX_VARCHAR_LENGTH);
            memcpy(data, iBlock->data + iBlock->tuple_dict[i].address, iBlock->tuple_dict[i].size);
            printf(" %s ", data);
        }
        printf("\n\n");
    }
    AK_reset_block(cTemp1);
    AK_reset_block(cTemp2); 

    AK_write_block(cTemp1); 
    AK_write_block(cTemp2);
    AK_EPI;
}

//extern int address_of_tempBlock = 0;
/*
 * @author Unknown, updated Tomislav Bobinac, Filip Žmuk
 * @brief Function that sorts files
 * @return No return value
 */
TestResult AK_filesort_test() {
	AK_PRO;
	printf("filesort_test: Present!\n");
    int success=0;
    int failed=0;
	char *srcTable="student";
	char *destTable="student_sorted";

	AK_print_table(srcTable);

    struct list_node* attributes = (struct list_node*) AK_malloc(sizeof(struct list_node));
    AK_InsertAtBegin_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes); 

	if (AK_sort_segment(srcTable, destTable,  attributes) == EXIT_SUCCESS)
    {
        AK_print_table(destTable);
        success++;
    }
    else
    {
        failed++;
    }    

	AK_EPI;
    return TEST_result(success,failed);
}
