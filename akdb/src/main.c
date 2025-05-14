/**
@file main.c Main program file
*/
/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * main.c
 * Copyright (C) Markus Schatten 2009 <markus.schatten@foi.hr>
 *
 * main.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * main.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <string.h> //ARCHIVE LOG
#include <stdlib.h>
#include "auxi/test.h"
#include "tests.h"
// Memory wrappers and debug mode
#include "auxi/mempro.h"
// Memory management
#include "mm/memoman.h"
// File management
#include "file/test.h"
// Logging
#include "rec/archive_log.h" //ARCHIVE LOG
// Other
#include "rec/redo_log.h"
#include "projectDetails.h"
#include "file/blobs.h"
/**
Main program function
@return EXIT_SUCCESS if successful, EXIT_ERROR otherwise
*/
int main(int argc, char *argv[])
{
    /*FILE *archiveLog;
    char archiveLogFilename[30];*/
    //--uncomment when AK_recover_archive_log is fixed
    AK_PRO;
    // initialize critical sections
    dbmanFileLock.ptr = AK_init_critical_section();
    AK_synchronization_info *const fileLock = dbmanFileLock.ptr;
    printf("Init: %d, ready: %d", fileLock->init, fileLock->ready);
    AK_check_folder_blobs();
    if ((argc == 2 && argv[1] && strcmp(argv[1], "help") == 0) ||
        (argc > 3) ||
        (argc == 2 && argv[1] && strcmp(argv[1], "test") != 0 && strcmp(argv[1], "alltest") != 0))
    {
        // If we write ./akdb test help, or write any mistake or ask for any kind of help, the help will pop up
        help();
    }
    else if ((argc == 3) && argv[1] && argv[2] && strcmp(argv[1], "test") == 0 && strcmp(argv[2], "show") == 0)
    {
        // If we write ./akdb test show, the inputted will start and show
        show_test();
    }
    else
    {
        printf("KALASHNIKOV DB %s - STARTING\n\n", AK_version);
        AK_inflate_config();
        printf("db_file: %s\n", DB_FILE);
        testMode = TEST_MODE_OFF;
        if (AK_init_disk_manager() == EXIT_SUCCESS)
        {
            if (AK_memoman_init() == EXIT_SUCCESS)
            {

                // retrieve last archive log filename

                /*char *latest = malloc(strlen(ARCHIVELOG_PATH)+strlen("/latest.txt")+1);
                strcpy(latest, ARCHIVELOG_PATH);
                strcat(latest, "/latest.txt");

                archiveLog = fopen(latest, "r");
                fscanf(fp, "%s", archiveLogFilename);
                fclose(fp);
                AK_free(latest);

                char *destination = malloc(strlen(ARCHIVELOG_PATH)+strlen(archiveLogFilename)+2);
                strcpy(destination, ARCHIVELOG_PATH);
                strcat(destination, "/");
                strcat(destination, archiveLogFilename);    */
                //--uncomment when AK_recover_archive_log is fixed

                sigset(SIGINT, AK_archive_log);
                AK_recover_archive_log("../src/rec/rec.bin"); // "../src/rec/rec.bin" needs to be replaced with destination when AK_recover_archive_log is fixed
                /* component test area --- begin */
                if ((argc == 2) && !strcmp(argv[1], "test"))
                {
                    choose_test();
                }
                if ((argc == 2) && !strcmp(argv[1], "alltest"))
                {
                    testing();
                }
                else if ((argc == 3) && !strcmp(argv[1], "test"))
                {
                    if (argv[2] == NULL || strlen(argv[2]) == 0)
                    {
                        printf("ERROR: Missing test index. Please provide a valid test index.\n");
                        return EXIT_ERROR;
                    }

                    char *endptr;
                    int pickedTest = strtol(argv[2], &endptr, 10) - 1;

                    // Check if argv[2] is a valid number
                    if (*endptr != '\0')
                    {
                        printf("ERROR: Invalid test index '%s'. Please provide a valid number.\n", argv[2]);
                        return EXIT_ERROR;
                    }

                    // Validate pickedTest against tests_length
                    if (pickedTest < 0 || pickedTest >= tests_length)
                    {
                        printf("ERROR: Test index out of range. Valid range is 1 to %d.\n", tests_length);
                        return EXIT_ERROR;
                    }

                    // Run the selected test
                    AK_create_test_tables();
                    set_catalog_constraints();
                    tests[pickedTest].func();
                }
                /*component test area --- end */
                if (AK_flush_cache() == EXIT_SUCCESS)
                {
                    int pickedTest = -1;
                    pickedTest = strtol(argv[2], NULL, 10) - 1;

                    if (pickedTest < 0 || pickedTest >= tests_length)
                    {
                        printf("Invalid test index\n");
                        return EXIT_ERROR;
                    }
                    printf("\nTEST:--- %s --- ENDED!\n", tests[pickedTest].name);
                    printf("\nEverything was fine!\nBye =)\n");

                    TEST_output_results(tests[pickedTest].func());
                    /* For easyer debugging and GDB usage
                    AK_create_test_tables();
                    AK_view_test();
                    */
                    // pthread_exit(NULL);
                    AK_EPI;
                    return (EXIT_SUCCESS);
                }
            }
            printf("ERROR. Failed to initialize memory manager\n");
            AK_EPI;
            return (EXIT_ERROR);
        }
        printf("ERROR. Failed to initialize disk manager\n");
        AK_EPI;
        return (EXIT_ERROR);
    }
    // delete critical sections
    AK_destroy_critical_section(dbmanFileLock.ptr);
    AK_EPI;
    return (EXIT_SUCCESS);
}
