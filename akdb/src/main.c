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
//Logging
#include "rec/archive_log.h" //ARCHIVE LOG
//Other
#include "rec/redo_log.h"
#include "projectDetails.h"

/**
Main program function
@return EXIT_SUCCESS if successful, EXIT_ERROR otherwise
*/
int main(int argc, char * argv[])
{
    /*FILE *archiveLog;
    char archiveLogFilename[30];*/ //--uncomment when AK_recover_archive_log is fixed
    AK_PRO;
    // initialize critical sections
    dbmanFileLock.ptr = AK_init_critical_section();
    AK_synchronization_info* const fileLock = dbmanFileLock.ptr;
    printf("Init: %d, ready: %d", fileLock->init, fileLock->ready);
    AK_check_folder_blobs();
    if((argc == 2) && (!strcmp(argv[1], "help") )|| (argc > 3)  || !(!strcmp(argv[1], "test") || !strcmp(argv[1], "alltest")))
		//if we write ./akdb test help, or write any mistake or ask for any kind of help the help will pop up
       help();
    else if((argc == 3) && !strcmp(argv[1], "test") && !strcmp(argv[2], "show"))
		//if we write ./akdb test test or show, the inputed will start and show
        show_test();
    else
    {
        printf( "KALASHNIKOV DB %s - STARTING\n\n", AK_version );
        AK_inflate_config();
        printf("db_file: %s\n", DB_FILE);
	    testMode = TEST_MODE_OFF;
        if( AK_init_disk_manager() == EXIT_SUCCESS )
        {
            if( AK_memoman_init() == EXIT_SUCCESS ) {

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
                strcat(destination, archiveLogFilename);    */  //--uncomment when AK_recover_archive_log is fixed
    
                sigset(SIGINT, AK_archive_log);
                AK_recover_archive_log("../src/rec/rec.bin");  // "../src/rec/rec.bin" needs to be replaced with destination when AK_recover_archive_log is fixed

                int choice;
                printf("Choose recovery mode:\n");
                printf("1 - Load the latest log file\n");
                printf("2 - Manually select a log file\n");
                printf("Enter your choice (1 or 2): ");
                scanf("%d", &choice);
                // Check the user's choice and call the appropriate function
                if (choice == 1) {
                    AK_load_latest_log();
                } else if (choice == 2) {
                    AK_load_chosen_log();
                } else {
                    printf("Invalid choice. Skipping recovery.\n");
                }


                /* component test area --- begin */
                if((argc == 2) && !strcmp(argv[1], "test"))
                {
                    choose_test();
                }
                if ((argc == 2) && !strcmp(argv[1], "alltest")){
                     testing();

                }
                else if((argc == 3) && !strcmp(argv[1], "test"))
                {

                    int pickedTest;
                    pickedTest = strtol(argv[2], NULL, 10)-1;

                    AK_create_test_tables();
                    set_catalog_constraints();
                    tests[pickedTest].func();
                }
                /*component test area --- end */
                if ( AK_flush_cache() == EXIT_SUCCESS ){
                    int pickedTest;
                    pickedTest = strtol(argv[2], NULL, 10)-1;	
                    printf("\nTEST:--- %s --- ENDED!\n", tests[pickedTest].name);
                    printf( "\nEverything was fine!\nBye =)\n" );
                    TEST_output_results(tests[pickedTest].func());
                    /* For easyer debugging and GDB usage
                    AK_create_test_tables();
                    AK_view_test();
                    */
                    // pthread_exit(NULL);
                    AK_EPI;
                    return ( EXIT_SUCCESS );
                }
            }
            printf( "ERROR. Failed to initialize memory manager\n" );
            AK_EPI;
            return ( EXIT_ERROR );
        }
        printf( "ERROR. Failed to initialize disk manager\n" );
    	AK_EPI;
        return ( EXIT_ERROR );
    }
    // delete critical sections
    AK_destroy_critical_section(dbmanFileLock.ptr);
    AK_EPI;
    return(EXIT_SUCCESS);
}

