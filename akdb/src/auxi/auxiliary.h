/**
@file auxiliary.h Header file that provides a data structure for the auxiliary functions
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


#ifndef AUXILIARY
#define AUXILIARY

#include "constants.h"
#include "configuration.h"
#include "test.h"

#include "assert.h"
#include "time.h"
#include "string.h"
#include "ctype.h"
#include "debug.h"
#include "mempro.h"

#define MAX_LOOP_ITERATIONS 1000
//#include "ini_parser/iniparser.h"

/**
 * @author Domagoj Šitum
 * @var testMode
 * @brief You can turn testMode on or off with TEST_MODE_ON and TEST_MODE_OFF.
 * To do this, simply enable or disable it in YOUR function (not in any other!)
 * Test mode can be used when you need some special cases in your functions
 * (i.e., when you are testing some functionality, which doesn't apply in normal conditions).
 * But don't forget to turn this mode off, after you are done (within test function for example)!
 */
int testMode;


/**
 * @author Aleksandra Polak
 * @brief Function that change type of argument from string to integer
 * @param *arg_type type of an argument
 * @return EXIT_SUCCESS of the function (return type of argument in value of integer) or EXIT_ERROR
*/
char* AK_convert_type(char* arg_type);


/**
 * @author Ljiljana Pintarić
 * @struct list_node
 * @brief Structure defines a list node.
 */
struct list_node { 
///TODO - type, attribute name, table staviti na početak polja data
		///data type
    int type;
    //data size in list element
    int size;
    ///loaded data
    char data[ MAX_VARCHAR_LENGTH ]; 
     //table name
    char table[ MAX_ATT_NAME ];
    //attribute name
    char attribute_name[ MAX_ATT_NAME ];
     int constraint; //if 0 then it is new data for insertion, if 1 then it is a constraint on which update and delete searches the data
    ///pointer to next element
    struct list_node *next;
};

typedef struct list_node AK_list;
typedef struct list_node *AK_list_elem;

#define TBL_BOX_OFFSET 1
//#define TBL_COL_DELIMITER '|'
//#define TBL_ROW_DELIMITER '-'

/*
enum tbl_valalign {
        ALIGN_LEFT  0,
        ALIGN_RIGHT 1;
};*/

int AK_strcmp(const void *a, const void *b);

/**
 * @author Frane Jakelić
 * @struct Vertex
 * @brief Structure defines a Vertex node element. Every Vertex has its VertexId, index, lowLink and pointer to next edge and vertex
 */
struct Vertex{
    int vertexId;
    int index;
    int lowLink;
    struct Succesor *nextSuccesor;
    struct Vertex *nextVertex;
};

/**
 * @author Frane Jakelić
 * @struct Succesor
 * @brief Structure defines a Succesor  element. Every Succesor has its Vertex pointer and pointer to next Succesor in the linked list
 */
struct Succesor{
	struct Vertex *link;
	struct Succesor *nextSuccesor;
};

/**
 * @author Frane Jakelić
 * @struct Stack
 * @brief Structure defines a Stack element. Every Stack has its Vertex pointer and pointer to next Stack in the linked list
 */
struct Stack{
	struct Vertex *link;
	struct Stack *nextElement;
};

typedef struct Vertex AK_graph;
typedef struct Succesor *AK_succesor;
typedef struct Vertex *AK_vertex;
typedef struct Stack *AK_stack;
typedef struct Stack AK_stackHead;

/**
 * @author Marko Sinko
 * @struct AK_synchronization_info
 * @brief Structure for managing the synchronization between multiple threads
 *        accessing the same resources (essentially a mutex).
 */
typedef struct {
    int init;
    int ready;
#ifdef _WIN32
    CRITICAL_SECTION critical_section;
#endif
#ifdef __linux__
    pthread_mutex_t mutex;
#endif
} AK_synchronization_info;

/**
 * @author Blaž Rajič
 * @brief Function for creating graph for testing tarjan algorithm
 * @param graph AK_graph where graph will be created
 */
void AK_define_tarjan_graph();

/**
 * @author Dino Laktašić.
 * @brief Function that gets the number of digits for any given number
 * @param number number to evaluate
 * @param int base mathematic base (e.g. 2, 10 etc.)
 * @return the number of digits for the given number
 */
