/**
 * @file api_functions.c
 * @brief Implementacija osnovnih SQL operatora i funkcija za AKDB
 *
 * Ovdje su definirani +, -, *, /, ^, % za integer i float,
 * te standardne funkcije: now, len, lower, upper, replace,
 * abs, round, sqrt, left, substr.
 *
 * @author Antonio Brković
 * @date 2025
 */

#include "api_functions.h"
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <stdio.h>

/**
 * @author Antonio Brković
 * @brief Zbrajanje dva cijela broja
 * @param a prvi operand
 * @param b drugi operand
 * @return njihov zbroj
 */
static int sql_add_int(int a, int b) {
    return a + b;
}

/**
 * @author Antonio Brković
 * @brief Oduzimanje dva cijela broja
 * @param a prvi operand
 * @param b drugi operand
 * @return rezultat a - b
 */
static int sql_subtract_int(int a, int b) {
    return a - b;
}

/**
 * @author Antonio Brković
 * @brief Množenje dva cijela broja
 * @param a prvi operand
 * @param b drugi operand
 * @return njihov produkt
 */
static int sql_multiply_int(int a, int b) {
    return a * b;
}

/**
 * @author Antonio Brković
 * @brief Dijeljenje dva cijela broja (ako je b=0 vraća 0)
 * @param a prvi operand
 * @param b drugi operand
 * @return rezultat a / b ili 0 ako je b = 0
 */
static int sql_divide_int(int a, int b) {
    return b != 0 ? a / b : 0;
}

/**
 * @author Antonio Brković
 * @brief Ostatak pri dijeljenju dva cijela broja (modulo)
 * @param a prvi operand
 * @param b drugi operand
 * @return a % b ili 0 ako je b = 0
 */
static int sql_modulo_int(int a, int b) {
    return b != 0 ? a % b : 0;
}

/**
 * @author Antonio Brković
 * @brief Potenciranje dva cijela broja
 * @param a baza
 * @param b exponent
 * @return (int) pow(a, b)
 */
static int sql_power_int(int a, int b) {
    return (int)powf((float)a, (float)b);
}

/**
 * @author Antonio Brković
 * @brief Zbrajanje dva float broja
 * @param a prvi operand
 * @param b drugi operand
 * @return njihov zbroj
 */
static float sql_add_float(float a, float b) {
    return a + b;
}

/**
 * @author Antonio Brković
 * @brief Oduzimanje dva float broja
 * @param a prvi operand
 * @param b drugi operand
 * @return rezultat a - b
 */
static float sql_subtract_float(float a, float b) {
    return a - b;
}

/**
 * @author Antonio Brković
 * @brief Množenje dva float broja
 * @param a prvi operand
 * @param b drugi operand
 * @return njihov produkt
 */
static float sql_multiply_float(float a, float b) {
    return a * b;
}

/**
 * @author Antonio Brković
 * @brief Dijeljenje dva float broja (ako je b=0 vraća 0.0f)
 * @param a prvi operand
 * @param b drugi operand
 * @return rezultat a / b ili 0.0f ako je b = 0
 */
static float sql_divide_float(float a, float b) {
    return b != 0.0f ? a / b : 0.0f;
}

/**
 * @author Antonio Brković
 * @brief Potenciranje dva float broja
 * @param a baza
 * @param b exponent
 * @return powf(a, b)
 */
static float sql_power_float(float a, float b) {
    return powf(a, b);
}

/**
 * @author Antonio Brković
 * @brief Zbrajanje dvaju vrijednosti (int ili float)
 * @param a pokazivač na prvu vrijednost
 * @param b pokazivač na drugu vrijednost
 * @param type TYPE_INT ili TYPE_FLOAT
 * @return pokazivač na novi alocirani rezultat
 */
void* AK_sql_add(void* a, void* b, int type) {
    AK_PRO;
    void* result = NULL;
    if (type == TYPE_INT) {
        int* r = AK_malloc(sizeof *r);
        *r = sql_add_int(*(int*)a, *(int*)b);
        result = r;
    } else {
        float* r = AK_malloc(sizeof *r);
        *r = sql_add_float(*(float*)a, *(float*)b);
        result = r;
    }
    AK_EPI;
    return result;
}

