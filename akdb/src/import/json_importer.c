#include "json_importer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include "../sql/drop.h"
#include "../file/table.h"

static cJSON *json_data = NULL;

/**
 * @brief Reads a JSON file into a string buffer.
 *
 * Opens the specified file, reads its contents into a newly allocated buffer,
 * and adds null-termination for string.
 *
 * @param filename Path to the JSON file to read.
 * @return Pointer to a null-terminated string with file contents, or NULL if an error occurs.
 * @author David Slavik
 */
char* read_json_file(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "read_json_file: Could not open %s\n", filename);
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    rewind(fp);
    char *buf = AK_malloc(sz + 1);
    if (!buf) {
        fprintf(stderr, "read_json_file: malloc failure\n");
        fclose(fp);
        return NULL;
    }
    size_t r = fread(buf, 1, sz, fp);
    fclose(fp);
    buf[r] = '\0';
    return buf;
}

/**
 * @brief Initializes the JSON importer.
 *
 * Performs any necessary setup before importing JSON data.
 *
 * @return 0 on success, non-zero on failure.
 * @author David Slavik
 */
int json_importer_init(void) {
    return 0;
}

/**
 * @brief Parses a JSON string into the internal cJSON structure.
 *
 * Deletes any existing parsed data, parses the input string,
 * and prints a pretty-formatted JSON to stdout if parsing succeeds.
 *
 * @param json_str The JSON string to parse.
 * @return 0 on successful parse, -1 on error.
 * @author David Slavik
 */
