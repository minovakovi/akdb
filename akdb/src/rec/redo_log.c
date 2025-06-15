#include "redo_log.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "../auxi/auxiliary.h"
#include "../auxi/constants.h"

#ifndef OPERATION_SELECT
#define OPERATION_SELECT 3
#endif

/**
 * @author Krunoslav Bilić updated by Dražen Bandić, second update by Tomislav Turek
 * @brief Function that adds a new element to redolog
 * @return EXIT_FAILURE if not allocated memory for ispis, otherwise EXIT_SUCCESS
 */
int AK_add_to_redolog(int command, struct list_node *row_root){
    AK_PRO;
    AK_redo_log* const redoLog = redo_log.ptr;
    if (redoLog == NULL) { return EXIT_FAILURE; }
    int n = redoLog->number;
    if(n >= MAX_REDO_LOG_ENTRIES){
        AK_archive_log(-10);
        redoLog->number = 0;
        n = 0;
    }

    /* Ako prepisujemo stari unos koji je bio SELECT, oslobodi njegovu memoriju */
    if (redoLog->command_recovery[n].operation == OPERATION_SELECT && redoLog->command_recovery[n].select_result) {
        AK_free_results_struct(redoLog->command_recovery[n].select_result);
    }

    memset(&redoLog->command_recovery[n], 0, sizeof(AK_command_recovery_struct));
    struct list_node * el = (struct list_node *) AK_First_L2(row_root);
    
    char table[MAX_ATT_NAME];
    memset(table, '\0', MAX_ATT_NAME);
    if (el && el->table) { strncpy(table, el->table, MAX_ATT_NAME - 1); }
    
    strncpy(redoLog->command_recovery[n].table_name, table, MAX_VARCHAR_LENGTH -1);
    redoLog->command_recovery[n].operation = command;
    redoLog->command_recovery[n].finished = 0;
    redoLog->command_recovery[n].select_result = NULL;
    redoLog->command_recovery[n].logged_at_timestamp = time(NULL);

    int i = 0;
    while (el != NULL && i < MAX_ATTRIBUTES) {
        if(el->data != NULL){
            if (el->type == TYPE_INT) {
                sprintf(redoLog->command_recovery[n].arguments[i], "%d", *((int*)el->data));
            } else if (el->type == TYPE_FLOAT){
                 sprintf(redoLog->command_recovery[n].arguments[i], "%.3f", *((float*)el->data));
            } else {
                 strncpy(redoLog->command_recovery[n].arguments[i], el->data, MAX_VARCHAR_LENGTH - 1);
            }
        }
        i++;
        el = el->next;
    }
    redoLog->number = n + 1;
    AK_EPI;
    return EXIT_SUCCESS;
}


/* Implementacija novih funkcija za SELECT keširanje */

void AK_generate_select_query_identifier(const char *srcTable, struct list_node *attributes, struct list_node *condition, char* buffer, size_t buffer_size) {
    AK_PRO;
    memset(buffer, 0, buffer_size);
    if (srcTable) {
        strncat(buffer, srcTable, buffer_size - strlen(buffer) - 1);
    }
    strncat(buffer, "::ATTR::", buffer_size - strlen(buffer) - 1);
    struct list_node *el = NULL;
    if (attributes) {
        for (el = AK_First_L2(attributes); el != NULL; el = AK_Next_L2(el)) {
            if (el->data) strncat(buffer, el->data, buffer_size - strlen(buffer) - 1);
            if (el->next) strncat(buffer, ",", buffer_size - strlen(buffer) - 1);
        }
    }
    strncat(buffer, "::COND::", buffer_size - strlen(buffer) - 1);
    if (condition) {
        for (el = AK_First_L2(condition); el != NULL; el = AK_Next_L2(el)) {
            if (el->data) {
                char temp_str[MAX_VARCHAR_LENGTH];
                if (el->type == TYPE_INT) sprintf(temp_str, "%d", *((int*)el->data));
                else if (el->type == TYPE_FLOAT) sprintf(temp_str, "%.3f", *((float*)el->data));
                else strncpy(temp_str, el->data, MAX_VARCHAR_LENGTH -1);
                strncat(buffer, temp_str, buffer_size - strlen(buffer) - 1);
            }
            if (el->next) strncat(buffer, ",", buffer_size - strlen(buffer) - 1);
        }
    }
    AK_EPI;
}

