/**
@file privileges.c Provides functions for privileges
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
#include "privileges.h"
#include <unistd.h>
#include "../auxi/constants.h"
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define SALT_LEN 16
#define SALT_HEX_LEN (SALT_LEN * 2)
#define HASH_HEX_LEN (EVP_MAX_MD_SIZE * 2)

/**
 * @author Luka Balažinec
 * @brief Reads len bytes from /dev/urandom into salt.
 * @param salt Buffer to receive len random bytes.
 * @param len Number of bytes to read.
 * @return 0 on success, -1 on error.
 */
static int generate_salt(unsigned char *salt, size_t len) {
    FILE *f = fopen("/dev/urandom", "rb");
    if (!f) return -1;
    if (fread(salt, 1, len, f) != len) {
        fclose(f);
        return -1;
    }
    fclose(f);
    return 0;
}

/**
 * @author Luka Balažinec
 * @brief Converts a binary buffer to a hex string.
 * @param in Input buffer of in_len bytes.
 * @param in_len Number of bytes in the input buffer.
 * @param out Buffer to receive the hex string.
 */
static void to_hex(const unsigned char *in, size_t in_len, char *out) {
    for (size_t i = 0; i < in_len; i++)
        sprintf(out + i*2, "%02x", in[i]);
    out[in_len*2] = '\0';
}

/**
 * @author Luka Balažinec
 * @brief Hashes a password with a hex-encoded salt using SHA-256.
 * @param password Plain-text password.
 * @param salt_hex Hex string of the salt (length SALT_HEX_LEN).
 * @param out_hash_hex Buffer to receive the resulting hash as a hex string.
 */
static void hash_password_with_salt(const char *password, const char *salt_hex, char *out_hash_hex) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX ctx;

    SHA256_Init(&ctx);
    SHA256_Update(&ctx, (unsigned char*)salt_hex, strlen(salt_hex));
    SHA256_Update(&ctx, (unsigned char*)password,  strlen(password));
    SHA256_Final(hash, &ctx);

    to_hex(hash, SHA256_DIGEST_LENGTH, out_hash_hex);
}

/**
 * @author Luka Balažinec
 * @brief Validates the strength of a password based on multiple security rules.
 * @param password The password to validate.
 * @param username The associated username (used to prevent inclusion in the password).
 * @return 1 if the password is strong, 0 otherwise. Prints messages for each failed condition.
 */
int is_password_strong(const char *password, const char *username) {
    if (strlen(password) <= 8) {
        printf("Password must be longer than 8 characters.\n");
        return 0;
    }

    int has_lower = 0, has_upper = 0, has_digit = 0, has_special = 0;
    int repeat_count = 1;

    for (size_t i = 0; password[i]; i++) {
        if (islower(password[i])) has_lower = 1;
        else if (isupper(password[i])) has_upper = 1;
        else if (isdigit(password[i])) has_digit = 1;
        else if (ispunct(password[i])) has_special = 1;

        if (i > 0 && password[i] == password[i - 1]) {
            repeat_count++;
            if (repeat_count > 4) {
                printf("Password must not contain more than 4 identical consecutive characters.\n");
                return 0;
            }
        } else {
            repeat_count = 1;
        }
    }

    if (!has_lower) {
        printf("Password must contain at least one lowercase letter.\n");
        return 0;
    }
    if (!has_upper) {
        printf("Password must contain at least one uppercase letter.\n");
        return 0;
    }
    if (!has_digit) {
        printf("Password must contain at least one digit.\n");
        return 0;
    }
    if (!has_special) {
        printf("Password must contain at least one special character.\n");
        return 0;
    }

    size_t len = strlen(password);
    int is_palindrome = 1;
    for (size_t i = 0; i < len / 2; i++) {
        if (password[i] != password[len - 1 - i]) {
            is_palindrome = 0;
            break;
        }
    }
    if (is_palindrome) {
        printf("Password must not be a palindrome.\n");
        return 0;
    }

    if (strstr(password, username) != NULL) {
        printf("Password must not contain the username.\n");
        return 0;
    }

    const char *sequences[] = {
        "1234", "abcd", "qwertz", "asdf", "0000", "1111",
        "password", "admin", NULL
    };
    for (int i = 0; sequences[i]; i++) {
        if (strstr(password, sequences[i]) != NULL) {
            printf("Password must not contain common or predictable patterns like '%s'.\n", sequences[i]);
            return 0;
        }
    }

    return 1;
}

/**
 * @author Kristina Takač, edited by Borna Romić, update by Luka Balažinec
 * @brief  Inserts a new user in the AK_user table 
 * @param *username Username of the user to be added
 * @param *password Hashed password of the user to be added (SHA-256 hash as a string)
 * @param set_id Object ID of the new user
 * @return user_id
 */
int AK_user_add(char *username, char *password, int set_id) {
    char *tblName = "AK_user";
    int usernameCheck;
    AK_PRO;

    usernameCheck = AK_user_get_id(username);
    if (usernameCheck != EXIT_ERROR) {
        printf("Username '%s' is not available!\n", username);
        AK_EPI;
        return EXIT_ERROR;
    }

    if (!is_password_strong(password, username)) {
        return 0;
    }    

    unsigned char salt_bin[SALT_LEN];
    char salt_hex[SALT_HEX_LEN + 1] = {0};
    char hash_hex [HASH_HEX_LEN + 1] = {0};

    if (generate_salt(salt_bin, SALT_LEN) != 0) return EXIT_ERROR;
    to_hex(salt_bin, SALT_LEN, salt_hex);
    hash_password_with_salt(password, salt_hex, hash_hex);

    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row_root);

    int user_id = AK_get_id();
    if (set_id != 0) user_id = set_id;

    AK_Insert_New_Element(TYPE_INT,     &user_id, tblName, "obj_id", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, username,  tblName, "username", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, hash_hex,  tblName, "password_hash", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, salt_hex,  tblName, "salt", row_root);
    AK_insert_row(row_root);

    printf("\nAdded user '%s' under ID %d!\n\n", username, user_id);

    AK_free(row_root);
    AK_EPI;
    return user_id;
}

/**
 * @author Kristina Takač, updated by Barbara Tatai (fix leaks)
 * @brief  Function that returns an ID of the given user
 * @param *username username of user whose id we are looking for
 * @return user_id, otherwise EXIT_ERROR
 */
int AK_user_get_id(char *username) {
    int i = 0, id = EXIT_ERROR;
    struct list_node *row;
    AK_PRO;
    while ((row = (struct list_node *) AK_get_row(i, "AK_user")) != NULL && id == EXIT_ERROR) {
        struct list_node *elem_in_strcmp = AK_GetNth_L2(2, row);
        if (strcmp(elem_in_strcmp->data, username) == 0) {
            id = (int) *(AK_GetNth_L2(1, row)->data);            
        }
        i++;
        AK_DeleteAll_L3(&row);
        AK_free(row);  
    }
    AK_EPI;
    return id;
}

/**
 * @author Fran Mikolić, update by Luka Balažinec
 * @brief  Function that checks if a user exists with the given password
 * @param *username Username of the user whose password we are checking
 * @param *password Hashed password (SHA-256 hash as a string) of the given username to be checked
 * @return check 0 if false or 1 if true
 */
int AK_user_check_pass(char *username, char *password) {
    AK_PRO;

    int user_id = AK_user_get_id(username);
    if (user_id == EXIT_ERROR) {
        printf("User '%s' does not exist!\n", username);
        AK_EPI;
        return 0;
    }

    struct list_node *row = NULL;
    int i = 0;
    while ((row = (struct list_node *) AK_get_row(i, "AK_user")) != NULL) {
        int current_id = *(int *)AK_GetNth_L2(1, row)->data;
        if (current_id == user_id) break;
        AK_DeleteAll_L3(&row);
        AK_free(row);
        row = NULL;
        i++;
    }
    if (!row) {
        printf("Error fetching user data for user_id: %d!\n", user_id);
        AK_EPI;
        return 0;
    }

    char *stored_hash = (char *)AK_GetNth_L2(3, row)->data;
    char *salt_hex = (char *)AK_GetNth_L2(4, row)->data;
    stored_hash[strcspn(stored_hash, "\n")] = '\0';
    salt_hex   [strcspn(salt_hex, "\n")] = '\0';

    char hashed_input[HASH_HEX_LEN + 1] = {0};
    hash_password_with_salt(password, salt_hex, hashed_input);

    int result = (strcmp(stored_hash, hashed_input) == 0) ? 1 : 0;
    if (result)
        printf("Login successful!\n");
    else
        printf("Incorrect password!\n");

    AK_DeleteAll_L3(&row);
    AK_free(row);
    AK_EPI;
    return result;
}

