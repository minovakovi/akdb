/**
@file btree.c Header file that provides functions for BTree indices
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

#include "btree.h"

/**
  * @author Anđelko Spevec
  * @brief Function that creates new btree index on integer attribute in table
  * @param tblName - name of the table on which we are creating index
  * @param attributes - attribute on which we are creating index
  * @param indexName - name of the index
 */
/* 
NOTES: 1. For now we assume that table is sorted on attribute on which we are
creating index. We should check that with filesort.c and include it in this function.
That file had some errors, so I couldn't test it. 2.working with multiple blocks
*/
AK_block * AK_btree_create(char *tblName, struct list_node *attributes, char *indexName){
	int i,n,exist;
	table_addresses *addresses;
	int num_attr;
	AK_PRO;
	
	//get table address
	addresses = (table_addresses*) AK_get_table_addresses(tblName);
	num_attr = AK_num_attr(tblName);

	while(addresses->address_from[ i ]){
		printf("\nAddress of the TABLE is from %u to %u \n",(addresses->address_from[ i ]),(addresses->address_to[ i ]));
		i++;
	}
	
	//get table header
	AK_header *table_header = (AK_header *)AK_get_header(tblName);
    	struct list_node *attribute = (struct list_node *) AK_First_L2(attributes);
	AK_header i_header[ MAX_ATTRIBUTES ];
	AK_header* temp;
    	n = 0;
    	
    	//get table attributes
	while (attribute != 0) {
		exist = 0;
		for (i = 0; i < num_attr; i++) {
		    if (strcmp((table_header + i)->att_name, attribute->data) == 0) {
			printf("Attribute %s exist in table, found on position: %d\n", (table_header + i)->att_name, i);
			AK_dbg_messg(HIGH, INDICES, "Attribute %s exist in table, found on position: %d\n", (table_header + i)->att_name, i);
			exist = 1;
			temp = (AK_header*) AK_create_header((table_header + i)->att_name, (table_header + i)->type, FREE_INT, FREE_CHAR, FREE_CHAR);
			memcpy(i_header + n, temp, sizeof ( AK_header));
			n++;
			if ((table_header)->type != TYPE_INT) {
			    printf("Unsupported data type for bree index! Only int!");
			    AK_EPI;
			    return NULL;
			    //return EXIT_ERROR;
			}
		    }
		}
		if (!exist) {
		    printf("Attribute %s does not exists in table", attribute->data);
		    AK_EPI;
		    return NULL;
		    //return EXIT_ERROR;
		}
		attribute = attribute->next;
	} 
	memset(i_header + n, 0, MAX_ATTRIBUTES - n);

	//creating index
	int startAddress = AK_initialize_new_segment(indexName, SEGMENT_TYPE_INDEX, i_header);
	if (startAddress != EXIT_ERROR)
		printf("\nINDEX %s CREATED!\n", indexName);
        int r;
	table_addresses *addIndex = (table_addresses*) AK_get_index_addresses(indexName);
	while(addIndex->address_from[ r ]){
		printf("\nAddress of the INDEX is from %u to %u \n",(addIndex->address_from[ r ]),(addIndex->address_to[ r ]));
		r++;
	}
	AK_block *block = (AK_block*) AK_read_block(startAddress);
	int adr_to_write = (int) AK_find_AK_free_space(AK_get_index_addresses(indexName));
	int number_el = AK_get_num_records(tblName);			
	root_info *rootEl = (root_info*) AK_malloc(sizeof(root_info));

	//number of LEAFS--------------------B=3---------------number_leaf
	float x = (float) number_el;
	float y = x/B;
	int number_leaf = 0;
	while(y > 0){
		y = y - 1;
		number_leaf = number_leaf + 1;
	}//instead of ceil function
	rootEl->level[0] = number_leaf;
	
	//number of NODES (level above leafs)--------(B+1)-----number_node
	float m = (float) number_leaf;
	float node = m/(B+1);
	int number_node = 0;
	while(node > 0){
		node = node - 1;
		number_node = number_node + 1;
	}//instead of ceil function
	rootEl->level[1] = number_node;

	//number of NODES (remaining nodes)---------(B+1)------number_node
	float temp_one = (float) number_node;
	int temp_two = 0;
	int lvl = 2;
	int go = 1;
	while(go == 1){
		node = temp_one/(B+1);
		temp_two = 0;
		while(node > 0){
			node = node - 1;
			temp_two = temp_two + 1;
		}
		rootEl->level[lvl] = temp_two;
		lvl++;
		number_node = number_node + temp_two;
		temp_one = (float) temp_two;
		if(temp_two == 1)
			go = 0;
	}
	
	//writing root_info
	rootEl->root = number_leaf + number_node;
	memcpy(block->data, rootEl, sizeof(root_info));
	block->tuple_dict[0].address = 0;
	block->tuple_dict[0].type = BLOCK_TYPE_NORMAL;
    	block->tuple_dict[0].size = sizeof (root_info);
	block->AK_free_space += sizeof (root_info);
	block->last_tuple_dict_id = 0;
	AK_write_block(block);
	
	//writting leafs
	int id,lf,b,help=0,elements=0;
	int help_two[number_el];
	for(lf=1;lf<=(rootEl->level[0]);lf++){
		btree_node *leaf = (btree_node*) AK_malloc(sizeof(btree_node));
		i=0;
		AK_block *temp = (AK_block*) AK_read_block(addresses->address_from[ i ]);

		for(b=0;b<B;b++){
			if(elements<number_el){
				leaf->pointers[b].addBlock = addresses->address_from[ i ];
				leaf->pointers[b].indexTd = help;
				void *tem = (void *) AK_calloc(MAX_VARCHAR_LENGTH, sizeof (void));
	      		        memcpy(tem, &(temp->data[temp->tuple_dict[help].address]), sizeof (int));
				leaf->values[b] = *((int *) tem);
				help_two[elements] = leaf->values[b];
				help += num_attr;
				elements++;
			}else{
				leaf->values[b] = -1;
				elements++;
			}
		}
		if(lf<(rootEl->level[0])){
			leaf->pointers[B].addBlock = addresses->address_from[ i ];
			leaf->pointers[B].indexTd = lf+1;
		}	
		memcpy(&block->data[block->AK_free_space], leaf, sizeof(btree_node));    
		id = block->last_tuple_dict_id + 1;
		block->tuple_dict[id].address = block->AK_free_space;
		block->tuple_dict[id].type = LEAF;
		block->AK_free_space += sizeof(btree_node);
		block->last_tuple_dict_id = id;
		AK_write_block(block);
	}
	
	//writting nodes
	int no=1,v=1,d,e=0,f,h,offset,diff,skip=1,check;
	while(no==1){
		check=0;
		offset=1;
		for(h=0;h<e;h++)////instead of pow function
			offset = offset*(B+1); 
		diff = offset * B;//difference WITHIN node
		offset = offset * B + 1;
		for(d=0;d<(rootEl->level[v]);d++){
			btree_node *nodes = (btree_node*) AK_malloc(sizeof(btree_node));
			for(f=0;f<B;f++){
				if((offset-1)<number_el){
					nodes->values[f] = help_two[offset-1];
				}else{
					nodes->values[f] = -1;
				}
				offset += diff;//difference WITHIN node
			}
			offset += diff;//difference BETWEEN nodes is doubled
			for(f=0;f<=B;f++){
					if(check<(rootEl->level[v-1])){
						nodes->pointers[f].indexTd = skip;
						nodes->pointers[f].addBlock = adr_to_write;
						skip++;
						check++;
					}	
			}
			memcpy(&block->data[block->AK_free_space], nodes, sizeof(btree_node)); 
			id = block->last_tuple_dict_id + 1;
			block->tuple_dict[id].address = block->AK_free_space;
			block->tuple_dict[id].type = NODE;
			block->AK_free_space += sizeof(btree_node);
			block->last_tuple_dict_id = id;
			AK_write_block(block);
		}		
		e++;
		if(rootEl->level[v] == 1)
			no = 0;
		v++;
	}
	printf("B tree created succesfully.");
	AK_EPI;
	return block;
	//return EXIT_SUCCESS;
}

