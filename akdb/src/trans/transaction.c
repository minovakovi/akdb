/**
   @file transaction.c Defines functions for transaction execution
*/
/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 *
 * transaction.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * transaction.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "transaction.h"
#include "../auxi/ptrcontainer.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

AK_transaction_list LockTable[NUMBER_OF_KEYS];

pthread_mutex_t accessLockMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t acquireLockMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t newTransactionLockMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t endTransationTestLockMutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cond_lock  = PTHREAD_COND_INITIALIZER;
PtrContainer observable_transaction;
pthread_t activeThreads[MAX_ACTIVE_TRANSACTIONS_COUNT];
pthread_mutex_t wfg_mutex = PTHREAD_MUTEX_INITIALIZER;

int activeTransactionsCount = 0;
int transactionsCount = 0;



static bool wfg[MAX_ACTIVE_TRANSACTIONS_COUNT][MAX_ACTIVE_TRANSACTIONS_COUNT];
static int tarjan_idx[MAX_ACTIVE_TRANSACTIONS_COUNT];
static int tarjan_low[MAX_ACTIVE_TRANSACTIONS_COUNT];
static bool tarjan_onstack[MAX_ACTIVE_TRANSACTIONS_COUNT];
static int tarjan_stack[MAX_ACTIVE_TRANSACTIONS_COUNT];
static int tarjan_stack_top;
static int tarjan_time;


/**
 * @author Petar Knezovic
 * @brief Reset the wait-for graph by clearing all existing wait edges.
 */
void AK_init_wait_for_graph(void) {
    memset(wfg, 0, sizeof(wfg));
}


/**
 * @author Petar Knezovic
 * @brief Add a directed edge to the wait-for graph indicating that one transaction is waiting on another.
 * @param from The thread ID of the transaction that is waiting.
 * @param to The thread ID of the transaction being waited on.
 */
 void AK_add_wait_edge(pthread_t from, pthread_t to) {
     AK_PRO;
     int i, j;
     pthread_mutex_lock(&wfg_mutex);

     for (i = 0; i < MAX_ACTIVE_TRANSACTIONS_COUNT; ++i)
         if (activeThreads[i] == from) break;
     for (j = 0; j < MAX_ACTIVE_TRANSACTIONS_COUNT; ++j)
         if (activeThreads[j] == to) break;
     if (i < MAX_ACTIVE_TRANSACTIONS_COUNT && j < MAX_ACTIVE_TRANSACTIONS_COUNT)
         wfg[i][j] = true;
     pthread_mutex_unlock(&wfg_mutex);
     AK_EPI;
 }

/**
 * @author Petar Knezovic
 * @brief Remove all outgoing wait edges from a given transaction in the wait-for graph.
 * @param tx The thread ID of the transaction whose outgoing edges should be cleared.
 */
 void AK_remove_wait_edges(pthread_t tx) {
     AK_PRO;
     int i;
     pthread_mutex_lock(&wfg_mutex);

     for (i = 0; i < MAX_ACTIVE_TRANSACTIONS_COUNT; ++i)
         if (activeThreads[i] == tx) break;
     if (i == MAX_ACTIVE_TRANSACTIONS_COUNT) {
        pthread_mutex_unlock(&wfg_mutex);
         return;
     }
     for (int j = 0; j < MAX_ACTIVE_TRANSACTIONS_COUNT; ++j)
         wfg[i][j] = false;
     pthread_mutex_unlock(&wfg_mutex);
     AK_EPI;
 }

/**
 * @author Petar Knezovic
 * @brief Recursive helper function for Tarjan’s algorithm to find strongly connected components in the wait-for graph.
 * @param v The index of the current vertex in the graph being explored.
 * @param cycle Pointer to an array where detected cycle thread IDs will be stored.
 * @param cycle_size Pointer to an integer where the size of the detected cycle will be recorded.
 * @param found Pointer to a boolean flag set to true when a cycle (deadlock) is found.
 */
static void AK_strongconnect(int v, pthread_t *cycle, int *cycle_size, bool *found) {
    AK_PRO;
    tarjan_idx[v] = tarjan_low[v] = tarjan_time++;
    tarjan_stack[tarjan_stack_top++] = v;
    tarjan_onstack[v] = true;
    for (int w = 0; w < MAX_ACTIVE_TRANSACTIONS_COUNT; ++w) {
        if (!wfg[v][w]) continue;
        if (tarjan_idx[w] < 0) {
            AK_strongconnect(w, cycle, cycle_size, found);
            tarjan_low[v] = tarjan_low[v] < tarjan_low[w] ? tarjan_low[v] : tarjan_low[w];
        } else if (tarjan_onstack[w]) {
            tarjan_low[v] = tarjan_low[v] < tarjan_idx[w] ? tarjan_low[v] : tarjan_idx[w];
        }
    }
    if (tarjan_low[v] == tarjan_idx[v]) {
        int count = 0, w;
        do {
            w = tarjan_stack[--tarjan_stack_top];
            tarjan_onstack[w] = false;
            cycle[count++] = activeThreads[w];
        } while (w != v);
        if (count > 1) {
            *cycle_size = count;
            *found = true;
        }
    }
    AK_EPI;
}

/**
 * @author Petar Knezovic
 * @brief Detect whether there is a deadlock (cycle) in the current wait-for graph.
 * @param cycle Pointer to an array that will be populated with the thread IDs forming the detected cycle.
 * @param cycle_size Pointer to an integer where the number of threads in the detected cycle will be stored.
 * @return true if a deadlock cycle is found; false otherwise.
 */
bool AK_detect_deadlock(pthread_t *cycle, int *cycle_size) {
    AK_PRO;
    tarjan_time = 0;
    tarjan_stack_top = 0;
    for (int i = 0; i < MAX_ACTIVE_TRANSACTIONS_COUNT; ++i) {
        tarjan_idx[i] = -1;
        tarjan_low[i] = 0;
        tarjan_onstack[i] = false;
    }
    bool found = false;
    for (int i = 0; i < MAX_ACTIVE_TRANSACTIONS_COUNT && !found; ++i)
        if (tarjan_idx[i] < 0)
            AK_strongconnect(i, cycle, cycle_size, &found);
    return found;
    AK_EPI;
}