/**
 * @author Antonio Brković
 * @brief Oduzimanje dvaju vrijednosti (int ili float)
 * @param a pokazivač na prvu vrijednost
 * @param b pokazivač na drugu vrijednost
 * @param type TYPE_INT ili TYPE_FLOAT
 * @return pokazivač na novi alocirani rezultat
 */
void* AK_sql_sub(void* a, void* b, int type) {
    AK_PRO;
    void* result = NULL;
    if (type == TYPE_INT) {
        int* r = AK_malloc(sizeof *r);
        *r = sql_subtract_int(*(int*)a, *(int*)b);
        result = r;
    } else {
        float* r = AK_malloc(sizeof *r);
        *r = sql_subtract_float(*(float*)a, *(float*)b);
        result = r;
    }
    AK_EPI;
    return result;
}

/**
 * @author Antonio Brković
 * @brief Množenje dvaju vrijednosti (int ili float)
 * @param a pokazivač na prvu vrijednost
 * @param b pokazivač na drugu vrijednost
 * @param type TYPE_INT ili TYPE_FLOAT
 * @return pokazivač na novi alocirani rezultat
 */
void* AK_sql_mul(void* a, void* b, int type) {
    AK_PRO;
    void* result = NULL;
    if (type == TYPE_INT) {
        int* r = AK_malloc(sizeof *r);
        *r = sql_multiply_int(*(int*)a, *(int*)b);
        result = r;
    } else {
        float* r = AK_malloc(sizeof *r);
        *r = sql_multiply_float(*(float*)a, *(float*)b);
        result = r;
    }
    AK_EPI;
    return result;
}

/**
 * @author Antonio Brković
 * @brief Dijeljenje dvaju vrijednosti (int ili float)
 * @param a pokazivač na prvu vrijednost
 * @param b pokazivač na drugu vrijednost
 * @param type TYPE_INT ili TYPE_FLOAT
 * @return pokazivač na novi alocirani rezultat (0 ako je b=0)
 */
void* AK_sql_div(void* a, void* b, int type) {
    AK_PRO;
    void* result = NULL;
    if (type == TYPE_INT) {
        int* r = AK_malloc(sizeof *r);
        *r = sql_divide_int(*(int*)a, *(int*)b);
        result = r;
    } else {
        float* r = AK_malloc(sizeof *r);
        *r = sql_divide_float(*(float*)a, *(float*)b);
        result = r;
    }
    AK_EPI;
    return result;
}


/**
 * @author Antonio Brković
 * @brief Potenciranje dvaju vrijednosti (int ili float)
 * @param a pokazivač na prvu vrijednost
 * @param b pokazivač na drugu vrijednost
 * @param type TYPE_INT ili TYPE_FLOAT
 * @return pokazivač na novi alocirani rezultat
 */
void* AK_sql_pow(void* a, void* b, int type) {
    AK_PRO;
    void* result = NULL;
    if (type == TYPE_INT) {
        int* r = AK_malloc(sizeof *r);
        *r = sql_power_int(*(int*)a, *(int*)b);
        result = r;
    } else {
        float* r = AK_malloc(sizeof *r);
        *r = sql_power_float(*(float*)a, *(float*)b);
        result = r;
    }
    AK_EPI;
    return result;
}

/**
 * @author Antonio Brković
 * @brief Moduo dijeljenje dvaju vrijednosti (int ili float)
 * @param a pokazivač na prvu vrijednost
 * @param b pokazivač na drugu vrijednost
 * @param type TYPE_INT ili TYPE_FLOAT
 * @return pokazivač na novi alocirani rezultat
 */
void* AK_sql_mod(void* a, void* b, int type) {
    AK_PRO;
    void* result = NULL;
    if (type == TYPE_INT) {
        int* r = AK_malloc(sizeof *r);
        *r = sql_modulo_int(*(int*)a, *(int*)b);
        result = r;
    } else {
        float* r = AK_malloc(sizeof *r);
        *r = fmodf(*(float*)a, *(float*)b);
        result = r;
    }
    AK_EPI;
    return result;
}

/**
 * @author Antonio Brković
 * @brief Vraća duljinu niza znakova
 * @param s pokazivač na C-string
 * @return pokazivač na novi alocirani int s duljinom
 */
