/**
@file theta_join.h Header file that provides data structures for theta-join
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


#ifndef THETA_JOIN
#define THETA_JOIN

#include "expression_check.h"

int AK_theta_join(char *srcTable1, char * srcTable2, char * dstTable, AK_list *constraints);

#endif /* THETA_JOIN */

int AK_create_theta_join_header(char *srcTable1, char * srcTable2, char *new_table);
void AK_check_constraints(AK_block *tbl1_temp_block, AK_block *tbl2_temp_block, int tbl1_num_att, int tbl2_num_att, AK_list *constraints, char *new_table);
int AK_theta_join(char *srcTable1, char * srcTable2, char * dstTable, AK_list *constraints);
void AK_op_theta_join_test();