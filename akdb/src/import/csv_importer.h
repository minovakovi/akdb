/**
 * @file    csv_importer.h
 * @brief   Public interface for the CSV importer / normaliser module.
 *          Mirrors json_importer.h for API symmetry.
 *
 * @author  David Slavik
 * @date    2025-05-22
 */

 #ifndef CSV_IMPORTER_H
 #define CSV_IMPORTER_H

 #include "../auxi/test.h"

 /* ------------------------------------------------------------------ */
 /*                        File I/O & parsing                          */
 /* ------------------------------------------------------------------ */
 
 /**
  * @brief  Read a CSV file from disk into heap buffer.
  * @param  filename  Path to CSV file.
  * @return NUL-terminated buffer or @c NULL on error
  */
 char *read_csv_file(const char *filename);
 
 /**
  * @brief  Parse an in-memory CSV buffer.
  * @param  csv_str  NUL-terminated CSV content (also including header).
  * @return 0 on success, -1 on parsing or allocation failure.
  */
 int   parse_csv(const char *csv_str);
 
 /**
  * @brief  Read file then call parse_csv().
  * @param  filename  Path to the CSV file.
  * @return 0 on success, -1 on failure.
  */
 int   import_csv_file(const char *filename);
 
 /* ------------------------------------------------------------------ */
 /*                      Table creation & insert                       */
 /* ------------------------------------------------------------------ */
 
 /**
  * @brief  Drop (if exists) and recreate 4 normalised tables
  *         @c csv_users, @c csv_profiles, @c csv_addresses, @c csv_tags.
  */
 void  create_csv_normalized_tables(void);
 
 /**
  * @brief  Insert the parsed CSV data into the normalised tables.
  */
 void  insert_normalized_csv(void);
 
 /* ------------------------------------------------------------------ */
 /*                              Test                                  */
 /* ------------------------------------------------------------------ */
 
 /**
  * @brief  Test to import `test_data.csv` file into AKDB tables
  * 
  * This function imports data from CSV into a AKDB generated table.
  * @return A TestResult structure indicating the number of successful and failed tests.
  */
 TestResult csv_importer_test(void);
 
 #endif
 