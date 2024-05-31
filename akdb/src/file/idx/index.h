/**
@file index.h Header file that provides data structures, functions and defines for bitmap index
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

#ifndef INDEX
#define	INDEX

#include "../../auxi/mempro.h"
#include "../../file/table.h"
#include "../../file/fileio.h"
#include "../../file/files.h"

/**
  * @author Unknown
  * @struct struct_add
  * @brief Structure defining node address
 */
typedef struct {
	/// block address
    int addBlock;
    /// index table destination
    int indexTd;
} struct_add;

/**
  * @struct list_structure_add
  * @brief Structure that defines linked list node for index
 */
typedef struct list_structure_ad {
	/// attribute name
    char *attName;
    /// addresses
    struct_add add;
    /// next node pointer
    struct list_structure_ad *next;
} list_structure_ad;

typedef list_structure_ad *element_ad;
typedef list_structure_ad list_ad;


/**
 * @author Matija Šestak, modified for indexes by Lovro Predovan
 * @brief Function that examines whether there is a table with the name "tblName" in the system catalog (AK_relation)
 * @param tblName table name
 * @return returns 1 if table exist or returns 0 if table does not exist
 */
//printing index table
int AK_index_table_exist(char *indexTblName);

/**
 * @author Matija Šestak, modified for indexes by Lovro Predovan
 * @brief  Function that prints out the index table
 * @param *tblName table name
 * @return No return value
 */
void AK_print_index_table(char *indexTblName);

/**
 * @author Matija Šestak, modified for indexes by Lovro Predovan
 * @brief Function that gets value in some row and column
 * @param row zero-based row index
 * @param column zero-based column index
 * @param *tblName table name
 * @return value in the list
 */
struct list_node *AK_get_index_tuple(int row, int column, char *indexTblName);

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
int AK_get_index_num_records(char *indexTblName);

/**
 * @author Lovro Predovan
 * @brief Function that fetches the number of elements in a index table
 * @param index table name
 * @return No return value
 * */
int AK_num_index_attr(char *indexTblName);

struct list_node *AK_get_index_tuple(int row, int column, char *indexTblName);

/**
 * @author Unknown
 * @brief Function that initialises a linked list
 * @param list_ad *L linked list head
 * @return No return value
 * */
void AK_InitializelistAd(list_ad *L);

/**
 * @author Unknown
 * @brief Function that finds the first node of linked list
 * @param list_ad *L linked list head
 * @return Address of first node
 * */
element_ad AK_Get_First_elementAd(list_ad *L);

/**
 * @author Unknown
 * @brief Function that finds the last node of linked list
 * @param list_ad *L linked list head
 * @return Address of last node or 0 if list is empty
 * */
element_ad AK_Get_Last_elementAd(list_ad *L);

/**
 * @author Unknown
 * @brief Function that finds the next node of a node in linked list
 * @param Currentelement_op address of current node
 * @return Address of next node or 0 if current node is last in list
 * */
element_ad AK_Get_Next_elementAd(element_ad Currentelement_op);

/**
 * @author Unknown
 * @brief Function that finds the previous node of a node in linked list
 * @param Currentelement_op Address of current node
 * @param L previous element
 * @return Address of previous node or 0 if the current node is the head or the list is empty
 * */
element_ad AK_Get_Previous_elementAd(element_ad Currentelement_op, element_ad L) ;

/**
 * @author Unknown
 * @brief Function that finds the position of a node in linked list
 * @param Searchedelement_op address of current note
 * @param *L linked list head
 * @return Integer value of current node's order in the list
 * */
int AK_Get_Position_Of_elementAd(element_ad Searchedelement_op, list_ad *L);

/**
 * @author Unknown
 * @brief Function that deletes a node from a linked list
 * @param Deletedelement_op - address of node to delete
 * @param list_ad *L - list head
 * @return No return value
 * */
void AK_Delete_elementAd(element_ad Deletedelement_op, list_ad *L) ;

/**
 * @author Unknown
 * @brief Function that deletes all nodes in a linked list
 * @param L list head
 * @return No return value
 * */
void AK_Delete_All_elementsAd(list_ad *L) ;

/**
 * @author Unknown
 * @brief Function that inserts a new element into a linked list
 * @param addBlock address block
 * @param indexTd index table destination
 * @param *attname attribute name
 * @param elementBefore address of the node after which the new node will be inserted
 * @return No return value
 * */
void AK_Insert_NewelementAd(int addBlock, int indexTd, char *attName, element_ad elementBefore);

void AK_index_test();


#endif
