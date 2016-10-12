#!/usr/bin/env python
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

import unittest
import weakref

from ossie.utils.notify import notification
from ossie.utils import weakobj

class TestClass(object):
    def foo(self):
        return 'foo'

    @notification
    def bar(self):
        pass

class TestPythonUtils(unittest.TestCase):
    """
    Test cases for Python modules that are not directly part of the component
    API and have no dependency on REDHAWK behaviors, such as notifications and
    extended weak object support.
    """
    def _notificationTest(self, sender, *args, **kwargs):
        # Simple receiver that just saves the arguments
        results = []
        def receiver(*args, **kwargs):
            results.append((args, kwargs))

        # Add a listener and check that the arguments are received.
        sender.addListener(receiver)
        sender(*args, **kwargs)
        self.assertEqual(len(results), 1)
        self.assertEqual(results, [(args, kwargs)])

        # Remove the listener and check that it is not called.
        sender.removeListener(receiver)
        sender(*args, **kwargs)
        self.assertEqual(len(results), 1)

        # Another receiver that adds its name to the received arguments, so
        # that it can be distinguished 
        def receiver2(*args, **kwargs):
            results.append(('receiver2', args, kwargs))

        # Clear the results, connect both receivers, and check the results.
        results = []
        sender.addListener(receiver)
        sender.addListener(receiver2)
        sender(*args, **kwargs)
        self.assertEqual(len(results), 2)
        # Don't assume the notifications occur in order; instead, check all of
        # the returned results.
        for msg in results:
            if msg[0] == 'receiver2':
                msg = msg[1:]
            self.assertEqual(msg, (args, kwargs))

        # Remove the second receiver and check that only the first is called.
        results = []
        sender.removeListener(receiver2)
        sender(*args, **kwargs)
        self.assertEqual(len(results), 1)
        self.assertEqual(results, [(args, kwargs)])

    def test_NotifyMethod(self):
        """
        Tests member notification functions.
        """
        obj = TestClass()
        self._notificationTest(obj.bar)
    
    def test_NotifyFunction(self):
        """
        Tests free-standing notification functions.
        """
        @notification
        def notifier(value, key=0, opt=None):
            pass

        self._notificationTest(notifier, 123, key='ABC')

    def test_NotifyMatch(self):
        """
        Tests filtering of notifications.
        """
        @notification
        def sender(key, value):
            pass

        class Receiver(object):
            def __init__(self, name):
                self.name = name
                self.values = []

            def __call__(self, key, value):
                self.values.append(value)

            def match(self, key, value):
                return key in (self.name, '*')

        recv1 = Receiver('A')
        recv2 = Receiver('B')

        sender.addListener(recv1, recv1.match)
        sender.addListener(recv2, recv2.match)

        sender('A', 1)
        sender('B', 2)
        sender('*', 3)
        sender('B', 4)

        self.assertEqual(recv1.values, [1,3])
        self.assertEqual(recv2.values, [2,3,4])

    def test_WeakObject(self):
        """
        Tests basic extended weak object support.
        """
        obj = TestClass()
        ref = weakref.ref(obj)

        objref = weakobj.objectref(obj)
        objref2 = weakobj.objectref(obj)

        self.assertEquals(objref, objref2)
        self.assertEquals(objref.foo(), obj.foo())

        # Delete what should be the only reference to the original object.
        del obj
        self.assertEqual(ref(), None)

        try:
            objref.foo()
        except weakref.ReferenceError:
            pass
        else:
            self.fail('Weak object should be invalidated')

    def test_WeakObjectInstanceMethodLifetime(self):
        """
        Checks that bound functions returned from a weak object do not prevent
        the referenced object from being deleted, as compared to weakref.proxy.
        """
        obj = TestClass()

        # Get a local reference to the bound method through a proxy; it will
        # increase obj's reference count.
        reffoo = weakref.proxy(obj).foo

        # Get a local reference to the bound method through a weak object; it
        # should not increase obj's reference count.
        objfoo = weakobj.objectref(obj).foo

        # Remove the original reference, and call the functions; this should
        # succeed because of reffoo's im_self.
        del obj
        reffoo()
        objfoo()

        # Remove the local reference-via-proxy, which should now allow the
        # original object to be deleted.
        del reffoo

        try:
            objfoo()
        except weakref.ReferenceError:
            pass
        else:
            self.fail('Weak object should be invalidated')

    def test_WeakObjectCallback(self):
        """
        Test that the weak object callback occurs as expected.
        """
        obj = TestClass()

        results = set()
        def callback(target):
            results.add(id(target))

        ref = weakobj.objectref(obj, callback)
        expected = set([id(ref)])
        del obj

        self.assertEqual(results, expected)

    def test_WeakBoundMethod(self):
        """
        Test that weakly-bound methods can be called like a regular bound
        method, and are invalidated on object destruction.
        """
        obj = TestClass()
        foo = weakobj.boundmethod(obj.foo)

        # Check that the weakly-bound method has the same behavior.
        self.assertEqual(obj.foo(), foo())

        # Check that weakly-bound notifications behave correctly.
        self._notificationTest(weakobj.boundmethod(obj.bar))

        # Delete the object and ensure that the method is invalidated.
        del obj
        try:
            foo()
        except weakref.ReferenceError:
            pass
        else:
            self.fail('Weak bound method should be invalidated')

    def test_WeakBoundMethodCallback(self):
        """
        Check that weakly-bound method finalizer callbacks are called as
        expected.
        """
        children = []

        # Add a weakly-bound member function that will remove itself from the
        # list when the object is destroyed.
        obj = TestClass()
        children.append(weakobj.boundmethod(obj.foo, children.remove))
        self.assertEqual(len(children), 1)

        # Add the same member function but with a different object.
        obj2 = TestClass()
        children.append(weakobj.boundmethod(obj2.foo, children.remove))
        self.assertEqual(len(children), 2)

        # Add a notification function from a third object.
        obj3 = TestClass()
        children.append(weakobj.boundmethod(obj3.bar, children.remove))
        self.assertEqual(len(children), 3)

        # Delete the second object and check that the callback removed only its
        # weak bound method from the list
        del obj2
        self.assertEqual(len(children), 2)
        self.assertEqual(children[0], weakobj.boundmethod(obj.foo))

        # Delete the first object, which should leave only the third object's
        # notification method
        del obj
        self.assertEqual(len(children), 1)
        self.assertEqual(children[0], weakobj.boundmethod(obj3.bar))

        del obj3
        self.assertEqual(len(children), 0)
