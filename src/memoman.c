/**
@file memoman.c Defines functions for the memory manager of
 Kalashnikov DB
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

#include <stdio.h>
#include "configuration.h"
#include "auxiliary.h"
#include "dbman.h"
#include "memoman.h"


/**
 @author Markus Schatten

 Initializes the global cache memory (variable db_cache)

 @return EXIT_SUCCESS if the cache memory has been initialized, EXIT_ERROR otherwise
*/
int KK_cache_malloc()
{
	register int i;

	if( ( db_cache = ( KK_db_cache * ) malloc ( sizeof( KK_db_cache ) ) ) == NULL )
	{
		exit( EXIT_ERROR );
	}
	db_cache->next_replace = MAX_CACHE_MEMORY - 1;
	for( i = 0; i < MAX_CACHE_MEMORY; i++ )
	{
		db_cache->cache[ i ] = ( KK_mem_block * ) malloc ( sizeof( KK_mem_block ) );
		if( ( KK_cache_block( i, db_cache->cache[ i ] ) ) == EXIT_ERROR )
		{
			exit( EXIT_ERROR );
		}
		//printf( "Cached block %d with address %d\n", i,  &db_cache->cache[ i ]->block->address );
	}

	return EXIT_SUCCESS;
}

/**
 @author Dejan Sambolić

 Initializes the global redo log memory (variable redo_log)

 @return EXIT_SUCCESS if the redo log memory has been initialized, EXIT_ERROR otherwise
*/
int KK_redo_log_malloc()
{
	if( redo_log = ( KK_redo_log * ) malloc ( sizeof( KK_redo_log ) ) == NULL)
	{
		exit( EXIT_ERROR );
	}
	return EXIT_SUCCESS;
}

/**
 @author Matija Novak

 Initializes the global query memory (variable query_mem)

 @return EXIT_SUCCESS if the query memory has been initialized, EXIT_ERROR otherwise
*/
int KK_query_mem_malloc()
{
	if( DEBUG )
		printf("KK_query_mem_malloc: Start query_mem_malloc");
	/// allocate memory for global variable query_mem
	if( (query_mem = ( KK_query_mem * ) malloc ( sizeof( KK_query_mem) ) ) == NULL )
	{
		printf( "KK_query_mem_malloc: ERROR. Cannot allocate query memory \n");
		exit( EXIT_ERROR );
	}

	/// allocate memory for variable query_mem_lib which is used in query_mem->parsed
	KK_query_mem_lib * query_mem_lib;
	if(( query_mem_lib = ( KK_query_mem_lib *) malloc(sizeof(KK_query_mem_lib)) ) == NULL )
	{
		printf( "KK_query_mem_malloc: ERROR. Cannot allocate query library memory \n");
		exit( EXIT_ERROR );
	}

	/// allocate memory for variable query_mem_dict which is used in query_mem->dictionary
	KK_query_mem_dict * query_mem_dict;
	if( (query_mem_dict = ( KK_query_mem_dict *) malloc(sizeof(KK_query_mem_dict)) ) == NULL )
	{
		printf( "KK_query_mem_malloc: ERROR. Cannot allocate query dictionary memory \n");
		exit( EXIT_ERROR );
	}

	/// allocate memory for variable query_mem_result which is used in query_mem->result
	KK_query_mem_result * query_mem_result;
	if(( query_mem_result = (KK_query_mem_result *) malloc(sizeof(KK_query_mem_result)) ) == NULL )
	{
		printf( "  KK_query_mem_malloc: ERROR. Cannot allocate query result memory \n");
		exit( EXIT_ERROR );
	}

	/// allocate memory for variable tuple_dict which is used in query_mem->dictionary->dictionary[]
	KK_tuple_dict * tuple_dict = (KK_tuple_dict *) malloc(sizeof(KK_tuple_dict));
	if(( tuple_dict = (KK_tuple_dict *) malloc(sizeof(KK_tuple_dict)) ) == NULL )
	{
		printf( "  KK_query_mem_malloc: ERROR. Cannot allocate tuple dictionary memory \n");
		exit( EXIT_ERROR );
	}


	memcpy(query_mem_dict->dictionary,tuple_dict,sizeof(* tuple_dict));

	query_mem->parsed = query_mem_lib;
	query_mem->dictionary = query_mem_dict;
	query_mem->result = query_mem_result;

/*	wrong way becouse we don't have data only adress which must be written in query_mem variables
	memcpy(query_mem->parsed, query_mem_lib, sizeof(* query_mem_lib));
	memcpy(query_mem->dictionary,query_mem_dict,sizeof(* query_mem_dict));
	memcpy(query_mem->result,query_mem_result,sizeof(* query_mem_result));*/

	if( DEBUG )
		printf("KK_query_mem_malloc: Success!");
	return EXIT_SUCCESS;
}

