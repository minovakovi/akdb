/* File: kalashnikovDB.i */
/*
The interface file contains ANSI C function prototypes and variable declarations.
The %module directive defines the name of the module that will be created by SWIG.
The %{ %} block provides a location for inserting additional code, such as C header
files or additional C declarations, into the generated C wrapper code.
*/
%module kalashnikovDB


/*
@brief used for conversion or marshalling of datatypes between programming languages.
Specifically, for every C/C++ declaration, SWIG must somehow generate wrapper code that
allows values to be passed back and forth between languages. Since every programming
language represents data differently, this is not a simple of matter of simply linking code
together with the C linker. Instead, SWIG has to know something about how data is
represented in each language and how it can be manipulated.

Documentation for typemaps:
  http://www.swig.org/Doc2.0/Typemaps.html#Typemaps

Convert from Python --> C
  %typemap(in)

Convert from C --> Python
  %typemap(out)

*/

/*
Convert python data type to C void*
check if data is of type int, string, long or float, and then cast it to (void*)
*/
%typemap(in) void* {
  if (PyString_Check($input))
   {
    char* b =  PyString_AsString($input);
    $1 = (void*)b;
   }
  else if (PyInt_Check($input))
  {
    int a =  PyInt_AsLong($input);
    $1 = (void*)&a;
  }
  else if (PyFloat_Check($input))
  {
    float c =  PyFloat_AsDouble($input);
    $1 = (void*)&c;
  }

}

/*
Convert python list to C int and int *
given a python list, eg. ["a", "b", "c"], passed to C function that takes int (number of elements) and
int* (elements in int array), list is split into int (representing list size) and int* (representing int array)
*/
%typemap(in) (int _num, int* _type) {
  int i;
  if (!PyList_Check($input)) {
    PyErr_SetString(PyExc_ValueError, "Expecting a list");
    return NULL;
  }
  $1 = PyList_Size($input);
  $2 = (int *) AK_malloc(($1)*sizeof(int));
  for (i = 0; i < $1; i++) {
    PyObject *s = PyList_GetItem($input,i);
    if (!PyInt_Check(s)) {
        AK_free($2);
        PyErr_SetString(PyExc_ValueError, "List items must be integers");
        return NULL;
    }
    $2[i] = PyInt_AsLong(s);
  }
}

/*
The "AK_freearg" typemap is used to cleanup argument data. It is only used when an argument might have
allocated resources that need to be cleaned up when the wrapper function exits. The "AK_freearg"
typemap usually cleans up argument resources allocated by the "in" typemap.
*/
%typemap(AK_freearg) (int _num, int* _type) {
   if ($2) AK_free($2);
}

/*
converts C char* into Python string
*/
%typemap(out) unsigned char * {
  $1 = PyString_FromString((char *)$input);
}

/*
converts Python datatypes (string, int or float) into C char* data type
*/
%typemap(in) char* data {

  if (PyString_Check($input))
   {
    char* b =  PyString_AsString($input);
    $1 = (char*)b;
   }
  else if (PyInt_Check($input))
  {
    int a =  PyInt_AsLong($input);
    $1 = (char*)&a;
  }
  else if (PyFloat_Check($input))
  {
    float c =  PyFloat_AsDouble($input);
    $1 = (char*)&c;
  }
}

/*
converts C int* datatype into Pythons list of integers
*/
%typemap(out) int * {
  PyObject *list = PyList_New(MAX_CONSTRAINTS);
  int i = 0;
  int *polje = $1;
  for (i = 0; i < MAX_CONSTRAINTS; i++) {
    PyObject *item = PyInt_FromLong(polje[i]);
    PyList_Append(list, item);
  }
  $result = list;
}

/*
handles C AK_block datatypes in Python
*/
%typemap(in) AK_block * block{
  AK_block *bl = AK_malloc(sizeof(AK_block));
  memcpy(bl, $input, sizeof(AK_block));
  $1 = bl;
}

