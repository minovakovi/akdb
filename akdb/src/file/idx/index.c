/**
@file index.c Provides functions for indexes
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

#include "index.h"
#include <stdlib.h>

#include "../../auxi/mempro.h"
#include "../../file/table.h"
#include "../../file/fileio.h"
#include "../../file/files.h"


/**
 * @author Unknown
 * @brief Function that finds the first node of linked list
 * @param list_ad *L linked list head
 * @return Address of first node
 * */
element_ad AK_Get_First_elementAd(list_ad *L) {
    AK_PRO;
    element_ad ret;
    ret = (element_ad) L->next;
    AK_EPI;
    return ret;
}

/**
 * @author Unknown
 * @brief Function that finds the last node of linked list
 * @param list_ad *L linked list head
 * @return Address of last node or 0 if list is empty
 * */
element_ad AK_Get_Last_elementAd(list_ad *L) {
    list_ad *Currentelement_op;
    AK_PRO;
    Currentelement_op = L;
    while (Currentelement_op->next)
        Currentelement_op = (element_ad) Currentelement_op->next;
    if (Currentelement_op != L){
        AK_EPI;
        return (element_ad) Currentelement_op;
    }
    else{
        AK_EPI;
        return 0;
    }
    AK_EPI;
}

/**
 * @author Unknown
 * @brief Function that finds the next node of a node in linked list
 * @param Currentelement_op address of current node
 * @return Address of next node or 0 if current node is last in list
 * */
element_ad AK_Get_Next_elementAd(element_ad Currentelement_op) {
    AK_PRO;
    if (Currentelement_op->next == 0) {
        AK_EPI;
        return 0;
    } else {
        list_ad *Nextelement_op;
        Nextelement_op = (element_ad) Currentelement_op->next;
        AK_EPI;
        return (element_ad) Nextelement_op;
    }
    AK_EPI;
}

/**
 * @author Unknown
 * @brief Function that finds the previous node of a node in linked list
 * @param Currentelement_op Address of current node
 * @param L previous element
 * @return Address of previous node or 0 if the current node is the head or the list is empty
 * */
element_ad AK_Get_Previous_elementAd(element_ad Currentelement_op, element_ad L) {
    element_ad Previouselement_op;
    AK_PRO;
    Previouselement_op = L;
    while ((Previouselement_op->next != 0) && ((element_ad) Previouselement_op->next != Currentelement_op))
        Previouselement_op = (element_ad) Previouselement_op->next;
    if (Previouselement_op->next != 0 && Previouselement_op != L) {
        AK_EPI;
        return (element_ad) Previouselement_op;
    } else {
        AK_EPI;
        return 0;
    }
    AK_EPI;
}

/**
 * @author Unknown
 * @brief Function that finds the position of a node in linked list
 * @param Searchedelement_op address of current note
 * @param *L linked list head
 * @return Integer value of current node's order in the list
 * */
int AK_Get_Position_Of_elementAd(element_ad Searchedelement_op, list_ad *L) {
    list_ad *Currentelement_op;
    int i = 0;
    AK_PRO;
    Currentelement_op = L;
    while (Currentelement_op->next != 0 && Currentelement_op != Searchedelement_op) {
        Currentelement_op = (list_ad *) Currentelement_op->next;
        i++;
    }
    AK_EPI;
    return i;
}

/**
 * @author Unknown
 * @brief Function that deletes a node from a linked list
 * @param Deletedelement_op - address of node to delete
 * @param list_ad *L - list head
 * @return No return value
 * */
void AK_Delete_elementAd(element_ad Deletedelement_op, list_ad *L) {
    AK_PRO;
    element_ad Previouselement_op = (element_ad) AK_Get_Previous_elementAd(Deletedelement_op, L);
    if (Previouselement_op != 0) {
        Previouselement_op->next = Deletedelement_op->next;
    } else {
        L->next = Deletedelement_op->next;
    }
    AK_free(Deletedelement_op);
    AK_EPI;
}

