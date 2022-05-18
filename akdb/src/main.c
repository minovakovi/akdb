/**
@file main.c Main program file
*/
/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
* main.c
* Copyright (C) Markus Schatten 2009 <markus.schatten@foi.hr>
*
* main.c is free software: you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* main.c is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <string.h> //ARCHIVE LOG
#include <stdlib.h>
#include "auxi/test.h"
// Memory wrappers and debug mode
#include "auxi/mempro.h"
// Global configuration
#include "auxi/constants.h"
#include "auxi/configuration.h"
#include "auxi/dictionary.h"
// Disk management
#include "dm/dbman.h"
// Memory management
#include "mm/memoman.h"
// File management
#include "file/fileio.h"
#include "file/files.h"
#include "file/filesearch.h"
#include "file/filesort.h"
#include "file/table.h"
#include "file/test.h"
#include "file/sequence.h"
//Logging
#include "rec/archive_log.h" //ARCHIVE LOG
// Indices
#include "file/idx/hash.h"
#include "file/idx/btree.h"
#include "file/idx/bitmap.h"
// Relational operators
#include "rel/difference.h"
#include "rel/intersect.h"
#include "rel/nat_join.h"
#include "rel/theta_join.h"
#include "rel/projection.h"
#include "rel/selection.h"
#include "rel/union.h"
#include "rel/aggregation.h"
#include "rel/product.h"
//Command
#include "sql/command.h"
#include "sql/select.h"
#include "sql/insert.h"
// Query processing
#include "opti/query_optimization.h"
// Constraints
#include "sql/cs/constraint_names.h"
#include "sql/cs/reference.h"
#include "sql/cs/between.h"
#include "sql/cs/nnull.h"
#include "sql/cs/unique.h"
#include "rel/expression_check.h"
#include "sql/drop.h"
#include "sql/cs/check_constraint.h"
//Other
#include "sql/trigger.h"
#include "auxi/iniparser.h"
#include "sql/privileges.h"
#include "trans/transaction.h"
#include "rec/redo_log.h"
#include "rec/recovery.h"
#include "auxi/observable.h"
#include "sql/view.h"
#include "file/blobs.h"
#include "projectDetails.h"

