#!/usr/bin/python

import unittest
from omniORB import CORBA

from ossie.utils.log4py import logging

from test_outports import *
from test_inports import *
from test_instreams import *
from test_outstreams import *
from test_utctime import *
from test_streamsri import *

if __name__ == '__main__':
    logging.basicConfig()
    #logging.getLogger().setLevel(logging.TRACE)

    orb = CORBA.ORB_init()
    root_poa = orb.resolve_initial_references("RootPOA")
    manager = root_poa._get_the_POAManager()
    manager.activate()

    unittest.main()

    orb.shutdown(True)
