"""
Detection of virtual environments
"""
import os
import sys

class InvalidVirtualEnvironment (RuntimeError): pass

def detect_virtualenv():
    """
    Detect if the current proces is in a virtual environment and return
    information about it. Returns None if the proces is not in a virtual
    environment
    """

    if os.path.exists(os.path.join(sys.prefix, "pyvenv.cfg")):
        # "venv" virtual environment

        real_prefix = None
        global_site_packages = False
        with open(os.path.join(sys.prefix, "pyvenv.cfg")) as fp:

            for ln in fp:
                if ln.startswith('home = '):
                    _, home_path = ln.split('=', 1)
                    real_prefix = os.path.dirname(home_path.strip())

                elif ln.startswith('include-system-site-packages = '):
                    _, global_site_packages = ln.split('=', 1)
                    global_site_packages = (global_site_packages == 'true')

        if real_prefix is None:
            raise InvalidVirtualEnvironment(
                "'venv' virtual environment detected, but cannot determine base prefix")

        return dict(
            kind="venv",
            real_prefix=real_prefix,
            global_site_packages=global_site_packages,
        )

    elif os.path.exists(os.path.join(sys.prefix, ".Python")):
        # "virtualenv" virtual environment

        global_site_packages = not os.path.exists(
            os.path.join(
                os.path.dirname(site.__file__),
                        'no-global-site-packages.txt'))

        return dict(
            kind="virtualenv",
            real_prefix=sys.real_prefix,
            global_site_packages=global_site_packages)

    else:
        # No virtual environment
        return None
