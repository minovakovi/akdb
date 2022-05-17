/**
@file dbman.c Defines functions for the disk manager
*/
/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
* dbman.c
* Copyright (C) Markus Schatten 2009 <markus.schatten@foi.hr>
*
* main.c is free software: you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* main.c is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//todo: We should have true/false values, one way is to include the boolean header but a more sensible
//      solution is to (#define false 0) and (#define true !false) in this header, or even better in the constants
//      header
#include "dbman.h"
#include "../mm/memoman.h"
pthread_mutex_t fileLockMutex = PTHREAD_MUTEX_INITIALIZER;


/**
* @author Markus Schatten
* @brief  Function that initializes a new database file named DB_FILE. It opens database file. New block is allocated. In this
block type of header is set to FREE_INT, attribute names are set to FREE_CHAR, integrities are set to FREE_INT,
constraint names are set to FREE_CHAR, constraint names and codes are set to FREE_CHAR. Type, address and size of tuples
* are set to FREE_INT. Data in block is set to FREE_CHAR. Type of block is BLOCK_TYPE_FREE, it is not chained and id of
* last tuple is 0.
* @param size size of new file in in blocks
* @return EXIT_SUCCESS if the file has been written to disk, EXIT_ERROR otherwise
*/
int
AK_init_db_file(int size)
{
    printf("\nInitialization\n");
    int sizeOfFile;
    AK_PRO;
    AK_dbg_messg(HIGH, DB_MAN, "AK_block: %i, AK_header: %i, AK_tuple_dict: %i , char: %i, int: %i\n",
		 sizeof(AK_block), sizeof(AK_header), sizeof(AK_tuple_dict), sizeof(char), sizeof(int));

    db_file_size = size;

    if ((db = fopen(DB_FILE, "rb+")) == NULL)
      {
        printf("AK_init_db_file: ERROR. Cannot open db file %s.\n", DB_FILE);
        AK_EPI;
        exit(EXIT_ERROR);
      }

    sizeOfFile = fsize(db);
    printf("AK_init_db_file: size db file %d. --- %d ---- %d\n", sizeOfFile, AK_ALLOCATION_TABLE_SIZE, AK_allocationbit->last_initialized);
    
    
    if (sizeOfFile > AK_ALLOCATION_TABLE_SIZE)
      {
        printf("AK_init_db_file: Already initialized.\n");
        fclose(db);
	db = NULL;
        AK_EPI;
        return (EXIT_SUCCESS);
      }

    printf("AK_init_db_file: Initializing DB file..."
	   "\nPlease be patient, this can take several minutes depending "
	   "on disk performance.\n");

    if(AK_allocate_blocks(db, AK_init_block(), 0, MAX_BLOCK_INIT_NUM) != EXIT_SUCCESS)
      {
        printf("AK_init_db_file: ERROR. Problem with blocks allocation %s.\n", DB_FILE);
        AK_EPI;
        exit(EXIT_ERROR);
      }

    AK_allocationbit->allocationtable[0] = 0;
    AK_allocationbit->last_allocated = 1;

    AK_blocktable_flush();  

    printf("AK_init_db_file: Done!\n");
    AK_EPI;
    return (EXIT_SUCCESS);
}


/**
 * @author dv
 * @param allocationSet Pointer to array which will be filled and represent the allocation set
 * @param fromWhere Has meaning only if mode is SEQUENCE. It describes from which address searching starts.
 * @param gaplength Tells how many used blocks can be tolerated in allocation set
 * @param numRequestedBlocks Tells how many AK_free blocks have been requested
 * @param mode Defines how to obtain set of indexes to AK_free addresses
 * @param target Has meaning just if mode is AROUND: set will be as close as possible to the requested target address
 * from both sides
 * @brief  Function prepare demanded sets from allocation table
 * @return the first element of the allocation set
 */
int
AK_get_allocation_set(int* allocationSet, int fromWhere, int gaplength, int numRequestedBlocks, AK_allocation_set_mode mode, int target)
{
  register int i = 0, j = 0, k = 0;
  int num_free_blocks_bitmap = 0,
      tmp = 0, tmpb = 0;
  int lastInitilizedBlock = AK_allocationbit->last_initialized;
  int freeBlocksMap[lastInitilizedBlock];
  AK_PRO;

  if (gaplength < 1)
    gaplength = 1;
 
  for (i = 0; i < numRequestedBlocks; i++)
    allocationSet[i] = FREE_INT;
  for (i = 0; i < lastInitilizedBlock; i++)
    freeBlocksMap[i] = FREE_INT;
  for (i = 0; i < lastInitilizedBlock; i++)
    if (!BITTEST(AK_allocationbit->bittable, i))
      freeBlocksMap[num_free_blocks_bitmap++] = i;

  if (num_free_blocks_bitmap < numRequestedBlocks)
    {
      AK_EPI;
      return allocationSet[0];
    }

  switch (mode)
    {
    case allocationSEQUENCE:
      if (fromWhere)
	{
	  if (numRequestedBlocks > (lastInitilizedBlock - AK_allocationbit->last_allocated))
	    {
	      AK_EPI;
	      return allocationSet[0];
	    }
	  for (i = 0; i < num_free_blocks_bitmap; ++i)
	    {
	      if (freeBlocksMap[i] == AK_allocationbit->last_allocated)
		break;
	    }
	}
      else
	i = 0;

      int hasEnoughSequentialBlocks = 0; //todo: set this to false; when #define true/false is included
      int startBlockIndex           = i; //todo: i is too vague, the whole function should use more clear variables.
      int freeBlockIndex;
      while( !hasEnoughSequentialBlocks &&
	     (startBlockIndex < num_free_blocks_bitmap - numRequestedBlocks + 1))
	{
	  hasEnoughSequentialBlocks = 1;
	  for (freeBlockIndex = startBlockIndex; freeBlockIndex < startBlockIndex + numRequestedBlocks; ++freeBlockIndex)
	    {
	      if(  (freeBlocksMap[freeBlockIndex] == (freeBlocksMap[freeBlockIndex + 1] - 1)) ||
		  ((freeBlocksMap[freeBlockIndex] == (freeBlocksMap[freeBlockIndex - 1] + 1))
		    &&
		   (freeBlockIndex  == (startBlockIndex + numRequestedBlocks - 1))) )
		continue;
	      
	      hasEnoughSequentialBlocks = 0; //todo: set this to false; when #define true/false is included
	      startBlockIndex = freeBlockIndex+1;
	      break;
	    }
	}
      if(hasEnoughSequentialBlocks)
	{
	  int setIndex = 0;
	  for (freeBlockIndex = startBlockIndex; freeBlockIndex < startBlockIndex + numRequestedBlocks; ++freeBlockIndex)
	    allocationSet[setIndex++] = freeBlocksMap[freeBlockIndex];
	}
      
      AK_EPI;
      return allocationSet[0];
	  
    case allocationUPPER:
      k = 0; j = 0; i = 0;
      tmp = freeBlocksMap[0];

      while (i < num_free_blocks_bitmap)
	{
	  if (freeBlocksMap[i] != FREE_INT && (gaplength >= abs(freeBlocksMap[i] - tmp)))
	    {
	      tmp = freeBlocksMap[i];
	      allocationSet[k++] = freeBlocksMap[i++];
	    }
	  else
	    i++;

	if (i == (num_free_blocks_bitmap - 1) || k == numRequestedBlocks)
	  break;

	if (k && (gaplength < abs(freeBlocksMap[i] - allocationSet[k - 1])))
	  {
	    k = 0;
	    for (j = 0; j < numRequestedBlocks; j++)allocationSet[j] = FREE_INT;
	    tmp = freeBlocksMap[i];
	  }
      }
      if (numRequestedBlocks != k)
	{
	  for (i = 0; i < numRequestedBlocks; i++)
	    allocationSet[i] = FREE_INT;
	  
	  AK_EPI;
	  return allocationSet[0];
	}
      
      AK_EPI;
      return allocationSet[0];
      
    case allocationLOWER:
      k = 0; j = 0;
      i = num_free_blocks_bitmap - 1;
      if (num_free_blocks_bitmap<numRequestedBlocks)
	{
	  AK_EPI;
	  return allocationSet[0];
	  break;
	}

      tmp = freeBlocksMap[i];

      while (i >= 0)
	{
	if (freeBlocksMap[i] != FREE_INT && (gaplength >= abs(freeBlocksMap[i] - tmp)))
	  {
	    tmp = freeBlocksMap[i];
	    allocationSet[k++] = freeBlocksMap[i--];
	  }
	else
	  i--;

	if (i == -1 || k == numRequestedBlocks)
	  break;

	if (k && (gaplength < abs(freeBlocksMap[i] - allocationSet[k - 1])))
	  {
	    k = 0;
	    for (j = 0; j < numRequestedBlocks; j++)
	      allocationSet[j] = FREE_INT;
	    tmp = freeBlocksMap[i];
	  }
	}
      if (numRequestedBlocks != k)
	{
	  for (i = 0; i < numRequestedBlocks; i++)
	    allocationSet[i] = FREE_INT;
	  AK_EPI;
	  return allocationSet[0];
	}

      AK_EPI;
      return allocationSet[0];
      
    case allocationAROUND:

      for (i = 0; i < num_free_blocks_bitmap; i++)
	if (freeBlocksMap[i] == target)
	  break;
      
      if ((i + 1) == num_free_blocks_bitmap)
	{
	  AK_EPI;
	  return allocationSet[0];
	}
      k = 0; j = i - 1;
      tmp  = freeBlocksMap[i];
      tmpb = freeBlocksMap[j];

      while (numRequestedBlocks)
	{
	if ((i < num_free_blocks_bitmap) && (freeBlocksMap[i] != FREE_INT) && (gaplength >= abs(freeBlocksMap[i] - tmp)))
	  {
	    allocationSet[k++] = freeBlocksMap[i];
	    numRequestedBlocks--;
	    tmp = freeBlocksMap[i];
	    i++;
	  }
	else
	  i = num_free_blocks_bitmap;

	if ((j >= 0) && (freeBlocksMap[j] != FREE_INT) && (gaplength >= abs(freeBlocksMap[j] - tmpb)))
	  {
	    allocationSet[k++] = freeBlocksMap[j];
	    numRequestedBlocks--;
	    tmpb = freeBlocksMap[j];
	    j--;
	  }
	else
	  j = -1;

	if ((i >= num_free_blocks_bitmap) && j<0 && numRequestedBlocks>0)
	  {
	    for (i = 0; i < numRequestedBlocks; i++)
	      allocationSet[i] = FREE_INT;
	    AK_EPI;
	    return allocationSet[0];
	  }
      }

      if (numRequestedBlocks != 0)
	{
	  for (i = 0; i < numRequestedBlocks; i++)
	    allocationSet[i] = FREE_INT;
	  AK_EPI;
	  return allocationSet[0];
	}
      
      AK_EPI;
      return allocationSet[0];
      
    case allocationNOMODE:
      ;
    }

  AK_EPI;
  return allocationSet[0];
}


/**
 * @author dv
 * @brief  Dumps the allocation table from the global allocation bit-vector onto standard output 
 * @param  verbosity level of verbosity (1 - minimal, 0 - no output)
 */
int AK_allocationtable_dump(int verbosity)
{
  int i;
  AK_PRO;
  printf("Dump of allocation table:\n\n");
  if (verbosity)
  {
      for(i = 0; i < DB_FILE_BLOCKS_NUM; i++)
	{
	  if(AK_allocationbit->allocationtable[i] != -1)
	    printf("(%5u) ", AK_allocationbit->allocationtable[i]);
	  else
	    printf("(%5d) ", 0);

	  if( ((i+1) % (CHAR_IN_LINE / 8) == 0) )
	    printf(" [%d]\n", i+1);
	  }
      printf("\n - - - - - -\n\n");
      return EXIT_SUCCESS;
      AK_EPI;
  }
  else
  {
    return EXIT_ERROR;
    AK_EPI;
  }
}


/**
 * @author dv
 * @brief  Dumps the bit-table from the global allocation bit-vector onto standard output 
 * @param  verbosity level of verbosity (1 - verbose, 0 - minimal)
 */
void
AK_blocktable_dump(int verbosity)
{
  int i;
  AK_PRO;
  printf("Dump of allocation table\n");
  if(verbosity)
    {
      for(i = 0; i < DB_FILE_BLOCKS_NUM; i++)
	{
	  if(AK_allocationbit->last_initialized < i)
	    printf("(%3d-U) ", i);
	  else if(BITTEST(AK_allocationbit->bittable, i))
	    printf("(%3d-X) ", i);
	  else
	    printf("(%3d-O) ", i);
	  
	  if(( (i+1) % (CHAR_IN_LINE / 8) == 0))
	    printf("\n");
        }
    }
  else
    {
      for(i = 0; i < DB_FILE_BLOCKS_NUM; i++)
	{
	  if(AK_allocationbit->last_initialized < i)
	    printf("U ");
	  else if (BITTEST(AK_allocationbit->bittable, i))
	    printf("X ");
	  else
	    printf("O ");
	  
	  if (( (i+1) % (CHAR_IN_LINE / 2) == 0))
	    printf("\n");
	}
    }

  printf("\n - - - - - -\n\n");
  AK_EPI;
}

/**
 * @author dv
 * @brief  Function flushes bitmask table to the disk
 * @return EXIT_SUCCESS if the file has been written to the disk, EXIT_ERROR otherwise
 */
int
AK_blocktable_flush()
{
  AK_PRO;
  if ((db = fopen(DB_FILE, "rb+")) == NULL)
    {
      printf("AK_allocationbit: ERROR. Cannot open db file %s.\n", DB_FILE);
      AK_EPI;
      exit(EXIT_ERROR);
    }

  pthread_mutex_lock(&fileLockMutex);
  if (AK_fwrite(AK_allocationbit, AK_ALLOCATION_TABLE_SIZE, 1, db) != 1)
    {
      printf("AK_allocationbit: ERROR. Cannot write bit vector \n");
      AK_EPI;
      exit(EXIT_ERROR);
    }
  pthread_mutex_unlock(&fileLockMutex);

  fclose(db);
  db = NULL;
  AK_EPI;
  
  return(EXIT_SUCCESS);
}



