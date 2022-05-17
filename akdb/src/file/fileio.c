/**
@file fileio.c Provides functions for file input/output
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
#include "fileio.h"

//START SPECIAL FUNCTIONS FOR WORK WITH row_element_structure

/**
   * @author Matija Novak
   * @brief !! YOU PROBABLY DON'T WANT TO USE THIS FUNCTION !! - Use AK_Update_Existing_Element or AK_Insert_New_Element instead. 
   Function inserts new element after some element, to insert on first place give list as before element. New element
            is allocated. Type, data, attribute name and constraint of new elemets are set according to function arguments. Pointers
            are changed so that before element points to new element.
   * @param newtype type of the data
   * @param data the data
   * @param table table name
   * @param attribute_name attribute name
   * @param element element after we which insert the new element
   * @param constraint NEW_VALUE if data is new value, SEARCH_CONSTRAINT if data is constraint to search for
   * @return No return value
 */
void AK_Insert_New_Element_For_Update(int newtype, void *data, char *table, char *attribute_name, struct list_node *ElementBefore, int newconstraint)
{
    AK_PRO;

    struct list_node *newElement = (struct list_node *)AK_malloc(sizeof(struct list_node));
    newElement->type = newtype;
    memcpy(newElement->data, data, AK_type_size(newtype, data));

    if (newtype == TYPE_VARCHAR)
        newElement->data[AK_type_size(newtype, data)] = '\0';

    memcpy(newElement->table, table, strlen(table));
    newElement->table[strlen(table)] = '\0';
    memcpy(newElement->attribute_name, attribute_name, strlen(attribute_name));

    newElement->attribute_name[strlen(attribute_name)] = '\0';
    newElement->constraint = newconstraint;
    newElement->next = ElementBefore->next;

    ElementBefore->next = newElement;

    AK_EPI;
}

/**
   * @author Igor Rinkovec
   * @brief Used to add a constraint attribute which will define what element gets updated when the operation is executed.
   * @param newtype type of the data
   * @param data the data
   * @param table table name
   * @param attribute_name attribute name
   * @param element element after we which insert the new element
   * @param constraint is NEW_VALUE
   * @return No return value
 */
void AK_Update_Existing_Element(int newtype, void *data, char *table, char *attribute_name, struct list_node *ElementBefore)
{
    AK_PRO;
    AK_Insert_New_Element_For_Update(newtype, data, table, attribute_name, ElementBefore, SEARCH_CONSTRAINT);
    AK_EPI;
}

/**
   * @author Matija Novak, changed by Dino Laktašić
   * @brief Used to add a new element after some element, to insert on first place give list as before element. It calls function
            AK_Insert_New_Element_For_Update.
   * @param newtype type of the data
   * @param data the data
   * @param table table name
   * @param attribute_name attribute name
   * @param element element after we which insert the new element
   * @param constraint is NEW_VALUE
   * @return No return value
 */
void AK_Insert_New_Element(int newtype, void *data, char *table, char *attribute_name, struct list_node *ElementBefore)
{
    AK_PRO;
    AK_Insert_New_Element_For_Update(newtype, data, table, attribute_name, ElementBefore, NEW_VALUE);
    AK_EPI;
}

//END SPECIAL FUNCTIONS row_element_structure

/** @author Matija Novak, updated by Dino Laktašić
        @brief Function inserts one row into some block.  Firstly it checks wether block contain attributes from the list. Then
               data, type, size and last_tuple_id are put in temp_block.
        @param row_root list of elements to insert
        @param temp_block block in which we insert data
        @return EXIT SUCCES if success
 */
