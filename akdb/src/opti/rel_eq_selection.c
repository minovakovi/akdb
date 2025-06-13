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

#include "rel_eq_selection.h"
#include "../auxi/auxiliary.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * @author Antonio Brković
 * @brief Applies selection merge optimization to relational algebra expressions.
 * @param expression_list List of input expressions.
 * @return Optimized expression list.
 */
struct list_node* AK_rel_eq_selection(struct list_node* expression_list) {
    if (!expression_list) return NULL;

    struct list_node* result = (struct list_node*) AK_calloc(1, sizeof(struct list_node));
    if (!result) return NULL;
    AK_Init_L3(&result);

    struct list_node* current = AK_First_L2(expression_list);
    while (current) {
        char* expr = AK_Retrieve_L2(current, expression_list);

        if (expr && strncmp(expr, "selection", 9) == 0) {
            // pokušaj pronaći drugu selection
            char* first_sel = expr + 9;
            char* second_sel = strstr(first_sel, "selection");
            if (second_sel) {
                char cond1[128], cond2[128], rel[64];
                memset(cond1, 0, sizeof(cond1));
                memset(cond2, 0, sizeof(cond2));
                memset(rel, 0, sizeof(rel));

                size_t len1 = second_sel - first_sel;
                strncpy(cond1, first_sel, len1);
                cond1[len1] = '\0';

                // cond2 + rel su u ostatku
                strncpy(cond2, second_sel + 9, sizeof(cond2) - 1);
                cond2[sizeof(cond2) - 1] = '\0';

                // pronađi zadnju riječ — relaciju
                char* last_space = strrchr(cond2, ' ');
                if (last_space) {
                    strncpy(rel, last_space + 1, sizeof(rel) - 1);
                    rel[sizeof(rel) - 1] = '\0';
                    *last_space = '\0';  // ukloni relaciju iz cond2

                    char new_expr[512];
                    snprintf(new_expr, sizeof(new_expr), "selection (%sAND %s) %s", cond1, cond2, rel);
                    AK_InsertAtEnd_L3(1, new_expr, strlen(new_expr) + 1, result);
                    current = AK_Next_L2(current);
                    continue;
                }
            }
        }

        // fallback
        if (expr)
            AK_InsertAtEnd_L3(1, expr, strlen(expr) + 1, result);

        current = AK_Next_L2(current);
    }

    return result;
}


/**
 * @author Antonio Brković
 * @brief Pokreće sve testove za selection spajanje.
 * @return TestResult
 */
TestResult AK_rel_eq_selection_all_tests() {
    AK_PRO;
    printf("*** AK_rel_eq_selection_all_tests ***\n");

    int ok = 0, fail = 0;

    // TEST 1: Spajanje dvaju selekcija
    {
        struct list_node* list = (struct list_node*) AK_calloc(1, sizeof(struct list_node));
        AK_Init_L3(&list);
        AK_InsertAtEnd_L3(1, "selection A > 5 selection B < 10 R", strlen("selection A > 5 selection B < 10 R") + 1, list);
        struct list_node* out = AK_rel_eq_selection(list);
        char* val = AK_Retrieve_L2(AK_End_L2(out), out);
        if (val && strstr(val, "AND")) {
            printf("TEST 1 OK\n");
            ok++;
        } else {
            printf("TEST 1 FAIL: %s\n", val ? val : "NULL");
            fail++;
        }
    }

    // TEST 2: Samo jedna selekcija
    {
        struct list_node* list = (struct list_node*) AK_calloc(1, sizeof(struct list_node));
        AK_Init_L3(&list);
        AK_InsertAtEnd_L3(1, "selection C > 20 R", strlen("selection C > 20 R") + 1, list);
        struct list_node* out = AK_rel_eq_selection(list);
        char* val = AK_Retrieve_L2(AK_End_L2(out), out);
        if (val && strcmp(val, "selection C > 20 R") == 0) {
            printf("TEST 2 OK\n");
            ok++;
        } else {
            printf("TEST 2 FAIL: %s\n", val ? val : "NULL");
            fail++;
        }
    }

    // TEST 3: Prazna lista
    {
        struct list_node* out = AK_rel_eq_selection(NULL);
        if (out == NULL) {
            printf("TEST 3 OK\n");
            ok++;
        } else {
            printf("TEST 3 FAIL: očekivan NULL\n");
            fail++;
        }
    }

    // TEST 4: Više selekcija
    {
        struct list_node* list = (struct list_node*) AK_calloc(1, sizeof(struct list_node));
        AK_Init_L3(&list);
        AK_InsertAtEnd_L3(1, "selection A = 1 selection B = 2 R", strlen("selection A = 1 selection B = 2 R") + 1, list);
        AK_InsertAtEnd_L3(1, "selection C = 3 R", strlen("selection C = 3 R") + 1, list);
        struct list_node* out = AK_rel_eq_selection(list);
        char* last = AK_Retrieve_L2(AK_End_L2(out), out);
        if (last && strcmp(last, "selection C = 3 R") == 0) {
            printf("TEST 4 OK\n");
            ok++;
        } else {
            printf("TEST 4 FAIL: %s\n", last ? last : "NULL");
            fail++;
        }
    }

    // TEST 5: Nevalidan izraz
    {
        struct list_node* list = (struct list_node*) AK_calloc(1, sizeof(struct list_node));
        AK_Init_L3(&list);
        AK_InsertAtEnd_L3(1, "project A,B R", strlen("project A,B R") + 1, list);
        struct list_node* out = AK_rel_eq_selection(list);
        char* val = AK_Retrieve_L2(AK_End_L2(out), out);
        if (val && strcmp(val, "project A,B R") == 0) {
            printf("TEST 5 OK\n");
            ok++;
        } else {
            printf("TEST 5 FAIL: %s\n", val ? val : "NULL");
            fail++;
        }
    }

    AK_EPI;
    return TEST_result(ok, fail);
}
