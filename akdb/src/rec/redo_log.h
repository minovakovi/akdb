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

/**
 * @author Danko Bukovac
 * @brief Function that adds a new select to redolog, commented code with the new select from select.c,
 * current code works with selection.c
 * @return EXIT_FAILURE if not allocated memory for ispis, otherwise EXIT_SUCCESS
 */
int AK_add_to_redolog_select(int command, struct list_node *condition, char *srcTable);

/**
 * @author Danko Bukovac
 * @brief Function that checks redolog for select, works only with selection.c, not select.c
 * @return 0 if select was not found, otherwise 1
 */
int AK_check_redo_log_select(int command, struct list_node *condition, char *srcTable);

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

#endif /* REDOLOG */
