import os
import sys
from modulegraph import modulegraph
from macholib.util import in_system_path


def has_filename_filter(module):
    if isinstance(module, modulegraph.MissingModule):
        return True
    if hasattr(modulegraph, 'InvalidRelativeImport') \
            and isinstance(module, modulegraph.InvalidRelativeImport):
        return True
    return getattr(module, 'filename', None) is not None


def not_stdlib_filter(module, prefix=None):
    """
    Return False if the module is located in the standard library
    """
    if prefix is None:
        prefix = sys.prefix
    if module.filename is None:
        return True

    prefix = os.path.join(os.path.realpath(prefix), '')

    rp = os.path.realpath(module.filename)
    if rp.startswith(prefix):
        rest = rp[len(prefix):]
        if '/site-python/' in rest:
            return True
        elif '/site-packages/' in rest:
            return True
        else:
            return False

    if os.path.exists(os.path.join(prefix, ".Python")):
        # Virtualenv
        fn = os.path.join(
            prefix, "lib", "python%d.%d" % (sys.version_info[:2]),
            "orig-prefix.txt")

        if os.path.exists(fn):
            with open(fn, 'rU') as fp:
                prefix = fp.read().strip()

            if rp.startswith(prefix):
                rest = rp[len(prefix):]
                if '/site-python/' in rest:
                    return True
                elif '/site-packages/' in rest:
                    return True
                else:
                    return False

    return True


def not_system_filter(module):
    """
    Return False if the module is located in a system directory
    """
    return not in_system_path(module.filename)


def bundle_or_dylib_filter(module):
    """
    Return False if the module does not have a filetype attribute
    corresponding to a Mach-O bundle or dylib
    """
    return getattr(module, 'filetype', None) in ('bundle', 'dylib')