int AK_insert_row_to_block(struct list_node *row_root, AK_block *temp_block)
{
    struct list_node *some_element;
    int type;        //type od entry data
    int id = 0;      //id tuple dict in which is inserted next data
    int head = 0;    //index of header which is curently inserted
    int search_elem; //serch for tuple dict id and searc for data in list
    char entry_data[MAX_VARCHAR_LENGTH];
    AK_PRO;

    while (strcmp(temp_block->header[head].att_name, "\0") != 0)
    { //inserting values of the list one by one
        while (temp_block->tuple_dict[id].size != FREE_INT)
        { //searches for AK_free tuple dict, maybe it can be last_tuple_dict_id
            id++;
        }
        //printf("insert_row_to_block: Position to write (tuple_dict_index) %d, header_att_name %s\n", id, temp_block->header[head].att_name);

        AK_dbg_messg(HIGH, FILE_MAN, "insert_row_to_block: Position to write (tuple_dict_index) %d, header_att_name %s\n", id, temp_block->header[head].att_name);

        search_elem = 1;
        some_element = (struct list_node *)AK_First_L2(row_root);
        while (search_elem)
        {
            if ((strcmp(some_element->attribute_name, temp_block->header[head].att_name) == 0) && (some_element->constraint == 0))
            { //found correct element
                type = some_element->type;

                memset(entry_data, '\0', MAX_VARCHAR_LENGTH);
                memcpy(entry_data, some_element->data, AK_type_size(type, some_element->data));

                search_elem = 0;
            }
            else
            {
                some_element = (struct list_node *)AK_Next_L2(some_element);
                if (some_element == 0)
                { //no data exist for this header write null
                    some_element = 1;
                    memcpy(entry_data, "null", strlen("null"));
                    type = TYPE_VARCHAR;
                    search_elem = 0;
                }
            }
        }

        memcpy(temp_block->data + temp_block->AK_free_space, entry_data, (int)AK_type_size(type, entry_data));
        temp_block->tuple_dict[id].address = temp_block->AK_free_space;
        temp_block->AK_free_space += AK_type_size(type, entry_data);
        temp_block->tuple_dict[id].type = type;
        temp_block->tuple_dict[id].size = AK_type_size(type, entry_data);

        memcpy(entry_data, temp_block->data + temp_block->tuple_dict[id].address, temp_block->tuple_dict[id].size);

        AK_dbg_messg(HIGH, FILE_MAN, "insert_row_to_block: Insert: data: %s, size: %d\n", entry_data, AK_type_size(type, entry_data));
        head++; //go to next header
    }
    //writes the last used tuple dict id

    temp_block->last_tuple_dict_id = id;
    AK_EPI;
    return EXIT_SUCCESS;
}

/** @author Matija Novak, updated by Matija Šestak (function now uses caching), updated by Dejan Frankovic (added reference check), updated by Dino         Laktašić (removed variable AK_free, variable table initialized using memset)
        @brief Function inserts a one row into table. Firstly it is checked whether inserted row would violite reference integrity.
        Then it is checked in which table should row be inserted. If there is no AK_free space for new table, new extent is allocated. New block is            allocated on given address. Row is inserted in this block and dirty flag is set to BLOCK_DIRTY.
        @param row_root list of elements which contain data of one row
        @return EXIT_SUCCESS if success else EXIT_ERROR

 */
int AK_insert_row(struct list_node *row_root)
{
    AK_PRO;
    AK_dbg_messg(HIGH, FILE_MAN, "insert_row: Start testing reference integrity.\n");

    // recovery checkpoint
    AK_add_to_redolog(INSERT, row_root);

    if (AK_reference_check_entry(row_root) == EXIT_ERROR)
    {
        printf("Could not insert row. Reference integrity violation.\n");
        AK_EPI;
        return EXIT_ERROR;
    }

    AK_dbg_messg(HIGH, FILE_MAN, "insert_row: Start inserting data\n");
    struct list_node *some_element = (struct list_node *)AK_First_L2(row_root);
    char table[MAX_ATT_NAME];
    table_addresses *table_addresses_return;

    memset(table, '\0', MAX_ATT_NAME);
    memcpy(&table, some_element->table, strlen(some_element->table));
    AK_dbg_messg(HIGH, FILE_MAN, "insert_row: Insert into table: %s\n", table);
    int adr_to_write;
    table_addresses_return = AK_get_table_addresses(table);
    adr_to_write = (int)AK_find_AK_free_space(table_addresses_return);
    AK_free(table_addresses_return);

    if (adr_to_write == -1)
        adr_to_write = (int)AK_init_new_extent(table, SEGMENT_TYPE_TABLE);
    if (strstr(some_element->table, "_bmapIndex"))
    {
        table_addresses_return = AK_get_index_addresses(table);
        adr_to_write = (int)AK_find_AK_free_space(table_addresses_return);
        AK_free(table_addresses_return);
    }

    if (adr_to_write == 0)
    {
        AK_EPI;
        return EXIT_ERROR;
    }

    table_addresses *addresses = (table_addresses *)AK_get_table_addresses(table);
    AK_dbg_messg(HIGH, FILE_MAN, "insert_row: Insert into block on adress: %d\n", adr_to_write);
    AK_mem_block *mem_block = (AK_mem_block *)AK_get_block(adr_to_write);

    int end = (int)AK_insert_row_to_block(row_root, mem_block->block);

    if (end == EXIT_SUCCESS)
        AK_redolog_commit();

    AK_mem_block_modify(mem_block, BLOCK_DIRTY);
    AK_EPI;
    return end;
}

