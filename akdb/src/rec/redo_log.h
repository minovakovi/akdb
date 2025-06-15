/*
/**
@file redo_log.h Header file that provides functions and defines for redo_log.h
*
 *  Created on: Apr 30, 2012
 *      Author: gigai
 */

#ifndef REDOLOG
#define REDOLOG

#include "../auxi/auxiliary.h"
#include "../mm/memoman.h"
#include "../auxi/constants.h"
#include "../auxi/configuration.h"
#include "../auxi/debug.h"
#include "../rec/archive_log.h"
#include "../file/table.h"

#include "../auxi/mempro.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"


/**
 * @author @author Krunoslav Bilić updated by Dražen Bandić, second update by Tomislav Turek
 * @brief Function that adds a new element to redolog
 * @return EXIT_FAILURE if not allocated memory for ispis, otherwise EXIT_SUCCESS
 */
int AK_add_to_redolog(int command, struct list_node *row_root);

void AK_generate_select_query_identifier(const char *srcTable, struct list_node *attributes, struct list_node *condition, char* buffer, size_t buffer_size);

/**
 * @author Danko Bukovac
 * @brief Function that adds a new select to redolog, commented code with the new select from select.c,
 * current code works with selection.c
 * @return EXIT_FAILURE if not allocated memory for ispis, otherwise EXIT_SUCCESS
 */
int AK_add_to_redolog_select(int command, struct list_node *condition, char *srcTable);

int AK_add_to_redolog_select_with_result(int command, const char *srcTable, struct list_node *attributes, struct list_node *condition, AK_results *result, const char* query_identifier);

/**
 * @author Danko Bukovac
 * @brief Function that checks redolog for select, works only with selection.c, not select.c
 * @return 0 if select was not found, otherwise 1
 */
int AK_check_redo_log_select(int command, struct list_node *condition, char *srcTable);

int AK_check_redo_log_select_for_caching(const char *srcTable, struct list_node *attributes, struct list_node *condition, const char* query_identifier, AK_results **out_result, time_t *out_logged_at_timestamp);

/**
 * @author Krunoslav Bilić updated by Dražen Bandić, second update by Tomislav Turek
 * @brief Function that prints out the content of redolog memory
 * @return No return value.
 */
void AK_printout_redolog();


void AK_redolog_commit();

/**
 * @author Dražen Bandić
 * @brief Function that checks if the attribute contains '|', and if it does it replaces it with "\|"
 * @return new attribute
 */
char* AK_check_attributes(char *attributes);

// NOVO: Dodane deklaracije za pomoćne funkcije
void AK_free_cached_select_result_deep(AK_results* res);

AK_results* AK_create_results_from_table(const char* tableName);

#endif /* REDOLOG */
