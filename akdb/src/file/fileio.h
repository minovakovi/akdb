/**
@file fileio.h Header file provides functions and defines for file input/output
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
#ifndef FILEIO
#define FILEIO

#include "../auxi/test.h"
#include "../auxi/constants.h"
#include "../sql/cs/reference.h"
#include "../mm/memoman.h"
#include "../rec/recovery.h"
#include "../rec/archive_log.h"
#include "../rec/redo_log.h"
#include "files.h"
#include "../auxi/mempro.h"

/**
   * @author Matija Novak
   * @brief !! YOU PROBABLY DON'T WANT TO USE THIS FUNCTION !! - Use AK_Update_Existing_Element or AK_Insert_New_Element instead. 
   Function inserts new element after some element, to insert on first place give list as before element. New element
            is allocated. Type, data, attribute name and constraint of new elemets are set according to function arguments. Pointers
            are changed so that before element points to new element.
   * @param newtype type of the data
   * @param data the data
   * @param table table name
   * @param attribute_name attribute name
   * @param element element after we which insert the new element
   * @param constraint NEW_VALUE if data is new value, SEARCH_CONSTRAINT if data is constraint to search for
   * @return No return value
 */
void AK_Insert_New_Element_For_Update(int newtype, void * data, char * table, char * attribute_name, struct list_node * ElementBefore, int newconstraint);

/**
   * @author Matija Novak, changed by Dino Laktašić
   * @brief Used to add a new element after some element, to insert on first place give list as before element. It calls function
            AK_Insert_New_Element_For_Update.
   * @param newtype type of the data
   * @param data the data
   * @param table table name
   * @param attribute_name attribute name
   * @param element element after we which insert the new element
   * @param constraint is NEW_VALUE
   * @return No return value
 */
void AK_Insert_New_Element(int newtype, void * data, char * table, char * attribute_name, struct list_node * ElementBefore);

/** @author Matija Novak, updated by Dino Laktašić
        @brief Function inserts one row into some block.  Firstly it checks wether block contain attributes from the list. Then
               data, type, size and last_tuple_id are put in temp_block.
        @param row_root list of elements to insert
        @param temp_block block in which we insert data
        @return EXIT SUCCES if success
 */
int AK_insert_row_to_block(struct list_node *row_root, AK_block *temp_block);

/** @author Matija Novak, updated by Matija Šestak (function now uses caching), updated by Dejan Frankovic (added reference check), updated by Dino         Laktašić (removed variable AK_free, variable table initialized using memset)
        @brief Function inserts a one row into table. Firstly it is checked whether inserted row would violite reference integrity.
        Then it is checked in which table should row be inserted. If there is no AK_free space for new table, new extent is allocated. New block is            allocated on given address. Row is inserted in this block and dirty flag is set to BLOCK_DIRTY.
        @param row_root list of elements which contain data of one row
        @return EXIT_SUCCESS if success else EXIT_ERROR

 */
int AK_insert_row(struct list_node *row_root);

/**
   * @author Matija Novak, updated by Dino Laktašić, updated by Mario Peroković - separated from deletion
   * @brief Function updates row from table in given block.
   * @param temp_block block to work with
   * @param row_list list of elements which contain data for delete or update
   * @return No return value
*/
int AK_update_row_from_block(AK_block *temp_block, struct list_node *row_root);

/**
   * @author Matija Novak, updated by Dino Laktašić, changed by Davorin Vukelic, updated by Mario Peroković
   * @brief Function deletes row from table in given block. Given list of elements is firstly back-upped.
   * @param temp_block block to work with
   * @param row_list list of elements which contain data for delete or update
   * @return No return value
*/
void AK_delete_row_from_block(AK_block *temp_block, struct list_node *row_root);

/**
      * @author Matija Novak, updated by Matija Šestak (function now uses caching)
      * @brief Function updates or deletes the whole segment of an table. Addresses for given table atr fetched. For each block
        in extent row is updated or deleted according to operator del.
      * @param row_root elements of one row
      * @param del - DELETE or UPDATE
      * @return EXIT_SUCCESS if success
*/
int AK_delete_update_segment(struct list_node *row_root, int del);

/** @author Matija Novak, Dejan Frankovic (added referential integrity)
        @brief Function deletes rows
        @param row_root elements of one row
        @returs EXIT_SUCCESS if success
 */
int AK_delete_row(struct list_node *row_root) ;

/** @author Matija Novak, Dejan Frankovic (added referential integrity)
        @brief Function updates rows of some table
        @param row_root elements of one row
        @return EXIT_SUCCESS if success
*/
int AK_update_row(struct list_node *row_root);
TestResult AK_fileio_test();

/**
 *@author Dražen Bandić
 *@brief Function deletes row by id
 *@param id id of row
 *@param tableName name of table to delete the row
 */
void AK_delete_row_by_id(int id, char* tableName);

#endif