/**
  * @author unknown
  * @brief Function that deletes index
  * @param indexName - name of the index+
 */
int AK_btree_delete(char *indexName){
    AK_PRO;
    AK_delete_segment(indexName, SEGMENT_TYPE_INDEX);
    printf("INDEX %s DELETED!\n", indexName);
    AK_EPI;
}

/**
  * @author Anđelko Spevec
  * @brief Function that searches or deletes a value in btree index
  * @param indexName - name of the index
  * @param searchValue - value that we are searching in the index
  * @param endRange - if 0 search is for 0 value, else searching in range
  * @param toDo - if 0 we just search else we delete the element if we find it
 */
int AK_btree_search_delete(char *indexName,int *searchValue,int *endRange,int *toDo, AK_block * inputBlock){
	int contin = 1;
        AK_PRO;
	if((searchValue == endRange) || (endRange <= 0)){
		endRange = 0;
		contin = 0;
	}
	if(searchValue > endRange){
		int help = *searchValue;
		searchValue = endRange;
		endRange = &help;
	}
	
	//finding correct block 
	int adr_to_read = (int) AK_find_AK_free_space(AK_get_index_addresses(indexName));
	//AK_block *block = (AK_block*) AK_read_block(adr_to_read);     functions AK_find_AK_free_space, AK_get_index_addresses and AK_read_block should be checked for mistakes
	AK_block *block = inputBlock;
	root_info *root = (root_info*) AK_malloc(sizeof (root_info));
	memset(root, 0, sizeof (root_info));
	memcpy(root,block->data,sizeof (root_info));
	btree_node *temp = (btree_node*) AK_malloc(sizeof(btree_node));
	
	//navigating through the tree
	int address= block->tuple_dict[root->root].address;
	int type = block->tuple_dict[root->root].type; //node == 1, leaf == 0
	while(type == 1){
		memset(temp,0,sizeof(btree_node));
		memcpy(temp,&block->data[address],sizeof(btree_node));
		int b,goTo = B,done=0;
		for(b=0;b<B;b++){
			if((*searchValue<(temp->values[b])) && (done == 0)){
				goTo=b;
				done = 1;
			}
		
		}
		address = block->tuple_dict[temp->pointers[goTo].indexTd].address;
		type = block->tuple_dict[temp->pointers[goTo].indexTd].type;
	}
	memset(temp,0,sizeof(btree_node));
	memcpy(temp,&block->data[address],sizeof(btree_node));
	int f,found = 0,idNext;
	
	//searching for value
	for(f=0;f<B;f++){
		if(*searchValue == (temp->values[f])){
			found = 1;
		}
		if((found == 1) && (temp->values[f] <= *endRange) && (*endRange > 0)) {
			//printing data after finding node
			printf("\n Value %i found! Block %u - IDX_TBL = %u", temp->values[f], temp->pointers[f].addBlock, temp->pointers[f].indexTd);
				
			//deleting node
			if(*toDo == 1){
				btree_delete(temp,block,address,f);
				AK_write_block(block);
			}
		}
		idNext=temp->pointers[B].indexTd;
	}
	if(found == 0){
		printf("\n Value not found!");
	}
	while(contin == 1){
		memset(temp,0,sizeof(btree_node));
		memcpy(temp,&block->data[block->tuple_dict[idNext].address],sizeof(btree_node));
		
		//searching for value
		for(f=0;f<B;f++){
			if((temp->values[f]) <= *endRange){
				//printing data after finding node
				printf("\n Value %i found! Block %u - IDX_TBL = %u", temp->values[f], temp->pointers[f].addBlock, temp->pointers[f].indexTd);
				
				//deleting node
				if(*toDo == 1){
					btree_delete(temp,block,idNext,f);
				}
			}else
				contin = 0;
		idNext=temp->pointers[B].indexTd;
		AK_write_block(block);
		}
	}
	AK_EPI;
	return EXIT_SUCCESS;
}


