/**
@file product.h Header file that provides functions and defines for relational product operation
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

#ifndef PRODUCT
#define PRODUCT

#include "../auxi/test.h"
#include "../file/table.h"
#include "../file/files.h"
#include "../auxi/mempro.h"
#include "../sql/drop.h"

/**
 * @author Dino Laktašić
 * @brief  Function that makes the structure of an empty destination table for product operation
 * @param srcTable1 name of the first table
 * @param srcTable2 name of the second table
 * @param dstTable name of the product table
 * @return if success returns EXIT_SUCCESS, else returns EXIT_ERROR
 */
int AK_product(char *srcTable1, char * srcTable2, char * dstTable);

/**
 * @author Dino Laktašić, Fabijan Josip Kraljić
 * @brief  Functions that iterates trough both tables and concates rows. The result is in destination table.
 * @param srcTable1 name of the first table
 * @param srcTable2 name of the second table
 * @param dstTable name of the product table
 * @param header header of product table
 */
void AK_product_procedure(char *srcTable1, char * srcTable2, char * dstTable, AK_header header[MAX_ATTRIBUTES]);
TestResult AK_op_product_test();

#endif