void* AK_sql_len(void* s) {
    AK_PRO;
    void* result = NULL;
    if (s) {
        int* r = AK_malloc(sizeof *r);
        *r = strlen((char*)s);
        result = r;
    }
    AK_EPI;
    return result;
}

/**
 * @author Antonio Brković
 * @brief Pretvara C-string u mala slova
 * @param s pokazivač na C-string
 * @return pokazivač na novi alocirani C-string
 */
void* AK_sql_lower(void* s) {
    AK_PRO;
    void* result = NULL;
    if (s) {
        char* str = (char*)s;
        size_t n = strlen(str);
        char* r = AK_malloc(n + 1);
        for (size_t i = 0; i <= n; i++) r[i] = tolower(str[i]);
        result = r;
    }
    AK_EPI;
    return result;
}

/**
 * @author Antonio Brković
 * @brief Pretvara C-string u velika slova
 * @param s pokazivač na C-string
 * @return pokazivač na novi alocirani C-string
 */
void* AK_sql_upper(void* s) {
    AK_PRO;
    void* result = NULL;
    if (s) {
        char* str = (char*)s;
        size_t n = strlen(str);
        char* r = AK_malloc(n + 1);
        for (size_t i = 0; i <= n; i++) r[i] = toupper(str[i]);
        result = r;
    }
    AK_EPI;
    return result;
}

/**
 * @author Antonio Brković
 * @brief Vraća trenutačno vrijeme kao yyyy-mm-dd HH:MM:SS
 * @return pokazivač na novi alocirani C-string
 */
void* AK_sql_now() {
    AK_PRO;
    char* buf = AK_malloc(20);
    if (buf) {
        time_t t = time(NULL);
        struct tm* tm_info = localtime(&t);
        strftime(buf, 20, "%Y-%m-%d %H:%M:%S", tm_info);
    }
    AK_EPI;
    return buf;
}

/**
 * @author Antonio Brković
 * @brief Zamjena svih pojavljivanja substringa
 * @param s   izvorni C-string
 * @param from ključni C-string koji se zamjenjuje
 * @param to   C-string zamjene
 * @return pokazivač na novi alocirani C-string
 */
void* AK_sql_replace(void* s, void* from, void* to) {
    AK_PRO;
    void* result = NULL;
    if (s && from && to) {
        char *str = (char*)s, *f = (char*)from, *rto = (char*)to;
        size_t len_f = strlen(f);
        char* buf = AK_malloc(strlen(str) + 64);
        buf[0] = '\0';
        char* p = str;
        while ((p = strstr(p, f))) {
            strncat(buf, str, p - str);
            strcat(buf, rto);
            str = p + len_f;
            p = str;
        }
        strcat(buf, str);
        result = buf;
    }
    AK_EPI;
    return result;
}

/**
 * @author Antonio Brković
 * @brief Apsolutna vrijednost (int ili float)
 * @param v    pokazivač na vrijednost
 * @param type TYPE_INT ili TYPE_FLOAT
 * @return pokazivač na novi alocirani rezultat
 */
void* AK_sql_abs(void* v, int type) {
    AK_PRO;
    void* result = NULL;
    if (v) {
        if (type == TYPE_INT) {
            int* r = AK_malloc(sizeof *r);
            *r = abs(*(int*)v);
            result = r;
        } else {
            float* r = AK_malloc(sizeof *r);
            *r = fabsf(*(float*)v);
            result = r;
        }
    }
    AK_EPI;
    return result;
}

/**
 * @author Antonio Brković
 * @brief Zaokruživanje na najbliži cijeli (float ili int)
 * @param v    pokazivač na vrijednost
 * @param type TYPE_INT ili TYPE_FLOAT
 * @return pokazivač na novi alocirani int
 */
void* AK_sql_round(void* v, int type) {
    AK_PRO;
    void* result = NULL;
    if (v) {
        float val = (type == TYPE_INT) ? (float)(*(int*)v) : (*(float*)v);
        int* r = AK_malloc(sizeof *r);
        *r = (int)roundf(val);
        result = r;
    }
    AK_EPI;
    return result;
}

/**
 * @author Antonio Brković
 * @brief Kvadratni korijen (float ili int)
 * @param v    pokazivač na vrijednost
 * @param type TYPE_INT ili TYPE_FLOAT
 * @return pokazivač na novi alocirani float
 */
