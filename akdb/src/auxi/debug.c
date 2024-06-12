/**
@file debug.c Provides a function for debuging
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
 
#include "debug.h"
#include "constants.h"
#include <stdarg.h>
#include <stdio.h>

#define DEFAULT_FORMAT "[%s][%s]: %s\n"

/**
 * @author Dino Laktašić
 * @brief Function that prints the debug message. Provides debug level, debug type and message with corresponding variables for the output. 
 * @param level level of debug information for a given DB module
 * @param type the name of DB module for which to print debug information
 * @param format format for the output message
 * @param ... variable number of (different) type args used in printf
 * @return if debug message is printed return 1, else return 0
 */
int AK_dbg_messg(DEBUG_LEVEL level, DEBUG_TYPE type, const char *format, ...) {
	/*if (DEBUG_NONE) {
		return 0;
	}*/
	AK_PRO;
	
	if ((level == DEBUG_LEVEL_0 || type == DEBUG_TYPE_0 || format == NULL) && !DEBUG_ALL) {
		AK_EPI;
		return 0;
	}
	
	va_list args;
	va_start(args, format);
	char message_buffer[MAX_DEBUG_MESSAGE_LENGTH];
	vsnprintf(message_buffer, MAX_DEBUG_MESSAGE_LENGTH, format, args);
	va_end(args);

	char final_buffer[MAX_DEBUG_MESSAGE_LENGTH];
	sprintf(final_buffer, MAX_DEBUG_MESSAGE_LENGTH, DEFAULT_FORMAT, debug_level_to_string(level), debug_type_to_string(type), message_buffer);
	printf("%s", final_buffer);
	//Dodana opcionalna logika za ispis razine i tipa debuga.
	//printf("[%s][%s]: %s\n", debug_level_to_string(level), debug_type_to_string(type), buffer);

	AK_EPI;
	return 1;
}


// Helper function to convert debug level to string
const char* debug_level_to_string(DEBUG_LEVEL level){
	switch (level)
	{
		case DEBUG_LEVEL_0: return "DEBUG_LEVEL_0";
		default: return "UNKNOWN_DEBUG_LEVEL";
		break;
	}
}

// Helper function to convert debug type to string
const char* debug_type_to_string(DEBUG_TYPE type){
	switch (type)
	{
		case DEBUG_TYPE_0: return "DEBUG_TYPE_0";
		default: return "UNKNOWN_DEBUG_TYPE";
		break;
	}
}