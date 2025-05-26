/*
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/**
 * @file api_functions.h
 * @brief Deklaracije osnovnih SQL operatora i funkcija za AKDB.
 *
 * Implementirani su +, -, *, /, ^, % za integer i float,
 * te standardne funkcije: now, len, lower, upper, replace,
 * abs, round, sqrt, left, substr.
 *
 * @author Antonio Brković
 * @date 2025
 */

#ifndef API_FUNCTIONS_H
#define API_FUNCTIONS_H

#include "../auxi/constants.h"
#include "../auxi/auxiliary.h"
#include "../auxi/mempro.h"
#include "../auxi/test.h"
#include "../file/fileio.h"
#include "../file/table.h"

/**
 * @author Antonio Brković
 * @brief Zbrajanje dvaju vrijednosti (int ili float)
 * @param a pokazivač na prvu vrijednost
 * @param b pokazivač na drugu vrijednost
 * @param type TYPE_INT ili TYPE_FLOAT
 * @return pokazivač na novi alocirani rezultat ili NULL na grešku
 */
void* AK_sql_add(void* a, void* b, int type);

/**
 * @author Antonio Brković
 * @brief Oduzimanje druge vrijednosti od prve (int ili float)
 * @param a pokazivač na minuend
 * @param b pokazivač na subtrahend
 * @param type TYPE_INT ili TYPE_FLOAT
 * @return pokazivač na novi alocirani rezultat ili NULL na grešku
 */
void* AK_sql_sub(void* a, void* b, int type);

/**
 * @author Antonio Brković
 * @brief Množenje dvaju vrijednosti (int ili float)
 * @param a pokazivač na prvi faktor
 * @param b pokazivač na drugi faktor
 * @param type TYPE_INT ili TYPE_FLOAT
 * @return pokazivač na novi alocirani rezultat ili NULL na grešku
 */
void* AK_sql_mul(void* a, void* b, int type);

/**
 * @author Antonio Brković
 * @brief Dijeljenje prve vrijednosti s drugom (int ili float)
 * @param a pokazivač na dividend
 * @param b pokazivač na divisor
 * @param type TYPE_INT ili TYPE_FLOAT
 * @return pokazivač na novi alocirani rezultat, ili NULL ako je divisor 0
 */
void* AK_sql_div(void* a, void* b, int type);

/**
 * @author Antonio Brković
 * @brief Potenciranje primera (int ili float)
 * @param a pokazivač na bazu
 * @param b pokazivač na exponent
 * @param type TYPE_INT ili TYPE_FLOAT
 * @return pokazivač na novi alocirani rezultat ili NULL na grešku
 */
void* AK_sql_pow(void* a, void* b, int type);

/**
 * @author Antonio Brković
 * @brief Ostatak pri dijeljenju (modulo) (int ili float)
 * @param a pokazivač na dividend
 * @param b pokazivač na divisor
 * @param type TYPE_INT ili TYPE_FLOAT
 * @return pokazivač na novi alocirani rezultat ili NULL na grešku
 */
void* AK_sql_mod(void* a, void* b, int type);

/**
 * @author Antonio Brković
 * @brief Vraća duljinu C-stringa
 * @param s pokazivač na null-terminirani string
 * @return pokazivač na int s duljinom (alocirano), ili NULL ako je s NULL
 */
void* AK_sql_len(void* s);

/**
 * @author Antonio Brković
 * @brief Pretvara string u mala slova
 * @param s pokazivač na null-terminirani string
 * @return pokazivač na novi lowercase string (alocirano), ili NULL ako je s NULL
 */
void* AK_sql_lower(void* s);

/**
 * @author Antonio Brković
 * @brief Pretvara string u velika slova
 * @param s pokazivač na null-terminirani string
 * @return pokazivač na novi uppercase string (alocirano), ili NULL ako je s NULL
 */
void* AK_sql_upper(void* s);

/**
 * @author Antonio Brković
 * @brief Vraća trenutačno vrijeme kao "YYYY-MM-DD HH:MM:SS"
 * @return pokazivač na timestamp string (alocirano)
 */
void* AK_sql_now();

/**
 * @author Antonio Brković
 * @brief Zamjenjuje sve pojavljivanja substringa
 * @param s pokazivač na source string
 * @param from pokazivač na substring koji se zamjenjuje
 * @param to pokazivač na novi substring
 * @return pokazivač na novi string (alocirano), ili NULL na grešku
 */
void* AK_sql_replace(void* s, void* from, void* to);

/**
 * @author Antonio Brković
 * @brief Apsolutna vrijednost (int ili float)
 * @param v pokazivač na ulaznu vrijednost
 * @param type TYPE_INT ili TYPE_FLOAT
 * @return pokazivač na novi alocirani rezultat ili NULL ako je v NULL
 */
void* AK_sql_abs(void* v, int type);

/**
 * @author Antonio Brković
 * @brief Zaokruživanje na najbliži cijeli (int ili float)
 * @param v pokazivač na ulaznu vrijednost
 * @param type TYPE_INT ili TYPE_FLOAT
 * @return pokazivač na novi alocirani int ili NULL ako je v NULL
 */
void* AK_sql_round(void* v, int type);

/**
 * @author Antonio Brković
 * @brief Kvadratni korijen (int ili float)
 * @param v pokazivač na ulaznu vrijednost
 * @param type TYPE_INT ili TYPE_FLOAT
 * @return pokazivač na novi alocirani float ili NULL ako je v NULL
 */
void* AK_sql_sqrt(void* v, int type);

/**
 * @author Antonio Brković
 * @brief Vraća prvih N znakova stringa
 * @param s pokazivač na null-terminirani string
 * @param len_ptr pokazivač na int s brojem znakova
 * @return pokazivač na substring (alocirano), ili NULL na grešku
 */
void* AK_sql_left(void* s, void* len_ptr);

/**
 * @author Antonio Brković
 * @brief Vraća substring od pozicije start, duljine len
 * @param s pokazivač na null-terminirani string
 * @param start_ptr pokazivač na int s početnom pozicijom
 * @param len_ptr pokazivač na int s duljinom
 * @return pokazivač na substring (alocirano), ili NULL na grešku
 */
void* AK_sql_substr(void* s, void* start_ptr, void* len_ptr);

/**
 * @author Antonio Brković
 * @brief Registracija ugrađenih SQL funkcija
 */
void AK_register_builtin_functions();

/**
 * @author Antonio Brković
 * @brief Samotestiranje svih API funkcija
 * @return TestResult s brojem uspješnih i neuspješnih testova
 */
TestResult AK_api_functions_test();

#endif /* API_FUNCTIONS_H */