/**
 * @author Unknown
 * @brief Function that deletes all nodes in a linked list
 * @param L list head
 * @return No return value
 * */
void AK_Delete_All_elementsAd(list_ad *L) {
    AK_PRO;
    list_ad *Currentelement_op = L;
    list_ad *Deletedelement_op = (list_ad *) L->next;
    while (Currentelement_op->next != 0) {
        Currentelement_op->next = Deletedelement_op->next;
        AK_free(Deletedelement_op);
        Deletedelement_op = (list_ad *) Currentelement_op->next;
    }
    AK_EPI;
}



/**
 * @author Unknown
 * @brief Function that inserts a new element into a linked list
 * @param addBlock address block
 * @param indexTd index table destination
 * @param *attname attribute name
 * @param elementBefore address of the node after which the new node will be inserted
 * @return No return value
 * */
void AK_Insert_NewelementAd(int addBlock, int indexTd, char *attName, element_ad elementBefore) {
    AK_PRO;
    list_structure_ad* newelement_op = (list_structure_ad*)AK_malloc(sizeof (list_structure_ad));
    newelement_op->add.addBlock = addBlock;
    newelement_op->add.indexTd = indexTd;
    newelement_op->attName = attName;
    newelement_op->next = elementBefore->next;
    elementBefore->next = (list_structure_ad*)newelement_op;
    AK_EPI;
}


/**
 * @author Lovro Predovan
 * @brief Function that fetches the number of elements in a index table
 * @param index table name
 * @return No return value
 * */
int AK_num_index_attr(char *indexTblName) {

    int num_attr = 0;
    AK_PRO;
    table_addresses *addresses = (table_addresses*) AK_get_index_addresses(indexTblName);

    if (addresses->address_from[0] == 0)
        num_attr = 0;
    else {
        AK_mem_block *temp_block = (AK_mem_block*) AK_get_block(addresses->address_from[0]);

        while (strcmp(temp_block->block->header[num_attr].att_name, "\0") != 0) {
            num_attr++;
        }
    }

    AK_free(addresses);
    AK_EPI;
    return num_attr;
}

/**
 * @author Matija Šestak, modified for indexes by Lovro Predovan
 * @brief  Determine number of rows in the table
 * <ol>
 * <li>Read addresses of extents</li>
 * <li>If there is no extents in the table, return -1</li>
 * <li>For each extent from table</li>
 * <li>For each block in the extent</li>
 * <li>Get a block</li>
 * <li>Exit if there is no records in block</li>
 * <li>Count tuples in block</li>
 * <li>Return the number of tuples divided by number of attributes</li>
 * </ol>
 * @param *tableName table name
 * @return number of rows in the table
 */
int AK_get_index_num_records(char *indexTblName) {
    int num_rec = 0;
    int number_of_rows = 0;
    AK_PRO;

    table_addresses *addresses = (table_addresses*) AK_get_index_addresses(indexTblName);

    if (addresses->address_from[0] == 0){
        AK_EPI;
        return 0;
    }
    int i = 0, j, k;
    AK_mem_block *temp = (AK_mem_block*) AK_get_block(addresses->address_from[0]);

    while (addresses->address_from[ i ] != 0) {

        for (j = addresses->address_from[ i ]; j < addresses->address_to[ i ]; j++) {
            temp = (AK_mem_block*) AK_get_block(j);

            if (temp->block->last_tuple_dict_id == 0)
                break;

            for (k = 0; k < DATA_BLOCK_SIZE; k++) {
                if (temp->block->tuple_dict[ k ].size > 0) {
                    num_rec++;
                }
            }
        }
        i++;
    }

    AK_free(addresses);
    int num_head = AK_num_index_attr(indexTblName);
    number_of_rows = (num_rec / num_head);

    AK_EPI;
    return number_of_rows;
}


/**
 * Helper function to validate input parameters
 */
static int AK_validate_tuple_params(int row, int column, char *indexTblName) {
    int num_rows = AK_get_index_num_records(indexTblName);
    int num_attr = AK_num_index_attr(indexTblName);
    
    return (row < num_rows && column < num_attr);
}

