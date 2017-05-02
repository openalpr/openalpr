import ctypes
import json
import platform

# We need to do things slightly differently for Python 2 vs. 3
# ... because the way str/unicode have changed to bytes/str
if platform.python_version_tuple()[0] == '2':
    # Using Python 2
    bytes = str
    _PYTHON_3 = False
else:
    # Assume using Python 3+
    unicode = str
    _PYTHON_3 = True


def _convert_to_charp(string):
    # Prepares function input for use in c-functions as char*
    if type(string) == unicode:
        return string.encode("UTF-8")
    elif type(string) == bytes:
        return string
    else:
        raise TypeError("Expected unicode string values or ascii/bytes values. Got: %r" % type(string))


def _convert_from_charp(charp):
    # Prepares char* output from c-functions into Python strings
    if _PYTHON_3 and type(charp) == bytes:
        return charp.decode("UTF-8")
    else:
        return charp


class Alpr:
    def __init__(self, country, config_file, runtime_dir):
        """
        Initializes an OpenALPR instance in memory.

        :param country: The default region for license plates. E.g., "us" or "eu"
        :param config_file: The path to the OpenALPR config file
        :param runtime_dir: The path to the OpenALPR runtime data directory
        :return: An OpenALPR instance
        """
        country = _convert_to_charp(country)
        config_file = _convert_to_charp(config_file)
        runtime_dir = _convert_to_charp(runtime_dir)
        try:
            # Load the .dll for Windows and the .so for Unix-based
            if platform.system().lower().find("windows") != -1:
                self._openalprpy_lib = ctypes.cdll.LoadLibrary("libopenalprpy.dll")
            elif platform.system().lower().find("darwin") != -1:
                self._openalprpy_lib = ctypes.cdll.LoadLibrary("libopenalprpy.dylib")
            else:
                self._openalprpy_lib = ctypes.cdll.LoadLibrary("libopenalprpy.so")
        except OSError as e:
            nex = OSError("Unable to locate the OpenALPR library. Please make sure that OpenALPR is properly "
                          "installed on your system and that the libraries are in the appropriate paths.")
            if _PYTHON_3:
                nex.__cause__ = e;
            raise nex

        self._initialize_func = self._openalprpy_lib.initialize
        self._initialize_func.restype = ctypes.c_void_p
        self._initialize_func.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]

        self._dispose_func = self._openalprpy_lib.dispose
        self._dispose_func.argtypes = [ctypes.c_void_p]

        self._is_loaded_func = self._openalprpy_lib.isLoaded
        self._is_loaded_func.argtypes = [ctypes.c_void_p]
        self._is_loaded_func.restype = ctypes.c_bool

        self._recognize_file_func = self._openalprpy_lib.recognizeFile
        self._recognize_file_func.restype = ctypes.c_void_p
        self._recognize_file_func.argtypes = [ctypes.c_void_p, ctypes.c_char_p]

        self._recognize_array_func = self._openalprpy_lib.recognizeArray
        self._recognize_array_func.restype = ctypes.c_void_p
        self._recognize_array_func.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_ubyte), ctypes.c_uint]

        try:
            import numpy as np
            import numpy.ctypeslib as npct
            self._recognize_raw_image_func = self._openalprpy_lib.recognizeRawImage
            self._recognize_raw_image_func.restype = ctypes.c_void_p
            array_1_uint8 = npct.ndpointer(dtype=np.uint8, ndim=1, flags='CONTIGUOUS')
            self._recognize_raw_image_func.argtypes = [
                ctypes.c_void_p, array_1_uint8, ctypes.c_uint, ctypes.c_uint, ctypes.c_uint]
        except ImportError:
            self._recognize_raw_image_func = None

        self._free_json_mem_func = self._openalprpy_lib.freeJsonMem

        self._set_country_func = self._openalprpy_lib.setCountry
        self._set_country_func.argtypes = [ctypes.c_void_p, ctypes.c_char_p]

        self._set_prewarp_func = self._openalprpy_lib.setPrewarp
        self._set_prewarp_func.argtypes = [ctypes.c_void_p, ctypes.c_char_p]

        self._set_default_region_func = self._openalprpy_lib.setDefaultRegion
        self._set_default_region_func.argtypes = [ctypes.c_void_p, ctypes.c_char_p]

        self._set_detect_region_func = self._openalprpy_lib.setDetectRegion
        self._set_detect_region_func.argtypes = [ctypes.c_void_p, ctypes.c_bool]

        self._set_top_n_func = self._openalprpy_lib.setTopN
        self._set_top_n_func.argtypes = [ctypes.c_void_p, ctypes.c_int]

        self._get_version_func = self._openalprpy_lib.getVersion
        self._get_version_func.argtypes = [ctypes.c_void_p]
        self._get_version_func.restype = ctypes.c_void_p

        self.alpr_pointer = self._initialize_func(country, config_file, runtime_dir)

        self.loaded = True

    def unload(self):
        """
        Unloads OpenALPR from memory.

        :return: None
        """

        if self.loaded:
            self.loaded = False
            self._openalprpy_lib.dispose(self.alpr_pointer)

    def is_loaded(self):
        """
        Checks if OpenALPR is loaded.

        :return: A bool representing if OpenALPR is loaded or not
        """
        if not self.loaded:
            return False

        return self._is_loaded_func(self.alpr_pointer)

    def recognize_file(self, file_path):
        """
        This causes OpenALPR to attempt to recognize an image by opening a file on
        disk.

        :param file_path: The path to the image that will be analyzed
        :return: An OpenALPR analysis in the form of a response dictionary
        """
        file_path = _convert_to_charp(file_path)
        ptr = self._recognize_file_func(self.alpr_pointer, file_path)
        json_data = ctypes.cast(ptr, ctypes.c_char_p).value
        json_data = _convert_from_charp(json_data)
        response_obj = json.loads(json_data)
        self._free_json_mem_func(ctypes.c_void_p(ptr))
        return response_obj

    def recognize_array(self, byte_array):
        """
        This causes OpenALPR to attempt to recognize an image passed in as a byte array.

        :param byte_array: This should be a string (Python 2) or a bytes object (Python 3)
        :return: An OpenALPR analysis in the form of a response dictionary
        """
        if type(byte_array) != bytes:
            raise TypeError("Expected a byte array (string in Python 2, bytes in Python 3)")
        pb = ctypes.cast(byte_array, ctypes.POINTER(ctypes.c_ubyte))
        ptr = self._recognize_array_func(self.alpr_pointer, pb, len(byte_array))
        json_data = ctypes.cast(ptr, ctypes.c_char_p).value
        json_data = _convert_from_charp(json_data)
        response_obj = json.loads(json_data)
        self._free_json_mem_func(ctypes.c_void_p(ptr))
        return response_obj

    def recognize_ndarray(self, ndarray):
        """
        This causes OpenALPR to attempt to recognize an image passed in as a numpy array.

        :param ndarray: numpy.array as used in cv2 module
        :return: An OpenALPR analysis in the form of a response dictionary
        """
        if self._recognize_raw_image_func is None:
            raise RuntimeError('NumPy missing')
        height, width = ndarray.shape[:2]
        bpp = ndarray.shape[2] if len(ndarray.shape) > 2 else 1
        ptr = self._recognize_raw_image_func(self.alpr_pointer, ndarray.flatten(), bpp, width, height)
        json_data = ctypes.cast(ptr, ctypes.c_char_p).value
        json_data = _convert_from_charp(json_data)
        response_obj = json.loads(json_data)
        self._free_json_mem_func(ctypes.c_void_p(ptr))
        return response_obj

    def get_version(self):
        """
        This gets the version of OpenALPR

        :return: Version information
        """

        ptr = self._get_version_func(self.alpr_pointer)
        version_number = ctypes.cast(ptr, ctypes.c_char_p).value
        version_number = _convert_from_charp(version_number)
        self._free_json_mem_func(ctypes.c_void_p(ptr))
        return version_number

    def set_top_n(self, topn):
        """
        Sets the number of returned results when analyzing an image. For example,
        setting topn = 5 returns the top 5 results.

        :param topn: An integer that represents the number of returned results.
        :return: None
        """
        self._set_top_n_func(self.alpr_pointer, topn)

    def set_country(self, country):
        """
        This sets the country for detecting license plates. For example,
        setting country to "us" for United States or "eu" for Europe.

        :param country: A unicode/ascii string (Python 2/3) or bytes array (Python 3)
        :return: None
        """
        country = _convert_to_charp(country)
        self._set_country_func(self.alpr_pointer, country)

    def set_prewarp(self, prewarp):
        """
        Updates the prewarp configuration used to skew images in OpenALPR before
        processing.

        :param prewarp: A unicode/ascii string (Python 2/3) or bytes array (Python 3)
        :return: None
        """
        prewarp = _convert_to_charp(prewarp)
        self._set_prewarp_func(self.alpr_pointer, prewarp)

    def set_default_region(self, region):
        """
        This sets the default region for detecting license plates. For example,
        setting region to "md" for Maryland or "fr" for France.

        :param region: A unicode/ascii string (Python 2/3) or bytes array (Python 3)
        :return: None
        """
        region = _convert_to_charp(region)
        self._set_default_region_func(self.alpr_pointer, region)

    def set_detect_region(self, enabled):
        """
        This allows OpenALPR to attempt to detect the region of a license plate
        automatically. By default this is disabled, but you can enable it here.

        :param enabled: A boolean representing whether or not auto-detection is enabled
        :return: None
        """
        self._set_detect_region_func(self.alpr_pointer, enabled)

    def __del__(self):
        if self.is_loaded():
            self.unload()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        if self.is_loaded():
            self.unload()


