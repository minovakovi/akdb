/**
@file hash.c Provides functions for Hash indices
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


#include "hash.h"

/**
  * @author Mislav Čakarić
  * @brief Function that computes a hash value from varchar or integer
  * @param elem element of row for wich value is to be computed
  * @return hash value
 
 */
int AK_elem_hash_value(struct list_node *elem) {
    int type = elem->type, value = 0, i = 0;
    char temp_char[MAX_VARCHAR_LENGTH];
    AK_PRO;
    switch (type) {
        case TYPE_INT:
            memcpy(&value, elem->data, elem->size);
            break;
        case TYPE_VARCHAR:
            memcpy(temp_char, elem->data, elem->size);
            temp_char[ elem->size ] = '\0';
            do {
                value += (int) temp_char[i++];
            } while (temp_char[i]);
            break;
    }
    AK_EPI;
    return value;
}

/**
  * @author Mislav Čakarić
  * @brief Function that inserts a bucket to block
  * @param indexName name of index
  * @param data content of bucket stored in char array
  * @param type type of bucket (MAIN_BUCKET or HASH_BUCKET)
  * @return address structure with data where the bucket is stored 
 */
struct_add* AK_insert_bucket_to_block(char *indexName, char *data, int type) {
    int size, id;
    AK_PRO;
    struct_add *add = (struct_add*) AK_malloc(sizeof (struct_add));
    add->addBlock = 0;
    add->indexTd = 0;

    int adr_to_write = (int) AK_find_AK_free_space(AK_get_index_addresses(indexName));
    if (adr_to_write == -1)
        adr_to_write = (int) AK_init_new_extent(indexName, SEGMENT_TYPE_INDEX);
    if (adr_to_write == 0){
	AK_EPI;
        return add;
    }

    AK_block *block = (AK_block*) AK_read_block(adr_to_write);

    AK_dbg_messg(HIGH, INDICES, "insert_bucket_to_block: Position to write (tuple_dict_index) %d\n", adr_to_write);

    switch (type) {
        case MAIN_BUCKET:
            size = sizeof (main_bucket);
            break;
        case HASH_BUCKET:
            size = sizeof (hash_bucket);
            break;
    }
    id = block->last_tuple_dict_id + 1;
    memcpy(&block->data[block->AK_free_space], data, size);
    block->tuple_dict[id].address = block->AK_free_space;
    block->AK_free_space += size;
    block->tuple_dict[id].type = type;
    block->tuple_dict[id].size = size;
    block->last_tuple_dict_id = id;
    AK_write_block(block);

    add->addBlock = adr_to_write;
    add->indexTd = id;
    AK_EPI;
    return add;
}

/**
  *  @author Mislav Čakarić
  *  @brief Function that updates a bucket in block
  *  @param add address of where the bucket is stored
  *  @param data content of bucket stored in char array
  *  @return No return value
 */
void AK_update_bucket_in_block(struct_add *add, char *data) {
    AK_PRO;
    AK_block *block = (AK_block*) AK_read_block(add->addBlock);
    int address = block->tuple_dict[add->indexTd].address;
    int size = block->tuple_dict[add->indexTd].size;
    memcpy(&block->data[address], data, size);
    AK_write_block(block);
    AK_EPI;
}

/**
  * @author Mislav Čakarić
  * @brief Function that changes a info of hash index
  * @param indexName name of index
  * @param modulo value for modulo hash function
  * @param main_bucket_num number of main buckets
  * @param hash_bucket_num number of hash buckets
  * @return No return value
 */
void AK_change_hash_info(char *indexName, int modulo, int main_bucket_num, int hash_bucket_num) {
    AK_PRO;
    table_addresses *hash_addresses = (table_addresses*) AK_get_index_addresses(indexName);
    int block_add = hash_addresses->address_from[ 0 ];
    if (block_add == 0) {
        printf("Hash index does not exist!\n");
    }
    AK_block *block = (AK_block*) AK_read_block(block_add);
    hash_info *info = (hash_info*) AK_malloc(sizeof (hash_info));
    info->modulo = modulo;
    info->main_bucket_num = main_bucket_num;
    info->hash_bucket_num = hash_bucket_num;

    memcpy(block->data, info, sizeof (hash_info));
    block->tuple_dict[0].address = 0;
    block->tuple_dict[0].type = INFO_BUCKET;
    block->tuple_dict[0].size = sizeof (hash_info);
    AK_write_block(block);
    AK_EPI;
}

