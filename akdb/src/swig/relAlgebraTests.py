import kalashnikovDB as AK47
import test_strings as ts
from test import *

f = Functions()
AK47.AK_inflate_config()
AK47.AK_init_disk_manager()
AK47.AK_memoman_init()

create_tables()

def rel_algebra_test():
    '''
    >>> f.product("student", "class", "product")
    0
    >>> f.verify_table("product", ts.ra_1, "table_test.txt")
    0
    >>> f.intersect("class_2", "class", "intersect")
    0
    >>> f.verify_table("intersect", ts.ra_9, "table_test.txt")
    0
    >>> f.difference("class_2", "class", "difference")
    0
    >>> f.verify_table("difference", ts.ra_8, "table_test.txt")
    0
    >>> f.union("class_2", "class", "union")
    0
    >>> f.verify_table("union", ts.ra_3, "table_test.txt")
    0
    >>> f.projection("student", "projection", ["id_student", "year"])
    0
    >>> f.verify_table("projection", ts.ra_4, "table_test.txt")
    0
    >>> f.projection("student", "projection2", ["year"])
    0
    >>> f.verify_table("projection2", ts.ra_6, "table_test.txt")
    0
    >>> f.projection("student", "projection3", ["firstname", "year", "lastname"])
    0
    >>> f.verify_table("projection3", ts.ra_7, "table_test.txt")
    0
    '''

