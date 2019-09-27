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
from ossie.cf import CF, CF__POA
from ossie.utils import uuid
from omniORB import any, CORBA

from ossie.device import Device
from ossie.properties import simple_property
from ossie.properties import struct_property
from ossie.properties import structseq_property
from ossie.properties import struct_to_props
from ossie.utils import model

import threading
from bulkio.bulkioInterfaces import BULKIO
from redhawk.frontendInterfaces import FRONTEND
from ossie.cf import ExtendedCF
from fe_types import *

class AllocationAlreadyExists(CF.Device.InvalidCapacity):
    def __init__(self, msg, props):
        CF.Device.InvalidCapacity(msg,props)

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

# Tuner Allocation IDs struct. This structure contains allocation tracking data.
class tunerAllocationIdsStruct:
    def __init__(self):
        self.reset()

    def reset(self):
        self.control_allocation_id = ''
        self.listener_allocation_ids = []

''' floatingPointCompare is a helper function to handle floating point comparison
      Return values:
        if lhs == rhs: 0.0
        if lhs >  rhs: 1.0 or greater
        if lhs <  rhs: -1.0 or less
      Recommended usage is to convert a comparison such as: (lhs OP rhs)
      to (floatingPointCompare(lhs,rhs) OP 0), where OP is a comparison operator
      (==, <, >, <=, >=, !=).
      "places" is used to specify precision. The default is 1, which
      uses a single decimal place of precision.
'''

def floatingPointCompare(lhs, rhs, places = 1):
    return round((lhs-rhs)*pow(10,places))

''' validateRequest is a helper function to verify a value is within a range, returning
     true if the value requested_val falls within the range [available_min:available_max]
     False is returned if min > max
'''
def validateRequestSingle(available_min, available_max, requested_val):
    if floatingPointCompare(requested_val,available_min) < 0: return False
    if floatingPointCompare(requested_val,available_max) > 0: return False
    if floatingPointCompare(available_min,available_max) > 0: return False
    return True

''' validateRequest is a helper function to compare two ranges, returning true if the range
     [requested_min:requested_max] falls within the range [available_min:available_max]
     False is returned if min > max for either available for requested values
'''
def validateRequest(available_min, available_max, requested_min, requested_max):
    center_request = (requested_max+requested_min)/2.0
    if floatingPointCompare(center_request,available_min) < 0: return False
    if floatingPointCompare(center_request,available_max) > 0: return False
    if floatingPointCompare(available_min,available_max) > 0: return False
    if floatingPointCompare(requested_min,requested_max) > 0: return False
    return True

''' validateRequestVsSRI is a helper function to check that the input data stream can support
     the allocation request. The output mode (true if complex output) is used when determining
     the necessary sample rate required to satisfy the request. The entire frequency band of the
     request must be available for True to be returned, not just the center frequency.
     True is returned upon success, otherwise FRONTEND.BadParameterException is thrown.
     If the CHAN_RF and FRONTEND.BANDWIDTH keywords are not found in the sri,
     FRONTEND.BadParameterException is thrown.
'''
def validateRequestVsSRI(request,upstream_sri,output_mode):

    # get center frequency and bandwidth from SRI keywords
    upstream_cf = 0.0
    upstream_bw = 0.0
    found_cf = False
    found_bw = False
    key_size = len(upstream_sri.keywords)
    for i in range(key_size):
        if upstream_sri.keywords[i].id == "CHAN_RF":
            upstream_cf = any.from_any(upstream_sri.keywords[i].value)
            found_cf = True
        elif upstream_sri.keywords[i].id == "FRONTEND::BANDWIDTH":
            upstream_bw = any.from_any(upstream_sri.keywords[i].value)
            found_bw = True
    if not found_cf or not found_bw:
        raise FRONTEND.BadParameterException("CANNOT VERIFY REQUEST -- SRI missing required keywords")

    # check bandwidth
    min_upstream_freq = upstream_cf-(upstream_bw/2)
    max_upstream_freq = upstream_cf+(upstream_bw/2)
    min_requested_freq = request.center_frequency-(request.bandwidth/2)
    max_requested_freq = request.center_frequency+(request.bandwidth/2)

    if not validateRequest(min_upstream_freq,max_upstream_freq,min_requested_freq,max_requested_freq):
        raise FRONTEND.BadParameterException("INVALID REQUEST -- input data stream cannot support freq/bw request")

    # check sample rate
    upstream_sr = 1/upstream_sri.xdelta
    input_scaling_factor = 2
    if upstream_sri.mode == 1:
        input_scaling_factor = 4 # adjust for complex data
    min_upstream_freq = upstream_cf-(upstream_sr/input_scaling_factor)
    max_upstream_freq = upstream_cf+(upstream_sr/input_scaling_factor)
    output_scaling_factor = 2
    if output_mode == 1:
        output_scaling_factor = 4 # adjust for complex data
    min_requested_freq = request.center_frequency-(request.sample_rate/output_scaling_factor)
    max_requested_freq = request.center_frequency+(request.sample_rate/output_scaling_factor)

    if not validateRequest(min_upstream_freq,max_upstream_freq,min_requested_freq,max_requested_freq):
        raise FRONTEND.BadParameterException("INVALID REQUEST -- input data stream cannot support freq/sr request")

    return True

''' validateRequestVsDevice is a helper function to check that the input data stream and the
      device can support an allocation request. The output mode (true if complex output) is used
      when determining the necessary sample rate required to satisfy the request. The entire
      frequency band of the request must be available for True to be returned, not just the center
      frequency.
      True is returned upon success, otherwise FRONTEND.BadParameterException is thrown.
      If the CHAN_RF and FRONTEND.BANDWIDTH keywords are not found in the sri,
      FRONTEND.BadParameterException is thrown.
'''
def validateRequestVsDeviceStream(request, upstream_sri, output_mode, min_device_center_freq, max_device_center_freq, max_device_bandwidth, max_device_sample_rate):

    # check if request can be satisfied using the available upstream data
    if not validateRequestVsSRI(request,upstream_sri, output_mode):
        raise FRONTEND.BadParameterException("INVALID REQUEST -- falls outside of input data stream")

    # check device constraints
    # check vs. device center frequency capability (ensure 0 <= request <= max device capability)
    if not validateRequestSingle(min_device_center_freq,max_device_center_freq,request.center_frequency):
        raise FRONTEND.BadParameterException("INVALID REQUEST -- device capabilities cannot support freq request")

    # check vs. device bandwidth capability (ensure 0 <= request <= max device capability)
    if not validateRequestSingle(0,max_device_bandwidth,request.bandwidth):
        raise FRONTEND.BadParameterException("INVALID REQUEST -- device capabilities cannot support bw request")

    # check vs. device sample rate capability (ensure 0 <= request <= max device capability)
    if not validateRequestSingle(0,max_device_sample_rate,request.sample_rate):
        raise FRONTEND.BadParameterException("INVALID REQUEST -- device capabilities cannot support sr request")

    # calculate overall frequency range of the device (not just CF range)
    output_scaling_factor = 2
    if output_mode == 1:
        output_scaling_factor = 4 # adjust for complex data
    min_device_freq = min_device_center_freq-(max_device_sample_rate/output_scaling_factor)
    max_device_freq = max_device_center_freq+(max_device_sample_rate/output_scaling_factor)

    # check based on bandwidth
    # this duplicates part of check above if device freq range = input freq range
    min_requested_freq = request.center_frequency-(request.bandwidth/2)
    max_requested_freq = request.center_frequency+(request.bandwidth/2)
    if not validateRequest(min_device_freq,max_device_freq,min_requested_freq,max_requested_freq):
        raise FRONTEND.BadParameterException("INVALID REQUEST -- device capabilities cannot support freq/bw request")

    # check based on sample rate
    # this duplicates part of check above if device freq range = input freq range
    min_requested_freq = request.center_frequency-(request.sample_rate/output_scaling_factor)
    max_requested_freq = request.center_frequency+(request.sample_rate/output_scaling_factor)
    if not validateRequest(min_device_freq,max_device_freq,min_requested_freq,max_requested_freq):
        raise FRONTEND.BadParameterException("INVALID REQUEST -- device capabilities cannot support freq/sr request")

    return True