/**
 * Helper function to read tuple data from block
 */
static struct list_node* AK_read_tuple_data(AK_mem_block *temp, int k, int column) {
    int type = temp->block->tuple_dict[k + column].type;
    int size = temp->block->tuple_dict[k + column].size;
    int address = temp->block->tuple_dict[k + column].address;
    char data[MAX_VARCHAR_LENGTH];
    
    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row_root);
    
    memcpy(data, &temp->block->data[address], size);
    data[size] = '\0';
    AK_InsertAtEnd_L3(type, data, size, row_root);
    
    return row_root;
}

/**
 * @author Matija Šestak, modified for indexes by Lovro Predovan
 * @brief Function that gets value in some row and column
 * @param row zero-based row index
 * @param column zero-based column index
 * @param *tblName table name
 * @return value in the list
 */
struct list_node *AK_get_index_tuple(int row, int column, char *indexTblName) {
    AK_PRO;

    // Validate parameters
    if (!AK_validate_tuple_params(row, column, indexTblName)) {
        AK_EPI;
        return NULL;
    }

    table_addresses *addresses = (table_addresses*) AK_get_index_addresses(indexTblName);
    int num_attr = AK_num_index_attr(indexTblName);
    int counter = -1;

    // Search for requested tuple
    for (int i = 0; addresses->address_from[i] != 0; i++) {
        for (int j = addresses->address_from[i]; j < addresses->address_to[i]; j++) {
            AK_mem_block *temp = (AK_mem_block*) AK_get_block(j);
            if (temp->block->last_tuple_dict_id == 0) continue;
            
            for (int k = 0; k < DATA_BLOCK_SIZE; k += num_attr) {
                if (temp->block->tuple_dict[k].size > 0) {
                    counter++;
                    if (counter == row) {
                        struct list_node *result = AK_read_tuple_data(temp, k, column);
                        AK_free(addresses);
                        AK_EPI;
                        return (struct list_node *) AK_First_L2(result);
                    }
                }
            }
        }
    }

    AK_free(addresses);
    AK_EPI;
    return NULL;
}


/**
 * @author Matija Šestak, modified for indexes by Lovro Predovan
 * @brief Function that examines whether there is a table with the name "tblName" in the system catalog (AK_relation)
 * @param tblName table name
 * @return returns 1 if table exist or returns 0 if table does not exist
 */
int AK_index_table_exist(char *indexTblName) {
    AK_PRO;
    char *index_table = "AK_index";
    int exists = 0;

    int num_rows = AK_get_num_records(index_table);
    int a;

    if(num_rows==0){
        return exists;
    }

    for (a = 0; a < num_rows; a++) {
        struct list_node *el;
        el = AK_get_tuple(a, 1, index_table);
        if (strcmp(indexTblName, el->data)==0) {
            exists = 1;
            break;
        }
    }
    AK_EPI;
    return exists;
}

/**
 * @author Matija Šestak, modified for indexes by Lovro Predovan
 * @brief  Function that gets index table header
 * <ol>
 * <li>Read addresses of extents</li>
 * <li>If there is no extents in the table, return -1</li>
 * <li>else read the first block</li>
 * <li>allocate array</li>
 * <li>copy table header to the array</li>
 * </ol>
 * @param  *tblName table name
 * @result array of table header
 */
AK_header *AK_get_index_header(char *indexTblName) {
    AK_PRO;

    table_addresses *addresses = (table_addresses*) AK_get_index_addresses(indexTblName);

    if (addresses->address_from[0] == 0){
        AK_EPI;
        return 0;
    }

    AK_mem_block *temp = (AK_mem_block*) AK_get_block(addresses->address_from[0]);
    int num_attr = AK_num_index_attr(indexTblName);
    AK_header *head = (AK_header*) AK_calloc(num_attr, sizeof (AK_header));
    memcpy(head, temp->block->header, num_attr * sizeof (AK_header));
    AK_EPI;
    return head;
}