/**
   * @author Matija Novak, updated by Dino Laktašić, updated by Mario Peroković - separated from deletion
   * @brief Function updates row from table in given block.
   * @param temp_block block to work with
   * @param row_list list of elements which contain data for delete or update
   * @return No return value
*/
void AK_update_row_from_block(AK_block *temp_block, struct list_node *row_root)
{
    int head = 0;                        //counting headers
    int attPlace = 0;                    //place of attribute which are same
    int del = 1;                         //if can delete gorup of tuple dicts which are in the same row of table
    int exists_equal_attrib = 0;         //if we found at least one header in the list
    char entry_data[MAX_VARCHAR_LENGTH]; //entry data when haeader is found in list which is copied to compare with data in block
    AK_PRO;
    struct list_node *new_data = (struct list_node *)AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&new_data);

    struct list_node *some_element = (struct list_node *)AK_First_L2(row_root);

    int i, overflow, address, size;

    for (i = 0; i < DATA_BLOCK_SIZE; i++)
    {
        exists_equal_attrib = 0;
        head = 0;
        attPlace = 0;
        address = temp_block->tuple_dict[i].address;
        size = temp_block->tuple_dict[i].size;
        overflow = address + size;

        while (strcmp(temp_block->header[head].att_name, "\0") != 0)
        { //going through headers

            some_element = row_root;
            while (some_element)
            {
                if ((strcmp(some_element->attribute_name, temp_block->header[head].att_name) == 0) && (some_element->constraint == SEARCH_CONSTRAINT))
                {
                    exists_equal_attrib = 1;
                    attPlace = head;

                    if (overflow < (temp_block->AK_free_space + 1) && overflow > -1)
                    {
                        memset(entry_data, '\0', MAX_VARCHAR_LENGTH);
                        memcpy(entry_data, temp_block->data + address, size);

                        // if the data in table isn't equal to data in attribute which is used for search, it won't be updated
                        if (strcmp(entry_data, some_element->data) != 0)
                            del = 0;
                    }
                    else
                        del = 0;
                }

                some_element = some_element->next;
            }
            head++; //next header
        }

        if (exists_equal_attrib == 1 && del == 1)
        {
            int j;
            for (j = i - attPlace; j < i + head - attPlace; j++)
            {
                AK_DeleteAll_L3(&new_data);
                int a = temp_block->tuple_dict[j].address;
                int s = temp_block->tuple_dict[j].size;
                if (a > 0 && s > 0)
                {
                    memset(entry_data, '\0', MAX_VARCHAR_LENGTH);
                    memcpy(entry_data, temp_block->data + a, s);
                }
                some_element = row_root;

                while (some_element)
                {
                    // save data from roow_root in a list new_data where whole row is being inserted
                    AK_Insert_New_Element(temp_block->tuple_dict[j].type, some_element->data, some_element->table, some_element->attribute_name, new_data);

                    if (strcmp(some_element->attribute_name, temp_block->header[j % head].att_name) == 0 && some_element->constraint == NEW_VALUE)
                    {
                        // we need to delete and insert row, because size of new data is larger than size of old data
                        if (strlen(some_element->data) > s)
                        {
                            for (int k = i - attPlace; k < i + head - attPlace; k++)
                            {
                                int o = temp_block->tuple_dict[k].address;
                                int p = temp_block->tuple_dict[k].size;
                                memset(entry_data, '\0', MAX_VARCHAR_LENGTH);
                                memcpy(entry_data, temp_block->data + o, p);
                                // if att_names are different, this is old data, and we need to insert it again
                                if (strcmp(some_element->attribute_name, temp_block->header[k % head].att_name) != 0)
                                    AK_Insert_New_Element(temp_block->tuple_dict[k].type, entry_data, some_element->table, temp_block->header[k % head].att_name, new_data);

                                temp_block->tuple_dict[k].size = 0;
                                temp_block->tuple_dict[k].type = 0;
                                temp_block->tuple_dict[k].address = 0;
                            }
                            // insert new data
                            AK_insert_row(new_data);
                        }
                        // we need to update row
                        else
                            memcpy(temp_block->data + a, some_element->data, s);
                    }
                    //some_element = (struct list_node *) AK_Next_L2(some_element);
                    some_element = some_element->next;
                }
            }
        }
        del = 1;
    }
    // everything from new data has been copied so we can deallocate all in list - corr. Elvis Popovic
    AK_DeleteAll_L3(&new_data);
    AK_free(new_data);
    AK_EPI;
    return EXIT_SUCCESS;
}