''' validateRequestVsRFInfo is a helper function to check that the analog capabilities can support
     the allocation request. The mode (true if complex) is used when determining the necessary
     sample rate required to satisfy the request. The entire frequency band of the request must be
     available for True to be returned, not just the center frequency.
     True is returned upon success, otherwise FRONTEND.BadParameterException is thrown.
'''
def validateRequestVsRFInfo(request, rfinfo, mode):

    min_analog_freq = rfinfo.rf_center_freq-(rfinfo.rf_bandwidth/2)
    max_analog_freq = rfinfo.rf_center_freq+(rfinfo.rf_bandwidth/2)

    # check bandwidth
    min_requested_freq = request.center_frequency-(request.bandwidth/2)
    max_requested_freq = request.center_frequency+(request.bandwidth/2)

    if not validateRequest(min_analog_freq,max_analog_freq,min_requested_freq,max_requested_freq):
        raise FRONTEND.BadParameterException("INVALID REQUEST -- analog freq range (RFinfo) cannot support freq/bw request")

    # check sample rate
    scaling_factor = 4
    if mode:
        scaling_factor = 2 # adjust for complex data
    min_requested_freq = request.center_frequency-(request.sample_rate/scaling_factor)
    max_requested_freq = request.center_frequency+(request.sample_rate/scaling_factor)

    if not validateRequest(min_analog_freq,max_analog_freq,min_requested_freq,max_requested_freq):
        raise FRONTEND.BadParameterException("INVALID REQUEST -- analog freq range (RFinfo) cannot support freq/sr request")

    return True

''' validateRequestVsDevice is a helper function to check that the analog capabilities and the
     device can support the allocation request. The mode (true if complex) is used when
     determining the necessary sample rate required to satisfy the request. The entire frequency
     band of the request must be available for True to be returned, not just the center frequency.
     True is returned upon success, otherwise FRONTEND.BadParameterException is thrown.
'''
def validateRequestVsDevice(request, rfinfo, mode, min_device_center_freq, max_device_center_freq, max_device_bandwidth, max_device_sample_rate):

    # check if request can be satisfied using the available upstream data
    if request.tuner_type != "TX" and not validateRequestVsRFInfo(request,rfinfo, mode):
        raise FRONTEND.BadParameterException("INVALID REQUEST -- analog freq range (RFinfo) cannot support request")

    # check device constraints
    # see if IF center frequency is set in rfinfo packet
    request_if_center_freq = request.center_frequency
    if request.tuner_type != "TX" and floatingPointCompare(rfinfo.if_center_freq,0) > 0 and floatingPointCompare(rfinfo.rf_center_freq,rfinfo.if_center_freq) > 0:
        if rfinfo.spectrum_inverted:
            request_if_center_freq = rfinfo.if_center_freq - (request.center_frequency - rfinfo.rf_center_freq)
        else:
            request_if_center_freq = rfinfo.if_center_freq + (request.center_frequency - rfinfo.rf_center_freq)

    # check vs. device center freq capability (ensure 0 <= request <= max device capability)
    if not validateRequestSingle(min_device_center_freq, max_device_center_freq, request_if_center_freq):
        raise FRONTEND.BadParameterException("INVALID REQUEST -- device capabilities cannot support freq request")

    # check vs. device bandwidth capability (ensure 0 <= request <= max device capability)
    if not validateRequestSingle(0, max_device_bandwidth, request.bandwidth):
        raise FRONTEND.BadParameterException("INVALID REQUEST -- device capabilities cannot support bw request")

    # check vs. device sample rate capability (ensure 0 <= request <= max device capability)
    if not validateRequestSingle(0, max_device_sample_rate, request.sample_rate):
        raise FRONTEND.BadParameterException("INVALID REQUEST -- device capabilities cannot support sr request")

    # calculate overall frequency range of the device (not just CF range)
    scaling_factor = 2
    if mode == 1:
        scaling_factor = 4 # adjust for complex data
    min_device_freq = min_device_center_freq-(max_device_sample_rate/scaling_factor)
    max_device_freq = max_device_center_freq+(max_device_sample_rate/scaling_factor)

    # check based on bandwidth
    min_requested_freq = request_if_center_freq-(request.bandwidth/2)
    max_requested_freq = request_if_center_freq+(request.bandwidth/2)

    if not validateRequest(min_device_freq, max_device_freq, min_requested_freq, max_requested_freq):
        raise FRONTEND.BadParameterException("INVALID REQUEST -- device capabilities cannot support freq/bw request")

    # check based on sample rate
    min_requested_freq = request_if_center_freq-(request.sample_rate/scaling_factor)
    max_requested_freq = request_if_center_freq+(request.sample_rate/scaling_factor)

    if not validateRequest(min_device_freq, max_device_freq, min_requested_freq, max_requested_freq):
        raise FRONTEND.BadParameterException("INVALID REQUEST -- device capabilities cannot support freq/sr request")

    return True

