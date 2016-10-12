#!/usr/bin/env python
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK frontendInterfaces.
#
# REDHAWK frontendInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK frontendInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
#
# AUTO-GENERATED CODE.  DO NOT MODIFY!
#
# Source: testPythonFeDevice.spd.xml
from ossie.cf import CF, CF__POA
from ossie.utils import uuid
from omniORB import any, CORBA

from ossie.device import Device
from ossie.properties import simple_property
from ossie.properties import struct_property
from ossie.properties import structseq_property
from ossie.utils import model

#import Queue, copy, time
import threading
#from ossie.resource import usesport, providesport
#import bulkio
from bulkio.bulkioInterfaces import BULKIO
from redhawk.frontendInterfaces import FRONTEND
#from redhawk.frontendInterfaces import FRONTEND__POA

def zeroSRI(sri):
    if sri:
        sri.hversion = 1
        sri.xstart = 0.0
        sri.xdelta = 1.0
        sri.xunits = 1
        sri.subsize = 1
        sri.ystart = 0.0
        sri.ydelta = 1.0
        sri.yunits = 1
        sri.mode = 0
        sri.streamID = ""
        sri.blocking = False
        sri.keywords = []
    else:
        sri = BULKIO.StreamSRI(hversion=1, xstart=0.0, xdelta=1.0, 
                              xunits=1, subsize=1, ystart=0.0, ydelta=1.0, 
                              yunits=1, mode=0, streamID="", blocking=False, keywords=[])
        
        
# Time Type Definition
J1970 = 1
JCY = 2
#timeTypes = {'J1970':J1970,'JCY':JCY}
    
''' Individual Tuner. This structure contains stream specific data for channel/tuner to include:
         - Additional stream metadata (sri)
         - Control information (allocation id's)
         - Reference to associated frontend_tuner_status property where additional information is held. Note: frontend_tuner_status structure is required by frontend interfaces v2.0
'''
class indivTuner:
    def __init__(self):
        self.frontend_status = None
        self.lock = threading.Lock()
        self.sri = None
        self.complex = None
        self.control_allocation_id = None
        

    def reset(self):
        zeroSRI(self.sri)
        self.control_allocation_id = None
        if frontend_status:
            frontend_status.allocation_id_csv = ""
            frontend_status.center_frequency = 0.0
            frontend_status.bandwidth = 0.0
            frontend_status.sample_rate = 0.0
            frontend_status.enabled = False
            

def createTunerAllocation(tuner_type='DDC',allocation_id=None,center_frequency=0.0,bandwidth=0.0,sample_rate=1.0,
                 device_control=True,group_id='',rf_flow_id='',bandwidth_tolerance=0.0,sample_rate_tolerance=0.0,returnDict=True):
    if returnDict:
        retval = {'FRONTEND::tuner_allocation':{'FRONTEND::tuner_allocation::tuner_type':tuner_type,'FRONTEND::tuner_allocation::allocation_id':allocation_id,
        'FRONTEND::tuner_allocation::center_frequency':center_frequency,'FRONTEND::tuner_allocation::bandwidth':bandwidth,
        'FRONTEND::tuner_allocation::sample_rate':sample_rate,'FRONTEND::tuner_allocation::device_control':device_control,
        'FRONTEND::tuner_allocation::group_id':group_id,'FRONTEND::tuner_allocation::rf_flow_id':rf_flow_id,
        'FRONTEND::tuner_allocation::bandwidth_tolerance':bandwidth_tolerance,'FRONTEND::tuner_allocation::sample_rate_tolerance':sample_rate_tolerance}}
        if allocation_id == None:
            retval['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::allocation_id']=model._uuidgen()
    else:
        alloc=[]
        alloc.append(CF.DataType(id='FRONTEND::tuner_allocation::tuner_type',value=any.to_any(tuner_type)))
        if allocation_id == None:
            alloc.append(CF.DataType(id='FRONTEND::tuner_allocation::allocation_id',value=any.to_any(model._uuidgen())))
        else:
            alloc.append(CF.DataType(id='FRONTEND::tuner_allocation::allocation_id',value=any.to_any(allocation_id)))
        alloc.append(CF.DataType(id='FRONTEND::tuner_allocation::center_frequency',value=any.to_any(center_frequency)))
        alloc[-1].value._t = CORBA.TC_double
        alloc.append(CF.DataType(id='FRONTEND::tuner_allocation::bandwidth',value=any.to_any(bandwidth)))
        alloc[-1].value._t = CORBA.TC_double
        alloc.append(CF.DataType(id='FRONTEND::tuner_allocation::sample_rate',value=any.to_any(sample_rate)))
        alloc[-1].value._t = CORBA.TC_double
        alloc.append(CF.DataType(id='FRONTEND::tuner_allocation::device_control',value=any.to_any(device_control)))
        alloc.append(CF.DataType(id='FRONTEND::tuner_allocation::group_id',value=any.to_any(group_id)))
        alloc.append(CF.DataType(id='FRONTEND::tuner_allocation::rf_flow_id',value=any.to_any(rf_flow_id)))
        alloc.append(CF.DataType(id='FRONTEND::tuner_allocation::bandwidth_tolerance',value=any.to_any(bandwidth_tolerance)))
        alloc[-1].value._t = CORBA.TC_double
        alloc.append(CF.DataType(id='FRONTEND::tuner_allocation::sample_rate_tolerance',value=any.to_any(sample_rate_tolerance)))
        alloc[-1].value._t = CORBA.TC_double
        retval = CF.DataType(id='FRONTEND::tuner_allocation',value=CORBA.Any(CF._tc_Properties,alloc))
    return retval

