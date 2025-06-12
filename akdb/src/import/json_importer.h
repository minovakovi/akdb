#ifndef JSON_IMPORTER_H
#define JSON_IMPORTER_H

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "signal.h"
#include <dirent.h>
#include <cjson/cJSON.h>

/**
 * @brief Reads the entire content of a file into a dynamically allocated string.
 *
 * This function opens the specified JSON file in binary mode, reads its entire content,
 * and returns a pointer to a null-terminated string. The caller is responsible for freeing
 * the allocated memory.
 *
 * @param filename The path to the JSON file.
 * @return A pointer to the allocated string containing the file content, or NULL on failure.
 */
char* read_json_file(const char *filename);

/**
 * @brief Imports JSON data from a file.
 *
 * This function reads the JSON file specified by @p filename using read_json_file and then
 * parses its content to initialize the global JSON data using parse_json.
 *
 * @param filename The path to the JSON file.
 * @return 0 on success, -1 on failure.
 */
int import_json_file(const char *filename);

/**
 * @brief Initializes the JSON importer module.
 *
 * This function performs any necessary initialization for the JSON importer.
 *
 * @return 0 on success, non-zero on failure.
 */
int json_importer_init(void);

/**
 * @brief Parses a JSON string.
 *
 * This function parses the provided JSON string and initializes the global JSON data.
 *
 * @param json_str The JSON string to be parsed.
 * @return 0 on success, negative on failure.
 */
int parse_json(const char *json_str);

/**
 * @brief Infers the schema from parsed JSON data.
 *
 * This function examines the globally stored JSON data and prints out the inferred schema.
 * It is intended for debugging and validation purposes.
 */
void infer_schema(void);

/**
 * @brief Generates the relational schema based on the inferred JSON structure.
 *
 * This is a placeholder function for generating a relational schema from the parsed JSON data.
 *
 * @return 0 if the function is called successfully (functionality not yet fully implemented).
 */
int generate_schema(void);

/**
 * @brief Tests the JSON importer functionality.
 *
 * This function performs a series of tests including:
 * - Initializing the importer,
 * - Importing JSON from a file,
 * - Parsing the JSON data,
 * - Inferring the JSON schema,
 * - Inserting test data into a table, and
 * - Generating a table schema.
 *
 * @return A TestResult structure indicating the number of successful and failed tests.
 */
TestResult json_importer_test(void);

#endif // JSON_IMPORTER_H