/**
   * @author Matija Novak, updated by Dino Laktašić, changed by Davorin Vukelic, updated by Mario Peroković
   * @brief Function deletes row from table in given block. Given list of elements is firstly back-upped.
   * @param temp_block block to work with
   * @param row_list list of elements which contain data for delete or update
   * @return No return value
*/
void AK_delete_row_from_block(AK_block *temp_block, struct list_node *row_root)
{
    int head = 0;                        //counting headers
    int attPlace = 0;                    //place of attribute which are same
    int del = 1;                         //if can delete gorup of tuple dicts which are in the same row of table
    int exists_equal_attrib = 0;         //if we found at least one header in the list
    char entry_data[MAX_VARCHAR_LENGTH]; //entry data when haeader is found in list which is copied to compare with data in block
    AK_PRO;
    struct list_node *row_root_backup = (struct list_node *)AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row_root_backup);

    struct list_node *some_element = (struct list_node *)AK_First_L2(row_root);

    while (some_element)
    { //make a copy of list
        AK_Insert_New_Element_For_Update(some_element->type, some_element->data, some_element->table, some_element->attribute_name, row_root_backup, some_element->constraint);
        //some_element = (struct list_node *) AK_Next_L2(some_element);
        some_element = some_element->next;
    }

    int i, overflow, address, size;

    for (i = 0; i < DATA_BLOCK_SIZE; i++)
    { //AK_freeze point, if there is no i++
        head = 0;
        attPlace = 0;
        address = temp_block->tuple_dict[i].address;
        size = temp_block->tuple_dict[i].size;
        overflow = address + size;

        while (strcmp(temp_block->header[head].att_name, "\0") != 0)
        { //going through headers
            some_element = row_root;

            while (some_element)
            {
                //if we found header that is constraint in list
                if ((strcmp(some_element->attribute_name, temp_block->header[head].att_name) == 0) && (some_element->constraint == SEARCH_CONSTRAINT))
                {

                    exists_equal_attrib = 1;
                    attPlace = head;

                    if ((overflow < (temp_block->AK_free_space + 1)) && (overflow > -1))
                    {
                        //before there was for loop to clear (check if memset works correct)
                        memset(entry_data, '\0', MAX_VARCHAR_LENGTH);
                        memcpy(entry_data, temp_block->data + address, size);

                        if (strcmp(entry_data, some_element->data) != 0)
                            del = 0; //if one constraint doesn't metch we dont delete or update
                    }
                    else
                        del = 0;
                }
                some_element = some_element->next;
            }
            head++;
        }

        if ((exists_equal_attrib == 1) && (del == 1))
        {
            for (int j = i - attPlace; j < i + head - attPlace; j++)
            { //delete one row

                int k = temp_block->tuple_dict[j].address;
                int l = temp_block->tuple_dict[j].size;
                memset(temp_block->data + k, '\0', l);
                AK_dbg_messg(HIGH, FILE_MAN, "update_delete_row_from_block: from: %d, to: %d\n", k, l + k);

                //clean tuple dict
                temp_block->tuple_dict[j].size = 0;
                temp_block->tuple_dict[j].type = 0;
                temp_block->tuple_dict[j].address = 0;
            }
        }
        del = 1;
        exists_equal_attrib = 0;
    }
    AK_free(row_root_backup);
    AK_EPI;
}

