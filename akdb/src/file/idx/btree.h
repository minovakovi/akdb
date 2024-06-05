/**
@file btree.h Header file that provides data strucures, functions and defines for BTree indices
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

#ifndef BTREE
#define BTREE

#define B 3
//order of the tree 6 - it can be lowered to usual 4-5 IF the B is increased!
#define ORDER 6
//now we have place for ((B+1)^(0RDER-1))*B = ((3+1)^(6-1))*3 = (4^5)*3 = 1024*3 = 3072 elements

//types for tuple_dict
#define LEAF 0
#define NODE 1

#include "../../auxi/test.h"
#include "index.h"
#include "../../file/table.h"
#include "../../auxi/constants.h"
#include "../../auxi/configuration.h"
#include "../../auxi/mempro.h"

typedef struct{
	//B values
	int values[B];
	/*
	B + 1 pointers
	IF type == 1 -> it points to other B+1 nodes/leafs
	IF type == 0 -> first B points to addresses in tblName
	and last pointer points to the next leaf (rangeSearch)
	*/
	struct_add pointers[B+1];
}btree_node;

/*
root_info->level[index] represents the number of nodes in 
that level from leafs to root, where root is always 1 (root doesn't
have to be on index 5 - if order of the tree is 3 it can be on 2 so we
distinct root lvl by value == 1). This is just theoretical representation
of the full usage of the array for B=3 and 0=6.

index|	     0 |       1 |       2 |       3 |       4 |       5 |
value| (B+1)^5 | (B+1)^4 | (B+1)^3 | (B+1)^2 | (B+1)^1 | (B+1)^0 |
level|   leaf  |         |         |         |         |   root  |
*/
typedef struct{
	//tuple_dic[ID] of the root node in the block
	int root;
	//array size of the 0RDER
	int level[ORDER]; 
}root_info;


/**
  * @author Anđelko Spevec
  * @brief Function that creates new btree index on integer attribute in table
  * @param tblName - name of the table on which we are creating index
  * @param attributes - attribute on which we are creating index
  * @param indexName - name of the index
 */
AK_block * AK_btree_create(char *tblName, struct list_node *attributes, char *indexName);
int AK_btree_delete(char *indexName);

btree_node * makevalues(btree_node * temp_help, int insertValue, int insertTd, int insertBlock, int i);
btree_node * searchValue(int inserted, int insertValue, btree_node * temp, btree_node * temp_help, int *insertTd, int *insertBlock,int* increase, int number);
btree_node * setNodePointers(btree_node * temp, btree_node * temp_help,int pointerIndex,int secondValue,int firstPointer,int secondPointer);
int findCorrectNumber(int number);
btree_node * findPointers(btree_node * temp_node_one, btree_node * temp, int id, int* nodeInserted,int* nodeIncrease, int number, int pointerIndex);
btree_node * findValues(btree_node * temp_node_one, AK_block * block, int* helpAddress,int* helpType, btree_node *value_help);
void btree_delete(btree_node * temp, AK_block * block, int idNext, int i);


/**
  * @author Anđelko Spevec
  * @brief Function that searches or deletes a value in btree index
  * @param indexName - name of the index
  * @param searchValue - value that we are searching in the index
  * @param endRange - if 0 search is for 0 value, else searching in range
  * @param toDo - if 0 we just search else we delete the element if we find it
 */
int AK_btree_search_delete(char *indexName,int *searchValue,int *endRange,int *toDo,AK_block * inputBlock);
int AK_btree_insert(char *indexName,int *insertValue, int *insertTd, int *insertBlock,AK_block * inputBlock);
TestResult AK_btree_test();

#endif
