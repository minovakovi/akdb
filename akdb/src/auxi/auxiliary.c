/**
@file auxiliary.c Provides auxiliary functions
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

#include "auxiliary.h"
int testMode = 0;
AK_graph G;
AK_stackHead S;
int indexCounter = 0;
int testLowArray[8];

/**
 * @author Dino Laktašić.
 * @brief Function that gets the number of digits for any given number
 * @param number number to evaluate
 * @param int base mathematic base (e.g. 2, 10 etc.)
 * @return the number of digits for the given number
 */
int AK_chars_num_from_number(int number, int base)
{
  int len = 0;
  AK_PRO;

  do
  {
    len++;
  } while ((double)(number /= base) > 0);

  AK_EPI;
  return len;
}

/**
 * @author Aleksandra Polak
 * @brief Function that changes the type of argument from a string to an integer
 * @param *arg_type type of  argument
 * @return EXIT_SUCCESS of the function (return type of argument as a value of
 * the integer) or EXIT_ERROR
 */
char *AK_convert_type(char *arg_type)
{
  AK_PRO;
  if (arg_type == NULL)
  {
    AK_EPI;
    return "-1";
  }
  if (strcmp(arg_type, "internal") == 0)
  {
    AK_EPI;
    return "0";
  }
  if (strcmp(arg_type, "int") == 0)
  {
    AK_EPI;
    return "1";
  }
  if (strcmp(arg_type, "float") == 0)
  {
    AK_EPI;
    return "2";
  }
  if (strcmp(arg_type, "number") == 0)
  {
    AK_EPI;
    return "3";
  }
  if (strcmp(arg_type, "varchar") == 0)
  {
    AK_EPI;
    return "4";
  }
  if (strcmp(arg_type, "date") == 0)
  {
    AK_EPI;
    return "5";
  }
  if (strcmp(arg_type, "datetime") == 0)
  {
    AK_EPI;
    return "6";
  }
  if (strcmp(arg_type, "time") == 0)
  {
    AK_EPI;
    return "7";
  }
  if (strcmp(arg_type, "blob") == 0)
  {
    AK_EPI;
    return "8";
  }
  if (strcmp(arg_type, "bool") == 0)
  {
    AK_EPI;
    return "9";
  }
  if (strcmp(arg_type, "operand") == 0)
  {
    AK_EPI;
    return "10";
  }
  if (strcmp(arg_type, "operator") == 0)
  {
    AK_EPI;
    return "11";
  }
  if (strcmp(arg_type, "attribs") == 0)
  {
    AK_EPI;
    return "12";
  }
  if (strcmp(arg_type, "condition") == 0)
  {
    AK_EPI;
    return "13";
  }
  AK_EPI;
  return NULL;
}

/**
 * @author Miroslav Policki
 * @brief Function returns the size in bytes for the provided database type
 * @param iDB_type database data type (defined in constants.h)
 * @param szVarchar if iDB_type == TYPE_VARCHAR, pointer to the string,
 * otherwise unused
 * @return size of provided data type in bytes if the provided data type is
 * valid, else return 0
 */
size_t AK_type_size(int iDB_type, char *szVarchar)
{
  AK_PRO;
  switch (iDB_type)
  {
  case TYPE_INTERNAL:
    AK_EPI;
    return sizeof(int);
  case TYPE_INT:
    AK_EPI;
    return sizeof(int);
  case TYPE_FLOAT:
    AK_EPI;
    return sizeof(double);
  case TYPE_NUMBER:
    AK_EPI;
    return sizeof(double);
  case TYPE_VARCHAR:
    AK_EPI;
    return (size_t)strlen(szVarchar);
  case TYPE_DATE:
    AK_EPI;
    return sizeof(int);
  case TYPE_DATETIME:
    AK_EPI;
    return sizeof(int);
  case TYPE_TIME:
    AK_EPI;
    return sizeof(int);
  case TYPE_INTERVAL:
    AK_EPI;
    return sizeof(int);
  case TYPE_PERIOD:
    AK_EPI;
    return sizeof(int);
  case TYPE_BLOB:
    AK_EPI;
    return sizeof(void *);
  case TYPE_BOOL:
    AK_EPI;
    return (size_t)1;
  default:
    AK_EPI;
    return (size_t)0;
  }
}

