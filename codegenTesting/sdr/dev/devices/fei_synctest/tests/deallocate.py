#!/usr/bin/env python

import sys
import traceback
import time
import frontend
from ossie import properties
from omniORB import CORBA

print " deallocate: args ", sys.argv
id = int(sys.argv[1])
tuner_type = sys.argv[2]
dev_ior = sys.argv[3]

start_time=time.time()
stop_time=start_time
retval=1
try:
    step="Step0"
    orb=CORBA.ORB_init()
    print step, " Setup the ORB ..", orb

    step="Step1:"        
    device=orb.string_to_object(dev_ior)
    if not device:
        raise Exception( "Unable to locate FEI device under test, device ior " + str(dev_ior) + " type " + str(tuner_type))
    print step, " Located FEI device under test, device ior ", dev_ior , " type ", tuner_type

    step="Step2:"
    freq=100e6+(id*1e5)
    print step, " Create deallocation request, allocation id ", id, "tuner ", tuner_type, " freq ", freq
    a = frontend.createTunerAllocation(allocation_id=str(id),tuner_type=tuner_type, center_frequency=freq)

    step="Step3:"
    aprops=properties.props_from_dict(a)
    print step, " Calling deallocateCapacity for allocation id ", id, a
    device.deallocateCapacity(aprops)
    stop_time=time.time()
    print("Deallocation: {0} PASSED  duration: {1} ".format(id,
                                                          stop_time-start_time))

    retval=0
except Exception as e:
    traceback.print_exc()
    print("Create deallocation failed, allocation id {0} - {1}: {2}".format(id,
                                                                       step,
                                                                       3))

# setup return code         
sys.exit(retval)