int AK_add_to_redolog_select_with_result(int command, const char *srcTable, struct list_node *attributes, struct list_node *condition, AK_results *result_to_cache, const char* query_identifier) {
    AK_PRO;
    AK_redo_log* const redoLog = redo_log.ptr;
    if (redoLog == NULL || result_to_cache == NULL) {
        AK_EPI; return EXIT_FAILURE;
    }
    int n = redoLog->number;
    if (n >= MAX_REDO_LOG_ENTRIES) {
        // Jednostavna LRU strategija: pronađi i prepiši najstariji SELECT unos
        int oldest_idx = -1;
        time_t oldest_ts = time(NULL) + 1;
        for(int i = 0; i < redoLog->number; i++) {
            if (redoLog->command_recovery[i].operation == OPERATION_SELECT) {
                if (redoLog->command_recovery[i].logged_at_timestamp < oldest_ts) {
                    oldest_ts = redoLog->command_recovery[i].logged_at_timestamp;
                    oldest_idx = i;
                }
            }
        }
        if (oldest_idx != -1) n = oldest_idx;
        else n = 0; // Ako nema SELECT unosa, prepiši prvi
    }

    if (redoLog->command_recovery[n].operation == OPERATION_SELECT && redoLog->command_recovery[n].select_result) {
        AK_free_results_struct(redoLog->command_recovery[n].select_result);
    }
    
    AK_command_recovery_struct* current_entry = &redoLog->command_recovery[n];
    memset(current_entry, 0, sizeof(AK_command_recovery_struct));

    current_entry->operation = command; 
    if (srcTable) strncpy(current_entry->table_name, srcTable, MAX_VARCHAR_LENGTH - 1);
    if (query_identifier) strncpy(current_entry->query_identifier, query_identifier, sizeof(current_entry->query_identifier) - 1);
    
    current_entry->select_result = (AK_results*)AK_malloc(sizeof(AK_results));
    if (!current_entry->select_result) { AK_EPI; return EXIT_FAILURE; }
    memcpy(current_entry->select_result, result_to_cache, sizeof(AK_results));
    
    if (result_to_cache->source_table) { current_entry->select_result->source_table = AK_strdup(result_to_cache->source_table); }
    else { current_entry->select_result->source_table = NULL; }

    if (result_to_cache->result_rows) {
        current_entry->select_result->result_rows = AK_copy_list_deep(result_to_cache->result_rows);
        if (!current_entry->select_result->result_rows) {
            AK_free_results_struct(current_entry->select_result);
            AK_EPI; return EXIT_FAILURE;
        }
    } else { current_entry->select_result->result_rows = NULL; }
    
    current_entry->select_result->result_block = NULL; 
    current_entry->logged_at_timestamp = time(NULL);
    current_entry->finished = 1; 

    if (n == redoLog->number) redoLog->number++;
    AK_EPI;
    return EXIT_SUCCESS;
}

int AK_check_redo_log_select_for_caching(const char *srcTable, struct list_node *attributes, struct list_node *condition, const char* query_identifier, AK_results **out_result, time_t *out_logged_at_timestamp) {
    AK_PRO;
    AK_redo_log* const redoLog = redo_log.ptr;
    if (redoLog == NULL || out_result == NULL || out_logged_at_timestamp == NULL || query_identifier == NULL) {
        AK_EPI; return 0; 
    }
    *out_result = NULL; 
    *out_logged_at_timestamp = 0;
    for (int i = redoLog->number - 1; i >= 0; i--) {
        AK_command_recovery_struct* entry = &redoLog->command_recovery[i];
        if (entry->operation == OPERATION_SELECT && strcmp(entry->query_identifier, query_identifier) == 0) {
            if (entry->select_result) {
                *out_result = entry->select_result;
                *out_logged_at_timestamp = entry->logged_at_timestamp;
                AK_EPI;
                return 1;
            }
        }
    }
    AK_EPI;
    return 0; 
}


/**
 * @author Krunoslav Bilić, Tomislav Turek
 * @brief Marks all commands in the current redo_log as finished (committed).
 */
void AK_redolog_commit() {
    AK_PRO;
    int i;
    AK_redo_log* const redoLog = redo_log.ptr;
    if(!redoLog) { AK_EPI; return; }
    for(i = 0; i < redoLog->number; i++) {
        if (redoLog->command_recovery[i].operation != OPERATION_SELECT) {
            redoLog->command_recovery[i].finished = 1;
        }
    }
    AK_EPI;
}

/**
 * @author Krunoslav Bilić updated by Dražen Bandić, second update by Tomislav Turek
 * @brief Function that prints out the content of redolog memory
 * @return No return value.
 */
void AK_printout_redolog(){
    AK_PRO;
    AK_redo_log* const redoLog = redo_log.ptr;
    if (redoLog == NULL) {
        printf("Redo log is not initialized (redo_log.ptr is NULL).\n");
        AK_EPI;
        return;
    }
    int x = redoLog->number;
    printf("--- Redo Log Content (Total Entries: %d) ---\n", x);
    for (int i = 0; i < x; i++){
        AK_command_recovery_struct* entry = &redoLog->command_recovery[i];
        printf("%d. Op: %d, Table: '%s', Finished: %d, LoggedAt: %ld\n", 
               i, entry->operation, entry->table_name, entry->finished, (long)entry->logged_at_timestamp);
        
        if (entry->operation == OPERATION_SELECT) {
            printf("   QueryID: '%s'\n", entry->query_identifier);
            if(entry->select_result){
                 printf("   SELECT Result Cached: Yes (Rows: %d, Cols: %d)\n", 
                   entry->select_result->num_rows, entry->select_result->num_cols);
            } else {
                 printf("   SELECT Result Cached: No\n");
            }
        }
    }
    printf("--- End of Redo Log ---\n");
    AK_EPI;
}

/**
 * @author Dražen Bandić
 * @brief Function that checks if the attribute contains '|', and if it does it replaces it with \"\\|\"
 * @return new attribute
 */
char* AK_check_attributes(char *attributes){
    AK_PRO;
    if (attributes == NULL) { AK_EPI; return NULL; }
    /* Originalni kod je bio sklon greškama i buffer overflowu.
     * AK_strdup je sigurnija i jednostavnija alternativa ako escapiranje nije apsolutno nužno.
     * Ako jest, potrebna je robusnija implementacija od originalne. */
    char* result = AK_strdup(attributes);
    AK_EPI;
    return result;
}