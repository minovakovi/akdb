/**
 * @file query_optimization.h
 * @brief Header file that provides functions and definitions for general query optimization.
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

/**
 * @file query_optimization.h
 * @brief Header for query optimization routines.
 */

#ifndef QUERY_OPTIMIZATION_H
#define QUERY_OPTIMIZATION_H

#include "../auxi/test.h"
#include "../auxi/auxiliary.h"

void AK_FreeList(struct list_node** head);
struct list_node *AK_parse_expression_string(const char *str);
struct list_node *AK_execute_rel_eq(struct list_node *expression_list, char rel_eq);
TestResult AK_query_optimization_test();

#endif // QUERY_OPTIMIZATION_H
