import time

# This module contains functions which will check if a given argument 
# is trully the type which we need it to be

# is_numeric
# returns int or float value based on input type
# @param lit the value to be checked
def is_numeric(lit):
    # Handle '0'
    if lit == '0':
        return 0
    # Hex/Binary
    litneg = lit[1:] if lit[0] == '-' else lit
    if litneg[0] == '0':
        if litneg[1] in 'xX':
            return int(lit, 16)
        elif litneg[1] in 'bB':
            return int(lit, 2)
        else:
            try:
                return int(lit, 8)
            except ValueError:
                pass
    # Int/Float
    try:
        return int(lit)
    except ValueError:
        pass
    try:
        return float(lit)
    except ValueError:
        pass


# is_date
# returns true if the input value is in date format
# @param lit the value to be checked
def is_date(lit):
    try:
        time.strptime(lit, '%Y-%m-%d')
        return True
    except ValueError:
        return False


# is_datetime
# returns true if the input value is in datetime format
# @param lit the value to be checked
def is_datetime(lit):
    try:
        time.strptime(lit, '%Y-%m-%d %H:%M:%S')
        return True
    except ValueError:
        return False


# is_time
# returns true if the input value is in time format
# @param lit the value to be checked
def is_time(lit):
    try:
        time.strptime(lit, '%H:%M:%S')
        return True
    except ValueError:
        return False


# is_bool
# returns true if the input value is boolean
# @param lit the value to be checked
def is_bool(lit):
    return lit.lower() in ("true", "false")

