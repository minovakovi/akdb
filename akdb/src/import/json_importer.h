#ifndef JSON_IMPORTER_H
#define JSON_IMPORTER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>

#include <cjson/cJSON.h>

#include "../auxi/test.h"
#include "../auxi/auxiliary.h"
#include "../mm/memoman.h"
#include "../auxi/constants.h"
#include "../auxi/configuration.h"
#include "../auxi/debug.h"
#include "../file/table.h"
#include "../file/fileio.h"
#include "../file/test.h"
#include "../auxi/mempro.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Reads a JSON file into a  string buffer.
 *
 * Opens the given file, reads its contents, and returns
 * a null-terminated string.
 *
 * @param filename Path to the JSON file.
 * @return Pointer to a string with file contents, or NULL on error.
 */
char* read_json_file(const char *filename);

/**
 * @brief Initializes the JSON importer.
 *
 * Performs optional setup prior to importing JSON data.
 *
 * @return 0 on success, non-zero on failure.
 */
int json_importer_init(void);

/**
 * @brief Parses a JSON string and stores the result internally.
 *
 * Clears any previous JSON structure and parses a new one using cJSON.
 *
 * @param json_str The JSON string to parse.
 * @return 0 on success, -1 on error.
 */
int parse_json(const char *json_str);

/**
 * @brief Imports and parses a JSON file.
 *
 * Combines file reading and parsing into one operation.
 *
 * @param filename Path to the JSON file.
 * @return 0 on success, -1 on error.
 */
int import_json_file(const char *filename);

/**
 * @brief Creates normalized database tables for imported JSON data.
 *
 * Drops any existing tables with the same names and creates fresh ones
 * for users, profiles, addresses, and tags.
 */
void create_normalized_tables(void);

/**
 * @brief Inserts data from parsed JSON into normalized tables.
 *
 * Processes the JSON array and maps user, profile, address, and tag data
 * into their respective relational table formats.
 */
void insert_normalized(void);

/**
 * @brief Tests the full JSON import process into normalized tables.
 *
 * Executes file import, table creation, and data insertion.
 *
 * @return TestResult structure indicating success or failure.
 */
TestResult json_importer_test(void);

#ifdef __cplusplus
}
#endif

#endif