/**
  * @author Anđelko Spevec
  * @brief Function that deletes a value in btree index
  * @param temp - node for deletion
  * @param block - block that contains binary tree
  * @param idNext - index of the node that is to be deleted
 */
void btree_delete(btree_node * temp, AK_block * block, int idNext, int i){
	temp->values[i] = -1;
	temp->pointers[i].addBlock = 0;
	temp->pointers[i].indexTd = 0;
	memcpy(&block->data[idNext],temp,sizeof(btree_node));
	printf("\n Value deleted!");
}

/**
  * @author unknown
  * @brief Function that inserts a value in btree index
  * @param indexName - name of the index
  * @param insertValue - value for insert
  * @param insertTd - index table destination
  * @param insertBlock - block address
  * @param inputBlock - block containing btree
  * @param EXIT_SUCCESS if successful 
 */
int AK_btree_insert(char *indexName,int *insertValue, int *insertTd, int *insertBlock, AK_block * inputBlock){
	AK_PRO;
	//finding correct block           !!!functions AK_find_AK_free_space, AK_get_index_addresses and AK_read_block should be checked for mistakes!!!
	int adr_to_read = (int) AK_find_AK_free_space(AK_get_index_addresses(indexName));
	//AK_block *block = (AK_block*) AK_read_block(adr_to_read);    
	AK_block *block = inputBlock;
	
	//assign root node to root_info variable
	root_info *root = (root_info*) AK_malloc(sizeof (root_info));
	memset(root, 0, sizeof (root_info));
	memcpy(root,block->data,sizeof (root_info));
	
	//assign values to properties of root node
	btree_node *temp = (btree_node*) AK_malloc(sizeof(btree_node));
	int insertPath[ORDER];//id of the selected node, iP[0] root
	int insertIndexPath[ORDER];//index of a pointer selected in node, iIP[0] is index selected in root
	int address = block->tuple_dict[root->root].address;
	insertPath[0] = root->root;
	int type = block->tuple_dict[root->root].type;
	int inc=1;
	
	//navigating through the tree
	while(type == 1){
		memset(temp,0,sizeof(btree_node));
		memcpy(temp,&block->data[address],sizeof(btree_node));
		int b,goTo = B,done=0;
		for(b=0;b<B;b++){
			if((*insertValue<(temp->values[b])) && (done == 0)){
				goTo=b;
				done = 1;
			}
		}
		insertIndexPath[inc-1] = goTo;
		insertPath[inc] = temp->pointers[goTo].indexTd;
		address = block->tuple_dict[temp->pointers[goTo].indexTd].address;
		type = block->tuple_dict[temp->pointers[goTo].indexTd].type;
		if(type == 1){
			inc++;
		}//node == 1, leaf == 0
	}
	
	//assign values to properties of leaf node
	int j;
	int destinationAddress = block->tuple_dict[insertPath[inc]].address;
	memset(temp,0,sizeof(btree_node));
	memcpy(temp,&block->data[destinationAddress],sizeof(btree_node));
	int AK_freeSpace = 0;
	for(j=0;j<B;j++){
		if(temp->values[j] == -1){
			AK_freeSpace++;
		}
	}
	int inserted = 0,increase=0;
	btree_node *temp_help = (btree_node*) AK_malloc(sizeof(btree_node));
	memset(temp_help,0,sizeof(btree_node));
	//we have FREE space in LEAF
	if(AK_freeSpace != 0){ 
		temp_help = searchValue(inserted, *insertValue, temp, temp_help, *insertTd,*insertBlock,&increase,-1);
		memset(&block->data[destinationAddress],0,sizeof(btree_node));
		memcpy(&block->data[destinationAddress],temp_help,sizeof(btree_node));
		AK_write_block(block);
		printf("\nNew value is added in leaf with available space");	
	}
	//we have to SPLIT the LEAF
	else{
		btree_node *temp_help_two = (btree_node*) AK_malloc(sizeof(btree_node));
		memset(temp_help_two,0,sizeof(btree_node));
		
		//assign values to temporary node
		temp_help = searchValue(inserted, *insertValue, temp, temp_help, *insertTd,*insertBlock,&increase,findCorrectNumber(1));
		temp_help->pointers[B].indexTd = block->last_tuple_dict_id + 1; //points to new leaf
		temp_help->pointers[B].addBlock = adr_to_read; //update when fix multiple blocks
		memset(&block->data[destinationAddress],0,sizeof(btree_node));
		memcpy(&block->data[destinationAddress],temp_help,sizeof(btree_node)); 
		
		//assign values to temporary node
		temp_help_two = searchValue(inserted, *insertValue, temp, temp_help_two, *insertTd,*insertBlock,&increase,findCorrectNumber(2));
		temp_help_two->pointers[B].indexTd = temp->pointers[B].indexTd; //points to next leaf
		temp_help_two->pointers[B].addBlock = temp->pointers[B].addBlock;
		memcpy(&block->data[block->AK_free_space], temp_help_two, sizeof(btree_node));
		 
		//assign values to block
		int id = block->last_tuple_dict_id + 1;
		block->tuple_dict[id].address = block->AK_free_space;
		block->tuple_dict[id].type = LEAF;
		block->AK_free_space += sizeof(btree_node);
		block->last_tuple_dict_id = id;
		
		//update root_info->level[0] number of leafs
		root->level[0]++;
		memcpy(&block->data[0],root,sizeof(root_info));
		AK_write_block(block);
		
		//checking nodes above 
		int pathIndex = inc - 1;
		int nodeAddress = block->tuple_dict[insertPath[pathIndex]].address;
		int pointerIndex = insertIndexPath[pathIndex];
		memset(temp,0,sizeof(btree_node));
		memcpy(temp,&block->data[nodeAddress],sizeof(btree_node));

		memset(temp_help,0,sizeof(btree_node));
		int AK_freePointer = 0;
		for(j=0;j<B;j++){
			if(temp->values[j] == -1)
				AK_freePointer = 1;
		}
		
		int startSecond = 0, copyNode = 1,skip = 1;
		if(AK_freePointer == 1){//in node above we have FREE POINTER to insert NEW LEAF
			//adding value
			temp_help = setNodePointers(temp_help,temp,pointerIndex,temp_help_two->values[0],insertPath[inc],id);
			memset(&block->data[nodeAddress],0,sizeof(btree_node));
			memcpy(&block->data[nodeAddress],temp_help,sizeof(btree_node)); 
			AK_write_block(block);
			printf("\nNew value is added in new leaf. Node above had available space.");
		}else{//spliting nodes
			int nodeIncrease,nodeInserted,helpAddress,helpType,nodeAboveAddress;
			int again = 1,newRoot=0,updateIdx = 1;
			
			btree_node *temp_node_one = (btree_node*) AK_malloc(sizeof(btree_node));
			btree_node *temp_node_two = (btree_node*) AK_malloc(sizeof(btree_node));
			btree_node *value_help = (btree_node*) AK_malloc(sizeof(btree_node));
			
			while(again == 1){
				memset(temp_node_one,0,sizeof(btree_node));
				memset(temp_node_two,0,sizeof(btree_node));
				memset(value_help,0,sizeof(btree_node));

				nodeIncrease = 0;
				nodeInserted = 0;
				helpType=NODE;
				
				//pointers for first node
				temp_node_one = findPointers(temp_node_one, temp, id, &nodeInserted, &nodeIncrease, 3, pointerIndex);
				
				//values for first node
				temp_node_one = findValues(temp_node_one, block, &helpAddress, &helpType, value_help);
				
				//pointers for second node
				temp_node_two = findPointers(temp_node_two, temp, id, &nodeInserted, &nodeIncrease, 4, pointerIndex);
				
				//values for second node
				temp_node_two = findValues(temp_node_two, block, &helpAddress, &helpType, value_help);
				
				//updating block with btree
				memset(&block->data[block->tuple_dict[insertPath[pathIndex]].address],0,sizeof(btree_node));
				memcpy(&block->data[block->tuple_dict[insertPath[pathIndex]].address],temp_node_one,sizeof(btree_node)); 
				memcpy(&block->data[block->AK_free_space],temp_node_two,sizeof(btree_node)); 
				id = block->last_tuple_dict_id + 1;
				block->tuple_dict[id].address = block->AK_free_space;
				block->tuple_dict[id].type = NODE;
				block->AK_free_space += sizeof(btree_node);
				block->last_tuple_dict_id = id;
				root->level[updateIdx]++;
				memcpy(&block->data[0],root,sizeof(root_info));
				updateIdx++;
				AK_write_block(block);

				if(newRoot == 1){
					again = 0;
					memset(temp,0,sizeof(btree_node));
					temp->pointers[0].indexTd = insertPath[pathIndex];
					temp->pointers[1].indexTd = id;
					helpAddress = block->tuple_dict[id].address;
					helpType = block->tuple_dict[id].type;
					memset(value_help,0,sizeof(btree_node));
					memcpy(value_help,&block->data[helpAddress],sizeof(btree_node));			
					while(helpType == NODE){
						helpAddress = block->tuple_dict[value_help->pointers[0].indexTd].address;
						helpType = block->tuple_dict[value_help->pointers[0].indexTd].type;
						memset(value_help,0,sizeof(btree_node));
						memcpy(value_help,&block->data[helpAddress],sizeof(btree_node));
					}
					temp->values[0] = value_help->values[0];
					printf("\nNew value is added and tree is updated! New root added, so order of the tree increased!");
					//updating block with btree
					memcpy(&block->data[block->AK_free_space],temp,sizeof(btree_node)); 
					id = block->last_tuple_dict_id + 1;
					block->tuple_dict[id].address = block->AK_free_space;
					block->tuple_dict[id].type = NODE;
					block->AK_free_space += sizeof(btree_node);
					block->last_tuple_dict_id = id;
					root->level[updateIdx] = 1;
					root->root = id;
					memcpy(&block->data[0],root,sizeof(root_info));
					AK_write_block(block);	
				}else{
					//get above node !
					if(pathIndex != 0)
						pathIndex = pathIndex - 1; //0, insertPath[0] = 13 (root)
					nodeAboveAddress = block->tuple_dict[insertPath[pathIndex]].address;
					memset(temp,0,sizeof(btree_node));
					memcpy(temp,&block->data[nodeAboveAddress],sizeof(btree_node));
					
					for(j=0;j<B;j++){
						if(temp->values[j] == -1)
							AK_freePointer = 1;
					}
					if(AK_freePointer == 1){
						again = 0;
						increase = 0;
						memset(temp_help,0,sizeof(btree_node));
						for(j=0;j<=B;j++){
							if(j == (insertIndexPath[pathIndex]+1)){
								temp_help->pointers[j].indexTd = id;
							}else{
								temp_help->pointers[j].indexTd = temp->pointers[increase].indexTd;
								increase++;
							}	
						}
						//updating block with btree
						temp_help = findValues(temp_help, block, &helpAddress, &helpType, value_help);
						memcpy(&block->data[block->AK_free_space],temp,sizeof(btree_node)); 
						id = block->last_tuple_dict_id + 1;
						block->tuple_dict[id].address = block->AK_free_space;
						block->tuple_dict[id].type = NODE;
						block->AK_free_space += sizeof(btree_node);
						block->last_tuple_dict_id = id;
						AK_write_block(block);	
					}else{//if root is full->new root...
						if(root->root == insertPath[pathIndex])
							newRoot = 1;
						else
							again = 1;
					}
				}
			}
			if(newRoot != 1)
				printf("\nNew value is added and tree is updated!");
		}
	}
	AK_EPI;
	return EXIT_SUCCESS;
}


