/**
@file debug.h Header file that defines global macros, constants and variables for debuging
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

#ifndef DEBUG
#define DEBUG

#include "stdarg.h"
#include "stdio.h"
#include "stdlib.h"
#include "mempro.h"

/**
 * @brief Set constant to 1 for a complete project debug, else set constant to 0
 * @author Dino Laktašić
 */
#define DEBUG_ALL 0
//#define DEBUG_NONE 0


/**
 * @author Dino Laktašić
 * @struct DEBUG_LEVEL
 * @brief Structure for setting debug level. Divide debug information according to their importance. 
 *         More levels can be defined in the enum if needed. Each debug level can be easily excluded 
 *         from output by setting corresponding enum element to 0.
 */
typedef enum debug_level {
	LOW = 1,	//BASIC		low level of details in output
	MIDDLE = 0,	//NORMAL	normal level of details in output
	HIGH = 0	//ADVANCED (EXHAUSTIVE)	high level of details in output (details in additional functions, per loop or per condition debugging)
} DEBUG_LEVEL;

/**
 * @author Dino Laktašić
 * @struct DEBUG_TYPE
 * @brief Structure for setting debug type. Divide debug information according to their type (e.g. DB modules). 
 *         More modules can be aditional added to the enum. Each debug type can be easly excluded from output 
 *         by setting corresponding enum element to 0
 * 
 */
typedef enum debug_type {
	GLOBAL = 0,
	DB_MAN = 0,
	FILE_MAN = 1,
	MEMO_MAN = 0,
	INDICES = 0,
	TABLES = 0,
	REL_OP = 0,
	REL_EQ = 1,
	CONSTRAINTS = 0,
	FUNCTIONS = 0,
	SEQUENCES = 0,
	TRIGGERS = 0,
	REDO = 0
} DEBUG_TYPE;


int AK_dbg_messg(DEBUG_LEVEL level, DEBUG_TYPE type, const char *format, ...);

#endif
