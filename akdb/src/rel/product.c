/**
@file product.c Provides functions for relational product operation
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

#include "product.h"

/**
 * @author Dino Laktašić
 * @brief  Function that makes the structure of an empty destination table for product operation
 * @param srcTable1 name of the first table
 * @param srcTable2 name of the second table
 * @param dstTable name of the product table
 * @result Created destination table as a result of product operation 
 * @return if success returns EXIT_SUCCESS, else returns EXIT_ERROR
 */
int AK_product(char *srcTable1, char *srcTable2, char *dstTable)
{
	AK_PRO;
	table_addresses *src_addr1 = (table_addresses *)AK_get_table_addresses(srcTable1);
	table_addresses *src_addr2 = (table_addresses *)AK_get_table_addresses(srcTable2);

	int startAddress1 = src_addr1->address_from[0];
	int startAddress2 = src_addr2->address_from[0];

	if ((startAddress1 != 0) && (startAddress2 != 0))
	{
		//initialize new segment
		int num_att1 = AK_num_attr(srcTable1);
		int num_att2 = AK_num_attr(srcTable2);

		register int i, j, k, m, n, o, u;

		int address, type, size;

		// will be needed later as a place to hold cell data
		char celldata[MAX_VARCHAR_LENGTH];

		AK_block *tbl1_temp_block = (AK_block *)AK_read_block(startAddress1);
		AK_block *tbl2_temp_block = (AK_block *)AK_read_block(startAddress2);

		//Currently it works with headers no longer than MAX_ATTRIBUTES. The same header is written in all allocated table blocks.
		//This is wrong and need to be corrected.
		//If header doesn't fit in the first block than system must write the remain attributes from header to the new block.
		//Correction must be handled in all functions that write, read or count header attributes.
		int head = 0;
		AK_header header[MAX_ATTRIBUTES];

		/* Adding header to new aggregation table */
		while (strcmp(tbl1_temp_block->header[head].att_name, "") != 0)
		{
			memcpy(&header[head], &tbl1_temp_block->header[head], sizeof(tbl1_temp_block->header[head]));
			head++;
		}

		head = 0;
		while (strcmp(tbl2_temp_block->header[head].att_name, "") != 0)
		{
			memcpy(&header[head + num_att1], &tbl2_temp_block->header[head], sizeof(tbl2_temp_block->header[head]));
			head++;
		}

		/* renaming column names which are same in both tables */
		for (i = 0; i < num_att1 + num_att2; i++)
		{
			for (j = i + 1; j < num_att1 + num_att2; j++)
			{
				if (strcmp(header[i].att_name, header[j].att_name) == 0)
				{
					char column_name[MAX_ATT_NAME]; // name of column which is same in both tables

					strcpy(column_name, header[i].att_name);

					strcpy(header[i].att_name, srcTable1);
					strcat(header[i].att_name, ".");
					strcat(header[i].att_name, column_name);

					strcpy(header[j].att_name, srcTable2);
					strcat(header[j].att_name, ".");
					strcat(header[j].att_name, column_name);
					break;
				}
			}
		}

		/* initializing new table with header */
		AK_initialize_new_segment(dstTable, SEGMENT_TYPE_TABLE, header);

		AK_dbg_messg(LOW, REL_OP, "\nTABLE %s CREATED from %s and %s\n", dstTable, srcTable1, srcTable2);
		AK_dbg_messg(MIDDLE, REL_OP, "\nAK_product: start copying data\n");

		/*product procedure function */
		AK_product_procedure(srcTable1, srcTable2, dstTable, header);

		AK_dbg_messg(LOW, REL_OP, "PRODUCT_TEST_SUCCESS\n\n");
		AK_EPI;
		return EXIT_SUCCESS;
	}
	else
	{
		AK_dbg_messg(LOW, REL_OP, "\n AK_product: Table/s doesn't exist!");
		AK_free(src_addr1);
		AK_free(src_addr2);
		AK_EPI;
		return EXIT_ERROR;
	}
}

/**
 * @author Dino Laktašić, Fabijan Josip Kraljić
 * @brief  Functions that iterates trough both tables and concates rows comparing headers and their row values.
 * @param srcTable1 name of the first table
 * @param srcTable2 name of the second table
 * @param dstTable name of the product table
 * @result destination table filled with data sized n(rows srcTable1)*m(rows srcTable2)
 * @param header header of product table
 */
