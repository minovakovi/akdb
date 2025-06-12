/**
@file bitmap.c Provides functions for bitmap indexes
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

#include "bitmap.h"
#include "../../auxi/iniparser.h"
#include "../../auxi/constants.h"

/**
  @author Saša Vukšić
  @brief Function that examines whether list L contains operator ele
  @param L list of elements
  @param ele operator to be found in list
  @return 1 if operator ele is found in list, otherwise 0
 */
  int AK_If_ExistOp(struct list_node *L, char *ele)
  {
    struct list_node *Currentelement_op;
    AK_PRO;
    Currentelement_op = L->next;
    while (Currentelement_op)
    {
        if (strcmp(Currentelement_op->attribute_name, ele) == 0)
        {
            AK_EPI;
            return 1;
        }
        Currentelement_op = (struct list_node *) Currentelement_op->next;
    }
    AK_EPI;
    return 0;
}

/**
 * Helper function to handle inserting elements into headerAttributes list
 */
static void AK_insert_header_attribute(struct list_node *headerAtributes, char *value) {
    struct list_node *ee = (struct list_node *) AK_First_L2(headerAtributes);
    if (ee == 0) {
        AK_Insert_New_Element(TYPE_VARCHAR, value, "indexLista", value, headerAtributes);
    }
    else {
        if (AK_If_ExistOp(headerAtributes, value) == 0) {
            ee = (struct list_node *) AK_End_L2(headerAtributes);
            AK_Insert_New_Element(TYPE_VARCHAR, value, "indexLista", value, ee);
        }
    }
}

/**
 * Helper function to create and initialize index headers
 */
static void AK_init_index_headers(AK_header *header, struct list_node *headerAtributes, int type) {
    int brr = 2; // First two places reserved for block and row pointers
    AK_header* tempHeader;
    struct list_node *e;

    // Add block address and index td headers
    tempHeader = (AK_header*) AK_create_header("addBlock", TYPE_INT, FREE_INT, FREE_CHAR, FREE_CHAR);
    memcpy(header, tempHeader, sizeof(AK_header));
    tempHeader = (AK_header*) AK_create_header("indexTd", TYPE_INT, FREE_INT, FREE_CHAR, FREE_CHAR); 
    memcpy(header + 1, tempHeader, sizeof(AK_header));

    // Add attribute headers
    e = (struct list_node *) AK_First_L2(headerAtributes);
    while (e != 0) {
        tempHeader = (AK_header*) AK_create_header(e->attribute_name, type, FREE_INT, FREE_CHAR, FREE_CHAR);
        memcpy(header + brr, tempHeader, sizeof(AK_header));
        e = (struct list_node *) AK_Next_L2(e);
        brr++;
    }

    // Fill remaining headers with null
    for (int z = brr; z < MAX_ATTRIBUTES; z++) {
        memcpy(header + z, "\0", sizeof("\0")); 
    }
}

/**
 * Helper function to create index name
 */
static char* AK_create_index_name(char *tblName, char *attributeName) {
    char inde[50];
    strcpy(inde, tblName);
    char *indexName = strcat(inde, attributeName);
    return strcat(indexName, "_bmapIndex");
}




/**
 * Process tuple data and add to header attributes
 */
static void AK_process_tuple_data(struct list_node *headerAtributes, AK_block *temp, 
                                 int tuple_idx, int tuple_size) {
    if (temp->tuple_dict[tuple_idx].size > 0) {
        char temp_char[MAX_VARCHAR_LENGTH];
                                
        switch (temp->tuple_dict[tuple_idx].type) {
            case TYPE_INT:
                int temp_int;
                memcpy(&temp_int, &(temp->data[temp->tuple_dict[tuple_idx].address]), 
                       temp->tuple_dict[tuple_idx].size);
                sprintf(temp_char, "%d", temp_int);
                AK_insert_header_attribute(headerAtributes, temp_char);
                break;

            case TYPE_VARCHAR:
                memcpy(temp_char, &(temp->data[temp->tuple_dict[tuple_idx].address]),
                       temp->tuple_dict[tuple_idx].size);
                temp_char[temp->tuple_dict[tuple_idx].size] = '\0';
                AK_insert_header_attribute(headerAtributes, temp_char);
                break;

            case TYPE_FLOAT:
                break; // FLOAT values are not indexed
        }
    }
}

