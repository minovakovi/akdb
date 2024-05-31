/**
@file files.c Header file provides functions for file management
 */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */
#include "files.h"
#include <pthread.h>
pthread_mutex_t fileMut = PTHREAD_MUTEX_INITIALIZER;

/**
 * @author Tomislav Fotak, updated by Matija Šestak (function now uses caching)
 * @brief Function that initializes a new segment and writes its start and finish address in system catalog table. 
 * For creting new table, index, temporary table, etc. call this function
 * @param name segment name
 * @param type segment type
 * @param header pointer to header that should be written to the new extent (all blocks)
 * @return start address of new segment
 */

int AK_initialize_new_segment(char *name, int type, AK_header *header) {

    int start_address = -1;
    int end_address = INITIAL_EXTENT_SIZE;
    AK_PRO;
    pthread_mutex_lock(&fileMut);
    int objectID = AK_get_id();
    pthread_mutex_unlock(&fileMut);
    char *sys_table;
    sys_table = "AK_relation";

    if ((start_address = AK_new_segment(name, type, header)) == EXIT_ERROR) {
        AK_dbg_messg(LOW, FILE_MAN, "AK_init_new_segment__ERROR: Cannot initialize segment!\n");
		
        AK_EPI;
        return EXIT_ERROR;

    } else {

        end_address += start_address;

        struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
        AK_Init_L3(&row_root);

        AK_Insert_New_Element(TYPE_INT, &objectID, sys_table, "obj_id", row_root);
        AK_Insert_New_Element(TYPE_VARCHAR, name, sys_table, "name", row_root);
        AK_Insert_New_Element(TYPE_INT, &start_address, sys_table, "start_address", row_root);
        AK_Insert_New_Element(TYPE_INT, &end_address, sys_table, "end_address", row_root);

        AK_insert_row(row_root);

        AK_dbg_messg(LOW, FILE_MAN, "AK_init_new_segment__NOTIFICATION: New segment initialized at %d\n", start_address);
		AK_DeleteAll_L3(&row_root);
		AK_free(row_root);
        AK_EPI;

		
        return start_address;
    }
    AK_EPI;
}

/**
 * @author Tomislav Fotak, updated by Matija Šestak (function now uses caching), reused by Lovro Predovan
 * @brief Function that initializes a new segment and writes its start and finish address in system catalog table.
 *  For creting new table, index, temporary table, etc. call this function
 * @param name segment name
 * @param type segment type
 * @param header pointer to header that should be written to the new extent (all blocks)
 * @return start address of new segment
 */

int AK_initialize_new_index_segment(char *name, char *table_id, int attr_id , AK_header *header) {

    int start_address = -1;
    int end_address = INITIAL_EXTENT_SIZE;
    AK_PRO;

    pthread_mutex_lock(&fileMut);
    int objectID = AK_get_id();
    pthread_mutex_unlock(&fileMut);

    char *sys_table;
    sys_table = "AK_index";
    char type = SEGMENT_TYPE_TABLE;

    if ((start_address = AK_new_segment(name, type, header)) == EXIT_ERROR) {
        AK_dbg_messg(LOW, FILE_MAN, "AK_init_new_segment__ERROR: Cannot initialize segment!\n");
        AK_EPI;
        return EXIT_ERROR;

    } else {

        end_address += start_address;

        struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
        AK_Init_L3(&row_root);


        AK_Insert_New_Element(TYPE_INT, &objectID, sys_table, "obj_id", row_root);
        AK_Insert_New_Element(TYPE_VARCHAR, name, sys_table, "name", row_root);
        AK_Insert_New_Element(TYPE_INT, &start_address, sys_table, "start_address", row_root);
        AK_Insert_New_Element(TYPE_INT, &end_address, sys_table, "end_address", row_root);
        AK_Insert_New_Element(TYPE_VARCHAR, table_id, sys_table, "table_id", row_root);
        AK_Insert_New_Element(TYPE_INT, &attr_id, sys_table, "attribute_id", row_root);

        AK_insert_row(row_root);

        AK_dbg_messg(LOW, FILE_MAN, "AK_init_new_segment__NOTIFICATION: New segment initialized at %d\n", start_address);
        AK_EPI;
        return start_address;
    }
    AK_EPI;
}


/**
  * @author Unknown
  * @brief Test function
  * @return No return value
  */
TestResult AK_files_test() {
    AK_header header[MAX_ATTRIBUTES], header1[MAX_ATTRIBUTES];
    AK_PRO;
    memset(header, '\0', MAX_ATTRIBUTES);
    memset(header1, '\0', MAX_ATTRIBUTES);

    int i, i1;
    char *ime = "Tomo";
    char *ime1 = "Tomo1";

    header[0].integrity[0] = 21;
    header[0].type = 1;

    header1[0].integrity[0] = 21;
    header1[0].type = 1;

    i = AK_initialize_new_segment(ime, SEGMENT_TYPE_TABLE, header);
    if (i != EXIT_ERROR)
        printf("AK_init_new_segment: Test succeded!\n");
    i1 = AK_initialize_new_segment(ime1, SEGMENT_TYPE_TABLE, header1);
    if (i1 != EXIT_ERROR)
        printf("AK_init_new_segment: Test1 succeded!\n");

    AK_printout_redolog();
    AK_EPI;

    return TEST_result(0,0);
}
