/**
@file query_optimization.c Provides functions for general query optimization
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
 * @file query_optimization.c
 * @brief Implements optimization routines for relational algebra expressions.
 */

#include "../auxi/test.h"
#include "query_optimization.h"
#include "rel_eq_selection.h"
#include "rel_eq_projection.h"
#include "rel_eq_assoc.h"
#include "rel_eq_comut.h"
#include "../auxi/test.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../auxi/auxiliary.h"


/**
 * @author Antonio Brković
 * @brief Parses a space-separated string into a list_node expression.
 * @param str The expression string.
 * @return Pointer to the head of the list representing the expression.
 */
struct list_node *AK_parse_expression_string(const char *str) {
    struct list_node *list = NULL;
    char *copy = strdup(str);
    char *token = strtok(copy, " ");
    while (token != NULL) {
        AK_InsertAtEnd_L3(0, token, 0, &list);
        token = strtok(NULL, " ");
    }
    free(copy);
    return list;
}

/**
 * @author Antonio Brković
 * @brief Executes the selected relational algebra optimization.
 * @param list_query Input expression list.
 * @param rel_eq Character indicating the optimization type.
 * @return Optimized expression list.
 */
struct list_node *AK_execute_rel_eq(struct list_node *list_query, char rel_eq) {
    if (rel_eq == 's') return AK_rel_eq_selection(list_query);
    else if (rel_eq == 'p') return AK_rel_eq_projection(list_query);
    else if (rel_eq == 'a') return AK_rel_eq_assoc(list_query);
    else if (rel_eq == 'c') return AK_rel_eq_comut(list_query);
    return list_query;
}

void AK_FreeList(struct list_node** head) {
    while (*head) {
        struct list_node* temp = *head;
        *head = (*head)->next;
        free(temp);
    }
}


/**
 * @author Antonio Brković
 * @brief Test function for query optimization pipeline.
 * @return TestResult indicating success or failure.
 */
TestResult AK_query_optimization_test() {
    TestResult result = TEST_result(1, 0);

    struct list_node *query = AK_parse_expression_string("selection cond selection A");
    struct list_node *optimized = AK_execute_rel_eq(query, 's');

    // Provjera očekivanog ponašanja (selection cond A)
    if (!optimized || !optimized->next || !optimized->next->next ||
        strcmp(optimized->data, "selection") != 0 ||
        strcmp(optimized->next->data, "cond") != 0 ||
        strcmp(optimized->next->next->data, "A") != 0) {
        result.testSucceded = 0;
        result.testFailed = 1;
    }

    AK_FreeList(query);
    AK_FreeList(optimized);
    return result;
}