/**
 * @author Luka Balažinec
 * @brief  Changes the password and salt for an existing user.
 * @param  username Username whose password is to be updated.
 * @param  new_password New plain-text password.
 * @return EXIT_SUCCESS on success, EXIT_ERROR on failure.
 */
int AK_user_change_password(char *username, char *new_password) {
    AK_PRO;

    int user_id = AK_user_get_id(username);
    if (user_id == EXIT_ERROR) {
        printf("User '%s' does not exist!\n", username);
        AK_EPI;
        return EXIT_ERROR;
    }

    if (!is_password_strong(new_password, username)) {
        return 0;
    }

    unsigned char salt_bin[SALT_LEN];
    char salt_hex[SALT_HEX_LEN + 1] = {0};
    char hash_hex [HASH_HEX_LEN + 1] = {0};

    if (generate_salt(salt_bin, SALT_LEN) != 0) {
        AK_EPI;
        return EXIT_ERROR;
    }
    to_hex(salt_bin, SALT_LEN, salt_hex);
    hash_password_with_salt(new_password, salt_hex, hash_hex);

    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row_root);

    AK_Update_Existing_Element(TYPE_VARCHAR, username, "AK_user", "username", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, hash_hex, "AK_user", "password_hash", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, salt_hex, "AK_user", "salt", row_root);

    if (AK_update_row(row_root) == EXIT_ERROR) {
        AK_DeleteAll_L3(&row_root);
        AK_free(row_root);
        AK_EPI;
        return EXIT_ERROR;
    }

    AK_DeleteAll_L3(&row_root);
    AK_free(row_root);
    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Ljubo Barać
 * @brief Function that removes the given user
 * @param name Name of the user to be removed
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_user_remove_by_name(char *name) {
    AK_PRO;

    int user_id = AK_user_get_id(name);

    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&row_root);
    AK_Update_Existing_Element(TYPE_VARCHAR, name, "AK_user", "username", row_root);
    int result = AK_delete_row(row_root);
    printf("\nRemoved user '%s' under ID %d!\n", name, user_id);

    AK_EPI;
    return result;
}

/**
 * @author Ljubo Barać, update by Lidija Lastavec, 
 * update by Marko Flajšek, update by Luka Balažinec
 * @brief Function that renames a given user
 * @param old_name Name of the user to be renamed
 * @param new_name New name of the user
 * @param password Password of the user to be renamed (hashed as a string)
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_user_rename(char *old_name, char *new_name, char *password) {
    AK_PRO;
    int result = 0;
    int user_id = AK_user_get_id(old_name);

    result = AK_user_remove_by_name(old_name);
    if (result == EXIT_ERROR) {
        printf("Failed to remove old user '%s'.\n", old_name);
        AK_EPI;
        return result;
    }

    result = AK_user_add(new_name, password, user_id);
    if (result == EXIT_ERROR) {
        printf("Failed to add new user '%s'.\n", new_name);
        AK_EPI;
        return result;
    }

    printf("Renamed user '%s' to '%s' under ID %d!\n", old_name, new_name, user_id);

    AK_EPI;
    return result;
}

/**
 * @author Kristina Takač, edited by Ljubo Barać, Borna Romić
 * @brief Function that adds a new group
 * @param *name name of group to be added
 * @param set_id non default id to be passed
 * @return id of group                                                      
 */
int AK_group_add(char *name, int set_id) {
    char *tblName = "AK_group";
    int groupNameCheck;
    char check = "";
    AK_PRO;

    groupNameCheck = AK_group_get_id(name);
    if (groupNameCheck != EXIT_ERROR) {
        printf("Name '%s' is not available!\n", name);
        check = "taken";
        AK_EPI;
        return check;
    }

    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&row_root);
    int group_id = AK_get_id();
    if (set_id != 0) group_id = set_id;

    AK_Insert_New_Element(TYPE_INT, &group_id, tblName, "obj_id", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, name, tblName, "name", row_root);
    AK_insert_row(row_root);
    printf("Added group '%s' under ID %d!\n", name, group_id);

    AK_free(row_root);

    AK_EPI;
    return group_id;
}

/**
 * @author Kristina Takač.
 * @brief Function that returns the ID  from the given group name
 * @param *name name of group whose id we are looking for
 * @return id of group, otherwise EXIT_ERROR                                                      
 */
int AK_group_get_id(char *name) {
    int i = 0;
    struct list_node *row;
    AK_PRO;

    while ((row = (struct list_node *) AK_get_row(i, "AK_group")) != NULL) {
        struct list_node *elem_in_strcmp = AK_GetNth_L2(2, row);
        if (strcmp(elem_in_strcmp->data, name) == 0) {
            i = (int) * row->next->data;
            AK_free(row);
            AK_EPI;
            return i;
        }
        i++;
    }
    AK_free(row);

    AK_EPI;
    return EXIT_ERROR;
}

/**
 * @author Ljubo Barać
 * @brief Function that removes the given group
 * @param name Name of the group to be removed
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_group_remove_by_name(char *name) {
    AK_PRO;
    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
    int group_id = AK_group_get_id(name);

    AK_Init_L3(&row_root);
    AK_Update_Existing_Element(TYPE_VARCHAR, name, "AK_group", "name", row_root);
    int result = AK_delete_row(row_root);
    printf("Removed group '%s' under ID %d!\n", name, group_id);

    AK_EPI;
    return result;
}

/**
 * @author Ljubo Barać, update by Lidija Lastavec
 * @brief Function that renames the given group
 * @param old_name Name of the group to be renamed
 * @param new_name New name of the group
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_group_rename(char *old_name, char *new_name) {
    AK_PRO;

    if (old_name == new_name) {
        printf("Please choose a different name!\n");
        AK_EPI;
        return EXIT_ERROR;
    }

    int result = 0;
    int *group_id = AK_group_get_id(old_name);

    result = AK_group_remove_by_name(old_name);
    result = AK_group_add(new_name, group_id);
    printf("Renamed group '%s' to '%s' under ID %d!\n", old_name, new_name, group_id);

    AK_EPI;
    return result;
}

/**
 * @author Kristina Takač, updated by Mario Peroković, inserting user id instead of username in AK_user_right, updated by Marko Flajšek
 * @brief Function that grants a specific privilege to the desired user on a given table
 * @param *username username of user to whom we want to grant privilege
 * @param *table name of table on which privilege will be granted to user
 * @param *right type of privilege which will be granted to user on given table
 * @return privilege_id or EXIT_ERROR if table or user aren't correct
 */
