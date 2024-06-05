/**
@file difference.h Header file that provides functions and defines for relational difference operation
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
#ifndef DIFFERENCE
#define DIFFERENCE

#include "../auxi/test.h"
#include "../file/table.h"
#include "../file/fileio.h"
#include "../auxi/mempro.h"
#include "../sql/drop.h"

/**
 * @author Dino Laktašić
 * @brief  Function that produces a difference of the two tables. Table addresses are get through names of tables.
 *         Specially start addresses are taken from them. They are used to allocate blocks for them. It is checked whether
           the tables have same table schemas. If not, it returns EXIT_ERROR. New segment for result of difference operation is
           initialized. Function compares every block in extent of the first table with every block in extent of second table. If there 
	   is a difference between their rows, they are put in dstTable.
 * @param srcTable1 name of the first table
 * @param srcTable2 name of the second table
 * @param dstTable name of the new table
 * @return if success returns EXIT_SUCCESS, else returns EXIT_ERROR
 */
int AK_difference(char *srcTable1, char *srcTable2, char *dstTable);
TestResult AK_op_difference_test();

#endif