def createTunerGenericListenerAllocation(tuner_type='DDC',allocation_id=None,center_frequency=0.0,bandwidth=0.0,sample_rate=1.0,
                 device_control=False,group_id='',rf_flow_id='',bandwidth_tolerance=0.0,sample_rate_tolerance=0.0,returnDict=True):
    if returnDict:
        retval = {'FRONTEND::tuner_allocation':{'FRONTEND::tuner_allocation::tuner_type':tuner_type,'FRONTEND::tuner_allocation::allocation_id':allocation_id,
        'FRONTEND::tuner_allocation::center_frequency':center_frequency,'FRONTEND::tuner_allocation::bandwidth':bandwidth,
        'FRONTEND::tuner_allocation::sample_rate':sample_rate,'FRONTEND::tuner_allocation::device_control':device_control,
        'FRONTEND::tuner_allocation::group_id':group_id,'FRONTEND::tuner_allocation::rf_flow_id':rf_flow_id,
        'FRONTEND::tuner_allocation::bandwidth_tolerance':bandwidth_tolerance,'FRONTEND::tuner_allocation::sample_rate_tolerance':sample_rate_tolerance}}
        if allocation_id == None:
            retval['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::allocation_id']=model._uuidgen()
    else:
        alloc=[]
        alloc.append(CF.DataType(id='FRONTEND::tuner_allocation::tuner_type',value=any.to_any(tuner_type)))
        if allocation_id == None:
            alloc.append(CF.DataType(id='FRONTEND::tuner_allocation::allocation_id',value=any.to_any(model._uuidgen())))
        else:
            alloc.append(CF.DataType(id='FRONTEND::tuner_allocation::allocation_id',value=any.to_any(allocation_id)))
        alloc.append(CF.DataType(id='FRONTEND::tuner_allocation::center_frequency',value=any.to_any(center_frequency)))
        alloc[-1].value._t = CORBA.TC_double
        alloc.append(CF.DataType(id='FRONTEND::tuner_allocation::bandwidth',value=any.to_any(bandwidth)))
        alloc[-1].value._t = CORBA.TC_double
        alloc.append(CF.DataType(id='FRONTEND::tuner_allocation::sample_rate',value=any.to_any(sample_rate)))
        alloc[-1].value._t = CORBA.TC_double
        alloc.append(CF.DataType(id='FRONTEND::tuner_allocation::device_control',value=any.to_any(device_control)))
        alloc.append(CF.DataType(id='FRONTEND::tuner_allocation::group_id',value=any.to_any(group_id)))
        alloc.append(CF.DataType(id='FRONTEND::tuner_allocation::rf_flow_id',value=any.to_any(rf_flow_id)))
        alloc.append(CF.DataType(id='FRONTEND::tuner_allocation::bandwidth_tolerance',value=any.to_any(bandwidth_tolerance)))
        alloc[-1].value._t = CORBA.TC_double
        alloc.append(CF.DataType(id='FRONTEND::tuner_allocation::sample_rate_tolerance',value=any.to_any(sample_rate_tolerance)))
        alloc[-1].value._t = CORBA.TC_double
        retval = CF.DataType(id='FRONTEND::tuner_allocation',value=CORBA.Any(CF._tc_Properties,alloc))
    return retval

def createTunerListenerAllocation(existing_allocation_id,listener_allocation_id=None,returnDict=True):
    if returnDict:
        retval = {'FRONTEND::listener_allocation':{'FRONTEND::listener_allocation::existing_allocation_id':existing_allocation_id,
        'FRONTEND::listener_allocation::listener_allocation_id':listener_allocation_id}}
        if listener_allocation_id == None:
            retval['FRONTEND::listener_allocation']['FRONTEND::listener_allocation::listener_allocation_id']=model._uuidgen()
    else:
        alloc=[]
        alloc.append(CF.DataType(id='FRONTEND::listener_allocation::existing_allocation_id',value=any.to_any(existing_allocation_id)))
        if listener_allocation_id == None:
            alloc.append(CF.DataType(id='FRONTEND::listener_allocation::listener_allocation_id',value=any.to_any(model._uuidgen())))
        else:
            alloc.append(CF.DataType(id='FRONTEND::listener_allocation::listener_allocation_id',value=any.to_any(listener_allocation_id)))
        retval = CF.DataType(id='FRONTEND::listener_allocation',value=CORBA.Any(CF._tc_Properties,alloc))
    return retval

def tune(device,tuner_type='RX_DIGITIZER',allocation_id=None,center_frequency=None,bandwidth=256000,sample_rate=None,device_control=True,group_id='',rf_flow_id='',bandwidth_tolerance=0.0,sample_rate_tolerance=0.0,returnDict=True,gain=None):
    tuners = len(device.frontend_tuner_status)
    newAllocation = False
    #No tuners found on device
    if tuners == 0:
        print "No Available Tuner"
    else:
        if tuners >= 1:
            for index, key in enumerate(device.frontend_tuner_status):
                id = device.frontend_tuner_status[index].allocation_id_csv
                if id == allocation_id:
                    break
                if id == '' and not newAllocation:
                    if sample_rate == None or center_frequency == None:
                        print "set center_frequency and sample_rate"
                    alloc=createTunerAllocation(tuner_type, allocation_id,center_frequency,bandwidth, sample_rate,device_control,group_id,rf_flow_id,bandwidth_tolerance,sample_rate_tolerance,returnDict)
                    alloc_results = device.allocateCapacity(alloc)
                    print alloc_results
                    newAllocation = True

        if allocation_id == None and not newAllocation and tuners > 1:
            print "All ", len(device.frontend_tune_status), " tuners have been allocated"
            print "Specify an allocation_id to change tuning properties"

        if not newAllocation:
            tuner=None
            tuner_type=None
            allocation_status = _getAllocationStatus(device, tuners, allocation_id) 
            if allocation_status == None:
                print "no matching allocation ID's", allocation_id
                return  allocation_status
            elif "DigitalTuner_in" in device._providesPortDict.keys():
                tuner_type = "DigitalTuner"
                tuner = device.getPort("DigitalTuner_in")
            elif "AnalogTuner_in" in device._providesPortDict.keys():
                tuner_type = "AnalogTuner"
                tuner = device.getPort("AnalogTuner_in")
            else:
                print "No DigitalTuner_in or AnalogTuner_in found"
                return allocation_status.allocation_id_csv
            allocation_id = allocation_status.allocation_id_csv
            allocation_id = ''.join(allocation_id)
            if center_frequency != None:
                tuner.setTunerCenterFrequency(allocation_id, center_frequency)
            if sample_rate != None:
                tuner.setTunerOutputSampleRate(allocation_id, sample_rate)
            if gain != None:
                tuner.setTunerGain(allocation_id, gain)
            return allocation_status.allocation_id_csv
    
    return None

