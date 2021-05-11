import ctypes

class CLib():
    def __init__(self, library_path) -> None:
        try:
            self.lib = ctypes.cdll.LoadLibrary(library_path)
        except OSError as err:
            raise ValueError("Could not load library. Reason: " + str(err))
    
    def get_fun(self, name, argtypes, restype):
        """
        Loads a function with the given name from the library and adds the function signature for type checking.

        :param name: The name of the function
        :param argtypes: The argument types
        :param restype: The return type
        """
        lib_fun = self.lib[name]
        lib_fun.restype = restype
        lib_fun.argtypes = argtypes
        return lib_fun