/**
 * @author Domagoj Šitum
 * @brief Allocation of an array which will contain information about which blocks are being accessed.
 * Creates an array. Each element of this array will correspond to one initialized block.
 * For more info, see explanation in dbman.h
 */
void
AK_allocate_block_activity_modes()
{
  int lastInitializedBlock = AK_allocationbit->last_initialized;
  AK_PRO;
    
  AK_free(AK_block_activity_info);
  if (lastInitializedBlock == 0)
    lastInitializedBlock = 1000;
    
  AK_block_activity_info = (AK_block_activity *) 
    AK_malloc((lastInitializedBlock + 1) * sizeof(AK_block_activity));

  int i;
  for (i = 0; i <= lastInitializedBlock; i++)
    {
      AK_block_activity_info[i].locked_for_reading = 0; // false
      AK_block_activity_info[i].locked_for_writing = 0; // false
      pthread_mutex_init(&AK_block_activity_info[i].block_lock,   NULL);
      pthread_cond_init (&AK_block_activity_info[i].reading_done, NULL);
      pthread_cond_init (&AK_block_activity_info[i].writing_done, NULL);
    }
  
  AK_EPI;
}

/**
 * @author dv
 * @brief  Function gets allocation table from the disk
 * @return EXIT_SUCCESS if the file has been taken from disk, EXIT_ERROR otherwise
 */
int
AK_blocktable_get()
{
  AK_PRO;
  if ((db = fopen(DB_FILE, "rb+")) == NULL)
    {
      printf("AK_allocationbit: ERROR. Cannot open db file %s.\n", DB_FILE);
      AK_EPI;
      exit(EXIT_ERROR);
    }

  pthread_mutex_lock(&fileLockMutex);
  if (AK_fread(AK_allocationbit, AK_ALLOCATION_TABLE_SIZE, 1, db) == 0)
    {
      printf("AK_allocationbit:  Cannot read bit-vector %d.\n", AK_ALLOCATION_TABLE_SIZE);
      AK_EPI;
      exit(EXIT_ERROR);
    }
  pthread_mutex_unlock(&fileLockMutex);
  
  fclose(db);
  db = NULL;

  AK_EPI;
  return (EXIT_SUCCESS);
}


/**
 * @brief  Helper function to determine file size
 * @return file size
 */
int
fsize(FILE *fp)
{
  AK_PRO;
  int sizeInBytes = 1;
    
  //todo: stat is POSIX, it will run into trouble on Windows presumably(?). Either way it is not recommended to use both definately.
  //      Also apparently SEEK_END isn't required to return anything meaningful. Consider unix and windows specific calls for filesize. 
#if 0
  struct stat stats;
  stat(DB_FILE, &stats);
  sizeInBytes = stats.st_size;
  if (sizeInBytes != 1)
    {
      AK_EPI;
      return sizeInBytes;
    }
#endif
    
  int prev = ftell(fp);
  fseek(fp, 0L, SEEK_END);
  sizeInBytes = ftell(fp);
  fseek(fp, prev, SEEK_SET);
  AK_EPI;
  return sizeInBytes;
}



/**
 * @author dv
 * @brief  Function that initializes the allocation table, writes it to the disk and caches it in memory
 * @return EXIT_SUCCESS if the file has been written to disk, EXIT_ERROR otherwise
 */
int
AK_init_allocation_table()
{
  int i, fileSizeBytes;
  AK_PRO;
  if ((AK_allocationbit = (AK_blocktable *)AK_malloc(sizeof(AK_blocktable))) == NULL)
    {
      printf("AK_allocationbit: ERROR. Cannot allocate  bit vector \n");
      AK_EPI;
      exit(EXIT_ERROR);
    }

  if ((db = fopen(DB_FILE, "rb+")) == NULL)
    {
      if ((db = fopen(DB_FILE, "wb+")) == NULL)
	{
	  printf("AK_allocationbit: ERROR. Cannot open db file %s.\n", DB_FILE);
	  AK_EPI;
	  exit(EXIT_ERROR);
	}
    }

  fileSizeBytes = fsize(db);

  pthread_mutex_lock(&fileLockMutex);
  if (fileSizeBytes == 0)
    {
      for (i = 0; i < DB_FILE_BLOCKS_NUM; i++)
	{
	  BITCLEAR(AK_allocationbit->bittable, i);
	  AK_allocationbit->allocationtable[i] = 0xFFFFFFFF;	    
	}
      AK_allocationbit->last_allocated   = 0;
      AK_allocationbit->last_initialized = 0;
      AK_allocationbit->prepared         = 0;
      AK_allocationbit->ltime            = time(NULL);

      if (AK_fwrite(AK_allocationbit, AK_ALLOCATION_TABLE_SIZE, 1, db) != 1)
	{
	  printf("AK_allocationbit: ERROR. Cannot write bit vector \n");
	  AK_EPI;
	  exit(EXIT_ERROR);
	}
      pthread_mutex_unlock(&fileLockMutex);
    }
  else if (AK_fread(AK_allocationbit, AK_ALLOCATION_TABLE_SIZE, 1, db) == 0)
    {
      printf("AK_allocationbit:  Cannot read bit-vector %d.\n", AK_ALLOCATION_TABLE_SIZE);
      AK_EPI;
      exit(EXIT_ERROR);
    }

  fclose(db);
  db = NULL;
  pthread_mutex_unlock(&fileLockMutex);

  AK_EPI;
  return (EXIT_SUCCESS);


}


/**
 * @author Markus Schatten , rearranged by dv
 * @brief  Function that initializes new block
 * @return pointer to block allocated in memory
 */
AK_block*
AK_init_block()
{
  register int i = 0, j, k;
  AK_block * block = NULL;
  AK_PRO;

  if ((block = (AK_block *)AK_malloc(sizeof (AK_block))) == NULL)
    {
      printf("AK_init_db_file: ERROR. Cannot allocate block %d\n", i);
      AK_EPI;
      exit(EXIT_ERROR);
    }

  for (i = 0; i < MAX_ATTRIBUTES; i++)
    {
      block->header[i].type = FREE_INT;
      for (j = 0; j < MAX_ATT_NAME; j++)
	{
	  block->header[i].att_name[j] = FREE_CHAR;
        }
      for (j = 0; j < MAX_CONSTRAINTS; j++)
	{
	  block->header[i].integrity[j] = FREE_INT;
	  for (k = 0; k < MAX_CONSTR_NAME; k++)
	    {
	      block->header[i].constr_name[j][k] = FREE_CHAR;
            }
	  for (k = 0; k < MAX_CONSTR_CODE; k++)
	    {
	      block->header[i].constr_code[j][k] = FREE_CHAR;
            }
        }
    }

  for (i = 0; i < DATA_BLOCK_SIZE; i++)
    {
      block->tuple_dict[i].type    = FREE_INT;
      block->tuple_dict[i].address = FREE_INT;
      block->tuple_dict[i].size    = FREE_INT;
    }

  for (i = 0; i < DATA_BLOCK_SIZE * DATA_ENTRY_SIZE; i++)
    {
      block->data[i] = FREE_CHAR;
    }

  block->type               = BLOCK_TYPE_FREE;
  block->chained_with       = NOT_CHAINED;
  block->AK_free_space      = DATA_BLOCK_SIZE * DATA_ENTRY_SIZE * sizeof (int);
  block->last_tuple_dict_id = 0;

  AK_EPI;
  return block;
}


/**
 * @author  dv
 * @brief  Function that dumps a block
 * @return nothing
 */
int
AK_print_block(AK_block * block, int num, char* gg, FILE *fpp)
{
  register int i = 0, j, k;
  int tmp = 0, tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0, kk = 0;
  struct timeval tv;
  struct timezone tz;
  struct tm *tm;
  AK_PRO;
  gettimeofday(&tv, &tz);
  tm = localtime(&tv.tv_sec);
  FILE *fp;

  if (fpp == NULL)
    {
      char log_name[60];

      sprintf(log_name, "log/%d%d%d%d%02d%02d%lu_%s.txt",
	      tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900,
	      tm->tm_hour, tm->tm_min, tm->tm_sec, tv.tv_usec,
	      gg);

      fp = fopen(log_name, "a+");

      if (fp == NULL)
	{
	  printf("Logging cannot open file %s\n", log_name);
	  AK_EPI;
	  exit(EXIT_ERROR);
        }
    }
  else
    fp = fpp;

  if (block == NULL)
    {

      if ((block = (AK_block *)AK_malloc(sizeof (AK_block))) == NULL)
	{
	  printf("AK_init_db_file: ERROR. Cannot allocate block %d\n", i);
	  AK_EPI;
	  exit(EXIT_ERROR);
        }

      block = AK_read_block(num);
      kk = 1;

    }

  fprintf(fp, "\n %d.%d.%d-%d:%02d:%02d.%lu \tcalling from:%s \n log address:%d \t memory address:%d \n\n", tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec, tv.tv_usec, gg, num, &block);


  if (block->type == BLOCK_TYPE_FREE)
    fprintf(fp, "block->type = BLOCK_TYPE_FREE\n");
  else fprintf(fp, "block->type = %d\n", block->type);

  if (block->chained_with == NOT_CHAINED)
    fprintf(fp, "block->chained_with = NOT_CHAINED\n");
  else fprintf(fp, "block->chained_with = %d\n", block->chained_with);

  fprintf(fp, "block->AK_free_space =%d\n", DATA_BLOCK_SIZE * DATA_ENTRY_SIZE * sizeof (int));
  fprintf(fp, "block->last_tuple_dict_id = %d\n", block->last_tuple_dict_id);


  fprintf(fp, "\n\n\t\tHEADER\n\n");
  for (i = 0; i < MAX_ATTRIBUTES; i++)
    {
      if (block->header[i].type == FREE_INT)
	{
	  if (tmp < 5)
	    fprintf(fp, "block->header[ %d ].type = FREE_INT;\n", i);
	  tmp++;
        }
      else
	{
	  if (tmp >= 5)fprintf(fp, "block->header[ ].type = Repeated %d times\n", tmp);
	  tmp = 0;
	  fprintf(fp, "block->header[ %d ].type = %d\n", i, block->header[i].type);
        }

      for (j = 0; j < MAX_ATT_NAME; j++)
	{
	  if (block->header[i].att_name[j] == FREE_CHAR)
	    {
	      if (tmp1 < 5)
		fprintf(fp, "block->header[ %d ].att_name[ %d ] = FREE_CHAR\n", i, j);
	      tmp1++;
            }
	  else
	    {
	      if (tmp1 >= 5)fprintf(fp, "header[].att_name[] = Repeated %d times\n", tmp1);
	      tmp1 = 0;
	      fprintf(fp, "block->header[ %d ].att_name[ %d ] = %c\n", i, j, block->header[i].att_name[j]);
            }
        }


      for (j = 0; j < MAX_CONSTRAINTS; j++)
	{

	  if (block->header[i].integrity[j] == FREE_INT)
	    {
	      if (tmp2 < 5)
		fprintf(fp, "block->header[ %d ].integrity[ %d ] = FREE_INT\n", i, j);
	      tmp2++;
            }
	  else
	    {
	      if (tmp2 >= 5)fprintf(fp, "block->header[].integrity[] = Repeated %d times\n", tmp2);
	      tmp2 = 0;
	      fprintf(fp, "block->header[ %d ].integrity[ %d ] = %d\n", i, j, block->header[i].integrity[j]);
            }


	  for (k = 0; k < MAX_CONSTR_NAME; k++)
	    {
	      if (block->header[i].constr_name[j][k] == FREE_CHAR)
		{
		  if (tmp3 < 5)
		    fprintf(fp, "block->header[ %d ].constr_name[%d][%d] = FREE_CHAR\n", i, j, k);
		  tmp3++;
                }
	      else
		{
		  if (tmp3 >= 5)fprintf(fp, "block->header[].constr_name[][]  = Repeated %d times\n", tmp3);
		  tmp3 = 0;
		  fprintf(fp, "block->header[%d].constr_name[%d][%d] = %c\n", i, j, k, block->header[i].constr_name[j][k]);
                }
            }

	  for (k = 0; k < MAX_CONSTR_CODE; k++)
	    {
	      if (block->header[i].constr_code[j][k] == FREE_CHAR)
		{
		  if (tmp4 < 5)
		    fprintf(fp, "block->header[ %d ].constr_code[%d][%d] = FREE_CHAR\n", i, j, k);
		  tmp4++;
                }
	      else
		{
		  if (tmp4 >= 5)fprintf(fp, "block->header[].constr_code[][]  = Repeated %d times\n", tmp4);
		  tmp4 = 0;
		  fprintf(fp, "block->header[%d].constr_code[%d][%d] = %c\n", i, j, k, block->header[i].constr_code[j][k]);
                }
            }
        }
    }
  if (tmp >= 4)fprintf(fp, "block->header[ ].type = Repeated %d times\n", tmp);
  if (tmp1 >= 4)fprintf(fp, "header[].att_name[] = Repeated %d times\n", tmp1);
  if (tmp2 >= 4)fprintf(fp, "block->header[].integrity[] = Repeated %d times\n", tmp2);
  if (tmp3 >= 4)fprintf(fp, "block->header[].constr_name[][]  = Repeated %d times\n", tmp3);
  if (tmp4 >= 4)fprintf(fp, "block->header[].constr_code[][]  = Repeated %d times\n", tmp4);

  tmp = 0;
  tmp1 = 0;
  tmp2 = 0;
  tmp3 = 0;
  tmp4 = 0;

  fprintf(fp, "\n\n\n\t\t tuple_dict\n\n");

  for (i = 0; i < DATA_BLOCK_SIZE; i++)
    {
      if (block->tuple_dict[i].type == FREE_INT)
	{
	  if (tmp < 5)
	    fprintf(fp, "block->tuple_dict[ %d ].type = FREE_INT\n", i);
	  tmp++;
        }
      else
	{
	  if (tmp >= 5)fprintf(fp, "block->tuple_dict[].type = Repeated %d times\n", tmp);
	  tmp = 0;
	  fprintf(fp, "block->tuple_dict[ %d ].type = %d\n", i, block->tuple_dict[i].type);
        }

      if (block->tuple_dict[i].address == FREE_INT)
	{
	  if (tmp1 < 5)
	    fprintf(fp, "block->tuple_dict[ %d ].address = FREE_INT\n", i);
	  tmp1++;
        }
      else
	{
	  if (tmp1 >= 5)fprintf(fp, "block->tuple_dict[].address = Repeated %d times\n", tmp1);
	  tmp1 = 0;
	  fprintf(fp, "block->tuple_dict[ %d ].address = %d\n", i, block->tuple_dict[i].address);
        }

      if (block->tuple_dict[i].size == FREE_INT)
	{
	  if (tmp2 < 5)
	    fprintf(fp, "block->tuple_dict[ %d ].size  = FREE_INT\n", i);
	  tmp2++;
        }
      else
	{
	  if (tmp2 >= 5)fprintf(fp, "block->tuple_dict[].size = Repeated %d times\n", tmp2);
	  tmp2 = 0;
	  fprintf(fp, "block->tuple_dict[ %d ].size  = %d\n", i, block->tuple_dict[i].size);
        }

    }

  if (tmp >= 4)fprintf(fp, "block->tuple_dict[].type = Repeated %d times\n", tmp);
  if (tmp1 >= 4)fprintf(fp, "block->tuple_dict[].address = Repeated %d times\n", tmp1);
  if (tmp2 >= 4)fprintf(fp, "block->tuple_dict[].size = Repeated %d times\n", tmp2);

  fprintf(fp, "\n\n\n\t\t data\n\n");

  for (i = 0; i < DATA_BLOCK_SIZE * DATA_ENTRY_SIZE; i++)
    {
      if (block->data[i] == FREE_CHAR)
	{
	  if (tmp3 < 5)
	    fprintf(fp, "block->data[ %d ]=FREE_CHAR\n", i);
	  tmp3++;
        }
      else
	{
	  if (tmp3 >= 5)fprintf(fp, "block->data[] = Repeated %d times\n", tmp3);
	  tmp3 = 0;
	  fprintf(fp, "block->data[ %d ]=%c\n", i, block->data[i]);
        }
    }
  if (tmp3 >= 5)fprintf(fp, "block->data[] = Repeated %d times\n", tmp3);

  if (kk)AK_free(block);

  if (fpp == NULL)
    {
      fclose(fp);
      db = NULL;
    }

  AK_EPI;
}