void AK_product_procedure(char *srcTable1, char *srcTable2, char *dstTable, AK_header header[MAX_ATTRIBUTES])
{
	table_addresses *src_addr1 = (table_addresses *)AK_get_table_addresses(srcTable1);
	table_addresses *src_addr2 = (table_addresses *)AK_get_table_addresses(srcTable2);

	int startAddress1 = src_addr1->address_from[0];
	int startAddress2 = src_addr2->address_from[0];

	//getting tables
	AK_mem_block *tbl1_temp_block = (AK_mem_block *)AK_get_block(startAddress1);
	AK_mem_block *tbl2_temp_block = (AK_mem_block *)AK_get_block(startAddress2);

	//initialize new segment
	int num_att1 = AK_num_attr(srcTable1);
	int num_att2 = AK_num_attr(srcTable2);

	// will be needed later as a place to hold cell data
	char celldata[MAX_VARCHAR_LENGTH];

	struct list_node *row_root = (struct list_node *)AK_malloc(sizeof(struct list_node));
	AK_Init_L3(&row_root);
	
	register j, k, l, m, n, o, u;
	
	/**
	* Product procedure
	* Going through one table, and for each row in it, going through another table,
	* and joining rows that way!
	*/
	register i = 0;	
	/* for each extent in first table */
	while (src_addr1->address_from[i] != 0)
	{
		/* for each block in extent (first table) */
		for (j = src_addr1->address_from[i]; j < src_addr1->address_to[i]; j++)
		{
			//tbl1_temp_block = (AK_block *)AK_read_block(j);
			tbl1_temp_block = (AK_mem_block *)AK_get_block(j);
			if (tbl1_temp_block->block->last_tuple_dict_id == 0)
			{
				break;
			}

			/* for each row in block (first table)... */
			for (k = 0; k < tbl1_temp_block->block->last_tuple_dict_id; k += num_att1)
			{

				// for each row in block, we have to read every other row from the second table
				m = 0;
				/* for each extent in other table */
				while (src_addr2->address_from[m] != 0)
				{
					/* for each block in extent (other table) */
					for (n = src_addr2->address_from[m]; n < src_addr2->address_to[m]; n++)
					{
						//tbl2_temp_block = (AK_block *)AK_read_block(n);
						tbl2_temp_block = (AK_mem_block *)AK_get_block(n);
						if (tbl2_temp_block->block->last_tuple_dict_id == 0)
						{
							break;
						}

						/* for each row in block (other table) ... */
						for (o = 0; o < tbl2_temp_block->block->last_tuple_dict_id; o += num_att2)
						{
							/* now we just have to copy cell data from one and from another row */
							int cellid, celltype, celladdress, cellsize;
							/* first table first */
							for (u = 0; u < num_att1; u++)
							{
								cellid = k + u;
								celltype = tbl1_temp_block->block->tuple_dict[cellid].type;
								celladdress = tbl1_temp_block->block->tuple_dict[cellid].address;
								cellsize = tbl1_temp_block->block->tuple_dict[cellid].size;
								memset(celldata, '\0', MAX_VARCHAR_LENGTH);
								memcpy(celldata, &(tbl1_temp_block->block->data[celladdress]), cellsize);
								if (celltype == TYPE_VARCHAR)
								{
									celldata[cellsize] = '\0';
								}

								AK_Insert_New_Element(celltype, celldata, dstTable,
													  header[u].att_name, row_root);
							}

							/* and then rows from the second table rows come into play */
							for (u = 0; u < num_att2; u++)
							{
								cellid = o + u;
								celltype = tbl2_temp_block->block->tuple_dict[cellid].type;
								celladdress = tbl2_temp_block->block->tuple_dict[cellid].address;
								cellsize = tbl2_temp_block->block->tuple_dict[cellid].size;
								memset(celldata, '\0', MAX_VARCHAR_LENGTH);
								memcpy(celldata, &(tbl2_temp_block->block->data[celladdress]), cellsize);
								if (celltype == TYPE_VARCHAR)
								{
									celldata[cellsize] = '\0';
								}

								AK_Insert_New_Element(celltype, celldata, dstTable,
													  header[num_att1 + u].att_name, row_root);
							}

							/* finally, inserting new row in table and emptying linked list */
							AK_insert_row(row_root);
							AK_DeleteAll_L3(&row_root);
						}
					}
					m++;
				}
			}
		}
		i++;
	}
	AK_free(row_root);
}

/**
 * @author Dino Laktašić, Fabijan Josip Kraljić
 * @brief  Function for product operator testing, where it is given 2 source table on which product operations are managed.
 * @result Product destination table and number od passed tests. 
 * @return Test result - number of successful and unsuccessful tests.
 */
TestResult AK_op_product_test()
{

	//Setting tables for testing
	AK_PRO;
	char *sys_table = "AK_relation";
	char *destTable = "product_test";
	char *tblName1 = "employee";
	char *tblName2 = "department";

	int test;

	printf("\n********** PRODUCT TEST **********\n\n");

	//Checking if destination tables already exists
	if (AK_if_exist(destTable, sys_table) == 0)
	{
		printf("Table %s does not exist!\n", destTable);
		//Execution of main function that retrieves int value
		test = AK_product(tblName1, tblName2, destTable);

		AK_EPI;
		if (test == EXIT_SUCCESS)
		{
			AK_print_table("employee");
			AK_print_table("department");
			//prints table size n*m rows
			AK_print_table(destTable);
			return TEST_result(1, 0);
		}
		else
		{
			AK_print_table("product_test");
			return TEST_result(0, 1);
		}
	}
	else
	{
		printf("Table %s already exists!\n", destTable);
		AK_print_table(destTable);
		return TEST_result(1, 0);
	}
}
