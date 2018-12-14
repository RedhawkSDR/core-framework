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

import unittest, os
from _unitTestHelpers import scatest
from ossie.cf import CF
from omniORB import any
from ossie.utils import sb

@scatest.requireJava
class TestDeviceJavaWO(scatest.CorbaTestCase):

    def setUp(self):
        self.dev=sb.launch('writeonly_java')

    def tearDown(self):
        sb.release()

    def test_writeonly_java(self):
        simple = CF.DataType(id='foo', value=any.to_any(None))
        self.assertRaises(CF.UnknownProperties, self.dev.query, [simple])
        simple_seq = CF.DataType(id='foo_seq', value=any.to_any(None))
        self.assertRaises(CF.UnknownProperties, self.dev.query, [simple_seq])
        struct = CF.DataType(id='foo_struct', value=any.to_any(None))
        self.assertRaises(CF.UnknownProperties, self.dev.query, [struct])
        struct_seq = CF.DataType(id='foo_struct_seq', value=any.to_any(None))
        self.assertRaises(CF.UnknownProperties, self.dev.query, [struct_seq])

class TestDeviceCppWO(scatest.CorbaTestCase):

    def setUp(self):
        self.dev=sb.launch('writeonly_cpp')

    def tearDown(self):
        sb.release()

    def test_writeonly_cpp(self):
        simple = CF.DataType(id='foo', value=any.to_any(None))
        self.assertRaises(CF.UnknownProperties, self.dev.query, [simple])
        simple_seq = CF.DataType(id='foo_seq', value=any.to_any(None))
        self.assertRaises(CF.UnknownProperties, self.dev.query, [simple_seq])
        struct = CF.DataType(id='foo_struct', value=any.to_any(None))
        self.assertRaises(CF.UnknownProperties, self.dev.query, [struct])
        struct_seq = CF.DataType(id='foo_struct_seq', value=any.to_any(None))
        self.assertRaises(CF.UnknownProperties, self.dev.query, [struct_seq])

class TestDevicePythonWO(scatest.CorbaTestCase):

    def setUp(self):
        self.dev=sb.launch('writeonly_py')

    def tearDown(self):
        sb.release()

    def test_writeonly_py(self):
        simple = CF.DataType(id='foo', value=any.to_any(None))
        self.assertRaises(CF.UnknownProperties, self.dev.query, [simple])
        simple_seq = CF.DataType(id='foo_seq', value=any.to_any(None))
        self.assertRaises(CF.UnknownProperties, self.dev.query, [simple_seq])
        struct = CF.DataType(id='foo_struct', value=any.to_any(None))
        self.assertRaises(CF.UnknownProperties, self.dev.query, [struct])
        struct_seq = CF.DataType(id='foo_struct_seq', value=any.to_any(None))
        self.assertRaises(CF.UnknownProperties, self.dev.query, [struct_seq])
