from datetime import datetime
from typing import Union, Optional

def is_numeric(lit: str) -> Optional[Union[int, float]]:
    """
    Determines if the input string is a numeric value.
    Supports int, float, hexadecimal, binary, and octal.
    """
    if lit == '0':
        return 0
    litneg = lit[1:] if lit.startswith('-') else lit
    if litneg.startswith('0'):
        if litneg[1:2] in ('x', 'X'):
            return int(lit, 16)
        elif litneg[1:2] in ('b', 'B'):
            return int(lit, 2)
        else:
            try:
                return int(lit, 8)
            except ValueError:
                pass
    try:
        return int(lit)
    except ValueError:
        pass
    try:
        return float(lit)
    except ValueError:
        return None


def is_date(lit: str) -> bool:
    """Checks if the string is in YYYY-MM-DD format."""
    try:
        datetime.strptime(lit, '%Y-%m-%d')
        return True
    except ValueError:
        return False


def is_datetime(lit: str) -> bool:
    """Checks if the string is in YYYY-MM-DD HH:MM:SS format."""
    try:
        datetime.strptime(lit, '%Y-%m-%d %H:%M:%S')
        return True
    except ValueError:
        return False


def is_time(lit: str) -> bool:
    """Checks if the string is in HH:MM:SS format."""
    try:
        datetime.strptime(lit, '%H:%M:%S')
        return True
    except ValueError:
        return False


def is_interval(lit: str) -> bool:
    """
    Checks if the string is in a valid interval format:
    e.g., "1 year 2 months 3 days 4 hours 5 minutes 6 seconds"
    """
    try:
        parts = lit.split()
        if len(parts) != 12:
            return False
        
        labels = [
            ("year", "years", 0, None),
            ("month", "months", 0, 11),
            ("day", "days", 0, 365),
            ("hour", "hours", 0, 23),
            ("minute", "minutes", 0, 59),
            ("second", "seconds", 0, 59),
        ]

        for i, (singular, plural, min_val, max_val) in enumerate(labels):
            val = int(parts[i * 2])
            label = parts[i * 2 + 1].lower()
            if label not in (singular, plural):
                return False
            if max_val is not None and not (min_val <= val <= max_val):
                return False

        return True
    except Exception:
        return False


def is_period(lit: str) -> bool:
    """
    Checks if the string is in a period format:
    'YYYY-MM-DD HH:MM:SS - YYYY-MM-DD HH:MM:SS'
    """
    try:
        start, end = lit.split(' - ')
        datetime.strptime(start, '%Y-%m-%d %H:%M:%S')
        datetime.strptime(end, '%Y-%m-%d %H:%M:%S')
        return True
    except ValueError:
        return False
    except Exception:
        return False


def is_bool(lit: str) -> bool:
    """Checks if the string represents a boolean value."""
    return lit.casefold() in ('true', 'false')