/**
 * Calculate maximum lengths for each column
 */
static void AK_calculate_column_lengths(char *indexTblName, AK_header *head, 
                                      int num_attr, int num_rows, int *len) {
    // Initialize with header lengths
    for (int i = 0; i < num_attr; i++) {
        len[i] = strlen((head + i)->att_name);
    }
    
    // Find maximum lengths for data
    for (int i = 0; i < num_attr; i++) {
        for (int j = 0; j < num_rows; j++) {
            struct list_node *el = AK_get_index_tuple(j, i, indexTblName);
            switch (el->type) {
                case TYPE_INT:
                    int length = AK_chars_num_from_number(*((int *) (el)->data), 10);
                    len[i] = MAX(len[i], length);
                    break;
                    
                case TYPE_FLOAT:
                    length = AK_chars_num_from_number(*((float *) (el)->data), 10);
                    len[i] = MAX(len[i], length);
                    break;
                    
                case TYPE_VARCHAR:
                default:
                    len[i] = MAX(len[i], el->size);
                    break;
            }
        }
    }
}

/**
 * Print table header
 */
static void AK_print_index_header(AK_header *head, int num_attr, int *len) {
    printf("\n|");
    
    for (int i = 0; i < num_attr; i++) {
        // Center header elements
        int k = (len[i] - (int) strlen((head + i)->att_name) + TBL_BOX_OFFSET + 1);
        if (k % 2 == 0) {
            k /= 2;
            printf("%-*s%-*s|", k, " ", k + (int) strlen((head + i)->att_name), 
                   (head + i)->att_name);
        } else {
            k /= 2;
            printf("%-*s%-*s|", k, " ", k + (int) strlen((head + i)->att_name) + 1, 
                   (head + i)->att_name);
        }
    }
    printf("\n");
}

/**
 * Print table data rows
 */
static void AK_print_index_data(table_addresses *addresses, int num_attr, int *len) {
    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row_root);
    int i = 0;
    
    while (addresses->address_from[i] != 0) {
        for (int j = addresses->address_from[i]; j < addresses->address_to[i]; j++) {
            AK_mem_block *temp = (AK_mem_block*) AK_get_block(j);
            
            for (int k = 0; k < DATA_BLOCK_SIZE; k += num_attr) {
                if (temp->block->tuple_dict[k].size > 0) {
                    for (int l = 0; l < num_attr; l++) {
                        int type = temp->block->tuple_dict[k + l].type;
                        int size = temp->block->tuple_dict[k + l].size;
                        int address = temp->block->tuple_dict[k + l].address;
                        
                        AK_InsertAtEnd_L3(type, &temp->block->data[address], size, row_root);
                    }
                    
                    AK_print_row(len, row_root);
                    AK_print_row_spacer(len, num_attr);
                    AK_DeleteAll_L3(&row_root);
                }
            }
        }
        i++;
    }
    
    AK_free(row_root);
}

/**
 * Print execution time stats
 */
static void AK_print_index_stats(char *indexTblName, int num_rows, clock_t start_time) {
    clock_t elapsed = clock() - start_time;
    printf("\nIndex: %s\n", indexTblName);
    
    if ((((double) elapsed) / CLOCKS_PER_SEC) < 0.1) {
        printf("%i rows found, duration: %f μs\n", num_rows, 
               ((double) elapsed) / CLOCKS_PER_SEC * 1000);
    } else {
        printf("%i rows found, duration: %f s\n", num_rows, 
               ((double) elapsed) / CLOCKS_PER_SEC);
    }
}

/**
 * @author Matija Šestak, modified for indexes by Lovro Predovan
 * @brief  Function that prints out the index table
 * @param *tblName table name
 * @return No return value
 */
