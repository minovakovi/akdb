/**
 * @file rel_eq_selection.c Provides functions for relational equivalences in selection
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

#include "rel_eq_projection.h"
#include "../auxi/auxiliary.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * @author Antonio Brković
 * @brief Applies projection merge optimization to relational algebra expressions.
 * @param list_rel_eq Pointer to input list of expressions.
 * @return New list with optimized projections.
 */
struct list_node* AK_rel_eq_projection(struct list_node** list_rel_eq) {
    if (!list_rel_eq || !(*list_rel_eq)) return NULL;

    struct list_node* result = NULL;
AK_Init_L3(&result);


    struct list_node* current = AK_First_L2(*list_rel_eq);
    while (current) {
        char* expr = AK_Retrieve_L2(current, *list_rel_eq);
        if (!expr) {
            current = AK_Next_L2(current);
            continue;
        }

        if (strncmp(expr, "projection", 10) == 0 && strstr(expr + 10, "projection")) {
            char* ptr = expr + 10;
            while (*ptr == ' ') ptr++;  // preskoči razmake

            char* end = strchr(ptr, ' ');
            size_t len = end ? (size_t)(end - ptr) : strlen(ptr);
            char attrs[128] = {0};
            strncpy(attrs, ptr, len);
            attrs[len] = '\0';

            char new_expr[256];
            snprintf(new_expr, sizeof(new_expr), "projection %s", attrs);
            AK_InsertAtEnd_L3(1, new_expr, strlen(new_expr) + 1, result);
        } else {
            AK_InsertAtEnd_L3(1, expr, strlen(expr) + 1, result);
        }

        current = AK_Next_L2(current);
    }

    return result;
}

/**
 * @author Antonio Brković
 * @brief Basic test for merging nested projections.
 * @return TestResult
 */
TestResult AK_rel_eq_projection_test() {
    TestResult result = {0};
    result.implemented = 1;

    struct list_node* list = NULL;
    AK_Init_L3(&list);
    AK_InsertAtEnd_L3(1, "projection A,B projection C,D R", strlen("projection A,B projection C,D R") + 1, list);

    struct list_node* out = AK_rel_eq_projection(&list);
    char* value = AK_Retrieve_L2(AK_End_L2(out), out);

    if (value && strcmp(value, "projection A,B") == 0) {
        result.testSucceded++;
    } else {
        printf("OČEKIVANO: projection A,B\nDOBIVENO:  %s\n", value ? value : "(null)");
        result.testFailed++;
    }

    AK_FreeList_L2(&list);
    AK_FreeList_L2(&out);
    return result;
}

/**
 * @author Antonio Brković
 * @brief Test with a single projection (should not change).
 * @return TestResult
 */
TestResult AK_rel_eq_projection_single_test() {
    TestResult result = {0};
    result.implemented = 1;

    struct list_node* list = NULL;
    AK_Init_L3(&list);
    AK_InsertAtEnd_L3(1, "projection A,B R", strlen("projection A,B R") + 1, list);

    struct list_node* out = AK_rel_eq_projection(&list);
    char* value = AK_Retrieve_L2(AK_End_L2(out), out);

    if (value && strcmp(value, "projection A,B R") == 0) {
        result.testSucceded++;
    } else {
        printf("OČEKIVANO: projection A,B R\nDOBIVENO:  %s\n", value ? value : "(null)");
        result.testFailed++;
    }

    AK_FreeList_L2(&list);
    AK_FreeList_L2(&out);
    return result;
}

/**
 * @author Antonio Brković
 * @brief Test with malformed projection input (should pass through unchanged).
 * @return TestResult
 */
TestResult AK_rel_eq_projection_malformed_test() {
    TestResult result = {0};
    result.implemented = 1;

    struct list_node* list = NULL;
    AK_Init_L3(&list);
    AK_InsertAtEnd_L3(1, "select A projection B", strlen("select A projection B") + 1, list);

    struct list_node* out = AK_rel_eq_projection(&list);
    char* value = AK_Retrieve_L2(AK_End_L2(out), out);

    if (value && strcmp(value, "select A projection B") == 0) {
        result.testSucceded++;
    } else {
        result.testFailed++;
    }

    AK_FreeList_L2(&list);
    AK_FreeList_L2(&out);
    return result;
}

/**
 * @author Antonio Brković
 * @brief Executes all tests for projection optimization.
 * @return TestResult indicating how many tests passed or failed.
 */
TestResult AK_rel_eq_projection_all_tests() {
    TestResult total = {0};
    total.implemented = 1;

    TestResult r1 = AK_rel_eq_projection_test();
    TestResult r2 = AK_rel_eq_projection_single_test();
    TestResult r3 = AK_rel_eq_projection_malformed_test();

    total.testSucceded += r1.testSucceded + r2.testSucceded + r3.testSucceded;
    total.testFailed += r1.testFailed + r2.testFailed + r3.testFailed;

    if (r1.testFailed) printf("TEST 1 FAIL\n"); else printf("TEST 1 OK\n");
    if (r2.testFailed) printf("TEST 2 FAIL\n"); else printf("TEST 2 OK\n");
    if (r3.testFailed) printf("TEST 3 FAIL\n"); else printf("TEST 3 OK\n");

    return total;
}

/**
 * @author Antonio Brković
 * @brief Extracts attributes from relational algebra projection expressions.
 * @param list_rel_eq List of relational algebra expressions.
 * @return Extracted attribute string, or NULL on failure.
 */
char* AK_rel_eq_get_atrributes_char(struct list_node** list_rel_eq) {
    if (!list_rel_eq || !(*list_rel_eq)) return NULL;

    struct list_node* current = AK_First_L2(*list_rel_eq);
    while (current) {
        char* expr = AK_Retrieve_L2(current, *list_rel_eq);
        if (expr && strncmp(expr, "projection ", 10) == 0) {
            char* attrs = expr + 10;
            char* space = strchr(attrs, ' ');
            size_t len = space ? (size_t)(space - attrs) : strlen(attrs);
            char* result = (char*) AK_calloc(len + 1, sizeof(char));
            if (!result) return NULL;
            strncpy(result, attrs, len);
            result[len] = '\0';
            return result;
        }
        current = AK_Next_L2(current);
    }

    return NULL;
}
