/**
@file transaction.h Header file that contains data structures,functions and defines for the transaction execution
 */
/*
 *
 * transaction.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * transaction.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TRANSACTION_H_
#define TRANSACTION_H_
#include <pthread.h> //TRANSACTIONS
#include "../auxi/test.h"
#include "../auxi/constants.h"
#include "../auxi/configuration.h"
#include "../mm/memoman.h"
#include "../sql/command.h"
#include "../auxi/observable.h"
#include "../file/table.h"
#include "../file/fileio.h"
#include <string.h>
#include "../auxi/mempro.h"

/**
 * @author Ivan Pusic
 * @enum NoticeType
 * @brief Enumeration which define notice types for transactions
 */
typedef enum {
    AK_LOCK_RELEASED,
    AK_TRANSACTION_FINISHED,
    AK_ALL_TRANSACTION_FINISHED,
} NoticeType;

/**
 * @author Ivan Pusic
 * @struct observable_transaction
 * @brief Structure which defines transaction observable type
 */
struct observable_transaction_struct {
    int (*AK_transaction_register_observer) (struct observable_transaction_struct*, AK_observer*);
    int (*AK_transaction_unregister_observer) (struct observable_transaction_struct*, AK_observer*);
    void (*AK_lock_released) ();
    void (*AK_transaction_finished) ();
    void (*AK_all_transactions_finished) ();
    AK_observable *observable;
};
typedef struct observable_transaction_struct AK_observable_transaction;

/**
 * @author Ivan Pusic
 * @struct observer_lock
 * @brief Structure which defines transaction lock observer type
 */
struct observer_lock {
    AK_observer *observer;
};
typedef struct observer_lock AK_observer_lock;

/**
 * @author Frane Jakelić
 * @struct transaction_locks_list_elem
 * @brief Structure that represents LockTable entry about transaction resource lock.
 */
struct transaction_locks_list_elem {
	pthread_t TransactionId;
    int lock_type;
    int isWaiting;
    struct transaction_locks_list_elem *nextLock;
    struct transaction_locks_list_elem *prevLock;
};

/**
 * @author Frane Jakelić
 * @struct transaction_list_elem
 * @brief Structure that represents LockTable entry about transaction lock holder.Element indexed by Hash table.
 */
struct transaction_list_elem {
	int address;
    int lock_type;
    int isWaiting;
    struct transaction_locks_list_elem *DLLLocksHead;
    struct transaction_list_elem *nextBucket;
    struct transaction_list_elem *prevBucket;

    // Observer 
    AK_observer_lock *observer_lock;
};

/**
 * @author Frane Jakelić
 * @struct transaction_list_head
 * @brief Structure that represents LockTable entry about doubly linked list of collision in Hash table.
 */
struct transaction_list_head {
    struct transaction_list_elem *DLLHead;
};

/**
 * @author Frane Jakelić
 * @struct memoryAddresses
 * @brief Structure that represents a linked list of locked addresses.
 */
struct memoryAddresses{
	int adresa;
	struct memoryAddresses *nextElement;
};
/**
 * @author Frane Jakelić
 * @struct transactionData
 * @brief Structure used to transport transaction data to the thread.
 */
struct transactionData{
    int lengthOfArray;
	command *array;
};

typedef struct transactionData AK_transaction_data;
typedef struct memoryAddresses AK_memoryAddresses;
typedef struct memoryAddresses* AK_memoryAddresses_link;

typedef struct transaction_list_head AK_transaction_list;
typedef struct transaction_list_elem* AK_transaction_elem_P;
typedef struct transaction_list_elem AK_transaction_elem;

typedef struct transaction_locks_list_elem* AK_transaction_lock_elem_P;
typedef struct transaction_locks_list_elem AK_transaction_lock_elem;

 /**     
  * @author Frane Jakelić    
  * @struct threadContainer      
  * @brief Structure that represents a linked list of threads.   
  */     
 struct threadContainer{     
         pthread_t thread;   
         struct threadContainer *nextThread;     
 };      
     
     
 typedef struct threadContainer *AK_thread_elem;     
 typedef struct threadContainer AK_thread_Container;

/**
 * @author Frane Jakelić
 * @brief Function that calculates the hash value for a given memory address. Hash values are used to identify location of locked resources.
 * @todo The current implementation is very limited it doesn't cope well with collision. recommendation use some better version of hash calculation. Maybe Knuth's memory address hashing function.
 * @param blockMemoryAddress integer representation of memory address, the hash value is calculated from this parameter.
 * @return integer containing the hash value of the passed memory address
 */