/**
 * @author Dino Laktašić
 * @brief Function compares two Strings
 * @param *a  pointer of a value to compare
 * @param *b pointer of a value to compare
 * @return result of the comparison in line with strcmp function
 */
int AK_strcmp(const void *a, const void *b)
{
  const char **ia = (const char **)a;
  const char **ib = (const char **)b;
  AK_PRO;
  AK_EPI;
  return strcmp(*ia, *ib);
}

/**
 * @author Ljiljana Pintarić
 * @brief  Function that initializes an empty list
 * @param L root of the list
 * @return NO return value
 */

void AK_Init_L3(struct list_node **L)
{
  AK_PRO;
  assert(L != NULL);
  (*L)->next = NULL;
  AK_EPI;
}

/**
 * @author Ljiljana Pintarić.
 * @brief  Function that fetches the first element of the list
 * @param L root of the list
 * @return first element of the list
 */

struct list_node *AK_First_L2(struct list_node *L)
{
  AK_PRO;
  if (L == NULL)
  {
    AK_EPI;
    return NULL;
  }
  AK_EPI;
  return L->next;
}

/**
 * @author Ljiljana Pintarić.
 * @brief  Function that fetches the last element of the list
 * @param L  root of the list
 * @return last element of the list
 */

struct list_node *AK_End_L2(struct list_node *L)
{
  AK_PRO;
  struct list_node *current;
  current = AK_First_L2(L);

  if (current == NULL)
  {
    AK_EPI;
    return NULL;
  }

  int counter = 0;
  while (current->next != NULL)
  {
    current = current->next;
    counter++;
    assert(counter < MAX_LOOP_ITERATIONS);
  }

  AK_EPI;
  return current;
}

/**
 * @author Ljiljana Pintarić.
 * @brief  Function that fetches the next element of the list
 * @param current  current element of the list
 * @return next element of the list
 */

struct list_node *AK_Next_L2(struct list_node *current)
{
  AK_PRO;
  if (current == NULL)
  {
    AK_EPI;
    return NULL;
  }
  AK_EPI;
  return current->next;
}

/**
 * @author Ljiljana Pintarić.
 * @brief  Function that fetches the previous element of the list
 * @param current current element of the list
 * @param L root of the list
 * @return previous element of the list
 */

struct list_node *AK_Previous_L2(struct list_node *current,
                                 struct list_node *L)
{
  AK_PRO;

  if (current == NULL || current == L)
  {
    AK_EPI;
    return NULL;
  }

  struct list_node *previous = L;
  int counter = 0;
  while (previous->next != current)
  {
    previous = previous->next;
    counter++;
    assert(counter < MAX_LOOP_ITERATIONS);
  }

  AK_EPI;
  return previous;
}

/**
 * @author Ljiljana Pintarić.
 * @brief  Function that tests if the list is empty
 * @param L root of the list
 * @return 1 if the list is empty, otherwise returns 0
 */

unsigned int AK_IsEmpty_L2(struct list_node *L)
{
  AK_PRO;
  unsigned int ret;
  ret = (AK_First_L2(L) == NULL ? 1 : 0);
  AK_EPI;
  return ret;
}

/**
 * @author Ljiljana Pintarić.
 * @brief  Function that inserts a new element before the current element of the
 * list.
 * @param data new data
 * @param current current element of the list
 * @param L root of the list
 * @return No return value
 */

void AK_InsertBefore_L2(int type, char *data, int size,
                        struct list_node **current, struct list_node **L)
{
  AK_PRO;
  struct list_node *new_elem;

  new_elem = (struct list_node *)AK_malloc(sizeof(struct list_node));
  new_elem->size = size;
  new_elem->type = type;

  memcpy(new_elem->data, data, MAX_VARCHAR_LENGTH);

  if ((*current) == NULL)
  {
    (*L)->next = new_elem;
    new_elem->next = NULL;
    AK_EPI;
    return;
  }
  struct list_node *previous = AK_Previous_L2((*current), (*L));
  previous->next = new_elem;
  new_elem->next = (*current);
  AK_EPI;
}

/**
 * @author Ljiljana Pintarić.
 * @brief  Function that inserts a new element after the current element of the
 * list
 * @param data new data
 * @param current current element of the list
 * @param L root of the list
 * @return No return value.
 */