/**
 * Process a single block's tuples
 */
static void AK_process_block_tuples(AK_block *block, struct list_node *headerAtributes, 
                                  int attr_position, int num_attr) {
    for (int k = attr_position; k < DATA_BLOCK_SIZE; k += num_attr) {
        AK_process_tuple_data(headerAtributes, block, k, num_attr);
    }
}

/**
 * Process all blocks in an address range
 */
static void AK_process_address_range(int from_addr, int to_addr, 
                                   struct list_node *headerAtributes,
                                   int attr_position, int num_attr) {
    for (int addr = from_addr; addr < to_addr; addr++) {
        AK_block *block = (AK_block*) AK_read_block(addr);
        AK_process_block_tuples(block, headerAtributes, attr_position, num_attr);
    }
}

/**
 * Process blocks in table and collect header attributes
 */
static void AK_process_table_blocks(table_addresses *addresses, struct list_node *headerAtributes,
                                  int attr_position, int num_attr) {
    int segment = 0;
    while (addresses->address_from[segment] != 0) {
        AK_process_address_range(
            addresses->address_from[segment],
            addresses->address_to[segment],
            headerAtributes,
            attr_position,
            num_attr
        );
        segment++;
    }
}

/**
 * Create and initialize index for an attribute
 */
static void AK_create_attribute_index(char *tblName, AK_header *temp_head, int attr_position,
                                    int num_attr, struct list_node *headerAtributes) {
    char *indexName;
    int startAddress;

    switch ((temp_head + attr_position)->type) {
        case TYPE_VARCHAR: {
            AK_header t_header[MAX_ATTRIBUTES];
            AK_init_index_headers(t_header, headerAtributes, TYPE_VARCHAR);
            indexName = AK_create_index_name(tblName, (temp_head + attr_position)->att_name);
            startAddress = AK_initialize_new_index_segment(indexName, 
                            AK_get_table_id(tblName), 
                            attr_position, 
                            t_header);
            
            if (startAddress != EXIT_ERROR) {
                printf("\nINDEX %s CREATED!\n", indexName);
                AK_create_Index(tblName, indexName, (temp_head + attr_position)->att_name, 
                              attr_position, num_attr, t_header);
            }
            break;
        }

        case TYPE_INT: {
            AK_header t_headerr[MAX_ATTRIBUTES];
            AK_init_index_headers(t_headerr, headerAtributes, TYPE_INT);
            indexName = AK_create_index_name(tblName, (temp_head + attr_position)->att_name);
            startAddress = AK_initialize_new_index_segment(indexName,
                            AK_get_table_id(tblName),
                            attr_position,
                            t_headerr);
                            
            if (startAddress != EXIT_ERROR) {
                printf("\nINDEX %s CREATED!\n", indexName);
                AK_create_Index(tblName, indexName, (temp_head + attr_position)->att_name,
                              attr_position, num_attr, t_headerr);
            }
            break;
        }
    }
}


/**
 * Find matching attribute and create index for it
 */
static void AK_process_matching_attribute(char *tblName, AK_header *temp_head,
                                        struct list_node *headerAtributes,
                                        table_addresses *addresses,
                                        struct list_node *attribute,
                                        int attr_idx, int num_attr) {
    if (strcmp((temp_head + attr_idx)->att_name, attribute->attribute_name) == 0) {
        AK_Init_L3(&headerAtributes);
        AK_process_table_blocks(addresses, headerAtributes, attr_idx, num_attr);
        AK_create_attribute_index(tblName, temp_head, attr_idx, num_attr, headerAtributes);
    }
}