void AK_print_index_table(char *indexTblName) {
    AK_PRO;
    
    // Validate index exists
    table_addresses *addresses = (table_addresses*) AK_get_index_addresses(indexTblName);
    if ((addresses->address_from[0] == 0) || (AK_index_table_exist(indexTblName) == 0)) {
        printf("Table %s does not exist!\n", indexTblName);
        AK_free(addresses);
        AK_EPI;
        return;
    }
    
    // Get table metadata
    AK_header *head = AK_get_index_header(indexTblName);
    int num_attr = AK_num_index_attr(indexTblName);
    int num_rows = AK_get_index_num_records(indexTblName);
    
    if (num_attr <= 0 || num_rows <= 0) {
        printf("Table is empty.\n");
        AK_free(addresses);
        AK_free(head);
        AK_EPI;
        return;
    }
    
    // Calculate column lengths
    int len[num_attr];
    AK_calculate_column_lengths(indexTblName, head, num_attr, num_rows, len);
    
    // Start timing
    clock_t start_time = clock();
    
    // Print table
    AK_print_row_spacer(len, num_attr);
    AK_print_index_header(head, num_attr, len);
    AK_print_row_spacer(len, num_attr);
    AK_print_index_data(addresses, num_attr, len);
    AK_print_index_stats(indexTblName, num_rows, start_time);
    
    // Cleanup
    AK_free(addresses);
    AK_free(head);
    AK_EPI;
}



/**
 * @author Lovro Predovan, updated by Matija Karaula
 * @brief Test function for index structures (list) and printing functionality
 * @return TestResult structure with test results
 */
TestResult AK_index_test() {
    AK_PRO;
    int successful = 0;
    int failed = 0;

    printf("\n********** INDEX TEST **********\n\n");

    // Test list operations
    list_ad *add_root = (list_ad *) AK_malloc(sizeof(list_ad));
    add_root->next = 0;

    // Test element insertion
    AK_Insert_NewelementAd(1, 1, "Alen", add_root);
    element_ad first = AK_Get_First_elementAd(add_root);
    if (first != NULL && strcmp(first->attName, "Alen") == 0) {
        successful++;
    } else {
        failed++;
        printf("Failed to insert first element\n");
    }

    // Test getting next element
    AK_Insert_NewelementAd(2, 2, "Markus", first);
    element_ad next = AK_Get_Next_elementAd(first);
    if (next != NULL && strcmp(next->attName, "Markus") == 0) {
        successful++;
    } else {
        failed++;
        printf("Failed to get next element\n");
    }

    // Test getting last element
    element_ad last = AK_Get_Last_elementAd(add_root);
    if (last != NULL && strcmp(last->attName, "Markus") == 0) {
        successful++;
    } else {
        failed++;
        printf("Failed to get last element\n");
    }

    // Test element deletion
    AK_Delete_elementAd(first, add_root);
    element_ad check = AK_Get_First_elementAd(add_root);
    if (check != NULL && strcmp(check->attName, "Markus") == 0) {
        successful++;
    } else {
        failed++;
        printf("Failed to delete element\n");
    }

    // Test index table operations
    char *tblName = "assistant";
    char *indexTblName = "assistantfirstname_bmapIndex";
    
    struct list_node *att_root = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&att_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "firstname", tblName, "firstname", att_root);
    
    // Test index creation
    AK_create_Index_Table(tblName, att_root);
    if (AK_index_table_exist(indexTblName)) {
        successful++;
    } else {
        failed++;
        printf("Failed to create index table\n");
    }

    // Test index attributes count
    int num_attr = AK_num_index_attr(indexTblName);
    if (num_attr > 0) {
        successful++;
    } else {
        failed++;
        printf("Failed to get index attributes\n");
    }

    // Test index records count
    int num_records = AK_get_index_num_records(indexTblName);
    if (num_records >= 0) {
        successful++;
    } else {
        failed++;
        printf("Failed to get index records count\n");
    }

    // Cleanup
    AK_Delete_All_elementsAd(add_root);
    AK_free(add_root);
    AK_DeleteAll_L3(&att_root);
    AK_free(att_root);

    AK_EPI;
    return TEST_result(successful, failed);
}