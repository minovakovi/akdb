/*
 /**
@file recovery.h Header file that provides functions and defines for recovery.c
 *
 *
 *  Created on: June 12, 2013
 *      Author: Drazen Bandic, 
 *              updated by Tomislav Turek on March 30, 2016
 */

#ifndef RECOVERY
#define RECOVERY

#include "../auxi/test.h"
#include "../auxi/auxiliary.h"
#include "../mm/memoman.h"
#include "../auxi/constants.h"
#include "../auxi/configuration.h"
#include "../auxi/debug.h"
#include "../rec/archive_log.h"
#include "../file/table.h"
#include "../file/fileio.h"
#include "../file/test.h"
#include "../opti/rel_eq_selection.h"
#include "../auxi/mempro.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "signal.h"
#include <dirent.h>

/** 
 * Function opens the recovery binary file and executes all commands that were
 * saved inside the redo_log structure
 * @author Dražen Bandić, update by Tomislav Turek
 * @brief Function that reads the binary file in which last commands were saved, and executes them
 * @param fileName - name of the archive log
 * @return no value
 */
void AK_recover_archive_log(char* fileName);

/** 
 * Function is given the table name with desired data that should be
 * inserted inside. By using the table name, function retrieves table 
 * attributes names and their types which uses afterwards for insert_data_test
 * function to insert data to designated table.
 * 
 * @author Dražen Bandić, updated by Tomislav Turek
 * @brief Function that inserts a new row in the table with attributes
 * @param table - table name to insert to
 * @param commandNumber - number of current command
 * @return no value
 */
void AK_recovery_insert_row(char* table, int commandNumber);

/** 
 * @author Dražen Bandić
 * @brief Function that tokenizes the input with the given delimiter and puts them in an double pointer structure (so we can execute an insert)
 * @param input - input to tokenize
 * @param delimiter - delimiter
 * @param valuesOrNot - 1 if the input are values, 0 otherwise
 * @return new double pointer structure with tokens
 */
char** AK_recovery_tokenize(char* input, char* delimiter, int valuesOrNot);
TestResult AK_recovery_test();

/**
 * Function is called when SIGINT signal is sent to the system.
 * All commands that are written to rec.bin file are recovered to
 * the designated structure and then executed.
 * 
 * @author Tomislav Turek
 * @brief Function that recovers and executes failed commands
 * @param sig required integer parameter for SIGINT handler functions
 */
void AK_recover_operation(int sig);

/** 
 * Function lists the contents of the archive_log directory.
 * The user then writes the name of the desired bin file to perform the neccessary actions.
 * @author Matija Večenaj
 * @brief Executes the recovery operation for the chosen bin file
 * @param none
 * @return no value
 */
void AK_load_chosen_log ();

/** 
 * Function reads the latest.txt file which contains the name of the latest bin file that's been created.
 * Then it loads it and does the neccessary recovery operations.
 * @author Matija Večenaj
 * @brief Executes the recovery operation for the latest bin file
 * @param none
 * @return no value
 */
void AK_load_latest_log ();

#endif /* RECOVERY */
