/**
 * @file rel_eq_selection.h
 * @brief Header file that provides function declarations for optimizing
 *        relational algebra expressions using selection equivalences.
 *
 * This module defines transformation functions for merging chained selection
 * operations into a single selection with a combined predicate.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef REL_EQ_SELECTION_H
#define REL_EQ_SELECTION_H

#include "../auxi/auxiliary.h"
#include "../auxi/test.h"

struct list_node* AK_rel_eq_selection(struct list_node* expression_list);

TestResult AK_rel_eq_selection_test();
TestResult AK_rel_eq_selection_single_test();
TestResult AK_rel_eq_selection_all_tests();  // Dodano za grupno testiranje

#endif
