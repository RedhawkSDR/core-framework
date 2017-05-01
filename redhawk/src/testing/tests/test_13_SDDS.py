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
import os
import sys
import time
import calendar
import contextlib
import cStringIO
import binascii
import struct
import re

from ossie.utils.sdds import *

@contextlib.contextmanager
def stdout_redirect(where):
    sys.stdout = where
    try:
        yield where
    finally:
        sys.stdout = sys.__stdout__

class Test_SDDS_Time(unittest.TestCase):

    def setUp(self):
        self.tfile = os.tmpfile()
        self.cur_year = datetime.date.today().year
        self.cur_year_str = str(self.cur_year)

    def _run_test(self, fmt_key):
        conv=conversions[fmt_key]
        config.strConfig(logcfg+conv[0])
        self.logger=logging.getLogger('')
        self.console=self.logger.handlers[0]
        self.console.stream=self.tfile
        
        pval=time.strftime(conv[1])
        
        # 
        self.logger.info('test1')
        self.tfile.seek(0)
        logline=self.tfile.read()
        logline=logline.strip()
        if len(conv) > 2:
            logline = logline.split(conv[2])[0]
            pval = pval.split(conv[2])[0]
        self.assertEquals( pval, logline)


    def test_startofyear(self):
        sdds_soy = Time.startOfYear()
        
        # calculate start of year
        soy = datetime.datetime(*(time.strptime(self.cur_year_str+"-01-01 00:00:00",
                                         "%Y-%m-%d %H:%M:%S")[0:6]))
        soy_time=calendar.timegm(soy.timetuple())
        self.assertEqual( sdds_soy, soy_time )

        sdds_time = Time()
        sdds_time.setFromTime(soy_time)
        # calculate start of year
        self.assertEqual( sdds_time.gmtime(), time.gmtime(soy_time) )


    def test_init(self):
        tod=time.time()
        sdds_time = Time()
        sdds_time.setFromTime(tod)
        
        # test current time of day
        self.assertEqual( sdds_time.seconds(), tod )

        # test current time of day struct
        self.assertEqual( sdds_time.gmtime(), time.gmtime(tod))

        # set parts
        sdds_time.set( 1234, 5678 )
        self.assertEqual( sdds_time.picoTicks(), 1234 )
        self.assertEqual( sdds_time.picoTicksFractional(), 5678 )

        # set partial
        sdds_time.setFromPartial( 4, .001 )
        self.assertEqual( sdds_time.picoTicks(), (4000000000*4) + long(4000000000*0.001) )
        self.assertEqual( sdds_time.picoTicksFractional(), 0 )


    def test_add(self):
        soy = Time.startOfYear()

        sdds_time = Time()
        sdds_time.setFromTime(soy)

        # add 1 second
        sdds_time = sdds_time + 1
        sdds_time_str = str(sdds_time)
        
        # calculate start of year
        match_soy_str = self.cur_year_str+":01:01::00:00:01.000000"
        self.assertEqual( sdds_time_str, match_soy_str )

        # add 59 second
        sdds_time = sdds_time + 59
        sdds_time_str = str(sdds_time)
        
        # 
        match_soy_str = self.cur_year_str+":01:01::00:01:00.000000"
        self.assertEqual( sdds_time_str, match_soy_str )

        # add 2 minutes
        sdds_time = sdds_time + 120
        sdds_time_str = str(sdds_time)
        
        # 
        match_soy_str = self.cur_year_str+":01:01::00:03:00.000000"
        self.assertEqual( sdds_time_str, match_soy_str )

        # add 2 hours
        sdds_time = sdds_time + (60*60*2)
        sdds_time_str = str(sdds_time)
        
        # 
        match_soy_str = self.cur_year_str+":01:01::02:03:00.000000"
        self.assertEqual( sdds_time_str, match_soy_str )


    def test_subtract(self):
        soy = Time.startOfYear()

        sdds_time = Time()
        sdds_time.setFromTime(soy)

        # add 2 hours
        sdds_time = sdds_time + (60*60*2)
        sdds_time_str = str(sdds_time)
        
        # set match
        match_soy_str = self.cur_year_str+":01:01::02:00:00.000000"
        self.assertEqual( sdds_time_str, match_soy_str )

        # subtract 10 seconds
        sdds_time = sdds_time -10 
        sdds_time_str = str(sdds_time)
        
        # set match
        match_soy_str = self.cur_year_str+":01:01::01:59:50.000000"
        self.assertEqual( sdds_time_str, match_soy_str )

        # subtract 9 minutes
        sdds_time = sdds_time -(9*60)
        sdds_time_str = str(sdds_time)
        
        # set match
        match_soy_str = self.cur_year_str+":01:01::01:50:50.000000"
        self.assertEqual( sdds_time_str, match_soy_str )



