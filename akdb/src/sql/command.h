/**
@file command.h Header file that provides data structures, functions and defines for command.c
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
#ifndef COMMAND
#define COMMAND

#include "../auxi/test.h"
#include "../file/table.h"
#include "../file/fileio.h"
#include "../rel/selection.h"
#include "../auxi/mempro.h"

struct AK_command_struct {
  int id_command;
  char *tblName;
  void *parameters;
};

typedef struct AK_command_struct command;

/** 
 * @author Mario Kolmacic updated by Ivan Pusic and Tomislav Ilisevic
 * @brief Function for executing given commands (SELECT, UPDATE, DELETE AND INSERT)
 * @param commands Commands array to execute
 * @param commandNum Number of commands in array
 * @return ERROR_EXIT only if command can't be executed returns EXIT_ERROR
 */
int AK_command(command * komande, int brojkomandi);
TestResult AK_test_command();

#endif