/**
 @author Miroslav Policki

 Initializes memory manager (cache, redo log and query memory)

 @return EXIT_SUCCESS if the query memory manager has been initialized, EXIT_ERROR otherwise
*/
int KK_memoman_init()
{
	printf( "KK_memoman_init: Initializing memory manager...\n\n" );

	if(KK_cache_malloc() == EXIT_ERROR)
	{
		printf( "KK_memoman_init: ERROR. KK_cache_malloc() failed.\n");
		return EXIT_ERROR;
	}

	if(KK_redo_log_malloc() == EXIT_ERROR)
	{
		printf( "KK_memoman_init: ERROR. KK_redo_log_malloc() failed.\n");
		return EXIT_ERROR;
	}

	if(KK_query_mem_malloc() == EXIT_ERROR) {
		printf( "KK_memoman_init: ERROR. KK_query_mem_malloc() failed.\n");
		return EXIT_ERROR;
	}

	printf( "KK_memoman_init: Memory manager initialized...\n\n" );

	return ( EXIT_SUCCESS );
}

/**
 @author Nikola Bakoš

 Caches block into memory.

 @param num block number (address)
 @param mem_block address of memmory block

 @return EXIT_SUCCESS if the block has been successfully read into memory, EXIT_ERROR otherwise
*/
int KK_cache_block( int num, KK_mem_block * mem_block )
{
	KK_block * block_cache;
	int timestamp;

	/// allocation of KK_block
	/*if( ( block_cache = ( KK_block * ) malloc ( sizeof( KK_block ) ) ) == NULL )
	{
		printf( " KK_cache_block: ERROR. Cannot allocate block memory \n");
		return ( EXIT_ERROR );
	}*/

	/// read the block from the given address
	block_cache = (KK_block *) KK_read_block( num );

	mem_block->block = block_cache; /// copy block to given mem_block
	mem_block->dirty = BLOCK_CLEAN; /// set dirty bit in mem_block struct

	timestamp = clock();  /// get the timestamp
	mem_block->timestamp_read = timestamp; /// set timestamp_read
	mem_block->timestamp_last_change = timestamp; /// set timestamp_last_change

	//free( block_cache );

	return (EXIT_SUCCESS); /// if all is succesfull
}


/**
 @author Tomislav Fotak

 Reads a block from memory. If the block is cached returns the cached block. Else uses
 KK_cache_block to read the block to cache and then returns it.

 @param num block number (address)

 @return segment start address
*/
KK_mem_block * KK_get_block( int num )
{
	int i = 0; /// counter
	int found_in_cache = 0;
	int first_free_mem_block = -1;
	int oldest_block = db_cache->next_replace;

	int get_second_oldest = 0; /// if block will be removed from cache, then second oldest should be
								/// marked in db_cache as the one that will be replaced next
	KK_mem_block *cached_block; /// cached memory block
	KK_block *data_block;
	int block_written;
	i = 0;

	while ( i < MAX_CACHE_MEMORY )
	{
	    if(DEBUG)
            printf( "i is now %d\n", i );
		/// if block is cached returns block
		if ( db_cache->cache[ i ]->block->address == num )
		{
			cached_block =  db_cache->cache[i];
			found_in_cache = 1;
			if(DEBUG)
                printf( "Found block %d in cache\n", i );
		}

        if(DEBUG)
            printf( "Passed first if-block\n" );

		/// get first free memory block for possible block caching
		/// checking by timestamp of last block reading
		if ( first_free_mem_block == -1 )
		{
			/// assume that free block has timestamp_read set to -1
			if ( db_cache->cache[i]->timestamp_read == -1 )
				first_free_mem_block = i;
		}

		/// get second oldest block index in db_cache
		if ( ( db_cache->cache[i]->timestamp_read > db_cache->cache[ oldest_block ]->timestamp_read ) &&
			( db_cache->cache[ i ]->timestamp_read < db_cache->cache[ get_second_oldest ]->timestamp_read ) )
			get_second_oldest = i;

		i++;
	}

	if ( !found_in_cache )
	{
		if ( first_free_mem_block != -1 )
		{
			/// assume that function KK_cahce_block returns (KK_mem_block*)
			/// and ih takes two arguments (block_number, allocation_address)
			/// cached_block = (KK_mem_block*) KK_cache_block ( num, db_cache.cache[first_free_mem_block] );

			/// if KK_cahce_block returns INT
			if ( KK_cache_block (num, db_cache->cache[ first_free_mem_block ] ) == EXIT_SUCCESS )
				cached_block = db_cache->cache[first_free_mem_block];

		}
		else
		{
			if ( db_cache->cache[oldest_block]->dirty == BLOCK_DIRTY )
			{
				data_block = (KK_block *) db_cache->cache[oldest_block]->block;
				block_written = KK_write_block ( data_block );
				/// if block form cache can not be writed to DB file -> EXIT_ERROR
				if ( block_written != EXIT_SUCCESS )
				{
					printf("KK_read_block: ERROR! Cannot write block from cache to DB File.\n");
					exit ( EXIT_ERROR );
				}
				/// if block is written to DB file, set next_replace to second oldest
				db_cache->next_replace = get_second_oldest;
			}
			/// cached_block = (KK_mem_block*) KK_cache_block ( num, db_cache.cache[oldest_block] );

			if ( KK_cache_block (num, db_cache->cache[ first_free_mem_block ] ) == EXIT_SUCCESS )
				cached_block = db_cache->cache[ oldest_block ];
		}
	}
	return ( cached_block );
}