/**
 * @author Saša Vukšić, Lovro Predovan, refactored and split into smaller functions by Matija Karaula
 * @brief Function that reads table on which we create index and call functions for creating index
           Elements that will be in index are put in list indexLista and headerAttributes. According to those elements new indexes
           are created.

 * @param tblName name of table
 * @param attributes list of attributes on which we will create indexes
 * @return No return value
 * */
void AK_create_Index_Table(char *tblName, struct list_node *attributes) {
    AK_PRO;
    
    // Initialize main variables
    table_addresses *addresses = (table_addresses*) AK_get_table_addresses(tblName);
    int num_attr = AK_num_attr(tblName);
    AK_header *temp_head = (AK_header *) AK_get_header(tblName);
    struct list_node *headerAtributes = (struct list_node *) AK_malloc(sizeof(struct list_node));
    
    // Process each attribute
    for (int i = 0; i < num_attr; i++) {
        struct list_node *curr_attr = AK_First_L2(attributes);
        while (curr_attr != 0) {
            AK_process_matching_attribute(tblName, temp_head, headerAtributes, 
                                       addresses, curr_attr, i, num_attr);
            curr_attr = (struct list_node *) AK_Next_L2(curr_attr);
        }
    }

    // Cleanup
    AK_DeleteAll_L3(&headerAtributes);
    AK_free(headerAtributes);
    AK_free(temp_head);
    AK_EPI;
}



/**
 * Helper function to process a tuple and create a new row
 */
static struct list_node* AK_create_index_row(AK_block *temp, int k, char *tblNameIndex, 
                                           int temp_indexTd, char *value) {
    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row_root);
    
    // Insert block address
    AK_Insert_New_Element(TYPE_INT, &(temp->address), tblNameIndex, "addBlock", row_root);
    
    // Insert index position
    AK_Insert_New_Element(TYPE_INT, &temp_indexTd, tblNameIndex, "indexTd", row_root);
    
    // Insert value
    AK_Insert_New_Element(TYPE_VARCHAR, value, tblNameIndex, value, row_root);
    
    return row_root;
}

/**
 * Helper function to process an INT type tuple
 */
static void AK_process_int_tuple(AK_block *temp, int k, char *tblNameIndex, int positionTbl) {
    int temp_int;
    char temp_char[MAX_VARCHAR_LENGTH];
    
    memcpy(&temp_int, &(temp->data[temp->tuple_dict[k].address]), 
           temp->tuple_dict[k].size);
    temp_int = sprintf(temp_char, "%d", temp_int);
    
    int temp_indexTd = k - positionTbl;
    struct list_node *row_root = AK_create_index_row(temp, k, tblNameIndex, 
                                                    temp_indexTd, "1");
    
    AK_insert_row(row_root);
    AK_DeleteAll_L3(&row_root);
    AK_free(row_root);
}

/**
 * Helper function to process a VARCHAR type tuple
 */
static void AK_process_varchar_tuple(AK_block *temp, int k, char *tblNameIndex, int positionTbl) {
    char temp_char[MAX_VARCHAR_LENGTH];
    
    memcpy(temp_char, &(temp->data[temp->tuple_dict[k].address]),
           temp->tuple_dict[k].size);
    temp_char[temp->tuple_dict[k].size] = '\0';
    
    int temp_indexTd = k - positionTbl;
    struct list_node *row_root = AK_create_index_row(temp, k, tblNameIndex, 
                                                    temp_indexTd, "1");
    
    AK_insert_row(row_root);
    AK_DeleteAll_L3(&row_root);
    AK_free(row_root);
}

/**
 * Process a single block's tuples for indexing
 */
