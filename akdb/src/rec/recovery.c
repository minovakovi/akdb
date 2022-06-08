/**
 @file recovery.c Provides recovery functions.
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

#include "recovery.h"


/** 
 * Function opens the recovery binary file and executes all commands that were
 * saved inside the redo_log structure
 * @author Dražen Bandić, update by Tomislav Turek
 * @brief Function that reads the binary file in which last commands were saved, and executes them
 * @param fileName - name of the archive log
 * @return no value
 */
 void AK_recover_archive_log(char* fileName){
    FILE * fp;
    AK_PRO;
    
    printf("AK_recover_archive_log: reading file contents to redo log\n");
    // open recovery file
    fp = fopen(fileName, "rb");

    // cannot open file
    if (fp == NULL){
        perror("fopen");
        AK_EPI;
        exit(EXIT_FAILURE);
    }
    
    // read structure from file
    AK_redo_log log_file;
    log_file.number = 0;
    int i = 0;
    fread(&log_file, sizeof(log_file), 1, fp);

    // read command by command
    for(i = 0; i < log_file.number; i++) {
        memcpy(redo_log->command_recovery, log_file.command_recovery, sizeof(log_file.command_recovery));
    }

    redo_log->number = log_file.number;

    printf("AK_recover_archive_log: Checking for unfinished archived data commands...\nNumber of archived commands: %d\n", redo_log->number);

    for(i = 0; i < redo_log->number; i++) {
        if(redo_log->command_recovery[i].finished != 1) {
            AK_recovery_insert_row(redo_log->command_recovery[i].table_name, i);
	}
    }
    
    //AK_empty_archive_log(); //- no need when archive log has dynamic name
    
    // close
    fclose(fp);
    AK_EPI;
}

/** 
 * Function is given the table name with desired data that should be
 * inserted inside. By using the table name, function retrieves table 
 * attributes names and their types which uses afterwards for insert_data_test
 * function to insert data to designated table.
 * 
 * @author Dražen Bandić, updated by Tomislav Turek
 * @brief Function that inserts a new row in the table with attributes
 * @param table - table name to insert to
 * @param commandNumber - number of current command
 * @return no value
 */
void AK_recovery_insert_row(char* table, int commandNumber){
    AK_PRO;
    
    printf("AK_recovery: found unfinished archived data commands for %s, executing...\n", table);
    int i;

    // retrieve table attributes names

    char* table_attr_names = AK_rel_eq_get_atrributes_char(table);
    char** attr_name = AK_recovery_tokenize(table_attr_names, ";", 0);
    
    // retrieve table attribute types

    char* table_attr_types = AK_get_table_atribute_types(table);
    char** attr_types = AK_recovery_tokenize(table_attr_types, ";", 0);

    // convert all attribute types to integers
    int type[MAX_ATTRIBUTES];

    for(i = 0; i < MAX_ATTRIBUTES; i++){
        if(attr_types[i] == NULL){
            break;
        }
        type[i] = atoi(attr_types[i]);
    }

    char** attributes = AK_malloc(MAX_ATTRIBUTES*sizeof(*attributes));
    int n = i;
    
    // insert data to table

    for(i=0;i<n;i++){
	attributes[i]=redo_log->command_recovery[commandNumber].arguments[i];
    }

    int result = recovery_insert_row(table, attr_name, attributes, n, type);
    
    //mark command as finished
    if(result == EXIT_SUCCESS)
    {
        redo_log->command_recovery[commandNumber].finished=1;
        printf("Uspješni recovery insert");
    }
    else printf("Neupješni recovery insert");

	
    AK_EPI;
}

/**
 * @author Danko Bukovac
 * @brief Function that inserts row in table
 * @return EXIT_SUCCESS if insert is successful, else EXIT_FAILURE
 */
int recovery_insert_row(char* table, char** attr_name, char** attributes, int n, int* type) {
    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row_root);

    AK_DeleteAll_L3(&row_root);

    for (int i = 0; i < n; i++) {
        if(type[i] == TYPE_INT){
            int attri = atoi(attributes[i]);
            AK_Insert_New_Element(type[i], &attri, table, attr_name[i], row_root);
        }
        if(type[i] == TYPE_FLOAT){
            float attrf = atof(attributes[i]);
            AK_Insert_New_Element(type[i], &attrf, table, attr_name[i], row_root);
        }
        else
            AK_Insert_New_Element(type[i], attributes[i], table, attr_name[i], row_root);
    }
    int result = AK_insert_row(row_root);
    return result;
}