void AK_InsertAfter_L2(int type, char *data, int size,
                       struct list_node **current, struct list_node **L)
{

  AK_PRO;
  struct list_node *new_elem;

  new_elem = (struct list_node *)AK_malloc(sizeof(struct list_node));
  new_elem->size = size;
  new_elem->type = type;
  memcpy(new_elem->data, data, MAX_VARCHAR_LENGTH);
  if ((*current) == NULL)
  {
    (*L)->next = new_elem;
    new_elem->next = NULL;
    AK_EPI;
    return;
  }
  new_elem->next = (*current)->next;
  (*current)->next = new_elem;
  AK_EPI;
}

/**
 * @author Ljiljana Pintarić.
 * @brief  Function that inserts a new element at the beginning of the list. It
 * uses function called: AK_InsertBefore_L
 * @param data new data
 * @param L root of the list
 * @return No return value
 */

void AK_InsertAtBegin_L3(int type, char *data, int size, struct list_node *L)
{
  AK_PRO;
  assert(L != NULL);

  struct list_node *current = AK_First_L2(L);
  current = AK_First_L2(L);
  AK_InsertBefore_L2(type, data, size, &current, &L);

  AK_EPI;
}

/**
 * @author Ljiljana Pintarić.
 * @brief  Function that inserts a new element at the end of the list. It uses a
 * function called: AK_InsertAfter_L2
 * @param data new data
 * @param L root of the list
 * @return No return value.
 */

void AK_InsertAtEnd_L3(int type, char *data, int size, struct list_node *L)
{
  AK_PRO;
  assert(L != NULL);
  struct list_node *current;
  current = NULL;
  current = AK_End_L2(L);
  AK_InsertAfter_L2(type, data, size, &current, &L);

  AK_EPI;
}

/**
 * @author Ljiljana Pintarić.
 * @brief  Function that deletes the current element of the list
 * @param current current element of the list
 * @param L root of the list
 * @retrun No return value
 */

void AK_Delete_L3(struct list_node **current, struct list_node **L)
{
  AK_PRO;
  if ((*current) == NULL || (*L) == NULL)
  {
    AK_EPI;
    return;
  }
  struct list_node *previous = AK_Previous_L2((*current), (*L));
  struct list_node *next = (*current)->next; // NextL( current, L );

  previous->next = next;
  AK_free((*current));
  AK_EPI;
}

/**
 * @author Ljiljana Pintarić.
 * @brief  Function that empties the list
 * @param L root of the list
 * @return No return value
 */

void AK_DeleteAll_L3(struct list_node **L)
{
  AK_PRO;

  struct list_node *current;
  if ((*L) == NULL)
  {
    AK_EPI;
    return;
  }

  int counter = 0;
  while ((current = AK_First_L2(*L)) != NULL)
  {
    (*L)->next = current->next;
    AK_free(current);
    counter++;
    assert(counter < MAX_LOOP_ITERATIONS);
  }

  AK_EPI;
}

/**
 * @author Ljiljana Pintarić.
 * @brief  Function that fetches the number of the elements in the list.
 * @param L root of the list
 * @return Size of the list
 */

int AK_Size_L2(struct list_node *L)
{
  AK_PRO;

  int size = 0;
  struct list_node *current;

  current = AK_First_L2(L);

  int counter = 0;
  while (current)
  {
    size++;
    current = AK_Next_L2(current);
    counter++;
    assert(counter < MAX_LOOP_ITERATIONS);
  }
  AK_EPI;
  return size;
}

/**
 * @author Ljiljana Pintarić.
 * @brief  Function that retrieves the data from the current element of the list
 * @param current current element of the list
 * @param L root of the list
 * @return data from the list element
 */

char *AK_Retrieve_L2(struct list_node *current, struct list_node *L)
{
  char *data;
  AK_PRO;

  if (current == NULL || L == NULL)
  {
    return NULL;
  }

  data = (char *)AK_malloc(MAX_VARCHAR_LENGTH);
  memcpy(data, current->data, MAX_VARCHAR_LENGTH);

  AK_EPI;
  return data;
}

