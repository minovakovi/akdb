#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../file/fileio.h"
//#include "../util/list.h"
#include "sql_insert.h"

/**
 * @file sql_insert.c
 * @author Klara Skaulj
 * @brief Implementacija SQL INSERT naredbe za AKdb sustav.
 * 
 * Ova datoteka sadrži implementaciju funkcionalnosti umetanja novog retka u tablicu korištenjem
 * povezane liste `list_node` i postojećih funkcija iz `fileio.c`.
 * 
 * Funkcija `sql_insert` parsira SQL INSERT naredbu i koristi funkcije poput `AK_Insert_New_Element`
 * i `AK_insert_row` za umetanje podataka u bazu.
 * 
 * @see sql_insert.h
 */

/**
 * @brief Parsira SQL INSERT naredbu i umeta red u zadanu tablicu.
 *
 * Funkcija prima naziv tablice, popis kolona i vrijednosti, te koristi pomoćne funkcije
 * za formiranje povezane liste elemenata koji predstavljaju redak. Na kraju se poziva
 * `AK_insert_row` kako bi se podatci zapisali u tablicu.
 * 
 * @param table_name Naziv tablice
 * @param columns_str Zarezom odvojeni popis kolona (npr. "ime,prezime")
 * @param values_str Zarezom odvojene vrijednosti (npr. "'Ivan','Horvat'")
 * @return int 0 ako je uspješno, negativna vrijednost ako dođe do greške
 */

int sql_insert(const char* table_name, const char* columns_str, const char* values_str) {
    AK_PRO;
    
    struct list_node row_root;
    row_root.next = NULL;
    
    // Priprema kolona i vrijednosti
    char columns[1024], values[1024];
    strncpy(columns, columns_str, sizeof(columns));
    strncpy(values, values_str, sizeof(values));

    char *col_token = strtok(columns, ",");
    char *val_token = strtok(values, ",");

    struct list_node *current = &row_root;

    while (col_token != NULL && val_token != NULL) {
        // Trimanje leading spaces
        while (*col_token == ' ') col_token++;
        while (*val_token == ' ') val_token++;

        // Uklanjanje navodnika ako postoje
        if (val_token[0] == '\'' || val_token[0] == '"') {
            val_token++;
            char *quote = strchr(val_token, '\'');
            if (quote) *quote = '\0';
        }

        char *data = (char *)malloc(MAX_VARCHAR_LENGTH);
        strncpy(data, val_token, MAX_VARCHAR_LENGTH);

        // Ubaci novi element
        AK_Insert_New_Element(TYPE_VARCHAR, data, (char*)table_name, col_token, current);
        current = current->next;

        col_token = strtok(NULL, ",");
        val_token = strtok(NULL, ",");
    }

    // Ubaci redak u tablicu
    int status = AK_insert_row(&row_root);
    if (status == EXIT_SUCCESS) {
        printf("Row successfully inserted into table '%s'.\n", table_name);
    } else {
        printf("Error inserting row into table '%s'.\n", table_name);
    }

    AK_EPI;
    return status;
}