/**
 * @author Petar Knezovic
 * @brief Abort a transaction by removing it from the active list, clearing its wait-for edges, deleting its acquired locks, and waking any waiting threads.
 * @param tx The thread ID of the transaction to abort.
 */
void AK_abort_transaction(pthread_t tx) {
    AK_PRO;
    int idx = -1;
    for (int i = 0; i < MAX_ACTIVE_TRANSACTIONS_COUNT; ++i) {
        if (pthread_equal(activeThreads[i], tx)) {
            idx = i;
            break;
        }
    }

    if (idx >= 0) {
        for (int j = 0; j < MAX_ACTIVE_TRANSACTIONS_COUNT; ++j) {
            wfg[idx][j] = false;   
            wfg[j][idx] = false;  
        }
    }

    for (int h = 0; h < NUMBER_OF_KEYS; ++h) {
        AK_transaction_elem_P elem = LockTable[h].DLLHead;
        if (!elem) continue;
        AK_transaction_lock_elem_P l = elem->DLLLocksHead;
        if (!l) continue;
        AK_transaction_lock_elem_P tmp = l;
        do {
            if (pthread_equal(tmp->TransactionId, tx))
                AK_delete_lock_entry_list(elem->address, tx);
            tmp = tmp->nextLock;
        } while (tmp != l);
    }

    AK_remove_transaction_thread(tx);

    pthread_cond_broadcast(&cond_lock);
    AK_EPI;
}

/**
 * @author Frane Jakelić
 * @brief Function that calculates the hash value for a given memory address. Hash values are used to identify location of locked resources.
 * @todo The current implementation is very limited it doesn't cope well with collision. recommendation use some better version of hash calculation. Maybe Knuth's memory address hashing function.
 * @param blockMemoryAddress integer representation of memory address, the hash value is calculated from this parameter.
 * @return integer containing the hash value of the passed memory address
 */
int AK_memory_block_hash(int blockMemoryAddress) {
    AK_PRO;
    uint32_t h = (uint32_t)blockMemoryAddress;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    int ret = (int)(h % NUMBER_OF_KEYS);
    AK_EPI;
    return ret;
}


/**
 * @author Frane Jakelić
 * @brief Function that searches for a existing entry in hash list of active blocks
 * @param blockAddress integer representation of memory address.
 * @return pointer to the existing hash list entry
 */
AK_transaction_elem_P AK_search_existing_link_for_hook(int blockAddress){
	AK_PRO;
	int hash = AK_memory_block_hash(blockAddress);
	AK_transaction_elem_P tmp = LockTable[hash].DLLHead;

	if(!tmp){
		AK_EPI;
		return NULL;
	}

	do{
		if(tmp->address == blockAddress){
			AK_EPI;
			return tmp;
		}
		tmp = tmp->nextBucket;
	} while(tmp != LockTable[hash].DLLHead);
	AK_EPI;
	return NULL;
}

/**
 * @author Frane Jakelić
 * @brief Function that searches for a empty link for new active block, helper method in case of address collision
 * @param blockAddress integer representation of memory address.
 * @return pointer to empty location to store new active address
 */
AK_transaction_elem_P AK_search_empty_link_for_hook(int blockAddress){
	AK_PRO;
	int hash = AK_memory_block_hash(blockAddress);

	if(!LockTable[hash].DLLHead){

		LockTable[hash].DLLHead = (AK_transaction_elem_P) AK_malloc(sizeof (AK_transaction_elem));
		memset(LockTable[hash].DLLHead, 0, sizeof (AK_transaction_elem));
		AK_EPI;
		return LockTable[hash].DLLHead;
	}else{
		AK_EPI;
		return LockTable[hash].DLLHead;
	}
	AK_EPI;
}

/**
 * @author Frane Jakelić
 * @brief Function that adds an element to the doubly linked list.
 * @param blockAddress integer representation of memory address.
 * @param type of lock issued to the provided memory address.
 * @return pointer to the newly created doubly linked element.
 */
AK_transaction_elem_P AK_add_hash_entry_list(int blockAddress, int type) {

    AK_PRO;
    AK_transaction_elem_P root = AK_search_existing_link_for_hook(blockAddress);
    AK_transaction_elem_P bucket;
    if(root){
	AK_EPI;
	return root;
    }

    root = AK_search_empty_link_for_hook(blockAddress);
    if(!root->nextBucket){
    	bucket = root;
    	root->nextBucket = root;
    	root->prevBucket = root;
    }else{

    	bucket = (AK_transaction_elem_P) AK_malloc(sizeof (AK_transaction_elem));
        memset(bucket, 0, sizeof (AK_transaction_elem));
        bucket->nextBucket = root;
        bucket->prevBucket = root->prevBucket;

        (*root->prevBucket).nextBucket = bucket;
        root->prevBucket = bucket;
    }

    bucket->address = blockAddress;
    bucket->lock_type = type;
    bucket->observer_lock = AK_init_observer_lock();
    AK_transaction_register_observer(observable_transaction.ptr, bucket->observer_lock->observer);
    AK_EPI;
    return bucket;
}

/**
 * @author Frane Jakelić
 * @brief Function that deletes a specific element in the lockTable doubly linked list.
 * @param blockAddress integer representation of memory address.
 * @return integer OK or NOT_OK based on success of finding the specific element in the list.
 */
int AK_delete_hash_entry_list(int blockAddress) {
    AK_PRO;
    int hash = AK_memory_block_hash(blockAddress);
    AK_transaction_elem_P elemDelete = AK_search_existing_link_for_hook(blockAddress);

    if (elemDelete) {

        (*elemDelete->prevBucket).nextBucket = elemDelete->nextBucket;
        (*elemDelete->nextBucket).prevBucket = elemDelete->prevBucket;


        if (elemDelete == LockTable[hash].DLLHead && elemDelete->nextBucket != elemDelete) {
            LockTable[hash].DLLHead = elemDelete->nextBucket;
        } else if(elemDelete->nextBucket == elemDelete){
            LockTable[hash].DLLHead = NULL;
        }

        elemDelete->prevBucket = NULL;
        elemDelete->nextBucket = NULL;
	AK_EPI;
        return OK;

    } else {
	AK_EPI;
        return NOT_OK;
    }
    AK_EPI;
}

