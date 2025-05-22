from setuptools import setup, Extension

kalashnikovDB_module = Extension(
    name='_kalashnikovDB',
    sources=['kalashnikovDB_wrap.c'],
    libraries=['crypto'],
)

setup(
    name='kalashnikovDB',
    version='0.1',
    description='kalashnikovDB database system',
    ext_modules=[kalashnikovDB_module],
    py_modules=['kalashnikovDB'],
)
