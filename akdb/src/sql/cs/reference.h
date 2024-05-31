/**
đ@file reference.h Provides data structures, functions and defines for referential integrity
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
 
#ifndef REFERENCE
#define REFERENCE

#include "../../auxi/test.h"
#include "../../dm/dbman.h"
#include "../../file/table.h"
#include "../../auxi/mempro.h"
/**
 * @def REF_TYPE_NONE
 * @brief Constant declaring none reference type 
*/
#define REF_TYPE_NONE -1
/**
 * @def REF_TYPE_SET_NULL
 * @brief Constant declaring set null reference type 
*/
#define REF_TYPE_SET_NULL 1
/**
 * @def REF_TYPE_NO_ACTION
 * @brief Constant declaring no action reference type 
*/
#define REF_TYPE_NO_ACTION 2
/**
 * @def REF_TYPE_NO_ACTION
 * @brief Constant declaring cascade reference type 
*/
#define REF_TYPE_CASCADE 3
/**
 * @def REF_TYPE_RESTRICT
 * @brief Constant declaring restrict reference type 
*/
#define REF_TYPE_RESTRICT 4
/**
 * @def REF_TYPE_SET_DEFAULT
 * @brief Constant declaring set default reference type 
*/
#define REF_TYPE_SET_DEFAULT 5 // reserved for future use
/**
 * @def MAX_REFERENCE_ATTRIBUTES
 * @brief Constant declaring maximum number of reference attributes
*/
#define MAX_REFERENCE_ATTRIBUTES 10 //how many attributes can be define in a single foreign key constraint
/**
 * @def MAX_CHILD_CONSTRAINTS
 * @brief Constant declaring maximum number of child constraints
*/
#define MAX_CHILD_CONSTRAINTS 20 // how many references can be made on one parent table, this will be deprecated by the use of a list
/**
 * @author Dejan Franković
 * @struct AK_ref_item
 * @brief Structure that represents reference item. It contains of table, attributes, parent table and it's attributes, number of attributes, constraint
	  and type of reference.
 */
typedef struct {
    char table[MAX_ATT_NAME];
    char attributes[MAX_REFERENCE_ATTRIBUTES][MAX_ATT_NAME];
    char parent[MAX_ATT_NAME];
    char parent_attributes[MAX_REFERENCE_ATTRIBUTES][MAX_ATT_NAME];
    int attributes_number;
    char constraint[MAX_VARCHAR_LENGTH];
    int type;
} AK_ref_item;

/**
 * @author Dejan Frankovic
 * @brief Function that adds a reference for a group of attributes over a given table to a group of attributes over another table with a given constraint name.
 * @param name of the child table
 * @param array of child table attribute names (foreign key attributes)
 * @param name of the parent table
 * @param array of parent table attribute names (primary key attributes)
 * @param number of attributes in foreign key
 * @param name of the constraint
 * @param type of the constraint, constants defined in 'reference.h'
 * @return EXIT_SUCCESS
 */
int AK_add_reference(char *childTable, char *childAttNames[], char *parentTable, char *parentAttNames[], int attNum, char *constraintName, int type) ;

/**
 * @author Dejan Frankovic
 * @brief Function that reads a reference entry from system table.
 * @param name of the table with reference (with foreign key)
 * @param name of the reference constraint
 * @return AK_ref_item object with all neccessary information about the reference
 */
AK_ref_item AK_get_reference(char *tableName, char *constraintName) ;
/**
 * @author Dejan Frankovic
 * @brief Function that checks referential integrity for one attribute
 * @param child table name
 * @param attribute name (foreign key attribute)
 * @param value of the attribute we're checking
 * @return EXIT ERROR if check failed, EXIT_SUCCESS if referential integrity is ok
 */
int AK_reference_check_attribute(char *tableName, char *attribute, char *value) ;
/*
int AK_reference_check_if_update_needed(AK_list *lista, int action) ;
int AK_reference_check_restricion(AK_list *lista, int action) ;
int AK_reference_update(AK_list *lista, int action) ;
int AK_reference_check_entry(AK_list *lista) ;
*/

/**
 * @author Dejan Frankovic
 * @brief Funcction that quickly checks if there are any referential constraints that should be applied on a given list of changes.
 * @param list of elements for update
 * @param is action UPDATE or DELETE ?
 * @return EXIT_SUCCESS if update is needed, EXIT_ERROR if not
 */
int AK_reference_check_if_update_needed(struct list_node *lista, int action) ;

/**
 * @author Dejan Franković
 * @brief Function that checks for a REF_TYPE_RESTRICT references appliable to the operation of updating or deleting a row in a table.
 * @param list of elements for update
 * @param is action UPDATE or DELETE?
 * @return EXIT_SUCCESS if there is no restriction on this action, EXIT_ERROR if there is
 */
int AK_reference_check_restricion(struct list_node *lista, int action) ;

/**
 * @author Dejan Franković
 * @brief Function that updates child table entries according to ongoing update of parent table entries.
 * @param list of elements for update
 * @param is action UPDATE or DELETE ?
 * @return EXIT_SUCCESS
 */
int AK_reference_update(struct list_node *lista, int action) ;

/**
 * @author Dejan Franković
 * @brief Function that checks a new entry for referential integrity.
 * @param list of elements for insert row
 * @return EXIT_SUCCESS if referential integrity is ok, EXIT_ERROR if it is compromised
 */
int AK_reference_check_entry(struct list_node *lista) ;

TestResult AK_reference_test();
/*
void AK_Insert_New_Element(int newtype, void * data, char * table, char * attribute_name, AK_list_elem ElementBefore);
int AK_insert_row(AK_list *row_root);
int AK_selection(char *srcTable, char *dstTable, AK_list *expr);
void AK_Insert_New_Element_For_Update(int newtype, void * data, char * table, char * attribute_name, AK_list_elem ElementBefore, int newconstraint);
int AK_delete_row(AK_list *row_root) ;
int AK_update_row(AK_list *row_root);
*/
void AK_Insert_New_Element(int newtype, void * data, char * table, char * attribute_name, struct list_node *ElementBefore);
void AK_Update_Existing_Element(int newtype, void * data, char * table, char * attribute_name, struct list_node *ElementBefore);
int AK_insert_row(struct list_node *row_root);
int AK_selection(char *srcTable, char *dstTable, struct list_node *expr);
void AK_Insert_New_Element_For_Update(int newtype, void * data, char * table, char * attribute_name, struct list_node *ElementBefore, int newconstraint);
int AK_delete_row(struct list_node *row_root) ;
int AK_update_row(struct list_node *row_root);

int AK_initialize_new_segment(char *name, int type, AK_header *header);

#endif