/**
 * @author Frane Jakelić
 * @brief Function that searches for a specific entry in the Locks doubly linked list using the transaction id as it's key.
 * @param memoryAddress integer representation of memory address.
 * @param id integer representation of transaction id.
 * @return NULL pointer if the element is not found otherwise it returns a pointer to the found element
 */
AK_transaction_lock_elem_P AK_search_lock_entry_list_by_key(AK_transaction_elem_P Lockslist, int memoryAddress, pthread_t id) {
    AK_PRO;
    AK_transaction_lock_elem_P tmpElem = Lockslist->DLLLocksHead;

    if (!tmpElem || pthread_equal(tmpElem->TransactionId,id)) {
	AK_EPI;
        return tmpElem;
    }
    
    int counter = 0;
    while (counter++ <= NUMBER_OF_KEYS) {
        if (pthread_equal(tmpElem->TransactionId,id)) {
	    AK_EPI;
            return tmpElem;
        }
        tmpElem = tmpElem->nextLock;
    }
    AK_EPI;
    return NULL;
}

/**
 * @author Frane Jakelić
 * @brief Function that deletes a specific entry in the Locks doubly linked list using the transaction id as it's key.
 * @param blockAddress integer representation of memory address.
 * @param id integer representation of transaction id.
 * @return int OK or NOT_OK based on success of finding the specific element in the list.
 */
int AK_delete_lock_entry_list(int blockAddress, pthread_t id) {
	AK_PRO;
	AK_transaction_elem_P elemListHolder = AK_search_existing_link_for_hook(blockAddress);
    AK_transaction_lock_elem_P elemDelete = AK_search_lock_entry_list_by_key(elemListHolder,blockAddress, id);

    if (!elemDelete) {
	AK_EPI;
	return NOT_OK;
    }
    
    do {

        (*elemDelete->prevLock).nextLock = elemDelete->nextLock;
        (*elemDelete->nextLock).prevLock = elemDelete->prevLock;

        if (elemDelete == elemListHolder->DLLLocksHead && elemDelete->nextLock != elemDelete) {

            elemListHolder->DLLLocksHead = elemDelete->nextLock;

        } else if (elemDelete == elemListHolder->DLLLocksHead) {

            elemListHolder->DLLLocksHead = NULL;
        }

        elemDelete->prevLock = NULL;
        elemDelete->nextLock = NULL;

        elemDelete = AK_search_lock_entry_list_by_key(elemListHolder,blockAddress, id);

    } while (elemDelete);
    AK_EPI;
    return OK;


}

/**
 * @author Frane Jakelić updated by Ivan Pusic
 * @brief Function that, based on the parameters, puts an transaction action in waiting phase or let's the transaction do it's actions.
 * @param lockHolder pointer to the hash list entry that is entitled to the specific memory address.
 * @param type of lock issued to the provided memory address.
 * @param transactionId integer representation of transaction id.
 * @param lock pointer to the lock element that is being tested.
 * @return int PASS_LOCK_QUEUE or WAIT_FOR_UNLOCK based on the rules described inside the function.
 */
int AK_isLock_waiting(AK_transaction_elem_P lockHolder, int type, pthread_t transactionId, AK_transaction_lock_elem_P lock) {
    AK_PRO;
    AK_transaction_lock_elem_P tmp = lockHolder->DLLLocksHead;
    
	if (tmp == lock) {
		lockHolder->lock_type = type;
		AK_EPI;
		return PASS_LOCK_QUEUE;
	} else if (tmp->lock_type == SHARED_LOCK) {
        //BEGIN SHARED LOCK
		if (type == SHARED_LOCK) {
			AK_EPI;
			return PASS_LOCK_QUEUE;

		} else if (type == EXCLUSIVE_LOCK) {
	    AK_EPI;
            return WAIT_FOR_UNLOCK;
		}
        //END SHARED LOCK

	} else if (tmp->lock_type == EXCLUSIVE_LOCK) {
        if ( pthread_equal(tmp->TransactionId, transactionId) ) {
	    AK_EPI;
            return PASS_LOCK_QUEUE;
        }
	}
	AK_EPI;
	return WAIT_FOR_UNLOCK;
}


/**
 * @author Frane Jakelić
 * @brief Function that adds an element to the locks doubly linked list.
 * @param memoryAddress integer representation of memory address.
 * @param type of lock issued to the provided memory address.
 * @param transactionId integer representation of transaction id.
 * @return pointer to the newly created Locks doubly linked element.
 */
AK_transaction_lock_elem_P AK_add_lock(AK_transaction_elem_P HashList, int type, pthread_t transactionId) {
    AK_PRO;
    AK_transaction_lock_elem_P root = HashList->DLLLocksHead;
    AK_transaction_lock_elem_P lock = (AK_transaction_lock_elem_P) AK_malloc(sizeof (AK_transaction_lock_elem));
    memset(lock, 0, sizeof (AK_transaction_lock_elem));

    if (!root) {
    	HashList->DLLLocksHead = lock;
        lock->prevLock = lock;
        lock->nextLock = lock;

    } else {
        lock->nextLock = root;
        lock->prevLock = root->prevLock;
        (*root->prevLock).nextLock = lock;
        root->prevLock = lock;
    }

    lock->TransactionId = transactionId;
    lock->lock_type = type;
    
    lock->isWaiting = AK_isLock_waiting(HashList, type, transactionId, lock);
    AK_EPI;
    return lock;
}