def createScannerAllocation(min_freq=0.0, max_freq=0.0, mode="", control_mode="", control_limit=0.0, returnDict=True):
    if returnDict:
        retval = {'FRONTEND::scanner_allocation':{'FRONTEND::scanner_allocation::min_freq':min_freq,'FRONTEND::scanner_allocation::max_freq':max_freq,
        'FRONTEND::scanner_allocation::mode':mode,'FRONTEND::scanner_allocation::control_mode':control_mode,
        'FRONTEND::scanner_allocation::control_limit':control_limit}}
    else:
        alloc=[]
        alloc.append(CF.DataType(id='FRONTEND::scanner_allocation::min_freq',value=any.to_any(min_freq)))
        alloc[-1].value._t = CORBA.TC_double
        alloc.append(CF.DataType(id='FRONTEND::scanner_allocation::max_freq',value=any.to_any(max_freq)))
        alloc[-1].value._t = CORBA.TC_double
        alloc.append(CF.DataType(id='FRONTEND::scanner_allocation::mode',value=any.to_any(mode)))
        alloc.append(CF.DataType(id='FRONTEND::scanner_allocation::control_mode',value=any.to_any(control_mode)))
        alloc.append(CF.DataType(id='FRONTEND::scanner_allocation::control_limit',value=any.to_any(control_limit)))
        alloc[-1].value._t = CORBA.TC_double
        retval = CF.DataType(id='FRONTEND::scanner_allocation',value=CORBA.Any(CF._tc_Properties,alloc))
    return retval