/*
converts Python list into char** type.
*/
%typemap(in) char ** {
  if (PyList_Check($input)) {
    int size = PyList_Size($input);
    int i = 0;
    $1 = (char **) AK_malloc((size+1)*sizeof(char *));
    for (i = 0; i < size; i++) {
      PyObject *o = PyList_GetItem($input,i);
      if (PyUnicode_Check(o))
  $1[i] = PyUnicode_AsUTF8(PyList_GetItem($input,i));
      else {
  PyErr_SetString(PyExc_TypeError,"list must contain strings");
  AK_free($1);
  return NULL;
      }
    }
    $1[i] = 0;
  } else {
    PyErr_SetString(PyExc_TypeError,"not a list");
    return NULL;
  }
}

/*
AK_frees char** allocated memory
*/
%typemap(AK_freearg) char ** {
  AK_free((char *) $1);
}

/*
handles AK_create_table_parameter * datatype in Python.
*/
%typemap(out) AK_create_table_parameter * {
  AK_create_table_parameter* parametar = AK_malloc(sizeof(AK_create_table_parameter));
  memcpy(parametar, $1, sizeof(AK_create_table_parameter));
  $result = parametar;
}

/*
Converts python list to AK_create_table_parameter* datatype
*/
%typemap(in) AK_create_table_parameter * parameters{
  if (PyList_Check($input)) {
    int size = PyList_Size($input);
    int i = 0;
    AK_create_table_parameter parameters[ size ];
    AK_create_table_parameter* parametar;

    for (i = 0; i < size; i++) {
      parametar = (AK_create_table_parameter *) PyList_GetItem($input, i);
      memcpy(parameters + i, parametar, sizeof ( AK_create_table_parameter));
    }
    $1 = parameters;
  }
  else $1 = (AK_create_table_parameter*)$input;
}

/*
AK_frees AK_create_table_parameter** datatype
*/
%typemap(AK_freearg) AK_create_table_parameter ** {
  AK_free($1);
}

/*
handles AK_list * datatype in Python.
*/
%typemap(out) AK_list * {
  AK_list* list = AK_malloc(sizeof(AK_list));
  memcpy(list, $1, sizeof(AK_list));
  $result = list;
}

/*
Converts python list to AK_list* datatype
*/
%typemap(in) AK_list * lists{
  if (PyList_Check($input)) {
    int size = PyList_Size($input);
    int i = 0;
    AK_list lists[ size ];
    AK_list* list;

    for (i = 0; i < size; i++) {
      list = (AK_list *) PyList_GetItem($input, i);
      memcpy(lists + i, list, sizeof ( AK_list));
    }
    $1 = lists;
  }
  else $1 = (AK_list*)$input;
}

/*
AK_frees AK_list** datatype
*/
%typemap(AK_freearg) AK_list ** {
  AK_free($1);
}

/*
Everything in the %{ ... %} block is simply copied verbatim to the resulting wrapper file created by SWIG.
*/
%{
  #define SWIG_FILE_WITH_INIT
  #include "implementation.h"
%}


%include "../auxi/debug.h"
%include "../auxi/debug.c"

%include "../auxi/constants.h"
%include "../auxi/configuration.h"

%include "../auxi/auxiliary.c"
%include "../auxi/auxiliary.h"

%include "../auxi/test.c"
%include "../auxi/test.h"

%include "../dm/dbman.h"
%include "../dm/dbman.c"
extern table_addresses *AK_get_segment_addresses(char * segmentName);



%include "../mm/memoman.c"
%include "../mm/memoman.h"

%include "../file/table.c"
%include "../file/table.h"

%include "../file/idx/index.h"

extern element_ad AK_Get_First_elementAd(list_ad *L);
extern element_ad AK_Get_Last_elementAd(list_ad *L);
extern element_ad AK_Get_Next_elementAd(element_ad Currentelement_op);
extern void AK_Insert_NewelementAd(int addBlock, int indexTd, char *attName, element_ad elementBefore);

%include "../file/idx/bitmap.c"

%include "../file/idx/hash.c"
%include "../file/idx/hash.h"

%include "../file/idx/btree.c"
%include "../file/idx/btree.h"
%include "../file/idx/index.c"

