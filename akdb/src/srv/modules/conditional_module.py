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
    

# is_interval
# returns true if the input value is in interval format
# @param lit the value to be checked
def is_interval(lit):
    isTypeCorrect = False
    try:
        parts = lit.split()
        if len(parts) == 12:
                    if parts[1] == "year" or parts[1] == "years":
                        if parts[3] == "month" or parts[3] == "months":
                            if parts[5] == "day" or parts[5] == "days":
                                if parts[7] == "hour" or parts[7] == "hours":
                                    if parts[9] == "minute" or parts[9] == "minutes":
                                        if parts[11] == "second" or parts[11] == "seconds":
                                            if int(parts[0]) or int(parts[0]) == 0 and int(parts[2]) or int(parts[2]) == 0 and int(parts[4]) or int(parts[4]) == 0 and int(parts[6]) or int(parts[6]) == 0 and int(parts[8]) or int(parts[8]) == 0 and int(parts[10]) or int(parts[10]) == 0:
                                                    if 0 <= int(parts[2]) <= 11:
                                                        if 0 <= int(parts[4]) <= 365:
                                                            if 0 <= int(parts[6]) <= 23:
                                                                if 0 <= int(parts[8]) <= 59:
                                                                    if 0 <= int(parts[10]) <= 59:
                                                                            isTypeCorrect = True
        return(isTypeCorrect)                                                                    
    except Exception:
        return(isTypeCorrect)
    

# is_period
# returns true if the input value is in period format
# @param lit the value to be checked
def is_period(lit):
    try:
        start_str, end_str = lit.split(" - ")
        time.strptime(start_str, '%Y-%m-%d %H:%M:%S')
        time.strptime(end_str, '%Y-%m-%d %H:%M:%S')
        return True
    except ValueError:
        return False


# is_bool
# returns true if the input value is boolean
# @param lit the value to be checked
def is_bool(lit):
    return lit.lower() in ("true", "false")