class Test_SDDS_Packet(unittest.TestCase):

    def setUp(self):
        self.tfile = os.tmpfile()
        self.cur_year = datetime.date.today().year
        self.cur_year_str = str(self.cur_year)

    
    def test_format_identifier_api(self):
        
        # get default format identifer.. sf=1, sos=1, dm=1 bps=8 all others 0
        res=binascii.unhexlify('2340')
        formatid = format_identifier()
        self.assertEqual( formatid.asString(), res )

        # assign from values sf=1, sos=1, dm =4, bps=16
        formatid= format_identifier.from_buffer_copy('\x83\x80')
        self.assertEqual( formatid.sf, 1 )
        self.assertEqual( formatid.sos, 1 )
        self.assertEqual( formatid.dm, 4 )
        self.assertEqual( formatid.bps, 16 )

        # assign 32 bit value and get back
        formatid= format_identifier()
        formatid.set_bps(32)
        self.assertEqual( formatid.bps, 31 )
        res=formatid.get_bps()
        self.assertEqual( res, 32 )

        # assign data mode...
        formatid.set_dmode( [ 1,1,1 ] )
        self.assertEqual( formatid.dm, 7 )

        formatid.set_dmode( 5 )
        self.assertEqual( formatid.dm, 5 )

        
    def test_frame_sequence(self):

        # get default frame sequence == 0
        res=binascii.unhexlify('0000')
        fsn = frame_sequence()
        self.assertEqual( fsn.asString(), res )
        
        # test big endian format for number
        seq=256
        res=struct.pack("!H",seq)
        fsn.seq = seq
        self.assertEqual( fsn.asString(), res )

        # add one...
        fsn.inc()
        seq +=1
        res=struct.pack("!H",seq)
        self.assertEqual( fsn.asString(), res )

        # set for rollover
        seq =65535
        fsn.seq = seq
        res=struct.pack("!H",seq)
        self.assertEqual( fsn.asString(), res )

        # set for rollover
        fsn.inc()
        res=struct.pack("!H",0)
        self.assertEqual( fsn.asString(), res )

    def test_msptr_data(self):

        # get default frame sequence == 0
        msptr_=0
        msdelta_=0
        res=struct.pack("!HH",msptr_, msdelta_)
        msptr = msptr_data()
        self.assertEqual( msptr.asString(), res )
        
        # test big endian format for number
        msptr_=256
        msdelta_=256
        res=struct.pack("!HH",msptr_, msdelta_)
        msptr.msptr=msptr_
        msptr.msdelta=msdelta_
        self.assertEqual( msptr.asString(), res )

        # set max value
        msptr_=2047
        msdelta_=65535
        res=struct.pack("!HH",msptr_, msdelta_)
        msptr.msptr=msptr_
        msptr.msdelta=msdelta_
        self.assertEqual( msptr.asString(), res )

    def test_ttag_info_struct(self):

        # get default 
        res=binascii.unhexlify('00000000')
        ttag_info = ttag_info_struct()
        self.assertEqual( ttag_info.asString(), res )
        
        # test msv
        res=binascii.unhexlify('00800000')
        ttag_info.set_msv()
        self.assertEqual( ttag_info.asString(), res )

        res=binascii.unhexlify('00000000')
        ttag_info.set_msv(False)
        self.assertEqual( ttag_info.asString(), res )

        # test ttv
        res=binascii.unhexlify('00400000')
        ttag_info.set_ttv()
        self.assertEqual( ttag_info.asString(), res )

        res=binascii.unhexlify('00000000')
        ttag_info.set_ttv(False)
        self.assertEqual( ttag_info.asString(), res )

        # test sscv
        res=binascii.unhexlify('0000000')
        ttag_info.set_sscv()
        self.assertEqual( ttag_info.asString(), res )

        res=binascii.unhexlify('00000000')
        ttag_info.set_sscv(False)
        self.assertEqual( ttag_info.asString(), res )

        # test pi
        res=binascii.unhexlify('08000000')
        ttag_info.set_pi()
        self.assertEqual( ttag_info.asString(), res )

        res=binascii.unhexlify('00000000')
        ttag_info.set_pi(False)
        self.assertEqual( ttag_info.asString(), res )

        # test peo
        res=binascii.unhexlify('10000000')
        ttag_info.set_peo(True)
        self.assertEqual( ttag_info.asString(), res )

        res=binascii.unhexlify('00000000')
        ttag_info.set_peo(False)
        self.assertEqual( ttag_info.asString(), res )

    def test_ttag_values(self):
        ttag_=0
        ttage_=0
        res=struct.pack("!QI", ttag_, ttage_)
        ttag_val = ttag_values()
        self.assertEqual( ttag_val.asString(), res )
        
        # test big endian format for number
        ttag_= 4294967296
        ttage_= 8388608
        res=struct.pack("!QI", ttag_, ttage_)
        ttag_val.ttag=ttag_
        ttag_val.ttage=ttage_
        self.assertEqual( ttag_val.asString(), res )

    def test_ttag_info(self):
        msptr_=0
        msdelta_=0
        ttag_=0
        ttage_=0
        res=struct.pack("!HHQI", msptr_, msdelta_, ttag_, ttage_)
        ttag_val = ttag_info()
        self.assertEqual( ttag_val.asString(), res )
        
        # test big endian format for number
        ttag_= 4294967296
        ttage_= 8388608
        msptr_=256
        msdelta_=256
        res=struct.pack("!HHQI", msptr_, msdelta_, ttag_, ttage_)
        ttag_val.info.msptr.msptr=msptr_
        ttag_val.info.msptr.msdelta=msdelta_
        ttag_val.tstamp.ttag=ttag_
        ttag_val.tstamp.ttage=ttage_
        self.assertEqual( ttag_val.asString(), res )

        ttag_val = ttag_info()
        ttag_= 4294967296
        ttage_= 8388608
        msptr_=2047
        msdelta_=256
        ttag_val.tstamp.ttag=ttag_
        ttag_val.tstamp.ttage=ttage_
        ttag_val.info.msptr.msptr=msptr_
        ttag_val.info.msptr.msdelta=msdelta_
        ttag_val.info.info.set_msv()
        ttag_val.info.info.set_ttv()
        ttag_val.info.info.set_sscv()
        res=struct.pack("!QI", ttag_, ttage_)
        tinfo=binascii.unhexlify('07370100')
        res=tinfo+res
        self.assertEqual( ttag_val.asString(), res )