int AK_grant_privilege_user(char *username, char *table, char *right) {
    int privilege_id;
    AK_PRO;
    int table_id = AK_get_table_obj_id(table);
    int user_id = AK_user_get_id(username);

    if (table_id == EXIT_ERROR || user_id == EXIT_ERROR) {
        printf("Invalid table name or username!\n");
        AK_EPI;
        return EXIT_ERROR;
    }

    if (strcmp(right, "ALL") == 0) {
        char rights[100];
        strcpy(&rights[0], "UPDATE\0");
        strcpy(&rights[10], "DELETE\0");
        strcpy(&rights[20], "INSERT\0");
        strcpy(&rights[30], "SELECT\0");
        int i;

        for (i = 0; i < 4; i++) {
            struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
            AK_Init_L3(&row_root);
            privilege_id = AK_get_id();

            AK_Insert_New_Element(TYPE_INT, &privilege_id, "AK_user_right", "obj_id", row_root);
            AK_Insert_New_Element(TYPE_INT, &user_id, "AK_user_right", "name", row_root);
            AK_Insert_New_Element(TYPE_INT, &table_id, "AK_user_right", "artifact_id ", row_root);
            AK_Insert_New_Element(TYPE_VARCHAR, &rights[i * 10], "AK_user_right", "right_type", row_root);
            AK_insert_row(row_root);
            printf("\nGranted privilege to %s data for user '%s' under ID %d on table '%s'!\n\n", right, username, user_id, table);

            AK_free(row_root);
        }
    } else {
        struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
        AK_Init_L3(&row_root);

        privilege_id = AK_get_id();
        AK_Insert_New_Element(TYPE_INT, &privilege_id, "AK_user_right", "obj_id", row_root);
        AK_Insert_New_Element(TYPE_INT, &user_id, "AK_user_right", "name", row_root);
        AK_Insert_New_Element(TYPE_INT, &table_id, "AK_user_right", "artifact_id", row_root);
        AK_Insert_New_Element(TYPE_VARCHAR, right, "AK_user_right", "right_type", row_root);
        AK_insert_row(row_root);
        printf("\nGranted privilege to %s data for user '%s' under ID %d on table '%s'!\n\n", right, username, user_id, table);

        AK_free(row_root);
    }

    AK_EPI;
    return privilege_id;
}

/**
 * NOTICE: Test 12 isn't currently revoking a privilege since the obj_id in the AK_group_right table is passing the value of 127.
     Once the issue #87 on GitHub concerning the data type is solved, the test should be working as expected.
 * @author Kristina Takač, updated by Mario Peroković - added comparing by table id, and use of user_id in AK_user_right
 * @brief Function that revokes users privilege on the given table
 * @param *username username of user to whom we want to grant privilege
 * @param *table name of table on which privilege will be revoked from user
 * @param *right type of privilege which will be revoked from user on given table
 * @return EXIT_SUCCESS if privilege is revoked, EXIT_ERROR if it isn't
 */
