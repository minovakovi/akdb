/**
    @file blobs.c Provides functions for manipulations of binary large objects
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

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fcntl.h>
#include <errno.h>

#ifdef _WIN32
#include <direct.h>
#endif

#include "../dm/dbman.h" // system catalog
#include "../auxi/configuration.h" // blobs folder path is specified here
#include "blobs.h"

int success = 0;
int failed = 0;

AK_File_Metadata AK_File_Metadata_malloc() {

  AK_File_Metadata meta = (AK_File_Metadata) AK_malloc(sizeof(AK_Metadata));
  meta->new_path = (char*) AK_malloc(sizeof(char) * 512);
  meta->new_name = (char*) AK_malloc(sizeof(char) * 128);
  meta->old_path = (char*) AK_malloc(sizeof(char) * 512);
  meta->old_name = (char*) AK_malloc(sizeof(char) * 128);
  meta->checksum = (char*) AK_malloc(sizeof(char) * 128);
  return meta;

}

/**
 * @author Samuel Picek
 * @brief Function that generates GUID
 * @return returns globaly universal identifier based on kernel implementation
 */
char *AK_GUID() {
  srand (clock());
  char *GUID = (char*) AK_malloc(sizeof(char) * 40);
  int t = 0;
  char *szTemp = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";
  char *szHex = "0123456789ABCDEF-";
  int nLen = strlen (szTemp);

  for (t = 0; t < nLen + 1; t++)
  {
      int r = rand () % 16;
      char c = ' ';

      switch (szTemp[t])
      {
          case 'x' : { c = szHex [r]; } break;
          case 'y' : { c = szHex [r & 0x03 | 0x08]; } break;
          case '-' : { c = '-'; } break;
          case '4' : { c = '4'; } break;
      }

      GUID[t] = ( t < nLen ) ? c : 0x00;
  }

  return GUID;
}


/**
 * @author Samuel Picek
 * @brief Function that checks if folder blobs already exists
 * @return returns 0 for true and 1 for false
 */
int AK_folder_exists(char *foldername) {
  struct stat st;
  stat(foldername, &st);
  return st.st_mode & S_IFDIR;
}


/**
 * @author Samuel Picek
 * @brief Function that creates new folder
 * @return returns 0 for true and 1 for false
 */
int AK_mkdir(const char *path) {
   #ifdef _WIN32 // not tested
        return _mkdir(path);
    #else
    #if _POSIX_C_SOURCE
        return mkdir(path, 0755);
    #else
        return mkdir(path, 0755); // not sure if this works on mac
    #endif
    #endif
}




int AK_copy(const char *from, const char *to) {
  //printf("Copying %s\n to %s\n", from, to);
  FILE  *ptr_old, *ptr_new;
	char  ch;


	ptr_old = fopen(from, "rb");


  printf("[INFO] Copying file from %s to %s\n", from, to);
  //printf("ptr_old %s\n", ptr_old);
	if(ptr_old == NULL)
  {
    perror("[ERROR]");
    return -1;
  }


  ptr_new = fopen(to, "wb");
	if(ptr_new == NULL)
  {
    perror("[ERROR]");
    return  -1;
  }


	while( ( ch = fgetc(ptr_old) ) != EOF )
      fputc(ch, ptr_new);

	fclose(ptr_new);
	fclose(ptr_old);
  printf("[INFO] Files closed.\n");
	return  0;
}




/**
 * @author Samuel Picek
 * @brief Function for AK_concatinating 2 strings
 * @return returns new string
 */
char* AK_concat(char *s1, char *s2)
{
    char *result = AK_malloc( strlen(s1) + strlen(s2) + 1); // +1 for the zero-terminator
    if (result != NULL) {
        strcpy(result, s1);
        strcat(result, s2);
        return result;
    } else
      return NULL;
}


char *AK_clear_all_newline(char *s) {

    char *n = malloc( strlen( s ? s : "\n" ) );
    if( s )
        strcpy( n, s );
    n[strlen(n)-1]='\0';
    return n;
}


