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

import inspect
import traceback
import sys

def _notification_signature(func):
    # To provide meaningful documentation of the signature of a
    # notification, inspect the wrapped function to get its arguments.
    args, varargs, varkw, defaults = inspect.getargspec(func)
    if len(args) > 0 and args[0] == 'self':
        # Discard "self" argument, if it exists.
        args = args[1:]
    return func.__name__+inspect.formatargspec(args, varargs, varkw, defaults)

class notify_callback(object):
    """
    Container for notification callbacks with optional filtering.
    """
    def __init__(self, func, match):
        self.nc_func = func
        self.nc_match = match

    def __call__(self, *args, **kwargs):
        if self.nc_match:
            # Prevent malformed match functions from derailing the entire
            # notification process
            try:
                match = self.nc_match(*args, **kwargs)
            except:
                print >>sys.stderr, 'Exception in match function for notification %s:' % (repr(self.nc_func),)
                traceback.print_exception(*sys.exc_info())

                # Treat an exception in the function as a negative response
                match = False

            if not match:
                return None
        return self.nc_func(*args, **kwargs)

    def __eq__(self, other):
        return self.nc_func == other

class notification_func(object):
    """
    A notification function.
    """
    def __init__(self, func, listeners=[]):
        self.nm_func = func
        self.nm_listeners = listeners
        for attr in ('__name__', '__doc__', '__module__'):
            setattr(self, attr, getattr(func, attr))            

    def __call__(self, *args, **kwargs):
        rv = self.execute(*args, **kwargs)
        self.notify(*args, **kwargs)
        return rv

    def execute(self, *args, **kwargs):
        """
        Execute the wrapped function. No notification occurs.
        """
        return self.nm_func(*args, **kwargs)

    def notify(self, *args, **kwargs):
        """
        Call all listener callbacks. The wrapped function is not executed.
        """
        for listener in self.nm_listeners:
            # Ensure that all callbacks get called, and no exceptions escape to
            # the caller.
            try:
                listener(*args, **kwargs)
            except:
                # If the target is a notify_callback--which should always be
                # the case as long as the proper API is used--show the actual
                # function
                target = getattr(listener, 'nc_func', listener)
                print >>sys.stderr, 'Exception in notification %s:' % (repr(target),)
                traceback.print_exception(*sys.exc_info())

class notification_method(notification_func):
    """
    A notification instance method.

    The first argument to a call is assumed to be the containing object, and
    is only used for execution, not notification.
    """
    def __call__(self, obj, *args, **kwargs):
        rv = self.execute(obj, *args, **kwargs)
        self.notify(*args, **kwargs)
        return rv

class bound_notification(object):
    """
    A notification method associated with an object instance.
    """
    def __init__(self, func, obj, owner):
        for attr in ('__name__', '__doc__', '__module__'):
            setattr(self, attr, getattr(func, attr))
        self.im_self = obj
        self.im_class = owner
        self.im_func = func

    def __call__(self, *args, **kwargs):
        """
        Executes the notification function and notifies any listeners.
        """
        func = notification_method(self.im_func, self.listeners)
        return func(self.im_self, *args, **kwargs)

    def __getattr__(self, name):
        return getattr(self.im_func, name)

    @property
    def listeners(self):
        # The list of listeners is stored as an attribute on the containing
        # object. If it doesn't exist, initialize it to an empty list first.
        # Since lists are passed by reference, this allows modifications to the
        # returned list without needing a reference to the owning object.
        attrname = '__' + self.__name__ + '_listeners'
        if not hasattr(self.im_self, attrname):
            setattr(self.im_self, attrname, [])
        return getattr(self.im_self, attrname)

    def addListener(self, callback, match=None):
        """
        Adds a listener to be called when a notification occurs.

        To filter out unwanted notifications, a callable can be provided for
        'match' that is called with the notification argument prior to the
        callback executing; if it returns False, the notification is ignored.
        """
        self.listeners.append(notify_callback(callback, match))

    def removeListener(self, callback):
        """
        Stop sending notifications to 'callback'.
        """
        self.listeners.remove(callback)


class unbound_notification(object):
    def __init__(self, func, owner):
        for attr in ('__name__', '__module__'):
            setattr(self, attr, getattr(func, attr))
        self.im_func = func
        self.im_self = None
        self.im_class = owner
        self.__doc__ = "Notification '%s'." % (_notification_signature(self.im_func),)
        if self.im_func.__doc__:
            self.__doc__ += '\n' + self.im_func.__doc__

    def __call__(self, obj, *args, **kwargs):
        func = bound_notification(self.im_func, obj, self.im_class)
        return func(*args, **kwargs)

    def __get__(self, obj, owner):
        # NB: Having __get__ defines causes the help for this class to be
        #     displayed as though it were a method.
        return self


class notification(object):
    """
    Decorator for functions that are intended as notification points.

    For objects that wish to notify listeners of changes in state, methods can
    be decorated with "@notification" to indicate that listeners may connect to
    the method itself.

      class observable(object):
        @notification
        def stateChanged(self, newState):
          \"""
          The state of the object changed to 'newState'.
          \"""
          pass

    Typically, this method will do nothing (or very little) on its own.

    Listeners should have the same signature, except for the initial "self"
    argument (unless they are themselves instance methods).

    To notify listeners, call the method with the appropriate arguments for the
    wrapped function.

    Standalone functions can also be decorated as notifications. The original
    function is available via the 'func' attribute.
    """
    def __init__(self, func):
        for attr in ('__name__', '__module__'):
            setattr(self, attr, getattr(func, attr))
        self.func = func

        self.__doc__ = "Notification '%s'." % (_notification_signature(self.func),)
        if self.func.__doc__:
            self.__doc__ += '\n' + self.func.__doc__

    def __get__(self, obj, owner):
        if obj is None:
            return unbound_notification(self.func, owner)

        # Return a new callable that binds the object to the method, just like
        # regular bound method; the caller can then apply the same techniques
        # for weak references. The advantage of using our own type here is more
        # control over the doc string.
        return bound_notification(self.func, obj, owner)

    def __set__(self, obj, value):
        # Notifications cannot be set; however, having __set__ defined makes
        # help() treat notifications as data descriptors instead of methods.
        # For standalone functions, this means that help() will show the
        # available methods rather than the function documentation.
        raise AttributeError, 'notification cannot be set'

    def __call__(self, *args, **kwargs):
        # This notification is being used as a free-standing function; get the
        # listener list stored on this object to create a temporary callable.
        func = notification_func(self.func, self.listeners)
        return func(*args, **kwargs)

    @property
    def listeners(self):
        """
        The list of currently registered callbacks for this notification.
        """
        attr = '__listeners__'
        if not hasattr(self, attr):
            setattr(self, attr, [])
        return getattr(self, attr)

    def addListener(self, callback, match=None):
        """
        Adds a listener to be called when a notification occurs.

        To filter out unwanted notifications, a callable can be provided for
        'match' that is called with the notification argument prior to the
        callback executing; if it returns False, the notification is ignored.
        """
        self.listeners.append(notify_callback(callback, match))

    def removeListener(self, callback):
        """
        Stop sending notifications to 'callback'.
        """
        self.listeners.remove(callback)
