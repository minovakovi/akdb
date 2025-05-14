#ifndef TESTS_H
#define TESTS_H

#include <stdio.h>
#include <unistd.h>
#include "auxi/mempro.h"
#include "auxi/test.h"

typedef struct Test
{
    char name[40];
    TestResult (*func)(void);
} Test;

extern Test tests[];
extern int tests_length;
void help();
void show_test();
void choose_test();
void set_catalog_constraints();
void run_all_tests();
void testing(void);

#endif // TESTS_H