/**
 * @author Samuel Picek
 * @brief Function that checks if folder blobs exists
 * @return OID (object ID)
 */
int AK_check_folder_blobs() {
  struct stat s;
  int err = stat(AK_BLOBS_PATH, &s);
  if(-1 == err) {
      if(ENOENT == errno) {
          printf("[INFO] There is no folder for blobs. Trying to create folder at: %s\n", AK_BLOBS_PATH);
          AK_mkdir(AK_BLOBS_PATH);
          //AK_create_blobs_table();
      } else {
          perror("stat");
          exit(1);
      }
  } else {
      if(S_ISDIR(s.st_mode)) {
        printf("[INFO] Folder blobs already exists\n");
      } else {
          /* exists but is no folder */
      }
  }
  return 1;
}



/**
 * @author Samuel Picek
 * @brief Function that splits a path from filename
 * @return void
 */
void AK_split_path_file(char **p, char** f, char *pf) {
  /* Find last delimiter. */
  char *z;
  for (z = pf + strlen(pf); z >= pf; z--) {
    if (*z == '/' || *z == '\\')
      break;
  }

  if (z >= pf) {
    *p = malloc(z - pf + 1);
    strncpy(*p, pf, z - pf);
    (*p)[z - pf] = '\0';
    *f = malloc(strlen(z));
    strcpy(*f, z + 1);
  } else {
    *p = NULL;
    *f = malloc(strlen(pf) + 1);
    strcpy(*f, pf);
  }
}




//TODO: Needs description, author and params
int AK_write_metadata(char *oid, AK_File_Metadata meta) {


  FILE *f = fopen(AK_concat(meta->new_path, ".meta"), "w");

  if (f == NULL)
  {
      return -1;
  }

  fprintf(f, "%s\n", meta->new_name);
  fprintf(f, "%s\n", meta->new_path);
  fprintf(f, "%s\n", meta->old_name);
  fprintf(f, "%s\n", meta->old_path);
  fprintf(f, "%s\n", meta->checksum);

  fclose(f);
  return 0;
}



//TODO: Needs description, author and params
AK_File_Metadata AK_read_metadata(char *oid) {

  if (oid == 0x0)
    return -1;

  AK_File_Metadata meta = AK_File_Metadata_malloc();

  int iteration = 0;
  char line[128];
  size_t len = 128;
  ssize_t read;


  char *blobs = AK_concat(AK_BLOBS_PATH, "/");
  char *oid_file = AK_concat(blobs, oid);
  char *metadata = AK_concat(oid_file, ".meta");


  printf("[INFO] Opening file at %s\n", metadata);
  FILE *fp = fopen(metadata, "r");

  if (fp == NULL)
  {
    printf("\t[+] Cannot open file.\n");
    return -1;
  } else
  {
    printf("\t[+] File opened.\n");
  }

  fgets( line, sizeof(line), fp);
  strcpy(meta->new_name, line);

  meta->new_name = AK_clear_all_newline(meta->new_name);


  if (meta->new_name == 0x0)
    return 0x0;


  fgets( line, sizeof(line), fp);
  strcpy(meta->new_path, line);
  meta->new_path = AK_clear_all_newline(meta->new_path);

  if (meta->new_path == 0x0)
    return 0x0;

  fgets( line, sizeof(line), fp);
  strcpy(meta->old_name, line);
  meta->old_name = AK_clear_all_newline(meta->old_name);

  if (meta->old_name == 0x0)
    return 0x0;

  fgets( line, sizeof(line), fp);
  strcpy(meta->old_path, line);
  meta->old_path = AK_clear_all_newline(meta->old_path);

  if (meta->old_path == 0x0)
    return 0x0;

  fclose(fp);
  printf("\t[+] File closed.\n");

  return meta;
}



/**
 * @author Samuel Picek
 * @brief Function that imports  large objects to database
 * @return OID (object ID)
 */