/**
 * @author Frane Jakelić
 * @brief Helper function that determines if there is a hash LockTable entry that corresponds to the given memory address. And if there isn't an entry the function calls for the creation of the Locks list holder.
 * @param memoryAddress integer representation of memory address.
 * @param type of lock issued to the provided memory address.
 * @param transactionId integer representation of transaction id.
 * @return pointer to the newly created Locks doubly linked element.
 */
AK_transaction_lock_elem_P AK_create_lock(int blockAddress, int type, pthread_t transactionId) {
    AK_PRO;
    AK_transaction_elem_P elem = AK_search_existing_link_for_hook(blockAddress);

    if (!elem) {
        elem = AK_add_hash_entry_list(blockAddress, type);
    }
    AK_EPI;
    return AK_add_lock(elem, type, transactionId);
}



/**
 * @author Frane Jakelić updated by Ivan Pusic
 * @brief Main interface function for the transaction API. It is responsible for the whole process of creating a new lock.
 * @todo Implement a better deadlock detection. This method uses a very simple approach. It waits for 60sec before it restarts a transaction.
 * @param memoryAddress integer representation of memory address.
 * @param type of lock issued to the provided memory address.
 * @param transactionId integer representation of transaction id.
 * @return OK or NOT_OK based on the success of the function.
 */

int AK_acquire_lock(int memoryAddress, int type, pthread_t transactionId) {
    AK_PRO;
    pthread_mutex_lock(&accessLockMutex);
    AK_transaction_lock_elem_P lock = AK_create_lock(memoryAddress, type, transactionId);
    pthread_mutex_unlock(&accessLockMutex);
    int counter = 0;
    
  AK_transaction_elem_P tmp = AK_search_existing_link_for_hook(memoryAddress);
if (lock->isWaiting == WAIT_FOR_UNLOCK) {
    pthread_t owner = tmp->DLLLocksHead->TransactionId; 
    AK_add_wait_edge(transactionId, owner);

    pthread_t cycle[MAX_ACTIVE_TRANSACTIONS_COUNT];
    int cycle_size = 0;
    if (AK_detect_deadlock(cycle, &cycle_size)) {
        AK_abort_transaction(cycle[cycle_size - 1]);
    }

    while (lock->isWaiting == WAIT_FOR_UNLOCK) {
        pthread_mutex_lock(&acquireLockMutex);
        pthread_cond_wait(&cond_lock, &acquireLockMutex);
        pthread_mutex_unlock(&acquireLockMutex);
        lock->isWaiting = AK_isLock_waiting(tmp, type, transactionId, lock);
    }

    AK_remove_wait_edges(transactionId);
}
    
    if (counter > 0) {

    	/* printf("################\n# Lock Granted after wait#\n#------------------------#\n# Lock	ID:%lu	TYPE:%i	 #\n#------------------------#\n# LockedAddress:%i	 #\n##########################\n\n", (unsigned long)lock->TransactionId, lock->lock_type, memoryAddress); */

    } else {

    	/* printf("##########################\n# Lock Granted		 #\n#------------------------#\n# Lock	ID:%lu	TYPE:%i	 #\n#------------------------#\n# LockedAddress:%i	 #\n##########################\n\n", (unsigned long)lock->TransactionId, lock->lock_type, memoryAddress); */
    }
    AK_EPI;
    return OK;
}

/**
 * @author Frane Jakelić updated by Ivan Pusic
 * @brief Main interface function for the transaction API. It is responsible for the whole process releasing locks acquired by a transaction. The locks are released either by COMMIT or ABORT .
 * @param adresses linked list of memory addresses locked by the transaction.
 * @param transactionId integer representation of transaction id.
 */
void AK_release_locks(AK_memoryAddresses_link addressesTmp, pthread_t transactionId) {
    AK_PRO;
    pthread_mutex_lock(&accessLockMutex);

    while (addressesTmp->nextElement != NULL) {

        // Unregister observer and deallocate used memory (dealocating memory is in observable.c unregister_observer)
        AK_observer *observer = AK_search_existing_link_for_hook(addressesTmp->adresa)->observer_lock->observer;
        if(observer != NULL) {
            AK_transaction_unregister_observer(observable_transaction.ptr, observer);
            observer = NULL;
        }
         
        AK_delete_lock_entry_list(addressesTmp->adresa, transactionId);

        // notify observable transaction about lock release
        AK_observable_transaction* const observableTransaction = observable_transaction.ptr;
        observableTransaction->AK_lock_released();
        addressesTmp = addressesTmp->nextElement;
    }
    pthread_mutex_unlock(&accessLockMutex);
    AK_EPI;
}

/**
 * @author Frane Jakelić
 * @brief Function that appends all addresses affected by the transaction
 * @param addressList pointer to the linked list where the addresses are stored.
 * @param tblName table name used in the transaction
 * @return OK or NOT_OK based on the success of the function.
 */
int AK_get_memory_blocks(char *tblName, AK_memoryAddresses_link addressList) {
    AK_PRO;
    table_addresses *addresses = (table_addresses*) AK_get_table_addresses(tblName);
    if (addresses->address_from[0] == 0){
	AK_EPI;
        return NOT_OK;
    }

    if (addressList->nextElement != NULL) {
        addressList->nextElement = NULL;
    }

    int i = 0, j;
    AK_memoryAddresses_link tmp = addressList;

    for (j = addresses->address_from[ i ]; j < addresses->address_to[ i ]; j++) {
        tmp->adresa = j;
        tmp->nextElement = (AK_memoryAddresses_link) AK_malloc(sizeof(struct memoryAddresses));
        memset(tmp->nextElement, 0, sizeof (struct memoryAddresses));
        tmp = tmp->nextElement;

    }
    AK_EPI;
    return OK;
}

/**
 * @author Frane Jakelić updated by Ivan Pusic
 * @brief Function that is called in a separate thread that is responsible for acquiring locks, releasing them and finding the associated block addresses
 * @todo Check multithreading, check if it's working correctly
 * @param commandArray array filled with commands that need to be secured using transactions
 * @param lengthOfArray length of commandArray
 * @param transactionId associated with the transaction
 * @return ABORT or COMMIT based on the success of the function.
 */
