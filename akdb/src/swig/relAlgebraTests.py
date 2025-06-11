import kalashnikovDB as AK47
import test_strings as ts
from test import Functions, create_tables
import os
import atexit

# Global flag to track if DB is initialized
_db_initialized = False
_tables_created = False
_functions_instance = None

def cleanup_db():
    """Cleanup function to be called on exit"""
    global _db_initialized, _tables_created, _functions_instance
    _db_initialized = False
    _tables_created = False
    _functions_instance = None

# Register cleanup function
atexit.register(cleanup_db)

def safe_initialize_db():
    """Initialize the database system safely"""
    global _db_initialized
    if _db_initialized:
        return True
        
    try:
        # Clean up any existing database file first
        db_path = "kalashnikov.db"
        if os.path.exists(db_path):
            os.remove(db_path)
            
        print("Initializing database...")
        AK47.AK_inflate_config()
        AK47.AK_init_disk_manager()
        AK47.AK_memoman_init()
        _db_initialized = True
        print("Database initialized successfully")
        return True
    except Exception as e:
        print(f"Database initialization failed: {e}")
        return False

def safe_setup_test_data():
    """Create test tables if they don't exist"""
    global _tables_created
    if _tables_created:
        return True
        
    try:
        print("Creating test tables...")
        create_tables()
        _tables_created = True
        print("Test tables created successfully")
        return True
    except Exception as e:
        print(f"Test data setup failed: {e}")
        return False

def get_functions_instance():
    """Get a single Functions instance"""
    global _functions_instance
    if _functions_instance is None:
        if safe_initialize_db() and safe_setup_test_data():
            _functions_instance = Functions()
        else:
            print("Failed to initialize database or create tables")
    return _functions_instance

# Get the functions instance
f = get_functions_instance()

def rel_algebra_test():
    '''
    >>> f = get_functions_instance()
    >>> f.product("student", "class", "product") if f else None
    0
    >>> f.verify_table("product", ts.ra_1, "table_test.txt") if f else None
    0
    >>> f.intersect("class_2", "class", "intersect") if f else None
    0
    >>> f.verify_table("intersect", ts.ra_9, "table_test.txt") if f else None
    0
    >>> f.difference("class_2", "class", "difference") if f else None
    0
    >>> f.verify_table("difference", ts.ra_8, "table_test.txt") if f else None
    0
    >>> f.union("class_2", "class", "union") if f else None
    0
    >>> f.verify_table("union", ts.ra_3, "table_test.txt") if f else None
    0
    >>> f.projection("student", "projection", ["id_student", "year"]) if f else None
    0
    >>> f.verify_table("projection", ts.ra_4, "table_test.txt") if f else None
    0
    >>> f.projection("student", "projection2", ["year"]) if f else None
    0
    >>> f.verify_table("projection2", ts.ra_6, "table_test.txt") if f else None
    0
    >>> f.projection("student", "projection3", ["firstname", "year", "lastname"]) if f else None
    0
    >>> f.verify_table("projection3", ts.ra_7, "table_test.txt") if f else None
    0
    '''
    pass