/**
  * @author unknown
  * @brief Function that sets values for node
  * @param temp_help - node that has it's values set
  * @param insertValue - value for insert
  * @param insertTd - index table destination
  * @param insertBlock - block address
  * @param i - determins the index of element of node
  * @return  node that has it's values set 
 */
btree_node * makevalues(btree_node * temp_help, int insertValue, int insertTd, int insertBlock, int i){
	temp_help->values[i] = insertValue;
	temp_help->pointers[i].indexTd = insertTd;
	temp_help->pointers[i].addBlock = insertBlock;
	return temp_help;
}

/**
  * @author unknown
  * @brief returns data about a leaf
  * @return required value
 */
int findCorrectNumber(int number){
	int amount = B % 2;
	if(amount == 0){//uneven number of pointers (B+1)%2 = 1
		//we need to insert B+1 elements in two leafs -> 4+1=3+2
		if(number == 1)
			return (B/2) + 1; //B=4,(4/2)+1=3 elements in first leaf
		if(number == 2)
			return B/2; //B=4,(4/2)=2 elements in second leaf
		if(number == 3)
			return (B + 2)/2;  //B=4, (B+1+1)/2 = 3 pointers in first LEAF
		if(number == 4)
			return (B + 2)/2; //B=4, (B+1+1)/2 = 3 pointers in second LEAF
	}else{//even number of pointers (B+1)%2 = 0
		//we need to insert B+1 elements in two leafs -> 3+1=2+2
		if(number == 1)
			return (B + 1)/2; //B=3,(3+1)/2=2 elements in first leaf
		if(number == 2)
			return (B + 1)/2; //B=3,(3+1)/2=2 elements in second leaf 
		if(number == 3)
			return ((B + 1)/2) + 1; //B=3, ((3+1)/2)+1= 3 pointers in first leaf
		if(number == 4)
			return (B + 1)/2; //B=3 (3+1)/2=2 pointers in second leaf			
	}
}