/**
* @author Markus Schatten , rearranged by dv
* @brief  Function that allocates new blocks by placing them to appropriate place
* and then updates the last initialized index
* @return EXIT_SUCCESS if the file has been written to disk, EXIT_ERROR otherwise
*/
int
AK_allocate_blocks(FILE* db, AK_block * block, int FromWhere, int HowMany)
{
  register int i = 0;
  AK_PRO;
  if (db == NULL)
    {
      if ((db = fopen(DB_FILE, "rb+")) == NULL)
	{
	  printf("AK_init_db_file: ERROR. Cannot open db file %s.\n", DB_FILE);
	  AK_EPI;
	  return EXIT_ERROR;
	} 
    }
    
    pthread_mutex_lock(&fileLockMutex);
    if (fseek(db, AK_ALLOCATION_TABLE_SIZE + FromWhere * sizeof(AK_block), SEEK_SET) != 0)
      {
	printf("AK_allocationbit: ERROR. Cannot set position to move for AK_blocktable \n");
	AK_EPI;
	return EXIT_ERROR;      
      }
    
    for (i = FromWhere; i < FromWhere + HowMany; i++)
      {
        block->address = i;
	
        if (AK_fwrite(block, sizeof (*block), 1, db) != 1)
	  {
	    printf("AK_init_db_file: ERROR. Cannot write block %d\n", i);
	    AK_EPI;
	    return EXIT_ERROR;
	  }
      }
    pthread_mutex_unlock(&fileLockMutex);

    fclose(db);
    db = NULL;
    
    AK_allocationbit->last_initialized = i;
    AK_allocate_block_activity_modes();
    AK_blocktable_flush();
    printf("AK_allocationbit->last_initialized %d\n", AK_allocationbit->last_initialized);
    AK_EPI;
    return (EXIT_SUCCESS);
}

/**
* @var test_lastCharacterWritten
* @brief This variable is used only when TEST_MODE is ON!
* It is used only for testing functionality of 
* AK_thread_safe_block_access_test() function.
* It will contain first character of last written block.
* When reading thread reads the block (written by some other thread),
* it will compare the first character from this block
* to character containted in this wariables.
* If they don't match, then the error occured!
* It is assumed that the same block is being written to
* and read from (just like AK_thread_safe_block_access_test
* function works!)
*/
char test_lastCharacterWritten = '\0';

/**
* @var test_threadSafeBlockAccessSucceeded
* @brief Used in combination with test_lastCharacterWritten.
* Will give the answer to question: 
* "Has AK_thread_safe_block_access_test suceeded?"
* 0 means NO, 1 means YES
*/
int test_threadSafeBlockAccessSucceeded = 1;

/**
 * @author Markus Schatten, updated by dv and Domagoj Šitum (thread-safe enabled)
 * @brief  Function that reads a block at a given address (block number less than db_file_size).
 * New block is allocated. Database file is opened. Position is set to provided address block.
 * At the end function reads file from that position. Completely thread-safe.
 * @param address block number (address)
 * @return pointer to block allocated in memory
 */
AK_block*
AK_read_block(int address)
{
  AK_PRO;
  //TODO: line 999 needs variable names
  int true = 1, false = 0;
  int locked_for_writing, locked_for_reading;
  int thread_id;
    
  if (DB_FILE_BLOCKS_NUM < address || 0 > address)
    {
      printf("AK_read_block: ERROR. Out of range %s  address:%d  DB_FILE_BLOCKS_NUM:%d\n", DB_FILE, address, DB_FILE_BLOCKS_NUM);
      AK_EPI;
      exit(EXIT_ERROR);
    }
    
  FILE * database;
  if ((database = fopen(DB_FILE, "rb")) == NULL)
    {
      printf("AK_read_block: ERROR. Cannot open db file %s.\n", DB_FILE);
      AK_EPI;
      exit(EXIT_ERROR);
    } 

  pthread_mutex_lock(&AK_block_activity_info[address].block_lock);
  // first we check if the block is already locked for writing by another thread
  locked_for_reading = AK_block_activity_info[address].locked_for_reading;
  locked_for_writing = AK_block_activity_info[address].locked_for_writing;
    
  if (!locked_for_reading && !locked_for_writing)
    {
      AK_block_activity_info[address].thread_holding_lock = &thread_id;
    }
    
  // if block is locked for writing, then we have to wait for another thread to unlock it
  //   (thus preventing it from accessing disk once again)
  // if another thread is only reading from this block, then we don't have to lock it's mutex, because
  // any number of threads can read the same block at the same time
  // else, we lock the block for reading and proceed    
  if (locked_for_writing == true)
    {
      pthread_cond_wait(&AK_block_activity_info[address].writing_done, &AK_block_activity_info[address].block_lock);
    }
  else
    {
      AK_block_activity_info[address].locked_for_reading = true;
    }
    
  // now we can safely write block to the disk
    
  // first we have to set position in file for reading new block
  if (fseek(database, address * sizeof(AK_block)+AK_ALLOCATION_TABLE_SIZE, SEEK_SET) != 0)
    {
      printf("AK_read_block: ERROR. Cannot set position to provided address block %d.\n", address);
      AK_EPI;
      exit(EXIT_ERROR);
    }
  //ERROR: a value of type "void *" cannot be used to initialize an entity of type "AK_block *"   
  AK_block * block = AK_malloc(sizeof(AK_block));

  // then we simply read block from the disk
  if (AK_fread(block, sizeof(AK_block), 1, database) == 0)
    {
      printf("AK_read_block: ERROR. Cannot read block %d.\n", address);
	  AK_free(block);
      AK_EPI;
      exit(EXIT_ERROR);
    }
    
  // block of code below is used only for testing purposes!
  // it is executed only when testMode is ON 
  // (It should be ON in AK_thread_safe_block_access_test function)
  // it takes first character of a block data and reads it
  // that character is then printed to the stdout. 
  // If everything goes well, we should get the same character that was written here
  // by last writing thread
  if (testMode == TEST_MODE_ON) {
    int FALSE = 0;
    if (test_lastCharacterWritten != '\0') {
      if (test_lastCharacterWritten != block->data[0]) {
	test_threadSafeBlockAccessSucceeded = FALSE;
      }
    }
  }
    
  // after reading is done, we unlock this block
  AK_block_activity_info[address].locked_for_reading = false;
  // and signalize other threads that reading is done!
  pthread_cond_signal(&AK_block_activity_info[address].reading_done);
    
  // and after everything is done, we just have to unlock block's mutex
  // thus making it accessible to other blocks which want to write data to it
  if (AK_block_activity_info[address].thread_holding_lock == &thread_id) {
    pthread_mutex_unlock(&AK_block_activity_info[address].block_lock);
  }
  fclose(database);
    
  AK_EPI;
  return block;
}

/**
* @author Markus Schatten, updated by Domagoj Šitum (thread-safe enabled)
* @brief  Function that writes a block to the DB file. Database file is opened. Position is set to provided address block. Block is
  written to provided address. Completely thread-safe.
* @param block poiner to block allocated in memory to write
* @return EXIT_SUCCESS if successful, EXIT_ERROR otherwise
*/
int
AK_write_block(AK_block * block)
{
  AK_PRO;
  //TODO: line 1107 needs variable names
  int true = 1, false = 0;
  int locked_for_reading = false, locked_for_writing = false, address;
  int thread_id;

  FILE * database;
  if ((database = fopen(DB_FILE, "rb+")) == NULL)
    {
      printf("AK_write_block: ERROR. Cannot open db file %s.\n", DB_FILE);
      AK_EPI;
      exit(EXIT_ERROR);
    }
    
  // first we have to find out block's address
  address = block->address;
    
  pthread_mutex_lock(&AK_block_activity_info[address].block_lock);
  // first we check if block is already locked for reading and for writing by another threads
  locked_for_writing = AK_block_activity_info[address].locked_for_writing;
  locked_for_reading = AK_block_activity_info[address].locked_for_reading;    
    
  if (!locked_for_reading && !locked_for_writing)
    {
      AK_block_activity_info[address].thread_holding_lock = &thread_id;
    }
    
  // if the block is locked for writing and/or is currently writing, then we have to wait for another thread to unlock it
  //   (thus preventing it from accessing disk once again)
  // if another thread is only reading from this block, then we don't have to lock it's mutex, because
  // any number of threads can read the same block at the same time
  // else, we lock the block for reading and proceed    
  if (locked_for_reading == true)
    {
      pthread_cond_wait(&AK_block_activity_info[address].reading_done, &AK_block_activity_info[address].block_lock);
    }
  if (locked_for_writing == true)
    {
      pthread_cond_wait(&AK_block_activity_info[address].writing_done, &AK_block_activity_info[address].block_lock);
    }
    
  // and when there is no other thread reading from it or writing to it, we can proceed
  AK_block_activity_info[address].locked_for_writing = true;
    
  // block of code below is used only for testing purposes!
  // it is executed only when testMode is ON 
  // (It should be ON in AK_thread_safe_block_access_test function)
  // it takes first character of a block data and replaces it with random ASCII character
  // than it writes it to the screen. Then, thread which is reading block can read this first character
  // and print it out. If everything goes well, we should get the same character that was written here.
  if (testMode == TEST_MODE_ON)
    {
      int character = rand() % 26 + 97;  // ascii code for letters a-z
      block->data[0] = (char)character;
      test_lastCharacterWritten = block->data[0];
    }
    
  // now we can safely write it to the disk

  // first we have to set position in file for new block writing
  if (fseek(database, block->address * sizeof(AK_block)+AK_ALLOCATION_TABLE_SIZE, SEEK_SET) != 0)
    {
      printf("AK_write_block: ERROR. Cannot set position to provided address block %d.\n", block->address);
      AK_EPI;
      exit(EXIT_ERROR);
    }

  // then we simply write block to the disk
  if (AK_fwrite(block, sizeof (*block), 1, database) != 1)
    {
      printf("AK_write_block: ERROR. Cannot write block at provided address %d.\n", block->address);
      AK_EPI;
      exit(EXIT_ERROR);
    }
        
  // after writing is done, we unlock this block for reading and/or writing
  AK_block_activity_info[address].locked_for_writing = false;
  // and signalize other threads that writing is done!
  pthread_cond_signal(&AK_block_activity_info[address].writing_done);
    
  // and after everything is done, we just have to unlock block's mutex
  // thus making it accessible to other blocks which want to write data to it or read from it
  if (AK_block_activity_info[address].thread_holding_lock == &thread_id)
    {
      pthread_mutex_unlock(&AK_block_activity_info[address].block_lock);
    }
    
  fclose(database);
    
  AK_EPI;
  return (EXIT_SUCCESS);
}




/**
 * @author Nikola Bakoš, updated by Dino Laktašić (fixed header BUG), refurbished by dv
 * @brief Function copy header to blocks. Completely thread-safe
 * @param header Pointer to header which will be copied into each block in blockSet
 * @param blockSet Pointer to array of block addresses into which to copy header
 * @param blockSetSize Number of blocks in blockSet
 * @return number of performed header copy
 */