int AK_chars_num_from_number(int number, int base);

/**
 * @author Miroslav Policki
 * @brief Function returns the size in bytes for the provided database type
 * @param iDB_type database data type (defined in constants.h)
 * @param szVarchar if iDB_type == TYPE_VARCHAR, pointer to the string, otherwise unused
 * @return size of provided data type in bytes if the provided data type is valid, else return 0
 */
size_t AK_type_size(int iDB_type, char *szVarchar);

/**
 * @author Dino Laktašić
 * @brief Function compares two Strings
 * @param *a  pointer of a value to compare
 * @param *b pointer of a value to compare
 * @return result of the comparison in line with strcmp function
 */
int AK_strcmp(const void *a, const void *b);
/**
 * @author Ljiljana Pintarić
 * @brief  Function that initializes an empty list
 * @param L root of the list
 * @return NO return value
 */
void AK_Init_L3(struct list_node **L);
/**
 * @author Ljiljana Pintarić.
 * @brief  Function that fetches the first element of the list
 * @param L root of the list
 * @return first element of the list
 */
struct list_node *AK_First_L2(struct list_node *L);
/**
 * @author Ljiljana Pintarić.
 * @brief  Function that fetches the last element of the list
 * @param L  root of the list
 * @return last element of the list
 */
struct list_node *AK_End_L2(struct list_node *L);
/**
 * @author Ljiljana Pintarić.
 * @brief  Function that fetches the next element of the list
 * @param current  current element of the list
 * @return next element of the list
 */
struct list_node *AK_Next_L2(struct list_node *current);
/**
 * @author Ljiljana Pintarić.
 * @brief  Function that fetches the previous element of the list
 * @param current current element of the list
 * @param L root of the list
 * @return previous element of the list
 */

struct list_node *AK_Previous_L2(struct list_node *current, struct list_node *L);
/**
 * @author Ljiljana Pintarić.
 * @brief  Function that tests if the list is empty
 * @param L root of the list
 * @return 1 if the list is empty, otherwise returns 0
 */
unsigned int AK_IsEmpty_L2(struct list_node *L);
/**
 * @author Ljiljana Pintarić.
 * @brief  Function that inserts a new element before the current element of the list.
 * @param data new data
 * @param current current element of the list
 * @param L root of the list
 * @return No return value
 */
void AK_InsertBefore_L2(int type, char* data, int size, struct list_node **current, struct list_node **L);

/**
 * @author Ljiljana Pintarić.
 * @brief  Function that inserts a new element after the current element of the list
 * @param data new data
 * @param current current element of the list
 * @param L root of the list
 * @return No return value.
 */
void AK_InsertAfter_L2(int type, char* data, int size, struct list_node **current,  struct list_node **L);

/**
 * @author Ljiljana Pintarić.
 * @brief  Function that inserts a new element at the beginning of the list. It uses function called: AK_InsertBefore_L
 * @param data new data
 * @param L root of the list
 * @return No return value
 */

void AK_InsertAtBegin_L3(int type, char* data, int size, struct list_node *L);
/**
 * @author Ljiljana Pintarić.
 * @brief  Function that inserts a new element at the end of the list. It uses a function called: AK_InsertAfter_L2
 * @param data new data
 * @param L root of the list
 * @return No return value.
 */
void AK_InsertAtEnd_L3(int type, char* data, int size, struct list_node *L);
/**
 * @author Ljiljana Pintarić.
 * @brief  Function that deletes the current element of the list
 * @param current current element of the list
 * @param L root of the list
 * @retrun No return value
 */
void AK_Delete_L3(struct list_node **current, struct list_node **L);
/**
 * @author Ljiljana Pintarić.
 * @brief  Function that empties the list
 * @param L root of the list
 * @return No return value
 */
void AK_DeleteAll_L3(struct list_node **L);
/**
 * @author Ljiljana Pintarić.
 * @brief  Function that fetches the number of the elements in the list.
 * @param L root of the list
 * @return Size of the list
 */
int AK_Size_L2(struct list_node *L);
/**
 * @author Ljiljana Pintarić.
 * @brief  Function that retrieves the data from the current element of the list
 * @param current current element of the list
 * @param L root of the list
 * @return data from the list element
 */
