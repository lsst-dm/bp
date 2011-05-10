"""
Dependencies and configuration for {product}.
"""
import os.path

def _get_root():
    """Return the root directory of the package."""
    ups, filename = os.path.split(__file__)
    return os.path.abs(os.path.join(ups, ".."))

dependencies = {
    # Names of packages required to build against this package.
    "required": [],

    # Names of packages optionally setup when building against this package.
    "optional": [],

    # Names of packages required to build this package, but not required to build against it.
    "buildRequired": [],

    # Names of packages optionally setup when building this package, but not used in building against it.
    "buildOptional": [],

    }

def setup(env, products, build=False):
    """
    Update an SCons environment to make use of the package.

    Arguments:
     env ------- An SCons environment to update.
     products -- A dictionary consisting of all dependencies and the return values of calls to their
                 setup() functions, or None if the dependency was optional and was not found.
     build ----- If True, this is the product currently being built, and products in "buildRequired" and
                 "buildOptional" dependencies will also be present in the products dict.
    """
    env.PrependUnique(**paths)
    env.AppendUnique(**doxygen)
    for target in libs:
        if target not in env["libs"]:
            env["libs"][target] = lib[target].copy()
        else:
            for lib in libs[target]:
                if lib not in env["libs"][target]:
                    env["libs"][target].append(lib)
    return {"paths": paths, "doxygen": doxygen, "libs": libs, "extra": {}}


###################################################################################################
# Variables for default implementation of setup() below; if the user provides 
# a custom implementation of setup(), everything below is unnecessary.

# Packages to be added to the environment.
paths = {
    # Sequence of paths to add to the include path.
    "CPPPATH": [os.path.join(_get_root(), "include")],

    # Sequence of paths to add to the linker path.
    "LIBPATH": [os.path.join(_get_root(), "lib")],
    
    }

doxygen = {
    # Sequence of Doxygen tag files produced by this product.
    "DOXYGEN_TAGFILES": [os.path.join(_get_root(), "doc", "{product}.tag")],

    # Sequence of Doxygen configuration files to include in dependent products.
    "DOXYGEN_INCLUDES": [],

    }

# Libraries provided by the package, not including standard library prefixes or suffixes.
# Additional custom targets besides the standard "main", "python", and "test" targets may
# be provided as well.
libs = {
    # Normal libraries.
    "main": ["{product}"],

    # Libraries only linked with C++-coded Python modules.
    "python": [],
    
    # Libraries only linked with C++-coded unit tests.
    "test": [],

    }