void* AK_sql_sqrt(void* v, int type) {
    AK_PRO;
    void* result = NULL;
    if (v) {
        float val = (type == TYPE_INT) ? (float)(*(int*)v) : (*(float*)v);
        float* r = AK_malloc(sizeof *r);
        *r = sqrtf(val);
        result = r;
    }
    AK_EPI;
    return result;
}

/**
 * @author Antonio Brković
 * @brief Vraća prvih N znakova C-stringa
 * @param s    pokazivač na C-string
 * @param lptr pokazivač na int s brojkom N
 * @return pokazivač na novi alocirani C-string
 */
void* AK_sql_left(void* s, void* lptr) {
    AK_PRO;
    void* result = NULL;
    if (s && lptr) {
        char* str = (char*)s;
        int len   = *(int*)lptr;
        char* r   = AK_malloc(len + 1);
        strncpy(r, str, len);
        r[len] = '\0';
        result = r;
    }
    AK_EPI;
    return result;
}

/**
 * @author Antonio Brković
 * @brief Izrezuje substring C-stringa [start, start+len)
 * @param s      pokazivač na C-string
 * @param startp pokazivač na int s početnom pozicijom
 * @param lenp   pokazivač na int s duljinom
 * @return pokazivač na novi alocirani C-string
 */
void* AK_sql_substr(void* s, void* startp, void* lenp) {
    AK_PRO;
    void* result = NULL;
    if (s && startp && lenp) {
        char* str = (char*)s;
        int   st  = *(int*)startp;
        int   ln  = *(int*)lenp;
        char* r   = AK_malloc(ln + 1);
        strncpy(r, str + st, ln);
        r[ln] = '\0';
        result = r;
    }
    AK_EPI;
    return result;
}

/**
 * @author Antonio Brković
 * @brief Registracija ugrađenih SQL funkcija
 */
void AK_register_builtin_functions() {
    AK_PRO;
    struct list_node* empty = AK_malloc(sizeof *empty);
    AK_Init_L3(&empty);
    AK_function_add("now",     TYPE_VARCHAR, empty);
    AK_function_add("len",     TYPE_INT,     empty);
    AK_function_add("lower",   TYPE_VARCHAR, empty);
    AK_function_add("upper",   TYPE_VARCHAR, empty);
    AK_function_add("replace", TYPE_VARCHAR, empty);
    AK_function_add("abs",     TYPE_INT,     empty);
    AK_function_add("round",   TYPE_INT,     empty);
    AK_function_add("sqrt",    TYPE_FLOAT,   empty);
    AK_function_add("left",    TYPE_VARCHAR, empty);
    AK_function_add("substr",  TYPE_VARCHAR, empty);
    AK_DeleteAll_L3(&empty);
    AK_free(empty);
    AK_EPI;
}

/**
 * @author Antonio Brković
 * @brief Testiranje svih API funkcija
 * @return TestResult s brojem uspješnih i neuspješnih testova
 */