int
AK_copy_header(AK_header *header, int* blockSet, int blockSetSize)
{
  int j = 0;
  int num_blocks = 0;
  int header_att_id = 0;
  AK_block *block;
    
  AK_PRO;
  for (j = 0; j < blockSetSize; j++)
    {
      block = AK_read_block(blockSet[j]);

      header_att_id = 0;
      //@TODO the check fails second time around if the table has MAX_ATTRIBUTES
      for( ;(header_att_id < MAX_ATTRIBUTES) && (header[header_att_id].type != 0); ++header_att_id)
	{
	  memcpy(&block->header[header_att_id], &header[header_att_id], sizeof(*header));
	}
      
      block->type = BLOCK_TYPE_NORMAL;
      block->AK_free_space = 0;
      block->last_tuple_dict_id = 0;
      
      if (AK_write_block(block) == EXIT_SUCCESS)
	{
	  num_blocks++;
	}
      
      AK_free(block);
    }
  
  AK_EPI;
  return num_blocks;
}


/**
* @author dv
* @brief Function that allocates new extent of blocks. Number of blocks is not ordered as well as a way of search for them.
* @param start_address address (block number) to start searching for sufficient space
* @param desired_size number of desired blocks
* @param AK_allocation_set_mode a way of trying to fing AK_free space. Can be one of:
allocationSEQUENCE,
allocationUPPER,
allocationLOWER,
allocationAROUND
* @param border number of allocated blocks gap
* @param target block address around which other blocks have to be searched
* @param header pointer to header that should be written to the new extent (all blocks)
* @param int gl gap size
* @return pointer to set of alocated block addresses
*/
int*
AK_get_extent(int start_address, int desired_size, AK_allocation_set_mode* mode, int border, int target, AK_header *header, int gl)
{
  register int i; /// vars for loop [for]
  int num_blocks = 0;
  int *blocknum, first_element_of_set;
  AK_PRO;
  blocknum = (int*)AK_malloc(sizeof(int)*(desired_size + 1));

  //todo: Replace this by an defined constant
  for (i = 0; i < 4; i++)
    {
      if (allocationNOMODE == mode[i])
	{
	  blocknum[0] = FREE_INT;
	  AK_EPI;
	  return blocknum;
	}
      //get set - addresses of allocated AK_free blocks
      first_element_of_set = AK_get_allocation_set(blocknum, border, gl, desired_size, mode[i], target);
      
      if (first_element_of_set == FREE_INT && mode[i] == allocationSEQUENCE)
	{
	  //there is no space at current boundaries - try to get more
	  if (AK_allocate_blocks(NULL, AK_init_block(), AK_allocationbit->last_initialized, desired_size) != EXIT_SUCCESS)
	    {
	      printf("AK_new_extent E1: ERROR. Problem with blocks allocation %s.\n", DB_FILE);
	      AK_EPI;
	      return blocknum;
	    }
	  
	  first_element_of_set = AK_get_allocation_set(blocknum, start_address, gl, desired_size, allocationSEQUENCE, 6);
	  if (first_element_of_set == FREE_INT)
	    {
	      printf("AK_new_extent E2: ERROR. Problem with blocks allocation %s.\n", DB_FILE);
	      AK_EPI;
	      return blocknum;
	    }
	  
	}
      if (blocknum[0] != FREE_INT)
	break;
    }
  
  if (blocknum[0] == FREE_INT)
    {
      AK_EPI;
      return blocknum;
    }

  num_blocks = AK_copy_header(header, blocknum, desired_size);

  AK_dbg_messg(HIGH, DB_MAN,
	       "AK_get_extent: first_address_of_extent= %i , num_alocated_blocks= %i , end_address= %i, num_blocks= %i\n",
	       blocknum[0], desired_size, blocknum[desired_size - 1], num_blocks);

  /// if some blocks are not succesfully allocated, which means that the extend allocation has FAILED
  if (num_blocks != desired_size)
    {
      AK_dbg_messg(LOW, DB_MAN, "AK_get_extent: ERROR. Cannot allocate extent %d\n", blocknum[0]);
      blocknum[0] = FREE_INT;
      AK_EPI;
      return blocknum;
    }

  //still haven't saved what happened to the allocation table
  for (i = 0; i < desired_size; i++)
    {
      BITSET(AK_allocationbit->bittable, blocknum[i]);
      if (i < (desired_size - 1))AK_allocationbit->allocationtable[blocknum[i]] = blocknum[i + 1];
    }
  AK_allocationbit->allocationtable[blocknum[i - 1]] = blocknum[0];
  AK_allocationbit->last_allocated += i;
  
  AK_blocktable_flush();
  //now we have

  AK_EPI;
  return blocknum;
}


/**
* @author dv
* @brief  Function that allocates a new blocks for increasing extent size.
* @param start_address first address of extent that is subject of increasing
* @param add_size number how many new blocks is to be added to existing extent
* @param AK_allocation_set_mode a way of trying to fing AK_free space. Can be one of:
allocationSEQUENCE,
allocationUPPER,
allocationLOWER,
allocationAROUND
* @param border number of allocated blocks gap
* @param target block address around which other blocks have to be searched
* @param header pointer to header that should be written to the new extent (all blocks)
* @param int gl gap size
* @return pointer to set of alocated block addresses
*/
int*
AK_increase_extent(int start_address, int add_size, AK_allocation_set_mode* mode, int border, int target, AK_header *header, int gl)
{
  register int i;
  unsigned int last_address = 0;
  int * blocknum;
  AK_PRO;
  
  blocknum = (int*)AK_malloc(sizeof(int)*(add_size + 1));
  blocknum[0] = FREE_INT;

  for (i = 0; i < DB_FILE_BLOCKS_NUM; i++)
    {
      if (AK_allocationbit->allocationtable[i] == start_address)
	{
	  last_address = AK_allocationbit->allocationtable[i];
	  break;
	}

    }

  //there is no start_address in extent's blocks allocation table
  if (!last_address)
    {
      AK_EPI;
      return blocknum;
    }


  //TODO maybe some situations will ask for start address. Zero covers all situations.
  blocknum = AK_get_extent(0, add_size, mode, border, target, header, gl);

  //problem with obtaining new blocks
  if (blocknum[0] == FREE_INT)
    {
      AK_EPI;
      return blocknum;
    }

  AK_allocationbit->allocationtable[last_address] = blocknum[0];
  for (i = 1; i < add_size; i++)
    {
      AK_allocationbit->allocationtable[blocknum[i - 1]] = blocknum[i];
    }
  AK_allocationbit->allocationtable[blocknum[add_size - 1]] = start_address;

  AK_EPI;
  return blocknum;
}

/**
* @author Nikola Bakoš, updated by Dino Laktašiæ (fixed header BUG), refurbished by dv
* @brief  Function that allocates new extent of blocks. If argument "old_size" is 0 than size of extent is INITIAL_EXTENT_SIZE.
* Otherwise, resize factor is set according to type of extent. If writing of block is successful, number of blocks is
*         incremented.
* @param start_address address (block number) to start searching for sufficient space
* @param old_size size of previous extent in same segment (in blocks)
* @param extent_type type of extent (can be one of:
SEGMENT_TYPE_SYSTEM_TABLE,
SEGMENT_TYPE_TABLE,
SEGMENT_TYPE_INDEX,
SEGMENT_TYPE_TRANSACTION,
SEGMENT_TYPE_TEMP
* @param header pointer to header that should be written to the new extent (all blocks)
* @return address (block number) of new extent if successful, EXIT_ERROR otherwise
*/
int
AK_new_extent(int start_address, int old_size, int extent_type, AK_header *header)
{
  int requested_space_in_blocks;
  register int i;
  int number_blocks_allocated = 0;
  int first_element_of_set, *allocation_set;
  AK_block* block;
  AK_PRO;

  if (old_size == 0)
    requested_space_in_blocks = INITIAL_EXTENT_SIZE;
  else
    {
      float scale_factor = 0; 
      switch (extent_type)
	{
	case SEGMENT_TYPE_TABLE:
	  scale_factor = EXTENT_GROWTH_TABLE;
	  break;
	case SEGMENT_TYPE_INDEX:
	  scale_factor = EXTENT_GROWTH_INDEX;
	  break;
	case SEGMENT_TYPE_TRANSACTION:
	  scale_factor = EXTENT_GROWTH_TRANSACTION;
	  break;
	case SEGMENT_TYPE_TEMP:
	  scale_factor = EXTENT_GROWTH_TEMP;
	  break;
	}

      requested_space_in_blocks = old_size + old_size*scale_factor;
    }

  allocation_set = (int*) AK_malloc(sizeof(int) * (requested_space_in_blocks + 1));

  first_element_of_set = AK_get_allocation_set(allocation_set, 1, 0, requested_space_in_blocks, allocationSEQUENCE, 6);
    
  if (first_element_of_set == FREE_INT)
  {
      	if (AK_allocate_blocks(NULL, block = AK_init_block(), AK_allocationbit->last_initialized, requested_space_in_blocks) != EXIT_SUCCESS)
		{
	  		AK_free(block);
	  		printf("AK_new_extent: ERROR. Problem with blocks allocation %s.\n", DB_FILE);
	  		AK_EPI;
	  		return(EXIT_ERROR);
		}
	  	AK_free(block);
      	first_element_of_set = AK_get_allocation_set(allocation_set, 1, 0, requested_space_in_blocks, allocationSEQUENCE, 6);
      	if (first_element_of_set == FREE_INT)
		{
	  		printf("AK_new_extent: ERROR. Problem with blocks allocation %s.\n", DB_FILE);
	  		AK_EPI;
	  		return(EXIT_ERROR);
		} 
  	}
  number_blocks_allocated = AK_copy_header(header, allocation_set, requested_space_in_blocks);

  printf("AK_new_extent: first_address_of_extent= %i , num_alocated_blocks= %i , end_address= %i, number_blocks_allocated= %i\n",
	 allocation_set[0], requested_space_in_blocks, allocation_set[requested_space_in_blocks - 1], number_blocks_allocated);
  AK_dbg_messg(HIGH, DB_MAN,
	       "AK_new_extent: first_address_of_extent= %i , num_alocated_blocks= %i , end_address= %i, number_blocks_allocated= %i\n",
	       allocation_set[0], requested_space_in_blocks, allocation_set[requested_space_in_blocks - 1], number_blocks_allocated);

  if (number_blocks_allocated != requested_space_in_blocks)
    {
      AK_dbg_messg(LOW, DB_MAN, "AK_new_extent: ERROR. Cannot allocate extent %d\n", allocation_set[0]);
      AK_EPI;
      return (EXIT_ERROR);
    }

  for (i = 0; i < requested_space_in_blocks; i++)
    {
      BITSET(AK_allocationbit->bittable, allocation_set[i]);
      if (i < (requested_space_in_blocks - 1))
	AK_allocationbit->allocationtable[allocation_set[i]] = allocation_set[i + 1];
    }
  
  AK_allocationbit->allocationtable[allocation_set[i - 1]] = allocation_set[0];
  AK_allocationbit->last_allocated += i;

  AK_blocktable_flush();
  AK_free(allocation_set);

  AK_EPI;
  return(first_element_of_set);
}


/**
* @author Tomislav Fotak, refurbished by dv
* @brief  Function that allocates new segment of extents. In this phase of implementation, only extents
containing INITIAL_EXTENT_SIZE blocks can be allocated. If extent is successfully allocated,
number of allocated extents is incremented and function goes to next block after allocated extent.
Otherwise, function moves to INITIAL_EXTENT_SIZE blocks. In that way function gets either first block of
new extent or some block in that extent which will not be AK_free.

* @param name (character pointer) name of segment
* @param type segment type (possible values:
SEGMENT_TYPE_SYSTEM_TABLE,
SEGMENT_TYPE_TABLE,
SEGMENT_TYPE_INDEX,
SEGMENT_TYPE_TRANSACTION,
SEGMENT_TYPE_TEMP)
* @param header (header pointer) pointer to header that should be written to the new extent (all blocks)
* @return EXIT_SUCCESS for success or EXIT_ERROR if some error occurs
*/
int
AK_new_segment(char * name, int type, AK_header *header)
{
  int segment_start_addr = 1; /// start address for segment because we can not allocate segment in block 0
  int first_allocated_block = -1;
  AK_PRO;

  AK_dbg_messg(HIGH, DB_MAN, "AK_new_segment: %d, %s.\n", type, name);

  first_allocated_block = AK_new_extent(segment_start_addr, 0, type, header);
  if (first_allocated_block == -1)
    {
      AK_EPI;
      return (EXIT_ERROR);
    }
  
  AK_EPI;
  return first_allocated_block;
}

/**
* @author Matija Novak
* @brief  Function that creates header and initalize integrity, constraint name and constraint
code with parameter values of function.

* @param name name of the atribute
* @param type type of the atribute
* @param integrity standard integrity costraint
* @param constr_name extra integrity constraint name
* @param contr_code extra integrity costraint code
* @return AK_header
*/
AK_header*
AK_create_header(char* attribute_name, int type, int integrity, char *constr_name, char *contr_code)
{
  AK_PRO;
  
  AK_header *catalog_header = (AK_header *)AK_malloc(sizeof (AK_header));
  memset(catalog_header, 0, sizeof (AK_header));

  //AK_archive_log("AK_create_header", name, type, integrity, constr_name, contr_code); //ARCHIVE_LOG
  AK_dbg_messg(HIGH, DB_MAN, "AK_create_header: Header: %s, %d\n", attribute_name, strlen(attribute_name));    

  catalog_header->type = type;

  int name_length = strlen(attribute_name);
  memcpy(catalog_header->att_name, attribute_name, name_length);

  register int i, j, k;

  // initialize catalog_header->integrity and catalog_header->constr_name[][] and catalog_header->constr_code[][] with
  //data given as functions parameters
  for (i = 0; i < MAX_CONSTRAINTS; i++)
    {
      catalog_header->integrity[i] = integrity;
      for (j = 0; j < MAX_CONSTR_NAME; j++)
	{
    //Error: a value of type "char *" cannot be assigned to an entity of type "char"
	  catalog_header->constr_name[i][j] = constr_name;
	}
      for (k = 0; k < MAX_CONSTR_CODE; k++)
	{
    //Error: a value of type "char *" cannot be assigned to an entity of type "char"
	  catalog_header->constr_code[i][k] = contr_code;
	}
    }
  
  AK_EPI;
  return catalog_header;
}