def _getAllocationStatus(device, tuners, allocation_id):
    if tuners == 1 and allocation_id == None:
        return device.frontend_tuner_status[0]
    else:
        for i in range(len(device.frontend_tuner_status)):
            if device.frontend_tuner_status[i].allocation_id == allocation_id:
                return device.frontend_tuner_status[i]
    return None

def deallocate(device,allocation_id=None):
    deallocated = False
    if len(device.frontend_tuner_status) == 1:
        device.deallocateCapacity(device._alloc[0])
        deallocated = True
    else:
        if allocation_id == None:
            print "no allocation_id given"
        for i in range(len(device._alloc)):
            if device._alloc[i].values()[0]['FRONTEND::tuner_allocation::allocation_id'] == allocation_id:
                device.deallocateCapacity(device._alloc[i])
                deallocated = True
    return deallocated

class FrontendTunerAllocation(object):
    tuner_type = simple_property(id_="FRONTEND::tuner_allocation::tuner_type",
                                     name="tuner_type",
                                     type_="string",
                                     )
    allocation_id = simple_property(id_="FRONTEND::tuner_allocation::allocation_id",
                                    name="allocation_id",
                                    type_="string",
                                    )
    center_frequency = simple_property(id_="FRONTEND::tuner_allocation::center_frequency",
                                       name="center_frequency",
                                       type_="double",
                                       )
    bandwidth = simple_property(id_="FRONTEND::tuner_allocation::bandwidth",
                                name="bandwidth",
                                type_="double",
                                )
    bandwidth_tolerance = simple_property(id_="FRONTEND::tuner_allocation::bandwidth_tolerance",
                                          name="bandwidth_tolerance",
                                          type_="double",
                                          defvalue=10.0,
                                          )
    sample_rate = simple_property(id_="FRONTEND::tuner_allocation::sample_rate",
                                  name="sample_rate",
                                  type_="double",
                                  )
    sample_rate_tolerance = simple_property(id_="FRONTEND::tuner_allocation::sample_rate_tolerance",
                                           name="sample_rate_tolerance",
                                            type_="double",
                                            defvalue=10.0,
                                            )
    device_control = simple_property(id_="FRONTEND::tuner_allocation::device_control",
                                     name="device_control",
                                     type_="boolean",
                                     defvalue=True,
                                     )
    group_id = simple_property(id_="FRONTEND::tuner_allocation::group_id",
                               name="group_id",
                               type_="string",
                               )
    rf_flow_id = simple_property(id_="FRONTEND::tuner_allocation::rf_flow_id",
                                 name="rf_flow_id",
                                 type_="string",
                                 )
    
    def __init__(self, **kw):
        """Construct an initialized instance of this struct definition"""
        for attrname, classattr in type(self).__dict__.items():
            if type(classattr) == simple_property:
                classattr.initialize(self)
        for k,v in kw.items():
            setattr(self,k,v)
    
    def __str__(self):
        """Return a string representation of this structure"""
        d = {}
        d["tuner_type"] = self.tuner_type
        d["allocation_id"] = self.allocation_id
        d["center_frequency"] = self.center_frequency
        d["bandwidth"] = self.bandwidth
        d["bandwidth_tolerance"] = self.bandwidth_tolerance
        d["sample_rate"] = self.sample_rate
        d["sample_rate_tolerance"] = self.sample_rate_tolerance
        d["device_control"] = self.device_control
        d["group_id"] = self.group_id
        d["rf_flow_id"] = self.rf_flow_id
        return str(d)
    
    def getId(self):
        return "FRONTEND::tuner_allocation"
    
    def isStruct(self):
        return True
    
    def getMembers(self):
        return [("tuner_type",self.tuner_type),("allocation_id",self.allocation_id),("center_frequency",self.center_frequency),("bandwidth",self.bandwidth),("bandwidth_tolerance",self.bandwidth_tolerance),("sample_rate",self.sample_rate),("sample_rate_tolerance",self.sample_rate_tolerance),("device_control",self.device_control),("group_id",self.group_id),("rf_flow_id",self.rf_flow_id)]

class FrontendListenerAllocation(object):
    existing_allocation_id = simple_property(id_="FRONTEND::listener_allocation::existing_allocation_id",
                                             name="existing_allocation_id",
                                             type_="string",
                                             )
    listener_allocation_id = simple_property(id_="FRONTEND::listener_allocation::listener_allocation_id",
                                             name="listener_allocation_id",
                                             type_="string",
                                             )
    
    def __init__(self, **kw):
        """Construct an initialized instance of this struct definition"""
        for attrname, classattr in type(self).__dict__.items():
            if type(classattr) == simple_property:
                classattr.initialize(self)
        for k,v in kw.items():
            setattr(self,k,v)
    
    def __str__(self):
        """Return a string representation of this structure"""
        d = {}
        d["existing_allocation_id"] = self.existing_allocation_id
        d["listener_allocation_id"] = self.listener_allocation_id
        return str(d)
    
    def getId(self):
        return "FRONTEND::listener_allocation"
    
    def isStruct(self):
        return True
    
    def getMembers(self):
        return [("existing_allocation_id",self.existing_allocation_id),("listener_allocation_id",self.listener_allocation_id)]