/**
  * @author unknown
  * @brief Function that sets values for node
  * @param inserted - determins if a value in a tree smaller then the value for insert has been found
  * @param insertValue - value for insert
  * @param temp - node with data about existing node
  * @param temp_help - node that has it's values set
  * @param insertTd - index table destination
  * @param insertBlock - block address
  * @param increase - determins the index of element of a node
  * @param number - gives information about the number of elements in a leaf
  * @return  node that has it's values set 
 */
btree_node * searchValue(int inserted, int insertValue, btree_node * temp, btree_node * temp_help, int *insertTd, int *insertBlock,int* increase, int number){
	int stop = 0;
	for(int j=0;j<B;j++){
		if(stop == 0){
			if(inserted == 0){
				if(insertValue < temp->values[*increase]){
					temp_help = makevalues(temp_help,insertValue,*insertTd,*insertBlock,j);
					inserted = 1;
				}else{
					temp_help = makevalues(temp_help,temp->values[*increase],temp->pointers[*increase].indexTd,temp->pointers[*increase].addBlock,j);
					*increase++;
				}
			}else{
				if(temp->values[*increase] == -1 && number == -1){
					temp_help = makevalues(temp_help,insertValue,*insertTd,*insertBlock,j);
					inserted = 1;
				}else{
					temp_help = makevalues(temp_help,temp->values[*increase],temp->pointers[*increase].indexTd,temp->pointers[*increase].addBlock,j);
					*increase++;
				}	
			}
			if((j+1) == number)
				stop = 1;
		}else{	
			if(number != -1) 
				temp_help = makevalues(temp_help,-1,0,0,j);
			else{
				temp_help->values[j] = temp->values[*increase];
				temp_help->pointers[j] = temp->pointers[*increase];
				*increase++;
			}
		}		
	}
	return temp_help;
}

