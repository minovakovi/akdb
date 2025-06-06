/**
 * @file rel_eq_comut.c Provides functions for relational equivalences regarding commutativity
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301, USA
 */

#include "rel_eq_comut.h"
#include "../auxi/auxiliary.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * @author Antonio Brković
 * @brief Helper to remove outer parentheses from a string (in-place).
 */
static char* strip_outer_parens(char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    if (len >= 2 && s[0] == '(' && s[len - 1] == ')') {
        s[len - 1] = '\0';
        return s + 1;
    }
    return s;
}

/**
 * @author Antonio Brković
 * @brief Applies commutativity transformation on relational algebra expressions.
 * @param list_rel_eq Pointer to input list of relational algebra expressions.
 * @return New list with transformed expressions where possible.
 */
struct list_node* AK_rel_eq_comut(struct list_node** list_rel_eq) {
    if (!list_rel_eq || !(*list_rel_eq)) return NULL;

    struct list_node* result_list = (struct list_node*) AK_calloc(1, sizeof(struct list_node));
    if (!result_list) return NULL;
    AK_Init_L3(&result_list);

    struct list_node* current = AK_First_L2(*list_rel_eq);
    while (current) {
        char* expr = AK_Retrieve_L2(current, *list_rel_eq);

        if (expr && expr[0] == '(' && strstr(expr, "⋈") && expr[strlen(expr) - 1] == ')') {
            char* copy = strdup(expr + 1); // preskoči '('
            if (!copy) goto copy_original;
            copy[strlen(copy) - 1] = '\0'; // ukloni zadnju ')'

            char* left = strtok(copy, "⋈");
            char* right = strtok(NULL, "⋈");

            if (left && right) {
                left = strip_outer_parens(left);
                right = strip_outer_parens(right);

                char new_expr[128];
                snprintf(new_expr, sizeof(new_expr), "(%s⋈%s)", right, left);
                AK_InsertAtEnd_L3(1, new_expr, strlen(new_expr) + 1, result_list);
            } else {
copy_original:
                AK_InsertAtEnd_L3(1, expr, strlen(expr) + 1, result_list);
            }

            free(copy);
        } else {
copy_original_alt:
            if (expr)
                AK_InsertAtEnd_L3(1, expr, strlen(expr) + 1, result_list);
        }

        current = AK_Next_L2(current);
    }

    return result_list;
}

/**
 * @author Antonio Brković
 * @brief Pokreće sve testove za komutativnost s ispisom rezultata.
 * @return TestResult s brojem uspješnih i neuspješnih testova.
 */
TestResult AK_rel_eq_comut_all_tests() {
    AK_PRO;
    printf("*** AK_rel_eq_comut_all_tests ***\n");

    int ok = 0, fail = 0;

    // --- TEST 1: Regularna komutacija ---
    {
        struct list_node* list = (struct list_node*) AK_calloc(1, sizeof(struct list_node));
        AK_Init_L3(&list);
        AK_InsertAtEnd_L3(1, "(A⋈B)", strlen("(A⋈B)") + 1, list);

        struct list_node* out = AK_rel_eq_comut(&list);
        char* result = AK_Retrieve_L2(AK_End_L2(out), out);

        if (result && strcmp(result, "(B⋈A)") == 0) {
            printf("TEST 1 OK\n");
            ok++;
        } else {
            printf("TEST 1 FAIL: dobiveno = %s\n", result ? result : "NULL");
            fail++;
        }
    }

    // --- TEST 2: Već transformiran izraz ---
    {
        struct list_node* list = (struct list_node*) AK_calloc(1, sizeof(struct list_node));
        AK_Init_L3(&list);
        AK_InsertAtEnd_L3(1, "(B⋈A)", strlen("(B⋈A)") + 1, list);

        struct list_node* out = AK_rel_eq_comut(&list);
        char* result = AK_Retrieve_L2(AK_End_L2(out), out);

        if (result && strcmp(result, "(A⋈B)") == 0) {
            printf("TEST 2 OK\n");
            ok++;
        } else {
            printf("TEST 2 FAIL: dobiveno = %s\n", result ? result : "NULL");
            fail++;
        }
    }

    // --- TEST 3: Neispravan izraz ---
    {
        struct list_node* list = (struct list_node*) AK_calloc(1, sizeof(struct list_node));
        AK_Init_L3(&list);
        AK_InsertAtEnd_L3(1, "A⋈B", strlen("A⋈B") + 1, list);

        struct list_node* out = AK_rel_eq_comut(&list);
        char* result = AK_Retrieve_L2(AK_End_L2(out), out);

        if (result && strcmp(result, "A⋈B") == 0) {
            printf("TEST 3 OK\n");
            ok++;
        } else {
            printf("TEST 3 FAIL: dobiveno = %s\n", result ? result : "NULL");
            fail++;
        }
    }

    // --- TEST 4: Više izraza, miješani slučajevi ---
    {
        struct list_node* list = (struct list_node*) AK_calloc(1, sizeof(struct list_node));
        AK_Init_L3(&list);
        AK_InsertAtEnd_L3(1, "(X⋈Y)", strlen("(X⋈Y)") + 1, list);
        AK_InsertAtEnd_L3(1, "neispravno", strlen("neispravno") + 1, list);
        AK_InsertAtEnd_L3(1, "(C⋈D)", strlen("(C⋈D)") + 1, list);

        struct list_node* out = AK_rel_eq_comut(&list);
        char* last = AK_Retrieve_L2(AK_End_L2(out), out);

        if (last && strcmp(last, "(D⋈C)") == 0) {
            printf("TEST 4 OK\n");
            ok++;
        } else {
            printf("TEST 4 FAIL: dobiveno = %s\n", last ? last : "NULL");
            fail++;
        }
    }

    // --- TEST 5: Ulaz NULL ---
    {
        struct list_node* out = AK_rel_eq_comut(NULL);
        if (out == NULL) {
            printf("TEST 5 OK\n");
            ok++;
        } else {
            printf("TEST 5 FAIL: očekivan NULL\n");
            fail++;
        }
    }

    AK_EPI;
    return TEST_result(ok, fail);
}