/**
      * @author Matija Novak, updated by Matija Šestak (function now uses caching)
      * @brief Function updates or deletes the whole segment of an table. Addresses for given table atr fetched. For each block
        in extent row is updated or deleted according to operator del.
      * @param row_root elements of one row
      * @param del - DELETE or UPDATE
      * @return EXIT_SUCCESS if success
*/
int AK_delete_update_segment(struct list_node *row_root, int del)
{
    char table[MAX_ATT_NAME];
    AK_PRO;
    struct list_node *some_element = (struct list_node *)AK_First_L2(row_root);

    strcpy(table, some_element->table);
    table[strlen(some_element->table)] = '\0';
    AK_dbg_messg(HIGH, FILE_MAN, "delete_update_segment: table to delete_update from: %s, source %s\n", table, some_element->table);

    table_addresses *addresses = (table_addresses *)AK_get_table_addresses(table);

    AK_mem_block *mem_block;
    int startAddress, j, i;

    for (j = 0; j < MAX_EXTENTS_IN_SEGMENT; j++)
    { //going through extent
        startAddress = addresses->address_from[j];
        if (startAddress != 0)
        {
            AK_dbg_messg(HIGH, FILE_MAN, "delete_update_segment: delete_update extent: %d\n", j);

            for (i = startAddress; i <= addresses->address_to[j]; i++)
            { //going through blocks
                AK_dbg_messg(HIGH, FILE_MAN, "delete_update_segment: delete_update block: %d\n", i);
                mem_block = (AK_mem_block *)AK_get_block(i);

                if (del == DELETE)
                    AK_delete_row_from_block(mem_block->block, row_root);
                else
                    AK_update_row_from_block(mem_block->block, row_root);
                AK_mem_block_modify(mem_block, BLOCK_DIRTY);
            }
        }
        else
            break;
    }
    AK_free(addresses);
    AK_EPI;
    return EXIT_SUCCESS;
}

/** @author Matija Novak, Dejan Frankovic (added referential integrity)
        @brief Function deletes rows
        @param row_root elements of one row
        @returs EXIT_SUCCESS if success
 */
