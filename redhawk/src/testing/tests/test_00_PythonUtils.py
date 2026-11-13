#!/usr/bin/env python3
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
import gc
import os
import tempfile
import shutil
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

        self.assertEqual(objref, objref2)
        self.assertEqual(objref.foo(), obj.foo())

        # Delete what should be the only reference to the original object.
        del obj
        self.assertEqual(ref(), None)

        try:
            objref.foo()
        except ReferenceError:
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

        gc.collect()
        try:
            objfoo()
        except ReferenceError:
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
        except ReferenceError:
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


class TestBulkioDataHelpers(unittest.TestCase):
    """Regression tests for ossie.utils.bulkio.bulkio_data_helpers.

    Guards against the Python 3 regression where octet/char (structFormat
    "B"/"b") data could not be written because array.array lost fromstring()
    in Python 3.9 (replaced by frombytes()). See FileSink.pushPacket.
    """
    def setUp(self):
        try:
            from bulkio.bulkioInterfaces import BULKIO__POA
            from ossie.utils.bulkio import bulkio_data_helpers
        except ImportError as e:
            self.skipTest("bulkio python support not available: %s" % e)
        self.BULKIO__POA = BULKIO__POA
        self.helpers = bulkio_data_helpers
        self.tmpdir = tempfile.mkdtemp()

    def tearDown(self):
        shutil.rmtree(self.tmpdir, ignore_errors=True)

    def _roundtrip(self, porttype, payload):
        path = os.path.join(self.tmpdir, "sink_out")
        sink = self.helpers.FileSink(path, porttype)
        # ts is unused for the octet/char write path; EOS=True flushes+closes.
        sink.pushPacket(payload, None, True, "regr")
        with open(path, "rb") as fp:
            return fp.read()

    def test_FileSink_octet_bytes(self):
        # Full 0..255 range exercises the unsigned octet path.
        payload = bytes(range(256))
        self.assertEqual(self._roundtrip(self.BULKIO__POA.dataOctet, payload),
                         payload)

    def test_FileSink_char_bytes(self):
        payload = bytes(range(256))
        self.assertEqual(self._roundtrip(self.BULKIO__POA.dataChar, payload),
                         payload)

    def test_FileSink_octet_str(self):
        # A str payload must still serialize (latin-1 -> raw bytes).
        text = "".join(chr(i) for i in range(256))
        self.assertEqual(self._roundtrip(self.BULKIO__POA.dataOctet, text),
                         text.encode("latin-1"))


class TestNumpyStringCompat(unittest.TestCase):
    """Regression tests for the numpy string->bytes API migration.

    numpy.ndarray.tostring()/numpy.fromstring() are deprecated (removed in
    numpy 2.0); the code now uses tobytes()/frombuffer(). frombuffer() returns
    a read-only view, so the callers add .copy() to keep the writable-copy
    semantics fromstring() used to provide. These tests guard both.
    """
    def setUp(self):
        self.tmpdir = tempfile.mkdtemp()

    def tearDown(self):
        shutil.rmtree(self.tmpdir, ignore_errors=True)

    def test_bluefile_byte_roundtrip(self):
        # Exercises the write (tobytes) and read (frombuffer) byte paths.
        import numpy
        from ossie.utils.bluefile import bluefile
        indata = numpy.arange(-128, 128, dtype=numpy.int8)
        path = os.path.join(self.tmpdir, "test.tmp")
        hdr = bluefile.header(1000, 'SB')
        bluefile.write(path, hdr, indata)
        outhdr, outdata = bluefile.read(path)
        self.assertEqual(outhdr['format'], 'SB')
        self.assertEqual(list(numpy.asarray(outdata, dtype=numpy.int8)),
                         list(indata))

    def test_burstio_chartraits_writable(self):
        # CharTraits.toArray must return a writable copy (frombuffer + .copy).
        import numpy
        try:
            from redhawk.burstio import traits
        except ImportError as e:
            self.skipTest("burstio python support not available: %s" % e)
        payload = bytes([250, 0, 1, 2, 255])
        # ByteTraits is CharTraits(..., numpy.int8); toArray uses frombuffer+copy
        arr = traits.ByteTraits.toArray(payload)
        self.assertEqual(list(arr),
                         list(numpy.frombuffer(payload, dtype=numpy.int8)))
        self.assertTrue(arr.flags.writeable)
        arr[0] = 7  # must not raise (read-only frombuffer view would)
        self.assertEqual(arr[0], 7)