/**
 * @author Matija Šestak.
 * @brief  Function that fetches a type of the current element of the list
 * @param current current list element
 * @param L root of the list
 * @return data type of the current list element
 */

// 0 uses found
/*
int AK_GetType_L(AK_list_elem current, AK_list *L) {
    int ret;
    AK_PRO;
    ret = (current == NULL) ? 0 : current->type;
    AK_EPI;
    return ret;
}
*/

/**
 * @author Matija Šestak.
 * @brief  Function that fetches the data size of the element
 * @param current current list element
 * @param L - root of the list
 * @return data size of the current list element
 */
// 0 uses found
/*
int AK_GetSize_L(AK_list_elem current, AK_list *L) {
    int ret;
    AK_PRO;
    ret = (current == NULL) ? 0 : current->size;
    AK_EPI;
    return ret;
}
*/

/**
 * @author Ljiljana Pintarić
 * @brief Function that fetches the nth element in a row
 * @param pos position of element in a row
 * @param row list of elements of a row in the table
 * @return element of list of elements of a row in the table

 */

struct list_node *AK_GetNth_L2(int pos, struct list_node *row)
{
  AK_PRO;
  int i;
  struct list_node *temp_elem;

  assert(pos > -1);
  temp_elem = AK_First_L2(row);
  for (i = 1; temp_elem != NULL; ++i)
  {
    if (i == pos)
    {
      AK_EPI;
      return temp_elem;
    }
    temp_elem = AK_Next_L2(temp_elem);
  };
  AK_EPI;
  return NULL;
}

/**
 * @author Matija Novak
 * @brief Function that fetches the position of a given element
 * @param SearchElement element whose posititon we search for
 * @param L root of the list
 * @return returns the posititon number of some elelemnt
 */
// 0 uses found
/*
int AK_Get_Position_Of_Element(AK_list_elem SearchedElement, AK_list *L) {
    AK_list *CurrentElement;
    int i = 0;
    AK_PRO;
    CurrentElement = L;
    while (CurrentElement->next != 0 && CurrentElement != SearchedElement) {
        CurrentElement = (AK_list *) CurrentElement->next;
        i++;
    }
    AK_EPI;
    return i;
}
*/

/**
 * @author Dino Laktašić.
 * @brief  Get all permutations without repetition (currently not used, but it
 * can be helpful)
 * @param arr array of chars to perform permutation on
 * @return char pointer to an array of pointers pointing to permuted char arrays
 */
char *AK_get_array_perms(char *arr)
{
  int div, num_perms = 1;
  int next_perm, next_chr;
  int len_arr = strlen(arr);
  AK_PRO;
  // Get total permutation without repetition number
  for (next_chr = 1; next_chr <= len_arr; num_perms *= next_chr++)
    ;

  char *res_perm, *perms[num_perms];

  for (next_perm = 0; next_perm < num_perms; next_perm++)
  {
    char *perm = (char *)AK_calloc(len_arr, sizeof(char));
    memcpy(perm, arr, len_arr);

    res_perm = (char *)AK_calloc(len_arr, sizeof(char));

    for (next_chr = len_arr, div = num_perms; next_chr > 0; next_chr--)
    {
      div /= next_chr;
      int index = (next_perm / div) % next_chr;
      // printf("%c", perm[index] );

      memcpy(res_perm + (len_arr - next_chr), &perm[index], 1);
      memmove(perm + index, perm + index + 1, len_arr - index);
    }
    // printf("\n");

    perms[next_perm] = res_perm;
    // printf("%s\n", perms[next_perm]);

    AK_free(perm);
    // perm = NULL;
  }

  res_perm = (char *)AK_calloc(num_perms, sizeof(char *));
  memcpy(res_perm, perms, sizeof(perms));
  AK_EPI;
  return res_perm;
}

/**
 * @author Frane Jakelić
 * @brief Function that searches for a specific graph node by its ID
 * @param id of the vertex that needs to be found
 * @param graphRoot root node of the graph structure
 * @return found graph nod or null
 */