%include "../file/filesort.c"
%include "../file/filesort.h"
%include "../file/filesearch.c"
%include "../file/filesearch.h"
%include "../file/fileio.c"
%include "../file/fileio.h"
%include "../file/files.c"
%include "../file/files.h"

%include "../rel/theta_join.c"
%include "../rel/theta_join.h"
%include "../rel/product.c"
%include "../rel/product.h"
%include "../rel/aggregation.c"
%include "../rel/aggregation.h"
%include "../rel/union.c"
%include "../rel/union.h"
%include "../rel/selection.c"
%include "../rel/selection.h"
%include "../sql/drop.c"
%include "../sql/drop.h"
%include "../sql/view.c"
%include "../sql/view.h"


%include "../rel/projection.c"
%include "../rel/projection.h"

%include "../rel/nat_join.c"
%include "../rel/nat_join.h"
%include "../rel/intersect.c"
%include "../rel/intersect.h"
%include "../rel/difference.c"
%include "../rel/difference.h"

extern AK_list *AK_rel_eq_assoc(AK_list *list_rel_eq);

extern char *AK_rel_eq_get_atrributes_char(char *tblName);
extern char *AK_rel_eq_cond_attributes(char *cond);
extern int AK_rel_eq_share_attributes(char *set, char *subset);
extern char *AK_rel_eq_commute_with_theta_join(char *cond, char *tblName);
extern struct list_node *AK_rel_eq_split_condition(char *cond) ;
extern AK_list *AK_rel_eq_selection(AK_list *list_rel_eq);

extern char *AK_rel_eq_projection_attributes(char *attribs, char *tblName);
extern AK_list *AK_rel_eq_projection(AK_list *list_rel_eq);
extern struct list_node *AK_rel_eq_get_attributes(char *tblName);

%include "../opti/query_optimization.c"
%include "../opti/query_optimization.h"


%include "../opti/rel_eq_comut.c"
%include "../opti/rel_eq_comut.h"
%include "../opti/rel_eq_assoc.c"
%include "../opti/rel_eq_assoc.h"

%include "../opti/rel_eq_selection.c"
%include "../opti/rel_eq_selection.h"
%include "../opti/rel_eq_projection.c"
%include "../opti/rel_eq_projection.h"

%include "../rel/expression_check.c"
%include "../rel/expression_check.h"
%include "../file/id.c"
%include "../file/id.h"

%include "../sql/cs/unique.c"
%include "../sql/cs/unique.h"

%include "../sql/cs/check_constraint.c"
%include "../sql/cs/check_constraint.h"

%include "../sql/cs/nnull.c"
%include "../sql/cs/nnull.h"
%include "../sql/cs/between.c"
%include "../sql/cs/between.h"
%include "../sql/cs/reference.c"
%include "../sql/cs/reference.h"

%include "../trans/transaction.h"
%include "../trans/transaction.c"

%include "../sql/privileges.c"
%include "../sql/privileges.h"

%include "../auxi/observable.c"
%include "../auxi/observable.h"

%include "../file/test.c"
%include "../file/test.h"
%include "../sql/trigger.c"
%include "../sql/trigger.h"


%include "../rec/archive_log.c"
%include "../rec/archive_log.h"

%include "../sql/command.c"
%include "../sql/command.h"

%include "../auxi/iniparser.c"
%include "../auxi/iniparser.h"

%include "../auxi/dictionary.c"
%include "../auxi/dictionary.h"

%include "../sql/function.h"
%include "../sql/function.c"

%include "../file/sequence.h"
%include "../file/sequence.c"

%include "../rec/redo_log.h"
%include "../rec/redo_log.c"

%include "../rec/recovery.h"
%include "../rec/recovery.c"



%include "../sql/cs/constraint_names.c"
%include "../sql/cs/constraint_names.h"


%import "../sql/insert.h"
%import "../sql/insert.c"

extern AK_header *AK_get_insert_header(int *size, char *tblName, struct list_node *columns);
extern int AK_insert(char* tableName, struct list_node *columns, struct list_node *values);


extern int AK_check_constraint_name(char *constraintName, char *constraintTable);
