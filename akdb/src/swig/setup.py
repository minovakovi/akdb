from setuptools import setup, Extension


kalashnikovDB_module = Extension('_kalashnikovDB',
                                 sources=['kalashnikovDB_wrap.c'],
                                 libraries=['ssl', 'crypto'],
                                 library_dirs=['/usr/lib', '/usr/lib/x86_64-linux-gnu'],
                                 include_dirs=['/usr/include/openssl'],
                                 extra_link_args=['-lssl', '-lcrypto'],
                                 )

setup (name = 'kalashnikovDB',
       description = """kalashnikovDB database system""",
       ext_modules = [kalashnikovDB_module],
       py_modules = ["kalashnikovDB"],
       )
