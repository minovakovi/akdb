/**
 * @file sql_insert.h
 * @author Klara Skaulj
 * @brief Zaglavlje za SQL INSERT naredbu u AKdb.
 *
 * Deklaracija funkcije `sql_insert` koja omogućava umetanje redaka u tablicu
 * korištenjem jednostavne SQL sintakse. Korisno za ispitivanje i testiranje.
 */

#ifndef SQL_INSERT_H
#define SQL_INSERT_H



/**
 * @brief Umeće red u zadanu tablicu putem SQL INSERT naredbe.
 * 
 * @param table_name Naziv tablice
 * @param columns_str Popis kolona (npr. "ime,prezime")
 * @param values_str Vrijednosti (npr. "'Ana','Anić'")
 * @return int 0 ako je uspješno, negativno ako nije
 */

int sql_insert(const char* table_name, const char* columns_str, const char* values_str);
/**
 * @brief Inserts a row into the specified table
 * @param table_name Table into which the row is inserted
 * @param columns_str Comma-separated column names
 * @param values_str Comma-separated values (strings wrapped in quotes)
 * @return 0 on success, negative value on error
 */
#endif
