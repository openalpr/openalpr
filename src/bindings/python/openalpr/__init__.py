import sys as _sys

if _sys.version_info.major >= 3:
    from .openalpr import Alpr
else:
    from openalpr import Alpr