/**
* @author Matija Novak
* @brief  Function that inserts an entry in tuple_dict and data of a block. Address, type and size of
catalog_tuple_dict are set.  Free space of block is also set.
* @param block_adress adress of a block in which we want insert data
* @param type type of entry_data
* @param entry_data (char) data which is inserted, can be int but must first be converted to char
* @param i (int) adress in tuple_dict array (example block_address->tuple_dict[i])
* @return No return value because it gets the address of an block like a function parameter
and works directly with the orginal block
*/
void
AK_insert_entry(AK_block* block_address, int type, void* entry_data, int i)
{
  AK_tuple_dict* catalog_tuple_dict;
  AK_PRO;
  catalog_tuple_dict = (AK_tuple_dict *)AK_malloc(sizeof (AK_tuple_dict));

  AK_dbg_messg(HIGH, DB_MAN, "AK_insert_entry: Insert data: %d  Size of data:\n", *((int *)entry_data));

  /// copy data into bloc->data on start position bloc->AK_free_space
  if (type == TYPE_INT)
    {
      //Error:argument of type "void *" is incompatible with parameter of type "char *"
      memcpy(block_address->data + block_address->AK_free_space, entry_data, AK_type_size(type, entry_data));
      AK_dbg_messg(HIGH, DB_MAN, "AK_insert_entry: Insert data: %d  Size of data:\n", (int)entry_data);
    }
  else
    {
      //Error: argument of type "void *" is incompatible with parameter of type "char *"
      memcpy(block_address->data + block_address->AK_free_space, entry_data, AK_type_size(type, entry_data));
      AK_dbg_messg(HIGH, DB_MAN, "AK_insert_entry: Insert data: %s  Size of data:\n", (char *)entry_data);
    }
  /// address of entry data in block->data
  catalog_tuple_dict->address = block_address->AK_free_space;

  /// calculate next AK_free space for the next entry data
  //Error: argument of type "void *" is incompatible with parameter of type "char *"
  block_address->AK_free_space += AK_type_size(type, entry_data); ///sizeof(entry_data)+1);///(sizeof(int));
  /// no need for "+strlen(entry_data)" while "+1" is like "new line"

  /// type of entry data
  catalog_tuple_dict->type = type;
  /// size of entry data
  //Error: argument of type "void *" is incompatible with parameter of type "char *"
  catalog_tuple_dict->size = AK_type_size(type, entry_data);

  /// copy tuple_dict to block->tuple_dict[i]
  /// must use & becouse tuple_dict[i] is value and catalog_tuple_dict adress
  memcpy(&block_address->tuple_dict[i], catalog_tuple_dict, sizeof (*catalog_tuple_dict));
	AK_free(catalog_tuple_dict);
  AK_EPI;
}

/**
 * @author Matija Novak
 * @brief  Function that initialises the sytem table catalog and writes the result in first (0) block in db_file. Catalog block,
 catalog header name, catalog header address are allocated. Address, type, chained_with and AK_free_space attributes are
 initialized. Names of various database elements are written in block.
 * @param relation address of system table of relation in db_file
 * @param attribute address of system table of attribute in db_file
 * @param index address of system table of index in db_file
 * @param view address of system table of view in db_file
 * @param sequence address of system table of sequence in db_file
 * @param function address of system table of function in db_file
 * @param function_arguments address of system table of function_arguments in db_file
 * @param trigger address of system table of trigger in db_file
 * @param trigger_conditions address of system table of trigger_conditions in db_file
 * @param db address of system table of db in db_file
 * @param db_obj address of system table of db_obj in db_file
 * @param user address of system table of user in db_file
 * @param group address of system table of group in db_file
 * @param user_group address of system table of users associated with groups in db_file
 * @param user_right address of system table of user right in db_file
 * @param group_right address of system table of group right in db_file
 * @param constraint address of system table of constraint in db_file
 * @param constraintNull address of system table of constraintNull in db_file
 * @param constraintCheck system table address for check constraint
 * @param reference address of system table of reference in db_file
 * @return EXIT_SUCCESS if initialization was succesful if not returns EXIT_ERROR
 */
int
AK_init_system_tables_catalog(int relation, int attribute, int index, int view, int sequence, int function, int function_arguments,
			      int trigger, int trigger_conditions, int db, int db_obj, int user, int group, int user_group,
			      int user_right, int group_right, int constraint, int constraintNull, int constraintCheck,
			      int constraintUnique, int reference)
{
  AK_block*  catalog_block;
  AK_header* catalog_header_name;
  AK_header* catalog_header_address;
  AK_PRO;

  AK_dbg_messg(HIGH, DB_MAN, "AK_init_system_tables_catalog: Initializing system tables catalog\n");

  catalog_block = (AK_block *)AK_malloc(sizeof (AK_block));
  /// first header attribute of catalog_block
  catalog_header_name = (AK_header *)AK_malloc(sizeof (AK_header));
  catalog_header_name = AK_create_header("Name", TYPE_VARCHAR, FREE_INT, FREE_CHAR, FREE_CHAR);
  /// second attribute of catalog_block
  catalog_header_address = (AK_header *)AK_malloc(sizeof (AK_header));
  catalog_header_address = AK_create_header("Address", TYPE_INT, FREE_INT, FREE_CHAR, FREE_CHAR);

  /// initialize other elements of block (adress, type, chained_with, AK_free_space)
  catalog_block->address = 0;
  catalog_block->type = BLOCK_TYPE_NORMAL;
  catalog_block->chained_with = NOT_CHAINED;
  catalog_block->AK_free_space = 0; ///using as an address for the first AK_free space in block->data

  /// merge catalog_heder with heders created before
  memcpy(&catalog_block->header[0], catalog_header_name, sizeof (*catalog_header_name));
  memcpy(&catalog_block->header[1], catalog_header_address, sizeof (*catalog_header_address));

  // AK_tuple_dict tuple_dict[DATA_BLOCK_SIZE];
  // memcpy(catalog_block->tuple_dict, tuple_dict, sizeof (tuple_dict));

  int i = 0;
  //Error: argument of type "const char *" is incompatible with parameter of type "void *"
  AK_insert_entry(catalog_block, TYPE_VARCHAR, "AK_relation", i++);
  AK_insert_entry(catalog_block, TYPE_INT, &relation, i++);
  AK_insert_entry(catalog_block, TYPE_VARCHAR, "AK_attribute", i++);
  AK_insert_entry(catalog_block, TYPE_INT, &attribute, i++);
  AK_insert_entry(catalog_block, TYPE_VARCHAR, "AK_index", i++);
  AK_insert_entry(catalog_block, TYPE_INT, &index, i++);
  AK_insert_entry(catalog_block, TYPE_VARCHAR, "AK_view", i++);
  AK_insert_entry(catalog_block, TYPE_INT, &view, i++);
  AK_insert_entry(catalog_block, TYPE_VARCHAR, "AK_sequence", i++);
  AK_insert_entry(catalog_block, TYPE_INT, &sequence, i++);
  AK_insert_entry(catalog_block, TYPE_VARCHAR, "AK_function", i++);
  AK_insert_entry(catalog_block, TYPE_INT, &function, i++);
  AK_insert_entry(catalog_block, TYPE_VARCHAR, "AK_function_arguments", i++);
  AK_insert_entry(catalog_block, TYPE_INT, &function_arguments, i++);
  AK_insert_entry(catalog_block, TYPE_VARCHAR, "AK_trigger", i++);
  AK_insert_entry(catalog_block, TYPE_INT, &trigger, i++);
  AK_insert_entry(catalog_block, TYPE_VARCHAR, "AK_trigger_conditions", i++);
  AK_insert_entry(catalog_block, TYPE_INT, &trigger_conditions, i++);
  AK_insert_entry(catalog_block, TYPE_VARCHAR, "AK_db", i++);
  AK_insert_entry(catalog_block, TYPE_INT, &db, i++);
  AK_insert_entry(catalog_block, TYPE_VARCHAR, "AK_db_obj", i++);
  AK_insert_entry(catalog_block, TYPE_INT, &db_obj, i++);
  AK_insert_entry(catalog_block, TYPE_VARCHAR, "AK_user", i++);
  AK_insert_entry(catalog_block, TYPE_INT, &user, i++);
  AK_insert_entry(catalog_block, TYPE_VARCHAR, "AK_group", i++);
  AK_insert_entry(catalog_block, TYPE_INT, &group, i++);
  AK_insert_entry(catalog_block, TYPE_VARCHAR, "AK_user_group", i++);
  AK_insert_entry(catalog_block, TYPE_INT, &user_group, i++);
  AK_insert_entry(catalog_block, TYPE_VARCHAR, "AK_user_right", i++);
  AK_insert_entry(catalog_block, TYPE_INT, &user_right, i++);
  AK_insert_entry(catalog_block, TYPE_VARCHAR, "AK_group_right", i++);
  AK_insert_entry(catalog_block, TYPE_INT, &group_right, i++);
  AK_insert_entry(catalog_block, TYPE_VARCHAR, "AK_constraints_between", i++);
  AK_insert_entry(catalog_block, TYPE_INT, &constraint, i++);
  AK_insert_entry(catalog_block, TYPE_VARCHAR, "AK_constraints_not_null", i++);
  AK_insert_entry(catalog_block, TYPE_INT, &constraintNull, i++);
  AK_insert_entry(catalog_block, TYPE_VARCHAR, AK_CONSTRAINTS_CHECK_CONSTRAINT, i++);
  AK_insert_entry(catalog_block, TYPE_INT, &constraintCheck, i++);
  AK_insert_entry(catalog_block, TYPE_VARCHAR, "AK_constraints_unique", i++);
  AK_insert_entry(catalog_block, TYPE_INT, &constraintUnique, i++);
  AK_insert_entry(catalog_block, TYPE_VARCHAR, "AK_reference", i++);
  AK_insert_entry(catalog_block, TYPE_INT, &reference, i);

  catalog_block->last_tuple_dict_id = i;

  if (AK_write_block(catalog_block) == EXIT_SUCCESS)
    {
      AK_EPI;
      return EXIT_SUCCESS;
    }
  else
    {
      AK_EPI;
      return EXIT_ERROR;
    }
}