AK_vertex AK_search_vertex(int id)
{
  AK_vertex tmp;
  AK_PRO;
  tmp = &G;

  while (tmp->nextVertex != NULL)
  {
    if (tmp->nextVertex->vertexId == id)
    {
      AK_EPI;
      return tmp->nextVertex;
    }
    tmp = tmp->nextVertex;
  }
  AK_EPI;
  return NULL;
}
/**
 * @author Frane Jakelić
 * @brief Looks for empty link for a new graph node
 * @param graphRoot oot node of the graph structure
 * @return empty link for a new graph node
 */
AK_vertex AK_search_empty_link()
{

  AK_vertex tmp;
  AK_PRO;
  tmp = &G;

  while (tmp->nextVertex != NULL)
  {
    tmp = tmp->nextVertex;
  }
  AK_EPI;
  return tmp;
}

/**
 * @author Frane Jakelić
 * @brief Function that adds a new graph node
 * @param id of the vertex that needs to be added
 * @param graphRoot root node of the graph structure
 * @return pointer to the newly created node
 */
AK_vertex AK_add_vertex(int id)
{
  AK_vertex node;
  AK_PRO;
  node = (AK_vertex)AK_malloc(sizeof(struct Vertex));
  memset(node, 0, sizeof(struct Vertex));

  AK_search_empty_link(&G)->nextVertex = node;
  node->vertexId = id;
  node->index = -1;
  node->lowLink = -1;
  AK_EPI;
  return node;
}
/**
 * @author Frane Jakelić
 * @brief Creates an edge between two nodes
 * @param succesorId id of a newly created edge
 * @param succesorOf source of the newly created edge
 * @return pointer to the newly created edge
 */
AK_succesor AK_add_succesor(int succesorId, int succesorOf)
{
  AK_succesor edge;
  AK_vertex root;
  AK_succesor suc;
  AK_PRO;
  edge = (AK_succesor)AK_malloc(sizeof(struct Succesor));
  memset(edge, 0, sizeof(struct Succesor));
  root = AK_search_vertex(succesorOf);
  suc = root->nextSuccesor;

  if (root->nextSuccesor == NULL)
  {
    root->nextSuccesor = edge;
  }
  else
  {
    while (suc->nextSuccesor != NULL)
    {
      suc = suc->nextSuccesor;
    }
    suc->nextSuccesor = edge;
  }

  edge->link = AK_search_vertex(succesorId);
  AK_EPI;
  return edge;
}

/**
 * @author Frane Jakelić
 * @brief Returns a empty link for the stack
 * @param stackRoot root node of the selected stack
 * @return pointer to the empty link
 */
AK_stack AK_search_empty_stack_link(AK_stack stackRoot)
{

  AK_stack tmp;
  AK_PRO;
  tmp = stackRoot;

  while (tmp->nextElement != NULL)
  {
    tmp = tmp->nextElement;
  }
  AK_EPI;
  return tmp;
}
/**
 * @author Frane Jakelić
 * @brief Adds a entry to the stack
 * @param id of the element that is being added to the stack
 * @return pointer to the newly added stack node
 */
AK_stack AK_push_to_stack(int id)
{

  AK_stack node;
  AK_PRO;
  node = (AK_stack)AK_malloc(sizeof(struct Stack));
  memset(node, 0, sizeof(struct Stack));

  AK_search_empty_stack_link(&S)->nextElement = node;
  node->link = AK_search_vertex(id);
  AK_EPI;
  return node;
}

/**
 * @author Frane Jakelić
 * @brief Pops a entry to the stack
 * @return pointer to the popped stack node
 */
AK_stack AK_pop_from_stack()
{

  AK_stack node;
  AK_stack tmp;
  AK_PRO;
  node = AK_search_empty_stack_link(&S);
  tmp = &S;

  if (node == tmp)
  {
    AK_EPI;
    return NULL;
  }
  while (tmp->nextElement != node)
  {
    tmp = tmp->nextElement;
  }
  tmp->nextElement = NULL;
  AK_EPI;
  return node;
}
/**
 * @author Frane Jakelić
 * @brief Finds an element in the stack
 * @param id of the node that needs to be found in the stack
 * @return pointer to the found stack node
 */
AK_stack AK_search_in_stack(int id)
{

  AK_stack tmp;
  AK_PRO;
  tmp = &S;

  while (tmp->nextElement != NULL)
  {
    if (tmp->nextElement->link->vertexId == id)
    {
      AK_EPI;
      return tmp->nextElement;
    }
    tmp = tmp->nextElement;
  }
  AK_EPI;
  return NULL;
}