char *AK_lo_import(char *filepath) {

  printf("-----------------------------------------------------------------\n");
  printf("[INFO] IMPORTING\n");
  printf("-----------------------------------------------------------------\n");

  AK_File_Metadata meta = AK_File_Metadata_malloc();
  char *oid = AK_GUID();
  meta->checksum = "";

  char *old_name = (char) AK_malloc(sizeof(char) * 128); // filename
  char *old_path = (char) AK_malloc(sizeof(char) * 512); // filepath from where file is imported (without trailing slash)

  AK_split_path_file(&old_path, &old_name, filepath);


  strcpy(meta->new_name, oid);
  strcpy(meta->old_name, old_name);
  strcpy(meta->old_path, old_path);

  meta->new_path = AK_concat(AK_concat(AK_BLOBS_PATH, "/"), oid);

  if ( AK_copy(filepath, meta->new_path) == 0 ) {
    printf("[INFO] Large object imported successfully.\n\t[+] Object is at %s\n", meta->new_path);
    printf("[INFO] Importing metadata to catalog\n");


    if (AK_write_metadata(oid, meta) == 0) {
      printf("[INFO] Success\n");
      success++;
    } else {
		failed++;
      printf("[INFO] There was an error writing metadata\n");
    }
  } else {
	  failed++;
    printf("[ERROR] There was an error while importing large object.\n");
  }


  return oid;
}


/**
 * @author Samuel Picek
 * @brief Function that retrieves large objects
 * @return returns 0 for true and 1 for false
 */
int AK_lo_export(char *oid, char *filepath) {

  printf("-----------------------------------------------------------------\n");
  printf("[INFO] EXPORTING\n");
  printf("-----------------------------------------------------------------\n");

  AK_File_Metadata metadata = AK_read_metadata(oid);


  char *destination_name = (char) AK_malloc(sizeof(char) * 128);
  char *destination_path = (char) AK_malloc(sizeof(char) * 512);
  AK_split_path_file(&destination_path, &destination_name, filepath);

  if (metadata != 0x0)
  {
    if ( AK_copy(metadata->new_path, filepath) == 0 )
    {
      printf("[INFO] File exported successfully.\n");
      success++;
    } else
    {
		failed++;
      printf("[INFO] There was an error exporting a file.\n");
    }
  } else
  {
	failed++;
    return -1;
  }

  return 0;
}


/**
 * @author Samuel Picek
 * @brief Function that deletes large objects
 * @return OID (object ID)
 */
int AK_lo_unlink(char *oid) {

  printf("-----------------------------------------------------------------\n");
  printf("[INFO] UNLINKING\n");
  printf("-----------------------------------------------------------------\n");

  AK_File_Metadata metadata = AK_read_metadata(oid);

  if (metadata != 0x0)
  {
    remove(metadata->new_path);
    remove(AK_concat(metadata->new_path, ".meta"));
    printf("[INFO] File removed successfully\n");
    success++;
  }
  else
  {
	  failed++;
  }

  return 0;
}


/**
 * @author Samuel Picek
 * @brief  Tests
 */
TestResult AK_lo_test() {

  FILE *f;
  FILE *f2;
long size;
long size2;

  printf("[INFO] Starting AK_lo_test.\n");
  char *oid = AK_lo_import("./config.ini");
  AK_lo_export(oid, "/tmp/config.ini");
  AK_lo_unlink(oid);

  f = fopen("./config.ini", "rb");
  if (f == NULL)
  {
     printf("config.ini orginal doesent exist\n");
  }
  fseek(f, 0, SEEK_END);
  size = ftell(f);
  fclose(f);

  f2 = fopen("/tmp/config.ini", "rb");
  if (f2 == NULL) 
  {
      printf("config.ini in temp doesent exist\n");
  }
  fseek(f2, 0, SEEK_END);
  size2 = ftell(f2);
  fclose(f2);

  if (f2 == f) 
  {
     printf("config.ini in temp and orginal are same\n");
  }

  return TEST_result(success,failed);
}
