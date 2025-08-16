import importlib
import importlib.metadata as importMetadata
import sys

__all__ = []
__version__ = "unknown"


def loadExtension():
    """
    Attempt to import the compiled WAAO extension.
    Raise a clear error if it cannot be found.
    """
    try:
        return importlib.import_module("waao._waao")
    except ImportError as err:
        raise ImportError(
            "The WAAO extension could not be imported.\n"
            "Make sure you have built it with:\n\n"
            "   python setup.py build_ext --inplace\n\n"
            "or reinstall the package."
        ) from err


extModule = loadExtension()
for attrName in dir(extModule):
    if not attrName.startswith("_"):
        globals()[attrName] = getattr(extModule, attrName)
        __all__.append(attrName)

try:
    __version__ = importMetadata.version("waao")
except importMetadata.PackageNotFoundError:
    __version__ = getattr(extModule, "__version__", "1.1.0")
sys.modules[__name__ + ".extModule"] = extModule

def availableSymbols():
    """
    Return a sorted list of public functions, classes, and constants
    exported by the WAAO extension.
    """
    return sorted(__all__)