/**
  * @author unknown
  * @brief Function that sets values for node
  * @param temp - node with data about existing node
  * @param temp_help - node that has it's values set
  * @param pointerIndex - indicates what pointer is used
  * @param secondValue - value of a node
  * @param firstPointer - value to which a pointer of a node is to be set
  * @param secondPointer - value to which a pointer of a node is to be set
  * @return  node that has it's values set 
 */
btree_node * setNodePointers(btree_node * temp, btree_node * temp_help,int pointerIndex,int secondValue,int firstPointer,int secondPointer){
	int firstValue = temp_help->values[0];
	int AK_freePointer = 0;
	for(int j=0;j<B;j++){
		if(temp->values[j] == -1)
			AK_freePointer = 1;
	}
	int startSecond = 0, copyNode = 1,skip = 1;
	int increase = 0;
	temp_help->pointers[0].indexTd = temp->pointers[0].indexTd;
	if(pointerIndex == 0){
		startSecond = 1;
		copyNode = 0;
		skip = 0;
	}
	for(int j=1;j<=B;j++){
		if((copyNode == 1) && (j != pointerIndex)){
			temp_help->values[j-1] = temp->values[increase];
			temp_help->pointers[j].indexTd = temp->pointers[increase+1].indexTd;
			increase++;
		}
		if(startSecond == 1){
			temp_help->values[j-1] = secondValue;
			temp_help->pointers[j].indexTd = secondPointer;
			if(skip == 1){
				increase++;
			}
			startSecond = 0;
			copyNode = 1;
		}
		if(j == pointerIndex){
			temp_help->values[j-1] = firstValue;
			temp_help->pointers[j].indexTd = firstPointer;
			startSecond = 1;
			copyNode = 0;
		}
	}
	return temp_help;
}