/**
* @author Miroslav Policki
* @brief  Function that sets the first num ints of a block of memory to the specified value
* @param block pointer to the block of memory to fill
* @param value int value to be set
* @param num number of ints in the block of memory to be set
* @return No return value
*/
void AK_memset_int(void *block, int value, size_t num) {
    size_t i;
    AK_PRO;
    for (i = 0; i < num; i++)
        *((int *)block + i) = value;
    AK_EPI;
}
/**
* @author Unknown
* @brief Function that registers system tables. Block at the given address is read. Various data from function arguments are written in block about different database elements.
* @param relation relation in database
* @param attribute attribute in databse
* @param index index in database
* @param view view in database
* @param sequence sequence in database
* @param function function in database
* @param function_arguments functional_arguments in databse
* @param trigger trigger in database
* @param trigger_conditions trigger conditions in databse
* @param db database
* @param db_obj database object
* @param user user in database
* @param group group in database
* @param user_group user associated with group in database
* @param user_right user right in database
* @param group_right group right in database
* @param constraint constraint in database
* @param constraintNull Null constraint in database
* @param constraintCheck Check constraint in database
* @param reference reference database
* @return EXIT_SUCCESS
*/
int
AK_register_system_tables(int relation, int attribute, int index, int view, int sequence, int function, int function_arguments,
			  int trigger, int trigger_conditions, int db, int db_obj, int user, int group, int user_group,
			  int user_right, int group_right, int constraint, int constraintNull, int constraintCheck,
			  int constraintUnique, int reference)
{
    AK_block *relationTable;
    int i = 1, j = 0;
    int end;
    AK_PRO;
    relationTable = AK_read_block(relation);
  
    AK_insert_entry(relationTable, TYPE_INT, &i, j++);
    //Error:argument of type "const char *" is incompatible with parameter of type "void *"
    AK_insert_entry(relationTable, TYPE_VARCHAR, "AK_relation", j++);
    AK_insert_entry(relationTable, TYPE_INT, &relation, j++);
    end = relation + INITIAL_EXTENT_SIZE;
    AK_insert_entry(relationTable, TYPE_INT, &end, j++);
    i++;

    AK_insert_entry(relationTable, TYPE_INT, &i, j++);
    //Error:argument of type "const char *" is incompatible with parameter of type "void *"
    AK_insert_entry(relationTable, TYPE_VARCHAR, "AK_attribute", j++);
    AK_insert_entry(relationTable, TYPE_INT, &attribute, j++);
    end = attribute + INITIAL_EXTENT_SIZE;
    AK_insert_entry(relationTable, TYPE_INT, &end, j++);
    i++;

    AK_insert_entry(relationTable, TYPE_INT, &i, j++);
    //Error:argument of type "const char *" is incompatible with parameter of type "void *"
    AK_insert_entry(relationTable, TYPE_VARCHAR, "AK_index", j++);
    AK_insert_entry(relationTable, TYPE_INT, &index, j++);
    end = index + INITIAL_EXTENT_SIZE;
    AK_insert_entry(relationTable, TYPE_INT, &end, j++);
    i++;

    AK_insert_entry(relationTable, TYPE_INT, &i, j++);
    //Error:argument of type "const char *" is incompatible with parameter of type "void *"
    AK_insert_entry(relationTable, TYPE_VARCHAR, "AK_view", j++);
    AK_insert_entry(relationTable, TYPE_INT, &view, j++);
    end = view + INITIAL_EXTENT_SIZE;
    AK_insert_entry(relationTable, TYPE_INT, &end, j++);
    i++;

    AK_insert_entry(relationTable, TYPE_INT, &i, j++);
    //Error:argument of type "const char *" is incompatible with parameter of type "void *"
    AK_insert_entry(relationTable, TYPE_VARCHAR, "AK_sequence", j++);
    AK_insert_entry(relationTable, TYPE_INT, &sequence, j++);
    end = sequence + INITIAL_EXTENT_SIZE;
    AK_insert_entry(relationTable, TYPE_INT, &end, j++);
    i++;

    AK_insert_entry(relationTable, TYPE_INT, &i, j++);
    //Error:argument of type "const char *" is incompatible with parameter of type "void *"
    AK_insert_entry(relationTable, TYPE_VARCHAR, "AK_function", j++);
    AK_insert_entry(relationTable, TYPE_INT, &function, j++);
    end = function + INITIAL_EXTENT_SIZE;
    AK_insert_entry(relationTable, TYPE_INT, &end, j++);
    i++;

    AK_insert_entry(relationTable, TYPE_INT, &i, j++);
    //Error:argument of type "const char *" is incompatible with parameter of type "void *"
    AK_insert_entry(relationTable, TYPE_VARCHAR, "AK_function_arguments", j++);
    AK_insert_entry(relationTable, TYPE_INT, &function_arguments, j++);
    end = function_arguments + INITIAL_EXTENT_SIZE;
    AK_insert_entry(relationTable, TYPE_INT, &end, j++);
    i++;

    AK_insert_entry(relationTable, TYPE_INT, &i, j++);
    //Error:argument of type "const char *" is incompatible with parameter of type "void *"
    AK_insert_entry(relationTable, TYPE_VARCHAR, "AK_trigger", j++);
    AK_insert_entry(relationTable, TYPE_INT, &trigger, j++);
    end = trigger + INITIAL_EXTENT_SIZE;
    AK_insert_entry(relationTable, TYPE_INT, &end, j++);
    i++;

    AK_insert_entry(relationTable, TYPE_INT, &i, j++);
    //Error:argument of type "const char *" is incompatible with parameter of type "void *"
    AK_insert_entry(relationTable, TYPE_VARCHAR, "AK_trigger_conditions", j++);
    AK_insert_entry(relationTable, TYPE_INT, &trigger_conditions, j++);
    end = trigger_conditions + INITIAL_EXTENT_SIZE;
    AK_insert_entry(relationTable, TYPE_INT, &end, j++);
    i++;

    AK_insert_entry(relationTable, TYPE_INT, &i, j++);
    //Error:argument of type "const char *" is incompatible with parameter of type "void *"
    AK_insert_entry(relationTable, TYPE_VARCHAR, "AK_db", j++);
    AK_insert_entry(relationTable, TYPE_INT, &db, j++);
    end = db + INITIAL_EXTENT_SIZE;
    AK_insert_entry(relationTable, TYPE_INT, &end, j++);
    i++;

    AK_insert_entry(relationTable, TYPE_INT, &i, j++);
    //Error:argument of type "const char *" is incompatible with parameter of type "void *"
    AK_insert_entry(relationTable, TYPE_VARCHAR, "AK_db_obj++", j++);
    AK_insert_entry(relationTable, TYPE_INT, &db_obj, j++);
    end = db_obj + INITIAL_EXTENT_SIZE;
    AK_insert_entry(relationTable, TYPE_INT, &end, j++);
    i++;

    AK_insert_entry(relationTable, TYPE_INT, &i, j++);
    //Error:argument of type "const char *" is incompatible with parameter of type "void *"
    AK_insert_entry(relationTable, TYPE_VARCHAR, "AK_user", j++);
    AK_insert_entry(relationTable, TYPE_INT, &user, j++);
    end = user + INITIAL_EXTENT_SIZE;
    AK_insert_entry(relationTable, TYPE_INT, &end, j++);
    i++;

    AK_insert_entry(relationTable, TYPE_INT, &i, j++);
    //Error:argument of type "const char *" is incompatible with parameter of type "void *"
    AK_insert_entry(relationTable, TYPE_VARCHAR, "AK_group", j++);
    AK_insert_entry(relationTable, TYPE_INT, &group, j++);
    end = group + INITIAL_EXTENT_SIZE;
    AK_insert_entry(relationTable, TYPE_INT, &end, j++);
    i++;

    AK_insert_entry(relationTable, TYPE_INT, &i, j++);
    //Error:argument of type "const char *" is incompatible with parameter of type "void *"
    AK_insert_entry(relationTable, TYPE_VARCHAR, "AK_user_group", j++);
    AK_insert_entry(relationTable, TYPE_INT, &user_group, j++);
    end = user_group + INITIAL_EXTENT_SIZE;
    AK_insert_entry(relationTable, TYPE_INT, &end, j++);
    i++;

    AK_insert_entry(relationTable, TYPE_INT, &i, j++);
    //Error:argument of type "const char *" is incompatible with parameter of type "void *"
    AK_insert_entry(relationTable, TYPE_VARCHAR, "AK_user_right", j++);
    AK_insert_entry(relationTable, TYPE_INT, &user_right, j++);
    end = user_right + INITIAL_EXTENT_SIZE;
    AK_insert_entry(relationTable, TYPE_INT, &end, j++);
    i++;

    AK_insert_entry(relationTable, TYPE_INT, &i, j++);
    //Error:argument of type "const char *" is incompatible with parameter of type "void *"
    AK_insert_entry(relationTable, TYPE_VARCHAR, "AK_group_right", j++);
    AK_insert_entry(relationTable, TYPE_INT, &group_right, j++);
    end = group_right + INITIAL_EXTENT_SIZE;
    AK_insert_entry(relationTable, TYPE_INT, &end, j++);
    i++;

    AK_insert_entry(relationTable, TYPE_INT, &i, j++);
    //Error:argument of type "const char *" is incompatible with parameter of type "void *"
    AK_insert_entry(relationTable, TYPE_VARCHAR, "AK_constraints_between", j++);
    AK_insert_entry(relationTable, TYPE_INT, &constraint, j++);
    end = constraint + INITIAL_EXTENT_SIZE;
    AK_insert_entry(relationTable, TYPE_INT, &end, j++);
    i++;

    AK_insert_entry(relationTable, TYPE_INT, &i, j++);
    //Error:argument of type "const char *" is incompatible with parameter of type "void *"
    AK_insert_entry(relationTable, TYPE_VARCHAR, "AK_constraints_not_null", j++);
    AK_insert_entry(relationTable, TYPE_INT, &constraintNull, j++);
    end = constraintNull + INITIAL_EXTENT_SIZE;
    AK_insert_entry(relationTable, TYPE_INT, &end, j++);
    i++;

    AK_insert_entry(relationTable, TYPE_INT, &i, j++);
    //Error:argument of type "const char *" is incompatible with parameter of type "void *"
    AK_insert_entry(relationTable, TYPE_VARCHAR, AK_CONSTRAINTS_CHECK_CONSTRAINT, j++);
    AK_insert_entry(relationTable, TYPE_INT, &constraintCheck, j++);
    end = constraintCheck + INITIAL_EXTENT_SIZE;
    AK_insert_entry(relationTable, TYPE_INT, &end, j++);
    i++;

    AK_insert_entry(relationTable, TYPE_INT, &i, j++);
    //Error:argument of type "const char *" is incompatible with parameter of type "void *"
    AK_insert_entry(relationTable, TYPE_VARCHAR, "AK_constraints_unique", j++);
    AK_insert_entry(relationTable, TYPE_INT, &constraintUnique, j++);
    end = constraintUnique + INITIAL_EXTENT_SIZE;
    AK_insert_entry(relationTable, TYPE_INT, &end, j++);
    i++;

    AK_insert_entry(relationTable, TYPE_INT, &i, j++);
    //Error:argument of type "const char *" is incompatible with parameter of type "void *"
    AK_insert_entry(relationTable, TYPE_VARCHAR, "AK_reference", j++);
    AK_insert_entry(relationTable, TYPE_INT, &reference, j++);
    end = reference + INITIAL_EXTENT_SIZE;
    AK_insert_entry(relationTable, TYPE_INT, &end, j);
    i++;

    relationTable->last_tuple_dict_id = j;

    AK_write_block(relationTable);
    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Miroslav Policki
 * @brief  Function that initializes the system catalog. Headers for system tables are defined. Segments for those system tables are
 allocated. Above function AK_register_system_tables() to register system tables.
 * @return EXIT_SUCCESS if the system catalog has been successfully initialized, EXIT_ERROR otherwise
 */
int
AK_init_system_catalog()
{
  int relation, attribute, index, view, sequence, function, function_arguments, trigger, trigger_conditions, db, db_obj,
    user, group, user_group, user_right, group_right, constraint, constraintNull, constraintCheck, constraintUnique, reference;
  int i;
  AK_PRO;
    
  AK_dbg_messg(HIGH, DB_MAN, "AK_init_system_catalog: System catalog initialization started...\n");

  AK_header hConstraintNotNull[5] =
    {
      { TYPE_INT,     "obj_id",    { 0 }, { { '\0' } }, { { '\0' } } },
      { TYPE_VARCHAR, "tableName", { 0 }, { { '\0' } }, { { '\0' } } },
      { TYPE_VARCHAR, "constraintName", 0, '\0', '\0' },
      { TYPE_VARCHAR, "attributeName", 0, '\0', '\0' },
      { 0, '\0', 0, '\0', '\0' }
    };

  AK_header hConstraintCheck[8] =
    {
      { TYPE_INT,     "obj_id", { 0 }, { { '\0' } }, { { '\0' } } },
      { TYPE_VARCHAR, "table_name", { 0 }, { { '\0' } }, { { '\0' } } },
      { TYPE_VARCHAR, "constraint_name", { 0 }, { { '\0' } }, { { '\0' } } },
      { TYPE_VARCHAR, "attribute_name", { 0 }, { { '\0' } }, { { '\0' } } },
      { TYPE_INT,     "constraint_value_type", { 0 }, { { '\0' } }, { { '\0' } } },
      { TYPE_VARCHAR, "constraint_condition", { 0 }, { { '\0' } }, { { '\0' } } },
      { TYPE_VARCHAR, "constraint_value", { 0 }, { { '\0' } }, { { '\0' } } },
      { 0, '\0', 0, '\0', '\0' }
    };

  AK_header hConstraintUnique[5] =
    {
      { TYPE_INT, "obj_id", { 0 }, { { '\0' } }, { { '\0' } } },
      { TYPE_VARCHAR, "tableName", 0, '\0', '\0' },
      { TYPE_VARCHAR, "constraintName", 0, '\0', '\0' },
      { TYPE_VARCHAR, "attributeName", 0, '\0', '\0' },
      { 0, '\0', 0, '\0', '\0' }
    };


  AK_header hConstraintBetween[7] =
    {
      { TYPE_INT, "obj_id", { 0 }, { { '\0' } }, { { '\0' } } },
      { TYPE_VARCHAR, "tableName", 0, '\0', '\0' },
      { TYPE_VARCHAR, "constraintName", 0, '\0', '\0' },
      { TYPE_VARCHAR, "attributeName", 0, '\0', '\0' },
      { TYPE_VARCHAR, "start_value", 0, '\0', '\0' },
      { TYPE_VARCHAR, "end_value", 0, '\0', '\0' },
      { 0, '\0', 0, '\0', '\0' }
    };


  AK_header hRelation[5] =
    {
      { TYPE_INT, "obj_id", { 0 }, { { '\0' } }, { { '\0' } } },
      { TYPE_VARCHAR, "name", 0, '\0', '\0' },
      { TYPE_INT, "start_address", 0, '\0', '\0' },
      { TYPE_INT, "end_address", 0, '\0', '\0' },
      { 0, '\0', 0, '\0', '\0' }
    };

  AK_header hAttribute[5] =
    {
      { TYPE_INT, "obj_id", { 0 }, { { '\0' } }, { { '\0' } } },
      { TYPE_VARCHAR, "name", 0, '\0', '\0', },
      { TYPE_INT, "type", 0, '\0', '\0', },
      { TYPE_INT, "table_id", 0, '\0', '\0', },
      { 0, '\0', 0, '\0', '\0' }
    };

  AK_header hIndex[7] =
    {
      { TYPE_INT, "obj_id", { 0 }, { { '\0' } }, { { '\0' } } },
      { TYPE_VARCHAR, "name", 0, '\0', '\0', },
      { TYPE_INT, "start_address", 0, '\0', '\0', },
      { TYPE_INT, "end_address", 0, '\0', '\0' },
      { TYPE_INT, "table_id", 0, '\0', '\0', },
      { TYPE_INT, "attribute_id", 0, '\0', '\0', },
      { 0, '\0', 0, '\0', '\0' }
    };

  AK_header hView[4] =
    {
      { TYPE_INT, "obj_id", { 0 }, { { '\0' } }, { { '\0' } } },
      { TYPE_VARCHAR, "name", 0, '\0', '\0', },
      { TYPE_VARCHAR, "query", 0, '\0', '\0', },
      { 0, '\0', 0, '\0', '\0' }
    };

  AK_header hSequence[8] =
    {
      { TYPE_INT, "obj_id", { 0 }, { { '\0' } }, { { '\0' } } },
      { TYPE_VARCHAR, "name", 0, '\0', '\0', },
      { TYPE_INT, "current_value", 0, '\0', '\0', },
      { TYPE_INT, "increment", 0, '\0', '\0', },
      { TYPE_INT, "max", 0, '\0', '\0', },
      { TYPE_INT, "min", 0, '\0', '\0', },
      { TYPE_BOOL, "cycle", 0, '\0', '\0', },
      { 0, '\0', 0, '\0', '\0' }
    };

  AK_header hFunction[5] =
    {
      { TYPE_INT, "obj_id", { 0 }, { { '\0' } }, { { '\0' } } },
      { TYPE_VARCHAR, "name", 0, '\0', '\0', },
      { TYPE_INT, "arg_num", 0, '\0', '\0', },
      { TYPE_INT, "return_type", 0, '\0', '\0', },
      { 0, '\0', 0, '\0', '\0' }
    };

  AK_header hFunction_arguments[5] =
    {
      { TYPE_INT, "func_id", 0, '\0', '\0', },
      { TYPE_INT, "att_num", 0, '\0', '\0', },
      { TYPE_VARCHAR, "att_type", 0, '\0', '\0', },
      { TYPE_VARCHAR, "att_name", 0, '\0', '\0', },
      { 0, '\0', 0, '\0', '\0' }
    };

  AK_header hTrigger[7] =
    {
      { TYPE_INT, "obj_id", { 0 }, { { '\0' } }, { { '\0' } } },
      { TYPE_VARCHAR, "name", 0, '\0', '\0', },
      { TYPE_VARCHAR, "event", 0, '\0', '\0', },
      { TYPE_VARCHAR, "condition", 0, '\0', '\0', },
      { TYPE_INT, "action", 0, '\0', '\0', },
      { TYPE_INT, "on", 0, '\0', '\0', },
      { 0, '\0', 0, '\0', '\0' }
    };

  AK_header hTrigger_conditions[5] =
    {
      { TYPE_INT, "trigger", 0, '\0', '\0', }, //pk
      { TYPE_INT, "id", 0, '\0', '\0', }, //pk
      { TYPE_VARCHAR, "data", 0, '\0', '\0', },
      { TYPE_INT, "type", 0, '\0', '\0', },
      { 0, '\0', 0, '\0', '\0' }
    };


  AK_header hDb[3] =
    {
      { TYPE_INT, "obj_id", { 0 }, { { '\0' } }, { { '\0' } } },
      { TYPE_VARCHAR, "name", 0, '\0', '\0', },
      { 0, '\0', 0, '\0', '\0' }
    };

  AK_header hDb_obj[3] =
    {
      { TYPE_INT, "db_id", 0, '\0', '\0', },
      { TYPE_INT, "artifact_id", 0, '\0', '\0', },
      { 0, '\0', 0, '\0', '\0' }
    };


  AK_header hUser[4] =
    {
      { TYPE_INT, "obj_id", { 0 }, { { '\0' } }, { { '\0' } } },
      { TYPE_VARCHAR, "username", 0, '\0', '\0', },
      { TYPE_INT, "password", 0, '\0', '\0', },
      { 0, '\0', 0, '\0', '\0' }
    };

  AK_header hGroup[3] =
    {
      { TYPE_INT, "obj_id", { 0 }, { { '\0' } }, { { '\0' } } },
      { TYPE_VARCHAR, "name", 0, '\0', '\0', },
      { 0, '\0', 0, '\0', '\0' }
    };

  AK_header hUserGroup[3] =
    {
      { TYPE_INT, "user_id", 0, '\0', '\0', },
      { TYPE_INT, "group_id", 0, '\0', '\0', },
      { 0, '\0', 0, '\0', '\0' }
    };

  AK_header hUserRight[5] =
    {
      { TYPE_INT, "obj_id", { 0 }, { { '\0' } }, { { '\0' } } },
      { TYPE_VARCHAR, "name", 0, '\0', '\0', },
      { TYPE_INT, "artifact_id", 0, '\0', '\0', },
      { TYPE_VARCHAR, "right_type", 0, '\0', '\0', },
      { 0, '\0', 0, '\0', '\0' }
    };

  AK_header hGroupRight[5] =
    {
      { TYPE_INT, "obj_id", { 0 }, { { '\0' } }, { { '\0' } } },
      { TYPE_INT, "group_id", 0, '\0', '\0', },
      { TYPE_INT, "artifact_id", 0, '\0', '\0', },
      { TYPE_VARCHAR, "right_type", 0, '\0', '\0', },
      { 0, '\0', 0, '\0', '\0' }
    };

  AK_header hReference[7] =
    {
      { TYPE_VARCHAR, "table", 0, '\0', '\0', },
      { TYPE_VARCHAR, "constraint", 0, '\0', '\0', },
      { TYPE_VARCHAR, "attribute", 0, '\0', '\0', },
      { TYPE_VARCHAR, "parent", 0, '\0', '\0', },
      { TYPE_VARCHAR, "parent_attribute", 0, '\0', '\0', },
      { TYPE_INT, "type", 0, '\0', '\0', },
      { 0, '\0', 0, '\0', '\0' }
    };

  for (i = 0; i < 4; i++) {
    AK_memset_int(hConstraintNotNull[i].integrity, FREE_INT, MAX_CONSTRAINTS);
    memset(hConstraintNotNull[i].constr_name, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_NAME);
    memset(hConstraintNotNull[i].constr_code, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_CODE);
  }

  for (i = 0; i < 8; i++) {
    AK_memset_int(hConstraintCheck[i].integrity, FREE_INT, MAX_CONSTRAINTS);
    memset(hConstraintCheck[i].constr_name, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_NAME);
    memset(hConstraintCheck[i].constr_code, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_CODE);
  }

  for (i = 0; i < 4; i++) {
    AK_memset_int(hConstraintUnique[i].integrity, FREE_INT, MAX_CONSTRAINTS);
    memset(hConstraintUnique[i].constr_name, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_NAME);
    memset(hConstraintUnique[i].constr_code, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_CODE);
  }

  for (i = 0; i < 6; i++) {
    AK_memset_int(hConstraintBetween[i].integrity, FREE_INT, MAX_CONSTRAINTS);
    memset(hConstraintBetween[i].constr_name, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_NAME);
    memset(hConstraintBetween[i].constr_code, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_CODE);
  }

  for (i = 0; i < 4; i++) {
    AK_memset_int(hRelation[i].integrity, FREE_INT, MAX_CONSTRAINTS);
    memset(hRelation[i].constr_name, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_NAME);
    memset(hRelation[i].constr_code, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_CODE);
  }

  for (i = 0; i < 4; i++) {
    AK_memset_int(hAttribute[i].integrity, FREE_INT, MAX_CONSTRAINTS);
    memset(hAttribute[i].constr_name, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_NAME);
    memset(hAttribute[i].constr_code, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_CODE);
  }

  for (i = 0; i < 6; i++) {
    AK_memset_int(hIndex[i].integrity, FREE_INT, MAX_CONSTRAINTS);
    memset(hIndex[i].constr_name, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_NAME);
    memset(hIndex[i].constr_code, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_CODE);
  }

  for (i = 0; i < 3; i++) {
    AK_memset_int(hView[i].integrity, FREE_INT, MAX_CONSTRAINTS);
    memset(hView[i].constr_name, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_NAME);
    memset(hView[i].constr_code, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_CODE);
  }

  for (i = 0; i < 7; i++) {
    AK_memset_int(hSequence[i].integrity, FREE_INT, MAX_CONSTRAINTS);
    memset(hSequence[i].constr_name, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_NAME);
    memset(hSequence[i].constr_code, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_CODE);
  }

  for (i = 0; i < 4; i++) {
    AK_memset_int(hFunction[i].integrity, FREE_INT, MAX_CONSTRAINTS);
    memset(hFunction[i].constr_name, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_NAME);
    memset(hFunction[i].constr_code, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_CODE);
  }

  for (i = 0; i < 4; i++) {
    AK_memset_int(hFunction_arguments[i].integrity, FREE_INT, MAX_CONSTRAINTS);
    memset(hFunction_arguments[i].constr_name, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_NAME);
    memset(hFunction_arguments[i].constr_code, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_CODE);
  }

  for (i = 0; i < 6; i++) {
    AK_memset_int(hTrigger[i].integrity, FREE_INT, MAX_CONSTRAINTS);
    memset(hTrigger[i].constr_name, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_NAME);
    memset(hTrigger[i].constr_code, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_CODE);
  }

  for (i = 0; i < 4; i++) {
    AK_memset_int(hTrigger_conditions[i].integrity, FREE_INT, MAX_CONSTRAINTS);
    memset(hTrigger_conditions[i].constr_name, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_NAME);
    memset(hTrigger_conditions[i].constr_code, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_CODE);
  }

  for (i = 0; i < 2; i++) {
    AK_memset_int(hDb[i].integrity, FREE_INT, MAX_CONSTRAINTS);
    memset(hDb[i].constr_name, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_NAME);
    memset(hDb[i].constr_code, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_CODE);
  }

  for (i = 0; i < 2; i++) {
    AK_memset_int(hDb_obj[i].integrity, FREE_INT, MAX_CONSTRAINTS);
    memset(hDb_obj[i].constr_name, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_NAME);
    memset(hDb_obj[i].constr_code, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_CODE);
  }

  for (i = 0; i < 3; i++) {
    AK_memset_int(hUser[i].integrity, FREE_INT, MAX_CONSTRAINTS);
    memset(hUser[i].constr_name, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_NAME);
    memset(hUser[i].constr_code, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_CODE);
  }

  for (i = 0; i < 2; i++) {
    AK_memset_int(hGroup[i].integrity, FREE_INT, MAX_CONSTRAINTS);
    memset(hGroup[i].constr_name, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_NAME);
    memset(hGroup[i].constr_code, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_CODE);
  }

  for (i = 0; i < 2; i++) {
    AK_memset_int(hUserGroup[i].integrity, FREE_INT, MAX_CONSTRAINTS);
    memset(hUserGroup[i].constr_name, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_NAME);
    memset(hUserGroup[i].constr_code, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_CODE);
  }

  for (i = 0; i < 4; i++) {
    AK_memset_int(hUserRight[i].integrity, FREE_INT, MAX_CONSTRAINTS);
    memset(hUserRight[i].constr_name, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_NAME);
    memset(hUserRight[i].constr_code, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_CODE);
  }

  for (i = 0; i < 4; i++) {
    AK_memset_int(hGroupRight[i].integrity, FREE_INT, MAX_CONSTRAINTS);
    memset(hGroupRight[i].constr_name, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_NAME);
    memset(hGroupRight[i].constr_code, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_CODE);
  }

  for (i = 0; i < 6; i++) {
    AK_memset_int(hReference[i].integrity, FREE_INT, MAX_CONSTRAINTS);
    memset(hReference[i].constr_name, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_NAME);
    memset(hReference[i].constr_code, FREE_CHAR, MAX_CONSTRAINTS * MAX_CONSTR_CODE);
  }

  AK_dbg_messg(HIGH, DB_MAN, "AK_init_system_catalog: Creating new segments...\n");


  relation           = AK_new_segment("AK_relation",    SEGMENT_TYPE_SYSTEM_TABLE, hRelation);
  attribute          = AK_new_segment("AK_attribute",   SEGMENT_TYPE_SYSTEM_TABLE, hAttribute);
  index              = AK_new_segment("AK_index",       SEGMENT_TYPE_SYSTEM_TABLE, hIndex);
  view               = AK_new_segment("AK_view",        SEGMENT_TYPE_SYSTEM_TABLE, hView);
  sequence           = AK_new_segment("AK_sequence",    SEGMENT_TYPE_SYSTEM_TABLE, hSequence);
  function           = AK_new_segment("AK_function",    SEGMENT_TYPE_SYSTEM_TABLE, hFunction);
  function_arguments = AK_new_segment("AK_function_arguments", SEGMENT_TYPE_SYSTEM_TABLE, hFunction_arguments);
  trigger            = AK_new_segment("AK_trigger",     SEGMENT_TYPE_SYSTEM_TABLE, hTrigger);
  trigger_conditions = AK_new_segment("AK_trigger_conditions", SEGMENT_TYPE_SYSTEM_TABLE, hTrigger_conditions);
  db                 = AK_new_segment("AK_db",          SEGMENT_TYPE_SYSTEM_TABLE, hDb);
  db_obj             = AK_new_segment("AK_db_obj",      SEGMENT_TYPE_SYSTEM_TABLE, hDb_obj);
  user               = AK_new_segment("AK_user",        SEGMENT_TYPE_SYSTEM_TABLE, hUser);
  group              = AK_new_segment("AK_group",       SEGMENT_TYPE_SYSTEM_TABLE, hGroup);
  user_group         = AK_new_segment("AK_user_group",  SEGMENT_TYPE_SYSTEM_TABLE, hUserGroup);
  user_right         = AK_new_segment("AK_user_right",  SEGMENT_TYPE_SYSTEM_TABLE, hUserRight);
  group_right        = AK_new_segment("AK_group_right", SEGMENT_TYPE_SYSTEM_TABLE, hGroupRight);
  constraint         = AK_new_segment("AK_constraints_between", SEGMENT_TYPE_SYSTEM_TABLE, hConstraintBetween);
  constraintNull     = AK_new_segment("AK_constraints_not_null", SEGMENT_TYPE_SYSTEM_TABLE, hConstraintNotNull);
  constraintCheck    = AK_new_segment(AK_CONSTRAINTS_CHECK_CONSTRAINT, SEGMENT_TYPE_SYSTEM_TABLE, hConstraintCheck);
  constraintUnique   = AK_new_segment("AK_constraints_unique", SEGMENT_TYPE_SYSTEM_TABLE, hConstraintUnique);
  reference          = AK_new_segment("AK_reference",   SEGMENT_TYPE_SYSTEM_TABLE, hReference);

  AK_dbg_messg(LOW, DB_MAN, "AK_init_system_catalog: Segments created!\n");

  if (EXIT_SUCCESS == AK_init_system_tables_catalog(relation, attribute, index, view, sequence, function, function_arguments,
						    trigger, trigger_conditions, db, db_obj, user, group, user_group, user_right,
						    group_right, constraint, constraintNull, constraintCheck, constraintUnique, reference))
    {
      AK_register_system_tables(relation, attribute, index, view, sequence, function, function_arguments, trigger, trigger_conditions,
				db, db_obj, user, group, user_group, user_right, group_right, constraint, constraintNull, constraintCheck,
				constraintUnique, reference);
      printf("AK_init_system_catalog: System catalog initialized!\n");
      AK_EPI;
      return EXIT_SUCCESS;
    }
  else
    {
      printf("AK_init_system_catalog: ERROR. AK_init_system_tables_catalog(%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d) \
                failed.\n",
	     relation, attribute, index, view, sequence, function, function_arguments, trigger, trigger_conditions, db, db_obj, user, group,
	     user_group, user_right, group_right);
      AK_EPI;
      return EXIT_ERROR;
    }
}

/**
* @author Markus Schatten
* @brief  Function that deletes a block by a given block address (resets the header and data). Types, integrities, constraint names,
constraint codes are set to "AK_free" values. In tuple dictionary type, address and size are set to FREE_INT values. Data of block is set to FREE_CHAR.
* @param address address of the block to be deleted
* @return returns EXIT_SUCCESS if deletion successful, else EXIT_ERROR
*/
int
AK_delete_block(int address)
{
    register int i, j, k;
    AK_header head[MAX_ATTRIBUTES];
    AK_tuple_dict tuple_dict[DATA_BLOCK_SIZE];
    unsigned char data[DATA_BLOCK_SIZE * DATA_ENTRY_SIZE];
    AK_PRO;

    for (i = 0; i < MAX_ATTRIBUTES; i++)
      {
        head[i].type = FREE_INT;
        for (j = 0; j < MAX_ATT_NAME; j++)
	  {
            head[i].att_name[j] = FREE_CHAR;
	  }
        for (j = 0; j < MAX_CONSTRAINTS; j++)
	  {
            head[i].integrity[j] = FREE_INT;
            for (k = 0; k < MAX_CONSTR_NAME; k++)
	      {
                head[i].constr_name[j][k] = FREE_CHAR;
	      }
            for (k = 0; k < MAX_CONSTR_CODE; k++)
	      {
                head[i].constr_code[j][k] = FREE_CHAR;
	      }
	  }
      }

    for (i = 0; i < DATA_BLOCK_SIZE; i++)
      {
        tuple_dict[i].type = FREE_INT;
        tuple_dict[i].address = FREE_INT;
        tuple_dict[i].size = FREE_INT;
      }
    for (i = 0; i < DATA_BLOCK_SIZE * DATA_ENTRY_SIZE; i++)
      {
        data[i] = FREE_CHAR;
      }

    AK_block * block = AK_read_block(address);
    block->type          = BLOCK_TYPE_FREE;
    block->chained_with  = NOT_CHAINED;
    block->AK_free_space = DATA_BLOCK_SIZE * DATA_ENTRY_SIZE * sizeof (int);
    block->last_tuple_dict_id = 0;
    memcpy(block->header, head, sizeof (*head));
    memcpy(block->tuple_dict, tuple_dict, sizeof (*tuple_dict));
    memcpy(block->data, data, sizeof (*data));

    BITCLEAR(AK_allocationbit->bittable, address);
    if (address == AK_allocationbit->last_allocated)
      AK_allocationbit->last_allocated = address - 1;
    AK_blocktable_flush();
    AK_blocktable_get();

    if (AK_write_block(block) == EXIT_SUCCESS)
      {
		/* added block deallocation, seems no more needed. All positive tests passed. For your convenvenience, I added comment here... Elvis Popovic, 10.05.2018.*/
		AK_free(block);
        AK_EPI;
        return EXIT_SUCCESS;
      }
	/* same here Elvis Popovic, 10.05.2018.*/
    AK_free(block);
    AK_EPI;
    return EXIT_ERROR;
}

/**
 * @author Dejan Sambolić
 * @brief  Function that deletes an extent between the first and the last block
 * @param begin address of extent's first block
 * @param end address of extent's last block
 * @return EXIT_SUCCESS if extent has been successfully deleted, EXIT_ERROR otherwise
 */
int
AK_delete_extent(int begin, int end)
{
  int address;
  AK_PRO;
  for (address = begin; address < end + 1; address++)
    {
      if (AK_delete_block(address) == EXIT_ERROR)
	{
	  AK_EPI;
	  return EXIT_ERROR;
        }
      AK_allocationbit->allocationtable[address] = 0xFFFFFFFF;
    }
  AK_EPI;
  return (EXIT_SUCCESS);
}

/**
 * @author Mislav Èakariæ, fixed by Josip Susnjara
 * @param name name of the segment
 * @param type type of the segment
 * @return EXIT_SUCCESS if extent has been successfully deleted, EXIT_ERROR otherwise
 */
int
AK_delete_segment(char * name, int type)
{
  int i = 0;
  table_addresses *addresses;
  AK_PRO;

  addresses = (table_addresses*)AK_get_segment_addresses(name);
  for (;addresses->address_from[i] != 0; ++i)
    {
      if (AK_delete_extent(addresses->address_from[i], addresses->address_to[i] - 1) == EXIT_ERROR)
	{
	  AK_EPI;
	  return EXIT_ERROR;
        }
    }
	
  struct list_node* row_root = (struct list_node*) AK_malloc(sizeof(struct list_node));
  AK_Init_L3(&row_root);
  
  char *system_table;
  switch (type)
    {
    case SEGMENT_TYPE_TABLE:
      system_table = "AK_relation";
      break;
    case SEGMENT_TYPE_INDEX:
      system_table = "AK_index";
      break;
    case SEGMENT_TYPE_TRANSACTION:
      //todo
      printf("Not implemented yet!\n");
      AK_EPI;
      return EXIT_ERROR;
      break;
    case SEGMENT_TYPE_TEMP:
      //todo
      printf("Not implemented yet!\n");
      AK_EPI;
      return EXIT_ERROR;
      break;
    default:
      AK_EPI;
      return EXIT_ERROR;
    }
  
  AK_DeleteAll_L3(&row_root);
  AK_Update_Existing_Element(TYPE_VARCHAR, name, system_table, "name", row_root);
  AK_delete_row(row_root);
  AK_free(row_root);

  AK_EPI;
  return EXIT_SUCCESS;
}

/**
 * @author Markus Schatten
 * @return Function that calls functions AK_init_db_file() and AK_init_system_catalog() to initialize disk manager.
 * It also calls AK_allocate_array_currently_accessed_blocks() to allocate memory needed for thread-safe reading
 * and writing to disk. 
 */
int
AK_init_disk_manager()
{
  int size = DB_FILE_BLOCKS_NUM; //1024 * 1024 * size_in_mb / sizeof ( AK_block);
  AK_PRO;
  if (AK_init_allocation_table() == EXIT_ERROR)
    {
      AK_EPI;
      exit(EXIT_ERROR);
    }
    
  AK_allocate_block_activity_modes();

  if (AK_allocationbit->prepared == 31)
    {
      printf("\n\tDisk manager has been initialized at %s\n\n", asctime(localtime(&AK_allocationbit->ltime)));
      AK_dbg_messg(LOW, DB_MAN, "Block size is: %d\n", sizeof (AK_block));
      AK_dbg_messg(LOW, DB_MAN, "%d blocks for %d MiB\n", size, DB_FILE_SIZE);
	
      AK_EPI;
      return EXIT_SUCCESS;
    }

  printf("AK_init_disk_manager: Initializing disk manager...\n\n");
  AK_dbg_messg(LOW, DB_MAN, "AK_init_disk_manager: Block size is: %d\n", sizeof (AK_block));
  AK_dbg_messg(LOW, DB_MAN, "AK_init_disk_manager: We need %d blocks for %d MiB\n", size, DB_FILE_SIZE);

  if (AK_init_db_file(size) == EXIT_SUCCESS)
    {
      if (AK_init_system_catalog() == EXIT_SUCCESS)
	{
	  printf("AK_init_disk_manager: Disk manager initialized!\n\n");
	  AK_allocationbit->prepared = 31;
	  AK_allocationbit->ltime = time(NULL);
	  AK_blocktable_flush();
	  AK_EPI;
	  return EXIT_SUCCESS;
	}
      printf("AK_init_disk_manager: ERROR. System catalog initialization failed!\n");
      AK_EPI;
      return EXIT_ERROR;
    }

  printf("AK_init_disk_manager: ERROR. DB file initialization failed!");

  AK_EPI;
  return EXIT_ERROR;
}


TestResult AK_allocationbit_test()
{
	//adding test code
	int i, success, failed;
	int bitNo;
    AK_PRO;
    AK_blocktable_dump(1);
	
	//for test of test itself
	//BITCLEAR(AK_allocationbit->bittable,AK_allocationbit->last_allocated-1);
	
	printf("Last allocated bit: %d\n",AK_allocationbit->last_allocated);
	printf("Last initialized bit: %d\n",AK_allocationbit->last_initialized);
	printf("Prepared: %d\n",AK_allocationbit->prepared);
	
	i=success=failed=0;
	if(BITTEST(AK_allocationbit->bittable, i))
	{
		failed++;
		printf("First bit has wrong avalue.\n",i);
	}
	else
		success++;
	success++;
	bitNo=1;
	for(i=1; i < AK_allocationbit->last_allocated; i++,bitNo++)
		if(!BITTEST(AK_allocationbit->bittable, i))
		{
			failed++;
			success--;
			printf("Bit %d has wrong value in allocated area.\n",i);
			break;
		}
	success++;
	bitNo=0;
	for(i=AK_allocationbit->last_allocated; i<DB_FILE_BLOCKS_NUM; i++,bitNo++)
	{
		if(BITTEST(AK_allocationbit->bittable, i))
		{
			failed++;
			success--;
			printf("Bit %d has wrong value out of allocated area.\n");
			break;
		}
	}
	printf("\n");
	if(failed==0)
		printf("All bit values O.K.\n");
	
    AK_EPI;
    return TEST_result(success,failed);
}

TestResult AK_allocationtable_test()
{
    AK_PRO;
    int success=0;
    int failed=0;
    int result=AK_allocationtable_dump(1);
    AK_EPI;
    if(result==EXIT_ERROR)
    {
      failed++;
    }
    else
    {
      success++;
    }
    
    return TEST_result(success,failed);
}

/**
 * @author Domagoj Šitum
 * @brief This function tests thread safe reading and writing to blocks.
 * There is N writing and N reading threads, which are going through iterations. 
 * Each reading thread should read
 * the data (character) that was set by last writing thread
 */
TestResult AK_thread_safe_block_access_test()
{
  int i, j, sum_of_suceeded_tests = 0;
  int block_address = 0;
  //Error: expected an identifier
  int true = 1, false = 0;
  AK_block *backup_block = (AK_block *) AK_malloc(sizeof(AK_block));
  AK_block *block;
  pthread_t *threads;
  AK_PRO;
    
  srand(time(NULL));
    
  printf("N reading threads + N writing threads are trying to read/write "
	 "to the same block. Result is printed to the screen.\n\n");
    
  // first we have to read original value of the first block
  // (so that we can save it somewhere and restore it after the whole operation is over)
  block = AK_read_block(block_address);
    
  // we have to backup that first block, because we will make changes to it
  // (explanation of the text above)
  memcpy((void *)backup_block, (void *)block, sizeof(AK_block));
    
  for (j=1; j<=50; j++)
    {
      printf("%d+%d threads: ", j, j);
        
      // we read first block of actual data (after allocation bit-vector)
      block = AK_read_block(block_address);
        
      testMode = TEST_MODE_ON;
        
      // then we create j reading and j writing threads
      threads = (pthread_t *) AK_malloc(2 * j * sizeof(pthread_t));

      // and send them to read and write to the same block (block with index 0)
      for (i = 0; i < j; i++)
	{
	  pthread_create(&threads[2*i], NULL, AK_write_block_for_testing, (void *)block);
	  pthread_create(&threads[2*i+1], NULL, AK_read_block_for_testing, (void *)&block_address);
        }
        
      // and then, we just have to wait all threads to finish
      for (i = 0; i < 2*j; i++) 
	pthread_join(threads[i],NULL);
        
      testMode = TEST_MODE_OFF;
        
      AK_free((void*)threads);
        
      // and at the end of each iteration,
      // we check if thread safe block access (reading/writing) succeeded.
      // if it failed, we count it
      if (test_threadSafeBlockAccessSucceeded == true)
	{
	  printf("Success\n");
	  sum_of_suceeded_tests++;
        }
      else
	{
	  printf("Failed\n");
        }
    
      test_lastCharacterWritten = '\0';
      test_threadSafeBlockAccessSucceeded = true;
    }
    
  // and at the end, we write backup block back to the file
  AK_write_block(backup_block);
  AK_free((void*)backup_block);
    
  printf("\n%d out of 50 tests succeeded.", sum_of_suceeded_tests);
    
  AK_EPI;
  return TEST_result(sum_of_suceeded_tests,50-sum_of_suceeded_tests);
}


/**
* @author Domagoj Šitum
* @brief This function is only for testing. It has to be there, because
* pthread_create only accepts void* function_name (void *) function format.
* So AK_read_block is no-go for pthread_create.
*/
void* AK_read_block_for_testing(void *address)
{
    AK_PRO;
    int adr = *(int*)address;
    AK_read_block(adr);

    AK_EPI;
    return 0;
}

/**
* @author Domagoj Šitum
* @brief This function is only for testing. It has to be there, because
* pthread_create only accepts void* function_name (void *) function format.
* So AK_write_block is no-go for pthread_create.
*/
void* AK_write_block_for_testing(void *block)
{
    AK_PRO;
    AK_block *blk = (AK_block *)block;
    AK_write_block(blk);

    AK_EPI;
    return 0;
}
