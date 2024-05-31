/**
@file bitmap.h Header file that declares functions
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

#ifndef BITMAP
#define BITMAP

#include "../../auxi/test.h"
#include "../../mm/memoman.h"
#include "index.h"
#include "../../file/table.h"
#include "../../file/fileio.h"
#include "../../file/files.h"
#include "../../auxi/mempro.h"

/**
  @author Saša Vukšić
  @brief Function that examines whether list L contains operator ele
  @param L list of elements
  @param ele operator to be found in list
  @return 1 if operator ele is found in list, otherwise 0
 */
int AK_If_ExistOp(struct list_node *L, char *ele);

/**
 * @author Saša Vukšić, Lovro Predovan
 * @brief Function that reads table on which we create index and call functions for creating index
           Elements that will be in index are put in list indexLista and headerAttributes. According to those elements new indexes
           are created.

 * @param tblName name of table
 * @param attributes list of attributes on which we will create indexes
 * @return No return value
 * */
void AK_create_Index_Table(char *tblName, struct list_node *attributes);

/**
 * @author Saša Vukšić
 * @brief Function that tests printing header of table
 * @param tblName name of table who's header we are printing
   @return No return value
 **/
void AK_print_Header_Test(char* tblName);

/**
 * @author Saša Vukšić, Lovro Predovan
 * @brief Function that loads index table with the value of particulary atribute
 * @param tblName source table
 * @param tblNameIndex new name of index table
 * @param attributeName attribute on which we make index
 * @param positionTbl position of attribute in header of table
 * @param numAtributes number of attributes in table
 * @param headerIndex header of index table
 * @return No return value
 */
void AK_create_Index(char *tblName, char *tblNameIndex, char *attributeName, int positionTbl, int numAtributes, AK_header *headerIndex);

/**
 * @author Saša Vukšić, Lovro Predovan
 * @brief Function that gets adresses of the particuliar attribute from bitmap index. It fetches addresses of indexes and header
          of index table.  Using while loop it goes through index and gets necessary data. That data is put in a list called
          add_root.
 * @param indexName name of index
 * @param attribute name of attribute
 * @return list of adresses
 **/
list_ad* AK_get_attribute(char *indexName, char *attribute);


void AK_create_List_Address_Test();

/**
 * @author Saša Vukšić, Lovro Predovan
 * @brief Function that prints the list of adresses
 * @param list list of adresses
 * @return No return value
 **/
void AK_print_Att_Test(list_ad *list);

/**
 * @author Saša Vukšić
 * @brief Function that fetches the values from the bitmap index if there is one for a given table.
 * It should be started when we are making selection on the table with bitmap index.
 * @param tableName name of table
 * @param attributeValue value of attribute
 * @return list of adresses
 **/
list_ad* AK_get_Attribute(char *tableName, char *attributeName, char *attributeValue);

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
void AK_update(int addBlock, int addTd, char *tableName, char *attributeName, char *attributeValue, char *newAttributeValue);

/**
 * @author Saša Vukšić
 * @brief Function that writes the new value in block when index is updated
 * @param block block to write on
 * @return EXIT_SUCESS when write operation is successful, otherwise EXIT_ERROR
 **/
int AK_write_block(AK_block * block);

/**
 * @author Saša Vukšić updated by Lovro Predovan
 * @brief Function that creates test table and makes index on test table,
    also prints original tables indexes tables and indexes, tests updating into tables
 * @return No return value
 * */
TestResult AK_bitmap_test();

/**
 * @author Lovro Predovan
 * @brief Function that deletes bitmap index based on the name of index
 * @param Bitmap index table name
   @return No return value
 **/
void AK_delete_bitmap_index(char *indexName);

/**
 * @author Lovro Predovan
 * @brief Function that updates the index. Function deletes and recreates the index values again if different number of params is detected 
 * @param tableName name of table
 * @param attributeName name of attribute
 * @param newAttributeValue new value of updated attribute
 * @return No return value
 **/
void AK_add_to_bitmap_index(char *tableName, char *attributeName);

#endif