int AK_memory_block_hash(int) ;

/**
 * @author Frane Jakelić
 * @brief Function that searches for a existing entry in hash list of active blocks
 * @param blockAddress integer representation of memory address.
 * @return pointer to the existing hash list entry
 */
AK_transaction_elem_P AK_search_existing_link_for_hook(int);

/**
 * @author Frane Jakelić
 * @brief Function that searches for a empty link for new active block, helper method in case of address collision
 * @param blockAddress integer representation of memory address.
 * @return pointer to empty location to store new active address
 */
AK_transaction_elem_P AK_search_empty_link_for_hook(int);


/**
 * @author Frane Jakelić
 * @brief Function that adds an element to the doubly linked list.
 * @param blockAddress integer representation of memory address.
 * @param type of lock issued to the provided memory address.
 * @return pointer to the newly created doubly linked element.
 */
AK_transaction_elem_P AK_add_hash_entry_list(int, int);

/**
 * @author Frane Jakelić
 * @brief Function that deletes a specific element in the lockTable doubly linked list.
 * @param blockAddress integer representation of memory address.
 * @return integer OK or NOT_OK based on success of finding the specific element in the list.
 */
int AK_delete_hash_entry_list(int);

/**
 * @author Frane Jakelić
 * @brief Function that searches for a specific entry in the Locks doubly linked list using the transaction id as it's key.
 * @param memoryAddress integer representation of memory address.
 * @param id integer representation of transaction id.
 * @return NULL pointer if the element is not found otherwise it returns a pointer to the found element
 */
AK_transaction_lock_elem_P AK_search_lock_entry_list_by_key(AK_transaction_elem_P, int, pthread_t);

/**
 * @author Frane Jakelić
 * @brief Function that deletes a specific entry in the Locks doubly linked list using the transaction id as it's key.
 * @param blockAddress integer representation of memory address.
 * @param id integer representation of transaction id.
 * @return int OK or NOT_OK based on success of finding the specific element in the list.
 */
int AK_delete_lock_entry_list(int, pthread_t);

/**
 * @author Frane Jakelić updated by Ivan Pusic
 * @brief Function that, based on the parameters, puts an transaction action in waiting phase or let's the transaction do it's actions.
 * @param lockHolder pointer to the hash list entry that is entitled to the specific memory address.
 * @param type of lock issued to the provided memory address.
 * @param transactionId integer representation of transaction id.
 * @param lock pointer to the lock element that is being tested.
 * @return int PASS_LOCK_QUEUE or WAIT_FOR_UNLOCK based on the rules described inside the function.
 */
int AK_isLock_waiting(AK_transaction_elem_P, int, pthread_t, AK_transaction_lock_elem_P);

/**
 * @author Frane Jakelić
 * @brief Function that adds an element to the locks doubly linked list.
 * @param memoryAddress integer representation of memory address.
 * @param type of lock issued to the provided memory address.
 * @param transactionId integer representation of transaction id.
 * @return pointer to the newly created Locks doubly linked element.
 */
AK_transaction_lock_elem_P AK_add_lock(AK_transaction_elem_P, int, pthread_t);

/**
 * @author Frane Jakelić
 * @brief Helper function that determines if there is a hash LockTable entry that corresponds to the given memory address. And if there isn't an entry the function calls for the creation of the Locks list holder.
 * @param memoryAddress integer representation of memory address.
 * @param type of lock issued to the provided memory address.
 * @param transactionId integer representation of transaction id.
 * @return pointer to the newly created Locks doubly linked element.
 */
AK_transaction_lock_elem_P AK_create_lock(int, int, pthread_t);

/**
 * @author Frane Jakelić updated by Ivan Pusic
 * @brief Main interface function for the transaction API. It is responsible for the whole process of creating a new lock.
 * @todo Implement a better deadlock detection. This method uses a very simple approach. It waits for 60sec before it restarts a transaction.
 * @param memoryAddress integer representation of memory address.
 * @param type of lock issued to the provided memory address.
 * @param transactionId integer representation of transaction id.
 * @return OK or NOT_OK based on the success of the function.
 */
int AK_acquire_lock(int, int, pthread_t);

/**
 * @author Frane Jakelić updated by Ivan Pusic
 * @brief Main interface function for the transaction API. It is responsible for the whole process releasing locks acquired by a transaction. The locks are released either by COMMIT or ABORT .
 * @param adresses linked list of memory addresses locked by the transaction.
 * @param transactionId integer representation of transaction id.
 */
