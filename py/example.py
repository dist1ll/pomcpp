import ctypes
import sys

try:
    lib = ctypes.cdll.LoadLibrary("./Release/libpomcpp.so")
except OSError as err:
    print("Could not load library. Reason: " + str(err))
    sys.exit(1)


def get_c_fun(name, argtypes, restype):
    """
    Loads a function with the given name from the library and adds the function signature for type checking.

    :param name: The name of the function
    :param argtypes: The argument types
    :param restype: The return type
    """
    lib_fun = lib[name]
    lib_fun.restype = restype
    lib_fun.argtypes = argtypes
    return lib_fun


hello = get_c_fun("hello_world", [ctypes.c_char_p], ctypes.c_int)

str_input = "This is input from python.".encode('utf-8')
res = hello(str_input)

print(f"Result: {res}")