static void AK_process_block_for_index(AK_block *temp, char *tblNameIndex, 
                                     int positionTbl, int numAtributes) {
    for (int k = positionTbl; k < DATA_BLOCK_SIZE; k = k + numAtributes) {
        if (temp->tuple_dict[k].size > 0) {
            switch (temp->tuple_dict[k].type) {
                case TYPE_INT:
                    AK_process_int_tuple(temp, k, tblNameIndex, positionTbl);
                    break;
                    
                case TYPE_VARCHAR:
                    AK_process_varchar_tuple(temp, k, tblNameIndex, positionTbl);
                    break;
                    
                case TYPE_FLOAT:
                    break; // FLOAT values are not indexed
            }
        }
    }
}

/**
 * @author Saša Vukšić, Lovro Predovan, refactored and divided into smaller functions by Matija Karaula
 * @brief Function that loads index table with the value of particulary atribute
 * @param tblName source table
 * @param tblNameIndex new name of index table
 * @param attributeName attribute on which we make index
 * @param positionTbl position of attribute in header of table
 * @param numAtributes number of attributes in table
 * @param headerIndex header of index table
 * @return No return value
 */
void AK_create_Index(char *tblName, char *tblNameIndex, char *attributeName, 
                    int positionTbl, int numAtributes, AK_header *headerIndex) {
    AK_PRO;
    
    table_addresses *addresses = (table_addresses*) AK_get_table_addresses(tblName);
    AK_block *temp = (AK_block*) AK_read_block(addresses->address_from[0]);
    
    int i = 0;
    while (addresses->address_from[i] != 0) {
        for (int j = addresses->address_from[i]; j < addresses->address_to[i]; j++) {
            AK_block *temp = (AK_block*) AK_read_block(j);
            AK_process_block_for_index(temp, tblNameIndex, positionTbl, numAtributes);
        }
        i++;
    }
    
    AK_free(addresses);
    AK_free(temp);
    AK_EPI;
}

/**
 * @author Saša Vukšić, Lovro Predovan, commented by Matija Karaula
 * @brief Function that retrieves addresses of a particular attribute from a bitmap index
 * @param indexName Name of the bitmap index to search
 * @param attribute Name of the attribute to find
 * @return List of addresses where the attribute is found
 */
list_ad* AK_get_attribute(char *indexName, char *attribute) {
    // Initialize variables for traversing the index
    int num_attr;                          // Number of attributes in index
    int i, j, k;                          // Loop counters
    table_addresses *addresses;            // Holds segment addresses
    AK_header *temp_head;                 // Table header
    int temp_int;                         // Temporary storage for INT values
    char temp_char[MAX_VARCHAR_LENGTH];    // Temporary storage for VARCHAR values
    float temp_float;                     // Temporary storage for FLOAT values
    int b, br = 0;                        // Position counters
    int addBlock, indexTd;                // Block and tuple addresses
    list_ad *add_root;                    // Result list to store found addresses
    
    AK_PRO;

    // Get index metadata
    addresses = (table_addresses*) AK_get_index_addresses(indexName);
    num_attr = AK_num_index_attr(indexName);
    temp_head = (AK_header *) AK_get_index_header(indexName);

    // Initialize result list
    add_root = (list_ad *) AK_malloc(sizeof(list_ad));
    AK_InitializelistAd(add_root);

    // Find position of target attribute in header
    for (b = 0; b < num_attr; b++) {
        if (strcmp((temp_head + b)->att_name, attribute) == 0) {
            br = b;  // Store attribute position
        }
    }
    AK_free(temp_head);
    
    // Iterate through all segments
    i = 0;
    while (addresses->address_from[i] != 0) {
        // Iterate through blocks in current segment
        for (j = addresses->address_from[i]; j < addresses->address_to[i]; j++) {
            AK_block *temp = (AK_block*) AK_read_block(j);
            
            // Iterate through tuples in current block
            for (k = br; k < DATA_BLOCK_SIZE; k = k + num_attr) {
                if (temp->tuple_dict[k].size > 0) {
                    // Process tuple based on its type
                    switch (temp->tuple_dict[k].type) {
                        case TYPE_INT:
                            memcpy(&temp_int, &(temp->data[temp->tuple_dict[k].address]),
                                   temp->tuple_dict[k].size);
                            break;
                            
                        case TYPE_FLOAT:
                            memcpy(&temp_float, &(temp->data[temp->tuple_dict[k].address]),
                                   temp->tuple_dict[k].size);
                            printf("float: %-10.2f", temp_float);
                            break;

                        case TYPE_VARCHAR:
                            // Get the value
                            memcpy(temp_char, &(temp->data[temp->tuple_dict[k].address]),
                                   temp->tuple_dict[k].size);
                            
                            // Get block address
                            memcpy(&addBlock, &(temp->data[temp->tuple_dict[k - br].address]),
                                   temp->tuple_dict[0].size);
                            
                            // Get tuple index
                            memcpy(&indexTd, &(temp->data[temp->tuple_dict[k - (br - 1)].address]),
                                   temp->tuple_dict[1].size);

                            temp_char[temp->tuple_dict[k].size] = '\0';

                            // If value is "1", add address to result list
                            if (strcmp(temp_char, "1") == 0) {
                                AK_Insert_NewelementAd(addBlock, indexTd, attribute, add_root);
                            }
                            break;
                    }
                    
                    // Print newline after processing all attributes
                    if ((k + 1) % num_attr == 0) {
                        printf("\n");
                    }
                }
            }
        }
        i++;
    }
    
    AK_EPI;
    AK_free(add_root);
    return add_root;
}