void AK_release_locks(AK_memoryAddresses_link, pthread_t);

/**
 * @author Frane Jakelić
 * @brief Function that appends all addresses affected by the transaction
 * @param addressList pointer to the linked list where the addresses are stored.
 * @param tblName table name used in the transaction
 * @return OK or NOT_OK based on the success of the function.
 */
int AK_get_memory_blocks(char*, AK_memoryAddresses_link);

/**
 * @author Frane Jakelić updated by Ivan Pusic
 * @brief Function that is called in a separate thread that is responsible for acquiring locks, releasing them and finding the associated block addresses
 * @todo Check multithreading, check if it's working correctly
 * @param commandArray array filled with commands that need to be secured using transactions
 * @param lengthOfArray length of commandArray
 * @param transactionId associated with the transaction
 * @return ABORT or COMMIT based on the success of the function.
 */
int AK_execute_commands(command* , int);

/**
 * @author Frane Jakelić updated by Ivan Pusic
 * @brief Function that is the thread start point all relevant functions. It acts as an intermediary between the main thread and other threads
 * @param data transmitted to the thread from the main thread
 */
void * AK_execute_transaction(void*);

/**
 * @author Frane Jakelić updated by Ivan Pusic
 * @brief Function that receives all the data and gives an id to that data and starts a thread that executes the transaction 
 * @param commandArray array filled with commands that need to be secured using transactions
 * @param lengthOfArray length of commandArray
 */
int AK_transaction_manager(command*, int);
TestResult AK_test_Transaction();

/** 
 * @author Ivan Pusic
 * @brief Function for creating new thread. Function also adds thread ID to pthread_t array
 * @param transaction_data Data for executing transaction
 *
 * @return Exit status (OK or NOT_OK)
 */
int AK_create_new_transaction_thread(AK_transaction_data*);
int AK_remove_transaction_thread(pthread_t);
void handle_transaction_notify(AK_observer_lock*);

/** 
 * @author Ivan Pusic
 * @brief Function for handling notify from some observable type
 * @param observer Observer type
 * @param observable Observable type
 * @param type Type of observable who sent some notice
 */
void AK_on_observable_notify(void*, void*, AK_ObservableType_Enum);

/** 
 * @author Ivan Pusic
 * @brief Function for handling event when some transaction is finished
 * @param transaction_thread Thread ID of transaction which is finished
 */
void AK_on_transaction_end(pthread_t);

/** 
 * @author Ivan Pusic
 * @brief Function for handling event when one of lock is released
 */
void AK_on_lock_release();

/** 
 * @author Ivan Pusic
 * @brief Function for handling  event when all transactions are finished
 */
void AK_on_all_transactions_end();

/** 
 * @author Ivan Pusic
 * @brief Function for handling action which is called from observable_transaction type
 * @param noticeType Type of action (event)
 */
void AK_handle_observable_transaction_action(NoticeType*);

/** 
 * @author Ivan Pusic
 * @brief Function which is called when the lock is released
 */
void AK_lock_released();
/** 
 * @author Ivan Pusic
 * @brief Function that is called when some transaction is finished
 */
void AK_transaction_finished();

/** 
 * @author Ivan Pusic
 * @brief Function that is called when all transactions are finished
 */
void AK_all_transactions_finished();

/** 
 * @author Ivan Pusic
 * @brief Function for registering new observer of AK_observable_transaction type
 * @param observable_transaction Observable type instance
 * @param observer Observer instance
 * 
 * @return Exit status (OK or NOT_OK)
 */
int AK_transaction_register_observer(AK_observable_transaction*, AK_observer*);

/** 
 * @author Ivan Pusic
 * @brief Function for unregistering observer from AK_observable_transction type
 * @param observable_transaction Observable type instance
 * @param observer Observer instance
 * 
 * @return Exit status (OK or NOT_OK)
 */
int AK_transaction_unregister_observer(AK_observable_transaction*, AK_observer*);

/** 
 * @author Ivan Pusic
 * @brief Function for initialization of AK_observable_transaction type
 * 
 * @return Pointer to new AK_observable_transaction instance
 */
AK_observable_transaction * AK_init_observable_transaction();

/** 
 * @author Ivan Pusic
 * @brief Function for initialization of AK_observer_lock type
 * @return Pointer to new AK_observer_lock instance
 */
AK_observer_lock * AK_init_observer_lock();

#endif /* TRANSACTION_H_ */