/**
 @author Nikola Bakoš

 Extents the segment
 @param table_name name of segment to extent
 @param extent_type type of extent (can be one of:
	SEGMENT_TYPE_SYSTEM_TABLE,
	SEGMENT_TYPE_TABLE,
	SEGMENT_TYPE_INDEX,
	SEGMENT_TYPE_TRANSACTION,
	SEGMENT_TYPE_TEMP
 @return address of new extent, otherwise EXIT_ERROR

*/
int KK_init_new_extent ( char *table_name , int extent_type){
	table_addresses *adrese;
	adrese = (table_addresses *) get_table_addresses(table_name);
	int adr_bloka = adrese->address_from[1];
	int old_size=0;

	//promjentiti temp_block = mem_block->block
	//KK_mem_block *mem_block = (KK_mem_block *) malloc(sizeof(KK_mem_block));
	//mem_block = KK_get_block(adr_bloka); // bilo koji blok tablice, samo da se dobije header iz njega

			KK_block *temp_block = (KK_block *) malloc(sizeof(KK_block));
			temp_block = (KK_block *) KK_read_block(adr_bloka);


	int velicina=0;
	register int i=0;
	for(i=0; i<200; i++){					//jer je tak novak stavio (fileio.h)
		if(adrese->address_from[i] == 0)		//ako smo prošli cijelo polje
			break;
		velicina = adrese->address_to[i] - adrese->address_from[i];
		if(velicina > old_size)			//trazim najveći extent
			old_size = velicina;
	}

	old_size += 1;
	int pocetna_adr = 0;
	if ( (pocetna_adr = KK_new_extent(1, old_size, extent_type, temp_block->header)) == EXIT_ERROR){
		printf("KK_init_new_extent: Could not alocate new extent\n");
		return EXIT_ERROR;
	}
	if(DEBUG)
		printf("KK_init_new_extent: pocetna_adr=%i, old_size=%i, extent_type=%i\n",pocetna_adr, old_size, extent_type);

	int zavrsna_adr = pocetna_adr;

	float RESIZE_FACTOR = 0;

		switch(extent_type)
		{
			case SEGMENT_TYPE_TABLE:
				RESIZE_FACTOR = EXTENT_GROWTH_TABLE;
				break;
			case SEGMENT_TYPE_INDEX:
				RESIZE_FACTOR = EXTENT_GROWTH_INDEX;
				break;
			case SEGMENT_TYPE_TRANSACTION:
				RESIZE_FACTOR = EXTENT_GROWTH_TRANSACTION;
				break;
			case SEGMENT_TYPE_TEMP:
				RESIZE_FACTOR = EXTENT_GROWTH_TEMP;
				break;
		}

		zavrsna_adr = pocetna_adr + ( old_size + old_size * RESIZE_FACTOR );


	//mem_block = KK_get_block( 0 );

		temp_block= (KK_block *) KK_read_block( 0 );						//tu zamjena



	int trazi=1;
	char name_sys[100];
	int address_sys;
	int free=0;
	if(DEBUG)
		printf("\nKK_init_new_extent: Tražim systemsku tablicu relacija \n");

	for(i=0;i<DATA_BLOCK_SIZE;)
	{

		//trazi=0;
		free=0;
		for(free;free<100;free++)
			name_sys[free]='\0';

		memcpy(name_sys,
			   temp_block->data + temp_block->tuple_dict[i].address,
			   temp_block->tuple_dict[i].size );

		printf("\n adresa: %s",name_sys);
		i++;

		memcpy(&address_sys,
			   temp_block->data + temp_block->tuple_dict[i].address,
			   temp_block->tuple_dict[i].size);

		if(strcmp(name_sys,"KK_relation")==0)
		{	if(DEBUG)
				printf("\nKK_init_new_extent: Pronasao adresu relacijeske sys tablice: %d \n",address_sys);
			trazi=0;
			break;
		}
		i++;

	}

	//adresa relacijske sistemske tablice je pronađena
	// zapisati u sistemski katalog relacije

		//mem_block = KK_get_block( address_sys );

		temp_block=(KK_block *)KK_read_block(address_sys);						//tu zamjena

	//trazi mjesto za slijedeci unos u sis katalogu
	int id=0;
	int nadjeno=1;
	while ( nadjeno )
	{
		id++; //to je vrijednost gdje se pise
		if( temp_block->tuple_dict[id].size == 0 )
		{
			nadjeno=0;
		}
	}


	//to je kaj unesem nutra
		int obj_id=2;
	if(DEBUG)
		printf("unosim: %d , %s, %i, %i", obj_id, table_name, pocetna_adr, zavrsna_adr);
	KK_insert_entry(temp_block, TYPE_INT, &obj_id, id );
	KK_insert_entry(temp_block, TYPE_VARCHAR, table_name, id + 1 );
	KK_insert_entry(temp_block, TYPE_INT, &pocetna_adr, id + 2 );
	KK_insert_entry(temp_block, TYPE_INT, &zavrsna_adr, id + 3);

	return pocetna_adr;
}