/**
 * @author Saša Vukšić, Lovro Predovan
 * @brief Function that prints the list of adresses
 * @param list list of adresses
 * @return No return value
 **/
 void AK_print_Att_Test(list_ad *list)
 {
    element_ad ele;
    AK_PRO;
    ele = AK_Get_First_elementAd(list);

    while (ele != 0)
    {
        printf("Attribute : %s Block address: %i Index position: %i\n",ele->attName,ele->add.addBlock,ele->add.indexTd);
        ele = AK_Get_Next_elementAd(ele);
    }
    AK_EPI;
}

/**
 * @author Saša Vukšić, renamed by Matija Karaula
 * @brief Function that fetches the values from the bitmap index if there is one for a given table.
 * It should be started when we are making selection on the table with bitmap index.
 * @param tableName name of table
 * @param attributeValue value of attribute
 * @return list of adresses
 **/
 list_ad* AK_get_Attribute_values(char *tableName, char *attributeName, char *attributeValue)
 {
    list_ad *list;
    char inde[50];
    char *indexName;
    AK_PRO;

    list = (list_ad *) AK_malloc(sizeof (list_ad));
    AK_InitializelistAd(list);
    strcpy(inde, tableName);
    indexName = strcat(inde, attributeName);
    indexName = strcat(indexName, "_bmapIndex");

    table_addresses *addresses = (table_addresses*) AK_get_index_addresses(indexName);
    if (addresses->address_from[ 0 ] == 0)
    {
        printf("There is no index for table: %s on attribute: %s", tableName, attributeName);
    }
    else
    {
        list = AK_get_attribute(indexName, attributeValue);
    }
    AK_EPI;
    return list;
}


/**
 * Helper function to create index name
 */
static char* AK_create_bitmap_index_name(char *tableName, char *attributeName) {
    char *indexName = (char*)AK_malloc(50);
    strcpy(indexName, tableName);
    strcat(indexName, attributeName);
    strcat(indexName, "_bmapIndex");
    return indexName;
}

/**
 * Find position of attribute in header
 */
static int AK_find_attribute_position(AK_header *temp_head, char *attributeValue, int num_attr) {
    for (int i = 0; i < num_attr; i++) {
        if (strcmp((temp_head + i)->att_name, attributeValue) == 0) {
            return i;
        }
    }
    return -1;
}

/**
 * Validate update operation is possible
 */
static int AK_validate_update(char *tableName, char *attributeName, 
                            char *attributeValue, char *indexName) {
    table_addresses *addresses = (table_addresses*) AK_get_index_addresses(indexName);
    if (addresses->address_from[0] == 0) {
        printf("There is no index for table : %s on attribute: %s", tableName, attributeName);
        return 0;
    }
    return 1;
}