/** 
 * @author Dražen Bandić
 * @brief Function that tokenizes the input with the given delimiter and puts them in an double pointer structure (so we can execute an insert)
 * @param input - input to tokenize
 * @param delimiter - delimiter
 * @param valuesOrNot - 1 if the input are values, 0 otherwise
 * @return new double pointer structure with tokens
 */
char** AK_recovery_tokenize(char* input, char* delimiter, int valuesOrNot){
    AK_PRO;
    char* str = strdup(input);
    int count = 0;
    char** result = AK_malloc(MAX_ATTRIBUTES*sizeof(*result));

    char* tok=strtok(str, delimiter); 

    while(1){
        result[count++] = tok? strdup(tok) : tok;
        if (!tok) break;

        tok = strtok(NULL, delimiter);
    } 
    AK_free(str);
    
    int i = 0;
    int j = 0;

    char** values = AK_malloc(MAX_ATTRIBUTES*sizeof(*result));

    if(valuesOrNot == 1){
        for(i = count - 2, j = 0; i >= 0 && j < count - 1; i--, j++){
            values[j] = result[i];
        }
        AK_free(result);
        AK_EPI;
        return values;
    } else{
        AK_EPI;
        return result;
    }
}

/**
 * this variable flags if system failed
 */
short grandfailure = 0;

/**
 * Function is called when SIGINT signal is sent to the system.
 * All commands that are written to rec.bin file are recovered to
 * the designated structure and then executed.
 * 
 * @author Tomislav Turek
 * @brief Function that recovers and executes failed commands
 * @param sig required integer parameter for SIGINT handler functions
 */
void AK_recover_operation(int sig) {
    FILE *fp;
    char filename[30];
    AK_PRO;
    // set flag that system failed
    grandfailure = 1;
    // acknowledge that the system has failed
    printf("\nUnexpected system failure, trying to recover...\n");

    // retrieve last archive log filename
    char *latest = malloc(strlen(ARCHIVELOG_PATH)+strlen("/latest.txt")+1);
    strcpy(latest, ARCHIVELOG_PATH);
    strcat(latest, "/latest.txt");

    fp = fopen(latest, "r");
    fscanf(fp, "%s", filename);
    fclose(fp);
    AK_free(latest);

    char *destination = malloc(strlen(ARCHIVELOG_PATH)+strlen(filename)+2);
    strcpy(destination, ARCHIVELOG_PATH);
    strcat(destination, "/");
    strcat(destination, filename);

    // recover from failure
    AK_recover_archive_log(destination);
    AK_EPI;
}

/**
 * Function does nothing while waiting a SIGINT signal 
 * (signal represents // doxygen @ for full description ???
 * system failure). 
 * Upon retrieving the signal it calls function
 * AK_recover_operation which starts the recovery by building commands.
 * To comply with the designated structure AK_command_recovery_struct // {link} to struct ???
 * it writes dummy commands to the file log.log
 * @brief Function for recovery testing.
 * @author Tomislav Turek
 */
TestResult AK_recovery_test() {
    AK_PRO;
    printf("Initiating recovery test\n");
    
    // allocate the needed space
    AK_command_recovery_struct *command = AK_malloc(sizeof(AK_command_recovery_struct));

    // build first command
    command->operation = INSERT;
    sprintf(command->table_name, "student");
    sprintf(command->arguments[0], "35898");
    sprintf(command->arguments[1], "Matija");
    sprintf(command->arguments[2], "Novak");
    sprintf(command->arguments[3], "2000");
    sprintf(command->arguments[4], "180");
    command->finished = 0;
    
    // save first command to redo_log
    redo_log->command_recovery[0] = *command;
    redo_log->number++;
    
    // build second command
    sprintf(command->arguments[0], "36996");
    sprintf(command->arguments[1], "Pero");
    sprintf(command->arguments[2], "Peric");
    sprintf(command->arguments[3], "2004");
    sprintf(command->arguments[4], "88");
    command->finished = 0;
    
    // save second command to redo_log
    redo_log->command_recovery[1] = *command;
    redo_log->number++;

    // write commands to file
    AK_archive_log(-10);
    // print instructions
    printf("Working... use Ctrl+C to destabilize the system\n");
    // register handler function
    sigset(SIGINT, AK_recover_operation); 
    
    // do nothing
    while(!grandfailure);                 
    AK_EPI;

    return TEST_result(1,0);
}

/** 
 * Function lists the contents of the archive_log directory.
 * The user then types in the name of the desired bin file to open and perform the neccessary actions.
 * @author Matija Večenaj
 * @brief Executes the recovery operation for the chosen bin file
 * @param none
 * @return no value
 */