int MIN(int X, int Y)
{
  int ret;
  AK_PRO;
  ret = X > Y ? Y : X;
  AK_EPI;
  return ret;
}

/**
 * @author Blaž Rajič
 * @brief Function for creating graph for testing tarjan algorithm
 */
void AK_define_tarjan_graph()
{
  AK_vertex node0;
  AK_vertex node1;
  AK_vertex node2;
  AK_vertex node3;
  AK_vertex node4;
  AK_vertex node5;
  AK_vertex node6;
  AK_vertex node7;

  AK_succesor node0_succ;
  AK_succesor node1_succ;
  AK_succesor node2_succ;
  AK_succesor node3_succ;
  AK_succesor node4_succ;
  AK_succesor node5_succ;
  AK_succesor node6_succ;
  AK_succesor node7_succ;

  AK_PRO;

  node0 = AK_add_vertex(0);
  node1 = AK_add_vertex(1);
  node2 = AK_add_vertex(2);
  node3 = AK_add_vertex(3);
  node4 = AK_add_vertex(4);
  node5 = AK_add_vertex(5);
  node6 = AK_add_vertex(6);
  node7 = AK_add_vertex(7);

  node0_succ = AK_add_succesor(node1->vertexId, node0->vertexId);
  node1_succ = AK_add_succesor(node2->vertexId, node1->vertexId);
  node2_succ = AK_add_succesor(node0->vertexId, node2->vertexId);
  AK_add_succesor(node3->vertexId, node2->vertexId);
  node3_succ = AK_add_succesor(node4->vertexId, node3->vertexId);
  AK_add_succesor(node5->vertexId, node3->vertexId);
  node4_succ = AK_add_succesor(node3->vertexId, node4->vertexId);
  node5_succ = AK_add_succesor(node6->vertexId, node5->vertexId);
  node6_succ = AK_add_succesor(node7->vertexId, node6->vertexId);
  node7_succ = AK_add_succesor(node5->vertexId, node7->vertexId);

  node0->nextVertex = node1;
  node0->nextSuccesor = node0_succ;
  node1->nextVertex = node2;
  node1->nextSuccesor = node1_succ;
  node2->nextVertex = node3;
  node2->nextSuccesor = node2_succ;
  node3->nextVertex = node4;
  node3->nextSuccesor = node3_succ;
  node4->nextVertex = node5;
  node4->nextSuccesor = node4_succ;
  node5->nextVertex = node6;
  node5->nextSuccesor = node5_succ;
  node6->nextVertex = node7;
  node6->nextSuccesor = node6_succ;
  node7->nextSuccesor = node7_succ;

  AK_EPI;
}

/**
 * @author Frane Jakelić, updated by Blaž Rajič
 * @brief Tarjan algorithm that looks for a strongly connected component inside
 * all subgraphs; using DFS
 * @param id of the element on which the algorithm looks for an id of a strongly
 * connected component
 */
void AK_tarjan(int id)
{
  AK_vertex node;
  AK_succesor succ;
  AK_PRO;
  node = AK_search_vertex(id);
  node->index = indexCounter;
  node->lowLink = indexCounter;
  indexCounter = indexCounter + 1;
  succ = node->nextSuccesor;

  AK_push_to_stack(id);

  while (succ != NULL)
  {

    if (succ->link->index == -1)
    {

      AK_tarjan(succ->link->vertexId);
      node->lowLink = MIN(node->lowLink, succ->link->lowLink);
    }
    else if (AK_search_in_stack(succ->link->vertexId) != NULL)
    {
      node->lowLink = MIN(node->lowLink, succ->link->index);
    }

    succ = succ->nextSuccesor;
  }

  if (node->lowLink == node->index)
  {
    AK_vertex loop = NULL;
    AK_stack elem = AK_pop_from_stack();

    if (elem->link != node)
    {
      printf("\nStrongy connected component detected. Edges:\n");
      do
      {
        if (elem == NULL)
          break;
        loop = elem->link;
        printf("%i\n", loop->vertexId);
        if (elem->link == node)
          break;
        elem = AK_pop_from_stack();
      } while (loop->vertexId != node->vertexId);
    }
  }
  testLowArray[node->vertexId] = node->lowLink;
  AK_EPI;
}