TestResult AK_api_functions_test() {
    AK_PRO;
    int success = 0, failed = 0;
    int test_num = 0;

    printf("Running API functions tests:\n");

    {   int  a = 5, b = 7;
        test_num++; printf("Test %2d: AK_sql_add (int)........ ", test_num);
        int *ires = AK_sql_add(&a, &b, TYPE_INT);
        if (ires && *ires == 12) { success++; printf("passed\n"); }
        else                    { failed++;  printf("FAILED\n"); }
        AK_free(ires);
    }
    {   float a = 2.5f, b = 3.5f;
        test_num++; printf("Test %2d: AK_sql_add (float)...... ", test_num);
        float *fres = AK_sql_add(&a, &b, TYPE_FLOAT);
        if (fres && fabsf(*fres -  6.0f) < 1e-6f) { success++; printf("passed\n"); }
        else                                      { failed++;  printf("FAILED\n"); }
        AK_free(fres);
    }

    {   int  a = 7, b = 5;
        test_num++; printf("Test %2d: AK_sql_sub (int)........ ", test_num);
        int *ires = AK_sql_sub(&a, &b, TYPE_INT);
        if (ires && *ires == 2) { success++; printf("passed\n"); }
        else                    { failed++;  printf("FAILED\n"); }
        AK_free(ires);
    }
    {   float a = 5.5f, b = 3.5f;
        test_num++; printf("Test %2d: AK_sql_sub (float)...... ", test_num);
        float *fres = AK_sql_sub(&a, &b, TYPE_FLOAT);
        if (fres && fabsf(*fres - 2.0f) < 1e-6f) { success++; printf("passed\n"); }
        else                                    { failed++;  printf("FAILED\n"); }
        AK_free(fres);
    }

    {   int  a = 5, b = 7;
        test_num++; printf("Test %2d: AK_sql_mul (int)........ ", test_num);
        int *ires = AK_sql_mul(&a, &b, TYPE_INT);
        if (ires && *ires == 35) { success++; printf("passed\n"); }
        else                     { failed++;  printf("FAILED\n"); }
        AK_free(ires);
    }
    {   float a = 2.5f, b = 3.0f;
        test_num++; printf("Test %2d: AK_sql_mul (float)...... ", test_num);
        float *fres = AK_sql_mul(&a, &b, TYPE_FLOAT);
        if (fres && fabsf(*fres - 7.5f) < 1e-6f) { success++; printf("passed\n"); }
        else                                     { failed++;  printf("FAILED\n"); }
        AK_free(fres);
    }

    {   int  a = 6, b = 3;
        test_num++; printf("Test %2d: AK_sql_div (int) ........ ", test_num);
        int *ires = AK_sql_div(&a, &b, TYPE_INT);
        if (ires && *ires == 2) { success++; printf("passed\n"); }
        else                    { failed++;  printf("FAILED\n"); }
        AK_free(ires);
    }
    {   float a = 7.5f, b = 2.5f;
        test_num++; printf("Test %2d: AK_sql_div (float) ...... ", test_num);
        float *fres = AK_sql_div(&a, &b, TYPE_FLOAT);
        if (fres && fabsf(*fres - 3.0f) < 1e-6f) { success++; printf("passed\n"); }
        else                                     { failed++;  printf("FAILED\n"); }
        AK_free(fres);
    }
    {   int  a = 5, b = 0;
        test_num++; printf("Test %2d: AK_sql_div by zero (int) ... ", test_num);
        int *ires = AK_sql_div(&a, &b, TYPE_INT);
        if (ires && *ires == 0) { success++; printf("passed\n"); }
        else                     { failed++;  printf("FAILED\n"); }
        AK_free(ires);
    }

    {   int  a = 7, b = 5;
        test_num++; printf("Test %2d: AK_sql_mod (int)........ ", test_num);
        int *ires = AK_sql_mod(&a, &b, TYPE_INT);
        if (ires && *ires == 2) { success++; printf("passed\n"); }
        else                    { failed++;  printf("FAILED\n"); }
        AK_free(ires);
    }
    {   float a = 7.5f, b = 2.5f;
        test_num++; printf("Test %2d: AK_sql_mod (float)...... ", test_num);
        float *fres = AK_sql_mod(&a, &b, TYPE_FLOAT);
        if (fres && fabsf(*fres - 0.0f) < 1e-6f) { success++; printf("passed\n"); }
        else                                     { failed++;  printf("FAILED\n"); }
        AK_free(fres);
    }

    {   int  a = 2, b = 3;
        test_num++; printf("Test %2d: AK_sql_pow (int)........ ", test_num);
        int *ires = AK_sql_pow(&a, &b, TYPE_INT);
        if (ires && *ires == 8) { success++; printf("passed\n"); }
        else                    { failed++;  printf("FAILED\n"); }
        AK_free(ires);
    }
    {   float a = 2.0f, b = 3.0f;
        test_num++; printf("Test %2d: AK_sql_pow (float)...... ", test_num);
        float *fres = AK_sql_pow(&a, &b, TYPE_FLOAT);
        if (fres && fabsf(*fres - 8.0f) < 1e-6f) { success++; printf("passed\n"); }
        else                                     { failed++;  printf("FAILED\n"); }
        AK_free(fres);
    }

    {   int   v = -5;
        test_num++; printf("Test %2d: AK_sql_abs (int)........ ", test_num);
        int *ires = AK_sql_abs(&v, TYPE_INT);
        if (ires && *ires == 5) { success++; printf("passed\n"); }
        else                    { failed++;  printf("FAILED\n"); }
        AK_free(ires);
    }
    {   float v = -2.5f;
        test_num++; printf("Test %2d: AK_sql_abs (float)...... ", test_num);
        float *fres = AK_sql_abs(&v, TYPE_FLOAT);
        if (fres && fabsf(*fres - 2.5f) < 1e-6f) { success++; printf("passed\n"); }
        else                                     { failed++;  printf("FAILED\n"); }
        AK_free(fres);
    }

    {   float v = 2.3f;
        test_num++; printf("Test %2d: AK_sql_round (float).... ", test_num);
        int *ires = AK_sql_round(&v, TYPE_FLOAT);
        if (ires && *ires == 2) { success++; printf("passed\n"); }
        else                    { failed++;  printf("FAILED\n"); }
        AK_free(ires);
    }
    {   float v = 2.6f;
        test_num++; printf("Test %2d: AK_sql_round (float).... ", test_num);
        int *ires = AK_sql_round(&v, TYPE_FLOAT);
        if (ires && *ires == 3) { success++; printf("passed\n"); }
        else                    { failed++;  printf("FAILED\n"); }
        AK_free(ires);
    }

    {   float v =  9.0f;
        test_num++; printf("Test %2d: AK_sql_sqrt (float)..... ", test_num);
        float *fres = AK_sql_sqrt(&v, TYPE_FLOAT);
        if (fres && fabsf(*fres -  3.0f) < 1e-6f) { success++; printf("passed\n"); }
        else                                     { failed++;  printf("FAILED\n"); }
        AK_free(fres);
    }
    {   int   v = 16;
        test_num++; printf("Test %2d: AK_sql_sqrt (int)....... ", test_num);
        float *fres = AK_sql_sqrt(&v, TYPE_INT);
        if (fres && fabsf(*fres -  4.0f) < 1e-6f) { success++; printf("passed\n"); }
        else                                     { failed++;  printf("FAILED\n"); }
        AK_free(fres);
    }

    {   int   n = 3;
        test_num++; printf("Test %2d: AK_sql_left............. ", test_num);
        char *cres = AK_sql_left("abcdef", &n);
        if (cres && strcmp(cres, "abc") == 0) { success++; printf("passed\n"); }
        else                                  { failed++;  printf("FAILED\n"); }
        AK_free(cres);
    }
    {   test_num++; printf("Test %2d: AK_sql_len.............. ", test_num);
        int *l = AK_sql_len("AKDB");
        if (l && *l == 4) { success++; printf("passed\n"); }
        else              { failed++;  printf("FAILED\n"); }
        AK_free(l);
    }
    {   test_num++; printf("Test %2d: AK_sql_lower............ ", test_num);
        char *s = AK_sql_lower("TEST");
        if (s && strcmp(s, "test") == 0) { success++; printf("passed\n"); }
        else                             { failed++;  printf("FAILED\n"); }
        AK_free(s);
    }
    {   test_num++; printf("Test %2d: AK_sql_upper............ ", test_num);
        char *s = AK_sql_upper("test");
        if (s && strcmp(s, "TEST") == 0) { success++; printf("passed\n"); }
        else                             { failed++;  printf("FAILED\n"); }
        AK_free(s);
    }
    {   test_num++; printf("Test %2d: AK_sql_now.............. ", test_num);
        char *s = AK_sql_now();
        if (s ) { success++; printf("passed\n"); }
        else    { failed++;  printf("FAILED\n"); }
        AK_free(s);
    }
    {   test_num++; printf("Test %2d: AK_sql_replace.......... ", test_num);
        char *s = AK_sql_replace("banana", "na", "x");
        if (s && strcmp(s, "baxx") == 0) { success++; printf("passed\n"); }
        else                             { failed++;  printf("FAILED\n"); }
        AK_free(s);
    }
    {   int st = 1, le = 2;
        test_num++; printf("Test %2d: AK_sql_substr........... ", test_num);
        char *s = AK_sql_substr("Hello", &st, &le);
        if (s && strcmp(s, "el") == 0) { success++; printf("passed\n"); }
        else                            { failed++;  printf("FAILED\n"); }
        AK_free(s);
    }

    printf("\nAPI functions: %d passed, %d failed\n", success, failed);
    AK_EPI;
    return TEST_result(success, failed);
}
