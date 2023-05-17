# This module contains functions related to errors in database.

# akdbError
# prints expression expr and a pointer at the position of error item
# @param expr the expression to be printed
# @param item item where the error occured
def akdbError(expr, item):
    print(expr)
    print(" " * expr.index(item) + "^")