/**
  * @author unknown
  * @brief Function that sets values for node
  * @param temp_node_one - node that has it's values set
  * @param temp - node with data about existing node
  * @param id - value to which a pointer of a node is to be set
  * @param nodeInserted - determins if a node has value
  * @param nodeIncrease - shows node that is currently watched
  * @param number - determins the way node values are checked
  * @param pointerIndex - indicates what pointer is used
  * @return  node that has it's values set 
 */
btree_node * findPointers(btree_node * temp_node_one, btree_node * temp, int id, int* nodeInserted,int* nodeIncrease, int number, int pointerIndex){
	for(int j=0;j<findCorrectNumber(number);j++){
		int compare;
		if(number == 3)
			compare = j;
		else
			compare = *nodeIncrease;
		if((compare==(pointerIndex+1)) && (*nodeInserted == 0)){
			temp_node_one->pointers[j].indexTd = id;
			temp_node_one->pointers[j].addBlock = 0;//update when fixing blocks
			if(number == 3)
				*nodeInserted = 1;
		}
		else{
			temp_node_one->pointers[j].indexTd = temp->pointers[*nodeIncrease].indexTd;
			temp_node_one->pointers[j].addBlock = temp->pointers[*nodeIncrease].addBlock;
			*nodeIncrease++;
		}
	}
	return temp_node_one;
}