void AK_load_chosen_log () {
	AK_PRO;
	
	//pointer for the dir
	struct dirent *de;
	char name[35];
	char path[] = "akdb/akdb/bin/archivelog/";
	
	//opendir will return a pointer of DIR type
	DIR *dr = opendir("akdb/akdb/bin/archivelog/");
	
	//will return NULL if there's an error
	if(dr == NULL){
		printf("Cannot open directory\n");
		AK_EPI;
		exit(EXIT_FAILURE);
	}
	
	//reading the dir contents
	while((de=readdir(dr)) != NULL){
		
		//printing file names
		printf("%s\n", de->d_name);
	}
	
	//ask the user for the log name
	printf("\nEnter the log name you wish to open\n");
	scanf("%s", name);
	
	//get and save the destination for the bin file to open
	char * dest;
	dest = (char*) malloc(sizeof(name)+sizeof(path));
	strcpy(dest, path);
    strcpy(dest+strlen(path), name);
	
	//open the log file 
	FILE *fp = fopen(dest, "rb");
	if (fp) {
				
		 // read structure from file
	    AK_redo_log log_file;
	    log_file.number = 0;
	    int i = 0;

      //read 1 element form the fp stream the size of log_file into log_file
	    fread(&log_file, sizeof(log_file), 1, fp);
	
	    // reading commands and storing them
	    for(i = 0; i < log_file.number; i++) {
	        memcpy(redo_log->command_recovery, log_file.command_recovery, sizeof(log_file.command_recovery));
	    }
	
		//getting the neccessary number for the next for-loop
	    redo_log->number = log_file.number;

	    //looking for unfinished commands and passing them on for recovery
	    for(i = 0; i < redo_log->number; i++) {
	        if(redo_log->command_recovery[i].finished != 1) {
	            AK_recovery_insert_row(redo_log->command_recovery[i].table_name, i);
			}
	    }
		
		//deallocation
		free(de);
		free(dest);
		fclose(fp);
	}
	
	//failed to open
	else{
		perror("fopen");
		free(dest);
		free(de);
        AK_EPI;
        exit(EXIT_FAILURE);
	}
	
	free(dest);
	free(de);
	closedir(dr);
	AK_EPI;
}

/** 
 * Function reads the latest.txt file which contains the name of the latest bin file that's been created.
 * Then it loads it and does the neccessary recovery operations.
 * @author Matija Večenaj
 * @brief Executes the recovery operation for the latest bin file
 * @param none
 * @return no value
 */
void AK_load_latest_log () {
	AK_PRO;	
	
	//get the path to archivelog
	char path[] = "akdb/akdb/bin/archivelog/";
	char buf[50];
	char * name;
	
	//open the txt file where the latest log name is written
	FILE *file = fopen("akdb/akdb/bin/archivelog/latest.txt", "r");
	
	//successfully opened
	if (file) {
		
		//read the file
		while (fgets(buf, sizeof(buf), file)) {
			
			//store the name of the log file
			name = (char *)malloc(sizeof(buf));
			strcpy(name, buf);
		}
	    fclose(file);
	}
	
	//failed to open
	else{
		perror("fopen");
        AK_EPI;
        exit(EXIT_FAILURE);
	}
	
	
	//get and save the destination for the bin file to open
	char * dest;
	dest = (char*) malloc(sizeof(name)+sizeof(path));
	
	//merging the two strings together
	strcpy(dest, path);
    strcpy(dest+strlen(path), name);
	
	//open the log file 
	FILE *fp = fopen(dest, "rb");
	if (fp) {
		
		 // read structure from file
	    AK_redo_log log_file;
	    log_file.number = 0;
	    int i = 0;

      //read 1 element form the fp stream the size of log_file into log_file
	    fread(&log_file, sizeof(log_file), 1, fp);
	
	    // reading commands and storing them
	    for(i = 0; i < log_file.number; i++) {
	        memcpy(redo_log->command_recovery, log_file.command_recovery, sizeof(log_file.command_recovery));
	    }
	
		//getting the neccessary number for the next for-loop
	    redo_log->number = log_file.number;

	    //looking for unfinished commands and passing them on for recovery
	    for(i = 0; i < redo_log->number; i++) {
	        if(redo_log->command_recovery[i].finished != 1) {
	            AK_recovery_insert_row(redo_log->command_recovery[i].table_name, i);
			}
	    }
		
		//deallocation
		free(name);
		free(dest);
		fclose(fp);
	}
	
	//failed to open
	else{
		perror("fopen");
		free(dest);
		free(name);
        AK_EPI;
        exit(EXIT_FAILURE);
	}
	

	AK_EPI;
}