int AK_revoke_privilege_user(char *username, char *table, char *right) {
    AK_PRO;
    int table_id = AK_get_table_obj_id(table);
    int user_id = AK_user_get_id(username);
    int result;

    if (table_id == EXIT_ERROR || user_id == EXIT_ERROR) {
        printf("Invalid table name or username!\n");
        AK_EPI;
        return EXIT_ERROR;
    }

    if (strcmp(right, "ALL") == 0) {
        struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
        AK_Init_L3(&row_root);
        int i = 0;

        struct list_node *row;

        while ((row = (struct list_node *) AK_get_row(i, "AK_user_right")) != NULL) {
            struct list_node *obj_id = AK_GetNth_L2(1, row);
            struct list_node *user_elem = AK_GetNth_L2(2, row);
            struct list_node *table_elem = AK_GetNth_L2(3, row);

            if (((int) *user_elem->data == user_id) && ((int) *table_elem->data == table_id)) {
                int id = (int) * obj_id->data;
                AK_Update_Existing_Element(TYPE_INT, &id, "AK_user_right", "obj_id", row_root);
                result = AK_delete_row(row_root);
            }
            i++;

            AK_DeleteAll_L3(&row_root);
            AK_free(row);
        }
        printf("Revoked all privileges for user '%s' under ID %d on table '%s'!\n", username, user_id, table);
    } else {
        struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
        AK_Init_L3(&row_root);
        int i = 0;
        struct list_node *row;

        while ((row = (struct list_node *) AK_get_row(i, "AK_user_right")) != NULL) {
            struct list_node *obj_id = AK_GetNth_L2(1, row);
            struct list_node *user_elem = AK_GetNth_L2(2, row);
            struct list_node *table_elem = AK_GetNth_L2(3, row);
            struct list_node *right_elem = AK_GetNth_L2(4, row);

            if (((int) * user_elem->data == user_id) && (table_id == (int) * table_elem->data) && strcmp(right_elem->data, right) == 0) {
                int id = (int) * obj_id->data;
                AK_Update_Existing_Element(TYPE_INT, &id, "AK_user_right", "obj_id", row_root);
                result = AK_delete_row(row_root);
            }
            i++;

            AK_DeleteAll_L3(&row_root);
            AK_free(row);
        }
        printf("Revoked privilege to %s data for user '%s' under ID %d on table '%s'!\n", right, username, user_id, table);
    }

    if (result == EXIT_ERROR) {
        AK_EPI;
        printf("User '%s' under ID %d doesn't have any privileges on table '%s'!\n", username, user_id, table);
        return EXIT_ERROR;
    }

    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Jurica Hlevnjak, updated by Marko Flajšek
 * @brief Function that revokes ALL user's privileges on ALL tables (for DROP user)
 * @param username name of user from whom we want to revoke all privileges
 * @return EXIT_SUCCESS if privilege is revoked, EXIT_ERROR if it isn't
 */
int AK_revoke_all_privileges_user(char *username) {
    AK_PRO;
    int user_id = AK_user_get_id(username);
    int result;

    if (user_id == EXIT_ERROR) {
        printf("Invalid username!\n");
        AK_EPI;
        return EXIT_ERROR;
    }

    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&row_root);
    int i = 0;

    struct list_node *row;

    while ((row = (struct list_node *) AK_get_row(i, "AK_user_right")) != NULL) {
        struct list_node *user = AK_GetNth_L2(2, row);
        if ((int) *user->data == user_id) {
            AK_Update_Existing_Element(TYPE_INT, &user_id, "AK_user_right", "user_id", row_root);
            result = AK_delete_row(row_root);
        }
        i++;

        AK_DeleteAll_L3(&row_root);
        AK_free(row);
    }

    printf("Revoked all privileges for user '%s' under ID %d!\n", username, user_id);

    if (result == EXIT_ERROR) {
        printf("User '%s' under ID %d doesn't have any privileges!\n", username, user_id);
        AK_EPI;
        return EXIT_ERROR;
    }

    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Kristina Takač.
 * @brief Function that grants a privilege to a given group on a given table
 * @param *groupname  name of group to which we want to grant privilege
 * @param *table name of table on which privilege will be granted to user
 * @param *right type of privilege which will be granted to user on given table
 * @return privilege_id or EXIT_ERROR if table or user aren't correct
 */
int AK_grant_privilege_group(char *groupname, char *table, char *right) {
    int privilege_id, table_id, group_id;
    AK_PRO;
    table_id = AK_get_table_obj_id(table);
    group_id = AK_group_get_id(groupname);

    if (table_id == EXIT_ERROR || group_id == EXIT_ERROR) {
        printf("Invalid table name or group name!\n");
        AK_EPI;
        return EXIT_ERROR;
    }

    if (strcmp(right, "ALL") == 0) {
        char rights[100];
        strcpy(&rights[0], "UPDATE\0");
        strcpy(&rights[10], "DELETE\0");
        strcpy(&rights[20], "INSERT\0");
        strcpy(&rights[30], "SELECT\0");
        int i;
        for (i = 0; i < 4; i++) {
            struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
            AK_Init_L3(&row_root);
            privilege_id = AK_get_id();

            AK_Insert_New_Element(TYPE_INT, &privilege_id, "AK_group_right", "obj_id", row_root);
            AK_Insert_New_Element(TYPE_INT, &group_id, "AK_group_right", "group_id", row_root);
            AK_Insert_New_Element(TYPE_INT, &table_id, "AK_group_right", "artifact_id", row_root);
            AK_Insert_New_Element(TYPE_VARCHAR, &rights[i * 10], "AK_group_right", "right_type", row_root);
            AK_insert_row(row_root);

            AK_free(row_root);
        }
        printf("\nGranted all privileges for group '%s' under ID %d on table '%s' under ID %d!\n\n", groupname, group_id, table, table_id);
    } else {
        struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
        AK_Init_L3(&row_root);

        privilege_id = AK_get_id();
        AK_Insert_New_Element(TYPE_INT, &privilege_id, "AK_group_right", "obj_id", row_root);
        AK_Insert_New_Element(TYPE_INT, &group_id, "AK_group_right", "group_id", row_root);
        AK_Insert_New_Element(TYPE_INT, &table_id, "AK_group_right", "artifact_id", row_root);
        AK_Insert_New_Element(TYPE_VARCHAR, right, "AK_group_right", "right_type", row_root);
        AK_insert_row(row_root);
        printf("\nGranted privilege to %s data for group '%s' under ID %d on table '%s' under ID %d!\n\n", right, groupname, group_id, table, table_id);

        AK_free(row_root);
    }

    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * NOTICE: Test 9 isn't currently revoking a privilege since the obj_id in the AK_group_right table is passing the value of 127.
     Once the issue #87 on GitHub concerning the data type is solved, the test should be working as expected.
 * @author Kristina Takač, updated by Mario Peroković - added comparing by table id
 * @brief Function that revokes a groups privilege on the given table 
 * @param *grounamep name of group which user belongs to
 * @param *table name of table on which privilege will be granted to group
 * @param *right type of privilege which will be granted to group on a given table
 * @return EXIT_SUCCESS if privilege is revoked, EXIT_ERROR if it isn't
 */
int AK_revoke_privilege_group(char *groupname, char *table, char *right) {
    int table_id, group_id;
    AK_PRO;
    group_id = AK_group_get_id(groupname);
    table_id = AK_get_table_obj_id(table);
    int result;

    if (table_id == EXIT_ERROR || group_id == EXIT_ERROR) {
        printf("Invalid table name or group name!\n");
        AK_EPI;
        return EXIT_ERROR;
    }

    if (strcmp(right, "ALL") == 0) {
        struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
        AK_Init_L3(&row_root);
        int i = 0;

        struct list_node *row;

        while ((row = (struct list_node *) AK_get_row(i, "AK_group_right")) != NULL) {
            struct list_node *obj_id = AK_GetNth_L2(1, row);
            struct list_node *group_elem = AK_GetNth_L2(2, row);
            struct list_node *table_elem = AK_GetNth_L2(3, row);

            if (((int) *group_elem->data == group_id) && ((int) *table_elem->data == table_id)) {
                int id = (int) * obj_id->data;
                AK_Update_Existing_Element(TYPE_INT, &id, "AK_group_right", "obj_id", row_root);
                result = AK_delete_row(row_root);
            }

            i++;

            AK_DeleteAll_L3(&row_root);
            AK_free(row);
        }
        printf("Revoked all privileges for group '%s' under ID %d on table '%s' under ID %d!\n", groupname, group_id, table, table_id);
    } else {
        struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
        AK_Init_L3(&row_root);
        int i = 0;

        struct list_node *row;

        while ((row = (struct list_node *) AK_get_row(i, "AK_group_right")) != NULL) {
            struct list_node *obj_id = AK_GetNth_L2(1, row);
            struct list_node *group_elem = AK_GetNth_L2(2, row);
            struct list_node *table_elem = AK_GetNth_L2(3, row);
            struct list_node *right_elem = AK_GetNth_L2(4, row);

            if (((int) * group_elem->data == group_id) && (table_id == (int) * table_elem->data) && strcmp(right_elem->data, right) == 0) {
                int id = (int) * obj_id->data;
                AK_Update_Existing_Element(TYPE_INT, &id, "AK_group_right", "obj_id", row_root);
                result = AK_delete_row(row_root);
            }

            i++;

            AK_DeleteAll_L3(&row_root);
            AK_free(row);
        }
        printf("Revoked privilege to %s data for group '%s' under ID %d on table '%s' under ID %d!\n", right, groupname, group_id, table, table_id);
    }

    if (result == EXIT_ERROR) {
        AK_EPI;
        printf("Group '%s' doesn't have any privileges!\n", groupname);
        return EXIT_ERROR;
    }

    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Jurica Hlevnjak
 * @brief Function that revokes ALL privileges from the desired group on ALL tables (needed for DROP group)
 * @param groupname name of group from which we want to revoke all privileges
 * @return EXIT_SUCCESS if privilege is revoked, EXIT_ERROR if it isn't
 */
int AK_revoke_all_privileges_group(char *groupname) {
    AK_PRO;
    int group_id = AK_group_get_id(groupname);
    int result;

    if (group_id == EXIT_ERROR) {
        printf("Invalid group name!\n");
        AK_EPI;
        return EXIT_ERROR;
    }

    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&row_root);
    int i = 0;

    struct list_node *row;

    while ((row = (struct list_node *) AK_get_row(i, "AK_group_right")) != NULL) {
        struct list_node *group = AK_GetNth_L2(2, row);
        if ((int) *group->data == group_id) {
            AK_Update_Existing_Element(TYPE_INT, &group_id, "AK_group_right", "group_id", row_root);
            printf("Revoked all privilege for group '%s' under ID %d!\n\n", groupname, group_id);
            result = AK_delete_row(row_root);
        }
        i++;

        AK_DeleteAll_L3(&row_root);
        AK_free(row);
    }

    if (result == EXIT_ERROR) {
        AK_EPI;
        printf("Group '%s' under ID %d doesn't have any privileges!\n", groupname, group_id);
        return EXIT_ERROR;
    }

    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Kristina Takač, updated by Mario Peroković, added verifying the existence of user in the group, updated by Maja Vračan 
 * @brief Function that puts the desired user in the given group
 * @param *user username of user which will be put in group
 * @param *group name of group in which user will be put
 * @return EXIT_SUCCESS or EXIT_ERROR if the user is already in the group
 */
int AK_add_user_to_group(char *user, char *group) {
    AK_PRO;
    int user_id = AK_user_get_id(user);
    int group_id = AK_group_get_id(group);
    int i = 0;
    struct list_node *row;

    if (group_id == EXIT_ERROR || user_id == EXIT_ERROR) {
        printf("Invalid group name or username!\n");
        AK_EPI;
        return EXIT_ERROR;
    }

    while ((row = (struct list_node *) AK_get_row(i, "AK_user_group")) != NULL) {

        // if user is already in group, return error
        if (user_id == (int) *row->next->data) {
            printf("User '%s' under ID %d is already a member of group '%s' under ID %d!\n", user, user_id, group, group_id);
            AK_EPI;
            return EXIT_ERROR;
        }
        i++;
    }

    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&row_root);

    AK_Insert_New_Element(TYPE_INT, &user_id, "AK_user_group", "user_id", row_root);
    AK_Insert_New_Element(TYPE_INT, &group_id, "AK_user_group", "group_id", row_root);
    AK_insert_row(row_root);
    printf("Added user '%s' under ID %d to group '%s' under ID %d!\n", user, user_id, group, group_id);

    AK_free(row_root);

    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Jurica Hlevnjak, update by Lidija Lastavec
 * @brief Function that removes user from all groups. Used for DROP user.
 * @param user name of user
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_remove_user_from_all_groups(char *user) {
    AK_PRO;

    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&row_root);

    int user_id = AK_user_get_id(user);
    int i = 0;
    int result;

    if (user_id == EXIT_ERROR) {
        printf("Invalid username!\n");
        AK_EPI;
        return EXIT_ERROR;
    }

    struct list_node *row;

    while ((row = (struct list_node *) AK_get_row(i, "AK_user_group")) != NULL) {
        struct list_node *user = AK_GetNth_L2(1, row);
        if (user_id == (int) *user->data) {
            AK_Update_Existing_Element(TYPE_INT, &user_id, "AK_user_group", "user_id", row_root);
            result = AK_delete_row(row_root);
            if (result == EXIT_ERROR) {
                printf("User '%s' under ID %d isn't a member of any group!\n", user, user_id);
                AK_EPI;
                return EXIT_ERROR;
            }
        }
        i++;

        AK_DeleteAll_L3(&row_root);
        AK_free(row);
    }

    printf("User '%s' under ID %d is removed from all groups!\n", user, user_id);
    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Jurica Hlevnjak, update by Lidija Lastavec
 * @brief Function that removes all users from a group. Used for DROP group.
 * @param group name of group
 * @return EXIT_SUCCESS or EXIT_ERROR
 */
int AK_remove_all_users_from_group(char *group) {
    AK_PRO;

    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&row_root);

    int group_id = AK_group_get_id(group);
    int i = 0;
    int result;

    if (group_id == EXIT_ERROR) {
        printf("Invalid group name!\n");
        AK_EPI;
        return EXIT_ERROR;
    }

    struct list_node *row;

    while ((row = (struct list_node *) AK_get_row(i, "AK_user_group")) != NULL) {
        struct list_node *group = AK_GetNth_L2(2, row);
        if (group_id == (int) *group->data) {
            AK_Update_Existing_Element(TYPE_INT, &group_id, "AK_user_group", "group_id", row_root);
            result = AK_delete_row(row_root);
        }
        i++;
        if (result == EXIT_ERROR) {
            printf("Group '%s' under ID %d doesn't contain any users!", group, group_id);
            AK_EPI;
            return EXIT_ERROR;
        }

        AK_DeleteAll_L3(&row_root);
        AK_free(row);
    }

    AK_free(row_root);
    printf("Users deleted from group '%s' under ID %d!\n", group, group_id);
    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Kristina Takač, updated by Marko Flajšek
 * @brief Function that checks whether the given user has a right for the given operation on the given table
 * @param *user username for which we want check privileges
 * @param *table name of table for which we want to check whether user has right on
 * @param *privilege privilege for which we want to check whether user has right for
 * @return EXIT_SUCCESS if user has right, EXIT_ERROR if user has no right
 */
int AK_check_privilege(char *username, char *table, char *privilege) {
    AK_PRO;
    int user_id = AK_user_get_id(username);
    int table_id = AK_get_table_obj_id(table);
    int i = 0;
    int number_of_groups = 0;
    int has_right = 0;
    int groups[100];

    if (table_id == EXIT_ERROR || user_id == EXIT_ERROR) {
        printf("Invalid table name or username!\n");
        AK_EPI;
        return EXIT_ERROR;
    }

    struct list_node *row;

    if (strcmp(privilege, "ALL") == 0) {

        int checking_privileges[4] = {0, 0, 0, 0};
        char found_privilege[10];
        while ((row = (struct list_node *) AK_get_row(i, "AK_user_right")) != NULL) {

            struct list_node *username_elem = AK_GetNth_L2(2, row);
            struct list_node *table_elem = AK_GetNth_L2(3, row);
            if (((int) * username_elem->data == user_id) && (table_id == (int) * table_elem->data)) {

                struct list_node *privilege_elem = AK_GetNth_L2(4, row);
                strcpy(found_privilege, privilege_elem->data);
                if (strcmp(found_privilege, "UPDATE") == 0)
                    checking_privileges[0] = 1;
                if (strcmp(found_privilege, "DELETE") == 0)
                    checking_privileges[1] = 1;
                if (strcmp(found_privilege, "INSERT") == 0)
                    checking_privileges[2] = 1;
                if (strcmp(found_privilege, "SELECT") == 0)
                    checking_privileges[3] = 1;
            }
            i++;
            AK_free(row);
        }
        for (i = 0; i < 4; i++) {
            if (checking_privileges[i] == 1) {
                has_right = 1;
            } else {
                has_right = 0;
                break;
            }
        }
        if (has_right == 1) {
            printf("User '%s' under ID %d has all privileges in the '%s' table under ID %d!", username, user_id, table, table_id);
            AK_EPI;
            return EXIT_SUCCESS;
        }

        i = 0;
        while ((row = (struct list_node *) AK_get_row(i, "AK_user_group")) != NULL) {
            while ((row = (AK_list *) AK_get_row(i, "AK_user_group")) != NULL) {
                struct list_node *user = AK_GetNth_L2(1, row);
                if (user_id == (int) * user->data) {
                    struct list_node *group = AK_GetNth_L2(2, row);
                    groups[number_of_groups] = (int) * group->data;
                    number_of_groups++;
                }
                i++;
                AK_free(row);
            }
            // set "flags" to 0
            checking_privileges[0] = 0;
            checking_privileges[1] = 0;
            checking_privileges[2] = 0;
            checking_privileges[3] = 0;
            for (i = 0; i < number_of_groups; i++) {
                int j = 0;
                while ((row = (AK_list *) AK_get_row(j, "AK_group_right")) != NULL) {
                    struct list_node *group_elem = AK_GetNth_L2(2, row);
                    struct list_node *table_elem = AK_GetNth_L2(3, row);
                    struct list_node *privilege_elem = AK_GetNth_L2(4, row);

                    if ((groups[i] == (int) * group_elem->data) && (table_id == (int) * table_elem->data)) {
                        strcpy(found_privilege, privilege_elem->data);
                        if (strcmp(found_privilege, "UPDATE") == 0)
                            checking_privileges[0] = 1;
                        if (strcmp(found_privilege, "DELETE") == 0)
                            checking_privileges[1] = 1;
                        if (strcmp(found_privilege, "INSERT") == 0)
                            checking_privileges[2] = 1;
                        if (strcmp(found_privilege, "SELECT") == 0)
                            checking_privileges[3] = 1;
                    }
                    j++;
                    AK_free(row);
                }
            }
            for (i = 0; i < 4; i++) {
                if (checking_privileges[i] == 1) {
                    has_right = 1;
                } else {
                    has_right = 0;
                    break;
                }
            }
            if (has_right == 1) {
                printf("User '%s' under ID %d has all privileges in the '%s' table under ID %d!", username, user_id, table, table_id);
                AK_EPI;
                return EXIT_SUCCESS;
            }
        }
    } else {        
        while ((row = (struct list_node *) AK_get_row(i, "AK_user_right")) != NULL) {           
            struct list_node *username_elem = AK_GetNth_L2(2, row);
            struct list_node *table_elem = AK_GetNth_L2(3, row);
            struct list_node *privilege_elem = AK_GetNth_L2(4, row);                

            if (((int) * username_elem->data == user_id) && (table_id == (int) * table_elem->data) && (strcmp(privilege_elem->data, privilege) == 0)) {

                has_right = 1;
                printf("User '%s' under ID %d has the right to %s data in the '%s' table under ID %d!", username, user_id, privilege, table, table_id);                
                AK_EPI;
                return EXIT_SUCCESS;
            }
            i++;
            AK_free(row);
        }
        i = 0;
        while ((row = (struct list_node *) AK_get_row(i, "AK_user_group")) != NULL) {
            struct list_node *user = AK_GetNth_L2(1, row);

            if (user_id == (int) * user->data) {
                struct list_node *group = AK_GetNth_L2(2, row);
                groups[number_of_groups] = (int) * group->data;
                number_of_groups++;
            }
            i++;
            AK_free(row);
        }
        for (i = 0; i < number_of_groups; i++) {
            int j = 0;

            while ((row = (struct list_node *) AK_get_row(j, "AK_group_right")) != NULL) {
                struct list_node *groups_elem = AK_GetNth_L2(2, row);
                struct list_node *table_elem = AK_GetNth_L2(3, row);
                struct list_node *privilege_elem = AK_GetNth_L2(4, row);
                if ((groups[i] == (int) * groups_elem->data) && (table_id == (int) * table_elem->data) && (strcmp(privilege_elem->data, privilege) == 0)) {
                    has_right = 1;
                    printf("User '%s' under ID %d has the right to %s data in the '%s' table under ID %d!", username, user_id, privilege, table, table_id);
                    AK_EPI;
                    return EXIT_SUCCESS;
                }
                j++;
                AK_free(row);
            }
        }
    }

    printf("User '%s' under ID %d has no right to %s data in the '%s' table under ID %d!", username, user_id, privilege, table, table_id);
    AK_EPI;
    return EXIT_ERROR;
}

/**
 * @author Jurica Hlevnjak, updated by Lidija Lastavec
 * @brief Function that checks if the user has any privileges or belongs to any group. Used in drop user for restriction.
 * @param user name of user
 * @return EXIT_ERROR or EXIT_SUCCESS
 */
int AK_check_user_privilege(char *user) {
    AK_PRO;
    int user_id = AK_user_get_id(user);
    int i = 0;

    if (user_id == EXIT_ERROR) {
        printf("Invalid username!\n");
        AK_EPI;
        return EXIT_ERROR;
    }

    struct list_node *row;
    int privilege = 0;

    while ((row = (struct list_node *) AK_get_row(i, "AK_user_right")) != NULL) {
        struct list_node *user_elem = AK_GetNth_L2(2, row);
        if ((int) *user_elem->data == user_id) {
            privilege = PRIVILEGE_GRANTED;
            printf("User '%s' under ID %d has some privileges!", user, user_id);
            AK_EPI;
            return EXIT_SUCCESS;
        }
        i++;
        AK_free(row);
    }

    int j = 0;
    while ((row = (struct list_node *) AK_get_row(j, "AK_user_group")) != NULL) {
        struct list_node *user_elem = AK_GetNth_L2(1, row);
        if ((int) *user_elem->data == user_id) {
            privilege = 1;
            printf("User '%s' under ID %d belongs to some group!", user, user_id);
            AK_EPI;
            return EXIT_SUCCESS;
        }
        j++;
        AK_free(row);
    }

    if (privilege == 0) {
        printf("User '%s' under ID %d hasn't got any privileges!", user, user_id);
        AK_EPI;
        return EXIT_ERROR;
    }

    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Jurica Hlevnjak, updated by Lidija Lastavec, updated by Marko Flajšek
 * @brief Function that checks if the group has any privileges. Used in drop group for restriction.
 * @param group name of group
 * @return EXIT_ERROR or EXIT_SUCCESS
 */
int AK_check_group_privilege(char *group) {
    AK_PRO;
    int group_id = AK_group_get_id(group);
    int i = 0;

    if (group_id == EXIT_ERROR) {
        printf("Invalid group name or username!\n");
        AK_EPI;
        return EXIT_ERROR;
    }

    struct list_node *row;
    int privilege = 0;

    while ((row = (struct list_node *) AK_get_row(i, "AK_group_right")) != NULL) {
        struct list_node *group_elem = AK_GetNth_L2(2, row);
        if ((int) *group_elem->data == group_id) {
            privilege = 1;
            printf("Group '%s' under ID %d has some privileges!", group, group_id);
            AK_EPI;
            return EXIT_SUCCESS;
        }
        i++;
        AK_free(row);
    }

    if (privilege == 0) {
        printf("Group '%s' under ID %d hasn't got any privileges!", group, group_id);
        AK_EPI;
        return EXIT_ERROR;
    }

    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Luka Balažinec
 * @brief List all users that have the specified privilege on any table.
 * @param  privilege A privilege string like "SELECT" or "INSERT".
 * @return EXIT_SUCCESS.
 */
int AK_list_users_with_privilege(char *privilege) {
    AK_PRO;

    printf("Users with privilege '%s':\n", privilege);

    int seen_cap = 16;
    int *seen_ids = malloc(seen_cap * sizeof(*seen_ids));
    if (!seen_ids) {
        AK_EPI;
        return EXIT_ERROR;
    }
    int seen_cnt = 0;

    struct list_node *row;
    int row_idx = 0;
    while ((row = (struct list_node *)AK_get_row(row_idx++, "AK_user_right")) != NULL) {
        struct list_node *user_elem  = AK_GetNth_L2(2, row);
        struct list_node *right_elem = AK_GetNth_L2(4, row);

        if (strcmp((char *)right_elem->data, privilege) == 0) {
            int user_id = *(int *)user_elem->data;

            int k;
            for (k = 0; k < seen_cnt; ++k) {
                if (seen_ids[k] == user_id) break;
            }
            if (k == seen_cnt) {
                if (seen_cnt == seen_cap) {
                    seen_cap *= 2;
                    int *tmp = realloc(seen_ids, seen_cap * sizeof(*tmp));
                    if (!tmp) break;
                    seen_ids = tmp;
                }
                seen_ids[seen_cnt++] = user_id;

                struct list_node *urow;
                int uidx = 0;
                while ((urow = (struct list_node *)AK_get_row(uidx++, "AK_user")) != NULL) {
                    int curr_id = *(int *)AK_GetNth_L2(1, urow)->data;
                    if (curr_id == user_id) {
                        char *username = (char *)AK_GetNth_L2(2, urow)->data;
                        printf("  - %s\n", username);
                        AK_DeleteAll_L3(&urow);
                        AK_free(urow);
                        break;
                    }
                    AK_DeleteAll_L3(&urow);
                    AK_free(urow);
                }
            }
        }

        AK_DeleteAll_L3(&row);
        AK_free(row);
    }

    if (seen_cnt == 0) {
        printf("  (none)\n");
    }

    free(seen_ids);
    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Kristina Takač, updated by Tomislav Ilisevic, updated by Lidija Lastavec, updated by Marko Flajšek
 * @brief Function that tests all the previous functions
 * @return no return value                                                     
 */
TestResult AK_privileges_test() {
    AK_PRO;

    int successful[19] = {0};

    printf("\n\n");
    printf("||====================================================================|| \n");
    printf("||////////////////////////////////////////////////////////////////////|| \n");
    printf("||========================| privileges.c Tests |======================|| \n");
    printf("||////////////////////////////////////////////////////////////////////|| \n");
    printf("||====================================================================|| \n");


    /******************/
    /* 1. AK_user_add */
    /******************/

    printf("\n1. Test - AK_user_add function - Inserts a new user in the AK_user table\n");
    printf("\nTest data: user1 1111; user2 2222; user3 3333; user4 4444;\n\n");
    printf("Result:\n\n");

    if (AK_user_add("user1", "?Baze2020", NEW_ID) == "taken") {
        printf("Test 1. - Fail!\n\n");
    } else {
        printf("Test 1. - Pass!\n\n");
        successful[0] = 1;
    }

    printf("Adding user2 with password '!Sifra2025'...\n");
    AK_user_add("user2", "!Sifra2025", NEW_ID);
    printf("\n");

    printf("Adding user3 with password '#Lozinka2024'...\n");
    AK_user_add("user3", "#Lozinka2024", NEW_ID);
    printf("\n");

    printf("Adding user4 with password '$Pass2023'...\n\n");
    AK_user_add("user4", "$Pass2023", NEW_ID);

    printf("Adding user10 with password 'Aa1!'...\n");
    AK_user_add("user10", "Aa1!", NEW_ID);
    printf("\n");

    printf("Adding user11 with password 'lowercase1]'...\n");
    AK_user_add("user11", "lowercase1]", NEW_ID);
    printf("\n");

    printf("Adding user12 with password 'NoDigits!'...\n");
    AK_user_add("user12", "NoDigits!", NEW_ID);
    printf("\n");

    printf("Adding user13 with password '4G?racecar?G4'...\n");
    AK_user_add("user13", "4G?racecar?G4", NEW_ID);
    printf("\n");

    printf("Adding user14 with password '2023user14X['...\n");
    AK_user_add("user14", "2023user14X[", NEW_ID);
    printf("\n");

    printf("Adding user15 with password 'Abc123456!'...\n");
    AK_user_add("user15", "Abc123456!", NEW_ID);
    printf("\n");

    printf("Adding user16 with password 'Aaaaaa0011@'...\n");
    AK_user_add("user16", "Aaaaaa0011@", NEW_ID);
    printf("\n");

    AK_print_table("AK_user");

    printf("\n\n||====================================================================|| \n");


    /*********************/
    /* 2. AK_user_rename */
    /*********************/

    printf("\n\n2. Test - AK_user_rename function - Renames a user in the AK_user table\n");
    printf("\nTest data: rename user4 to user5\n\n");
    printf("Result:\n\n");

    if (AK_user_rename("user4", "user5", "4444") == EXIT_ERROR) {
        printf("\nTest 2. - Fail!\n");
    } else {
        printf("\nTest 2. - Pass!\n");
        successful[1] = 1;
    }

    printf("\n");
    AK_print_table("AK_user");

    printf("\n\n||====================================================================|| \n");


    /*****************************/
    /* 3. AK_user_remove_by_name */
    /*****************************/

    printf("\n\n3. Test - AK_user_remove_by_name function - Removes a user by name from the AK_user table\n");
    printf("\nTest data: remove user5\n\n");
    printf("Result:\n");

    if (AK_user_remove_by_name("user5") == EXIT_ERROR) {
        printf("\nTest 3. - Fail!\n");
    } else {
        printf("\nTest 3. - Pass!\n");
        successful[2] = 1;
    }

    printf("\n");
    AK_print_table("AK_user");

    printf("\n\n||====================================================================|| \n");


    /*******************/
    /* 4. AK_group_add */
    /*******************/

    printf("\n\n4. Test - AK_group_add function - Adds a new group in the AK_group table\n");
    printf("\nTest data: group1, group2, group3, group4\n\n");
    printf("Result:\n\n");

    if (AK_group_add("group1", NEW_ID) == EXIT_ERROR) {
        printf("\nTest 4. - Fail!\n\n");
    } else {
        printf("\nTest 4. - Pass!\n\n");
        successful[3] = 1;
    }

    //adding 3 more groups for future tests
    AK_group_add("group2", NEW_ID);
    AK_group_add("group3", NEW_ID);
    AK_group_add("group4", NEW_ID);

    printf("\n");
    AK_print_table("AK_group");

    printf("\n\n||====================================================================|| \n");


    /**********************/
    /* 5. AK_group_rename */
    /**********************/

    printf("\n\n5. Test - AK_group_rename function - Renames a group\n");
    printf("\nTest data: rename group4 to group5\n\n");
    printf("Result:\n\n");

    if (AK_group_rename("group4", "group5") == EXIT_ERROR) {
        printf("\nTest 5. - Fail!\n\n");
    } else {
        printf("\nTest 5. - Pass!\n\n");
        successful[4] = 1;
    }

    AK_print_table("AK_group");

    printf("\n\n||====================================================================|| \n");


    /******************************/
    /* 6. AK_group_remove_by_name */
    /******************************/

    printf("\n\n6. Test - AK_group_remove_by_name function - Remove a group by name from the AK_group table\n");
    printf("\nTest data: group5\n");

    if (AK_group_remove_by_name("group5") == EXIT_ERROR) {
        printf("\nTest 6. - Fail!\n\n");
    } else {
        printf("\nTest 6. - Pass!\n\n");
        successful[5] = 1;
    }

    AK_print_table("AK_group");

    printf("\n\n||====================================================================|| \n");


    /***************************/
    /* 7. AK_add_user_to_group */
    /***************************/

    printf("\n\n7. Test - AK_add_user_to_group function - Puts a user into the given group\n");
    printf("\nTest data: add user1 to group1; user2 to group1; user2 to group2, user2 to group3\n\n");
    printf("Result:\n\n");

    if (AK_add_user_to_group("user1", "group1") == EXIT_ERROR) {
        printf("\nTest 7. - Fail!\n\n");
    } else {
        printf("\nTest 7. - Pass!\n\n");
        successful[6] = 1;
    }

    //adding 1 more user to a group for future tests
    AK_add_user_to_group("user2", "group1");

    printf("\n");
    AK_print_table("AK_user_group");

    printf("\n\n||====================================================================|| \n");


    /****************************************/
    /* 8. AK_grant_privilege_group function */
    /****************************************/

    printf("\n\n8. Test - AK_grant_privilege_group function - Grants privilege to a given group on a given table\n");
    printf("\nTest data: grant all privileges to group1 on professor table\n\n");
    printf("Result:\n\n");

    if (AK_grant_privilege_group("group1", "professor", "ALL") == EXIT_ERROR) {
        printf("Test 8. - Fail!\n\n");
    } else {
        printf("Test 8. - Pass!\n\n");
        successful[7] = 1;
    }
    
    //running 4 more functions for future tests
    AK_grant_privilege_group("group1", "student", "ALL");
    AK_grant_privilege_group("group2", "student", "ALL");
    AK_grant_privilege_group("group2", "professor", "UPDATE");
    AK_grant_privilege_group("group2", "professor2", "DELETE");

    printf("\n");
    AK_print_table("AK_group_right");

    printf("\n\n||====================================================================|| \n");


    /********************************/
    /* 9. AK_revoke_privilege_group */
    /********************************/

    /* NOTICE: This test isn't currently revoking a privilege since the obj_id in the AK_group_right table is passing the value of 127.
     Once the issue #87 on GitHub concerning the data type is solved, the test should be working as expected. */
    
    printf("\n\n9. Test - AK_revoke_privilege_group function - Revokes privilege for a group on a given table\n");
    printf("\nTest data: revoke UPDATE privilege for group1 on the professor table\n\n");
    printf("Result:\n\n");

    if (AK_revoke_privilege_group("group1", "professor", "UPDATE") == EXIT_ERROR) {
        printf("\nTest 9. - Fail!\n\n");
    } else {
        printf("\nTest 9. - Pass!\n\n");
        successful[8] = 1;
    }

    AK_print_table("AK_group_right");

    printf("\n\n||====================================================================|| \n");


    /**************************************/
    /* 10. AK_revoke_all_privileges_group */
    /**************************************/


    printf("\n\n10. Test - AK_revoke_all_privileges_group function - Revokes ALL privileges for a group on ALL tables\n");
    printf("\nTest data: revoke all privileges for group1 on all tables\n\n");
    printf("Result:\n\n");

    if (AK_revoke_all_privileges_group("group1") == EXIT_ERROR) {
        printf("Test 10. - Fail!\n\n");
    } else {
        printf("Test 10. - Pass!\n\n");
        successful[9] = 1;
    }

    AK_print_table("AK_group_right");

    printf("\n\n||====================================================================|| \n");


    /*******************************/
    /* 11. AK_grant_privilege_user */
    /*******************************/

    printf("\n\n11. Test - AK_grant_privilege_user function - Grants privilege to a given user on a given table\n");
    printf("\nTest data: grant privileges for users user1, user2 and user3 on tables student and professor\n\n");
    printf("Result:\n\n");

    if (AK_grant_privilege_user("user1", "student", "UPDATE") == EXIT_ERROR) {
        printf("Test 11. - Fail!\n\n");
    } else {
        printf("Test 11. - Pass!\n\n");
        successful[10] = 1;
    }

    //adding 7 more privileges for future tests    
    AK_grant_privilege_user("user1", "student", "DELETE");
    AK_grant_privilege_user("user1", "student", "SELECT");
    AK_grant_privilege_user("user1", "professor", "ALL");
    AK_grant_privilege_user("user2", "student", "UPDATE");
    AK_grant_privilege_user("user2", "professor", "SELECT");
    AK_grant_privilege_user("user3", "professor", "SELECT");
    AK_grant_privilege_user("user3", "student", "SELECT");

    printf("\n");
    AK_print_table("AK_user_right");

    printf("\n\n||====================================================================|| \n");


    /********************************/
    /* 12. AK_revoke_privilege_user */
    /********************************/
    
    /* NOTICE: This test isn't currently revoking a privilege since the obj_id in the AK_user_right is passing the value of 127.
     Once the issue #87 on GitHub concerning the data type is solved, the test should be working as expected. */

    printf("\n\n12. Test - AK_revoke_privilege_user function - Revokes user's privilege(s) on a given table\n");
    printf("\nTest data: revoke DELETE privilege for user1 on the 'student' table\n\n");
    printf("Result:\n\n");

    if (AK_revoke_privilege_user("user1", "student", "DELETE") == EXIT_ERROR) {
        printf("\nTest 12. - Fail!\n\n");
    } else {
        printf("\nTest 12. - Pass!\n\n");
        successful[11] = 1;
    }

    AK_print_table("AK_user_right");

    printf("\n\n||====================================================================|| \n");


    /*************************************/
    /* 13. AK_revoke_all_privileges_user */
    /*************************************/

    printf("\n\n13. Test - AK_revoke_all_privileges_user function - Revokes ALL user's privileges on ALL tables\n");
    printf("\nTest data: revoke user3's privileges on all tables\n\n");
    printf("Result:\n\n");

    if (AK_revoke_all_privileges_user("user3") == EXIT_ERROR) {
        printf("\nTest 13. - Fail!\n\n");
    } else {
        printf("\nTest 13. - Pass!\n\n");
        successful[12] = 1;
    }

    AK_print_table("AK_user_right");

    printf("\n\n||====================================================================|| \n");


    /**************************/
    /* 14. AK_check_privilege */
    /**************************/

    printf("\n\n14. Test - AK_check_privilege function - Checks whether the given user has the right for the given operation on a desired table\n");
    printf("\nTest data: check if user user2 has the right to UPDATE data in the STUDENT table\n\n");
    printf("Result:\n\n");

    if (AK_check_privilege("user2", "student", "UPDATE") == EXIT_ERROR) {
        printf("\n\nTest 14. - Fail!\n\n");
    } else {
        printf("\n\nTest 14. - Pass!\n\n");
        successful[13] = 1;
    }

    //test for user without a right
    printf("\nTest data: check if user user1 has right to INSERT data into the PROFESSOR2 table\n\n");
    printf("Result:\n\n");

    if (AK_check_privilege("user1", "professor2", "INSERT") == EXIT_ERROR) {
        printf("\n\nTest 14. - Pass!\n");
    } else {
        printf("\n\nTest 14. - Fail!\n");
    }

    printf("\n");
    AK_print_table("AK_user_right");

    printf("\n\n||====================================================================|| \n");


    /********************************/
    /* 15. AK_check_group_privilege */
    /********************************/

    printf("\n\n15. Test - AK_check_group_privilege function - Checks whether the given group has any privileges\n");
    printf("\nTest data: check if group 2 has privileges\n\n");
    printf("Result:\n\n");

    if (AK_check_group_privilege("group2") == EXIT_SUCCESS) {
        printf("\n\nTest 15. - Pass!\n\n");
        successful[14] = 1;
    } else {
        printf("\n\nTest 15. - Fail!\n\n");
    }

    //test for a group without privilege
    printf("\nTest data: group3 - without privileges\n\n");
    printf("Result:\n\n");

    if (AK_check_group_privilege("group3") == EXIT_ERROR) {
        printf("\n\nTest 15. - Pass!\n");
    } else {
        printf("\n\nTest 15. - Fail!\n");
    }

    printf("\n");
    AK_print_table("AK_group_right");

    printf("\n\n||====================================================================|| \n");


    /*******************************/
    /* 16. AK_check_user_privilege */
    /*******************************/

    printf("\n\n16. Test - AK_check_user_privilege function - Checks if the given user has any privileges or belongs to any group\n");
    printf("\nTest data: check if user user1 has ANY privileges\n\n");
    printf("Result:\n\n");

    if (AK_check_user_privilege("user1") == EXIT_ERROR) {
        printf("\n\nTest 16. - Fail!\n\n");
    } else {
        printf("\n\nTest 16. - Pass!\n\n");
        successful[15] = 1;
    }

    //test for a user without any privileges
    printf("\nTest data: check for user without privileges (user3)\n\n");
    printf("Result:\n\n");

    if (AK_check_user_privilege("user3") == EXIT_ERROR) {
        printf("\n\nTest 16. - Pass!\n");
    } else {
        printf("\n\nTest 16. - Fail!\n");
    }

    printf("\n");
    AK_print_table("AK_user_right");
    printf("\n\n");
    AK_print_table("AK_group_right");

    printf("\n\n||====================================================================|| \n");


    /**************************************/
    /* 17. AK_remove_user_from_all_groups */
    /**************************************/

    printf("\n\n17. Test - AK_remove_user_from_all_groups function - Removes the given user from all groups\n");
    printf("\nTest data: remove user1 (member of group1)\n\n");

    AK_print_table("AK_user_group");

    printf("\nResult:\n\n");

    if (AK_remove_user_from_all_groups("user1") == EXIT_SUCCESS) {
        printf("\n\nTest 17. - Pass!\n");
        successful[16] = 1;
    } else {
        printf("\n\nTest 17. - Fail!\n");
    }

    printf("\n");
    AK_print_table("AK_user_group");

    printf("\n\n||====================================================================|| \n");


    /**************************************/
    /* 18. AK_remove_all_users_from_group */
    /**************************************/

    printf("\n\n18. Test - AK_remove_all_users_from_group function - Removes all users from the given group\n");
    printf("\nTest data: remove users from group1 (with added users)\n\n");

    AK_print_table("AK_user_group");

    printf("\nResult:\n\n");

    if (AK_remove_all_users_from_group("group1") == EXIT_SUCCESS) {
        printf("\n\nTest 18. - Pass!\n");
        successful[17] = 1;
    } else {
        printf("\n\nTest 18. - Fail!\n");
    }

    printf("\n");
    AK_print_table("AK_user_group");

    printf("\n\n||====================================================================|| \n");


    /**************************************/
    /* 19. AK_user_check_pass */
    /**************************************/

    printf("\n19. Test - AK_user_check_pass function - Checks if given username and matching password exists in database\n");
    printf("Result:\n\n");


    if (AK_user_check_pass("user2", "!Sifra2025") == 0) {
        printf("\n\nTest 19. - Fail!\n");
    } else {
        printf("\n\nTest 19. - Pass!\n");
        successful[18] = 1;
    }

    printf("\n");
    AK_print_table("AK_user");

    printf("\n\n||====================================================================|| \n");


    /**************************************/
    /* 20. AK_user_change_password        */
    /**************************************/
    printf("\n20. Test - AK_user_change_password function - Changes the users password\n");
    printf("Original AK_user table:\n\n");
    AK_print_table("AK_user");

    printf("\nChanging password for 'user3' from '#Lozinka2024' to '#Lozinka2026'...\n");
    if (AK_user_change_password("user3", "#Lozinka2026") != EXIT_SUCCESS) {
        printf("\n\nTest 20. - Fail!\n");
    } else {
        printf("\nPassword changed successfully.\n\n");
        printf("Updated AK_user table:\n\n");
        AK_print_table("AK_user");
        printf("\n\nTest 20. - Pass!\n");
        successful[19] = 1;
    }
    
    printf("\n\n||====================================================================|| \n");


    /**************************************/
    /* 21. AK_list_users_with_privilege   */
    /**************************************/
    printf("\n21. Test - AK_list_users_with_privilege for SELECT, INSERT, UPDATE and DELETE privileges\n\n");

    const char *privileges[] = { "SELECT", "INSERT", "UPDATE", "DELETE" };
    for (int i = 0; i < 4; ++i) {
        const char *p = privileges[i];
        AK_list_users_with_privilege((char *)p);
        printf("\n");
    }
    printf("Test 21. - Pass!\n\n");
    successful[20] = 1;

    printf("||====================================================================||\n");

    /* END SUMMARY*/

    printf("\nSummary: \n");
    int num = 0;
    int numFail = 0;
    for (num = 0; num < 21; num++) {
        printf("%i. Test: %s \n", (num + 1), (successful[num] == 1 ? "Pass" : "Fail"));
        if (successful[num] == 0) numFail++;
    }

    if (numFail == 0) printf("\nALL TESTS PASSED!\n");
    else printf("\nNumber of failed tests: %d\n", numFail);

    AK_EPI;

    return TEST_result(num-numFail,numFail);
}