/**
  * @author Mislav Čakarić
  * @brief Function that fetches the info for hash index
  * @param indexName name of index
  * @return info bucket with info data for hash index
 */
hash_info* AK_get_hash_info(char *indexName) {
    AK_PRO;
    table_addresses *hash_addresses = (table_addresses*) AK_get_index_addresses(indexName);
    int block_add = hash_addresses->address_from[ 0 ];
    hash_info *info = (hash_info*) AK_malloc(sizeof (hash_info));
    memset(info, 0, sizeof (hash_info));
    if (block_add == 0) {
        printf("Hash index does not exist!\n");
	AK_EPI;
        return info;
    }
    AK_block *block = (AK_block*) AK_read_block(block_add);
    memcpy(info, block->data, sizeof (hash_info));
    AK_EPI;
    return info;
}

/**
  * @author Mislav Čakarić
  * @brief Function that fetches nth main bucket
  * @param indexName name of index
  * @param n number of main bucket
  * @return address structure with data where the bucket is stored
 */
struct_add* AK_get_nth_main_bucket_add(char *indexName, int n) {
    int i = 0, j = 0, k = 0, counter = 0, end = 0;
    AK_PRO;
    struct_add *add = (struct_add*) AK_malloc(sizeof (struct_add));
    add->addBlock = 301;
    add->indexTd = 2;
    table_addresses *addresses = (table_addresses*) AK_get_index_addresses(indexName);
    while (addresses->address_from[i]) {
        for (j = addresses->address_from[i]; j < addresses->address_to[i]; j++) {
            AK_block *temp = (AK_block*) AK_read_block(j);
            for (k = 0; k < DATA_BLOCK_SIZE; k++) {
                if (temp->tuple_dict[k].type == FREE_INT)
                    break;
                if (temp->tuple_dict[k].type == MAIN_BUCKET) {
                    if (n == counter) {
                        add->addBlock = j;
                        add->indexTd = k;
                        end = 1;
                        break;
                    }
                    counter++;
                }
            }
            if (end) break;
        }
        i++;
    }
    AK_EPI;
    return add;
    AK_free(add);
}

/**
  *  @author Mislav Čakarić
  *  @brief Function that inserts a record in hash bucket
  *  @param indexName name of index
  *  @param hashValue hash value of record that is being inserted
  *  @param add address structure with data where the hash bucket is stored
  *  @return No return value
 */