def createTunerAllocation(tuner_type='RX_DIGITIZER',allocation_id=None,center_frequency=0.0,bandwidth=0.0,sample_rate=0.0,
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

def createTunerGenericListenerAllocation(tuner_type='RX_DIGITIZER',allocation_id=None,center_frequency=0.0,bandwidth=0.0,sample_rate=1.0,
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
    numTuners = len(device.frontend_tuner_status)
    newAllocation = False
    allAllocated = False
    #No tuners found on device
    if numTuners == 0:
        print "No Available Tuner"
    else:
        if numTuners >= 1:
            for index, key in enumerate(device.frontend_tuner_status):
                id_csv = device.frontend_tuner_status[index].allocation_id_csv
                if len(id_csv) != 0:
                    allAllocated = True
                if allocation_id != None and allocation_id in id_csv:
                    break
                if id_csv == '':
                    if sample_rate == None or center_frequency == None:
                        print "tune(): tune did not occur, must set center_frequency and sample_rate"
                        return None
                    else:
                        alloc=createTunerAllocation(tuner_type, allocation_id,center_frequency,bandwidth, sample_rate,device_control,group_id,rf_flow_id,bandwidth_tolerance,sample_rate_tolerance,returnDict)
                        alloc_results = device.allocateCapacity(alloc)
                        if alloc_results == True:
                            newAllocation = True
                            allocation_id = alloc['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::allocation_id']
                            if gain != None:
                                tuner = None
                                if "DigitalTuner_in" in device._providesPortDict.keys():
                                    tuner_type = "DigitalTuner"
                                    tuner = device.getPort("DigitalTuner_in")
                                elif "AnalogTuner_in" in device._providesPortDict.keys():
                                    tuner_type = "AnalogTuner"
                                    tuner = device.getPort("AnalogTuner_in")
                                if tuner != None:
                                    if tuner.getTunerAgcEnable(allocation_id) == True:
                                        print "tune(): Agc is enabled, disabling to allow setting of the gain to " + str(gain)
                                        tuner.setTunerAgcEnable(allocation_id,False)
                                    tuner.setTunerGain(allocation_id, gain)
                            break

        if allocation_id == None and not newAllocation and numTuners >= 1:
            if allAllocated:
                print "tune(): All tuners (", len(device.frontend_tuner_status), ") have been allocated.  Specify an allocation_id to change tuning properties"
            else:
                print "tune(): unable to allocate a tuner with the given parameters"

        elif not newAllocation:
            tuner=None
            tuner_type=None
            allocation_status = _getAllocationStatus(device, numTuners, allocation_id)
            if allocation_status == None:
                print "tune(): unable to allocate a tuner with the given parameters"
                return  allocation_status
            elif "DigitalTuner_in" in device._providesPortDict.keys():
                tuner_type = "DigitalTuner"
                tuner = device.getPort("DigitalTuner_in")
            elif "AnalogTuner_in" in device._providesPortDict.keys():
                tuner_type = "AnalogTuner"
                tuner = device.getPort("AnalogTuner_in")
            else:
                print "tune(): No DigitalTuner_in or AnalogTuner_in found"
                return allocation_status.allocation_id_csv
            if len(allocation_status.allocation_id_csv) > 0:
                # Get the control allocation_id
                allocation_id = allocation_status.allocation_id_csv.split(",")[0]
                if center_frequency != None:
                    tuner.setTunerCenterFrequency(allocation_id, center_frequency)
                if sample_rate != None:
                    tuner.setTunerOutputSampleRate(allocation_id, sample_rate)
                if gain != None:
                    if tuner.getTunerAgcEnable(allocation_id) == True:
                        print "tune(): Agc is enabled, disabling to allow setting of the gain to " + str(gain)
                        tuner.setTunerAgcEnable(allocation_id,False)
                    tuner.setTunerGain(allocation_id, gain)

            return allocation_status.allocation_id_csv
    
    return None

def _getAllocationStatus(device, numTuners, allocation_id):
    if numTuners == 1 and allocation_id == None:
        return device.frontend_tuner_status[0]
    else:
        for i in range(len(device.frontend_tuner_status)):
            allocation_ids = device.frontend_tuner_status[i].allocation_id_csv.split(',')
            if allocation_id in allocation_ids:
                return device.frontend_tuner_status[i]
    return None

def deallocate(device,allocation_id=None,allTuners=False):
    deallocated = False
    if len(device.frontend_tuner_status) == 1:
        if len(device.frontend_tuner_status[0].allocation_id_csv) > 0:
            allocProps = createTunerAllocation(allocation_id=device.frontend_tuner_status[0].allocation_id_csv.split(",")[0])
            device.deallocateCapacity(allocProps)
            deallocated = True
    else:
        if allocation_id == None and allTuners == False:
            print "deallocate(): no tuner deallocated because no allocation_id specified and allTuners set to False"
        else:
            if allTuners == True:
                for i in range(len(device.frontend_tuner_status)):
                    if len(device.frontend_tuner_status[i].allocation_id_csv) > 0:
                        allocProps = createTunerAllocation(allocation_id=device.frontend_tuner_status[i].allocation_id_csv.split(",")[0])
                        device.deallocateCapacity(allocProps)
                        deallocated = True
            else:
                for i in range(len(device.frontend_tuner_status)):
                    if allocation_id in device.frontend_tuner_status[i].allocation_id_csv:
                        allocProps = createTunerAllocation(allocation_id=allocation_id)
                        device.deallocateCapacity(allocProps)
                        deallocated = True
    return deallocated

class FrontendTunerDevice(Device):

    def __init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams):
        Device.__init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams)
        
        # tuner_allocation_ids is exclusively paired with property frontend_tuner_status.
        # tuner_allocation_ids tracks allocation ids while frontend_tuner_status provides tuner information.
        self.tuner_allocation_ids = []
    
        # Provides mapping from unique allocation ID to internal tuner (channel) number
        self.allocation_id_to_tuner_id = {}
        self.allocation_id_mapping_lock = threading.RLock()
        self.supports_scan = False

    def deallocateCapacity(self, properties):
        """
        Takes the list of properties and turns it into a dictionary.  If the 
        device has a deallocateCapacities(propDict) method it is invoked.
        
        Input:
            <properties>    A list of CF.DataType properties to allocate
            
        Output:
            None
        """
        self._deviceLog.debug("deallocateCapacity(%s)", properties)
        # Validate
        self._validateAllocProps(properties)
        # Consume
        propdict = {}
        for prop in properties:
            propdef = self._props.getPropDef(prop.id)
            # if it is a minimal tuner_allocation property (from the IDE), then add None for all the other values
            if prop.id == 'FRONTEND::tuner_allocation' and len(prop.value._v) == 1 and prop.value._v[0].id == 'FRONTEND::tuner_allocation::allocation_id':
                prop.value._v += [CF.DataType(id='FRONTEND::tuner_allocation::tuner_type',value=any.to_any(None))]
                prop.value._v += [CF.DataType(id='FRONTEND::tuner_allocation::center_frequency',value=any.to_any(None))]
                prop.value._v += [CF.DataType(id='FRONTEND::tuner_allocation::bandwidth',value=any.to_any(None))]
                prop.value._v += [CF.DataType(id='FRONTEND::tuner_allocation::bandwidth_tolerance',value=any.to_any(None))]
                prop.value._v += [CF.DataType(id='FRONTEND::tuner_allocation::sample_rate',value=any.to_any(None))]
                prop.value._v += [CF.DataType(id='FRONTEND::tuner_allocation::sample_rate_tolerance',value=any.to_any(None))]
                prop.value._v += [CF.DataType(id='FRONTEND::tuner_allocation::device_control',value=any.to_any(None))]
                prop.value._v += [CF.DataType(id='FRONTEND::tuner_allocation::group_id',value=any.to_any(None))]
                prop.value._v += [CF.DataType(id='FRONTEND::tuner_allocation::rf_flow_id',value=any.to_any(None))]
            if prop.id == 'FRONTEND::listener_allocation' and len(prop.value._v) == 1:
                prop.value._v += [CF.DataType(id='FRONTEND::listener_allocation::existing_allocation_id',value=any.to_any(None))]
            propdict[prop.id] = propdef._fromAny(prop.value)

        self._capacityLock.acquire()
        try:
            self._deallocateCapacities(propdict)
        finally:
            self._capacityLock.release()

        # Update usage state
        self._usageState = self.updateUsageState()

        self._deviceLog.debug("deallocateCapacity() -->")

    def getControlAllocationId(self, idx):
        return self.tuner_allocation_ids[idx].control_allocation_id

    def createAllocationIdCsv(self, tuner_id):
        alloc_ids = []
        self.allocation_id_mapping_lock.acquire()
        try:
            # ensure control allocation_id is first in list
            if self.tuner_allocation_ids[tuner_id].control_allocation_id:
                alloc_ids = [self.tuner_allocation_ids[tuner_id].control_allocation_id]
            # now add the rest
            for allocID,tunerID in self.allocation_id_to_tuner_id.items():
                if tunerID == tuner_id and allocID not in alloc_ids:
                    alloc_ids.append(allocID)
        finally:
            self.allocation_id_mapping_lock.release()

        return ','.join(alloc_ids)

    def updateUsageState(self):
        """
        This is called automatically after allocateCapacity or deallocateCapacity are called.
        Your implementation should determine the current state of the device:
           self._usageState = CF.Device.IDLE   # not in use
           self._usageState = CF.Device.ACTIVE # in use, with capacity remaining for allocation
           self._usageState = CF.Device.BUSY   # in use, with no capacity remaining for allocation
        """
        tunerAllocated = 0
        self.allocation_id_mapping_lock.acquire()
        try:
            for tuner in self.tuner_allocation_ids:
                if tuner.control_allocation_id:
                    tunerAllocated+=1

            # If no tuners are allocated, device is idle
            if tunerAllocated == 0:
                return CF.Device.IDLE
            # If all tuners are allocated, device is busy
            if tunerAllocated == len(self.tuner_allocation_ids):
                return CF.Device.BUSY
        finally:
            self.allocation_id_mapping_lock.release()
        # Else, device is active
        return CF.Device.ACTIVE

    # This sets the number of entries in the frontend_tuner_status struct sequence property
    # as well as the tuner_allocation_ids vector. Call this function during initialization
    def setNumChannels(self,num,tuner_type='RX_DIGITIZER'):
        self.frontend_tuner_status = []
        self.addChannels(num, tuner_type)

    # This sets the number of entries in the frontend_tuner_status struct sequence property
    # as well as the tuner_allocation_ids vector. Call this function during initialization
    def addChannels(self,num,tuner_type='RX_DIGITIZER'):
        for ii in range(num):
            tuner_status = self.frontend_tuner_status_struct_struct()
            tuner_status.tuner_type = tuner_type
            tuner_status.enabled = False
            self.frontend_tuner_status.append(tuner_status)
        self.allocation_id_mapping_lock.acquire()
        try:
            self.tuner_allocation_ids = []
        finally:
            self.allocation_id_mapping_lock.release()

    def allocateCapacity(self, capacities):
        return False

    def _checkValidIds(self, propdict):
        self._has_scanner = False
        for prop_key in propdict:
            if prop_key == "FRONTEND::scanner_allocation":
                raise CF.Device.InvalidCapacity("FRONTEND::scanner_allocation found in allocation; this is not a scanning device", [CF.DataType(id=prop_key,value=any.to_any(propdict[prop_key]))])

        for prop_key in propdict:
            if prop_key != "FRONTEND::tuner_allocation" and prop_key != "FRONTEND::listener_allocation":
                raise CF.Device.InvalidCapacity("UNKNOWN ALLOCATION PROPERTY "+prop_key, [CF.DataType(id=prop_key,value=any.to_any(propdict[prop_key]))])


    def allocateCapacity(self, properties):
        self._deviceLog.debug("allocateCapacity(%s)", properties)

        self._validateAllocProps(properties)

        propdict = {}
        for prop in properties:
            propdef = self._props.getPropDef(prop.id)
            propdict[prop.id] = propdef._fromAny(prop.value)

        self._checkValidIds(propdict)
        scanner_prop = None
        if propdict.has_key('FRONTEND::scanner_allocation'):
            scanner_prop = propdict['FRONTEND::scanner_allocation']

        if propdict.has_key('FRONTEND::tuner_allocation'):
            return self._allocate_frontend_tuner_allocation(propdict['FRONTEND::tuner_allocation'], scanner_prop)
        if propdict.has_key('FRONTEND::listener_allocation'):
            return self._allocate_frontend_listener_allocation(propdict['FRONTEND::listener_allocation'])

        raise CF.Device.InvalidCapacity("Unable to allocate this FEI device because FRONTEND::tuner_allocation and FRONTEND::listener_allocation not present", properties)

    def _allocate_frontend_tuner_allocation(self, frontend_tuner_allocation, scanner_prop = None):
        exception_raised = False
        try:
            # Check allocation_id
            if not frontend_tuner_allocation.allocation_id:
                self._deviceLog.info("allocate_frontend_tuner_allocation: MISSING ALLOCATION_ID")
                raise CF.Device.InvalidCapacity("MISSING ALLOCATION_ID", struct_to_props(frontend_tuner_allocation))

            # Check if allocation ID has already been used
            if  self.getTunerMapping(frontend_tuner_allocation.allocation_id) >= 0:
                self._deviceLog.info("allocate_frontend_tuner_allocation: ALLOCATION_ID "+frontend_tuner_allocation.allocation_id+" ALREADY IN USE")
                raise CF.Device.InvalidCapacity("ALLOCATION_ID "+frontend_tuner_allocation.allocation_id+" ALREADY IN USE", struct_to_props(frontend_tuner_allocation))

            self.allocation_id_mapping_lock.acquire()
            # Next, try to allocate a new tuner
            try:
                if len(self.tuner_allocation_ids) != len(self.frontend_tuner_status):
                    for idx in range(len(self.frontend_tuner_status)-len(self.tuner_allocation_ids)):
                        self.tuner_allocation_ids.append(tuner_allocation_ids_struct())
                for tuner_id in range(len(self.tuner_allocation_ids)):
                    if self.frontend_tuner_status[tuner_id].tuner_type != frontend_tuner_allocation.tuner_type:
                        self._deviceLog.debug("allocate_frontend_tuner_allocation: Requested tuner type '" + str(frontend_tuner_allocation.tuner_type) + "' does not match tuner[" + str(tuner_id) + "].tuner_type ("+str(self.frontend_tuner_status[tuner_id].tuner_type)+")")
                        continue

                    if len(frontend_tuner_allocation.group_id) > 0 and frontend_tuner_allocation.group_id != self.frontend_tuner_status[tuner_id].group_id:
                        self._deviceLog.debug("allocate_frontend_tuner_allocation: Requested group_id '"+str(frontend_tuner_allocation.group_id)+"' does not match tuner[" + str(tuner_id) + "].group_id ("+str(self.frontend_tuner_status[tuner_id].group_id)+")")
                        continue

                    # special case because allocation is specifying the input stream, which determines the rf_flow_id, etc.
                    if len(frontend_tuner_allocation.rf_flow_id) > 0 and \
                       frontend_tuner_allocation.rf_flow_id != self.frontend_tuner_status[tuner_id].rf_flow_id and \
                       frontend_tuner_allocation.tuner_type != "CHANNELIZER":
                        self._deviceLog.debug("allocate_frontend_tuner_allocation: Requested rf_flow_id '"+str(frontend_tuner_allocation.rf_flow_id)+"' does not match tuner[" +str(tuner_id)+ "].rf_flow_id ("+str(self.frontend_tuner_status[tuner_id].rf_flow_id)+")")
                        continue

                    if frontend_tuner_allocation.device_control:
                        if len(self.tuner_allocation_ids[tuner_id].control_allocation_id)>0:
                            self._deviceLog.debug("allocate_frontend_tuner_allocation: Tuner["+str(tuner_id)+"] is either not available")
                            continue
                        # device control
                        center_frequency = self.frontend_tuner_status[tuner_id].center_frequency
                        bandwidth = self.frontend_tuner_status[tuner_id].bandwidth
                        sample_rate = self.frontend_tuner_status[tuner_id].sample_rate
                        self.frontend_tuner_status[tuner_id].center_frequency = frontend_tuner_allocation.center_frequency
                        self.frontend_tuner_status[tuner_id].bandwidth = frontend_tuner_allocation.bandwidth
                        self.frontend_tuner_status[tuner_id].sample_rate = frontend_tuner_allocation.sample_rate
                        if self.supports_scan and scanner_prop:
                            if not self.deviceSetTuningScan(frontend_tuner_allocation, scanner_prop, self.frontend_tuner_status[tuner_id], tuner_id):
                                # either not available or didn't succeed setting tuning, try next tuner
                                self._deviceLog.debug("allocate_frontend_tuner_allocation: Tuner["+str(tuner_id)+"] didn't succeed while setting tuning ")
                                continue
                        else:
                            if not self.deviceSetTuning(frontend_tuner_allocation, self.frontend_tuner_status[tuner_id], tuner_id):
                                # either not available or didn't succeed setting tuning, try next tuner
                                self._deviceLog.debug("allocate_frontend_tuner_allocation: Tuner["+str(tuner_id)+"] didn't succeed while setting tuning ")
                                continue

                        if center_frequency == self.frontend_tuner_status[tuner_id].center_frequency and \
                           bandwidth == self.frontend_tuner_status[tuner_id].bandwidth and \
                           sample_rate == self.frontend_tuner_status[tuner_id].sample_rate:
                            self.frontend_tuner_status[tuner_id].center_frequency = frontend_tuner_allocation.center_frequency
                            self.frontend_tuner_status[tuner_id].bandwidth = frontend_tuner_allocation.bandwidth
                            self.frontend_tuner_status[tuner_id].sample_rate = frontend_tuner_allocation.sample_rate

                        self.tuner_allocation_ids[tuner_id].control_allocation_id = frontend_tuner_allocation.allocation_id
                        self.allocation_id_to_tuner_id[frontend_tuner_allocation.allocation_id] =  tuner_id
                        self.frontend_tuner_status[tuner_id].allocation_id_csv = self.createAllocationIdCsv(tuner_id)
                    else:
                        # channelizer allocations must specify device control = true
                        if frontend_tuner_allocation.tuner_type == "CHANNELIZER" or frontend_tuner_allocation.tuner_type == "TX":
                            eout = str(frontend_tuner_allocation.tuner_type) + " allocation with device_control=false is invalid."
                            self._deviceLog.debug(eout)
                            raise CF.Device.InvalidCapacity(eout, struct_to_props(frontend_tuner_allocation))
                        # listener
                        if len(self.tuner_allocation_ids[tuner_id].control_allocation_id) == 0 or not self.listenerRequestValidation(frontend_tuner_allocation, tuner_id):
                            # either not allocated or can't support listener request
                            self._deviceLog.debug("allocate_frontend_tuner_allocation: Tuner["+str(tuner_id)+"] is either not available or can not support listener request ")
                            continue
                        self.tuner_allocation_ids[tuner_id].listener_allocation_ids.append(frontend_tuner_allocation.allocation_id)
                        self.allocation_id_to_tuner_id[frontend_tuner_allocation.allocation_id] = tuner_id
                        self.frontend_tuner_status[tuner_id].allocation_id_csv = self.createAllocationIdCsv(tuner_id)
                        self.assignListener(frontend_tuner_allocation.allocation_id,self.tuner_allocation_ids[tuner_id].control_allocation_id)

                    # if we've reached here, we found an eligible tuner with correct frequency

                    # check tolerances
                    # only check when sample_rate was not set to don't care
                    self._deviceLog.debug(" allocate_frontend_tuner_allocation - SR requested: " + str(frontend_tuner_allocation.sample_rate) + "  SR got: " +str(self.frontend_tuner_status[tuner_id].sample_rate))
                    if  floatingPointCompare(frontend_tuner_allocation.sample_rate,0)!=0 and  \
                        (floatingPointCompare(self.frontend_tuner_status[tuner_id].sample_rate,frontend_tuner_allocation.sample_rate)<0 or
                         floatingPointCompare(self.frontend_tuner_status[tuner_id].sample_rate,frontend_tuner_allocation.sample_rate+frontend_tuner_allocation.sample_rate * frontend_tuner_allocation.sample_rate_tolerance/100.0)>0 ):
                        eout = "allocate_frontend_tuner_allocation(" + str(int(tuner_id)) +"): returned sr "+str(self.frontend_tuner_status[tuner_id].sample_rate)+" does not meet tolerance criteria of "+str(frontend_tuner_allocation.sample_rate_tolerance)+" percent"
                        self._deviceLog.info(eout)
                        raise allocationException(eout)

                    self._deviceLog.debug(" allocate_frontend_tuner_allocation - BW requested: " + str(frontend_tuner_allocation.bandwidth) + "  BW got: " +str(self.frontend_tuner_status[tuner_id].bandwidth))
                    # Only check when bandwidth was not set to don't care
                    if floatingPointCompare(frontend_tuner_allocation.bandwidth,0)!=0 and \
                        (floatingPointCompare(self.frontend_tuner_status[tuner_id].bandwidth,frontend_tuner_allocation.bandwidth)<0 or \
                        floatingPointCompare(self.frontend_tuner_status[tuner_id].bandwidth,frontend_tuner_allocation.bandwidth+frontend_tuner_allocation.bandwidth * frontend_tuner_allocation.bandwidth_tolerance/100.0)>0 ):
                        eout = "allocate_frontend_tuner_allocation("+str(int(tuner_id))+"): returned bw "+str(self.frontend_tuner_status[tuner_id].bandwidth)+" does not meet tolerance criteria of "+str(frontend_tuner_allocation.bandwidth_tolerance)+" percent"
                        self._deviceLog.info(eout)
                        raise allocationException(eout)

                    if frontend_tuner_allocation.device_control:
                        # enable tuner after successful allocation
                        self.enableTuner(tuner_id,True)

                    self._usageState = self.updateUsageState()
                    return True
            
            except allocationException as e:
                # if we made it here, we failed to find an available tuner
                self._deviceLog.info(e)
                raise
            except Exception as e:
                self._deviceLog.exception(e)
                raise allocationException(e)
            finally:
                self.allocation_id_mapping_lock.release()
                    
        except allocationException as e:
            tuner_id = self.getTunerMapping(frontend_tuner_allocation.allocation_id)
            if tuner_id >= 0:
                self.deallocate_frontend_tuner_allocation(frontend_tuner_allocation)
            return False

        except AllocationAlreadyExists, e:
            # Don't call deallocateCapacity if the allocationId already exists
            #   - Would end up deallocating a valid tuner/listener
            raise CF.Device.InvalidCapacity(e)
        
        except CF.Device.InvalidCapacity, e:
            raise e
        
        except FRONTEND.BadParameterException, e:
            #self.deallocateCapacity([frontend_tuner_allocation.getProp()])
            return False
        
        except Exception, e:
            self._deviceLog.exception('The following error occurred on allocation:',e)
            #self.deallocateCapacity([frontend_tuner_allocation.getProp()])
            raise e

        return False

    def deallocate_frontend_tuner_allocation(self, frontend_tuner_allocation):
        # Try to remove control of the device
        tuner_id = self.getTunerMapping(frontend_tuner_allocation.allocation_id)
        if tuner_id < 0:
            self._deviceLog.debug("deallocate_frontend_tuner_allocation: ALLOCATION_ID NOT FOUND: [" + str(frontend_tuner_allocation.allocation_id) + "]")
            raise CF.Device.InvalidCapacity("ALLOCATION_ID NOT FOUND: [" + str(frontend_tuner_allocation.allocation_id) + "]",struct_to_props(frontend_tuner_allocation))
        
        self.allocation_id_mapping_lock.acquire()
        try:
            if self.frontend_tuner_status[self.allocation_id_to_tuner_id[frontend_tuner_allocation.allocation_id]].allocation_id_csv.split(',')[0] == frontend_tuner_allocation.allocation_id:
                # remove if it is controlling
                while self.frontend_tuner_status[self.allocation_id_to_tuner_id[frontend_tuner_allocation.allocation_id]].allocation_id_csv != frontend_tuner_allocation.allocation_id:
                    split_id = self.frontend_tuner_status[self.allocation_id_to_tuner_id[frontend_tuner_allocation.allocation_id]].allocation_id_csv.split(',')
                    for idx in range(len(split_id)):
                        if split_id[idx] == frontend_tuner_allocation.allocation_id:
                            continue
                        else:
                            self.removeTunerMappingByAllocationId(split_id[idx])
                            self.removeListenerId(tuner_id, split_id[idx])
                        break
            else:
                # remove if it is not controlling
                split_id = self.frontend_tuner_status[self.allocation_id_to_tuner_id[frontend_tuner_allocation.allocation_id]].allocation_id_csv.split(',')
                for idx in range(len(split_id)):
                    if split_id[idx] != frontend_tuner_allocation.allocation_id:
                        continue
                    else:
                        self.removeTunerMappingByAllocationId(split_id[idx])
                        self.removeListenerId(tuner_id, split_id[idx])
                        return
        finally:
            self.allocation_id_mapping_lock.release()
        
        self.allocation_id_mapping_lock.acquire()
        try:
            if self.tuner_allocation_ids[tuner_id].control_allocation_id == frontend_tuner_allocation.allocation_id:
                self.enableTuner(tuner_id,False)
                self.removeTunerMappingByAllocationId(frontend_tuner_allocation.allocation_id)
                self.frontend_tuner_status[tuner_id].allocation_id_csv = ''
                self.removeTuner(tuner_id)
            else:
                self.removeTunerMappingByAllocationId(frontend_tuner_allocation.allocation_id)
                self.frontend_tuner_status[tuner_id].allocation_id_csv = ''
        finally:
            self.allocation_id_mapping_lock.release()
        
        self.frontend_tuner_status[tuner_id].allocation_id_csv = ''

    def _allocate_frontend_listener_allocation(self, frontend_listener_allocation):
        try:
            # Check validity of allocation_id's
            if not frontend_listener_allocation.existing_allocation_id:
                self._deviceLog.info("allocate_frontend_listener_allocation: MISSING EXISTING ALLOCATION ID")
                raise CF.Device.InvalidCapacity("MISSING EXISTING ALLOCATION ID", struct_to_props(frontend_listener_allocation))
            
            if not frontend_listener_allocation.listener_allocation_id:
                self._deviceLog.info("allocate_frontend_listener_allocation: MISSING LISTENER ALLOCATION ID")
                raise CF.Device.InvalidCapacity("MISSING LISTENER ALLOCATION ID", struct_to_props(frontend_listener_allocation))
            
            # Check if listener allocation ID has already been used
            if self.getTunerMapping(frontend_listener_allocation.listener_allocation_id) >= 0:
                self._deviceLog.info("allocate_frontend_listener_allocation: LISTENER ALLOCATION ID ALREADY IN USE: [" + str(frontend_listener_allocation.listener_allocation_id + "]"))
                raise AllocationAlreadyExists("LISTENER ALLOCATION ID ALREADY IN USE", frontend_listener_allocation)

            # Do not allocate if existing allocation ID does not exist
            tuner_id = self.getTunerMapping(frontend_listener_allocation.existing_allocation_id)
            if tuner_id < 0:
                self._deviceLog.debug("allocate_frontend_listener_allocation: UNKNOWN CONTROL ALLOCATION ID: [" + str(frontend_listener_allocation.existing_allocation_id)+"]")
                raise FRONTEND.BadParameterException("UNKNOWN CONTROL ALLOCATION ID")

            # listener allocations are not permitted for channelizers or TX
            if self.frontend_tuner_status[tuner_id].tuner_type == "CHANNELIZER" or self.frontend_tuner_status[tuner_id].tuner_type == "TX":
                eout = "allocate_frontend_listener_allocation: listener allocations are not permitted for " + str(self.frontend_tuner_status[tuner_id].tuner_type) + " tuner type"
                self._deviceLog.debug(eout)
                raise CF.Device.InvalidCapacity(eout, struct_to_props(frontend_listener_allocation))

            self.allocation_id_mapping_lock.acquire()
            try:
                self.tuner_allocation_ids[tuner_id].listener_allocation_ids.append(frontend_listener_allocation.listener_allocation_id)
                self.allocation_id_to_tuner_id[frontend_listener_allocation.listener_allocation_id] = tuner_id
            finally:
                self.allocation_id_mapping_lock.release()
            self.frontend_tuner_status[tuner_id].allocation_id_csv = self.createAllocationIdCsv(tuner_id)
            self.assignListener(frontend_listener_allocation.listener_allocation_id,frontend_listener_allocation.existing_allocation_id)
            return True
                
        except RuntimeError, e:
            return False

        except AllocationAlreadyExists, e:
            # Don't call deallocateCapacity if the allocationId already exists
            #   - Would end up deallocating a valid tuner/listener
            raise CF.Device.InvalidCapacity(str(e), struct_to_props(frontend_listener_allocation))
        
        except CF.Device.InvalidCapacity, e:
            raise e
        
        except FRONTEND.BadParameterException, e:
            return False
        
        except Exception, e:
            self._deviceLog.info('The following error occurred on allocation:',e)
            raise e

        return False

    def removeListenerId(self, tuner_id, allocation_id):
        split_id = self.frontend_tuner_status[tuner_id].allocation_id_csv.split(',')
        for idx in range(len(split_id)):
            if split_id[idx] == allocation_id:
                split_id.pop(idx)
                break
        self.frontend_tuner_status[tuner_id].allocation_id_csv = ','.join(split_id)
        
    def deallocate_frontend_listener_allocation(self, frontend_listener_allocation):
        tuner_id = self.getTunerMapping(frontend_listener_allocation.listener_allocation_id)
        if tuner_id < 0:
            self._deviceLog.debug("ALLOCATION_ID NOT FOUND: [" + str(frontend_listener_allocation.listener_allocation_id) + "]")
            retval_struct = [CF.DataType(id='FRONTEND::listener_allocation::existing_allocation_id',value=any.to_any(frontend_listener_allocation.existing_allocation_id)),CF.DataType(id='FRONTEND::listener_allocation::listener_allocation_id',value=any.to_any(frontend_listener_allocation.existing_allocation_id))]
            retval = CF.DataType(id='FRONTEND::listener_allocation',value=any.to_any(retval_struct))
            raise CF.Device.InvalidCapacity("ALLOCATION_ID NOT FOUND", [retval])
        if self.tuner_allocation_ids[tuner_id].control_allocation_id == frontend_listener_allocation.listener_allocation_id:
            self._deviceLog.debug("Controlling allocation id cannot be used as a listener id in a deallocation: [" + str(frontend_listener_allocation.listener_allocation_id) + "]")
            retval_struct = [CF.DataType(id='FRONTEND::listener_allocation::existing_allocation_id',value=any.to_any(frontend_listener_allocation.existing_allocation_id)),CF.DataType(id='FRONTEND::listener_allocation::listener_allocation_id',value=any.to_any(frontend_listener_allocation.existing_allocation_id))]
            retval = CF.DataType(id='FRONTEND::listener_allocation',value=any.to_any(retval_struct))
            raise CF.Device.InvalidCapacity("Controlling allocation id cannot be used as a listener id", [retval])
        # send EOS to listener connection only
        self.removeTunerMappingByAllocationId(frontend_listener_allocation.listener_allocation_id)
        self.removeListenerId(tuner_id, frontend_listener_allocation.listener_allocation_id)

    #*****************************************************************
    #* Tuner Configurations                                          *
    #*****************************************************************

    # Configure tuner - gets called during allocation
    def enableTuner(self, tuner_id, enable):
        prev_enabled = self.frontend_tuner_status[tuner_id].enabled
        # If going from disabled to enabled
        if enable:
            self.deviceEnable(self.frontend_tuner_status[tuner_id], tuner_id)

        # If going from enabled to disabled
        if not enable:
            self.deviceDisable(self.frontend_tuner_status[tuner_id], tuner_id)

        return True

    def listenerRequestValidation(self, request, tuner_id):
        self._deviceLog.trace("listenerRequestValidation(): request " + str(request) + " ,tuner_id " + str(tuner_id))

        # ensure requested values are non-negative
        if floatingPointCompare(request.center_frequency,0)<0 or \
            floatingPointCompare(request.bandwidth,0)<0 or \
            floatingPointCompare(request.sample_rate,0)<0 or \
            floatingPointCompare(request.bandwidth_tolerance,0)<0 or \
            floatingPointCompare(request.sample_rate_tolerance,0)<0:
            return False

        # ensure lower end of requested band fits
        if floatingPointCompare((request.center_frequency-(request.bandwidth*0.5)),(self.frontend_tuner_status[tuner_id].center_frequency-(self.frontend_tuner_status[tuner_id].bandwidth*0.5))) < 0:
            self._deviceLog.trace("listenerRequestValidation(): FAILED LOWER END TEST")
            return False

        # ensure upper end of requested band fits
        if floatingPointCompare((request.center_frequency + (request.bandwidth*0.5)),(self.frontend_tuner_status[tuner_id].center_frequency + (self.frontend_tuner_status[tuner_id].bandwidth*0.5))) > 0:
            self._deviceLog.trace("listenerRequestValidation(): FAILED UPPER END TEST")
            return False

        # ensure tuner bandwidth meets requested tolerance
        if floatingPointCompare(request.bandwidth,self.frontend_tuner_status[tuner_id].bandwidth) > 0:
            return False

        if floatingPointCompare(request.bandwidth,0)!=0 and floatingPointCompare((request.bandwidth+(request.bandwidth*request.bandwidth_tolerance/100)),self.frontend_tuner_status[tuner_id].bandwidth) < 0:
            return False

        # ensure tuner sample rate meets requested tolerance
        if floatingPointCompare(request.sample_rate,self.frontend_tuner_status[tuner_id].sample_rate) > 0:
            return False

        if floatingPointCompare(request.sample_rate,0)!=0 and floatingPointCompare((request.sample_rate+(request.sample_rate*request.sample_rate_tolerance/100)),self.frontend_tuner_status[tuner_id].sample_rate) < 0:
            return False

        return True

    #//////////////////////////
    #        MAPPING         //
    #//////////////////////////

    def getTunerMapping(self, _allocation_id):
        NO_VALID_TUNER = -1
        self.allocation_id_mapping_lock.acquire()
        try:
            for key in self.allocation_id_to_tuner_id:
                if key == _allocation_id:
                    return self.allocation_id_to_tuner_id[_allocation_id]
        finally:
            self.allocation_id_mapping_lock.release()
        return NO_VALID_TUNER

    def sendEOS(self, allocation_id):
        ports = self.getPortSet()
        for port in ports:
            repid = str(port.repid)
            if 'BULKIO' in repid:
                prt = port.obj_ptr._narrow(ExtendedCF.QueryablePort)
                if prt is None:
                    continue
                try:
                    prt.disconnectPort(allocation_id)
                except:
                    continue
                _connections = prt._get_connections()
                for _connection in _connections:
                    if _connection.connectionId == allocation_id:
                        prt.connectPort(_connection.port, allocation_id)
                        break
        
    def removeTunerMappingByAllocationId(self, allocation_id):
        self._deviceLog.trace("removeTunerMapping(allocation_id) allocation_id " + str(allocation_id))
        self.allocation_id_mapping_lock.acquire()
        try:
            if self.frontend_tuner_status[self.allocation_id_to_tuner_id[allocation_id]].allocation_id_csv.split(',')[0] == allocation_id:
                self.deviceDeleteTuning(self.frontend_tuner_status[self.allocation_id_to_tuner_id[allocation_id]], self.allocation_id_to_tuner_id[allocation_id])
            self.removeListener(allocation_id)
            self.sendEOS(allocation_id)
            if allocation_id in self.allocation_id_to_tuner_id:
                del self.allocation_id_to_tuner_id[allocation_id]
                return True
            return False
        finally:
            self.allocation_id_mapping_lock.release()

    def removeTuner(self, tuner_id):
        self.enableTuner(tuner_id, False)
        self.allocation_id_mapping_lock.acquire()
        try:
            self.tuner_allocation_ids[tuner_id].reset()
        finally:
            self.allocation_id_mapping_lock.release()
        return True

    def assignListener(self, listen_alloc_id, alloc_id):
        # this function is specialized in the generated code
        pass

    def removeListener(self, listen_alloc_id):
        # this function is specialized in the generated code
        pass

    ############################
    ## Other helper functions ##
    ############################

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
            print "\t KEYWORD KEY/VAL ::",  keywords.id + ":",  any.from_any(keywords.value)
            
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
                                  kinds=("allocation","property"),
                                  description="""This specifies the device kind"""
                                  )
    device_model = simple_property(id_="DCE:0f99b2e4-9903-4631-9846-ff349d18ecfb",
                                   name="device_model",
                                   type_="string",
                                   defvalue="",
                                   mode="readonly",
                                   action="eq",
                                   kinds=("allocation","property"),
                                   description=""" This specifies the specific device"""
                                   )
    frontend_tuner_allocation = struct_property(id_="FRONTEND::tuner_allocation",
                                                name="frontend_tuner_allocation",
                                                structdef=frontend_tuner_allocation,
                                                configurationkind=("allocation",),
                                                mode="writeonly",
                                                description="""Frontend Interfaces v2.0 main allocation structure"""
                                                )
    frontend_listener_allocation = struct_property(id_="FRONTEND::listener_allocation",
                                                   name="frontend_listener_allocation",
                                                   structdef=frontend_listener_allocation,
                                                   configurationkind=("allocation",),
                                                   mode="writeonly",
                                                   description="""Allocates a listener (subscriber) based off a previous allocation """
                                                   )
    frontend_tuner_status = structseq_property(id_="FRONTEND::tuner_status",
                                               name="frontend_tuner_status",
                                               structdef=default_frontend_tuner_status_struct_struct,
                                               defvalue=[],
                                               configurationkind=("property",),
                                               mode="readonly",
                                               description="""Frontend Interfaces v2.0 status structure. One element for every frontend resource (receiver, transmitter) configured on this hardware"""
                                               )


class FrontendScannerDevice(FrontendTunerDevice):

    def __init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams):
        FrontendTunerDevice.__init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams)
        self.supports_scan = True

    frontend_scanner_allocation = struct_property(id_="FRONTEND::scanner_allocation",
                                                name="frontend_scanner_allocation",
                                                structdef=frontend_scanner_allocation,
                                                configurationkind=("allocation",),
                                                mode="writeonly",
                                                description="""Frontend Interfaces v2.0 scanner allocation structure"""
                                                )

    def _checkValidIds(self, propdict):
        self._has_scanner = False
        for prop_key in propdict:
            if prop_key != "FRONTEND::tuner_allocation" and prop_key != "FRONTEND::listener_allocation" and prop_key != "FRONTEND::scanner_allocation":
                raise CF.Device.InvalidCapacity("UNKNOWN ALLOCATION PROPERTY "+prop_key, [CF.DataType(id=prop_key,value=any.to_any(propdict[prop_key]))])
            if prop_key == "FRONTEND::scanner_allocation":
                self._has_scanner = True

