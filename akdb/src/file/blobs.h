/**
    @file blobs.h Provides data structures, functions and defines for manipulating blobs
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

#ifndef BLOBS
#define BLOBS

#include "../auxi/test.h"
#include "table.h"
#include "fileio.h"

#include "id.h"

struct _file_metadata {
  char *new_path;
  char *new_name;
  char *old_path;
  char *old_name;
  char *checksum;
};

typedef struct _file_metadata AK_Metadata;
typedef struct _file_metadata *AK_File_Metadata;

AK_File_Metadata AK_File_Metadata_malloc();

/* Helper functions */
/**
 * @author Samuel Picek
 * @brief Function that creates new folder
 * @return returns 0 for true and 1 for false
 */
int AK_mkdir(const char *path);
int AK_mkdir(const char *path);
int AK_copy(const char *from, const char *to);

/**
 * @author Samuel Picek
 * @brief Function for AK_concatinating 2 strings
 * @return returns new string
 */
char *AK_concat(char *s1, char *s2);
char *AK_clear_all_newline(char *str);

/**
 * @author Samuel Picek
 * @brief Function that splits a path from filename
 * @return void
 */
void AK_split_path_file(char **p, char **f, char *pf);

/**
 * @author Samuel Picek
 * @brief Function that generates GUID
 * @return returns globaly universal identifier based on kernel implementation
 */

char *AK_GUID();

/**
 * @author Samuel Picek
 * @brief Function that checks if folder blobs already exists
 * @return returns 0 for true and 1 for false
 */
int AK_folder_exists(char *foldername);

/**
 * @author Samuel Picek
 * @brief Function that checks if folder blobs exists
 * @return OID (object ID)
 */
int AK_check_folder_blobs();

int AK_write_metadata(char *oid, AK_File_Metadata meta);
AK_File_Metadata AK_read_metadata(char *oid);

/**
 * @author Samuel Picek
 * @brief Function that imports  large objects to database
 * @return OID (object ID)
 */
char *AK_lo_import(char *filepath);

/**
 * @author Samuel Picek
 * @brief Function that retrieves large objects
 * @return returns 0 for true and 1 for false
 */
int AK_lo_export(char *oid, char *filepath);

/**
 * @author Samuel Picek
 * @brief Function that deletes large objects
 * @return OID (object ID)
 */
int AK_lo_unlink(char *oid);

TestResult AK_lo_test();

#endif