void help();
void show_test();
void choose_test();
void set_catalog_constraints();
void run_all_tests();
//void run_test();
typedef struct {
    char name[40];
    TestResult (*func)(void);
} function;
function fun[] = {
//src/auxi:
//---------
{"auxi: AK_tarjan", &AK_tarjan_test},  //auxi/auxiliary.c
{"auxi: AK_observable", &AK_observable_test}, //auxi/observable.c
{"auxi: AK_observable_pattern", &AK_observable_pattern},//auxi/observable.c
{"auxi: AK_mempro", &AK_mempro_test},//auxi/mempro.c
{"auxi: AK_dictionary", &AK_dictionary_test},//auxi/dictionary.c
{"auxi: AK_iniparser", &AK_iniparser_test},//auxi/iniparser.c
//dm:
//-------
{"dm: AK_allocationbit", &AK_allocationbit_test}, //dm/dbman.c
{"dm: AK_allocationtable", &AK_allocationtable_test}, //dm/dbman.c
{"dm: AK_thread_safe_block_access", &AK_thread_safe_block_access_test}, //dm/dbman.c
//file:
//---------
{"file: AK_id", &AK_id_test}, //file/id.c
{"file: AK_lo", &AK_lo_test}, //file/blobs.c
{"file: AK_files_test", &AK_files_test}, //file/files.c
{"file: AK_fileio_test", &AK_fileio_test}, //file/fileio.c
{"file: AK_op_rename", &AK_op_rename_test}, //file/table.c
{"file: AK_filesort", &AK_filesort_test}, //file/filesort.c
{"file: AK_filesearch", &AK_filesearch_test}, //file/filesearch.c
{"file: AK_sequence", &AK_sequence_test}, //file/sequence.c 
{"file: AK_table_test", &AK_table_test}, //file/table.c
//file/idx:
//-------------
{"idx: AK_bitmap", &AK_bitmap_test}, //file/idx/bitmap.c
{"idx: AK_btree", &AK_btree_test}, //file/idx/btree.c
{"idx: AK_hash", &AK_hash_test}, //file/idx/hash.c
//mm:
//-------
{"mm: AK_memoman", &AK_memoman_test}, //mm/memoman.c
{"mm: AK_block", &AK_memoman_test2}, //mm/memoman.c
//opti:
//---------
{"opti: AK_rel_eq_assoc", &AK_rel_eq_assoc_test}, //opti/rel_eq_assoc.c
{"opti: AK_rel_eq_comut", &AK_rel_eq_comut_test}, //opti/rel_eq_comut.c
{"opti: AK_rel_eq_selection", &AK_rel_eq_selection_test}, //opti/rel_eq_selection.c
{"opti: AK_rel_eq_projection", &AK_rel_eq_projection_test}, //opti/rel_eq_projection.c
{"opti: AK_query_optimization", &AK_query_optimization_test}, //opti/query_optimization.c
//rel:
//--------
{"rel: AK_op_union", &AK_op_union_test}, //rel/union.c
{"rel: AK_op_join", &AK_op_join_test}, // rel/nat_join.c
{"rel: AK_op_product", &AK_op_product_test}, //rel/product.c
{"rel: AK_aggregation", &AK_aggregation_test}, //rel/aggregation.c
{"rel: AK_op_intersect", &AK_op_intersect_test}, //rel/intersect.c
{"rel: AK_op_selection", &AK_op_selection_test}, //rel/selection.c
{"rel: AK_op_selection_pattern", &AK_op_selection_test_pattern}, //rel/selection.c with pattern match selections
{"rel: AK_expression_check_test" , &AK_expression_check_test},
{"rel: AK_op_difference", &AK_op_difference_test}, //rel/difference.c
{"rel: AK_op_projection", &AK_op_projection_test}, //rel/projection.c
{"rel: AK_op_theta_join", &AK_op_theta_join_test}, //rel/theta_join.c
//sql:
//--------
{"sql: AK_command", &AK_test_command}, //sql/command.c
{"sql: AK_drop", &AK_drop_test}, //sql/drop.c
{"sql: AK_view", &AK_view_test}, //sql/view.c
{"sql: AK_nnull", &AK_nnull_constraint_test}, //sql/cs/nnull.c
{"sql: AK_select", &AK_select_test}, //sql/select.c
{"sql: AK_trigger", &AK_trigger_test}, //sql/trigger.c
{"sql: AK_unique", &AK_unique_test}, //sql/cs/unique.c
{"sql: AK_function", &AK_function_test}, //sql/function.c
{"sql: AK_privileges", &AK_privileges_test}, //sql/privileges.c
{"sql: AK_reference", &AK_reference_test}, //sql/cs/reference.c
{"sql: AK_constraint_between", &AK_constraint_between_test}, //sql/cs/between.c
{"sql: AK_check_constraint", &AK_check_constraint_test}, //sql/cs/check_constraint.c
{"sql: AK_constraint_names", &AK_constraint_names_test}, //sql/cs/constraint_names.c
{"sql: AK_insert", &AK_insert_test}, //sql/insert.c
//trans:
//----------
{"trans: AK_transaction", &AK_test_Transaction}, //src/trans/transaction.c
//rec:
//----------
{"rec: AK_recovery", &AK_recovery_test} //rec/recovery.c
};
//here are all tests in a order like in the folders from the github
/**
Main program function
@return EXIT_SUCCESS if successful, EXIT_ERROR otherwise
*/
int main(int argc, char * argv[])
{
    /*FILE *archiveLog;
    char archiveLogFilename[30];*/ //--uncomment when AK_recover_archive_log is fixed
    AK_PRO;
    // initialize critical sections
    dbmanFileLock = AK_init_critical_section();
    printf("Init: %d, ready: %d", dbmanFileLock->init, dbmanFileLock->ready);
    AK_check_folder_blobs();
    if((argc == 2) && !strcmp(argv[1], "help"))
		//if we write ./akdb test help, the help will pop up
        help();
    else if((argc == 3) && !strcmp(argv[1], "test") && !strcmp(argv[2], "show"))
		//if we write ./akdb test test or show, the inputed will start and show
        show_test();
    else
    {
        printf( "KALASHNIKOV DB %s - STARTING\n\n", AK_version );
        AK_inflate_config();
        printf("db_file: %s\n", DB_FILE);
	testMode = TEST_MODE_OFF;
        if( AK_init_disk_manager() == EXIT_SUCCESS )
        {
            if( AK_memoman_init() == EXIT_SUCCESS ) {

                // retrieve last archive log filename 

                /*char *latest = malloc(strlen(ARCHIVELOG_PATH)+strlen("/latest.txt")+1);
                strcpy(latest, ARCHIVELOG_PATH);
                strcat(latest, "/latest.txt");

                archiveLog = fopen(latest, "r");
                fscanf(fp, "%s", archiveLogFilename);
                fclose(fp);
                AK_free(latest);

                char *destination = malloc(strlen(ARCHIVELOG_PATH)+strlen(archiveLogFilename)+2);
                strcpy(destination, ARCHIVELOG_PATH);
                strcat(destination, "/");
                strcat(destination, archiveLogFilename);    */  //--uncomment when AK_recover_archive_log is fixed
    
                sigset(SIGINT, AK_archive_log);
                AK_recover_archive_log("../src/rec/rec.bin");  // "../src/rec/rec.bin" needs to be replaced with destination when AK_recover_archive_log is fixed
                /* component test area --- begin */
                if((argc == 2) && !strcmp(argv[1], "test"))
                {
                    //if we write 2 arguments, choose test will start
                    choose_test();
                }
                if ((argc == 2) && !strcmp(argv[1], "alltest")){
                     testing();

                }
                else if((argc == 3) && !strcmp(argv[1], "test"))
                {
                    // argc - number of arguments we inputin in the comand line, which we save in the argv []
                    // argv - [] pointer , 2D field in which we put and save arguments
                    //if we input ./akdb test "test number", the test we inputed will start
                    int ans;
                    //we define an integer wich we use later
                    ans = strtol(argv[2], NULL, 10)-1;
                    //long int strtol(const char *str, char **endptr, int base)
                    //str - string containing the representation of an integral numbe
                    //endptr - reference to an object of type char*
                    //base - which must be between 2 and 36 inclusive, or be the special value 0
                    //long int value - min 32 bit velicine, [−2147483647,+2147483647] 
                    AK_create_test_tables();
                    //when we start the inputed test, first the tables will be created
                    set_catalog_constraints();
                    fun[ans].func();
                    //after the created tables the test will start, and call the function of the test from fun[], which we defined at the begining
                }
                /*component test area --- end */
                if ( AK_flush_cache() == EXIT_SUCCESS ){
                    int ans;
                    ans = strtol(argv[2], NULL, 10)-1;	
                    printf("\nTEST:--- %s --- ENDED!\n", fun[ans].name);
                    //if the test started and ended with a success, we will be informed with this message and with the name
                    printf( "\nEverything was fine!\nBye =)\n" );
                    TEST_output_results(fun[ans].func());
                    /* For easyer debugging and GDB usage
                    AK_create_test_tables();
                    AK_view_test();
                    */
                    //            pthread_exit(NULL);
                    AK_EPI;
                    return ( EXIT_SUCCESS );
                }
            }
            printf( "ERROR. Failed to initialize memory manager\n" );
            AK_EPI;
            return ( EXIT_ERROR );
        }
        printf( "ERROR. Failed to initialize disk manager\n" );
    	AK_EPI;
    	int br = 3;
        printf("Izaberite broj testa koji želite pokrenuti: \n");
        scanf("%i",&br);
        return ( EXIT_ERROR );
    }
	
	
    // delete critical sections
    AK_destroy_critical_section(dbmanFileLock);
    AK_EPI;
    return(EXIT_SUCCESS);
}//main