int AK_execute_commands(command * commandArray, int lengthOfArray) {
    int i = 0, status = 0;
    AK_memoryAddresses addresses;
    AK_PRO;
    AK_memoryAddresses_link address = (AK_memoryAddresses_link) AK_malloc(sizeof(struct memoryAddresses));

    for (i = 0; i < lengthOfArray; i++) {

        if (!AK_get_memory_blocks(commandArray[i].tblName, &addresses)) {
            printf("Error reading block Addresses. Aborting\n");
	    AK_EPI;
            return ABORT;
        };

        address = &addresses;
        while (address->nextElement != NULL) {

            switch (commandArray[i].id_command) {
            case UPDATE:
                status = AK_acquire_lock(address->adresa, EXCLUSIVE_LOCK, pthread_self());
                break;
            case DELETE:
                status = AK_acquire_lock(address->adresa, EXCLUSIVE_LOCK, pthread_self());
                break;
            case INSERT:
                status = AK_acquire_lock(address->adresa, EXCLUSIVE_LOCK, pthread_self());
                break;
            case SELECT:
                status = AK_acquire_lock(address->adresa, SHARED_LOCK, pthread_self());
                break;
            default:
                break;
            }

            if (status == NOT_OK) {
                printf("Error acquiring lock. Aborting\n");
                AK_release_locks(&addresses, pthread_self());
		AK_EPI;
                return ABORT;
            }
            address = address->nextElement;
        }
    }
    
    if(AK_command(commandArray, lengthOfArray) == EXIT_ERROR){
	AK_EPI;
        return ABORT;
    }
    
    AK_release_locks(&addresses, pthread_self());
    AK_EPI;
    return COMMIT;
}


/**
 * @author Frane Jakelić updated by Ivan Pusic
 * @brief Function that is the thread start point all relevant functions. It acts as an intermediary between the main thread and other threads
 * @param data transmitted to the thread from the main thread
 */
void * AK_execute_transaction(void *params) {
    int status;
    AK_PRO;
    AK_transaction_data *data = (AK_transaction_data *)params;

    status = AK_execute_commands(data->array, data->lengthOfArray);
    if (status == ABORT) {
        printf("Transaction ABORTED!\n");
    } else {
        printf("Transaction COMMITED!\n");
    }
    // notify observable_transaction about transaction finish
    AK_observable_transaction* const observableTransaction = observable_transaction.ptr;
    observableTransaction->AK_transaction_finished();
    AK_EPI;
    return NULL;
}

/** 
 * @author Ivan Pusic
 * @brief Function for deleting one of active threads from array of all active transactions threads
 * @param transaction_thread Active thread to delete
 * 
 * @return Exit status (OK or NOT_OK)
 */
int AK_remove_transaction_thread(pthread_t transaction_thread) {
    int i;
    AK_PRO;
    for (i = 0; i < MAX_ACTIVE_TRANSACTIONS_COUNT; ++i) {
        if(activeThreads[i] == transaction_thread) {
            activeThreads[i] = (pthread_t)NULL;
            activeTransactionsCount--;
	    AK_EPI;
            return OK;
        }
    }
    AK_EPI;
    return NOT_OK;
}

/** 
 * @author Ivan Pusic
 * @brief Function for creating new thread. Function also adds thread ID to pthread_t array
 * @param transaction_data Data for executing transaction
 *
 * @return Exit status (OK or NOT_OK)
 */
int AK_create_new_transaction_thread(AK_transaction_data *transaction_data) {
    pthread_t t1;
    AK_PRO;
    int i;
    for (i = 0; i < MAX_ACTIVE_TRANSACTIONS_COUNT; ++i) {
        if(activeThreads[i] == (pthread_t)NULL) {
            pthread_create(&t1, NULL, AK_execute_transaction, transaction_data);
            activeThreads[i] = t1;
            activeTransactionsCount++;
	    AK_EPI;
            return OK;
        }
    }
    AK_EPI;
    return NOT_OK;
}

/**
 * @author Frane Jakelić updated by Ivan Pusic
 * @brief Function that receives all the data and gives an id to that data and starts a thread that executes the transaction 
 * @param commandArray array filled with commands that need to be secured using transactions
 * @param lengthOfArray length of commandArray
 */
int AK_transaction_manager(command * commandArray, int lengthOfArray) {
    AK_PRO;
    AK_transaction_data* params = AK_malloc(sizeof(AK_transaction_data));
    params->array = AK_malloc(sizeof(command));
    params->array = commandArray;
    params->lengthOfArray = lengthOfArray;
    transactionsCount++;

    if(activeTransactionsCount < MAX_ACTIVE_TRANSACTIONS_COUNT) {
        pthread_mutex_lock(&accessLockMutex);
        AK_create_new_transaction_thread(params);
        pthread_mutex_unlock(&accessLockMutex);
    }

    else {
        // wait until some transaction finishes 
        pthread_mutex_lock(&newTransactionLockMutex);
        AK_create_new_transaction_thread(params);
    }
    AK_EPI;
    return OK;
}

/** 
 * @author Ivan Pusic
 * @brief Function for registering new observer of AK_observable_transaction type
 * @param observable_transaction Observable type instance
 * @param observer Observer instance
 * 
 * @return Exit status (OK or NOT_OK)
 */
int AK_transaction_register_observer(AK_observable_transaction *observable_transaction, AK_observer *observer) {
    int ret;
    AK_PRO;
    ret = observable_transaction->observable->AK_register_observer(observable_transaction->observable, observer);
    AK_EPI;
    return ret;
}

/** 
 * @author Ivan Pusic
 * @brief Function for unregistering observer from AK_observable_transction type
 * @param observable_transaction Observable type instance
 * @param observer Observer instance
 * 
 * @return Exit status (OK or NOT_OK)
 */
int AK_transaction_unregister_observer(AK_observable_transaction *observable_transaction, AK_observer *observer) {
    int ret;
    AK_PRO;
    ret = observable_transaction->observable->AK_unregister_observer(observable_transaction->observable, observer);
    AK_EPI;
    return ret;
}