class DefaultFrontendTunerStatusStruct(object):
    tuner_type = simple_property(id_="FRONTEND::tuner_status::tuner_type",
                                 name="tuner_type",
                                 type_="string",
                                 )
    allocation_id_csv = simple_property(id_="FRONTEND::tuner_status::allocation_id_csv",
                                        name="allocation_id_csv",
                                        type_="string",
                                        )
    center_frequency = simple_property(id_="FRONTEND::tuner_status::center_frequency",
                                       name="center_frequency",
                                       type_="double",
                                       )
    bandwidth = simple_property(id_="FRONTEND::tuner_status::bandwidth",
                                name="bandwidth",
                                type_="double",
                                )
    sample_rate = simple_property(id_="FRONTEND::tuner_status::sample_rate",
                                  name="sample_rate",
                                  type_="double",
                                  )
    group_id = simple_property(id_="FRONTEND::tuner_status::group_id",
                               name="group_id",
                               type_="string",
                               )
    rf_flow_id = simple_property(id_="FRONTEND::tuner_status::rf_flow_id",
                                 name="rf_flow_id",
                                 type_="string",
                                 )
    enabled = simple_property(id_="FRONTEND::tuner_status::enabled",
                              name="enabled",
                              type_="boolean",
                              )
    
    def __init__(self, tuner_type="", allocation_id_csv="", center_frequency=0.0, bandwidth=0.0, sample_rate=0.0, group_id="", rf_flow_id="", enabled=False):
        self.tuner_type = tuner_type
        self.allocation_id_csv = allocation_id_csv
        self.center_frequency = center_frequency
        self.bandwidth = bandwidth
        self.sample_rate = sample_rate
        self.group_id = group_id
        self.rf_flow_id = rf_flow_id
        self.enabled = enabled
    
    def __str__(self):
        """Return a string representation of this structure"""
        d = {}
        d["tuner_type"] = self.tuner_type
        d["allocation_id_csv"] = self.allocation_id_csv
        d["center_frequency"] = self.center_frequency
        d["bandwidth"] = self.bandwidth
        d["sample_rate"] = self.sample_rate
        d["group_id"] = self.group_id
        d["rf_flow_id"] = self.rf_flow_id
        d["enabled"] = self.enabled
        return str(d)
    
    def getId(self):
        return "frontend_tuner_status_struct"
    
    def isStruct(self):
        return True
    
    def getMembers(self):
        return [("tuner_type",self.tuner_type),("allocation_id_csv",self.allocation_id_csv),("center_frequency",self.center_frequency),("bandwidth",self.bandwidth),("sample_rate",self.sample_rate),("group_id",self.group_id),("rf_flow_id",self.rf_flow_id),("enabled",self.enabled)]