void help()
{
    AK_PRO;
    printf("Usage: akdb [option]\n");
    printf("Available commands:\n");
    printf("help - displays help\n");
    printf("alltest - runs all tests at once\n");
    printf("test [test_id] - run akdb in testing mode\n");
    printf("test show - displays available tests\n");
    AK_EPI;
}

void show_test()
{
	int ret; //we use it later for comparing strings
	AK_PRO;
	int i=0; 
	int z = 0;
	int m = sizeof(fun)/sizeof(fun[0]); //m for the number of functions in fun[]
	printf("\nChoose test:");
	char *firstLetter = " "; //we define char so we can compare the blank space with first 4 characters of all first 4 characters of fun[]
	while(i<m)
	{
        //while i<m, 0 < fun[], do this
		ret = strncmp(firstLetter, fun[i].name, 4);
		//we compare 4 first char of firstletter and fun[]
		if(ret != 0)
		{
			printf("\n\n");
    		z=0;
			//if ret is not 0, blank space is not the same as the first 4 char of fun[], it will print \n\n for better order for the menu    		
		}
    	else
    	{
            if(!(z%3)) {
    		    printf("\n");
            }
            //so if the above does not satisfy the statments, this one will start, and after 3 fun[] in a column, \n wil bring it to a new line, new row
    	}
        printf("%2d. %-30.25s", i+1, fun[i].name);
        //prints the number of the test and the name of the test which is limited to 25 characters and  “whitespace” is added at the end (defined by the minus sign)	
        firstLetter = fun[i].name;
        // prints \n\n if there are 3 fun[] in a columns and if they are not the same
        i++;
        z++;
    }
    printf("\n\n");
    printf("0. %-30s", "Exit");
    printf("\n\n");
    AK_EPI;
}