/**
  * @author unknown
  * @brief Function that sets values for node
  * @param temp_node_one - node that has it's values set
   * @param block - block containing btree
  * @param helpAddress - address of current node
  * @param helpType - type of current node
  * @param value_help - node in helpAddress
  * @return  node that has it's values set 
 */
btree_node * findValues(btree_node * temp_node_one, AK_block * block, int* helpAddress,int* helpType, btree_node *value_help){
	for(int j=0;j<B;j++){
		if(temp_node_one->pointers[j+1].indexTd != 0){
			*helpAddress = block->tuple_dict[temp_node_one->pointers[j+1].indexTd].address;
			*helpType = block->tuple_dict[temp_node_one->pointers[j+1].indexTd].type;
			memset(value_help,0,sizeof(btree_node));
			memcpy(value_help,&block->data[*helpAddress],sizeof(btree_node));			
			while(*helpType == NODE){
				*helpAddress = block->tuple_dict[value_help->pointers[0].indexTd].address;
				*helpType = block->tuple_dict[value_help->pointers[0].indexTd].type;
				memset(value_help,0,sizeof(btree_node));
				memcpy(value_help,&block->data[*helpAddress],sizeof(btree_node));
			}
			temp_node_one->values[j] = value_help->values[0];
		}
	}
	return temp_node_one;
}

/**
 * @author unknown
 * @brief Returns the amount of successful and failed tests.
 * @return TestResult
 */ 
TestResult AK_btree_test() {
	int passed_tests=0;
    	int failed_tests=0;
    	
	char *tblName = "student";
	char *indexName = "student_btree_index";
	AK_PRO;
	struct list_node *att_list = (struct list_node *) AK_malloc(sizeof (struct list_node));
	AK_Init_L3(&att_list);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "mbr\0", 4, att_list);

	AK_block * block = AK_btree_create(tblName, att_list, indexName);
	if(block == NULL){
		failed_tests++;
	}
	else{
		passed_tests++;
	}
	printf("\n\n---------------------------");
	printf("\nInserting new value...\n");
	int iv=39123;
	int ii=101;
	int ib=301;
	int *insertValue = &iv; //insert in leaf with available space
	//int *insertValue = 35901; //insert in full leaf -> new leaf, full node above - split
	int *insertTd = &ii;
	int *insertBlock = &ib;
	if(AK_btree_insert(indexName,insertValue,insertTd,insertBlock,block) == EXIT_SUCCESS){
		passed_tests++;
	}
	else{
		failed_tests++;
	}
	//use insert function for 39123 and again for 39000 to test spliting leaf with AK_free space above
	iv=39000;
	insertValue = &iv; //insert in full leaf -> new leaf, AK_free space in node above
	if(AK_btree_insert(indexName,insertValue,insertTd,insertBlock,block) == EXIT_SUCCESS){
		passed_tests++;
	}
	else{
		failed_tests++;
	}
	printf("\n\n---------------------------");
	printf("\nSearching value...\n");
	int sv=35906;
	int er=35916;
	int td=1;
	int *searchValue = &sv;
	int *endRange = &er;
	//int *endRange = 0;
	int *toDo = &td;//0 search, 1 delete
	if(AK_btree_search_delete(indexName, searchValue, endRange, toDo, block) == EXIT_SUCCESS){
		passed_tests++;
	}
	else{
		failed_tests++;
	}
	printf("\n");
	AK_EPI;
	return TEST_result(passed_tests,failed_tests);
}

