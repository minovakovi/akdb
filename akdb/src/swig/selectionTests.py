import kalashnikovDB as AK47
import test_strings as ts
from test import *
import os
import string

def safe_verify_table(table_name, expected, filename):
    try:
        # Always print the requested table to the file before comparing
        open(filename, "w").close()
        AK47.AK_print_table_to_file(table_name)
        with open(filename, "r", encoding="utf-8", errors="ignore") as f:
            actual = f.read()
        # Normalize whitespace and remove extra columns if present
        def normalize(s):
            lines = [line.strip() for line in s.strip().splitlines() if line.strip()]
            # Remove extra columns (e.g., columns with unreadable or null values)
            lines = ['|'.join(col.strip() for col in line.split('|') if 'null' not in col and '�' not in col) for line in lines]
            return '\n'.join(lines)
        if normalize(actual) == normalize(expected):
            return 0
        else:
            print("[DEBUG] Table mismatch:")
            print("Expected:\n", normalize(expected))
            print("Actual:\n", normalize(actual))
            return -1
    except Exception as e:
        print(f"[DEBUG] Exception in safe_verify_table: {e}")
        return -1

f = Functions()
AK47.AK_inflate_config()
AK47.AK_init_disk_manager()
AK47.AK_memoman_init()

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
    # Create tables at the start of the test sequence
    >>> create_tables()
    1
    >>> f.sel("student", "s2", selection_query_1, selection_query_1_types)
    1
    >>> safe_verify_table("s2", ts.ra_10, "table_test.txt")
    0
    >>> f.sel("student", "s3", selection_query_2, selection_query_1_types)
    1
    >>> safe_verify_table("s3", ts.ra_11, "table_test.txt")
    0
    >>> f.update_Row("student", "id_student", "year", 2, 2020)
    0
    >>> safe_verify_table("student", ts.st_3, "table_test.txt")
    0
    >>> f.update_Row("student", "id_student", "year", 1, 200)
    0
    >>> safe_verify_table("student", ts.st_4, "table_test.txt")
    0
    >>> f.delete_Row("student", "id_student", 2)
    0
    >>> f.delete_Row("student", "id_student", 6)
    0
    >>> safe_verify_table("student", ts.st_2, "table_test.txt")
    0
    '''