int AK_delete_row(struct list_node *row_root)
{
    AK_PRO;

    if (AK_reference_check_restricion(row_root, DELETE) == EXIT_ERROR)
    {
        AK_dbg_messg(HIGH, FILE_MAN, "Could not delete row. Reference integrity violation (restricted).\n");
        return EXIT_ERROR;
    }

    // recovery checkpoint
    if (AK_reference_check_if_update_needed(row_root, DELETE) == EXIT_SUCCESS)
        AK_reference_update(row_root, DELETE);

    AK_delete_update_segment(row_root, DELETE);
    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 *@author Dražen Bandić
 *@brief Function deletes row by id
 *@param id id of row
 *@param tableName name of table to delete the row
 */
void AK_delete_row_by_id(int id, char *tableName)
{
    AK_PRO;
    char *attributes = AK_rel_eq_get_atrributes_char(tableName);
    char *nameID = AK_malloc(MAX_VARCHAR_LENGTH * sizeof(char));
    int index = 0;
    do
    {
        if (*attributes == ';')
        {
            nameID[index] = '\0';
            break;
        }
        else
            nameID[index++] = *attributes;
        attributes++;
    } while (*attributes != '\0' || index < MAX_VARCHAR_LENGTH);

    struct list_node *row_root = (struct list_node *)AK_malloc(sizeof(struct list_node));
    AK_Update_Existing_Element(TYPE_INT, &id, tableName, nameID, row_root);
    AK_delete_row(row_root);
    AK_EPI;
}

/** @author Matija Novak, Dejan Frankovic (added referential integrity)
        @brief Function updates rows of some table
        @param row_root elements of one row
        @return EXIT_SUCCESS if success
*/
int AK_update_row(struct list_node *row_root)
{
    AK_PRO;
    if (AK_reference_check_restricion(row_root, UPDATE) == EXIT_ERROR)
    {
        AK_dbg_messg(HIGH, FILE_MAN, "Could not update row. Reference integrity violation (restricted).\n");
        AK_EPI;
        return EXIT_ERROR;
    }

    // recovery checkpoint

    if (AK_reference_check_if_update_needed(row_root, UPDATE) == EXIT_SUCCESS)
        AK_reference_update(row_root, UPDATE);
    AK_delete_update_segment(row_root, UPDATE);
    AK_EPI;
    return EXIT_SUCCESS;
}

TestResult AK_fileio_test()
{
    AK_PRO;
    printf("\n\nThis is fileio test!\n");
    int ok = 0, fail = 0;

    AK_header t_header[4] = {
        {TYPE_INT, "Redni_broj", {0}, {{'\0'}}, {{'\0'}}},
        {TYPE_VARCHAR, "Ime", {0}, {{'\0'}}, {{'\0'}}},
        {TYPE_VARCHAR, "Prezime", {0}, {{'\0'}}, {{'\0'}}},
        {0, {'\0'}, {0}, {{'\0'}}, {{'\0'}}}};

    int startAddress = AK_initialize_new_segment("testna", SEGMENT_TYPE_TABLE, t_header);

    if (startAddress != EXIT_ERROR)
        printf("\nTABLE %s CREATED!\n", "testna");

    struct list_node *row_root = (struct list_node *)AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row_root);
    int number;

    number = 1;
    AK_DeleteAll_L3(&row_root);
    AK_Insert_New_Element(TYPE_INT, &number, "testna", "Redni_broj", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "Matija", "testna", "Ime", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "Novak", "testna", "Prezime", row_root);
    if (AK_insert_row(row_root) == EXIT_SUCCESS)
        ok++;
    else
        fail++;

    AK_DeleteAll_L3(&row_root);
    number = 2;
    AK_Insert_New_Element(TYPE_INT, &number, "testna", "Redni_broj", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "Nikola", "testna", "Ime", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "Bakoš", "testna", "Prezime", row_root);
    if (AK_insert_row(row_root) == EXIT_SUCCESS)
        ok++;
    else
        fail++;

    AK_DeleteAll_L3(&row_root);
    number = 3;
    AK_Insert_New_Element(TYPE_INT, &number, "testna", "Redni_broj", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "Matija", "testna", "Ime", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "Bakoš", "testna", "Prezime", row_root);
    if (AK_insert_row(row_root) == EXIT_SUCCESS)
        ok++;
    else
        fail++;

    for (int i = 5; i < 10; i++)
    {
        AK_DeleteAll_L3(&row_root);
        number = i;
        AK_Insert_New_Element(TYPE_INT, &number, "testna", "Redni_broj", row_root);
        AK_Insert_New_Element(TYPE_VARCHAR, "Maja", "testna", "Ime", row_root);
        AK_Insert_New_Element(TYPE_VARCHAR, "Vacenovski", "testna", "Prezime", row_root);
        if (AK_insert_row(row_root) == EXIT_SUCCESS)
            ok++;
        else
            fail++;
    }

    AK_print_table("testna");

    AK_DeleteAll_L3(&row_root);
    number = 3;
    AK_Update_Existing_Element(TYPE_INT, &number, "testna", "Redni_broj", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "Vjekoslavoski", "testna", "Prezime", row_root);
    AK_update_row(row_root);

    AK_print_table("testna");

    AK_DeleteAll_L3(&row_root);
    number = 2;
    AK_Update_Existing_Element(TYPE_INT, &number, "testna", "Redni_broj", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "Francina", "testna", "Ime", row_root);
    AK_update_row(row_root);

    AK_print_table("testna");

    AK_DeleteAll_L3(&row_root);
    number = 7;
    AK_Update_Existing_Element(TYPE_INT, &number, "testna", "Redni_broj", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "M", "testna", "Prezime", row_root);
    AK_update_row(row_root);

    AK_print_table("testna");

    AK_DeleteAll_L3(&row_root);
    AK_Update_Existing_Element(TYPE_VARCHAR, "Maja", "testna", "Ime", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "DugackoPrezime", "testna", "Prezime", row_root);
    AK_update_row(row_root);

    AK_print_table("testna");

    AK_DeleteAll_L3(&row_root);
    AK_Update_Existing_Element(TYPE_VARCHAR, "Maja", "testna", "Ime", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "DugackoIme", "testna", "Ime", row_root);
    AK_update_row(row_root);

    AK_print_table("testna");

    AK_print_table("testna");

    AK_DeleteAll_L3(&row_root);
    AK_free(row_root);
    AK_EPI;
    return TEST_result(ok, fail);
}