/**
 * Update tuple values in block
 */
static void AK_update_tuple_values(AK_block *temp, int k, int pos, int posNew) {
    memcpy(&(temp->data[temp->tuple_dict[k + pos].address]), "n", 1);
    memset(&(temp->data[temp->tuple_dict[k + posNew].address]), 0, 4);
    memcpy(&(temp->data[temp->tuple_dict[k + posNew].address]), "1", 1);
    AK_write_block(temp);
}

/**
 * Process single block for update
 */
static void AK_process_block_update(AK_block *temp, int addBlock, int addTd, 
                                  int pos, int posNew, int num_attr) {
    for (int k = 0; k < DATA_BLOCK_SIZE; k += num_attr) {
        if (temp->tuple_dict[k].size > 0) {
            int temp_adr_block, temp_adr_Td;
            
            memcpy(&temp_adr_block, &(temp->data[temp->tuple_dict[k].address]), 
                   temp->tuple_dict[k].size);
            memcpy(&temp_adr_Td, &(temp->data[temp->tuple_dict[k + 1].address]), 
                   temp->tuple_dict[k + 1].size);

            if ((temp_adr_block == addBlock) && (temp_adr_Td == addTd)) {
                printf("Block address: %d, Td address: %d segment:%i\n", 
                       temp_adr_block, temp_adr_Td, temp->address);
                AK_update_tuple_values(temp, k, pos, posNew);
            }
        }
    }
}

/**
 * Process address range for update
 */
static void AK_process_address_range_update(int from_addr, int to_addr,
                                          int addBlock, int addTd,
                                          int pos, int posNew, int num_attr) {
    for (int j = from_addr; j < to_addr; j++) {
        AK_block *temp = (AK_block*) AK_read_block(j);
        AK_process_block_update(temp, addBlock, addTd, pos, posNew, num_attr);
    }
}


/**
 * @author Saša Vukšić
 * @brief Function that updates the index, only on values that alredy exist. 
 * If there is no value in bitmap index or bitmap index on this value, warning is showed to the user. Otherwise, bitmap index is updated with new attribute value.
 * @param addBlock adress of block
 * @param addTD adress of tuple dict
 * @param tableName name of table
 * @param attributeName name of attribute
 * @param attributeValue value of atribute
 * @param newAttributeValue new value of updated attribute
 * @return No return value
 **/
void AK_update(int addBlock, int addTd, char *tableName, char *attributeName, 
               char *attributeValue, char *newAttributeValue) {
    AK_PRO;
    
    // Create index name
    char *indexName = AK_create_bitmap_index_name(tableName, attributeName);
    
    // Get index metadata
    AK_header *temp_head = AK_get_index_header(indexName);
    int num_attr = AK_num_index_attr(indexName);
    
    // Find attribute positions
    int pos = AK_find_attribute_position(temp_head, attributeValue, num_attr);
    int posNew = AK_find_attribute_position(temp_head, newAttributeValue, num_attr);
    
    // Validate update operation
    if (posNew == -1) {
        printf("Update is possible ONLY for existing values!\n");
        AK_EPI;
        AK_free(indexName);
        return;
    }
    
    if (!AK_validate_update(tableName, attributeName, attributeValue, indexName)) {
        AK_EPI;
        AK_free(indexName);
        return;
    }
    
    // Process updates
    table_addresses *addresses = (table_addresses*) AK_get_index_addresses(indexName);
    int segment = 0;
    
    while (addresses->address_from[segment] != 0) {
        AK_process_address_range_update(
            addresses->address_from[segment],
            addresses->address_to[segment],
            addBlock, addTd, pos, posNew, num_attr
        );
        segment++;
    }
    
    printf("Index %s updated for %s table\n", indexName, tableName);
    
    // Cleanup
    AK_free(indexName);
    AK_EPI;
}


/**
 * Helper function to get the number of new records to index
 */
