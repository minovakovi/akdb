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
#include <signal.h>
#include <errno.h>

// Project headers
#include "auxi/test.h"
#include "tests.h"
#include "auxi/mempro.h"
#include "mm/memoman.h"
#include "file/test.h"
#include "rec/archive_log.h"
#include "rec/redo_log.h"
#include "projectDetails.h"

// External declarations
extern Test tests[];

// Constants
#define MAX_FILENAME_LENGTH 256
#define LATEST_LOG_FILE "/latest.txt"

// Helper function to get number of tests
static int get_num_tests(void);

// Function prototypes
static int parse_arguments(int argc, char *argv[]);
static int initialize_system(void);
static void cleanup_system(void);
static int run_tests(int argc, char *argv[]);
static int run_single_test(int test_index);
static void setup_signal_handlers(void);
static void print_usage(void);
static int recover_from_archive_log(void);

// Global variables
static AK_synchronization_info *g_file_lock = NULL;

/**
Main program function
 * @param argc Number of command line arguments
 * @param argv Array of command line arguments
@return EXIT_SUCCESS if successful, EXIT_ERROR otherwise
 */
int main(int argc, char *argv[])
{
    int result = EXIT_FAILURE;

    AK_PRO;

    // Parse command line arguments first
    int parse_result = parse_arguments(argc, argv);
    if (parse_result != 0) {
        if (parse_result == 1) {
            // Help was requested
            result = EXIT_SUCCESS;
        }
        goto cleanup;
    }

    printf("KALASHNIKOV DB %s - STARTING\n\n", AK_version);

    // Initialize system components
    if (initialize_system() != EXIT_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to initialize system\n");
        goto cleanup;
    }

    // Set up signal handlers
    setup_signal_handlers();

    // Recover from archive log
    if (recover_from_archive_log() != EXIT_SUCCESS) {
        fprintf(stderr, "WARNING: Failed to recover from archive log\n");
        // Continue execution - this might not be fatal
    }

    // Run tests if requested
    if (run_tests(argc, argv) != EXIT_SUCCESS) {
        fprintf(stderr, "ERROR: Test execution failed\n");
        goto cleanup;
    }

    // Flush cache before exit
    if (AK_flush_cache() != EXIT_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to flush cache\n");
        goto cleanup;
    }

    printf("\nEverything completed successfully!\nBye =)\n");
    result = EXIT_SUCCESS;

cleanup:
    cleanup_system();
    AK_EPI;
    return result;
}

/**
 * @brief Parse and validate command line arguments
 * @param argc Number of arguments
 * @param argv Argument array
 * @return 0 on success, 1 if help shown, -1 on error
 */
static int parse_arguments(int argc, char *argv[])
{
    if (argc == 1) {
        // No arguments - normal operation
        return 0;
    }

    if (argc == 2 && strcmp(argv[1], "help") == 0) {
        print_usage();
        return 1;
    }

    if (argc > 3) {
        fprintf(stderr, "ERROR: Too many arguments\n");
        print_usage();
        return -1;
    }

    // Validate test commands
    if (argc >= 2) {
        if (strcmp(argv[1], "test") == 0 || strcmp(argv[1], "alltest") == 0) {
            return 0;
        } else {
            fprintf(stderr, "ERROR: Invalid command '%s'\n", argv[1]);
            print_usage();
            return -1;
        }
    }

    return 0;
}

/**
 * @brief Initialize all system components
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error
 */