void AK_insert_in_hash_index(char *indexName, int hashValue, struct_add *add) {
    int i, address, size, hash_AK_free_space = 0;
    AK_PRO;
    struct_add *main_add = (struct_add*) AK_malloc(sizeof (struct_add));
    struct_add *hash_add = (struct_add*) AK_malloc(sizeof (struct_add));
    main_bucket *temp_main_bucket = (main_bucket*) AK_malloc(sizeof (main_bucket));
    hash_bucket *temp_hash_bucket = (hash_bucket*) AK_malloc(sizeof (hash_bucket));

    table_addresses *addresses = (table_addresses*) AK_get_index_addresses(indexName);
    if (addresses->address_from[0] == 0)
        printf("Hash index does not exist!\n");
    else {
        char data[255];
        memset(data, 0, 255);
        hash_info *info = (hash_info*) AK_malloc(sizeof (hash_info));
        info = AK_get_hash_info(indexName);
        if (info->main_bucket_num == 0) {
            for (i = 0; i < MAIN_BUCKET_SIZE; i++) {
                temp_main_bucket->element[i].value = i;
                memset(&temp_main_bucket->element[i].add, 0, sizeof (struct_add));
            }
            memcpy(&data, temp_main_bucket, sizeof (main_bucket));
            main_add = AK_insert_bucket_to_block(indexName, data, MAIN_BUCKET);

            temp_hash_bucket->bucket_level = MAIN_BUCKET_SIZE;
            for (i = 0; i < HASH_BUCKET_SIZE; i++) {
                temp_hash_bucket->element[i].value = -1;
            }
            memcpy(&data, temp_hash_bucket, sizeof (hash_bucket));
            for (i = 0; i < MAIN_BUCKET_SIZE; i++) {
                hash_add = AK_insert_bucket_to_block(indexName, data, HASH_BUCKET);

                memcpy(&temp_main_bucket->element[i].add, hash_add, sizeof (struct_add));
            }
            memcpy(&data, temp_main_bucket, sizeof (main_bucket));
            AK_update_bucket_in_block(main_add, data);
            AK_change_hash_info(indexName, MAIN_BUCKET_SIZE, 1, MAIN_BUCKET_SIZE);
        }
        int hash_bucket_id = hashValue % info->modulo;
        int main_bucket_id = (int) (hash_bucket_id / MAIN_BUCKET_SIZE);

        main_add = AK_get_nth_main_bucket_add(indexName, main_bucket_id);

        AK_block *temp_block = (AK_block*) AK_read_block(main_add->addBlock);
        address = temp_block->tuple_dict[main_add->indexTd].address;
        size = temp_block->tuple_dict[main_add->indexTd].size;
        memcpy(temp_main_bucket, &temp_block->data[address], size);

        memcpy(hash_add, &temp_main_bucket->element[hash_bucket_id % MAIN_BUCKET_SIZE].add, sizeof (struct_add));


        temp_block = (AK_block*) AK_read_block(hash_add->addBlock);
        address = temp_block->tuple_dict[hash_add->indexTd].address;
        size = temp_block->tuple_dict[hash_add->indexTd].size;
        memcpy(temp_hash_bucket, &temp_block->data[address], size);
        for (i = 0; i < HASH_BUCKET_SIZE; i++) {
            if (temp_hash_bucket->element[i].value == -1) {
                hash_AK_free_space = 1;
                temp_hash_bucket->element[i].value = hashValue;
                memcpy(&temp_hash_bucket->element[i].add, add, sizeof (struct_add));
                memcpy(&data, temp_hash_bucket, sizeof (hash_bucket));
                AK_update_bucket_in_block(hash_add, data);
                break;
            }
        }
        if (hash_AK_free_space == 0) {

            if (temp_hash_bucket->bucket_level == info->modulo) {
                //adding new main buckets
                for (i = 0; i < info->main_bucket_num; i++) {
                    main_add = AK_get_nth_main_bucket_add(indexName, i);
                    AK_block *temp_block = (AK_block*) AK_read_block(main_add->addBlock);
                    address = temp_block->tuple_dict[main_add->indexTd].address;
                    size = temp_block->tuple_dict[main_add->indexTd].size;
                    memcpy(data, &temp_block->data[address], size);
                    AK_insert_bucket_to_block(indexName, data, MAIN_BUCKET);
                }
                AK_change_hash_info(indexName, info->modulo * 2, info->main_bucket_num * 2, info->hash_bucket_num);
                info = AK_get_hash_info(indexName);
            }
            int hash_bucket_id2 = (hash_bucket_id + info->modulo / 2) % info->modulo;
            int main_bucket_id2 = (int) (hash_bucket_id2 / MAIN_BUCKET_SIZE);

            //swapping hash bucket id's
            if (hash_bucket_id2 < hash_bucket_id) {
                int temp = hash_bucket_id;
                hash_bucket_id = hash_bucket_id2;
                hash_bucket_id2 = temp;
                temp = main_bucket_id;
                main_bucket_id = main_bucket_id2;
                main_bucket_id2 = temp;
            }

            hash_bucket *temp_hash_bucket2 = (hash_bucket*) AK_malloc(sizeof (hash_bucket));
            temp_hash_bucket2->bucket_level = temp_hash_bucket->bucket_level * 2;
            for (i = 0; i < HASH_BUCKET_SIZE; i++) {
                temp_hash_bucket2->element[i].value = -1;
                memset(&temp_hash_bucket2->element[i].add, 0, sizeof (struct_add));
            }
            memcpy(data, temp_hash_bucket2, sizeof (hash_bucket));
            AK_update_bucket_in_block(hash_add, data);

            main_add = AK_get_nth_main_bucket_add(indexName, main_bucket_id2);
            temp_block = (AK_block*) AK_read_block(main_add->addBlock);
            address = temp_block->tuple_dict[main_add->indexTd].address;
            size = temp_block->tuple_dict[main_add->indexTd].size;
            memcpy(temp_main_bucket, &temp_block->data[address], size);

            hash_add = AK_insert_bucket_to_block(indexName, data, HASH_BUCKET);
            memcpy(&temp_main_bucket->element[hash_bucket_id2 % MAIN_BUCKET_SIZE].add, hash_add, sizeof (struct_add));
            memcpy(data, temp_main_bucket, sizeof (main_bucket));
            AK_update_bucket_in_block(main_add, data);

            AK_change_hash_info(indexName, info->modulo, info->main_bucket_num, info->hash_bucket_num + 1);
            for (i = 0; i < HASH_BUCKET_SIZE; i++) {
                int value = temp_hash_bucket->element[i].value;
                memcpy(main_add, &temp_hash_bucket->element[i].add, sizeof (struct_add));
                AK_insert_in_hash_index(indexName, value, main_add);
            }
            AK_insert_in_hash_index(indexName, hashValue, add);
        }
    }
    AK_EPI;
}

