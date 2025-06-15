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
int AK_elem_hash_value(struct list_node *elem)
{
    int type = elem->type, value = 0, i = 0;
    char temp_char[MAX_VARCHAR_LENGTH];
    AK_PRO;
    switch (type)
    {
    case TYPE_INT:
        memcpy(&value, elem->data, elem->size);
        break;
    case TYPE_VARCHAR:
        memcpy(temp_char, elem->data, elem->size);
        temp_char[elem->size] = '\0';
        do
        {
            value += (int)temp_char[i++];
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
struct_add *AK_insert_bucket_to_block(char *indexName, char *data, int type)
{
    int size, id;
    AK_PRO;
    struct_add *add = (struct_add *)AK_malloc(sizeof(struct_add));
    add->addBlock = 0;
    add->indexTd = 0;

    int adr_to_write = (int)AK_find_AK_free_space(AK_get_index_addresses(indexName));
    if (adr_to_write == -1)
        adr_to_write = (int)AK_init_new_extent(indexName, SEGMENT_TYPE_INDEX);
    if (adr_to_write == 0)
    {
        AK_EPI;
        return add;
    }

    AK_block *block = (AK_block *)AK_read_block(adr_to_write);

    AK_dbg_messg(HIGH, INDICES, "insert_bucket_to_block: Position to write (tuple_dict_index) %d\n", adr_to_write);

    switch (type)
    {
    case MAIN_BUCKET:
        size = sizeof(main_bucket);
        break;
    case HASH_BUCKET:
        size = sizeof(hash_bucket);
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
void AK_update_bucket_in_block(struct_add *add, char *data)
{
    AK_PRO;
    AK_block *block = (AK_block *)AK_read_block(add->addBlock);
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
void AK_change_hash_info(char *indexName, int modulo, int main_bucket_num, int hash_bucket_num)
{
    AK_PRO;
    table_addresses *hash_addresses = (table_addresses *)AK_get_index_addresses(indexName);
    int block_add = hash_addresses->address_from[0];
    if (block_add == 0)
    {
        printf("Hash index does not exist!\n");
    }
    AK_block *block = (AK_block *)AK_read_block(block_add);
    hash_info *info = (hash_info *)AK_malloc(sizeof(hash_info));
    info->modulo = modulo;
    info->main_bucket_num = main_bucket_num;
    info->hash_bucket_num = hash_bucket_num;

    memcpy(block->data, info, sizeof(hash_info));
    block->tuple_dict[0].address = 0;
    block->tuple_dict[0].type = INFO_BUCKET;
    block->tuple_dict[0].size = sizeof(hash_info);
    AK_write_block(block);
    AK_EPI;
}

/**
 * @author Mislav Čakarić
 * @brief Function that fetches the info for hash index
 * @param indexName name of index
 * @return info bucket with info data for hash index
 */
hash_info *AK_get_hash_info(char *indexName)
{
    AK_PRO;
    table_addresses *hash_addresses = (table_addresses *)AK_get_index_addresses(indexName);
    int block_add = hash_addresses->address_from[0];
    hash_info *info = (hash_info *)AK_malloc(sizeof(hash_info));
    memset(info, 0, sizeof(hash_info));
    if (block_add == 0)
    {
        printf("Hash index does not exist!\n");
        AK_EPI;
        return info;
    }
    AK_block *block = (AK_block *)AK_read_block(block_add);
    memcpy(info, block->data, sizeof(hash_info));
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
struct_add *AK_get_nth_main_bucket_add(char *indexName, int n)
{
    int i = 0, j = 0, k = 0, counter = 0, end = 0;
    AK_PRO;
    struct_add *add = (struct_add *)AK_malloc(sizeof(struct_add));
    add->addBlock = 301;
    add->indexTd = 2;
    table_addresses *addresses = (table_addresses *)AK_get_index_addresses(indexName);
    while (addresses->address_from[i])
    {
        for (j = addresses->address_from[i]; j < addresses->address_to[i]; j++)
        {
            AK_block *temp = (AK_block *)AK_read_block(j);
            for (k = 0; k < temp->last_tuple_dict_id; k++)
            {
                if (temp->tuple_dict[k].type == FREE_INT)
                    break;
                if (temp->tuple_dict[k].type == MAIN_BUCKET)
                {
                    if (n == counter)
                    {
                        add->addBlock = j;
                        add->indexTd = k;
                        end = 1;
                        break;
                    }
                    counter++;
                }
            }
            if (end)
                break;
        }
        i++;
    }
    AK_EPI;
    return add;
}

/**
 *  @author Mislav Čakarić
 *  @brief Function that inserts a record in hash bucket
 *  @param indexName name of index
 *  @param hashValue hash value of record that is being inserted
 *  @param add address structure with data where the hash bucket is stored
 *  @return No return value
 */
void AK_insert_in_hash_index(char *indexName, int hashValue, struct_add *add)
{
    int i, address, size, hash_AK_free_space = 0;
    AK_PRO;
    struct_add *main_add = (struct_add *)AK_malloc(sizeof(struct_add));
    struct_add *hash_add = (struct_add *)AK_malloc(sizeof(struct_add));
    main_bucket *temp_main_bucket = (main_bucket *)AK_malloc(sizeof(main_bucket));
    hash_bucket *temp_hash_bucket = (hash_bucket *)AK_malloc(sizeof(hash_bucket));

    table_addresses *addresses = (table_addresses *)AK_get_index_addresses(indexName);
    if (addresses->address_from[0] == 0)
        printf("Hash index does not exist!\n");
    else
    {
        char data[255];
        memset(data, 0, 255);
        hash_info *info = (hash_info *)AK_malloc(sizeof(hash_info));
        info = AK_get_hash_info(indexName);
        if (info->main_bucket_num == 0)
        {
            for (i = 0; i < MAIN_BUCKET_SIZE; i++)
            {
                temp_main_bucket->element[i].value = i;
                memset(&temp_main_bucket->element[i].add, 0, sizeof(struct_add));
            }
            memcpy(&data, temp_main_bucket, sizeof(main_bucket));
            main_add = AK_insert_bucket_to_block(indexName, data, MAIN_BUCKET);

            temp_hash_bucket->bucket_level = MAIN_BUCKET_SIZE;
            for (i = 0; i < HASH_BUCKET_SIZE; i++)
            {
                temp_hash_bucket->element[i].value = -1;
            }
            memcpy(&data, temp_hash_bucket, sizeof(hash_bucket));
            for (i = 0; i < MAIN_BUCKET_SIZE; i++)
            {
                hash_add = AK_insert_bucket_to_block(indexName, data, HASH_BUCKET);

                memcpy(&temp_main_bucket->element[i].add, hash_add, sizeof(struct_add));
            }
            memcpy(&data, temp_main_bucket, sizeof(main_bucket));
            AK_update_bucket_in_block(main_add, data);
            AK_change_hash_info(indexName, MAIN_BUCKET_SIZE, 1, MAIN_BUCKET_SIZE);
        }
        int hash_bucket_id = hashValue % info->modulo;
        int main_bucket_id = (int)(hash_bucket_id / MAIN_BUCKET_SIZE);

        main_add = AK_get_nth_main_bucket_add(indexName, main_bucket_id);

        AK_block *temp_block = (AK_block *)AK_read_block(main_add->addBlock);
        address = temp_block->tuple_dict[main_add->indexTd].address;
        size = temp_block->tuple_dict[main_add->indexTd].size;
        memcpy(temp_main_bucket, &temp_block->data[address], size);

        memcpy(hash_add, &temp_main_bucket->element[hash_bucket_id % MAIN_BUCKET_SIZE].add, sizeof(struct_add));

        temp_block = (AK_block *)AK_read_block(hash_add->addBlock);
        address = temp_block->tuple_dict[hash_add->indexTd].address;
        size = temp_block->tuple_dict[hash_add->indexTd].size;
        memcpy(temp_hash_bucket, &temp_block->data[address], size);
        for (i = 0; i < HASH_BUCKET_SIZE; i++)
        {
            if (temp_hash_bucket->element[i].value == -1)
            {
                hash_AK_free_space = 1;
                temp_hash_bucket->element[i].value = hashValue;
                memcpy(&temp_hash_bucket->element[i].add, add, sizeof(struct_add));
                memcpy(&data, temp_hash_bucket, sizeof(hash_bucket));
                AK_update_bucket_in_block(hash_add, data);
                break;
            }
        }
        if (hash_AK_free_space == 0)
        {

            if (temp_hash_bucket->bucket_level == info->modulo)
            {
                // adding new main buckets
                for (i = 0; i < info->main_bucket_num; i++)
                {
                    main_add = AK_get_nth_main_bucket_add(indexName, i);
                    AK_block *temp_block = (AK_block *)AK_read_block(main_add->addBlock);
                    address = temp_block->tuple_dict[main_add->indexTd].address;
                    size = temp_block->tuple_dict[main_add->indexTd].size;
                    memcpy(data, &temp_block->data[address], size);
                    AK_insert_bucket_to_block(indexName, data, MAIN_BUCKET);
                }
                AK_change_hash_info(indexName, info->modulo * 2, info->main_bucket_num * 2, info->hash_bucket_num);
                info = AK_get_hash_info(indexName);
            }
            int hash_bucket_id2 = (hash_bucket_id + info->modulo / 2) % info->modulo;
            int main_bucket_id2 = (int)(hash_bucket_id2 / MAIN_BUCKET_SIZE);

            // swapping hash bucket id's
            if (hash_bucket_id2 < hash_bucket_id)
            {
                int temp = hash_bucket_id;
                hash_bucket_id = hash_bucket_id2;
                hash_bucket_id2 = temp;
                temp = main_bucket_id;
                main_bucket_id = main_bucket_id2;
                main_bucket_id2 = temp;
            }

            hash_bucket *temp_hash_bucket2 = (hash_bucket *)AK_malloc(sizeof(hash_bucket));
            temp_hash_bucket2->bucket_level = temp_hash_bucket->bucket_level * 2;
            for (i = 0; i < HASH_BUCKET_SIZE; i++)
            {
                temp_hash_bucket2->element[i].value = -1;
                memset(&temp_hash_bucket2->element[i].add, 0, sizeof(struct_add));
            }
            memcpy(data, temp_hash_bucket2, sizeof(hash_bucket));
            AK_update_bucket_in_block(hash_add, data);

            main_add = AK_get_nth_main_bucket_add(indexName, main_bucket_id2);
            temp_block = (AK_block *)AK_read_block(main_add->addBlock);
            address = temp_block->tuple_dict[main_add->indexTd].address;
            size = temp_block->tuple_dict[main_add->indexTd].size;
            memcpy(temp_main_bucket, &temp_block->data[address], size);

            hash_add = AK_insert_bucket_to_block(indexName, data, HASH_BUCKET);
            memcpy(&temp_main_bucket->element[hash_bucket_id2 % MAIN_BUCKET_SIZE].add, hash_add, sizeof(struct_add));
            memcpy(data, temp_main_bucket, sizeof(main_bucket));
            AK_update_bucket_in_block(main_add, data);

            AK_change_hash_info(indexName, info->modulo, info->main_bucket_num, info->hash_bucket_num + 1);
            for (i = 0; i < HASH_BUCKET_SIZE; i++)
            {
                int value = temp_hash_bucket->element[i].value;
                memcpy(main_add, &temp_hash_bucket->element[i].add, sizeof(struct_add));
                AK_insert_in_hash_index(indexName, value, main_add);
            }
            AK_insert_in_hash_index(indexName, hashValue, add);
        }
    }
    AK_EPI;
}

/**
  * @author Mislav Čakarić, updated by Valentina Kušter
  * @brief Function that fetches or deletes a record from hash index
  * @param indexName name of index
  * @param values list of values (one row) to search in hash index
  * @param delete if delete is 0 then record is only read otherwise it's deleted from hash index
  * @return address structure with data where the record is in table

 */

struct_add *AK_find_delete_in_hash_index(char *indexName, struct list_node *values, int delete)
{
    AK_PRO;
    struct_add *result_add = NULL;

    if (delete == 1)
    {
        AK_delete_in_hash_index(indexName, values);
    }
    else
    {
        result_add = AK_find_in_hash_index(indexName, values);
    }

    AK_EPI;
    return result_add;
}

/**
  * @author Mislav Čakarić, updated by Valentina Kušter
  * @brief Function that fetches a record from the hash index
  * @param indexName name of index
  * @param values list of values (one row) to search in hash index
  * @return address structure with data where the record is in table

 */
struct_add *AK_find_in_hash_index(char *indexName, struct list_node *values)
{
    AK_PRO;

    // Allocate memory for return value
    struct_add *add = (struct_add *)AK_malloc(sizeof(struct_add));
    memset(add, 0, sizeof(struct_add));

    // Declarations for intermediate structures
    struct_add *main_add = NULL;
    struct_add *hash_add = NULL;
    main_bucket *temp_main_bucket = NULL;
    hash_bucket *temp_hash_bucket = NULL;
    AK_block *temp_block = NULL; 
    AK_block *temp_table_block = NULL; 
    
    // Get addresses of the hash index segments
    table_addresses *addresses = AK_get_index_addresses(indexName); 
    if (addresses == NULL || addresses->address_from[0] == 0) 
    {
        printf("Hash index does not exist or is invalid!\n");
        AK_EPI;
        return add; 
    }
    else
    {
        int hashValue = 0, address, size, i, j, k, found, match;
        struct list_node *temp_elem;

        // Calculate hash value by summing individual element hashes
        temp_elem = AK_First_L2(values);
        while (temp_elem)
        {
            hashValue += AK_elem_hash_value(temp_elem);
            temp_elem = AK_Next_L2(temp_elem);
        }
        
        // Allocate necessary structures for processing
        main_add = (struct_add *)AK_malloc(sizeof(struct_add));
        hash_add = (struct_add *)AK_malloc(sizeof(struct_add));
        temp_main_bucket = (main_bucket *)AK_malloc(sizeof(main_bucket));
        temp_hash_bucket = (hash_bucket *)AK_malloc(sizeof(hash_bucket));
        char data[255];
        memset(data, 0, 255);
        
        // Get hash index metadata
        hash_info *info = AK_get_hash_info(indexName); 
        if (info == NULL) { 
            printf("Failed to get hash info for index %s!\n", indexName);
            AK_free(main_add);
            AK_free(hash_add);
            AK_free(temp_main_bucket);
            AK_free(temp_hash_bucket);
            AK_EPI;
            return add; 
        }

        // Determine main and hash bucket IDs
        int hash_bucket_id = hashValue % info->modulo;
        int main_bucket_id = (int)(hash_bucket_id / MAIN_BUCKET_SIZE);

        // Get address of the main bucket
        main_add = AK_get_nth_main_bucket_add(indexName, main_bucket_id);
        if (main_add == NULL) { 
            printf("Failed to get main bucket address for index %s, main_bucket_id %d!\n", indexName, main_bucket_id);
            AK_free(main_add); 
            AK_free(hash_add);
            AK_free(temp_main_bucket);
            AK_free(temp_hash_bucket);
            AK_EPI;
            return add; 
        }
        
        // Load the main bucket block
        temp_block = (AK_block *)AK_read_block(main_add->addBlock);
        if (temp_block == NULL) { 
            printf("Failed to read block %d for main bucket.\n", main_add->addBlock);
            AK_free(main_add);
            AK_free(hash_add);
            AK_free(temp_main_bucket);
            AK_free(temp_hash_bucket);
            AK_EPI;
            return add; 
        }
        
        // Copy main bucket data
        address = temp_block->tuple_dict[main_add->indexTd].address;
        size = temp_block->tuple_dict[main_add->indexTd].size;
        memcpy(temp_main_bucket, &temp_block->data[address], size);
        AK_free(temp_block); 
        temp_block = NULL; 

        // Extract hash bucket address from main bucket
        memcpy(hash_add, &temp_main_bucket->element[hash_bucket_id % MAIN_BUCKET_SIZE].add, sizeof(struct_add));

        if (hash_add == NULL || hash_add->addBlock == 0) { 
            printf("Hash bucket address is invalid.\n");
            AK_free(main_add);
            AK_free(hash_add); 
            AK_free(temp_main_bucket);
            AK_free(temp_hash_bucket);
            AK_EPI;
            return add; 
        }
        
        // Load the hash bucket block
        temp_block = (AK_block *)AK_read_block(hash_add->addBlock);
        if (temp_block == NULL) { 
            printf("Failed to read block %d for hash bucket.\n", hash_add->addBlock);
            AK_free(main_add);
            AK_free(hash_add);
            AK_free(temp_main_bucket);
            AK_free(temp_hash_bucket);
            AK_EPI;
            return add; 
        }
        
        // Copy hash bucket data
        address = temp_block->tuple_dict[hash_add->indexTd].address;
        size = temp_block->tuple_dict[hash_add->indexTd].size;
        memcpy(temp_hash_bucket, &temp_block->data[address], size);
        AK_free(temp_block); 
        temp_block = NULL;
        
        // Search for matching record in the hash bucket
        for (i = 0; i < HASH_BUCKET_SIZE; i++)
        {
            if (temp_hash_bucket->element[i].value == hashValue)
            {
                if (temp_hash_bucket->element[i].add.addBlock == 0) {
                    AK_dbg_messg(LOW, INDICES, "Hash element at index %d has invalid block address. Skipping.\n", i);
                    continue;
                }

                // Read the table block for validation
                temp_table_block = (AK_block *)AK_read_block(temp_hash_bucket->element[i].add.addBlock);
                if (temp_table_block == NULL) {
                    AK_dbg_messg(LOW, INDICES, "Failed to read table block %d for validation. Skipping.\n", temp_hash_bucket->element[i].add.addBlock);
                    continue;
                }

                match = 1; 
                struct list_node *current_val = (struct list_node *)AK_First_L2(values);
                int tuple_dict_offset = 0; 
                
                // Validate the tuple content
                while (current_val != NULL) {
                    if ((temp_hash_bucket->element[i].add.indexTd + tuple_dict_offset) >= temp_table_block->last_tuple_dict_id) {
                        match = 0; 
                        AK_dbg_messg(LOW, INDICES, "TupleDict index out of bounds for record validation.\n");
                        break;
                    }

                    int record_address = temp_table_block->tuple_dict[temp_hash_bucket->element[i].add.indexTd + tuple_dict_offset].address;
                    int record_size = temp_table_block->tuple_dict[temp_hash_bucket->element[i].add.indexTd + tuple_dict_offset].size;
                    int record_type = temp_table_block->tuple_dict[temp_hash_bucket->element[i].add.indexTd + tuple_dict_offset].type;

                    if (record_address < 0 || (record_address + record_size) > (DATA_BLOCK_SIZE * DATA_ENTRY_SIZE)) {
                        match = 0; 
                        AK_dbg_messg(LOW, INDICES, "Invalid record data address/size for validation.\n");
                        break;
                    }
                    
                    char temp_record_data[MAX_VARCHAR_LENGTH + 1]; 
                    if (record_size >= MAX_VARCHAR_LENGTH) { 
                        record_size = MAX_VARCHAR_LENGTH -1;
                    }
                    memcpy(temp_record_data, &temp_table_block->data[record_address], record_size);
                    temp_record_data[record_size] = '\0'; 

                    if (current_val->type != record_type || memcmp(temp_record_data, current_val->data, record_size) != 0) {
                        match = 0; 
                        break;
                    }
                    
                    current_val = AK_Next_L2(current_val);
                    tuple_dict_offset++;
                }
                
                // If match is successful, return the found address
                if (match)
                {
                    int addBlock = temp_hash_bucket->element[i].add.addBlock;
                    int indexTd = temp_hash_bucket->element[i].add.indexTd;
                    AK_dbg_messg(HIGH, INDICES, "Record found in table block %d and TupleDict ID %d\n", addBlock, indexTd);
                    add->addBlock = addBlock;
                    add->indexTd = indexTd;
                    
                    AK_free(main_add);
                    AK_free(hash_add);
                    AK_free(temp_main_bucket);
                    AK_free(temp_hash_bucket);
                    if (temp_table_block != NULL) AK_free(temp_table_block);
                    AK_EPI;
                    return add;
                }
                if (temp_table_block != NULL) AK_free(temp_table_block);
                temp_table_block = NULL;
            }
        }
    }
    if (main_add != NULL) AK_free(main_add);
    if (hash_add != NULL) AK_free(hash_add);
    if (temp_main_bucket != NULL) AK_free(temp_main_bucket);
    if (temp_hash_bucket != NULL) AK_free(temp_hash_bucket);
    AK_EPI;
    return add; 
}

/**
  * @author Mislav Čakarić, updated by Valentina Kušter
  * @brief Function that deletes a record from the hash index
  * @param indexName name of index
  * @param values list of values (one row) to search in hash index
  * @return No return value

 */
void AK_delete_in_hash_index(char *indexName, struct list_node *values)
{
    AK_PRO;
    
    // Declare required pointers and buffers
    struct_add *main_add = NULL;
    struct_add *hash_add = NULL;
    main_bucket *temp_main_bucket = NULL;
    hash_bucket *temp_hash_bucket = NULL;
    AK_block *temp_block = NULL;
    AK_block *temp_table_block = NULL;

    // Retrieve index segment addresses
    table_addresses *addresses = AK_get_index_addresses(indexName);
    if (addresses == NULL || addresses->address_from[0] == 0) 
    {
        printf("Hash index does not exist or is invalid!\n");
        AK_EPI;
        return;
    }
    else
    {
        int hashValue = 0, address, size, i, j, k, found, match;
        struct list_node *temp_elem;

        // Compute the hash value from the list of input values
        temp_elem = AK_First_L2(values);
        while (temp_elem)
        {
            hashValue += AK_elem_hash_value(temp_elem);
            temp_elem = AK_Next_L2(temp_elem);
        }
        
        // Allocate required structures for traversal and modification
        main_add = (struct_add *)AK_malloc(sizeof(struct_add));
        hash_add = (struct_add *)AK_malloc(sizeof(struct_add));
        temp_main_bucket = (main_bucket *)AK_malloc(sizeof(main_bucket));
        temp_hash_bucket = (hash_bucket *)AK_malloc(sizeof(hash_bucket));

        char data[255]; 
        memset(data, 0, 255);
        
        // Get hash index metadata
        hash_info *info = AK_get_hash_info(indexName); 
        if (info == NULL) { 
            printf("Failed to get hash info for index %s!\n", indexName);
            if (main_add != NULL) AK_free(main_add);
            if (hash_add != NULL) AK_free(hash_add);
            if (temp_main_bucket != NULL) AK_free(temp_main_bucket);
            if (temp_hash_bucket != NULL) AK_free(temp_hash_bucket);
            AK_EPI;
            return;
        }

        // Determine target main and hash bucket IDs
        int hash_bucket_id = hashValue % info->modulo;
        int main_bucket_id = (int)(hash_bucket_id / MAIN_BUCKET_SIZE);

        // Get address of the corresponding main bucket
        main_add = AK_get_nth_main_bucket_add(indexName, main_bucket_id);
        if (main_add == NULL) { 
            printf("Failed to get main bucket address for index %s, main_bucket_id %d!\n", indexName, main_bucket_id);
            if (main_add != NULL) AK_free(main_add);
            if (hash_add != NULL) AK_free(hash_add);
            if (temp_main_bucket != NULL) AK_free(temp_main_bucket);
            if (temp_hash_bucket != NULL) AK_free(temp_hash_bucket);
            AK_EPI;
            return;
        }
        
        // Load main bucket block and extract its data
        temp_block = (AK_block *)AK_read_block(main_add->addBlock);
        if (temp_block == NULL) { 
            printf("Failed to read block %d for main bucket.\n", main_add->addBlock);
            if (main_add != NULL) AK_free(main_add);
            if (hash_add != NULL) AK_free(hash_add);
            if (temp_main_bucket != NULL) AK_free(temp_main_bucket);
            if (temp_hash_bucket != NULL) AK_free(temp_hash_bucket);
            AK_EPI;
            return;
        }
        
        address = temp_block->tuple_dict[main_add->indexTd].address;
        size = temp_block->tuple_dict[main_add->indexTd].size;
        memcpy(temp_main_bucket, &temp_block->data[address], size);
        AK_free(temp_block); 
        temp_block = NULL;

        // Load the corresponding hash bucket address from main bucket
        memcpy(hash_add, &temp_main_bucket->element[hash_bucket_id % MAIN_BUCKET_SIZE].add, sizeof(struct_add));

        if (hash_add == NULL || hash_add->addBlock == 0) { 
            printf("Hash bucket address is invalid.\n");
            if (main_add != NULL) AK_free(main_add);
            if (hash_add != NULL) AK_free(hash_add); 
            if (temp_main_bucket != NULL) AK_free(temp_main_bucket);
            if (temp_hash_bucket != NULL) AK_free(temp_hash_bucket);
            AK_EPI;
            return;
        }

        // Read hash bucket block and extract the bucket data
        temp_block = (AK_block *)AK_read_block(hash_add->addBlock);
        if (temp_block == NULL) { 
            printf("Failed to read block %d for hash bucket.\n", hash_add->addBlock);
            if (main_add != NULL) AK_free(main_add);
            if (hash_add != NULL) AK_free(hash_add);
            if (temp_main_bucket != NULL) AK_free(temp_main_bucket);
            if (temp_hash_bucket != NULL) AK_free(temp_hash_bucket);
            AK_EPI;
            return;
        }
        
        address = temp_block->tuple_dict[hash_add->indexTd].address;
        size = temp_block->tuple_dict[hash_add->indexTd].size;
        memcpy(temp_hash_bucket, &temp_block->data[address], size);
        AK_free(temp_block); 
        temp_block = NULL;
        
        // Iterate through the elements in the hash bucket
        for (i = 0; i < HASH_BUCKET_SIZE; i++)
        {
            if (temp_hash_bucket->element[i].value == hashValue)
            {
                if (temp_hash_bucket->element[i].add.addBlock == 0) {
                    AK_dbg_messg(LOW, INDICES, "Hash element at index %d has invalid block address. Skipping.\n", i);
                    continue;
                }

                // Read the actual data block from the table for validation
                temp_table_block = (AK_block *)AK_read_block(temp_hash_bucket->element[i].add.addBlock);
                if (temp_table_block == NULL) { 
                    AK_dbg_messg(LOW, INDICES, "Failed to read table block %d for validation. Skipping.\n", temp_hash_bucket->element[i].add.addBlock);
                    continue;
                }
                
                match = 1; 
                struct list_node *current_val = (struct list_node *)AK_First_L2(values);
                int tuple_dict_offset = 0; 
                
                // Compare each attribute in the tuple to input values
                while (current_val != NULL) {
                    if ((temp_hash_bucket->element[i].add.indexTd + tuple_dict_offset) >= temp_table_block->last_tuple_dict_id) {
                        match = 0; 
                        AK_dbg_messg(LOW, INDICES, "TupleDict index out of bounds for record validation during deletion.\n");
                        break;
                    }

                    int record_address = temp_table_block->tuple_dict[temp_hash_bucket->element[i].add.indexTd + tuple_dict_offset].address;
                    int record_size = temp_table_block->tuple_dict[temp_hash_bucket->element[i].add.indexTd + tuple_dict_offset].size;
                    int record_type = temp_table_block->tuple_dict[temp_hash_bucket->element[i].add.indexTd + tuple_dict_offset].type;

                    if (record_address < 0 || (record_address + record_size) > (DATA_BLOCK_SIZE * DATA_ENTRY_SIZE)) {
                        match = 0; 
                        AK_dbg_messg(LOW, INDICES, "Invalid record data address/size for validation during deletion.\n");
                        break;
                    }
                    
                    char temp_record_data[MAX_VARCHAR_LENGTH + 1]; 
                    if (record_size >= MAX_VARCHAR_LENGTH) { 
                        record_size = MAX_VARCHAR_LENGTH -1;
                    }
                    memcpy(temp_record_data, &temp_table_block->data[record_address], record_size);
                    temp_record_data[record_size] = '\0';

                    if (current_val->type != record_type || memcmp(temp_record_data, current_val->data, record_size) != 0) {
                        match = 0; 
                        break;
                    }
                    
                    current_val = AK_Next_L2(current_val);
                    tuple_dict_offset++;
                }

                // If a matching record is found, invalidate it in the hash bucket
                if (match)
                {
                    temp_hash_bucket->element[i].value = -1; 
                    memset(&temp_hash_bucket->element[i].add, 0, sizeof(struct_add)); 
                    memcpy(data, temp_hash_bucket, sizeof(hash_bucket));
                    // Update the block with the modified hash bucket
                    AK_update_bucket_in_block(hash_add, data);
                    AK_dbg_messg(HIGH, INDICES, "Record deleted from hash index in table block %d and TupleDict ID %d\n", temp_hash_bucket->element[i].add.addBlock, temp_hash_bucket->element[i].add.indexTd);
                    
                    if (main_add != NULL) AK_free(main_add);
                    if (hash_add != NULL) AK_free(hash_add);
                    if (temp_main_bucket != NULL) AK_free(temp_main_bucket);
                    if (temp_hash_bucket != NULL) AK_free(temp_hash_bucket);
                    if (temp_table_block != NULL) AK_free(temp_table_block);
                    return; 
                }
                if (temp_table_block != NULL) AK_free(temp_table_block); 
                temp_table_block = NULL;
            }
        }
    }
    if (main_add != NULL) AK_free(main_add);
    if (hash_add != NULL) AK_free(hash_add);
    if (temp_main_bucket != NULL) AK_free(temp_main_bucket);
    if (temp_hash_bucket != NULL) AK_free(temp_hash_bucket);
    AK_EPI;
}

/**
  * @author Mislav Čakarić, updated by Valentina Kušter
  * @brief Function that creates a hash index
  * @param tblName name of table for which the index is being created
  * @param indexName name of index
  * @param attributes list of attributes over which the index is being created
  * @return success or error

 */
int AK_create_hash_index(char *tblName, struct list_node *attributes, char *indexName)
{
    int i, j, k, l, n, exist, hashValue;
    int num_rows = 0; // Counter for inserted rows (for debug)
    AK_PRO;

    // Retrieve table extent addresses
    table_addresses *addresses = (table_addresses *)AK_get_table_addresses(tblName);
    if (addresses == NULL)
    {
        AK_dbg_messg(LOW, FILE_MAN, "AK_create_hash_index: ERROR: Failed to get table addresses for %s!\n", tblName);
        AK_EPI;
        return EXIT_ERROR;
    }

    // Get number of attributes in the table
    int num_attr = AK_num_attr(tblName);
    if (num_attr <= 0)
    {
        AK_dbg_messg(LOW, FILE_MAN, "AK_create_hash_index: ERROR: Invalid number of attributes for table %s!\n", tblName);
        AK_free(addresses);
        AK_EPI;
        return EXIT_ERROR;
    }

    // Load table header
    AK_header *table_header = (AK_header *)AK_get_header(tblName);
    if (table_header == NULL)
    {
        AK_dbg_messg(LOW, FILE_MAN, "AK_create_hash_index: ERROR: Failed to get table header for %s!\n", tblName);
        AK_free(addresses);
        AK_EPI;
        return EXIT_ERROR;
    }

    // Prepare headers for the index attributes
    AK_header i_header[MAX_ATTRIBUTES];
    memset(i_header, 0, sizeof(i_header));
    AK_header *temp;

    // Get the first attribute from the list
    struct list_node *attribute = (struct list_node *)AK_First_L2(attributes);
    if (attribute == NULL)
    {
        AK_dbg_messg(LOW, FILE_MAN, "AK_create_hash_index: ERROR: No attributes provided for index creation!\n");
        AK_free(addresses);
        AK_free(table_header);
        AK_EPI;
        return EXIT_ERROR;
    }

    n = 0; // Index attribute counter
    while (attribute != NULL)
    {
        exist = 0;
        for (i = 0; i < num_attr; i++)
        {
            if (strcmp((table_header + i)->att_name, attribute->data) == 0)
            {
                AK_dbg_messg(HIGH, INDICES, "Attribute %s exist in table, found on position: %d\n", (table_header + i)->att_name, i);
                exist = 1;

                // Only VARCHAR and INT types are allowed
                if ((table_header + i)->type != TYPE_VARCHAR && (table_header + i)->type != TYPE_INT)
                {
                    printf("Unsupported data type for hash index! Only int and varchar!\n");
                    AK_free(addresses);
                    AK_free(table_header);
                    AK_EPI;
                    return EXIT_ERROR;
                }

                if (n >= MAX_ATTRIBUTES)
                {
                    AK_dbg_messg(LOW, FILE_MAN, "AK_create_hash_index: ERROR: Too many attributes for index! Max %d.\n", MAX_ATTRIBUTES);
                    AK_free(addresses);
                    AK_free(table_header);
                    AK_EPI;
                    return EXIT_ERROR;
                }

                // Create and copy the header for the index
                temp = (AK_header *)AK_create_header((table_header + i)->att_name, (table_header + i)->type, FREE_INT, FREE_CHAR, FREE_CHAR);
                if (temp == NULL)
                {
                    AK_dbg_messg(LOW, FILE_MAN, "AK_create_hash_index: ERROR: Failed to create header for attribute %s!\n", (table_header + i)->att_name);
                    AK_free(addresses);
                    AK_free(table_header);
                    AK_EPI;
                    return EXIT_ERROR;
                }
                memcpy(i_header + n, temp, sizeof(AK_header));
                AK_free(temp);
                n++;
                break;
            }
        }
        if (!exist)
        {
            printf("Attribute %s does not exist in the table!\n", attribute->data);
            AK_free(addresses);
            AK_free(table_header);
            AK_EPI;
            return EXIT_ERROR;
        }
        attribute = attribute->next;
    }

    // Create new segment for the hash index
    int startAddress = AK_initialize_new_segment(indexName, SEGMENT_TYPE_INDEX, i_header);
    if (startAddress == EXIT_ERROR)
    {
        AK_dbg_messg(LOW, FILE_MAN, "AK_create_hash_index: ERROR: Failed to initialize new segment for index %s!\n", indexName);
        AK_free(addresses);
        AK_free(table_header);
        AK_EPI;
        return EXIT_ERROR;
    }
    printf("\nINDEX %s CREATED!\n", indexName);

    // Read the first block of the newly created index segment
    AK_block *block = (AK_block *)AK_read_block(startAddress);
    if (block == NULL)
    {
        AK_dbg_messg(LOW, FILE_MAN, "AK_create_hash_index: ERROR: Failed to read initial segment block for %s!\n", indexName);
        AK_free(addresses);
        AK_free(table_header);
        AK_EPI;
        return EXIT_ERROR;
    }

    block->AK_free_space = DATA_BLOCK_SIZE * DATA_ENTRY_SIZE;

    // Initialize hash index metadata
    hash_info *info = (hash_info *)AK_malloc(sizeof(hash_info));
    if (info == NULL)
    {
        AK_dbg_messg(LOW, FILE_MAN, "AK_create_hash_index: ERROR: Failed to allocate hash_info!\n");
        AK_free(addresses);
        AK_free(table_header);
        AK_free(block);
        AK_EPI;
        return EXIT_ERROR;
    }

    info->modulo = 4;
    info->main_bucket_num = 0;
    info->hash_bucket_num = 0;

    // Make sure hash_info fits into the block
    if (sizeof(hash_info) > (DATA_BLOCK_SIZE * DATA_ENTRY_SIZE))
    {
        AK_dbg_messg(LOW, FILE_MAN, "AK_create_hash_index: ERROR: hash_info is too large for block data area (max %d bytes)!\n", (DATA_BLOCK_SIZE * DATA_ENTRY_SIZE));
        AK_free(addresses);
        AK_free(table_header);
        AK_free(block);
        AK_free(info);
        AK_EPI;
        return EXIT_ERROR;
    }

    // Store hash_info in the block
    memcpy(block->data, info, sizeof(hash_info));
    block->tuple_dict[0].address = 0;
    block->tuple_dict[0].type = INFO_BUCKET;
    block->tuple_dict[0].size = sizeof(hash_info);
    block->AK_free_space -= sizeof(hash_info);
    block->last_tuple_dict_id = 1;

    AK_write_block(block);
    AK_free(info);

    // Prepare row list node for storing temporary tuples
    struct list_node *row = (struct list_node *)AK_malloc(sizeof(struct list_node));
    if (row == NULL)
    {
        AK_dbg_messg(LOW, FILE_MAN, "AK_create_hash_index: ERROR: Failed to allocate row list_node!\n");
        AK_free(addresses);
        AK_free(table_header);
        AK_free(block);
        AK_EPI;
        return EXIT_ERROR;
    }
    AK_Init_L3(&row);

    char data_buffer[MAX_VARCHAR_LENGTH + 1];
    memset(data_buffer, 0, sizeof(data_buffer));

    // Loop through all table extents and blocks to process rows
    for (i = 0; addresses->address_from[i]; i++)
    {
        for (j = addresses->address_from[i]; j < addresses->address_to[i]; j++)
        {
            AK_block *temp_block = (AK_block *)AK_read_block(j);
            if (temp_block == NULL)
            {
                AK_dbg_messg(LOW, FILE_MAN, "AK_create_hash_index: ERROR: Failed to read block %d for data population! Skipping block.\n", j);
                continue;
            }

            for (k = 0; k < temp_block->last_tuple_dict_id; k += num_attr)
            {
                if ((k + num_attr) > temp_block->last_tuple_dict_id)
                {
                    AK_dbg_messg(LOW, FILE_MAN, "AK_create_hash_index: ERROR: Partial or malformed row detected in block %d at tuple_dict index %d. Skipping block portion.\n", j, k);
                    break;
                }

                if (temp_block->tuple_dict[k].type == FREE_INT || temp_block->tuple_dict[k].size == 0)
                {
                    continue;
                }

                AK_DeleteAll_L3(&row);

                for (l = 0; l < num_attr; l++)
                {
                    if ((k + l) >= temp_block->last_tuple_dict_id)
                    {
                        AK_dbg_messg(LOW, FILE_MAN, "AK_create_hash_index: ERROR: tuple_dict index out of bounds during attribute read (k=%d, l=%d) for block %d! Skipping row.\n", k, l, j);
                        goto cleanup_row_and_continue_k_loop;
                    }

                    int type = temp_block->tuple_dict[k + l].type;
                    int size = temp_block->tuple_dict[k + l].size;
                    int address_in_block_data = temp_block->tuple_dict[k + l].address;

                    if (address_in_block_data < 0 || (address_in_block_data + size) > (DATA_BLOCK_SIZE * DATA_ENTRY_SIZE))
                    {
                        AK_dbg_messg(LOW, FILE_MAN, "AK_create_hash_index: ERROR: Data address (%d) or size (%d) out of bounds for block %d! Skipping row.\n", address_in_block_data, size, j);
                        goto cleanup_row_and_continue_k_loop;
                    }

                    if (size >= MAX_VARCHAR_LENGTH)
                    {
                        size = MAX_VARCHAR_LENGTH - 1;
                    }

                    memcpy(data_buffer, &(temp_block->data[address_in_block_data]), size);
                    data_buffer[size] = '\0';

                    AK_InsertAtEnd_L3(type, data_buffer, size, row);
                }

                if (l < num_attr)
                {
                    AK_DeleteAll_L3(&row);
                    continue;
                }

                // Compute hash from the indexed attributes
                hashValue = 0;
                struct list_node *current_attribute_for_hash = (struct list_node *)AK_First_L2(attributes);
                if (current_attribute_for_hash == NULL)
                {
                    AK_dbg_messg(LOW, FILE_MAN, "AK_create_hash_index: ERROR: Attributes list became NULL during hash computation!\n");
                    goto cleanup_block_and_break_j_loop;
                }

                int hash_attr_count = 0;
                while (current_attribute_for_hash != NULL)
                {
                    int found_attr_in_header = -1;
                    for (l = 0; l < num_attr; l++)
                    {
                        if (strcmp((table_header + l)->att_name, current_attribute_for_hash->data) == 0)
                        {
                            found_attr_in_header = l;
                            break;
                        }
                    }

                    if (found_attr_in_header == -1)
                    {
                        AK_dbg_messg(LOW, FILE_MAN, "AK_create_hash_index: ERROR: Attribute '%s' from index definition not found in table header during hash! Skipping row.\n", current_attribute_for_hash->data);
                        goto cleanup_row_and_continue_k_loop;
                    }

                    struct list_node *temp_elem = AK_GetNth_L2(found_attr_in_header, row);
                    if (temp_elem == NULL)
                    {
                        AK_dbg_messg(LOW, FILE_MAN, "AK_create_hash_index: ERROR: Failed to get Nth element (%d) from row list for hash! Check row data integrity. Skipping row.\n", found_attr_in_header);
                        goto cleanup_row_and_continue_k_loop;
                    }
                    hashValue += AK_elem_hash_value(temp_elem);
                    current_attribute_for_hash = current_attribute_for_hash->next;
                    hash_attr_count++;
                }

                if (hash_attr_count != n)
                {
                    AK_dbg_messg(LOW, INDICES, "AK_create_hash_index: WARNING: Mismatch in hash attribute count. Expected %d, got %d. Skipping row.\n", n, hash_attr_count);
                    goto cleanup_row_and_continue_k_loop;
                }

                // Insert the row address into the hash index
                num_rows++;
                AK_dbg_messg(HIGH, INDICES, "Insert in hash index %d. record\n", num_rows);

                struct_add *add = (struct_add *)AK_malloc(sizeof(struct_add));
                if (add == NULL)
                {
                    AK_dbg_messg(LOW, FILE_MAN, "AK_create_hash_index: ERROR: Failed to allocate struct_add for hash insertion!\n");
                    goto cleanup_block_and_break_j_loop;
                }
                add->addBlock = j;
                add->indexTd = k;
                AK_insert_in_hash_index(indexName, hashValue, add);
                AK_free(add);

            cleanup_row_and_continue_k_loop:
                AK_DeleteAll_L3(&row);
            }
            AK_free(temp_block);
        }
    }

cleanup_block_and_break_j_loop:
    AK_free(addresses);
    AK_free(table_header);
    AK_free(block);
    AK_free(row);

    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @brief Function that deletes a hash index by name
 * @param indexName name of the hash index to be deleted
 * @return No return value
 */
void AK_delete_hash_index(char *indexName)
{
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
TestResult AK_hash_test()
{
    int passedTest = 0;
    int failedTest = 0;
    char *tblName = "student";
    char *indexName = "student_hash_index";
    // AK_print_table("AK_relation");
    AK_PRO;
    struct list_node *att_list = (struct list_node *)AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&att_list);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "mbr\0", 4, att_list);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname\0", 10, att_list);

    if (AK_create_hash_index(tblName, att_list, indexName) == EXIT_ERROR)
    {
        failedTest++;
    }
    else
    {
        passedTest++;
    }

    struct list_node *values = (struct list_node *)AK_malloc(sizeof(struct list_node));
    struct list_node *row = (struct list_node *)AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&values);

    // AK_delete_hash_index(indexName);

    hash_info *info = (hash_info *)AK_malloc(sizeof(hash_info));
    info = AK_get_hash_info(indexName);
    if (info == NULL)
    {
        failedTest++;
    }
    else
    {
        passedTest++;
    }
    printf("Main buckets:%d, Hash buckets:%d, Modulo:%d\n", info->main_bucket_num, info->hash_bucket_num, info->modulo);

    // AK_delete_hash_index(indexName);
    AK_print_table("AK_relation");
    AK_print_table("AK_index");

    printf("Hash index search test:\n");
    int i, num_rec = AK_get_num_records(tblName);
    for (i = 0; i < num_rec; i++)
    {
        row = AK_get_row(i, tblName);
        struct list_node *value = AK_GetNth_L2(1, row);
        AK_InsertAtEnd_L3(value->type, value->data, value->size, values);
        value = AK_GetNth_L2(1, row);
        AK_InsertAtEnd_L3(value->type, value->data, value->size, values);
        struct_add *add = AK_find_in_hash_index(indexName, values);
        if (add->addBlock == NULL)
        {
            failedTest++;
        }
        else
        {
            passedTest++;
        }
        AK_DeleteAll_L3(&values);
        if (add->addBlock && add->indexTd)
            printf("Record found in table block %d and TupleDict ID %d\n", add->addBlock, add->indexTd);
    }
    printf("hash_test: Present!\n");
    AK_EPI;
    return TEST_result(passedTest, failedTest);
}