/** 
 * @author Ivan Pusic
 * @brief Function for handling AK_observable_transaction notify. Function is associated to some observer instance
 * @param observer_lock Observer type instance
 */
void handle_transaction_notify(AK_observer_lock *observer_lock) {
    AK_PRO;
    printf ("MESSAGE FROM TRANSACTION RECIEVED!\n");
    AK_EPI;
}

/** 
 * @author Ivan Pusic
 * @brief Function for handling notify from some observable type
 * @param observer Observer type
 * @param observable Observable type
 * @param type Type of observable who sent some notice
 */
void AK_on_observable_notify(void *observer, void *observable, AK_ObservableType_Enum type) {
    AK_PRO;
    switch(type) {
    case AK_TRANSACTION:
        handle_transaction_notify((AK_observer_lock*) observer);
        break;
    default:
        break;
    }
    AK_EPI;
}

/** 
 * @author Ivan Pusic
 * @brief Function for handling event when some transaction is finished
 * @param transaction_thread Thread ID of transaction which is finished
 */
void AK_on_transaction_end(pthread_t transaction_thread) {
    AK_PRO;
    AK_remove_transaction_thread(transaction_thread);
    pthread_cond_broadcast(&cond_lock);
    // unlock mutex -> after this new transaction can be executed if lock stops transaction execution
    pthread_mutex_unlock(&newTransactionLockMutex);
    transactionsCount--;
    printf ("TRANSACTIN END!!!!\n");
    AK_observable_transaction* const observableTransaction = observable_transaction.ptr;
    if(transactionsCount == 0)
        observableTransaction->AK_all_transactions_finished();
    AK_EPI;
}

/** 
 * @author Ivan Pusic
 * @brief Function for handling  event when all transactions are finished
 */
void AK_on_all_transactions_end() {
    // after unlocking this mutex, main thread will continue execution
    AK_PRO;
    pthread_mutex_unlock(&endTransationTestLockMutex);
    printf ("ALL TRANSACTIONS ENDED!!!\n");
    AK_EPI;
}

/** 
 * @author Ivan Pusic
 * @brief Function for handling event when one of lock is released
 */
void AK_on_lock_release() {
    AK_PRO;
    pthread_cond_broadcast(&cond_lock);
    printf ("TRANSACTION LOCK RELEASED!\n");
    AK_EPI;
}

/** 
 * @author Ivan Pusic
 * @brief Function for handling action which is called from observable_transaction type
 * @param noticeType Type of action (event)
 */
void AK_handle_observable_transaction_action(NoticeType *noticeType) {
    AK_PRO;
    switch((NoticeType)noticeType) {
    case AK_LOCK_RELEASED:
        AK_on_lock_release();
        break;
    case AK_TRANSACTION_FINISHED:
        AK_on_transaction_end(pthread_self());
        break;
    case AK_ALL_TRANSACTION_FINISHED:
        AK_on_all_transactions_end();
        break;
    default:
        break;
    }
    AK_EPI;
}

/** 
 * @author Ivan Pusic
 * @brief Function which is called when the lock is released
 */
void AK_lock_released() {
    AK_observable_transaction* const observableTransaction = observable_transaction.ptr;
    observableTransaction->observable->AK_run_custom_action((NoticeType*)AK_LOCK_RELEASED);
}

/** 
 * @author Ivan Pusic
 * @brief Function that is called when some transaction is finished
 */
void AK_transaction_finished() {
    AK_PRO;
    AK_observable_transaction* const observableTransaction = observable_transaction.ptr;
    observableTransaction->observable->AK_run_custom_action((NoticeType*)AK_TRANSACTION_FINISHED);
    AK_EPI;
}

/** 
 * @author Ivan Pusic
 * @brief Function that is called when all transactions are finished
 */
void AK_all_transactions_finished() {
    AK_PRO;
    AK_observable_transaction* const observableTransaction = observable_transaction.ptr;
    observableTransaction->observable->AK_run_custom_action((NoticeType*)AK_ALL_TRANSACTION_FINISHED);
    AK_EPI;
}

/** 
 * @author Ivan Pusic
 * @brief Function for initialization of AK_observable_transaction type
 * 
 * @return Pointer to new AK_observable_transaction instance
 */
AK_observable_transaction * AK_init_observable_transaction() {
    AK_PRO;
    observable_transaction.ptr = AK_calloc(1, sizeof(AK_observable_transaction));
    AK_observable_transaction* const observableTransaction = observable_transaction.ptr;
    observableTransaction->AK_transaction_register_observer = &AK_transaction_register_observer;
    observableTransaction->AK_transaction_unregister_observer = &AK_transaction_unregister_observer;
    observableTransaction->AK_lock_released = &AK_lock_released;
    observableTransaction->AK_transaction_finished = &AK_transaction_finished;
    observableTransaction->AK_all_transactions_finished = &AK_all_transactions_finished;
    observableTransaction->observable = AK_init_observable(observable_transaction.ptr, AK_TRANSACTION, &AK_handle_observable_transaction_action);
    AK_EPI;
    return observable_transaction.ptr;
}

/** 
 * @author Ivan Pusic
 * @brief Function for initialization of AK_observer_lock type
 * 
 * @return Pointer to new AK_observer_lock instance
 */
AK_observer_lock * AK_init_observer_lock() {
    AK_PRO;
    AK_observer_lock *self;
    self = AK_calloc(1, sizeof(AK_observer_lock));
    self->observer = AK_init_observer(self, &AK_on_observable_notify);
    AK_EPI;
    return self;
}


/** 
 * @author Petar Knezovic
 * @brief Function that computes the integer square root of a number using Heron's method
 */

static unsigned isqrt(unsigned long n) {
    unsigned long x = n;
    unsigned long y = (x + 1) >> 1;
    while (y < x) {
        x = y;
        y = (x + n / x) >> 1;
    }
    return (unsigned)x;
}

