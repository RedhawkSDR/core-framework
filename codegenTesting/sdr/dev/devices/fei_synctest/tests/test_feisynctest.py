#!/usr/bin/env python


import sys
import ossie.utils.testing
from ossie.utils import sb
import itertools
import traceback
import time

def allocate(id=1, tuner_type='RX_DIGITIZER', dev_ior=None, verbose=False ):
    import commands
    import os
    fpath=os.path.dirname(os.path.realpath(__file__))
    if verbose:
        print "launching allocation ", id, tuner_type, dev_ior
    cmd=os.path.join(fpath,"allocate.py") + " " + " ".join([ str(id), tuner_type, dev_ior ] )
    if verbose:
        print " issue cmd ", cmd
    (status, output) = commands.getstatusoutput(cmd)
    if verbose:
        print "completed allocation ", id, tuner_type, " return values ", status, output
    return status

def deallocate(id=1, tuner_type='RX_DIGITIZER', dev_ior=None, verbose=False ):
    import commands
    import os
    fpath=os.path.dirname(os.path.realpath(__file__))
    if verbose:
        print "launching deallocation ", id, tuner_type, dev_ior
    cmd=os.path.join(fpath,"deallocate.py") + " " + " ".join([ str(id), tuner_type, dev_ior ] )
    if verbose:
        print " issue cmd ", cmd    
    (status, output) = commands.getstatusoutput(cmd)
    if verbose:
        print "completed deallocation  ", id, tuner_type, " return values ", status, output
    return status


class SyncTest(ossie.utils.testing.RHTestCase):
    SPD_FILE = '../fei_synctest.spd.xml'

    def setUp(self):
        self.ntuners=20
        self.verbose=False
        # Launch the device, using the selected implementation
        self.comp = sb.launch(self.spd_file,
                              properties={'DEBUG_LEVEL': 3, 'tuners' : self.ntuners },
                              impl=self.impl)
    
    def tearDown(self):
        # Clean up all sandbox artifacts created during test
        sb.release()

    def testBasicBehavior(self):
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        try:
            import concurrent.futures
        except:
            return
        
        self.comp.start()
        
        n_execs=None
        n_allocs=self.ntuners
        start_time=time.time()
        allocations=range(1,n_allocs+1)

        alloc_start_time=time.time()
        dev_ior=sb.orb.object_to_string(self.comp.ref)
        retvals=None
        if self.verbose:
            print("Starting up tests, concurrent workers: {0} ".format(n_execs))
        with concurrent.futures.ProcessPoolExecutor(max_workers=n_execs) as e1:
            retval=e1.map(allocate,
                          allocations,
                          itertools.repeat('RX_DIGITIZER'),
                          itertools.repeat(dev_ior),
                          itertools.repeat(self.verbose) )
            e1.shutdown(wait=True)

        if self.verbose:
            print("Total Allocate Elapsed Time: {0} seconds".format(time.time()-alloc_start_time))

        # check return values
        for i, rv in zip( allocations, retval):
            self.assertEqual(rv,0, "Allocation failed for request: "+ str(i))


        # check fronend tuner status for proper allocation id
        def checkEqual(L1, L2):
            return len(L1) == len(L2) and sorted(L1) == sorted(L2)
        alloc_ids=[ x["FRONTEND::tuner_status::allocation_id_csv"] for x  in self.comp.frontend_tuner_status ]
        alloc_ids.sort()
        expected_ids=[ str(i) for i in allocations ]
        if self.verbose:
            print " alloc id ", alloc_ids
            print " expected id ", expected_ids
        self.assertEqual(checkEqual(alloc_ids, expected_ids),
                         True,
                         "Allocation Ids for allocate operation did not match: expected " + str(expected_ids) + " actual " + str(alloc_ids))
                         

        dealloc_start_time=time.time()
        retvals=None
        print("Starting up tests, concurrent workers: {0} ".format(n_execs))
        with concurrent.futures.ProcessPoolExecutor(max_workers=n_execs) as e1:
            retval=e1.map(deallocate,
                          allocations,
                          itertools.repeat('RX_DIGITIZER'),
                          itertools.repeat(dev_ior),
                          itertools.repeat(self.verbose) )            
            e1.shutdown(wait=True)

        if self.verbose:
            print("Total Deallocate Elapsed Time: {0} seconds".format(time.time()-dealloc_start_time))

        # check return values
        for i, rv in zip( allocations, retval):
            self.assertEqual(rv,0, "Allocation failed for request: "+ str(i))

        # check fronend tuner status for empty allocation id
        def checkEqual(L1, L2):
            return len(L1) == len(L2) and sorted(L1) == sorted(L2)
        alloc_ids=[ x["FRONTEND::tuner_status::allocation_id_csv"] for x  in self.comp.frontend_tuner_status ]
        alloc_ids.sort()
        expected_ids=[ '' ] * len(allocations)
        self.assertEqual(checkEqual(alloc_ids, expected_ids),
                         True,  
                         "Allocation Ids for deallocate operatin did not match: expected " + str(expected_ids) + " actual " + str(alloc_ids))        

        self.comp.stop()

if __name__ == "__main__":
    ossie.utils.testing.main() # By default tests all implementations