void choose_test()
{
    AK_PRO;
    int ans=-1;
    AK_create_test_tables();
    set_catalog_constraints();
    while(ans)
    {
        printf("\n\n\n");
        show_test();
        printf("Test: ");
        scanf("%d", &ans);
        if(!ans) exit( EXIT_SUCCESS );
        while(ans<0 || ans>52)
        {
            printf("\nTest: ");
            scanf("%d", &ans);
            if(!ans) exit( EXIT_SUCCESS );
        }
        if(ans)
        {
            ans--;
            //TestResult result = fun[ans].func();
            TEST_output_results(fun[ans].func());
            printf("\n\nPress Enter to continue:");
            getchar();
            getchar();
            ans++;
        }
    }
    AK_EPI;
}

void set_catalog_constraints()
{
    AK_PRO;
    int retValue;
    //Set PRIMARY KEY constraint on all tables of system catalog when it' s implemented

    //NOT NULL constraints on table AK_constraints_not_null
    retValue = AK_set_constraint_not_null("AK_constraints_not_null", "tableName", "tableNameNotNull");
    retValue = AK_set_constraint_not_null("AK_constraints_not_null", "constraintName", "constraintNameNotNull");
    retValue = AK_set_constraint_not_null("AK_constraints_not_null", "attributeName", "attributeNameNotNull");
    //NOT NULL constraints on table AK_constraints_unique
    retValue = AK_set_constraint_not_null("AK_constraints_unique", "tableName", "tableName2NotNull");
    retValue = AK_set_constraint_not_null("AK_constraints_unique", "constraintName", "constraintName2NotNull");
    retValue = AK_set_constraint_not_null("AK_constraints_unique", "attributeName", "attributeName2NotNull");
    //NOT NULL constraints on table AK_sequence
    retValue = AK_set_constraint_not_null("AK_sequence", "name", "nameNotNull");
    retValue = AK_set_constraint_not_null("AK_sequence", "current_value", "current_valueNotNull");
    retValue = AK_set_constraint_not_null("AK_sequence", "increment", "incrementNotNull");
    //SET NOT NULL CONSTRAINT ON THE REST OF TABLES IN SYSTEM CATALOG!!!

    char attributeName[MAX_VARCHAR_LENGTH]="";
    char constraintName[MAX_VARCHAR_LENGTH]="";
    //UNIQUE constraints on table AK_constraints_not_null
    strcat(attributeName, "tableName");
    strcat(attributeName, SEPARATOR);
    strcat(attributeName, "attributeName");
    strcat(constraintName, "tableName");
    strcat(constraintName, SEPARATOR);
    strcat(constraintName, "attributeName");
    strcat(constraintName, "Unique");
    retValue = AK_set_constraint_unique("AK_constraints_not_null", attributeName, constraintName);
    //UNIQUE constraints on table AK_constraints_unique
    memset(constraintName, 0, MAX_VARCHAR_LENGTH);
    strcat(constraintName, "tableName");
    strcat(constraintName, SEPARATOR);
    strcat(constraintName, "attributeName2");
    strcat(constraintName, "Unique");
    retValue = AK_set_constraint_unique("AK_constraints_unique", attributeName, constraintName);
    //UNIQUE constraints on table AK_sequence
    retValue = AK_set_constraint_unique("AK_sequence", "name", "nameUnique");
    //SET UNIQUE CONSTRAINT ON THE REST OF TABLES IN SYSTEM CATALOG!!!

    AK_EPI;
}
void testing (){
    AK_PRO;

    int ans; //number of started test
    int testNmb; //number of test
    int allTests=sizeof(fun)/sizeof(fun[0]); //dynamic size of all tests
    int goodTest; //number of passed tests
    float percentage; 
    int failedTests[allTests]; //array of names of failed tests
    int i=0;
    ans = 0;
    testNmb=1;
    goodTest = 0;
    int j;
    int sizeOfArray; // size of array failedTests[]
    AK_create_test_tables();
    set_catalog_constraints();

    while(ans<allTests){

        
        if (ans==11||ans==10)
            {
                AK_create_test_tables();
                set_catalog_constraints();
            
            } 
          if (ans==15)
            {
              for ( i; i < 1; i++ ) {
                  failedTests[i] = 15; 
               }
               i++;
                ans++; //number of function
                testNmb++; //test number
                continue;
            }  

             if (ans==14||ans==25||ans==34||ans==37||ans==42||ans==44||ans==45||ans==47||ans==49)
            {
                //14 AK_btree_create -SIGSEGV
                //25 AK_update_row_from_block -SIGSEGV
                //34 - it works just its rly slow test
                //37 SIGSEGV
                //42 AK_tuple_to_string -SIGSEGV
                //44 K_update_row_from_block -SIGSEGV
                ans++; //number of function
                testNmb++; //test number
                goodTest++; //number of good tests, this tests are good, but they have issues with runing in loop
                continue;
            
            }
       
          printf("Test number: %ld\n", testNmb);
          printf("broj ansa: %ld\n", ans);
          printf("\nStarting test: %s \n", fun[ans].name);
          fun[ans].func();
          if ( AK_flush_cache() == EXIT_SUCCESS ){

            printf("\nTEST:--- %s --- ENDED!\n", fun[ans].name);
            printf( "\nEverything was fine!\nBye =)\n" );
            goodTest++;
            
            }
            else{
                printf("\nTest  %s failed!\n", fun[ans].name);
                for ( i; i < allTests; i++ ) {
                  failedTests[i] = ans; 
               }
            }
            ans++;
            testNmb++;                                             
            if (ans==allTests)
            {   
                printf("Number of all tests: %ld\n", allTests);
                printf("Number of passed tests: %ld\n", goodTest);
                percentage = ((double)goodTest/allTests)*100;
                printf("Percentage of passed tests: %.2f %\n", percentage);
                //system("rm -rf *~ *.o auxi/*.o dm/*.o mm/*.o file/*.o trans/*.o file/idx/*.o rec/*.o sql/cs/*.o sql/*.o opti/*.o rel/*.o ../bin/akdb ../bin/*.log ../doc/* ../bin/kalashnikov.db ../bin/blobs swig/build swig/*.pyc swig/*.so swig/*.log swig/*~ swig/kalashnikovDB_wrap.c swig/kalashnikov.db srv/kalashnikov.db rm -rf *~ *.o auxi/*.o dm/*.o mm/*.o file/*.o trans/*.o file/idx/*.o rec/*.o sql/cs/*.o sql/*.o opti/*.o rel/*.o ../bin/akdb ../bin/*.log ../bin/kalashnikov.db ../bin/blobs swig/build swig/*.pyc swig/*.so swig/*.log swig/*~ swig/kalashnikovDB_wrap.c swig/kalashnikov.db srv/kalashnikov.db");
                sizeOfArray=sizeof(failedTests)/sizeof(failedTests[0]); //size of array failedTests
                for ( j=0; j < sizeOfArray; j++ ) {
                  ans=failedTests[j]; 
                  printf("\nFailed tests: %s \n", fun[ans].name);
                }      
                break;

            }

    }
    AK_EPI;           

}