/** 
 * @author Petar Knezovic
 * @brief Function that approximates the 95% critical value for chi-square distribution
 */

static double chi2_crit95(int df) {
    return df + 2u * isqrt(2u * df);
}


TestResult AK_test_Transaction() {
    AK_PRO;
    int successfulTest = 0;
    int failedTest = 0;
    srand((unsigned)time(NULL));   
    int sample_min = 0;
    int sample_max = 0;
    double sample_avg = 0.0;
    double sample_chi2 = 0.0;
    double sample_crit = 0.0;
    int sample_passed = 0;
    
    double avalanche_avg_dist = 0.0;
    int    avalanche_passed   = 0;
    printf("***Test Transaction***\n");
    pthread_mutex_lock(&endTransationTestLockMutex);
    pthread_mutex_lock(&newTransactionLockMutex);
    
    if(AK_init_observable_transaction() != NULL){
    	successfulTest++;
    }
    else{
    	failedTest++;
    }

    // NOTE: This is the way on which we can broadcast notice to all observers
    // observable_transaction->observable->AK_notify_observers(observable_transaction->observable);
    
    memset(LockTable, 0, NUMBER_OF_KEYS * sizeof (struct transaction_list_head));

    /**************** HASH UNIFORMITY TEST ******************/
{
    const int sampleSize = 100000;
    int counts[NUMBER_OF_KEYS] = {0};

    for (int i = 0; i < sampleSize; ++i) {
        int blockAddr = i;
        int h = AK_memory_block_hash(blockAddr);
        counts[h]++;
    }


    int min = counts[0], max = counts[0];
    for (int b = 1; b < NUMBER_OF_KEYS; ++b) {
        if (counts[b] < min) min = counts[b];
        if (counts[b] > max) max = counts[b];
    }

    double expected_uniform = (double)sampleSize / NUMBER_OF_KEYS;


    double chi2 = 0.0;
    for (int b = 0; b < NUMBER_OF_KEYS; ++b) {
        double diff = counts[b] - expected_uniform;
        chi2 += (diff * diff) / expected_uniform;
    }

    int df = NUMBER_OF_KEYS - 1;
    double crit = chi2_crit95(df);


    sample_min    = min;
    sample_max    = max;
    sample_avg    = expected_uniform;
    sample_chi2   = chi2;
    sample_crit   = crit;
    sample_passed = (chi2 < crit);
    
     if (sample_chi2 < sample_crit) {
        successfulTest++;
    } else {
        failedTest++;
    }
    
}


    /**************** AVALANCHE TEST ******************/
{
    const int N = 1000;      
    const int B = 32;        
    long total_dist = 0;    
    srand((unsigned)time(NULL));
    #define POPCNT(x) __builtin_popcount((x))


    uint32_t murmur32(uint32_t h) {
        h ^= h >> 16;
        h *= 0x85ebca6b;
        h ^= h >> 13;
        h *= 0xc2b2ae35;
        h ^= h >> 16;
        return h;
    }

    for (int i = 0; i < N; ++i) {
        uint32_t x  = (uint32_t)rand();
        uint32_t h1 = murmur32(x);
        for (int b = 0; b < B; ++b) {
            uint32_t x2 = x ^ (1u << b);
            uint32_t h2 = murmur32(x2);
            total_dist += POPCNT(h1 ^ h2);
        }
    }

    avalanche_avg_dist = (double)total_dist / (N * B);


    if (avalanche_avg_dist > B*0.5*0.75 && avalanche_avg_dist < B*0.5*1.25) {
        avalanche_passed = 1;
        successfulTest++;
    } else {
        avalanche_passed = 0;
        failedTest++;
    }
}

 /************ DEADLOCK SYSTEM TESTS ************/

{
    pthread_t cycle[MAX_ACTIVE_TRANSACTIONS_COUNT];
    int cycle_size = 0;
    bool all_passed = true;

    AK_init_wait_for_graph();
    activeTransactionsCount = 2;
    activeThreads[0] = (pthread_t)1;
    activeThreads[1] = (pthread_t)2;

    AK_add_wait_edge(activeThreads[0], activeThreads[1]);
    AK_add_wait_edge(activeThreads[1], activeThreads[0]);
    if (!(AK_detect_deadlock(cycle, &cycle_size) && cycle_size == 2)) all_passed = false;

    AK_abort_transaction(activeThreads[1]);
    if (AK_detect_deadlock(cycle, &cycle_size)) all_passed = false;

    AK_init_wait_for_graph();
    activeTransactionsCount = 3;
    activeThreads[0] = (pthread_t)1;
    activeThreads[1] = (pthread_t)2;
    activeThreads[2] = (pthread_t)3;

    AK_add_wait_edge(activeThreads[0], activeThreads[1]);
    AK_add_wait_edge(activeThreads[1], activeThreads[2]);
    AK_add_wait_edge(activeThreads[2], activeThreads[0]);
    if (!(AK_detect_deadlock(cycle, &cycle_size) && cycle_size == 3)) all_passed = false;

    if (all_passed) successfulTest++;
    else failedTest++;
}

{
    pthread_t cycle[MAX_ACTIVE_TRANSACTIONS_COUNT];
    int cycle_size = 0;
    bool all_passed = true;

    AK_init_wait_for_graph();
    activeTransactionsCount = 3;
    activeThreads[0] = (pthread_t)1;
    activeThreads[1] = (pthread_t)2;
    activeThreads[2] = (pthread_t)3;

    AK_add_wait_edge(activeThreads[0], activeThreads[1]);
    AK_add_wait_edge(activeThreads[1], activeThreads[2]);
    AK_abort_transaction(activeThreads[2]);
    if (AK_detect_deadlock(cycle, &cycle_size)) all_passed = false;

    AK_init_wait_for_graph();
    activeTransactionsCount = 3;
    activeThreads[0] = (pthread_t)10;
    activeThreads[1] = (pthread_t)20;
    activeThreads[2] = (pthread_t)30;

    AK_add_wait_edge(activeThreads[0], activeThreads[1]);
    AK_add_wait_edge(activeThreads[1], activeThreads[2]);
    if (AK_detect_deadlock(cycle, &cycle_size)) all_passed = false;

    AK_init_wait_for_graph();
    activeTransactionsCount = 2;
    activeThreads[0] = (pthread_t)100;
    activeThreads[1] = (pthread_t)200;

    AK_add_wait_edge(activeThreads[0], activeThreads[1]);
    AK_add_wait_edge(activeThreads[0], activeThreads[0]);
    AK_remove_wait_edges(activeThreads[0]);

    int idx = 0, j;
    for (j = 0; j < MAX_ACTIVE_TRANSACTIONS_COUNT; ++j) {
        if (wfg[idx][j]) { all_passed = false; break; }
    }

    if (all_passed) successfulTest++;
    else failedTest++;
}
    /**************** INSERT AND UPDATE COMMAND TEST ******************/
    char *tblName = "student";
    struct list_node *row_root_insert = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&row_root_insert);
    AK_DeleteAll_L3(&row_root_insert);
    int mbr, year;
    float weight;
    mbr = 38262;
    year = 2012;
    weight = 82.00;
    AK_DeleteAll_L3(&row_root_insert);
    AK_Insert_New_Element(TYPE_INT, &mbr, tblName, "mbr", row_root_insert);
    AK_Insert_New_Element(TYPE_VARCHAR, "Ivan", tblName, "firstname", row_root_insert);
    AK_Insert_New_Element(TYPE_VARCHAR, "Pusic", tblName, "lastname", row_root_insert);
    AK_Insert_New_Element(TYPE_INT, &year, tblName, "year", row_root_insert);
    AK_Insert_New_Element(TYPE_FLOAT, &weight, tblName, "weight", row_root_insert);

    struct list_node *row_root_update = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&row_root_update);
    AK_DeleteAll_L3(&row_root_update);
    AK_Update_Existing_Element(TYPE_INT, &mbr, tblName, "mbr", row_root_update);
    AK_Insert_New_Element(TYPE_VARCHAR, "pppppppppp", tblName, "lastname", row_root_update);

    command* commands_ins_up = AK_malloc(sizeof (command) * 2);
    commands_ins_up[0].tblName = "student";
    commands_ins_up[0].id_command = INSERT;
    commands_ins_up[0].parameters = row_root_insert;

    commands_ins_up[1].tblName = "student";
    commands_ins_up[1].id_command = UPDATE;
    commands_ins_up[1].parameters = row_root_update;

    int id_prof;
    id_prof = 35893;
    struct list_node *row_root_p_update = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&row_root_p_update);
    AK_DeleteAll_L3(&row_root_p_update);
    
    AK_Update_Existing_Element(TYPE_INT, &id_prof, "professor", "id_prof", row_root_p_update);
    AK_Insert_New_Element(TYPE_VARCHAR, "FOI", "professor", "firstname", row_root_p_update);


    /**************** DELETE COMMAND TEST ******************/
    command* commands_delete = AK_malloc(sizeof (command) * 1);
    commands_delete[0].tblName = "professor";
    commands_delete[0].id_command = DELETE;
    commands_delete[0].parameters = row_root_p_update;


    struct list_node *expr = (struct list_node *) AK_malloc(sizeof (struct list_node));
	AK_Init_L3(&expr);
	int num = 2010;

	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof ("year"), expr);
	AK_InsertAtEnd_L3(TYPE_INT, (char*)&num, sizeof (int), expr);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, "<", sizeof ("<"), expr);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof ("firstname"), expr);
	AK_InsertAtEnd_L3(TYPE_VARCHAR, "Robert", sizeof ("Robert"), expr);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, "=", sizeof ("="), expr);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, "OR", sizeof ("OR"), expr);
    
    /**************** SELECT COMMAND TEST ******************/
    command* commands_select = AK_malloc(sizeof(command) * 1);
    commands_select[0].tblName = "student";
    commands_select[0].id_command = SELECT;
    commands_select[0].parameters = expr;

    /**************** EXECUTE TRANSACTIONS ******************/
    if(AK_transaction_manager(commands_ins_up, 2) == OK){
    	successfulTest++;
    }
    else{
    	failedTest++;
    }
    if(AK_transaction_manager(commands_ins_up, 2) == OK){
    	successfulTest++;
    }
    else{
    	failedTest++;
    }
    if(AK_transaction_manager(commands_delete, 1) == OK){
    	successfulTest++;
    }
    else{
    	failedTest++;
    }
    if(AK_transaction_manager(commands_select, 1) == OK){
    	successfulTest++;
    }
    else{
    	failedTest++;
    }
    
    pthread_mutex_lock(&endTransationTestLockMutex);
    AK_free(expr);
    AK_free(commands_delete);
    AK_free(commands_select);
    AK_free(commands_ins_up);
    AK_free(row_root_insert);
    AK_free(row_root_update);
    AK_free(row_root_p_update);
    AK_free(observable_transaction.ptr);

    pthread_mutex_unlock(&endTransationTestLockMutex);
    
    printf("***End test Transaction***\n");
    AK_EPI;


    printf("\n========== HASH UNIFORMITY TEST SUMMARY ==========\n");
printf("Sample size: %d\n", 10000);
printf("Result: %s\n", sample_passed ? "PASSED" : "FAILED");
printf("  min  = %d\n", sample_min);
printf("  max  = %d\n", sample_max);
printf("  avg  = %.2f\n", sample_avg);
printf("  chi2 = %.2f\n", sample_chi2);
printf("  crit = %.2f\n", sample_crit);
printf("===============================================\n\n");


printf("\n====== AVALANCHE TEST ======\n");
printf("Samples: %d  Bits flipped/test: %d\n", 1000, 32);
printf("Avg Hamming distance: %.2f  (ideal ~%d)\n",
       avalanche_avg_dist, 32/2);
printf("Result: %s\n", avalanche_passed ? "PASSED" : "FAILED");
printf("============================\n\n");



    return TEST_result(successfulTest,failedTest);
}
