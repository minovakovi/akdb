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
 * @file rel_eq_assoc.c Provides functions for relational equivalences regarding associativity
 */

#include "rel_eq_assoc.h"
#include "../auxi/auxiliary.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @author Antonio Brković
 * @brief Applies associativity transformation on relational algebra expressions.
 * @param list_rel_eq Pointer to the list of relational expressions.
 * @return A new list with transformed expressions, if applicable.
 */
struct list_node* AK_rel_eq_assoc(struct list_node** list_rel_eq) {
    if (!list_rel_eq || !(*list_rel_eq)) return NULL;

    struct list_node* result_list = (struct list_node*) AK_calloc(1, sizeof(struct list_node));
    if (!result_list) return NULL;
    AK_Init_L3(&result_list);

    struct list_node* current = AK_First_L2(*list_rel_eq);
    while (current) {
        char* expr = AK_Retrieve_L2(current, *list_rel_eq);

        if (expr && strncmp(expr, "((", 2) == 0) {
            const char* p = expr + 2; // skip "(("
            const char* delim1 = strstr(p, "⋈");
            if (!delim1) goto copy_original;

            size_t lenA = delim1 - p;
            const char* p2 = delim1 + strlen("⋈");

            const char* delim2 = strchr(p2, ')');
            if (!delim2) goto copy_original;

            size_t lenB = delim2 - p2;
            const char* p3 = strstr(delim2 + 1, "⋈");
            if (!p3) goto copy_original;

            const char* p4 = p3 + strlen("⋈");
            const char* delim3 = strchr(p4, ')');
            if (!delim3) goto copy_original;

            size_t lenC = delim3 - p4;

            char a[64], b[64], c[64];
            strncpy(a, p, lenA); a[lenA] = '\0';
            strncpy(b, p2, lenB); b[lenB] = '\0';
            strncpy(c, p4, lenC); c[lenC] = '\0';

            char reformatted[256];
            snprintf(reformatted, sizeof(reformatted), "(%s⋈(%s⋈%s))", a, b, c);
            AK_InsertAtEnd_L3(1, reformatted, strlen(reformatted) + 1, result_list);
        } else {
copy_original:
            if (expr)
                AK_InsertAtEnd_L3(1, expr, strlen(expr) + 1, result_list);
        }

        current = AK_Next_L2(current);
    }

    return result_list;
}

/**
 * @author Antonio Brković
 * @brief Svi testovi za asocijativnost unutar jedne funkcije.
 * @return TestResult s brojem uspješnih i neuspješnih testova.
 */
TestResult AK_rel_eq_assoc_all_tests() {
    AK_PRO;
    printf("*** AK_rel_eq_assoc_all_tests ***\n");

    int ok = 0, fail = 0;

    // --- TEST 1: Regularna asocijacija ---
    {
        struct list_node* list = (struct list_node*) AK_calloc(1, sizeof(struct list_node));
        AK_Init_L3(&list);
        AK_InsertAtEnd_L3(1, "((A⋈B)⋈C)", strlen("((A⋈B)⋈C)") + 1, list);

        struct list_node* out = AK_rel_eq_assoc(&list);
        char* result = AK_Retrieve_L2(AK_End_L2(out), out);

        if (result && strcmp(result, "(A⋈(B⋈C))") == 0) {
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
        AK_InsertAtEnd_L3(1, "(A⋈(B⋈C))", strlen("(A⋈(B⋈C))") + 1, list);

        struct list_node* out = AK_rel_eq_assoc(&list);
        char* result = AK_Retrieve_L2(AK_End_L2(out), out);

        if (result && strcmp(result, "(A⋈(B⋈C))") == 0) {
            printf("TEST 2 OK\n");
            ok++;
        } else {
            printf("TEST 2 FAIL: dobiveno = %s\n", result ? result : "NULL");
            fail++;
        }
    }

    // --- TEST 3: Malformiran izraz ---
    {
        struct list_node* list = (struct list_node*) AK_calloc(1, sizeof(struct list_node));
        AK_Init_L3(&list);
        AK_InsertAtEnd_L3(1, "(A⋈B⋈C)", strlen("(A⋈B⋈C)") + 1, list);

        struct list_node* out = AK_rel_eq_assoc(&list);
        char* result = AK_Retrieve_L2(AK_End_L2(out), out);

        if (result && strcmp(result, "(A⋈B⋈C)") == 0) {
            printf("TEST 3 OK\n");
            ok++;
        } else {
            printf("TEST 3 FAIL: dobiveno = %s\n", result ? result : "NULL");
            fail++;
        }
    }

    // --- TEST 4: Više izraza, samo jedan vrijedi ---
    {
        struct list_node* list = (struct list_node*) AK_calloc(1, sizeof(struct list_node));
        AK_Init_L3(&list);
        AK_InsertAtEnd_L3(1, "(X⋈(Y⋈Z))", strlen("(X⋈(Y⋈Z))") + 1, list);
        AK_InsertAtEnd_L3(1, "((A⋈B)⋈C)", strlen("((A⋈B)⋈C)") + 1, list);

        struct list_node* out = AK_rel_eq_assoc(&list);
        char* last = AK_Retrieve_L2(AK_End_L2(out), out);

        if (last && strcmp(last, "(A⋈(B⋈C))") == 0) {
            printf("TEST 4 OK\n");
            ok++;
        } else {
            printf("TEST 4 FAIL: dobiveno = %s\n", last ? last : "NULL");
            fail++;
        }
    }

    // --- TEST 5: Potpuno neispravan unos ---
    {
        struct list_node* list = (struct list_node*) AK_calloc(1, sizeof(struct list_node));
        AK_Init_L3(&list);
        AK_InsertAtEnd_L3(1, "nevalidan izraz", strlen("nevalidan izraz") + 1, list);

        struct list_node* out = AK_rel_eq_assoc(&list);
        char* result = AK_Retrieve_L2(AK_End_L2(out), out);

        if (result && strcmp(result, "nevalidan izraz") == 0) {
            printf("TEST 5 OK\n");
            ok++;
        } else {
            printf("TEST 5 FAIL: dobiveno = %s\n", result ? result : "NULL");
            fail++;
        }
    }

    AK_EPI;
    return TEST_result(ok, fail);
}
