/**
@file command.c TODO: Description
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
#include "command.h"
/** 
 * @author Mario Kolmacic updated by Ivan Pusic and Tomislav Ilisevic
 * @brief Function for executing given commands (SELECT, UPDATE, DELETE AND INSERT)
 * @param commands Commands array to execute
 * @param commandNum Number of commands in array
 * @return ERROR_EXIT only if command can't be executed returns EXIT_ERROR
 */
int AK_command(command * commands, int commandNum) {
    int i;
    AK_PRO;
    for (i = 0; i < commandNum; ++i) {
        switch(commands[i].id_command){
        case SELECT:
            printf("***SELECT***\n");
            char *ext = "_selection_tmp_table";
            char *dest_table = AK_malloc(strlen(ext) + strlen(commands[i].tblName)+1);
            strcpy(dest_table, commands[i].tblName);
            strcat(dest_table, ext);
            if(AK_selection(commands[i].tblName, dest_table, (struct list_node*)commands[i].parameters) == EXIT_ERROR){ // print temp table inside the function
        AK_EPI;
                return EXIT_ERROR;
        }

            break;
            
        case UPDATE:
            printf("***UPDATE***\n");
            if(AK_update_row( (struct list_node *) (commands[i].parameters)) == EXIT_ERROR){
                AK_EPI;
                return EXIT_ERROR;
            }
            AK_print_table(commands[i].tblName);

            break;
        case DELETE:
            printf("***DELETE***\n");            
            if(AK_delete_row( (struct list_node *) (commands[i].parameters)) == EXIT_ERROR){
        AK_EPI;
                return EXIT_ERROR;
        }
            AK_print_table(commands[i].tblName);
            break;
           
        case INSERT:
            printf("***INSERT***\n");
            if(AK_insert_row( (struct list_node *) (commands[i].parameters)) == EXIT_ERROR){
        AK_EPI;
                return EXIT_ERROR;
        }
            AK_print_table(commands[i].tblName);
            break;
        default:
            break;
        }
    }
    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Unknown, updated by Tomislav Ilisevic
 * @brief  Function for testing commands
 *
 */
TestResult AK_test_command(){
    AK_PRO;
    printf("***Test Command***\n");
    int commandNum = 4;
    command *commands = AK_malloc(sizeof(command)*(commandNum+1));


    // test for insert

    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&row_root);
    
    char *tblName = "student";
   
    int studentId, year;
    float weight;
    studentId = 35917;
    year = 2012;
    weight = 82.00;
    AK_DeleteAll_L3(&row_root);
    AK_Insert_New_Element(TYPE_INT, &studentId, tblName, "mbr", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "Mario", tblName, "firstname", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "Kolmacic", tblName, "lastname", row_root);
    AK_Insert_New_Element(TYPE_INT, &year, tblName, "year", row_root);
    AK_Insert_New_Element(TYPE_FLOAT, &weight, tblName, "weight", row_root);

    commands[0].id_command = INSERT;
    commands[0].tblName = "student";
    commands[0].parameters = row_root;


    // test for update

    studentId = 35900;
    
    row_root = (struct list_node *) AK_malloc(sizeof (struct list_node ));
    AK_Init_L3(&row_root);
    AK_DeleteAll_L3(&row_root);

    AK_Update_Existing_Element(TYPE_INT, &studentId, tblName, "mbr", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "FOI", tblName, "firstname", row_root);

    commands[1].id_command = UPDATE;
    commands[1].tblName = "student";
    commands[1].parameters = row_root;


    // test for delete

    int id_prof;
    id_prof = 35893;
    
    row_root = (struct list_node *) AK_malloc(sizeof (struct list_node ));
    AK_Init_L3(&row_root);
    AK_DeleteAll_L3(&row_root);

    AK_Update_Existing_Element(TYPE_INT, &id_prof, tblName, "id_prof", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "FOI", tblName, "firstname", row_root);

    commands[2].id_command = DELETE;
    commands[2].tblName = "professor";
    commands[2].parameters = row_root;


    // Test for select

    row_root = (struct list_node *) AK_malloc(sizeof (struct list_node ));
    AK_Init_L3(&row_root);
    AK_DeleteAll_L3(&row_root);

    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof ("firstname"), row_root);
    AK_InsertAtEnd_L3(TYPE_VARCHAR, "Robert", sizeof ("Robert"), row_root);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "=", sizeof ("="), row_root);

    commands[3].id_command = SELECT;
    commands[3].tblName = "student";
    commands[3].parameters = row_root;


    // execute commands
    int test_command = AK_command(commands, commandNum);
    AK_DeleteAll_L3(&row_root);
    int ok=0, fail=0;
	if (test_command == EXIT_SUCCESS){
		printf("\n\nTest succeeded!\n");
		ok++;
    }
    else{
		printf("\n\nTest failed!\n");
		fail++;
    }
	
    AK_EPI;
    return TEST_result(ok,fail);
}
