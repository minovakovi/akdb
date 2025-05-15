from modules.conditional_module import *
import kalashnikovDB as AK47

# This module contains functions which return the data types


# get_attr_type
# returns value type code in AK47 notation defined in constants.c
# @param value the value to be checked
def get_attr_type(value):

    integer = type(is_numeric(value)) == int
    decimal = type(is_numeric(value)) == float
    varchar = isinstance(value, str)
    date = is_date(value.replace("'", ""))
    datetime = is_datetime(value.replace("'", ""))
    time = is_time(value.replace("'", ""))
    interval = is_interval(value.replace("'", ""))
    period = is_period(value.replace("'", ""))
    boolean = is_bool(value.replace("'", ""))
    
    if integer:
        return AK47.TYPE_INT
    elif decimal:
        return AK47.TYPE_FLOAT
    elif date:
        return AK47.TYPE_DATE
    elif datetime:
        return AK47.TYPE_DATETIME
    elif time:
        return AK47.TYPE_TIME
    elif interval:
        return AK47.TYPE_INTERVAL
    elif period:
        return AK47.TYPE_PERIOD
    elif boolean:
        return AK47.TYPE_BOOL
    elif varchar:
        return AK47.TYPE_VARCHAR
    else:
        print("UNDEFINED")

# get_type_name
# returns type name for supplied type code in AK47 notation defined in constants.c
# @param code the type code to be checked


def get_type_name(code):

    if code == AK47.TYPE_INT:
        return "int"
    elif code == AK47.TYPE_FLOAT:
        return "float"
    elif code == AK47.TYPE_DATE:
        return "date"
    elif code == AK47.TYPE_DATETIME:
        return "datetime"
    elif code == AK47.TYPE_TIME:
        return "time"
    elif code == AK47.TYPE_INTERVAL:
        return "interval"
    elif code == AK47.TYPE_PERIOD:
        return "period"
    elif code == AK47.TYPE_BOOL:
        return "boolean"
    elif code == AK47.TYPE_VARCHAR:
        return "varchar"
    else:
        return "UNDEFINED"
