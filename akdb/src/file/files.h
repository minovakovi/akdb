/**
@file files.h Header file that provides functions and defines for file management
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

#ifndef FILES
#define FILES

#include "../auxi/test.h"
#include "id.h"
#include "../auxi/mempro.h"


int AK_initialize_new_segment(char *name, int type, AK_header *header);
int AK_initialize_new_index_segment(char *name, char *table_id,int attr_id , AK_header *header);

TestResult AK_files_test();

#endif