static int initialize_system(void)
{
    // Initialize critical sections
    dbmanFileLock.ptr = AK_init_critical_section();
    g_file_lock = dbmanFileLock.ptr;

    if (!g_file_lock) {
        fprintf(stderr, "ERROR: Failed to initialize critical sections\n");
        return EXIT_FAILURE;
    }

    printf("Critical section - Init: %d, Ready: %d\n",
           g_file_lock->init, g_file_lock->ready);

    // Check blob folder
    AK_check_folder_blobs();

    // Load configuration
    AK_inflate_config();
    printf("Database file: %s\n", DB_FILE);

    // Set test mode
    testMode = TEST_MODE_OFF;

    // Initialize disk manager
    if (AK_init_disk_manager() != EXIT_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to initialize disk manager\n");
        return EXIT_FAILURE;
    }

    // Initialize memory manager
    if (AK_memoman_init() != EXIT_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to initialize memory manager\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Clean up system resources
 */
static void cleanup_system(void)
{
    if (g_file_lock) {
        AK_destroy_critical_section(g_file_lock);
        g_file_lock = NULL;
    }
}

/**
 * @brief Set up signal handlers
 */
static void setup_signal_handlers(void)
{
    if (signal(SIGINT, AK_archive_log) == SIG_ERR) {
        fprintf(stderr, "WARNING: Failed to set SIGINT handler\n");
    }
}

/**
 * @brief Get the number of available tests
 * @return Number of tests
 */
static int get_num_tests(void)
{
    // Count tests by iterating until we find an empty entry or reach a reasonable limit
    int count = 0;
    // Safety limit
    const int max_tests = 100;

    while (count < max_tests && tests[count].name != NULL && tests[count].func != NULL) {
        count++;
    }

    return count;
}

/**
 * @brief Recover from archive log
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error
 */
static int recover_from_archive_log(void)
{
    // TODO: Implement proper archive log recovery.
    // The logic from the original main.c is included below, commented out,
    // to guide the implementation.

    // retrieve last archive log filename
    /*
    FILE *archiveLog;
    char archiveLogFilename[30];

    char *latest = malloc(strlen(ARCHIVELOG_PATH)+strlen("/latest.txt")+1);
    strcpy(latest, ARCHIVELOG_PATH);
    strcat(latest, "/latest.txt");

    archiveLog = fopen(latest, "r");
    fscanf(archiveLog, "%s", archiveLogFilename); // Corrected from 'fp'
    fclose(archiveLog); // Corrected from 'fp'
    AK_free(latest);

    char *destination = malloc(strlen(ARCHIVELOG_PATH)+strlen(archiveLogFilename)+2);
    strcpy(destination, ARCHIVELOG_PATH);
    strcat(destination, "/");
    strcat(destination, archiveLogFilename);
    */  //--uncomment when AK_recover_archive_log is fixed

    // For now, use hardcoded path. This should be replaced with 'destination'
    // from the logic above once it is implemented and uncommented.
    const char *log_path = "../src/rec/rec.bin";

    // AK_recover_archive_log returns void, so just call it
    AK_recover_archive_log(log_path);

    return EXIT_SUCCESS;
}


/**
 * @brief Run tests based on command line arguments
 * @param argc Number of arguments
 * @param argv Argument array
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error
 */
static int run_tests(int argc, char *argv[])
{
    if (argc < 2) {
        return EXIT_SUCCESS; // No tests to run
    }

    if (strcmp(argv[1], "test") == 0) {
        if (argc == 2) {
            // Interactive test selection
            choose_test();
        } else if (argc == 3) {
            if (strcmp(argv[2], "show") == 0) {
                show_test();
            } else {
                // Run specific test by number
                char *endptr;
                errno = 0;
                long test_num = strtol(argv[2], &endptr, 10);

                if (errno != 0 || *endptr != '\0' || test_num < 1) {
                    fprintf(stderr, "ERROR: Invalid test number '%s'\n", argv[2]);
                    return EXIT_FAILURE;
                }

                return run_single_test((int)test_num - 1);
            }
        }
    } else if (strcmp(argv[1], "alltest") == 0) {
        // Run all tests
        return testing();
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Run a single test by index
 * @param test_index Index of test to run (0-based)
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error
 */
static int run_single_test(int test_index)
{
    // Get number of tests
    int num_tests = get_num_tests();

    // Validate test index
    if (test_index < 0 || test_index >= num_tests) {
        fprintf(stderr, "ERROR: Test index %d out of range (0-%d)\n",
                test_index, num_tests - 1);
        return EXIT_FAILURE;
    }

    printf("Running test: %s\n", tests[test_index].name);

    // Create test tables and set constraints
    AK_create_test_tables();
    set_catalog_constraints();

    // Run the test
    TestResult test_result = tests[test_index].func();

    printf("\nTEST: %s - COMPLETED\n", tests[test_index].name);

    // Output test results
    TEST_output_results(test_result);

    return EXIT_SUCCESS;
}

/**
 * @brief Print usage information
 */
static void print_usage(void)
{
    int num_tests = get_num_tests();

    printf("Usage: akdb [COMMAND] [OPTIONS]\n\n");
    printf("Commands:\n");
    printf("  help              Show this help message\n");
    printf("  test              Run interactive test selection\n");
    printf("  test <number>     Run specific test by number (1-%d)\n", num_tests);
    printf("  test show         Show available tests\n");
    printf("  alltest           Run all tests\n\n");
    printf("Examples:\n");
    printf("  akdb              Start database normally\n");
    printf("  akdb test         Interactive test selection\n");
    printf("  akdb test 1       Run test number 1\n");
    printf("  akdb alltest      Run all tests\n");
}