static int AK_get_new_records_count(char *tableName, char *indexName) {
    int tbl_num_rows = AK_get_num_records(tableName);
    int idx_num_rows = AK_get_index_num_records(indexName);
    return tbl_num_rows - idx_num_rows;
}

/**
 * Helper function to get the starting position for new records
 */
static int AK_get_index_start_position(char *tableName, char *indexName) {
    return AK_get_index_num_records(indexName);
}

/**
 * @author Matija Karaula 
 * @brief Function that updates bitmap index with new records
 * @param tableName name of table
 * @param attributeName name of attribute to index
 * @return No return value
 */
void AK_add_to_bitmap_index(char *tableName, char *attributeName) {
    AK_PRO;
    
    // Create index name
    char *tblNameIndex = AK_create_bitmap_index_name(tableName, attributeName);
    
    // Check if we need to update
    int new_records = AK_get_new_records_count(tableName, tblNameIndex);
    if (new_records <= 0) {
        printf("Index %s is up to date for %s table\n", tblNameIndex, tableName);
        AK_free(tblNameIndex);
        AK_EPI;
        return;
    }
    
    // Get table metadata
    table_addresses *addresses = (table_addresses*) AK_get_table_addresses(tableName);
    int num_attr = AK_num_attr(tableName);
    AK_header *temp_head = (AK_header *) AK_get_header(tableName);
    
    // Find attribute position
    int attr_position = -1;
    for (int i = 0; i < num_attr; i++) {
        if (strcmp((temp_head + i)->att_name, attributeName) == 0) {
            attr_position = i;
            break;
        }
    }
    
    if (attr_position == -1) {
        printf("Attribute %s not found in table %s\n", attributeName, tableName);
        AK_free(tblNameIndex);
        AK_free(temp_head);
        AK_EPI;
        return;
    }
    
    // Get starting position for new records
    int start_pos = AK_get_index_start_position(tableName, tblNameIndex);
    int processed = 0;
    int segment = 0;
    
    // Process only new records
    while (addresses->address_from[segment] != 0 && processed < new_records) {
        AK_block *block = (AK_block*) AK_read_block(addresses->address_from[segment]);
        
        // Skip already indexed records
        int k = attr_position + (start_pos * num_attr);
        
        // Process new records in block
        while (k < DATA_BLOCK_SIZE && processed < new_records) {
            if (block->tuple_dict[k].size > 0) {
                switch (block->tuple_dict[k].type) {
                    case TYPE_INT:
                        AK_process_int_tuple(block, k, tblNameIndex, attr_position);
                        break;
                        
                    case TYPE_VARCHAR:
                        AK_process_varchar_tuple(block, k, tblNameIndex, attr_position);
                        break;
                }
                processed++;
            }
            k += num_attr;
        }
        segment++;
    }
    
    printf("Index %s updated with %d new records for %s table\n", 
           tblNameIndex, processed, tableName);
    
    // Cleanup
    AK_free(tblNameIndex);
    AK_free(temp_head);
    AK_EPI;
}

/**
 * @author Saša Vukšić
 * @brief Function that tests printing header of table
 * @param tblName name of table who's header we are printing
   @return No return value
 **/
   void AK_print_Header_Test(char* tblName)
   {
    AK_header *temp_head;
    int i;
    int num_attr;
    AK_PRO;
    temp_head = AK_get_header(tblName);
    num_attr = AK_num_attr(tblName);
    printf("Number of attributes in header: %d", num_attr);
    printf("\n");
    for (i = 0; i < num_attr; i++)
        printf("%-10s", (temp_head + i)->att_name);
    printf("\n----------------------------------------------\n");
    AK_free(temp_head);
    AK_EPI;
}

/**
 * @author Lovro Predovan
 * @brief Function that deletes bitmap index based on the name of index
 * @param Bitmap index table name
   @return No return value
 **/
   void AK_delete_bitmap_index(char *indexName) {
    AK_PRO;
    AK_delete_segment(indexName, SEGMENT_TYPE_INDEX);
    printf("INDEX %s DELETED!\n", indexName);
    AK_EPI;
}



