/**
 * @file        xml_importer.c
 * @brief       Import a hierarchical XML file and normalise it into four
 *              relational AK tables (users, profiles, addresses, tags).
 *
 *
 *              Parsing is done with libxml2 (DOM traversal).
 *              Memory is held in a temporary array of @ref XmlUser structs
 *              before being inserted.
 *
 *
 * Author: David Slavik
 */

 #include "xml_importer.h"

 #include <libxml/parser.h>
 #include <libxml/tree.h>
 
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <ctype.h>
 
 #include "../sql/drop.h"
 #include "../file/table.h"
 
 /* --------------------------------------------------------------------- */
 /*                         In-memory structures                          */
 /* --------------------------------------------------------------------- */
 typedef struct {
     int   id;          /* Unique ID (XML attribute)          */
     char *name;        /* Display name                       */
     int   age;         /* Age (profile)                      */
     char *email;       /* Email (profile)                    */
     char *street;      /* Street (address)                   */
     char *city;        /* City (address)                     */
     char **tags;       /* Dynamic array of tag strings       */
     int   tag_cnt;     /* Number of tags                     */
 } XmlUser;
 
 static XmlUser *xml_data = NULL;     /* dynamic array of users         */
 static int       xml_rows = 0;       /* number of elements      */
 
 /* --------------------------------------------------------------------- */
 /*                         Helper functions                              */
 /* --------------------------------------------------------------------- */
 
 /* strdup() + whitespace trim (left & right) */
 static inline char *strdup_trim(const char *s)
 {
     while (isspace((unsigned char)*s)) ++s;          /* left trim */
     size_t len = strlen(s);
     while (len && isspace((unsigned char)s[len - 1])) --len; /* right */
     char *out = AK_malloc(len + 1);
     memcpy(out, s, len);
     out[len] = '\0';
     return out;
 }
 
 /* Release the global xml_data buffer */
 static void free_xml_data(void)
 {
     if (!xml_data) return;
 
     for (int i = 0; i < xml_rows; ++i) {
         XmlUser *u = &xml_data[i];
         AK_free(u->name);
         AK_free(u->email);
         AK_free(u->street);
         AK_free(u->city);
         for (int t = 0; t < u->tag_cnt; ++t)
             AK_free(u->tags[t]);
         AK_free(u->tags);
     }
     AK_free(xml_data);
     xml_data = NULL;
     xml_rows = 0;
 }
 
 /* --------------------------------------------------------------------- */
 /*                                libxml2                                */
 /* --------------------------------------------------------------------- */
 
 /* Extract <tag> elements under <tags> and push into u->tags[] */
 static void read_tags(xmlNodePtr tag_root, XmlUser *u)
 {
     for (xmlNodePtr n = tag_root->children; n; n = n->next) {
         if (n->type != XML_ELEMENT_NODE ||
             xmlStrcmp(n->name, BAD_CAST "tag") != 0)
             continue;
 
         char *val = (char *)xmlNodeGetContent(n);
         if (!val || !*val) { xmlFree(val); continue; }
 
         if (u->tag_cnt % 4 == 0) {         /* grow by chunks of 4 */
             u->tags = AK_realloc(u->tags,
                                  (u->tag_cnt + 4) * sizeof(char *));
         }
         u->tags[u->tag_cnt++] = strdup_trim(val);
         xmlFree(val);
     }
 }
 
 /**
  * @brief Parse an XML document which is in-memory.
  *
  * The result is stored in global xml_data / xml_rows for later insertion.
  */
 int parse_xml(const char *xml_str)
 {
     free_xml_data();                        /* clear previous parse */
 
     xmlDocPtr doc = xmlReadMemory(xml_str, (int)strlen(xml_str),
                                   "inline.xml", NULL, 0);
     if (!doc) {
         fprintf(stderr, "parse_xml: libxml2 failed to parse input\n");
         return -1;
     }
 
     xmlNodePtr root = xmlDocGetRootElement(doc);
     if (!root || xmlStrcmp(root->name, BAD_CAST "users") != 0) {
         fprintf(stderr, "parse_xml: root element <users> not found\n");
         xmlFreeDoc(doc);
         return -1;
     }
 
     /* Allocate a buffer; */
     int cap = 8;
     xml_data = AK_malloc(cap * sizeof *xml_data);
 
     for (xmlNodePtr user = root->children; user; user = user->next) {
         if (user->type != XML_ELEMENT_NODE ||
             xmlStrcmp(user->name, BAD_CAST "user") != 0)
             continue;
 
         /* Ensure capacity */
         if (xml_rows == cap)
             xml_data = AK_realloc(xml_data,
                                   (cap *= 2) * sizeof *xml_data);
 
         XmlUser *u = &xml_data[xml_rows];
         memset(u, 0, sizeof *u);
 
         /* ---- Attribute: id ---- */
         xmlChar *id_attr = xmlGetProp(user, BAD_CAST "id");
         u->id = id_attr ? atoi((char *)id_attr) : 0;
         xmlFree(id_attr);
 
         /* ---- Child elements ---- */
         for (xmlNodePtr n = user->children; n; n = n->next) {
             if (n->type != XML_ELEMENT_NODE) continue;
 
             if (xmlStrcmp(n->name, BAD_CAST "name") == 0) {
                 char *val = (char *)xmlNodeGetContent(n);
                 u->name = strdup_trim(val);
                 xmlFree(val);
             }
             else if (xmlStrcmp(n->name, BAD_CAST "profile") == 0) {
                 for (xmlNodePtr p = n->children; p; p = p->next) {
                     if (p->type != XML_ELEMENT_NODE) continue;
 
                     if (xmlStrcmp(p->name, BAD_CAST "age") == 0) {
                         char *val = (char *)xmlNodeGetContent(p);
                         u->age = atoi(val);
                         xmlFree(val);
                     } else if (xmlStrcmp(p->name, BAD_CAST "email") == 0) {
                         char *val = (char *)xmlNodeGetContent(p);
                         u->email = strdup_trim(val);
                         xmlFree(val);
                     }
                 }
             }
             else if (xmlStrcmp(n->name, BAD_CAST "address") == 0) {
                 for (xmlNodePtr a = n->children; a; a = a->next) {
                     if (a->type != XML_ELEMENT_NODE) continue;
 
                     if (xmlStrcmp(a->name, BAD_CAST "street") == 0) {
                         char *val = (char *)xmlNodeGetContent(a);
                         u->street = strdup_trim(val);
                         xmlFree(val);
                     } else if (xmlStrcmp(a->name, BAD_CAST "city") == 0) {
                         char *val = (char *)xmlNodeGetContent(a);
                         u->city = strdup_trim(val);
                         xmlFree(val);
                     }
                 }
             }
             else if (xmlStrcmp(n->name, BAD_CAST "tags") == 0) {
                 read_tags(n, u);
             }
         }
 
         ++xml_rows;
     }
 
     xmlFreeDoc(doc);
     printf("Parsed %d XML users.\n", xml_rows);
     return 0;
 }
 
 /**
  * @brief Wrapper function: read file from disk (argument is file path) then parse the XML file.
  */
 int import_xml_file(const char *filename)
 {
     FILE *fp = fopen(filename, "rb");
     if (!fp) {
         fprintf(stderr, "import_xml_file: cannot open %s\n", filename);
         return -1;
     }
     fseek(fp, 0, SEEK_END);
     long sz = ftell(fp);
     rewind(fp);
 
     char *buf = AK_malloc(sz + 1);
     if (!buf) { fclose(fp); return -1; }
     fread(buf, 1, sz, fp);
     fclose(fp);
     buf[sz] = '\0';
 
     int res = parse_xml(buf);
     AK_free(buf);
     return res;
 }
 
 /* --------------------------------------------------------------------- */
 /*                 Table creation & row insertion                        */
 /* --------------------------------------------------------------------- */
 
 void create_xml_normalized_tables(void)
 {
     /* Drop old tables (if exists) */
     AK_drop_arguments da;
     da.value = "xml_tags";      AK_drop_table(&da);
     da.value = "xml_addresses"; AK_drop_table(&da);
     da.value = "xml_profiles";  AK_drop_table(&da);
     da.value = "xml_users";     AK_drop_table(&da);
 
     /* ---- xml_users(id, name) ---- */
     AK_header users_hdr[] = {
         { TYPE_INT,     "id"   },
         { TYPE_VARCHAR, "name" },
         { 0, "" }
     };
     AK_initialize_new_segment("xml_users", SEGMENT_TYPE_TABLE, users_hdr);
 
     /* ---- xml_profiles(user_id, age, email) ---- */
     AK_header prof_hdr[] = {
         { TYPE_INT,     "user_id" },
         { TYPE_INT,     "age"     },
         { TYPE_VARCHAR, "email"   },
         { 0, "" }
     };
     AK_initialize_new_segment("xml_profiles", SEGMENT_TYPE_TABLE, prof_hdr);
 
     /* ---- xml_addresses(user_id, street, city) ---- */
     AK_header addr_hdr[] = {
         { TYPE_INT,     "user_id" },
         { TYPE_VARCHAR, "street"  },
         { TYPE_VARCHAR, "city"    },
         { 0, "" }
     };
     AK_initialize_new_segment("xml_addresses", SEGMENT_TYPE_TABLE, addr_hdr);
 
     /* ---- xml_tags(user_id, tag) ---- */
     AK_header tag_hdr[] = {
         { TYPE_INT,     "user_id" },
         { TYPE_VARCHAR, "tag"     },
         { 0, "" }
     };
     AK_initialize_new_segment("xml_tags", SEGMENT_TYPE_TABLE, tag_hdr);
 }
 
 void insert_normalized_xml(void)
 {
     for (int i = 0; i < xml_rows; ++i) {
         XmlUser *u = &xml_data[i];
         struct list_node *row;
 
         /* ---- xml_users ---- */
         row = AK_malloc(sizeof *row); AK_Init_L3(&row);
         AK_Insert_New_Element(TYPE_INT,    &u->id,
                               "xml_users", "id",   row);
         AK_Insert_New_Element(TYPE_VARCHAR, u->name,
                               "xml_users", "name", row);
         AK_insert_row(row);
         AK_DeleteAll_L3(&row); AK_free(row);
 
         /* ---- xml_profiles ---- */
         row = AK_malloc(sizeof *row); AK_Init_L3(&row);
         AK_Insert_New_Element(TYPE_INT,    &u->id,
                               "xml_profiles", "user_id", row);
         AK_Insert_New_Element(TYPE_INT,    &u->age,
                               "xml_profiles", "age",     row);
         AK_Insert_New_Element(TYPE_VARCHAR, u->email,
                               "xml_profiles", "email",   row);
         AK_insert_row(row);
         AK_DeleteAll_L3(&row); AK_free(row);
 
         /* ---- xml_addresses ---- */
         row = AK_malloc(sizeof *row); AK_Init_L3(&row);
         AK_Insert_New_Element(TYPE_INT,    &u->id,
                               "xml_addresses", "user_id", row);
         AK_Insert_New_Element(TYPE_VARCHAR, u->street,
                               "xml_addresses", "street",  row);
         AK_Insert_New_Element(TYPE_VARCHAR, u->city,
                               "xml_addresses", "city",    row);
         AK_insert_row(row);
         AK_DeleteAll_L3(&row); AK_free(row);
 
         /* ---- xml_tags (multiple rows per user) ---- */
         for (int t = 0; t < u->tag_cnt; ++t) {
             row = AK_malloc(sizeof *row); AK_Init_L3(&row);
             AK_Insert_New_Element(TYPE_INT,    &u->id,
                                   "xml_tags", "user_id", row);
             AK_Insert_New_Element(TYPE_VARCHAR, u->tags[t],
                                   "xml_tags", "tag",     row);
             AK_insert_row(row);
             AK_DeleteAll_L3(&row); AK_free(row);
         }
     }
 }
 
 
 /**
  * 
  */
 TestResult xml_importer_test(void)
 {
     AK_PRO;
 
     if (import_xml_file("../src/import/test_data.xml") != 0)
         return TEST_result(0, 1);
 
     create_xml_normalized_tables();
     insert_normalized_xml();
 
     printf("Imported XML into 4 tables.\n");
     AK_EPI;
     return TEST_result(1, 0);
 }
 