/**
 * @author Blaž Rajič
 * @brief Function for testing Tarjan's algorithm
 * @return No return value
 */

TestResult AK_tarjan_test()
{
  AK_vertex root;
  AK_vertex ro_ot;
  AK_vertex loop;
  AK_PRO;
  AK_define_tarjan_graph();
  root = G.nextVertex;
  ro_ot = G.nextVertex;

  // TEST 1
  printf("\n============== Running Test #1 ==============\n");
  printf("\nTarjan's algorithm in short finds all strongly connected "
         "components(SCC) in a directed graph, SCC is when each vertex of the "
         "graph is reachable from every other vertex in that SCC.\n");
  printf("\nTarjan's algorithm will be tested on the graph below, SCC's in "
         "this graph are (0,1,2), (3,4) and (5,6,7).\n");

  printf("                    ┌────────┐ \n");
  printf("                    ▼        │ \n");
  printf(" ┌─┐     ┌─┐       ┌─┐      ┌┴┐\n");
  printf(" │0│◄────┤2├──────►│3├─────►│4│\n");
  printf(" └┬┘     └─┘       └┬┘      └─┘\n");
  printf("  │       ▲         │  \n");
  printf("  │  ┌─┐  │         ▼  \n");
  printf("  └─►│1├──┘        ┌─┐ \n");
  printf("     └─┘      ┌───►│5├────┐ \n");
  printf("              │    └─┘    │ \n");
  printf("              │           │ \n");
  printf("              │           ▼ \n");
  printf("             ┌┴┐         ┌─┐ \n");
  printf("             │7│◄────────┤6│ \n");
  printf("             └─┘         └─┘ \n");

  for (int i = 0; i < 5; i++)
  {
    loop = AK_search_vertex(i);
    if (loop->index == -1)
    {
      AK_tarjan(loop->vertexId);
    }
  }

  int testingArray[8] = {0, 0, 0, 3,
                         3, 5, 5, 5}; // values that are expected as end result
  for (int i = 0; i < 8; i++)
  {
    if (testLowArray[i] != testingArray[i])
    {
      printf("\nFAILURE\n");
      AK_EPI;
      return TEST_result(0, 1);
    }
  }
  printf("\nSUCCESS\n");
  AK_EPI;
  return TEST_result(1, 0);
}

/**
 * @author Marko Sinko
 * @brief Initializes an AK_synchronization_info structure and returns an owned
 *        pointer that must later be passed on to AK_destroy_critical_section.
 * @return Initialized synchronization object
 */
AK_synchronization_info *AK_init_critical_section()
{
  AK_synchronization_info *info = AK_calloc(1, sizeof(AK_synchronization_info));
  if (info == NULL)
    return NULL;

#ifdef _WIN32
  if (InitializeCriticalSectionAndSpinCount(&info->critical_section,
                                            0x00000400) == 0)
  {
    free(info);
    return NULL;
  }
#endif
#ifdef __linux__
  pthread_mutex_init(&info->mutex, NULL);
#endif
  info->init = 1;
  info->ready = 1;
  return info;
}

/**
 * @author Marko Sinko
 * @param info Synchronization info structure
 * @brief Destroys a synchronization object when it is no longer necessary and
 *        frees the pointer.
 * @return void
 */
void AK_destroy_critical_section(AK_synchronization_info *info)
{
  if (info == NULL)
    return;
  if (info->init != 1)
    return;
#ifdef _WIN32
  DeleteCriticalSection(&info->critical_section);
#endif
#ifdef __linux__
  pthread_mutex_destroy(&info->mutex);
#endif
  AK_free(info);
  info = NULL;
}

/**
 * @author Marko Sinko
 * @param info Synchronization info structure
 * @brief Enters a critical section.
 * @return void
 */
void AK_enter_critical_section(AK_synchronization_info *info)
{
  assert(info != NULL && info->init == 1);
#ifdef __linux__
  pthread_mutex_lock(&info->mutex);
#endif
#ifdef _WIN32
  EnterCriticalSection(&info->critical_section);
#endif
  while (info->ready != 1)
    ; /* wait loop */
  info->ready = 0;
#ifdef __linux__
  pthread_mutex_unlock(&info->mutex);
#endif
#ifdef _WIN32
  LeaveCriticalSection(&info->critical_section);
#endif
}