/**
  * @author Mislav Čakarić
  * @brief Function that fetches or deletes a record from hash index
  * @param indexName name of index
  * @param values list of values (one row) to search in hash index
  * @param delete if delete is 0 then record is only read otherwise it's deleted from hash index
  * @return address structure with data where the record is in table
 
 */
//Error: expected a ')'
struct_add *AK_find_delete_in_hash_index(char *indexName, struct list_node *values, int delete) {
    AK_PRO;
    struct_add *add = (struct_add*) AK_malloc(sizeof (struct_add));
    memset(add, 0, sizeof (struct_add));
    table_addresses *addresses = (table_addresses*) AK_get_index_addresses(indexName);
    if (addresses->address_from[0] == 0) {
        printf("Hash index does not exist!\n");
        AK_EPI;
        return add;
    } else {
        int hashValue = 0, address, size, i, j, k, found, match;
        struct list_node *temp_elem;
        temp_elem = AK_First_L2(values);
        while (temp_elem) {
            hashValue += AK_elem_hash_value(temp_elem);
            temp_elem = AK_Next_L2(temp_elem);
        }
        struct_add *main_add = (struct_add*) AK_malloc(sizeof (struct_add));
        struct_add *hash_add = (struct_add*) AK_malloc(sizeof (struct_add));
        main_bucket *temp_main_bucket = (main_bucket*) AK_malloc(sizeof (main_bucket));
        hash_bucket *temp_hash_bucket = (hash_bucket*) AK_malloc(sizeof (hash_bucket));
        char data[255];
        memset(data, 0, 255);
        hash_info *info = (hash_info*) AK_malloc(sizeof (hash_info));
        info = AK_get_hash_info(indexName);
        int hash_bucket_id = hashValue % info->modulo;
        int main_bucket_id = (int) (hash_bucket_id / MAIN_BUCKET_SIZE);

        main_add = AK_get_nth_main_bucket_add(indexName, main_bucket_id);
        AK_block *temp_block = (AK_block*) AK_read_block(main_add->addBlock);
        address = temp_block->tuple_dict[main_add->indexTd].address;
        size = temp_block->tuple_dict[main_add->indexTd].size;
        memcpy(temp_main_bucket, &temp_block->data[address], size);

        memcpy(hash_add, &temp_main_bucket->element[hash_bucket_id % MAIN_BUCKET_SIZE].add, sizeof (struct_add));

        temp_block = (AK_block*) AK_read_block(hash_add->addBlock);
        address = temp_block->tuple_dict[hash_add->indexTd].address;
        size = temp_block->tuple_dict[hash_add->indexTd].size;
        memcpy(temp_hash_bucket, &temp_block->data[address], size);
        for (i = 0; i < HASH_BUCKET_SIZE; i++) {
            if (temp_hash_bucket->element[i].value == hashValue) {
                AK_block *temp_table_block = (AK_block*) AK_read_block(temp_hash_bucket->element[i].add.addBlock);
                j = 0;
                while (strcmp(temp_block->header[j].att_name, "\0")) {
                    k = 0;
                    found = 0;
                    while (strcmp(temp_table_block->header[k].att_name, "\0")) {
                        if (strcmp(temp_block->header[j].att_name, temp_table_block->header[k].att_name) == 0) {
                            found = 1;
                            match = 0;
                            int indexTd = temp_hash_bucket->element[i].add.indexTd + k;
                            int record_address = temp_table_block->tuple_dict[indexTd].address;
                            int record_size = temp_table_block->tuple_dict[indexTd].size;
                            int record_type = temp_table_block->tuple_dict[indexTd].type;
                            memcpy(data, &temp_table_block->data[record_address], record_size);
                            temp_elem = (struct list_node *)AK_First_L2(values);
                            while (temp_elem) {
                                if (temp_elem->type == record_type && memcmp(data, &temp_elem->data, record_size) == 0)
                                    match = 1;
                                temp_elem = temp_elem->next;
                            }
                            break;
                        }
                        k++;
                    }
                    if (found == 0 || match == 0) break;
                    j++;
                }
                if (match) {
                    int addBlock = temp_hash_bucket->element[i].add.addBlock;
                    int indexTd = temp_hash_bucket->element[i].add.indexTd;
                    //Error: expected an expression
                    if (delete==1) {
                        temp_hash_bucket->element[i].value = -1;
                        memcpy(data, temp_hash_bucket, sizeof (hash_bucket));
                        AK_update_bucket_in_block(hash_add, data);
                    } else
                        AK_dbg_messg(HIGH, INDICES, "Record found in table block %d and TupleDict ID %d\n", addBlock, indexTd);
                    add->addBlock = addBlock;
                    add->indexTd = indexTd;
                    break;
                }
            }
        }
    }
    AK_EPI;
    return add;
}

