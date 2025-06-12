#ifndef XML_IMPORTER_H
#define XML_IMPORTER_H
/**
 * @file    xml_importer.h
 * @brief   Public interface for importing a hierarchical XML file and
 *          normalising it into four AK tables:
 *              - xml_users
 *              - xml_profiles
 *              - xml_addresses
 *              - xml_tags
 *
 * The XML test document follows the structure:
 * @code{.xml}
 * <users>
 *   <user id="...">
 *     <name>…</name>
 *     <profile><age>…</age><email>…</email></profile>
 *     <address><street>…</street><city>…</city></address>
 *     <tags><tag>…</tag>…</tags>
 *   </user>
 *   …
 * </users>
 * @endcode
 *
 * After parsing, call @ref create_xml_normalized_tables() to create AK tables
 * and then @ref insert_normalized_xml() to insert the in-memory data.
 */

#include "../auxi/test.h"   /* For TestResult structure*/

/* ---------- Helper functions --------------------------------------- */

/**
 * Parse an XML document provided as a NUL-terminated string.
 *
 * @param xml_str  XML document in memory.
 * @return 0 on success, non-zero on error.
 */
int parse_xml(const char *xml_str);

/**
 * Read an XML file from disk and call @ref parse_xml().
 *
 * @param filename  Path to the XML file.
 * @return 0 on success, non-zero on error.
 */
int import_xml_file(const char *filename);

/**
 * Create the four normalised tables (xml_users, xml_profiles,
 * xml_addresses, xml_tags). Drops (if exists) existing tables.
 */
void create_xml_normalized_tables(void);

/**
 * Insert the parsed XML data into the normalised tables.
 * Called after @ref create_xml_normalized_tables().
 */
void insert_normalized_xml(void);

/**
 * Parses test XML file, creates tables,
 * inserts rows and prints a short summary.
 */
TestResult xml_importer_test(void);

#endif /* XML_IMPORTER_H */
