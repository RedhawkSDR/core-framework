#
# This file is protected by Copyright. Please refer to the COPYRIGHT file 
# distributed with this source distribution.
# 
# This file is part of REDHAWK core.
# 
# REDHAWK core is free software: you can redistribute it and/or modify it under 
# the terms of the GNU Lesser General Public License as published by the Free 
# Software Foundation, either version 3 of the License, or (at your option) any 
# later version.
# 
# REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
# 
# You should have received a copy of the GNU Lesser General Public License 
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

import os
import subprocess
import threading
import errno

# subprocess.Popen() is not thread-safe in most of the current versions of
# Python, leading to unintuitive lockups. Therefore, we provide an explicitly
# thread-safe interface by placing a lock around the call to Popen().
_popen_lock = threading.Lock()

def Popen(*args, **kwargs):
    """
    Wrapper for subprocess.Popen() that provides thread-safety and works around
    other known bugs.
    """
    _popen_lock.acquire()
    try:
        # Python's subprocess module has a bug where it propagates the
        # exception to the caller when it gets interrupted trying to read the
        # status back from the child process, leaving the child process
        # effectively orphaned and registering a false failure. To work around
        # it, we temporarily replace os.read with a retrying version that
        # allows Popen to succeed in this case.
        class RetryFunc(object):
            def __init__ (self, func):
                import os
                self.func = func

            def __call__ (self, *args, **kwargs):
                while True:
                    try:
                        return self.func(*args, **kwargs)
                    except OSError, e:
                        if e.errno != errno.EINTR:
                            raise

        reader = RetryFunc(os.read)
        os.read = reader
        return subprocess.Popen(*args, **kwargs)
    finally:
        os.read = reader.func
        _popen_lock.release()