/**
  * @author Mislav Čakarić
  * @brief Function that fetches a record from the hash index
  * @param indexName name of index
  * @param values list of values (one row) to search in hash index
  * @return address structure with data where the record is in table
 
 */
struct_add * AK_find_in_hash_index(char *indexName, struct list_node *values) {
    AK_PRO;
    struct_add* ret = AK_find_delete_in_hash_index(indexName, values, FIND);
    AK_EPI;
    return ret;
}

/**
  * @author Mislav Čakarić
  * @brief Function that deletes a record from the hash index
  * @param indexName name of index
  * @param values list of values (one row) to search in hash index
  * @return No return value
 
 */
void AK_delete_in_hash_index(char *indexName, struct list_node *values) {
    AK_PRO;
    AK_find_delete_in_hash_index(indexName, values, DELETE);
    AK_EPI;
}

/**
  * @author Mislav Čakarić
  * @brief Function that creates a hash index
  * @param tblName name of table for which the index is being created
  * @param indexName name of index
  * @param attributes list of attributes over which the index is being created
  * @return success or error
 
 */
int AK_create_hash_index(char *tblName, struct list_node *attributes, char *indexName) {
    int i, j, k, l, n, exist, hashValue;
    AK_PRO;
    table_addresses *addresses = (table_addresses*) AK_get_table_addresses(tblName);
    int num_attr = AK_num_attr(tblName);
    AK_header *table_header = (AK_header *)AK_get_header(tblName);

    AK_header i_header[ MAX_ATTRIBUTES ];
    AK_header* temp;

    struct list_node *attribute = (struct list_node *) AK_First_L2(attributes);
    n = 0;
    while (attribute != 0) {
        exist = 0;
        for (i = 0; i < num_attr; i++) {
            if (strcmp((table_header + i)->att_name, attribute->data) == 0) {
				AK_dbg_messg(HIGH, INDICES, "Attribute %s exist in table, found on position: %d\n", (table_header + i)->att_name, i);
                exist = 1;
                temp = (AK_header*) AK_create_header((table_header + i)->att_name, (table_header + i)->type, FREE_INT, FREE_CHAR, FREE_CHAR);
                memcpy(i_header + n, temp, sizeof ( AK_header));
                n++;
                if ((table_header + 1)->type != TYPE_VARCHAR && (table_header + 1)->type != TYPE_INT) {
                    printf("Unsupported data type for hash index! Only int and varchar!");
                    AK_EPI;
                    return EXIT_ERROR;
                }
            }
        }
        if (!exist) {
            printf("Atribut %s ne postoji u tablici", attribute->data);
            AK_EPI;
            return EXIT_ERROR;
        }
        attribute = attribute->next;
    }
    memset(i_header + n, 0, MAX_ATTRIBUTES - n);

    int startAddress = AK_initialize_new_segment(indexName, SEGMENT_TYPE_INDEX, i_header);
    if (startAddress != EXIT_ERROR)
        printf("\nINDEX %s CREATED!\n", indexName);

    AK_block *block = (AK_block*) AK_read_block(startAddress);
    hash_info *info = (hash_info*) AK_malloc(sizeof (hash_info));
    info->modulo = 4;
    info->main_bucket_num = 0;
    info->hash_bucket_num = 0;
    memcpy(block->data, info, sizeof (hash_info));
    block->tuple_dict[0].address = 0;
    block->tuple_dict[0].type = INFO_BUCKET;
    block->tuple_dict[0].size = sizeof (hash_info);
    block->AK_free_space += sizeof (hash_info);
    block->last_tuple_dict_id = 0;
    AK_write_block(block);

    struct list_node *temp_elem;

    char data[ MAX_VARCHAR_LENGTH ];
    struct list_node *row = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&row);
    i = 0;
    n = 0;
    while (addresses->address_from[ i ]) {
        for (j = addresses->address_from[ i ]; j < addresses->address_to[ i ]; j++) {
            AK_block *temp = (AK_block*) AK_read_block(j);
            for (k = 0; k < DATA_BLOCK_SIZE; k += num_attr) {
                n++;
                if (temp->tuple_dict[k].type == FREE_INT)
                    break;
                for (l = 0; l < num_attr; l++) {
                    int type = temp->tuple_dict[ k + l ].type;
                    int size = temp->tuple_dict[ k + l ].size;
                    int address = temp->tuple_dict[ k + l ].address;
                    memcpy(data, &(temp->data[address]), size);
                    data[ size ] = '\0';
                    AK_InsertAtEnd_L3(type, data, size, row);
                }
                /* *************** */
                hashValue = 0;
                attribute = (struct list_node *) AK_First_L2(attributes);
                while (attribute) {

                    for (l = 0; l < num_attr; l++) {
                        if (strcmp((table_header + l)->att_name, attribute->data) == 0)
                            break;
                    }
                    temp_elem = AK_GetNth_L2(l, row);
                    hashValue += AK_elem_hash_value(temp_elem);

                    attribute = attribute->next;
                }
                AK_dbg_messg(HIGH, INDICES, "Insert in hash index %d. record\n", n);
                struct_add *add = (struct_add*) AK_malloc(sizeof (struct_add));
                add->addBlock = j;
                add->indexTd = k;
                AK_insert_in_hash_index(indexName, hashValue, add);
                AK_DeleteAll_L3(&row);
                /* *************** */
            }
        }
        i++;
    }
    AK_EPI;
    return EXIT_SUCCESS;
}

