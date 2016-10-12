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

"""
Utility module to extend the capabilities of weak references.
"""

import weakref
import inspect
import types

from notify import bound_notification

def getref(obj):
    """
    Returns a strong reference to the weakly-referenced object 'obj'. If 'obj'
    is a weakly-bound method, returns a regular bound method.
    """
    return type(obj).__getref__(obj)

def objectref(obj, callback=None):
    """
    Return a weakly-referenced proxy object. If callback is provided and not
    None, the callback will be called when the referenced object is about to be
    finalized; the weak object will be passed to the callback.
    """
    return WeakObject(obj, callback)

def boundmethod(func, callback=None):
    """
    Return a weakly-bound instance method from the given bound method. If
    callback is provided and not None, the callback will be called when the
    referenced object (the method's self) is about to be finalized; the weakly-
    bound instance method will be passed to the callback.

    If func is already a weakly-bound instance method, return a new weakly-
    bound instance method to the underlying bound method.
    """
    if inspect.ismethod(func):
        return WeakBoundMethod(func, callback)
    elif isinstance(func, bound_notification):
        return WeakNotification(func, callback)
    elif isinstance(func, _WeakCallable):
        return boundmethod(getref(func), callback)
    else:
        raise TypeError("can not create weakly-bound method from '%s' object" % (type(func).__name__,))

def addListener(target, listener, *args):
    """
    Register bound method 'listener' with notification 'target', using weak
    binding to ensure that the notification does not keep the listener's
    object alive. When the object associated with the bound method is
    finalized, the listener is automatically removed.
    """
    target.addListener(boundmethod(listener, callback=target.removeListener), *args)

def _ismethod(func):
    for name in ('im_self', 'im_class', 'im_func'):
        if not hasattr(func, name):
            return False
    return True

def _make_callback(obj, callback):
    # Wraps the given callback function for use with weakref.ref callbacks,
    # ensuring that a reference cycle is not created between the wrapper object
    # and the weak reference.
    if callback is not None:
        self_ref = weakref.ref(obj)
        def delref(ref):
            callback(self_ref())
        return delref
    else:
        return None

class WeakObject(object):
    """
    A weakly-referenced proxy to an object.

    As compared to weakref.proxy, WeakObject returns weakly-bound instance
    methods to avoid creating unexpected object references.
    """
    def __init__(self, ref, callback=None):
        self.__ref__ = weakref.ref(ref, _make_callback(self, callback))

    def __getref__(self):
        ref = self.__ref__()
        if ref is None:
            raise weakref.ReferenceError('weakly-referenced object no longer exists')
        return ref

    def __getattribute__(self, name):
        if name in ('__ref__','__getref__'):
            return object.__getattribute__(self, name)
        else:
            attr = getattr(getref(self), name)
            if _ismethod(attr):
                return boundmethod(attr)
            return attr

    def __setattr__(self, name, value):
        if name in ('__ref__',):
            object.__setattr__(self, name, value)
        else:
            setattr(getref(self), name, value)

    def __str__(self):
        return str(getref(self))

    def __repr__(self):
        return '<weak object at 0x%x proxy to %s>' % (id(self), repr(self.__ref__()))

    def __eq__(self, other):
        if not isinstance(other, WeakObject):
            return False
        return self.__ref__ == other.__ref__

class _WeakBoundCallable(object):
    """
    Base class for weakly-bound callable objects (methods and notifications).
    """
    def __init__(self, func, callback):
        self.im_self = weakref.ref(func.im_self, _make_callback(self, callback))

    def __call__(self, *args, **kwargs):
        func = getref(self)
        return func(*args, **kwargs)

    def __getref__(self):
        ref = self.im_self()
        if ref is None:
            raise weakref.ReferenceError('weakly-referenced object no longer exists')
        return self.__functype__(self.im_func, ref, self.im_class)

    def __eq__(self, other):
        if not isinstance(other, type(self)):
            return False
        return (self.im_func == other.im_func) and \
            (self.im_self == other.im_self) and \
            (self.im_class == other.im_class)

    def __repr__(self):
        name = self.im_class.__name__ + '.' + self.im_func.__name__
        return '<weak bound %s %s of %s>' % (self.__funckind__, name, self.im_self)

class WeakBoundMethod(_WeakBoundCallable):
    """
    A bound instance method on a weakly-referenced object.

    Unlike normal bound methods, a WeakBoundMethod does not hold a strong
    reference back to the owning object. This can be used, for example, to
    prevent callbacks from inadvertently keeping an object alive.
    """
    __functype__ = types.MethodType
    __funckind__ = 'method'

    def __init__(self, func, callback=None):
        if not _ismethod(func):
            raise TypeError("can not create weakly-bound method from '%s' object" % (type(func).__name__,))
        _WeakBoundCallable.__init__(self, func, callback)
        self.im_func = func.im_func
        self.im_class = func.im_class

class WeakNotification(_WeakBoundCallable, bound_notification):
    """
    A bound notification on a weakly-referenced object.
    """
    __functype__ = bound_notification
    __funckind__ = 'notification'

    def __init__(self, func, callback=None):
        if not isinstance(func, bound_notification):
            raise TypeError("can not create weakly-bound notification from '%s' object" % (type(func).__name__,))
        bound_notification.__init__(self, func.im_func, None, func.im_class)
        _WeakBoundCallable.__init__(self, func, callback)

    @property
    def listeners(self):
        return getref(self).listeners

WeakTypes = (WeakObject, WeakBoundMethod, WeakNotification)

# Provide meaningful help for weak objects by forwarding help requested to the
# referenced object, instead of the default behavior of showing help for the
# weak object.
import site
import __builtin__
class _WeakObjectHelper(site._Helper):
    def __call__(self, request, *args, **kwargs):
        if isinstance(request, WeakTypes):
            request = getref(request)
        return super(_WeakObjectHelper,self).__call__(request, *args, **kwargs)
__builtin__.help = _WeakObjectHelper()
