#!/usr/bin/python
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK bulkioInterfaces.
#
# REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

import unittest
from omniORB.any import to_any, from_any

from ossie.cf import CF

import bulkio
from bulkio.bulkioInterfaces import BULKIO

class StreamSRITest(unittest.TestCase):
    def assertSRIFieldsEqual(self, fields, sri):
        for field, expected in fields.iteritems():
            actual = getattr(sri, field)
            self.assertEqual(expected, actual, "sri.%s: expected '%s', actual '%s'" % (field, expected, actual))

    def testCreate(self):
        expected = {
            'hversion': 1,
            'xstart': 0.0,
            'xdelta': 1.0,
            'xunits': BULKIO.UNITS_TIME,
            'subsize': 0,
            'ystart': 0.0,
            'ydelta': 0.0,
            'yunits': BULKIO.UNITS_NONE,
            'mode': 0,
            'streamID': 'defStream',
            'blocking': False,
            'keywords': []
        }
                    
        sri = bulkio.sri.create()
        self.assertSRIFieldsEqual(expected, sri)

        # Test stream ID and sample rate arguments
        sample_rate = 2.5e6
        expected['streamID'] = 'new_stream_id'
        expected['xdelta'] = 1.0 / sample_rate
        sri = bulkio.sri.create('new_stream_id', sample_rate)
        self.assertSRIFieldsEqual(expected, sri)

    def testCompare(self):
        a_sri = bulkio.sri.create()
        b_sri = bulkio.sri.create()
        c_sri = bulkio.sri.create()
        c_sri.streamID = "THIS_DOES_NOT_MATCH"

        self.assertEqual( bulkio.sri.compare( a_sri, b_sri ), True, " bulkio.sri.compare method - same.")
        self.assertEqual( bulkio.sri.compare( a_sri, c_sri ), False, " bulkio.sri.compare method - different - StreamID .")

        c_sri = bulkio.sri.create()
        c_sri.hversion = 2
        self.assertEqual( bulkio.sri.compare( a_sri, c_sri ), False, " bulkio.sri.compare method - different - hversion ")

        c_sri = bulkio.sri.create()
        c_sri.xstart = 3
        self.assertEqual( bulkio.sri.compare( a_sri, c_sri ), False, " bulkio.sri.compare method - different - xstart ")

        c_sri = bulkio.sri.create()
        c_sri.xdelta = 100.0
        self.assertEqual( bulkio.sri.compare( a_sri, c_sri ), False, " bulkio.sri.compare method - different - xdelta ")

        c_sri = bulkio.sri.create()
        c_sri.xunits = 100.0
        self.assertEqual( bulkio.sri.compare( a_sri, c_sri ), False, " bulkio.sri.compare method - different - xunits ")

        c_sri = bulkio.sri.create()
        c_sri.subsize = 100
        self.assertEqual( bulkio.sri.compare( a_sri, c_sri ), False, " bulkio.sri.compare method - different - subsize ")

        c_sri = bulkio.sri.create()
        c_sri.ystart = 3
        self.assertEqual( bulkio.sri.compare( a_sri, c_sri ), False, " bulkio.sri.compare method - different - ystart ")

        c_sri = bulkio.sri.create()
        c_sri.ydelta = 100.0
        self.assertEqual( bulkio.sri.compare( a_sri, c_sri ), False, " bulkio.sri.compare method - different - ydelta ")

        c_sri = bulkio.sri.create()
        c_sri.yunits = 100.0
        self.assertEqual( bulkio.sri.compare( a_sri, c_sri ), False, " bulkio.sri.compare method - different - yunits ")

        c_sri = bulkio.sri.create()
        c_sri.mode = 100
        self.assertEqual( bulkio.sri.compare( a_sri, c_sri ), False, " bulkio.sri.compare method - different - mode ")

        kv = CF.DataType( id="key_one", value=to_any(1) )
        kv2 = CF.DataType( id="key_one", value=to_any(1) )
        a_sri.keywords = [kv]
        c_sri = bulkio.sri.create()
        c_sri.keywords = [kv2]
        self.assertEqual( bulkio.sri.compare( a_sri, c_sri ), True, " bulkio.sri.compare method - same - keyword item ")

        kv2 = CF.DataType( id="key_one", value=to_any(100) )
        c_sri = bulkio.sri.create()
        c_sri.keywords = [kv2]
        self.assertEqual( bulkio.sri.compare( a_sri, c_sri ), False, " bulkio.sri.compare method - different - keywords value mismatch ")

        kv2 = CF.DataType( id="key_two", value=to_any(100) )
        c_sri = bulkio.sri.create()
        c_sri.keywords = [kv2]
        self.assertEqual( bulkio.sri.compare( a_sri, c_sri ), False, " bulkio.sri.compare method - different - keywords name mismatch  ")

    def testHasKeyword(self):
        sri = bulkio.sri.create('has_keyword')
        sri.keywords.append(CF.DataType('string', to_any('first')))
        sri.keywords.append(CF.DataType('number', to_any(2.0)))
        
        self.failUnless(bulkio.sri.hasKeyword(sri, 'string'))
        self.failUnless(bulkio.sri.hasKeyword(sri, 'number'))
        self.failIf(bulkio.sri.hasKeyword(sri, 'missing'))

    def testGetKeyword(self):
        sri = bulkio.sri.create('get_keyword')
        sri.keywords.append(CF.DataType('string', to_any('first')))
        sri.keywords.append(CF.DataType('number', to_any(2.0)))

        # Basic get
        self.assertEqual('first', bulkio.sri.getKeyword(sri, 'string'))
        self.assertEqual(2.0, bulkio.sri.getKeyword(sri, 'number'))

        # Add a duplicate keyword at the end, should still return first value
        sri.keywords.append(CF.DataType('string', to_any('second')))
        self.assertEqual('first', bulkio.sri.getKeyword(sri, 'string'))

        self.assertRaises(KeyError, bulkio.sri.getKeyword, sri, 'missing')

    def testSetKeyword(self):
        sri = bulkio.sri.create('set_keyword')
        sri.keywords.append(CF.DataType('string', to_any('first')))
        sri.keywords.append(CF.DataType('number', to_any(2.0)))

        # Update first keyword
        bulkio.sri.setKeyword(sri, 'string', 'modified')
        self.assertEqual(2, len(sri.keywords))
        self.assertEqual('modified', bulkio.sri.getKeyword(sri, 'string'))
        self.assertEqual(2.0, bulkio.sri.getKeyword(sri, 'number'))

        # Update second keyword
        bulkio.sri.setKeyword(sri, 'number', -1)
        self.assertEqual(2, len(sri.keywords))
        self.assertEqual('modified', bulkio.sri.getKeyword(sri, 'string'))
        self.assertEqual(-1, bulkio.sri.getKeyword(sri, 'number'))

        # Add new keyword
        bulkio.sri.setKeyword(sri, 'new', True)
        self.assertEqual(3, len(sri.keywords))
        self.assertEqual('modified', bulkio.sri.getKeyword(sri, 'string'))
        self.assertEqual(-1, bulkio.sri.getKeyword(sri, 'number'))
        self.assertEqual(True, bulkio.sri.getKeyword(sri, 'new'))

    def testEraseKeyword(self):
        sri = bulkio.sri.create('erase_keyword')
        sri.keywords.append(CF.DataType('string', to_any('first')))
        sri.keywords.append(CF.DataType('number', to_any(2.0)))

        # Basic erase
        self.failUnless(bulkio.sri.hasKeyword(sri, 'string'))
        bulkio.sri.eraseKeyword(sri, 'string')
        self.assertEqual(1, len(sri.keywords))
        self.failIf(bulkio.sri.hasKeyword(sri, 'string'))
        self.failUnless(bulkio.sri.hasKeyword(sri, 'number'))

        # Non-existant key, no modification
        bulkio.sri.eraseKeyword(sri, 'missing')
        self.assertEqual(1, len(sri.keywords))
        self.failUnless(bulkio.sri.hasKeyword(sri, 'number'))

        # Add some more keywords, including a duplicate; erasing the duplicate
        # should only erase the first instance
        sri.keywords.append(CF.DataType('string', to_any('first')))
        sri.keywords.append(CF.DataType('number', to_any(500)))
        self.assertEqual(2.0, bulkio.sri.getKeyword(sri, 'number'))
        bulkio.sri.eraseKeyword(sri, 'number')
        self.assertEqual(2, len(sri.keywords))
        self.assertEqual(500, bulkio.sri.getKeyword(sri, 'number'))

if __name__ == '__main__':
    import runtests
    runtests.main()
