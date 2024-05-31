# all tests are ok
# functions are written in test.py and imported here then tested with doctest

import kalashnikovDB as AK47
import test_strings as ts
from test import *

f = Functions()
AK47.AK_inflate_config()
AK47.AK_init_disk_manager()
AK47.AK_memoman_init()

create_tables()

# author: Luka Rajcevic
# this function tests methods from Functions class,
# it queries table for its properties (attribute count, record count etc.)
# bugs:
#   -> get_row_test() does not print number values
#   -> rename_Table() does not work (segfault)

def table_properties_test():
    '''
    >>> f.attribute_count("class")
    3
    >>> f.records_count("class")
    4
    >>> f.clear_file("table_test.txt")
    0
    >>> f.get_column_test(1, "class")
    1
    >>> f.verify_row_or_column(ts.ct_5, "table_test.txt")
    0
    >>> f.clear_file("table_test.txt")
    0
    >>> f.get_column_test(0, "class")
    1
    >>> f.verify_row_or_column(ts.ct_4, "table_test.txt")
    0
    >>> f.clear_file("table_test.txt")
    0
    >>> f.get_row_test(1, "class")
    1
    >>> f.verify_row_or_column(ts.ct_3, "table_test.txt")
    0
    >>> f.get_attr_name("class", 1)
    'class_name'
    >>> f.get_attr_name("class", 0)
    'id_class'
    >>> f.get_attr_index("class", "year")
    2
    >>> f.get_value(1, 2, "class")
    '2011'
    >>> f.rename_Attribute("class", "year", "anno")
    0
    >>> f.verify_table("class", ts.ct_2, "table_test.txt")
    0
    ''' 

