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
 * @brief Function that initialises a linked list
 * @param list_ad *L linked list head
 * @return No return value
 * */
void AK_InitializelistAd(list_ad *L) {
    AK_PRO;
    L->next = 0;
    AK_EPI;
}

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
 * @author Matija Šestak, modified for indexes by Lovro Predovan
 * @brief Function that gets value in some row and column
 * @param row zero-based row index
 * @param column zero-based column index
 * @param *tblName table name
 * @return value in the list
 */
struct list_node *AK_get_index_tuple(int row, int column, char *indexTblName) {
    AK_PRO;

    int num_rows = AK_get_index_num_records(indexTblName);
    int num_attr = AK_num_index_attr(indexTblName);

    if (row >= num_rows || column >= num_attr){
        AK_EPI;
        return NULL;
    }

    table_addresses *addresses = (table_addresses*) AK_get_index_addresses(indexTblName);

    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&row_root);

    int i, j, k, counter;
    char data[ MAX_VARCHAR_LENGTH ];

    i = 0;
    counter = -1;
    while (addresses->address_from[ i ] != 0) {
        for (j = addresses->address_from[ i ]; j < addresses->address_to[ i ]; j++) {
            AK_mem_block *temp = (AK_mem_block*) AK_get_block(j);
            if (temp->block->last_tuple_dict_id == 0) break;
            for (k = 0; k < DATA_BLOCK_SIZE; k += num_attr) {
                if (temp->block->tuple_dict[k].size > 0)
                    counter++;
                if (counter == row) {
                    int type = temp->block->tuple_dict[ k + column ].type;
                    int size = temp->block->tuple_dict[ k + column ].size;
                    int address = temp->block->tuple_dict[ k + column ].address;
                    memcpy(data, &temp->block->data[address], size);
                    data[ size ] = '\0';
                    AK_InsertAtEnd_L3(type, data, size, row_root);
                    AK_free(addresses);
                    AK_EPI;
                    return (struct list_node *) AK_First_L2(row_root);
                }
            }
        }
        i++;
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
 * @author Matija Šestak, modified for indexes by Lovro Predovan
 * @brief  Function that prints out the index table
 * @param *tblName table name
 * @return No return value
 */
void AK_print_index_table(char *indexTblName) {
    AK_PRO;
    table_addresses *addresses = (table_addresses*) AK_get_index_addresses(indexTblName);
    char *index_table = "AK_index";

    if ((addresses->address_from[0] == 0)  || (AK_index_table_exist(indexTblName) == 0)) {
        printf("Table %s does not exist!\n", indexTblName);
        AK_EPI;
    } else {

        AK_header *head = AK_get_index_header(indexTblName);

        int i, j, k, l;
        int num_attr = AK_num_index_attr(indexTblName);
        int num_rows = AK_get_index_num_records(indexTblName);
        int len[num_attr]; //max length for each attribute in row
        int length = 0; //length of spacer
        clock_t t;

        //printf("BROJ ATTR %i ; BROJ REDOVA %i U TABLICI %s",num_attr,num_rows,indexTblName);
        //store lengths of header attributes
        for (i = 0; i < num_attr; i++) {
            len[i] = strlen((head + i)->att_name);
        }


        //for each header attribute iterate through all table rows and check if
        //there is longer element than previously longest and store it in array
        for (i = 0; i < num_attr; i++) {
            for (j = 0; j < num_rows; j++) {
                struct list_node *el = AK_get_index_tuple(j, i, indexTblName);
                switch (el->type) {

                    case TYPE_INT:
                        length = AK_chars_num_from_number(*((int *) (el)->data), 10);
                        if (len[i] < length) {
                            len[i] = length;
                        }
                        break;
                    case TYPE_FLOAT:
                        length = AK_chars_num_from_number(*((float *) (el)->data), 10);
                        if (len[i] < length) {
                            len[i] = length;
                        }
                        break;
                    case TYPE_VARCHAR:
                    default:
                        if (len[i] < el->size) {
                            len[i] = el->size;
                        }
                        break;
                }
            }
        }
        //num_attr is number of char | + space in printf
        //set offset to change the box size

        length = 0;
        for (i = 0; i < num_attr; length += len[i++]);
        length += num_attr * TBL_BOX_OFFSET + 2 * num_attr + 1;

        //start measuring time
        t = clock();

        if (num_attr <= 0 || num_rows <= 0) {
            printf("Table is empty.\n");
            AK_EPI;
        } else {

            AK_print_row_spacer(len, length);
            printf("\n|");

            //HEADER
            for (i = 0; i < num_attr; i++) {
                //PRINTING HEADER ELEMENTS CENTERED
                k = (len[i] - (int) strlen((head + i)->att_name) + TBL_BOX_OFFSET + 1);
                if (k % 2 == 0) {
                    k /= 2;
                    printf("%-*s%-*s|", k, " ", k + (int) strlen((head + i)->att_name), (head + i)->att_name);
                } else {
                    k /= 2;
                    printf("%-*s%-*s|", k, " ", k + (int) strlen((head + i)->att_name) + 1, (head + i)->att_name);
                }
            }
            printf("\n");
            AK_print_row_spacer(len, length);
            //END HEADER

            struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
            AK_Init_L3(&row_root);
            int type, size, address,last_addres_value;
            i = 0;

            if(addresses->address_to[i] != 0){
                last_addres_value = addresses->address_to[i];
            }


            while (addresses->address_from[i] != 0) {

                for (j = addresses->address_from[i]; j < addresses->address_to[i]; j++) {
                    AK_mem_block *temp = (AK_mem_block*) AK_get_block(j);

                    //THIS IS KINDA HACK, THIS PROBABLY SHOULD BE CHANGED SO MAIN INDEX TABLE ARE NOT INCLUDED
                    if ((temp->block->last_tuple_dict_id == 0) && (last_addres_value == addresses->address_to[i])){
                        printf("\nIndex: %s\n", indexTblName);


                        //PRINTING OUT TIME STATS
                        t = clock() - t;
                        if ((((double) t) / CLOCKS_PER_SEC) < 0.1) {
                            printf("%i rows found, duration: %f μs\n", num_rows, ((double) t) / CLOCKS_PER_SEC * 1000);
                        } else {
                            printf("%i rows found, duration: %f s\n", num_rows, ((double) t) / CLOCKS_PER_SEC);
                        }

                        AK_free(row_root);
                        AK_free(addresses);
                        AK_EPI;
                        //break;
                        return 1;
                    }

                    //PRINTING VALUES IN INDEX TABLE
                    for (k = 0; k < DATA_BLOCK_SIZE; k += num_attr) {
                        if (temp->block->tuple_dict[k].size > 0) {
                            for (l = 0; l < num_attr; l++) {

                                type = temp->block->tuple_dict[k + l].type;
                                size = temp->block->tuple_dict[k + l].size;
                                address = temp->block->tuple_dict[k + l].address;

                                AK_InsertAtEnd_L3(type, &temp->block->data[address], size, row_root);
                            }

                            AK_print_row(len, row_root);
                            AK_print_row_spacer(len, length);
                            AK_DeleteAll_L3(&row_root);
                        }
                    }
                }
                i++;
            }

        AK_free(row_root);
        }
    }

    AK_free(addresses);
    AK_EPI;
}



/**
 * @author Lovro Predovan
 * @brief  Test funtion for index structures(list) and printing table
 * @return No return value
 */
void AK_index_test() {

    AK_PRO;
    int position;
    printf("\n********** INDEX TEST **********\n\n");

    list_ad *add_root;
    add_root = (list_ad *) AK_malloc(sizeof (list_ad));


    AK_InitializelistAd(add_root);

    AK_Insert_NewelementAd(1, 1, "Alen", add_root);
    element_ad some_element;

    some_element = AK_Get_First_elementAd(add_root);
    AK_Insert_NewelementAd(2, 2, "Markus", some_element);

    element_ad some_elementt = AK_Get_Last_elementAd(add_root);
    AK_Insert_NewelementAd(3, 3, "Mirko", some_elementt);

    element_ad some_elementtt = AK_Get_Last_elementAd(add_root);
    AK_Insert_NewelementAd(4, 4, "Sandra", some_elementtt);

    element_ad some_elementttt = AK_Get_Last_elementAd(add_root);
    AK_Insert_NewelementAd(5, 5, "Tonimir", some_elementttt);


    printf("\n********** Printing all values **********\n\n");
    element_ad ele = AK_Get_First_elementAd(add_root);
    while (ele != 0)
    {
        printf("Value : %s\n", ele->attName);
        ele = AK_Get_Next_elementAd(ele);
    }

    printf("\n********** Printing position values **********\n\n");
     element_ad last = AK_Get_Last_elementAd(add_root);   
     printf("Last element in list: %s \n",last->attName);

     element_ad previous = AK_Get_Previous_elementAd(last,add_root);
     printf("Previous element from list: %s \n",previous->attName);

     element_ad first = AK_Get_First_elementAd(add_root);
     printf("First element in list: %s \n",first->attName);

     element_ad next = AK_Get_Next_elementAd(first);
     printf("Next element in list after '%s' is : %s \n",first->attName,next->attName);

     printf("Position of element '%s' above is : %i \n",next->attName,AK_Get_Position_Of_elementAd(next, add_root));

    printf("\n********** Deleting values **********\n\n");

    printf("Deleting element: %s \n",previous->attName);
    AK_Delete_elementAd(previous, add_root) ;
    element_ad elem = AK_Get_First_elementAd(add_root);

    printf("\n********** Printing values after delete**********\n\n");

        while (elem != 0)
        {
            printf("Value : %s\n", elem->attName);
            elem = AK_Get_Next_elementAd(elem);
        }

    printf("\n********** Printing values after deleting all values**********\n\n");
    //Error: incomplete type is not allowed
    void AK_Delete_All_elementsAd(add_root) ;
    element_ad eleme = AK_Get_First_elementAd(add_root);
    while (elem != 0)
        {
            printf("Value : %s\n", eleme->attName);
            eleme = AK_Get_Next_elementAd(eleme);

        }

    printf("\n********** Index test **********\n\n");
        //CREATING BITMAP INDEXES ON ATTRIBUTE FIRSTNAME AS REFERENCE

    char *tblName = "assistant";
    char *indexTblName = "assistantfirstname_bmapIndex";

    char *notExistingIndexName = "assistantincome_bmapIndex";

    struct list_node *att_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&att_root);
    //Error: argument of type "const char *" is incompatible with parameter of type "void *"
    AK_Insert_New_Element(TYPE_VARCHAR, "firstname", tblName, "firstname", att_root);
 
    AK_create_Index_Table(tblName, att_root);

    AK_print_table(tblName);
    AK_print_table("AK_index");
    AK_print_index_table(indexTblName);

    printf("\n\nIndex table %s exists: %i\n\n",indexTblName,AK_index_table_exist(indexTblName));
    printf("Number of rows in index %s: %i\n\n",indexTblName,AK_get_index_num_records(indexTblName));
    printf("Number of attributes in index %s: %i\n\n",indexTblName,AK_num_index_attr(indexTblName));
    printf("Index table %s exists: %i\n\n\n",notExistingIndexName,AK_index_table_exist(notExistingIndexName));


    printf("\n********** Index test Passed**********\n\n");


    AK_EPI;
}