char* AK_Retrieve_L2(struct list_node *current, struct list_node *L);

//int AK_GetType_L(AK_list_elem current, AK_list *L);
//int AK_GetSize_L(AK_list_elem current, AK_list *L);

/**
 * @author Ljiljana Pintarić
 * @brief Function that fetches the nth element in a row
 * @param pos position of element in a row
 * @param row list of elements of a row in the table
 * @return element of list of elements of a row in the table

 */
struct list_node * AK_GetNth_L2(int pos, struct list_node *row);

//int AK_Get_Position_Of_Element(AK_list_elem SearchedElement, AK_list *L);

/**
 * @author Dino Laktašić.
 * @brief  Get all permutations without repetition (currently not used, but it can be helpful)
 * @param arr array of chars to perform permutation on
 * @return char pointer to an array of pointers pointing to permuted char arrays
 */
char *AK_get_array_perms(char *arr) ;

/**
 * @author Frane Jakelić
 * @brief Function that searches for a specific graph node by its ID
 * @param id of the vertex that needs to be found
 * @param graphRoot root node of the graph structure
 * @return found graph nod or null
 */
AK_vertex AK_search_vertex(int id);

/**
 * @author Frane Jakelić
 * @brief Looks for empty link for a new graph node
 * @param graphRoot oot node of the graph structure
 * @return empty link for a new graph node
 */
AK_vertex AK_search_empty_link() ;

/**
 * @author Frane Jakelić
 * @brief Function that adds a new graph node
 * @param id of the vertex that needs to be added
 * @param graphRoot root node of the graph structure
 * @return pointer to the newly created node
 */
AK_vertex AK_add_vertex(int id);

/**
 * @author Frane Jakelić
 * @brief Creates an edge between two nodes
 * @param succesorId id of a newly created edge
 * @param succesorOf source of the newly created edge
 * @return pointer to the newly created edge
 */
AK_succesor AK_add_succesor(int succesorId, int succesorOf);

/**
 * @author Frane Jakelić
 * @brief Returns a empty link for the stack
 * @param stackRoot root node of the selected stack
 * @return pointer to the empty link
 */
AK_stack AK_search_empty_stack_link(AK_stack stackRoot);

/**
 * @author Frane Jakelić
 * @brief Adds a entry to the stack
 * @param id of the element that is being added to the stack
 * @return pointer to the newly added stack node
 */
AK_stack AK_push_to_stack(int id);

/**
 * @author Frane Jakelić
 * @brief Pops a entry to the stack
 * @return pointer to the popped stack node
 */
AK_stack AK_pop_from_stack();

/**
 * @author Frane Jakelić
 * @brief Finds an element in the stack
 * @param id of the node that needs to be found in the stack
 * @return pointer to the found stack node
 */
AK_stack AK_search_in_stack(int id);
int MIN(int X, int Y);

/**
 * @author Frane Jakelić
 * @brief Tarjan algorithm that looks for a strongly connected component inside all subgraphs; using DFS
 * @param id of the element on which the algorithm looks for an id of a strongly connected component
 */
void AK_tarjan(int id);
TestResult AK_tarjan_test();

/**
 * @author Marko Sinko
 * @brief Initializes an AK_synchronization_info structure and returns an owned
 *        pointer that must later be passed on to AK_destroy_critical_section.
 * @return Initialized synchronization object
 */
AK_synchronization_info* AK_init_critical_section();

/**
 * @author Marko Sinko
 * @param info Synchronization info structure
 * @brief Destroys a synchronization object when it is no longer necessary and
 *        frees the pointer.
 * @return void
 */
void AK_destroy_critical_section(AK_synchronization_info* info);

/**
 * @author Marko Sinko
 * @param info Synchronization info structure
 * @brief Enters a critical section.
 * @return void
 */
void AK_enter_critical_section(AK_synchronization_info* info);

/**
 * @author Marko Sinko
 * @param info Synchronization info structure
 * @brief Leaves a critical section
 * @return void
 */
void AK_leave_critical_section(AK_synchronization_info* info);

//void AK_copy_L_Ele(AK_list_elem srcElem, AK_list_elem destElem);
//void AK_copy_L(AK_list *src, AK_list *dest) ;
//int AK_compare_L(AK_list *srcInput, AK_list *srcOriginal);


#endif
