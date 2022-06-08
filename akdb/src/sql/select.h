/**
@file select.h Header file that provides functions  for select.h
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


#include "../file/table.h"
#include "../auxi/test.h"
#include "../file/fileio.h"
#include "../rel/selection.h"
#include "../rel/projection.h"
#include "../auxi/auxiliary.h"
#include "../auxi/mempro.h"
#include "../file/filesort.h"

/**
 * @author Filip Žmuk
 * @brief Function that implements SELECT relational operator
 * @param srcTable - original table that is used for selection
 * @param destTable - table that contains the result
 * @param condition - condition for selection
 * @param attributes - atributes to be selected
 * @param ordering - atributes for result sorting
 * @return EXIT_SUCCESS if cache result in memory and print table else break 
 */
int AK_select(char *srcTable,char *destTable,struct list_node *attributes,struct list_node *condition, struct list_node *ordering);
TestResult AK_select_test();
