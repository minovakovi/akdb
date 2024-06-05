/**
@file privileges.h Header file that provides functions and defines for privileges.c
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
#ifndef PRIVILEGES
#define PRIVILEGES

#include "../auxi/test.h"
#include "../file/table.h"
#include "../file/fileio.h"
#include "../file/id.h"
#include "../rec/archive_log.h"
#include "../auxi/mempro.h"


/**
 * @author Kristina Takač.
 * @brief  Inserts a new user in the AK_user table 
 * @param *username username of user to be added
 * @param *password password of user to be added
 * @param set_id obj_id of the new user
 * @return user_id
 */
int AK_user_add(char *username, int *password, int set_id);

/**
 * @author Kristina Takač.
 * @brief  Function that returns an ID of the given user
 * @param *username username of user whose id we are looking for
 * @return user_id, otherwise EXIT_ERROR
 */
int AK_user_get_id(char *username);
/**
* @author Fran MIkolić.
* @brief  Function that checks if there is user with given password
* @param *username username of user whose password we are checking
* @param *password password of given username whom we will check
* @return check 0 if false or 1 if true
*/
int AK_user_check_pass(char *username, int *password);
/**
 * @author Kristina Takač, edited by Ljubo Barać
 * @brief Function that adds a new group
 * @param *name name of group to be added
 * @param set_id non default id to be passed
 * @return id of group                                                      
 */
int AK_group_add(char *name, int set_id);
/**
 * @author Kristina Takač.
 * @brief Function that returns the ID  from the given group name
 * @param *name name of group whose id we are looking for
 * @return id of group, otherwise EXIT_ERROR                                                      
 */
int AK_group_get_id(char *name);
/**
 * @author Kristina Takač, updated by Mario Peroković, inserting user id instead of username in AK_user_right, updated by Marko Flajšek
 * @brief Function that grants a specific privilege to the desired user on a given table
 * @param *username username of user to whom we want to grant privilege
 * @param *table name of table on which privilege will be granted to user
 * @param *right type of privilege which will be granted to user on given table
 * @return privilege_id or EXIT_ERROR if table or user aren't correct
 */
int AK_grant_privilege_user(char *username, char *table, char *right);
/**
 * @author Kristina Takač, updated by Mario Peroković - added comparing by table id, and use of user_id in AK_user_right
 * @brief Function that revokes users privilege on the given table
 * @param *username username of user to whom we want to grant privilege
 * @param *table name of table on which privilege will be revoked from user
 * @param *right type of privilege which will be revoked from user on given table
 * @return EXIT_SUCCESS if privilege is revoked, EXIT_ERROR if it isn't
 */
int AK_revoke_privilege_user(char *username, char *table, char *right);
/**
 * @author Jurica Hlevnjak, updated by Marko Flajšek
 * @brief Function that revokes ALL user's privileges on ALL tables (for DROP user)
 * @param username name of user from whom we want to revoke all privileges
 * @return EXIT_SUCCESS if privilege is revoked, EXIT_ERROR if it isn't
 */
int AK_revoke_all_privileges_user(char *username);
/**
 * @author Kristina Takač.
 * @brief Function that grants a privilege to a given group on a given table
 * @param *groupname  name of group to which we want to grant privilege
 * @param *table name of table on which privilege will be granted to user
 * @param *right type of privilege which will be granted to user on given table
 * @return privilege_id or EXIT_ERROR if table or user aren't correct
 */
int AK_grant_privilege_group(char *groupname, char *table, char *right);
/**
 * @author Kristina Takač, updated by Mario Peroković - added comparing by table id
 * @brief Function that revokes a groups privilege on the given table 
 * @param *grounamep name of group which user belongs to
 * @param *table name of table on which privilege will be granted to group
 * @param *right type of privilege which will be granted to group on a given table
 * @return EXIT_SUCCESS if privilege is revoked, EXIT_ERROR if it isn't
 */
int AK_revoke_privilege_group(char *groupname, char *table, char *right);
/**
 * @author Jurica Hlevnjak
 * @brief Function that revokes ALL privileges from the desired group on ALL tables (needed for DROP group)
 * @param groupname name of group from which we want to revoke all privileges
 * @return EXIT_SUCCESS if privilege is revoked, EXIT_ERROR if it isn't
 */
int AK_revoke_all_privileges_group(char *groupname);
/**
 * @author Kristina Takač, updated by Mario Peroković, added verifying the existence of user in the group, updated by Maja Vračan 
 * @brief Function that puts the desired user in the given group
 * @param *user username of user which will be put in group
 * @param *group name of group in which user will be put
 * @return EXIT_SUCCESS or EXIT_ERROR if the user is already in the group
 */
int AK_add_user_to_group(char *user, char *group);
/**
 * @author Jurica Hlevnjak, update by Lidija Lastavec
 * @brief Function that removes user from all groups. Used for DROP user.
 * @param user name of user
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_remove_user_from_all_groups(char *user);
/**
 * @author Jurica Hlevnjak, update by Lidija Lastavec
 * @brief Function that removes all users from a group. Used for DROP group.
 * @param group name of group
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_remove_all_users_from_group(char *group);
/**
 * @author Kristina Takač, updated by Marko Flajšek
 * @brief Function that checks whether the given user has a right for the given operation on the given table
 * @param *user username for which we want check privileges
 * @param *table name of table for which we want to check whether user has right on
 * @param *privilege privilege for which we want to check whether user has right for
 * @return EXIT_SUCCESS if user has right, EXIT_ERROR if user has no right
 */
int AK_check_privilege(char *username, char *table, char *privilege);
/**
 * @author Jurica Hlevnjak, updated by Lidija Lastavec
 * @brief Function that checks if the user has any privileges or belongs to any group. Used in drop user for restriction.
 * @param user name of user
 * @return EXIT_ERROR or EXIT_SUCCESS
 */
int AK_check_user_privilege(char *user);
/**
 * @author Jurica Hlevnjak, updated by Lidija Lastavec, updated by Marko Flajšek
 * @brief Function that checks if the group has any privileges. Used in drop group for restriction.
 * @param group name of group
 * @return EXIT_ERROR or EXIT_SUCCESS
 */
int AK_check_group_privilege(char *group);
/**
 * @author Ljubo Barać
 * @brief Function that removes the given group
 * @param name Name of the group to be removed
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_group_remove_by_name(char *name);
/**
 * @author Ljubo Barać, update by Lidija Lastavec, update by Marko Flajšek
 * @brief Function that renames a given user
 * @param old_name Name of the user to be renamed
 * @param new_name New name of the user
 * @param password Password of the user to be renamed (should be provided)
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_user_rename(char *old_name, char *new_name, int *password);
/**
 * @author Ljubo Barać, update by Lidija Lastavec
 * @brief Function that renames the given group
 * @param old_name Name of the group to be renamed
 * @param new_name New name of the group
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_group_rename(char *old_name, char *new_name);
TestResult AK_privileges_test();

#endif