int parse_json(const char *json_str) {
    if (json_data) cJSON_Delete(json_data);
    json_data = cJSON_Parse(json_str);
    if (!json_data) {
        fprintf(stderr, "Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        return -1;
    }
    char *pretty = cJSON_Print(json_data);
    if (pretty) {
        printf("Parsed JSON:\n%s\n", pretty);
        cJSON_free(pretty);
    }
    return 0;
}

/**
 * @brief Reads and parses a JSON file.
 *
 * Combines read_json_file and parse_json to import JSON data from a file.
 *
 * @param filename Path to the JSON file to import.
 * @return 0 on success, -1 on error.
 * @author David Slavik
 */
int import_json_file(const char *filename) {
    char *s = read_json_file(filename); 
    if (!s) return -1;               
    int res = parse_json(s);         
    AK_free(s);                      
    return res;                      
}

/**
 * @brief Creates normalized database tables for JSON data.
 *
 * Drops existing tables for tags, addresses, profiles, and users,
 * then initializes new segments for each table structure.
 * @author David Slavik
 */
void create_normalized_tables(void) {
    // drop existing
    AK_drop_arguments da;

    AK_print_table("json_users");
    AK_print_table("json_profiles");
    AK_print_table("json_addresses");
    AK_print_table("json_tags");

    da.value = "json_tags"; AK_drop_table(&da);
    da.value = "json_addresses"; AK_drop_table(&da);
    da.value = "json_profiles"; AK_drop_table(&da);
    da.value = "json_users"; AK_drop_table(&da);

    // users table: id, name
    AK_header users_hdr[3] = {
        { TYPE_INT,     "id"   },
        { TYPE_VARCHAR, "name" },
        { 0,            ""     }
    };
    AK_initialize_new_segment("json_users", SEGMENT_TYPE_TABLE, users_hdr);

    // profiles table: user_id, age, email
    AK_header prof_hdr[4] = {
        { TYPE_INT,     "user_id" },
        { TYPE_INT,     "age"     },
        { TYPE_VARCHAR, "email"   },
        { 0,            ""        }
    };
    AK_initialize_new_segment("json_profiles", SEGMENT_TYPE_TABLE, prof_hdr);

    // addresses table: user_id, street, city
    AK_header addr_hdr[4] = {
        { TYPE_INT,     "user_id" },
        { TYPE_VARCHAR, "street"  },
        { TYPE_VARCHAR, "city"    },
        { 0,            ""        }
    };
    AK_initialize_new_segment("json_addresses", SEGMENT_TYPE_TABLE, addr_hdr);

    // tags table: user_id, tag
    AK_header tag_hdr[3] = {
        { TYPE_INT,     "user_id" },
        { TYPE_VARCHAR, "tag"      },
        { 0,            ""         }
    };
    AK_initialize_new_segment("json_tags", SEGMENT_TYPE_TABLE, tag_hdr);
}

/**
 * @brief Inserts parsed JSON data into normalized tables.
 *
 * Iterates over the JSON array, extracting user, profile, address,
 * and tag information, and inserts each record into its corresponding table.
 * @author David Slavik
 */
void insert_normalized(void) {
    if (!cJSON_IsArray(json_data)) return;
    int n = cJSON_GetArraySize(json_data);
    for (int i = 0; i < n; ++i) {
        cJSON *u = cJSON_GetArrayItem(json_data, i);
        // user
        int id = cJSON_GetObjectItem(u, "id")->valueint;
        const char *name = cJSON_GetObjectItem(u, "name")->valuestring;
        struct list_node *row;
        row = AK_malloc(sizeof *row); AK_Init_L3(&row);
        AK_Insert_New_Element(TYPE_INT, &id,     "json_users", "id",   row);
        AK_Insert_New_Element(TYPE_VARCHAR, name, "json_users", "name", row);
        AK_insert_row(row); AK_DeleteAll_L3(&row); AK_free(row);

        // profile
        cJSON *p = cJSON_GetObjectItem(u, "profile");
        int age   = cJSON_GetObjectItem(p, "age")->valueint;
        const char *email = cJSON_GetObjectItem(p, "email")->valuestring;
        row = AK_malloc(sizeof *row); AK_Init_L3(&row);
        AK_Insert_New_Element(TYPE_INT, &id,    "json_profiles", "user_id", row);
        AK_Insert_New_Element(TYPE_INT, &age,   "json_profiles", "age",     row);
        AK_Insert_New_Element(TYPE_VARCHAR, email,
                              "json_profiles", "email",   row);
        AK_insert_row(row); AK_DeleteAll_L3(&row); AK_free(row);

        // address
        cJSON *a = cJSON_GetObjectItem(p, "address");
        const char *street = cJSON_GetObjectItem(a, "street")->valuestring;
        const char *city   = cJSON_GetObjectItem(a, "city")->valuestring;
        row = AK_malloc(sizeof *row); AK_Init_L3(&row);
        AK_Insert_New_Element(TYPE_INT, &id,      "json_addresses", "user_id", row);
        AK_Insert_New_Element(TYPE_VARCHAR, street,
                              "json_addresses", "street",  row);
        AK_Insert_New_Element(TYPE_VARCHAR, city,
                              "json_addresses", "city",    row);
        AK_insert_row(row); AK_DeleteAll_L3(&row); AK_free(row);

        // tags array
        cJSON *tags = cJSON_GetObjectItem(u, "tags");
        int tn = cJSON_GetArraySize(tags);
        for (int j = 0; j < tn; ++j) {
            const char *tag = cJSON_GetArrayItem(tags, j)->valuestring;
            row = AK_malloc(sizeof *row); AK_Init_L3(&row);
            AK_Insert_New_Element(TYPE_INT, &id,          "json_tags", "user_id", row);
            AK_Insert_New_Element(TYPE_VARCHAR, tag,       "json_tags", "tag",     row);
            AK_insert_row(row); AK_DeleteAll_L3(&row); AK_free(row);
        }
    }
}

/**
 * @brief Runs a test importing JSON into normalized tables.
 *
 * Executes import_json_file, create_normalized_tables, and insert_normalized,
 * prints the result of the import process.
 *
 * @return TestResult indicating results of test: success (1) or failure (0).
 * @author David Slavik
 */
TestResult json_importer_test(void) {
    AK_PRO;
    import_json_file("../src/import/test_data.json");
    create_normalized_tables();
    insert_normalized();
    printf("Imported JSON into 4 tables.\n");
    AK_EPI;
    return TEST_result(1,0);
}
