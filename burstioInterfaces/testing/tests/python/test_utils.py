#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK burstioInterfaces.
#
# REDHAWK burstioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK burstioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
import random
import unittest
import sys
import time
import types
from   ossie.utils import sb
from   bulkio.bulkioInterfaces import *

# Add the local search paths to find local IDL files
from ossie.utils import model
from ossie.utils.idllib import IDLLibrary
model._idllib = IDLLibrary()
model._idllib.addSearchPath('../../../idl')
model._idllib.addSearchPath('/usr/local/redhawk/core/share/idl')

# add local build path to test out api, issue with bulkio.<library> and bulkio.bulkioInterfaces... __init__.py
# differs during build process
sys.path = [ '../../build/lib' ] + sys.path

from redhawk.burstio import *

def str_to_class(s):
    if s in globals() and isinstance(globals()[s], types.ClassType):
        return globals()[s]
    return None

class BurstioUtils_Tests(unittest.TestCase):
    def __init__(self, methodName='runTest'):
        unittest.TestCase.__init__(self, methodName)

    def setUp(self):
        self.seq = range(10)

    def test_sri_create(self):
        sri = utils.createSRI("test_stream_id")
        self.assertEqual( sri.hversion, 1, "Version Incompatable" )
        self.assertEqual( "test_stream_id", sri.streamID, "Stream ID mismatch.")
        self.assertEqual( 1, sri.hversion , "Version mismatch." )
        self.assertEqual( sri.xdelta, 1.000, "XDelta mismatch.")
        self.assertEqual(sri.mode, 0, "Mode mismatch.")
        self.assertEqual(sri.flags, 0, "Flags mismatch.")
        self.assertEqual(sri.tau, 0.00, "Tau mismatch.")
        self.assertEqual(sri.theta, 0.00, "Theta mismatch.")
        self.assertEqual( sri.uwlength, 0, "UW Length mismatch.")
        self.assertEqual( sri.bursttype, 0, "Burst Type mismatch.")
        self.assertEqual( sri.burstLength, 0, "Burst Length mismatch.")
        self.assertEqual( sri.CHAN_RF, 0.00, "CHAN_RF mismatch.")
        self.assertEqual( sri.baudestimate, 0.00, "Baud Estimate mismatch.")
        self.assertEqual( sri.baudrate, 0.00, "Baud Rate mismatch.")
        self.assertEqual( sri.carrieroffset, 0.00, "Carrier Offset mismatch.")
        self.assertEqual( sri.SNR, 0.00, "SNR mismatch.")
        self.assertEqual( sri.modulation,"", "Modulation mismatch.")
        self.assertEqual( sri.fec,"", "FEC mismatch.")
        self.assertEqual(  sri.fecrate, "", "FEC Rate mismatch.")
        self.assertEqual(  sri.randomizer, "",  "Randomizer mismatch.")
        self.assertEqual(  sri.overhead, "", "Overhead mismatch.")
        self.assertEqual( len(sri.keywords), 0, "Keywords Length mismatch.")


    def test_time_now(self):
        ts = utils.now();
        self.assertEqual( ts.tcmode, BULKIO.TCM_CPU, " tcmode mismatch." )
        self.assertEqual( ts.tcstatus, BULKIO.TCS_VALID , " tcstatus mismatch.")


    def test_time_keywords(self):
        kwl = []

        sri = utils.createSRI("defaultSRI");
        utils.addKeyword( kwl, "test-kw-one", 22.5 );

        self.assertEqual(len(kwl),1,"Keyword list length mismatch.")

        utils.addKeyword( kwl, "test-kw-two", 115.5 )
        self.assertEqual(len(kwl),2,"Keyword list length mismatch.")

    def test_time_elapsed(self):
        b = utils.now();
        e = b

        elapsed = utils.elapsed( b, e )
        self.assertEqual( elapsed, 0.00, "Elapse same begin/end mismatch." )

        e=utils.now()
        b.twsec = 100
        b.tfsec = 0
        e.twsec = 200
        e.tfsec = 0
        elapsed = utils.elapsed( b, e )
        self.assertEqual( elapsed, 100.00, "Elapse calcsame begin/end mismatch." )

        e=utils.now()
        b.twsec = 100
        b.tfsec = 50
        e.twsec = 200
        e.tfsec = 100
        elapsed = utils.elapsed( b, e )
        self.assertEqual( elapsed, 150.00, "Elapse same begin/end mismatch." )


if __name__ == '__main__':
    if len(sys.argv) < 2 :
        unittest.main()
    else:
        suite = unittest.TestLoader().loadTestsFromTestCase(globals()[sys.argv[1]] )
        try:
            import xmlrunner
            runner = xmlrunner.XMLTestRunner(verbosity=2)
        except ImportError:
            runner = unittest.TextTestRunner(verbosity=2)
        runner.run(suite)
