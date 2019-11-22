def _reset_sys_path():
    # Clear generic sys.path[0]
    import sys
    import os

    resources = os.environ['RESOURCEPATH']
    while sys.path[0] == resources:
        del sys.path[0]


_reset_sys_path()