/**
 * @author Saša Vukšić updated by Lovro Predovan
 * @brief Function that creates test table and makes index on test table,
    also prints original tables indexes tables and indexes, tests updating into tables
 * @return No return value
 * */
 TestResult AK_bitmap_test()
 {
    int id_prof;
    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));


    AK_PRO;
    printf("\n********** BITMAP INDEX TEST **********\n\n");

    //INDEXED TABLE
    char *tblName = "assistant";

    //CREATING BITMAP INDEXES ON ATTRIBUTES FIRSTNAME AND TEL
    struct list_node *att_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&att_root);
    //Error: argument of type "const char *" is incompatible with parameter of type "void *"
    AK_Insert_New_Element(TYPE_VARCHAR, "firstname", tblName, "firstname", att_root);
    

    struct list_node *some_element;
    some_element = (struct list_node *) AK_First_L2(att_root);
    //Error: argument of type "const char *" is incompatible with parameter of type "void *"
    AK_Insert_New_Element(TYPE_VARCHAR, "tel", tblName, "tel", some_element);
    AK_create_Index_Table(tblName, att_root);


    AK_print_table(tblName);
    AK_print_table("AK_index");

    printf("\n************Index tables******************\n\n");
    AK_print_index_table("assistantfirstname_bmapIndex");
    AK_print_index_table("assistanttel_bmapIndex");

    printf("Bitmap index tables should be now visible\n");


    //Test value that doesn't passes
    printf("\n\n************Printing non existing index table******************\n\n");
    AK_print_index_table("assistanrandom_bmapIndex");
    printf("\n\n***************************************\n\n");


    printf("\n\n************Updating table indexes******************\n\n");
    AK_Init_L3(&row_root);
    id_prof = 35890;
    AK_DeleteAll_L3(&row_root);
    AK_Insert_New_Element(TYPE_INT, &id_prof, tblName, "id_prof", row_root);
    //Error: argument of type "const char *" is incompatible with parameter of type "void *"
    AK_Insert_New_Element(TYPE_VARCHAR, "Miran", tblName, "firstname", row_root);
        //Error: argument of type "const char *" is incompatible with parameter of type "void *"
    AK_Insert_New_Element(TYPE_VARCHAR, "Zlatović", tblName, "lastname", row_root);
        //Error: argument of type "const char *" is incompatible with parameter of type "void *"
    AK_Insert_New_Element(TYPE_INT, "042390858", tblName, "tel", row_root);
        //Error: argument of type "const char *" is incompatible with parameter of type "void *"
    AK_Insert_New_Element(TYPE_VARCHAR, "miran.zlatovic@foi.hr", tblName, "email", row_root);
        //Error: argument of type "const char *" is incompatible with parameter of type "void *"
    AK_Insert_New_Element(TYPE_VARCHAR, "www.foi.hr/nastavnici/zlatovic.miran/index.html", tblName, "web_page", row_root);
    AK_insert_row(row_root);
    AK_DeleteAll_L3(&row_root);
    AK_free(row_root);

    AK_add_to_bitmap_index(tblName, "firstname");

    AK_print_table(tblName);
    AK_print_index_table("assistantfirstname_bmapIndex");
    
    printf("\n\n************Searching index locations******************\n\n");
    AK_print_Att_Test(AK_get_Attribute_values(tblName,"firstname","Markus"));
    AK_print_Att_Test(AK_get_Attribute_values(tblName,"tel","858928176"));
    
     printf("\n\n************Deleting bitmap index******************\n\n");
    AK_delete_bitmap_index("assistantfirstname_bmapIndex");

    AK_print_table("AK_index");


    printf("\n\n********** BITMAP INDEX TEST END**********\n\n");
    printf("\n\n BITMAP INDEX TEST PASSED\n\n");
    AK_EPI;
    return TEST_result(0,0);
}