/**
 * @author Marko Sinko
 * @param info Synchronization info structure
 * @brief Leaves a critical section
 * @return void
 */
void AK_leave_critical_section(AK_synchronization_info *info)
{
  assert(info != NULL && info->init == 1);
  info->ready = 1;
}

/**
 * @author Krunoslav Bilić
 * @brief Function that copies one "AK_list_elem" to another.
 * @param first "AK_list_elem" to be compared - source
 * @param second "AK_list_elem" to be compared - destination
 * @return No return value.
 */
// 0 uses found
/*
void AK_copy_L_Ele(AK_list_elem srcElem, AK_list_elem destElem) {
        AK_PRO;
        destElem->size = srcElem->size;
        destElem->type = srcElem->type;
        memcpy(destElem->data, srcElem->data, srcElem->size);
        memcpy(destElem->table, srcElem->table, sizeof(srcElem->table));
        AK_EPI;
}
*/

/**
 * @author Krunoslav Bilić
 * @brief Function that copies "AK_list" to another. Uses AK_copy_L_Ele(..) for
 * copying.
 * @param first "AK_list" to be compared - source
 * @param second "AK_list" to be compared - destination
 * @return No return value.
 */
// 0 uses found
/*
void AK_copy_L(AK_list *src, AK_list *dest) {

        AK_list_elem srcElem;
        AK_list_elem destElem;
        AK_PRO;
        //srcElem = (AK_list_elem) AK_malloc(sizeof(struct list_elem));
        srcElem = (AK_list_elem) AK_First_L2(src);
        destElem = (AK_list_elem) AK_First_L2(dest);

        do {
                AK_copy_L_Ele(srcElem, destElem);

                srcElem = (AK_list_elem) AK_Next_L2(srcElem);

                if (destElem->next == NULL && srcElem != NULL) {
                        destElem->next = (AK_list_elem) AK_malloc(sizeof(struct
list_elem)); destElem = (AK_list_elem) AK_Next_L2(destElem); destElem->next =
NULL;
                }
        } while (srcElem != NULL);
        AK_EPI;
}
*/

/**
 * @author Krunoslav Bilić
 * @brief Function that compares two "AK_list" lists by DATA, SIZE and TYPE
 * @param first "AK_list" to be compared
 * @param second "AK_list" to be compared
 * @return EXIT_SUCCESS if content is same, otherwise EXIT_FAILURE
 */
// 0 uses found
/*
int AK_compare_L(AK_list *srcInput, AK_list *srcOriginal) {
        AK_list_elem srcElemInp;
        AK_list_elem srcElemOrig;
        AK_PRO;
        // inicijalna provjera, da li su velicine identicne, ukoliko nisu :
izlaz if (AK_Size_L(srcInput) != AK_Size_L(srcOriginal)){ AK_EPI; return
EXIT_FAILURE;
        }

        srcElemInp = (AK_list_elem) AK_First_L2(srcInput);
        srcElemOrig = (AK_list_elem) AK_First_L2(srcOriginal);

        // provjera za elemente data, size, type, table
        do {
                int compare = 0;
                compare = srcElemInp->size == srcElemOrig->size ? 0 : 1;
                if (compare != 0){
                        AK_EPI;
                        return EXIT_FAILURE;
                }
                compare = srcElemInp->type == srcElemOrig->type ? 0 : 1;
                if (compare != 0){
                        AK_EPI;
                        return EXIT_FAILURE;
                }
                compare = strcmp(srcElemInp->data, srcElemOrig->data);
                if (compare != 0){
                        AK_EPI;
                        return EXIT_FAILURE;
                }
                compare = strcmp(srcElemInp->table, srcElemOrig->table);
                if (compare != 0){
                        AK_EPI;
                        return EXIT_FAILURE;
                }


                //dohvaca novi element liste
                srcElemInp = (AK_list_elem) AK_Next_L2(srcElemInp);
                srcElemOrig = (AK_list_elem) AK_Next_L2(srcElemOrig);
        } while (srcElemOrig != NULL);
        AK_EPI;
        return EXIT_SUCCESS;

}
*/
