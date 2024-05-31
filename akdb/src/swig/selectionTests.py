import kalashnikovDB as AK47
import test_strings as ts
from test import *

f = Functions()
AK47.AK_inflate_config()
AK47.AK_init_disk_manager()
AK47.AK_memoman_init()

create_tables()
# author: Luka Rajcevic
# Test data for selection
# select * from student where year < 1990
student_attr_name = ["id_student", "firstname", "lastname", "year", "weight"]
selection_query_1 = ["year", "1991", ">"]
selection_query_2 = ["year", "1990", ">"]
selection_query_3 = ["firstname", "Manuel", "=="]
selection_query_1_types = [AK47.TYPE_ATTRIBS, AK47.TYPE_INT, AK47.TYPE_OPERATOR]
# author: Luka Rajcevic
# test functions for CRUD operations
# current bugs:
#   -> select works only on integers (floats and strings do not work)
#   -> update doesn't do anything (table stays the same)
#   -> delete is removing the wrong row (eg. 3 instead of 4, or 7 instead of 8)
def selection_test():
    '''
    >>> f.sel("student", "s2", selection_query_1, selection_query_1_types)
    1
    >>> f.verify_table("s2", ts.ra_10, "table_test.txt")
    0
    >>> f.sel("student", "s3", selection_query_2, selection_query_1_types)
    1
    >>> f.verify_table("s3", ts.ra_11, "table_test.txt")
    0
    >>> f.update_Row("student", "id_student", "year", 2, 2020)
    0
    >>> f.verify_table("student", ts.st_3, "table_test.txt")
    0
    >>> f.update_Row("student", "id_student", "year", 1, 200)
    0
    >>> f.verify_table("student", ts.st_4, "table_test.txt")
    0
    >>> f.delete_Row("student", "id_student", 2)
    0
    >>> f.delete_Row("student", "id_student", 6)
    0
    >>> f.verify_table("student", ts.st_2, "table_test.txt")
    0
    '''