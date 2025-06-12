/**
 * @file        csv_importer.c
 * @brief       Import a flat CSV file and normalise it into four AK tables
 *              (users, profiles, addresses, tags).
 *
 *              Expected columns (see test_data.csv):
 *                  id,name,age,email,street,city,tags
 *              where *tags* is a semicolon-separated list.
 *
 * @author      David Slavik
 */

 #include "csv_importer.h"

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <ctype.h>
 
 #include "../sql/drop.h"
 #include "../file/table.h"

 
 /** Memory representation of one CSV line. */
 typedef struct {
     int   id;         /**< User ID (primary key)             */
     char *name;       /**< Display name                      */
     int   age;        /**< Profile age                       */
     char *email;      /**< Profile email                     */
     char *street;     /**< Address street                    */
     char *city;       /**< Address city                      */
     char **tags;      /**< Dynamic array of tag strings      */
     int   tag_cnt;    /**< Number of tags                   */
 } CsvUser;
 
 /* Parse buffer --------------------------------------------------     */
 static CsvUser *csv_data = NULL;  /**< Array of parsed rows            */
 static int      csv_rows = 0;     /**< Valid entries counter          */
 
 /* --------------------------------------------------------------------- */
 /*                         Helper functions                              */
 /* --------------------------------------------------------------------- */
 
 static inline char *strdup_trim(const char *s)
 {
     while (isspace((unsigned char)*s)) ++s;            /* left trim  */
     size_t len = strlen(s);
     while (len && isspace((unsigned char)s[len - 1])) --len;  /* right */
     char *out = AK_malloc(len + 1);
     memcpy(out, s, len);
     out[len] = '\0';
     return out;
 }
 
 char *read_csv_file(const char *filename)
 {
     FILE *fp = fopen(filename, "rb");
     if (!fp) {
         fprintf(stderr, "read_csv_file: cannot open %s\n", filename);
         return NULL;
     }
     fseek(fp, 0, SEEK_END);
     long sz = ftell(fp);
     rewind(fp);
 
     char *buf = AK_malloc(sz + 1);
     if (!buf) {
         fprintf(stderr, "read_csv_file: malloc failed\n");
         fclose(fp);
         return NULL;
     }
     size_t r = fread(buf, 1, sz, fp);
     fclose(fp);
     buf[r] = '\0';
     return buf;
 }
 
 static void free_csv_data(void)
 {
     if (!csv_data) return;
     for (int i = 0; i < csv_rows; ++i) {
         CsvUser *u = &csv_data[i];
         AK_free(u->name);
         AK_free(u->email);
         AK_free(u->street);
         AK_free(u->city);
         for (int t = 0; t < u->tag_cnt; ++t)
             AK_free(u->tags[t]);
         AK_free(u->tags);
     }
     AK_free(csv_data);
     csv_data = NULL;
     csv_rows = 0;
 }
 
 /* --------------------------------------------------------------------- */
 /*                              CSV parser                               */
 /* --------------------------------------------------------------------- */
 
 static int split_csv_line(char *line, char **cols, int max_cols)
 {
     int n = 0;
     char *ptr = line;
     while (ptr && n < max_cols) {
         char *tok = strsep(&ptr, ",");
         cols[n++] = tok ? tok : (char *)"";
     }
     return n;
 }
 
 int parse_csv(const char *csv_str)
 {
     free_csv_data();                 /* clear previous parse */
 
     char *buf = strdup(csv_str);
     if (!buf) return -1;
 
     /* Count lines forr memory allocation */
     int lines = 0;
     for (char *p = buf; *p; ++p)
         if (*p == '\n') ++lines;
     if (lines <= 1) { AK_free(buf); return -1; }
 
     csv_data = AK_malloc(lines * sizeof *csv_data);
 
     /* Tokenise */
     int row = 0;
     const int MAXCOL = 16;
     char *cols[MAXCOL];
 
     char *saveptr;
     char *line = strtok_r(buf, "\r\n", &saveptr);   /* skip header */
     line = strtok_r(NULL, "\r\n", &saveptr);
 
     while (line) {
         if (row == lines)
             csv_data = AK_realloc(csv_data,
                         (lines *= 2) * sizeof *csv_data);
 
         CsvUser *u = &csv_data[row];
         memset(u, 0, sizeof *u);
 
         int ncol = split_csv_line(line, cols, MAXCOL);
         if (ncol < 7) {                          /* schema check */
             fprintf(stderr, "parse_csv: bad line: %s\n", line);
             line = strtok_r(NULL, "\r\n", &saveptr);
             continue;
         }
 
         /* Map columns */
         u->id     = atoi(cols[0]);
         u->name   = strdup_trim(cols[1]);
         u->age    = atoi(cols[2]);
         u->email  = strdup_trim(cols[3]);
         u->street = strdup_trim(cols[4]);
         u->city   = strdup_trim(cols[5]);
 
         /* split tags on ';' */
         char *tagbuf = strdup_trim(cols[6]);
         int tagcap   = 4;
         u->tags      = AK_malloc(tagcap * sizeof(char *));
         char *tagptr = tagbuf;
         char *tagtok;
         while ((tagtok = strsep(&tagptr, ";"))) {
             if (!*tagtok) continue;
             if (u->tag_cnt == tagcap)
                 u->tags = AK_realloc(u->tags,
                           (tagcap *= 2) * sizeof(char *));
             u->tags[u->tag_cnt++] = strdup_trim(tagtok);
         }
         AK_free(tagbuf);
 
         ++row;
         line = strtok_r(NULL, "\r\n", &saveptr);
     }
 
     csv_rows = row;
     AK_free(buf);
     printf("Parsed %d CSV rows.\n", csv_rows);
     return 0;
 }
 
 int import_csv_file(const char *filename)
 {
     char *s = read_csv_file(filename);
     if (!s) return -1;
     int res = parse_csv(s);
     AK_free(s);
     return res;
 }
 
 /* --------------------------------------------------------------------- */
 /*                       AK table normalisation                          */
 /* --------------------------------------------------------------------- */
 
 void create_csv_normalized_tables(void)
 {
     AK_drop_arguments da;
 
     AK_print_table("csv_users");
     AK_print_table("csv_profiles");
     AK_print_table("csv_addresses");
     AK_print_table("csv_tags");
 
     da.value = "csv_tags";      AK_drop_table(&da);
     da.value = "csv_addresses"; AK_drop_table(&da);
     da.value = "csv_profiles";  AK_drop_table(&da);
     da.value = "csv_users";     AK_drop_table(&da);
 
     /* users */
     AK_header users_hdr[3] = {
         { TYPE_INT,     "id"   },
         { TYPE_VARCHAR, "name" },
         { 0,            ""     }
     };
     AK_initialize_new_segment("csv_users", SEGMENT_TYPE_TABLE, users_hdr);
 
     /* profiles */
     AK_header prof_hdr[4] = {
         { TYPE_INT,     "user_id" },
         { TYPE_INT,     "age"     },
         { TYPE_VARCHAR, "email"   },
         { 0,            ""        }
     };
     AK_initialize_new_segment("csv_profiles", SEGMENT_TYPE_TABLE, prof_hdr);
 
     /* addresses */
     AK_header addr_hdr[4] = {
         { TYPE_INT,     "user_id" },
         { TYPE_VARCHAR, "street"  },
         { TYPE_VARCHAR, "city"    },
         { 0,            ""        }
     };
     AK_initialize_new_segment("csv_addresses", SEGMENT_TYPE_TABLE, addr_hdr);
 
     /* tags */
     AK_header tag_hdr[3] = {
         { TYPE_INT,     "user_id" },
         { TYPE_VARCHAR, "tag"     },
         { 0,            ""        }
     };
     AK_initialize_new_segment("csv_tags", SEGMENT_TYPE_TABLE, tag_hdr);
 }
 
 void insert_normalized_csv(void)
 {
     for (int i = 0; i < csv_rows; ++i) {
         CsvUser *u = &csv_data[i];
         struct list_node *row;
 
         /* csv_users */
         row = AK_malloc(sizeof *row); AK_Init_L3(&row);
         AK_Insert_New_Element(TYPE_INT,    &u->id,
                               "csv_users", "id",   row);
         AK_Insert_New_Element(TYPE_VARCHAR, u->name,
                               "csv_users", "name", row);
         AK_insert_row(row);
         AK_DeleteAll_L3(&row); AK_free(row);
 
         /* csv_profiles */
         row = AK_malloc(sizeof *row); AK_Init_L3(&row);
         AK_Insert_New_Element(TYPE_INT,    &u->id,
                               "csv_profiles", "user_id", row);
         AK_Insert_New_Element(TYPE_INT,    &u->age,
                               "csv_profiles", "age",     row);
         AK_Insert_New_Element(TYPE_VARCHAR, u->email,
                               "csv_profiles", "email",   row);
         AK_insert_row(row);
         AK_DeleteAll_L3(&row); AK_free(row);
 
         /* csv_addresses */
         row = AK_malloc(sizeof *row); AK_Init_L3(&row);
         AK_Insert_New_Element(TYPE_INT,    &u->id,
                               "csv_addresses", "user_id", row);
         AK_Insert_New_Element(TYPE_VARCHAR, u->street,
                               "csv_addresses", "street",  row);
         AK_Insert_New_Element(TYPE_VARCHAR, u->city,
                               "csv_addresses", "city",    row);
         AK_insert_row(row);
         AK_DeleteAll_L3(&row); AK_free(row);
 
         /* csv_tags */
         for (int t = 0; t < u->tag_cnt; ++t) {
             row = AK_malloc(sizeof *row); AK_Init_L3(&row);
             AK_Insert_New_Element(TYPE_INT,    &u->id,
                                   "csv_tags", "user_id", row);
             AK_Insert_New_Element(TYPE_VARCHAR, u->tags[t],
                                   "csv_tags", "tag",     row);
             AK_insert_row(row);
             AK_DeleteAll_L3(&row); AK_free(row);
         }
     }
 }
 
 /* --------------------------------------------------------------------- */
 /*                               Test                                    */
 /* --------------------------------------------------------------------- */
 
 TestResult csv_importer_test(void)
 {
     AK_PRO;
 
     if (import_csv_file("../src/import/test_data.csv") != 0)
         return TEST_result(0, 1);
 
     create_csv_normalized_tables();
     insert_normalized_csv();
 
     printf("Imported CSV into 4 tables.\n");
     AK_EPI;
     return TEST_result(1, 0);
 }
 