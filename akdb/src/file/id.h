/**
@file id.h Provides functions and defines for creating id of objects
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

#ifndef ID
#define ID

#include "../auxi/test.h"
#include "table.h"
#include "fileio.h"
#include "../auxi/mempro.h"

/**
 * @def ID_START_VALUE
 * @brief Constant declaring start value of id 
*/
#define ID_START_VALUE 100

/**
 * @author Saša Vukšić, updated by Mislav Čakarić, changed by Mario Peroković, now uses AK_update_row, updated by Nenad Makar
 * @brief Function that fetches unique ID for any object, stored in a sequence
 * @return objectID
 */
int AK_get_id();
TestResult AK_id_test();

#endif
