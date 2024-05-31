/**
@file archive_log.h Header file that provides functions and defines for archive logging
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

#ifndef ARCHIVE
#define ARCHIVE

#include "../file/table.h"
#include "sys/time.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../auxi/mempro.h"

/**
 * @brief  Function for making archive log.
 * @author Dražen Bandić, update by Tomislav Turek
 * @return No retun value
 */
void AK_archive_log(int sig);

/**
 * @author Dražen Bandić main logic, replaced by Tomislav Turek
 * @brief Function that returns the current timestamp 
 * @return char array in format day.month.year-hour:min:sec.usecu.bin
 */
char* AK_get_timestamp();

#endif