void AK_delete_hash_index(char *indexName) {
    AK_PRO;
    AK_delete_segment(indexName, SEGMENT_TYPE_INDEX);
    printf("INDEX %s DELETED!\n", indexName);
    AK_EPI;
}

/**
  * @author Mislav Čakarić
  * @brief Function that tests hash index
  * @return No return value 
 */
TestResult AK_hash_test() {
    int passedTest = 0;
    int failedTest = 0;
    char *tblName = "student";
    char *indexName = "student_hash_index";
    //AK_print_table("AK_relation");
    AK_PRO;
    struct list_node *att_list = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&att_list);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "mbr\0", 4, att_list);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname\0", 10, att_list);

    if(AK_create_hash_index(tblName, att_list, indexName) == EXIT_ERROR){
    	failedTest++;
    }
    else{
    	passedTest++;
    }

    struct list_node *values = (struct list_node *) AK_malloc(sizeof (struct list_node));
    struct list_node *row = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&values);

    //AK_delete_hash_index(indexName);

    hash_info *info = (hash_info*) AK_malloc(sizeof (hash_info));
    info = AK_get_hash_info(indexName);
    if(info == NULL){
    	failedTest++;
    }
    else{
    	passedTest++;
    }
    printf("Main buckets:%d, Hash buckets:%d, Modulo:%d\n", info->main_bucket_num, info->hash_bucket_num, info->modulo);

    //AK_delete_hash_index(indexName);
    AK_print_table("AK_relation");
    AK_print_table("AK_index");

    printf("Hash index search test:\n");
    int i, num_rec = AK_get_num_records(tblName);
    for (i = 0; i < num_rec; i++) {
        row = AK_get_row(i, tblName);
        struct list_node *value = AK_GetNth_L2(1, row);
        AK_InsertAtEnd_L3(value->type, value->data, value->size, values);
        value = AK_GetNth_L2(1, row);
        AK_InsertAtEnd_L3(value->type, value->data, value->size, values);
        struct_add *add = AK_find_in_hash_index(indexName, values);
        if(add->addBlock == NULL){
    		failedTest++;
    	}
    	else{
    		passedTest++;
    	}
        AK_DeleteAll_L3(&values);
        if (add->addBlock && add->indexTd)
            printf("Record found in table block %d and TupleDict ID %d\n", add->addBlock, add->indexTd);
    }
    printf("hash_test: Present!\n");
    AK_EPI;
    return TEST_result(passedTest,failedTest);
}