class FrontendTunerDevice(CF__POA.Device, Device):

    def __init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams):
        Device.__init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams)
        
        # self.tunerChannels is exclusively paired with property tuner_status.
        # self.tunerChannels provide stream information for the channel while tuner_status provides the tuner information.
        self.tunerChannels = [] # this will be populated with indivTuner() objects, one for each channel
    
        # Provides mapping from unique allocation ID to internal tuner (channel) number
        self.allocationID_to_tunerID = {}
        self.streamID_to_tunerID ={}
        self.allocationID_MappingLock = threading.Lock()
    
    """ Allocation handlers """
    def allocate_frontend_tuner_allocation(self, frontend_tuner_allocation):
        try:
            if not self._valid_tuner_type(frontend_tuner_allocation.tuner_type):
                #TODO: add back log messages
                print "allocate_frontend_tuner_allocation: UNKNOWN FRONTEND TUNER TYPE"
                raise CF.Device.InvalidCapacity("UNKNOWN FRONTEND TUNER TYPE", frontend_tuner_allocation)

            # Check allocation_id
            if not frontend_tuner_allocation.allocation_id:
                #TODO: add back log messages
                print "allocate_frontend_tuner_allocation: MISSING ALLOCATION_ID"
                raise CF.Device.InvalidCapacity("MISSING ALLOCATION_ID", frontend_tuner_allocation)
            
            # Check if allocation ID has already been used
            if  self.getTunerMapping(frontend_tuner_allocation.allocation_id) >= 0:
                #TODO: add back log messages
                print "allocate_frontend_tuner_allocation: ALLOCATION_ID ALREADY IN USE"
                raise CF.Device.InvalidCapacity("ALLOCATION_ID ALREADY IN USE", frontend_tuner_allocation)

            # Check if available tuner (if not requesting device control, this is all that's needed to add listener)
            tuner_id = self.addTunerMapping(frontend_tuner_allocation)
            if tuner_id < 0:
                #TODO: add back log messages
                print "allocate_frontend_tuner_allocation: NO AVAILABLE TUNER"
                raise RuntimeError("NO AVAILABLE TUNER")

            # Initialize the tuner (only if requesting device control)
            if frontend_tuner_allocation.device_control:
                self.tunerChannels[tuner_id].lock.acquire()
                try:
                    if frontend_tuner_allocation.group_id and frontend_tuner_allocation.group_id != self.tunerChannels[tuner_id].frontend_status.group_id:
                        #TODO: add back log messages
                        print "allocate_frontend_tuner_allocation: CANNOT ALLOCATE A TUNER WITH THAT GROUP ID"
                        raise FRONTEND.BadParameterException("CAN NOT ALLOCATE A TUNER WITH THAT GROUP ID!")

                    if frontend_tuner_allocation.rf_flow_id and frontend_tuner_allocation.rf_flow_id != self.tunerChannels[tuner_id].frontend_status.rf_flow_id:
                        #TODO: add back log messages
                        print "allocate_frontend_tuner_allocation: CANNOT ALLOCATE A TUNER WITH THAT RF FLOW ID"
                        raise FRONTEND.BadParameterException("CAN NOT ALLOCATE A TUNER WITH RF FLOW ID = %s!"%(frontend_tuner_allocation.rf_flow_id))

                    #Check Validity
                    if not self._valid_center_frequency(frontend_tuner_allocation.center_frequency,tuner_id):
                        #TODO: add back log messages
                        print "allocate_frontend_tuner_allocation: INVALID FREQUENCY"
                        raise FRONTEND.BadParameterException("allocateCapacity(): INVALID FREQUENCY")
                    
                    if not self._valid_bandwidth(frontend_tuner_allocation.bandwidth,tuner_id):
                        #TODO: add back log messages
                        print "allocate_frontend_tuner_allocation: INVALID BANDWIDTH"
                        raise FRONTEND.BadParameterException("allocateCapacity(): INVALID BANDWIDTH")
                    
                    if not self._valid_sample_rate(frontend_tuner_allocation.sample_rate,tuner_id):
                        #TODO: add back log messages
                        print "allocate_frontend_tuner_allocation: INVALID RATE"
                        raise FRONTEND.BadParameterException("allocateCapacity(): INVALID RATE")
                    

                    try:
                        self._dev_set_all(frontend_tuner_allocation.center_frequency,
                                     frontend_tuner_allocation.bandwidth,
                                     frontend_tuner_allocation.sample_rate,
                                     tuner_id)
                        #self._dev_set_center_frequency(frontend_tuner_allocation.center_frequency,tuner_id);
                        #self._dev_set_bandwidth(frontend_tuner_allocation.bandwidth,tuner_id);
                        #self._dev_set_sample_rate(frontend_tuner_allocation.sample_rate,tuner_id);
                    except:
                        #TODO: add back log messages
                        print "allocate_frontend_tuner_allocation: failed when configuring device hardware"
                        raise RuntimeError("allocateCapacity(%s): failed when configuring device hardware"%(tuner_id))

                    try:
                        self._dev_get_all(self.tunerChannels[tuner_id].frontend_status.center_frequency,
                                     self.tunerChannels[tuner_id].frontend_status.bandwidth,
                                     self.tunerChannels[tuner_id].frontend_status.sample_rate,
                                     tuner_id);
                        #self.tunerChannels[tuner_id].frontend_status.center_frequency = self._dev_get_center_frequency(tuner_id);
                        #self.tunerChannels[tuner_id].frontend_status.bandwidth = self._dev_get_bandwidth(tuner_id);
                        #self.tunerChannels[tuner_id].frontend_status.sample_rate = self._dev_get_sample_rate(tuner_id);
                    
                    except:
                        #TODO: add back log messages
                        print "allocate_frontend_tuner_allocation: failed when querying device hardware"
                        raise RuntimeError("allocateCapacity(%s): failed when querying device hardware"%(tuner_id))

                    # Only check non-TX when bandwidth was not set to don't care
                    if (self.tunerChannels[tuner_id].frontend_status.tuner_type != "TX" and frontend_tuner_allocation.bandwidth != 0.0) and \
                        (self.tunerChannels[tuner_id].frontend_status.bandwidth < frontend_tuner_allocation.bandwidth or \
                        self.tunerChannels[tuner_id].frontend_status.bandwidth > frontend_tuner_allocation.bandwidth+frontend_tuner_allocation.bandwidth * frontend_tuner_allocation.bandwidth_tolerance/100.0 ):
                        #TODO: add back log messages
                        print "allocate_frontend_tuner_allocation: did not meet BW tolerance"
                        raise RuntimeError('allocateCapacity(%s): returned bw "%s" does not meet tolerance criteria of "%s + %s percent".'%(tuner_id,
                                                                                                                                            self.tunerChannels[tuner_id].frontend_status.bandwidth,
                                                                                                                                            frontend_tuner_allocation.bandwidth,
                                                                                                                                            frontend_tuner_allocation.bandwidth_tolerance))
                    
                    # always check TX, but only check non-TX when sample_rate was not set to don't care)
                    if (self.tunerChannels[tuner_id].frontend_status.tuner_type == "TX" or frontend_tuner_allocation.sample_rate != 0.0) and \
                        (self.tunerChannels[tuner_id].frontend_status.sample_rate < frontend_tuner_allocation.sample_rate or \
                        self.tunerChannels[tuner_id].frontend_status.sample_rate > frontend_tuner_allocation.sample_rate+frontend_tuner_allocation.sample_rate * frontend_tuner_allocation.sample_rate_tolerance/100.0 ):
                        #TODO: add back log messages
                        print "allocate_frontend_tuner_allocation: did not meet sample rate tolerance"
                        raise RuntimeError('allocateCapacity(%s): returned sample rate "%s" does not meet tolerance criteria of "%s + %s percent".'%(tuner_id,
                                                                                                                                                     self.tunerChannels[tuner_id].frontend_status.sample_rate,
                                                                                                                                                     frontend_tuner_allocation.sample_rate,
                                                                                                                                                     frontend_tuner_allocation.sample_rate_tolerance))
                    
                finally:
                    # release tuner lock
                    self.tunerChannels[tuner_id].lock.release()
                    
                # enable tuner after successful allocation
                try:
                    enableTuner(tuner_id,True);
                except:
                    #TODO: add back log messages
                    print "allocate_frontend_tuner_allocation: FAILED TO ENABLE TUNER AFTER ALLOCATION"
                    raise RuntimeError("FAILED TO ENABLE TUNER AFTER ALLOCATION")
                
        except RuntimeError, e:
            self.deallocateCapacity(frontend_tuner_allocation)
            return False
        
        except CF.Device.InvalidCapacity, e:
            # without the following check, a valid allocation could be deallocated due to an attempt to alloc w/ an existing alloc id
            if e.msg != "ALLOCATION_ID ALREADY IN USE":
                self.deallocateCapacity(frontend_tuner_allocation)
            #else
            raise e
        
        except FRONTEND.BadParameterException, e:
            self.deallocateCapacity(frontend_tuner_allocation)
            return False
        
        except Exception, e:
            self.deallocateCapacity(frontend_tuner_allocation)
            raise e

        return True

    def deallocate_frontend_tuner_allocation(self, frontend_tuner_allocation):
        # Try to remove control of the device
        tuner_id = self.getTunerMapping(frontend_tuner_allocation.allocation_id)
        if tuner_id < 0:
            raise CF.Device.InvalidState
        if self.tunerChannels[tuner_id].control_allocation_id == frontend_tuner_allocation.allocation_id:
            self.removeTuner(tuner_id)
            self.removeTunerMapping(tuner_id)
        else:
            # send EOS to listener connection only
            self.push_EOS_on_listener(frontend_tuner_allocation.allocation_id)
            self.removeTunerMapping(frontend_tuner_allocation.allocation_id)
        
        self.tunerChannels[tuner_id].frontend_status.allocation_id_csv = self.create_allocation_id_csv(tuner_id)

    def allocate_frontend_listener_allocation(self, frontend_listener_allocation):
        try:
            # Check validity of allocation_id's
            if not frontend_listener_allocation.existing_allocation_id:
                #TODO: add back log messages
                print "allocateCapacity: MISSING EXISTING ALLOCATION ID"
                raise CF.Device.InvalidCapacity("MISSING EXISTING ALLOCATION ID", frontend_listener_allocation)
            
            if not frontend_listener_allocation.listener_allocation_id:
                #TODO: add back log messages
                print "allocateCapacity: MISSING LISTENER ALLOCATION ID"
                raise CF.Device.InvalidCapacity("MISSING LISTENER ALLOCATION ID", frontend_listener_allocation)
            
            # Check if listener allocation ID has already been used
            if self.getTunerMapping(frontend_listener_allocation.listener_allocation_id) >= 0:
                #TODO: add back log messages
                print "allocateCapacity: LISTENER ALLOCATION ID ALREADY IN USE"
                raise CF.Device.InvalidCapacity("LISTENER ALLOCATION ID ALREADY IN USE", frontend_listener_allocation)
            

            if self.addTunerMapping(frontend_listener_allocation) < 0:
                #TODO: add back log messages
                print "allocateCapacity: UNKNOWN CONTROL ALLOCATION ID"
                raise FRONTEND.BadParameterException("UNKNOWN CONTROL ALLOCATION ID");
            
                
        except RuntimeError, e:
            self.deallocateCapacity(frontend_listener_allocation)
            return False
        
        except CF.Device.InvalidCapacity, e:
            # without the following check, a valid allocation could be deallocated due to an attempt to alloc w/ an existing alloc id
            if e.msg != "ALLOCATION_ID ALREADY IN USE":
                self.deallocateCapacity(frontend_listener_allocation)
            #else
            raise e
        
        except FRONTEND.BadParameterException, e:
            self.deallocateCapacity(frontend_listener_allocation)
            return False
        
        except Exception, e:
            self.deallocateCapacity(frontend_listener_allocation)
            raise e

        return True

    def deallocate_frontend_listener_allocation(self, frontend_listener_allocation):
        tuner_id = self.getTunerMapping(frontend_tuner_allocation.allocation_id)
        if tuner_id < 0:
            raise CF.Device.InvalidState
        # send EOS to listener connection only
        self.push_EOS_on_listener(frontend_listener_allocation.listener_allocation_id)
        self.removeTunerMapping(frontend_listener_allocation.listener_allocation_id)
        self.tunerChannels[tuner_id].frontend_status.allocation_id_csv = self.create_allocation_id_csv(tuner_id)

    def updateUsageState(self):
        """
        This is called automatically after allocateCapacity or deallocateCapacity are called.
        Your implementation should determine the current state of the device:
           self._usageState = CF.Device.IDLE   # not in use
           self._usageState = CF.Device.ACTIVE # in use, with capacity remaining for allocation
           self._usageState = CF.Device.BUSY   # in use, with no capacity remaining for allocation
        """
        tunerAllocated = 0;
        for tuner in self.tunerChannels:
            if tuner.control_allocation_id:
                tunerAllocated+=1
                
        # If no tuners are allocated, device is idle
        if tunerAllocated == 0:
            return CF.Device.IDLE
        # If all tuners are allocated, device is busy
        if tunerAllocated == self.tunerChannels.size():
            return CF.Device.BUSY
        # Else, device is active
        return CF.Device.ACTIVE
        
        
    # Mapping and translation helpers. External string identifiers to internal numerical identifiers
    def addTunerMapping(self, frontend_alloc):
        NO_VALID_TUNER = -1

        # Do not allocate if allocation ID has already been used
        if self.getTunerMapping(frontend_alloc.allocation_id) >= 0 :
            return NO_VALID_TUNER
            
        self.allocationID_MappingLock.acquire()
        try:

            # Next, try to allocate a new tuner
            for tuner_id,tunerChannel in enumerate(self.tunerChannels):
                if tunerChannel.frontend_status.tuner_type != frontend_alloc.tuner_type:
                    continue
                
                #listen
                if not frontend_alloc.device_control and tunerChannel.control_allocation_id:
                    freq_valid = self.is_freq_valid(
                            frontend_alloc.center_frequency,
                            frontend_alloc.bandwidth,
                            frontend_alloc.sample_rate,
                            tunerChannel.frontend_status.center_frequency,
                            tunerChannel.frontend_status.bandwidth,
                            tunerChannel.frontend_status.sample_rate)
                    if freq_valid:
                        self.allocationID_to_tunerID[frontend_alloc.allocation_id] = tuner_id
                        tunerChannel.frontend_status.allocation_id_csv = self.create_allocation_id_csv(tuner_id)
                        return tuner_id
                    
                #control
                elif frontend_alloc.device_control and not tunerChannel.control_allocation_id:
                    tunerChannel.control_allocation_id = frontend_alloc.allocation_id
                    self.allocationID_to_tunerID[frontend_alloc.allocation_id] = tuner_id
                    tunerChannel.frontend_status.allocation_id_csv = self.create_allocation_id_csv(tuner_id)
                    return tuner_id
            return NO_VALID_TUNER
        finally:
            self.allocationID_MappingLock.release()
    
    def addListenerMapping(self, frontend_listener_alloc):
        NO_VALID_TUNER = -1

        # Do not allocate if allocation ID has already been used
        if self.getTunerMapping(frontend_listener_alloc.listener_allocation_id) >= 0:
            return NO_VALID_TUNER

        # Do not allocate if existing allocation ID does not exist
        tuner_id = self.getTunerMapping(frontend_listener_alloc.existing_allocation_id)
        if tuner_id < 0:
            return NO_VALID_TUNER

        self.allocationID_MappingLock.acquire()
        try:
            self.allocationID_to_tunerID[frontend_listener_alloc.listener_allocation_id] = tuner_id
            self.tunerChannels[tuner_id].frontend_status.allocation_id_csv = create_allocation_id_csv(tuner_id)
            return tuner_id
        finally:
            self.allocationID_MappingLock.release()

    def removeTunerMapping(self, allocation_id):
        self.allocationID_MappingLock.acquire()
        try:
            if allocation_id in self.allocationID_to_tunerID:
                del self.allocationID_to_tunerID[allocation_id]
                return True;
            return False;
        finally:
            self.allocationID_MappingLock.release()
    
    def removeTunerMapping(self, tuner_id):
        self.allocationID_MappingLock.acquire()
        try:
            cnt = 0
            for k,v in allocationID_to_tunerID.items():
                if v == tuner_id:
                    del self.allocationID_to_tunerID[k]
                    cnt+=1
            return cnt > 0
        finally:
            self.allocationID_MappingLock.release()
            
    def getTunerMapping(self, allocation_id):
        NO_VALID_TUNER = -1
        self.allocationID_MappingLock.acquire()
        try:
            if allocation_id in self.allocationID_to_tunerID:
                return self.allocationID_to_tunerID[allocation_id]
            return NO_VALID_TUNER
        finally:
            self.allocationID_MappingLock.release()
    
    def is_connectionID_valid_for_tunerID(self, tuner_id, connectionID):
        if connectionID not in self.allocationID_to_tunerID:
            return False
        if self.allocationID_to_tunerID[connectionID] != tuner_id:
            return False
        return True
    
    def is_connectionID_valid_for_streamID(self, streamID, connectionID):
        if streamID not in self.streamID_to_tunerID:
            return False
        return self.is_connectionID_valid_for_tunerID(self.streamID_to_tunerID[streamID], connectionID)
    
    def is_connectionID_controller_for_streamID(self, streamID, connectionID):
        if streamID not in self.streamID_to_tunerID:
            return False
        if not self.is_connectionID_valid_for_tunerID(self.streamID_to_tunerID[streamID], connectionID):
            return False
        if self.tunerChannels[self.streamID_to_tunerID[streamID]].control_allocation_id != connectionID:
            return False
        return True
    
    def is_connectionID_listener_for_streamID(self, streamID, connectionID):
        if streamID not in self.streamID_to_tunerID:
            return False
        if not self.is_connectionID_valid_for_tunerID(self.streamID_to_tunerID[streamID], connectionID):
            return False
        if self.tunerChannels[self.streamID_to_tunerID[streamID]].control_allocation_id == connectionID:
            return False
        return True
    
    def is_freq_valid(self, req_cf, req_bw, req_sr, cf, bw, sr):
        req_min_bw_sr = min(req_bw,req_sr)
        min_bw_sr = min(bw,sr)
        if (req_cf + req_min_bw_sr/2.0 <= cf + min_bw_sr/2.0) and (req_cf - req_min_bw_sr/2.0 >= cf - min_bw_sr/2.0):
            return True
        return False

    # Configure tuner - gets called during allocation
    def enableTuner(self, tuner_id, enable):
        ''' assumes collector RF and channel RF are the same. If not True, override function
        '''
        # Lock the tuner
        self.tunerChannels[tuner_id].lock.acquire()
        try:
            prev_enabled = self.tunerChannels[tuner_id].frontend_status.enabled
            self.tunerChannels[tuner_id].frontend_status.enabled = enable
    
            # If going from disabled to enabled
            if not prev_enabled and enable:
                self.configureTunerSRI(self.tunerChannels[tuner_id].sri,
                        self.tunerChannels[tuner_id].frontend_status.center_frequency,
                        self.tunerChannels[tuner_id].frontend_status.bandwidth,
                        self.tunerChannels[tuner_id].frontend_status.sample_rate,
                        self.tunerChannels[tuner_id].complex,
                        self.tunerChannels[tuner_id].frontend_status.rf_flow_id)
                self.streamID_to_tunerID[self.tunerChannels[tuner_id].sri.streamID] = tuner_id
                self._dev_enable(tuner_id)
    
            # If going from enabled to disabled
            if prev_enabled and not enable and self.tunerChannels[tuner_id].sri.streamID:
                self._dev_disable(tuner_id)
                streamID = self.tunerChannels[tuner_id].sri.streamID
                if streamID in self.streamID_to_tunerID:
                    del self.streamID_to_tunerID[streamID]
                zeroSRI(self.tunerChannels[tuner_id].sri)
    
            return True
        finally:
            self.tunerChannels[tuner_id].lock.release()
    
    def removeTuner(self, tuner_id):
        self.enableTuner(tuner_id, False);
        self.tunerChannels[tuner_id].reset()
        return True

    def create_allocation_id_csv(self, tuner_id):
        alloc_ids = []
        # ensure control allocation_id is first in list
        if self.tunerChannels[tuner_id].control_allocation_id:
            alloc_ids = [self.tunerChannels[tuner_id].control_allocation_id]
        # now add the rest
        for allocID,tunerID in self.allocationID_to_tunerID.items():
            if tunerID == tuner_id and allocID not in alloc_ids:
                alloc_ids.append(allocID)
                
        return ','.join(alloc_ids)

    #############################
    # Device specific functions # -- to be implemented by device developer
    #############################
    def push_EOS_on_listener(self, listener_allocation_id):
        raise NotImplementedError

    def _valid_tuner_type(self, tuner_type):
        raise NotImplementedError
    def _valid_center_frequency(self, req_freq, tuner_id):
        raise NotImplementedError
    def _valid_bandwidth(self, req_bw, tuner_id):
        raise NotImplementedError
    def _valid_sample_rate(self, req_sr, tuner_id):
        raise NotImplementedError

    def _dev_enable(self, tuner_id):
        raise NotImplementedError
    def _dev_disable(self, tuner_id):
        raise NotImplementedError

    def _dev_set_all(self, req_freq,  req_bw,  req_sr, tuner_id):
        raise NotImplementedError
    def _dev_set_center_frequency(self, req_freq, tuner_id):
        raise NotImplementedError
    def _dev_set_bandwidth(self, req_bw, tuner_id):
        raise NotImplementedError
    def _dev_set_sample_rate(self, req_sr, tuner_id):
        raise NotImplementedError

    def _dev_get_all(self, freq, bw, sr, tuner_id):
        raise NotImplementedError
    def _dev_get_center_frequency(self, tuner_id):
        raise NotImplementedError
    def _dev_get_bandwidth(self, tuner_id):
        raise NotImplementedError
    def _dev_get_sample_rate(self, tuner_id):
        raise NotImplementedError

    ############################
    ## Other helper functions ##
    ############################

    def optimize_rate(self, req_rate, max_rate, min_rate):
        #for dec in range(int(max_rate/min_rate),0,-1):
        #    if(max_rate/float(dec) >= req_rate):
        #        return max_rate/double(dec)
        #
        if req_rate < min_rate:
            return min_rate
        return req_rate

    def addModifyKeyword(self, sri, id, myValue, addOnly=False):
        if not addOnly:
            for keyword in sri.keywords:
                if keyword.id == id:
                    keyword.value = any.to_any(myValue)
                    return True
        sri.keywords.append(CF.DataType(id=id, value=any.to_any(myValue)))
        return True

    def configureTunerSRI(self, sri, channel_frequency, bandwidth, sample_rate, mode, rf_flow_id, collector_frequency = -1.0):
        if sri == None:
            return

        chanFreq = int(channel_frequency)

        #Create new streamID
        streamID = "tuner_freq_" + str(chanFreq) + "_Hz_" + str(uuid.uuid4())
        
        sri.mode = mode
        sri.hversion = 0
        sri.xstart = 0.0
        sri.xdelta = 1.0 / sample_rate
        sri.subsize = 0 # 1-dimensional data
        sri.xunits = 1
        sri.ystart = 0
        sri.ydelta = 0.001
        sri.yunits = 1
        sri.streamID = streamID
        sri.blocking = False
        
        
        # for some devices, colFreq is the same as chanFreq
        if (collector_frequency < 0):
            colFreq = chanFreq
        else:
            colFreq = int(collector_frequency)

        addModifyKeyword(sri, "COL_RF", float(colFreq))
        addModifyKeyword(sri, "CHAN_RF", float(chanFreq))
        addModifyKeyword(sri,"FRONTEND::RF_FLOW_ID",str(rf_flow_id))
        addModifyKeyword(sri,"FRONTEND::BANDWIDTH", float(bandwidth))
        addModifyKeyword(sri,"FRONTEND::DEVICE_ID", str(self._get_identifier()))

    # This is not currently used but is available as a debugging tool
    def printSRI(self, sri, strHeader = "DEBUG SRI"):
        print strHeader
        print "\thversion:",  sri.hversion
        print "\txstart:",  sri.xstart
        print "\txdelta:",  sri.xdelta
        print "\txunits:",  sri.xunits
        print "\tsubsize:",  sri.subsize
        print "\tystart:",  sri.ystart
        print "\tydelta:",  sri.ydelta
        print "\tyunits:",  sri.yunits
        print "\tmode:",  sri.mode
        print "\tstreamID:",  sri.streamID
        for keyword in sri.keywords:
            print "\t KEYWORD KEY/VAL ::",  keywords.id << ":",  any.from_any(keywords.value)
            
    ######################################################################
    # PROPERTIES
    # 
    # DO NOT ADD NEW PROPERTIES HERE.  You can add properties in your derived class, in the PRF xml file
    # or by using the IDE.
    device_kind = simple_property(id_="DCE:cdc5ee18-7ceb-4ae6-bf4c-31f983179b4d",
                                  name="device_kind",
                                  type_="string",
                                  defvalue="FRONTEND::TUNER",
                                  mode="readonly",
                                  action="eq",
                                  kinds=("allocation","configure"),
                                  description="""This specifies the device kind"""
                                  )
    device_model = simple_property(id_="DCE:0f99b2e4-9903-4631-9846-ff349d18ecfb",
                                   name="device_model",
                                   type_="string",
                                   defvalue="",
                                   mode="readonly",
                                   action="eq",
                                   kinds=("allocation","configure"),
                                   description=""" This specifies the specific device"""
                                   )
    frontend_tuner_allocation = struct_property(id_="FRONTEND::tuner_allocation",
                                                name="frontend_tuner_allocation",
                                                structdef=FrontendTunerAllocation,
                                                configurationkind=("allocation",),
                                                mode="readwrite",
                                                description="""Frontend Interfaces v2.0 main allocation structure"""
                                                )
    frontend_listener_allocation = struct_property(id_="FRONTEND::listener_allocation",
                                                   name="frontend_listener_allocation",
                                                   structdef=FrontendListenerAllocation,
                                                   configurationkind=("allocation",),
                                                   mode="readwrite",
                                                   description="""Allocates a listener (subscriber) based off a previous allocation """
                                                   )
    # TODO - need to have this use the updated non-default tuner status struct class in the device's auto generated base class
    frontend_tuner_status = structseq_property(id_="FRONTEND::tuner_status",
                                               name="frontend_tuner_status",
                                               structdef=DefaultFrontendTunerStatusStruct,
                                               defvalue=[],
                                               configurationkind=("configure",),
                                               mode="readonly",
                                               description="""Frontend Interfaces v2.0 status structure. One element for every frontend resource (receiver, transmitter) configured on this hardware"""
                                               )


    
