from distutils.core import setup, Extension


kalashnikovDB_module = Extension('_kalashnikovDB',
                           sources=['kalashnikovDB_wrap.c'],
                           )

setup (name = 'kalashnikovDB',
       description = """kalashnikovDB database system""",
       ext_modules = [kalashnikovDB_module],
       py_modules = ["kalashnikovDB"],
       )
