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

import bulkio
from ossie.cf import CF
from ossie.properties import simple_property
from omniORB import any as _any

# Time Type Definition
J1950 = 1
J1970 = 2
JCY = 3
    
class tuner_allocation_ids_struct(object):
    def __init__(self):
        self.reset()

    def reset(self):
        self.control_allocation_id = ''
        self.listener_allocation_ids = []
        

class FreqRange(object):
    def __init__(self,min_val=0.0,max_val=0.0, values=[]):
        self.min_val = min_val
        self.max_val = max_val
        self.values = values

class AntennaInfo(object):
    def __init__(self,name="",type="",size="",description=""):
        self.name = name
        self.type = type
        self.size = size
        self.description = description

class FeedInfo(object):
    def __init__(self,name="", polarization="",freq_range=FreqRange()):
        self.name = name
        self.polarization = polarization
        self.freq_range = freq_range

class SensorInfo(object):
    def __init__(self,mission="",collector="",rx="",antenna=AntennaInfo(),feed=FeedInfo()):
        self.mission = mission
        self.collector = collector
        self.rx = rx
        self.antenna = antenna
        self.feed = feed

class PathDelay(object):
    def __init__(self,freq=0.0,delay_ns=0.0):
        self.freq = freq
        self.delay_ns = delay_ns

class RFCapabilities(object):
    def __init__(self,freq_range=FreqRange(),bw_range=FreqRange()):
        self.freq_range = freq_range
        self.bw_range = bw_range

class RFInfoPkt(object):
    def __init__(self,rf_flow_id="",rf_center_freq=0.0,rf_bandwidth=0.0,if_center_freq=0.0,spectrum_inverted=False,sensor=SensorInfo(),ext_path_delays=[],capabilities=[],additional_info=[]):
        self.rf_flow_id = rf_flow_id
        self.rf_center_freq = rf_center_freq
        self.rf_bandwidth = rf_bandwidth
        self.if_center_freq = if_center_freq
        self.spectrum_inverted = spectrum_inverted
        self.sensor = sensor
        self.ext_path_delays = ext_path_delays
        self.capabilities = capabilities
        self.additional_info = additional_info
    
class PositionInfo(object):
    def __init__(self,valid=False,datum="",lat=0.0,lon=0.0,alt=0.0):
        self.valid = valid
        self.datum = datum
        self.lat = lat
        self.lon = lon
        self.alt = alt

class GPSInfo(object):
    def __init__(self,source_id="",rf_flow_id="",mode="",fom=0,tfom=0,dataID=0,time_offset=0.0,freq_offset=0.0,time_variance=0.0,freq_variance=0.0,satellite_count=0,snr=0.0,status_message="",timestamp=bulkio.timestamp.create(),additional_info=[]):
        self.source_id = source_id
        self.rf_flow_id = rf_flow_id
        self.mode = mode
        self.fom = fom
        self.tfom = tfom
        self.datumID = datumID
        self.time_offset = time_offset
        self.freq_offset = freq_offset
        self.time_variance = time_variance
        self.freq_variance = freq_variance
        self.satellite_count = satellite_count
        self.snr = snr
        self.status_message = status_message
        self.timestamp = timestamp
        self.additional_info = additional_info

class GpsTimePos(object):
    def __init__(self,position=PositionInfo(),timestamp=bulkio.timestamp.create()):
        self.position = position
        self.timestamp = timestamp

class CartesianPositionInfo(object):
    def __init__(self,valid=False,datum="",x=0.0,y=0.0,z=0.0):
        self.valid = valid
        self.datum = datum
        self.x = x 
        self.y = y
        self.z = z

class AttitudeInfo(object):
    def __init__(self,valid=False,pitch=0.0,yaw=0.0,roll=0.0):
        self.valid = valid
        self.pitch = pitch
        self.yaw = yaw
        self.roll = roll

class VelocityInfo(object):
    def __init__(self,valid=False,datum="",coordinate_system="",x=0.0,y=0.0,z=0.0):
        self.valid = valid
        self.datum = datum
        self.coordinate_system = coordinate_system
        self.x = x 
        self.y = y
        self.z = z

class AccelerationInfo(object):
    def __init__(self,valid=False,datum="",coordinate_system="",x=0.0,y=0.0,z=0.0):
        self.valid = valid
        self.datum = datum
        self.coordinate_system = coordinate_system
        self.x = x
        self.y = y
        self.z = z

class NavigationPacket(object):
    def __init__(self,source_id="",rf_flow_id="",position=PositionInfo(),cposition=CartesianPositionInfo(),velocity=VelocityInfo(),acceleration=AccelerationInfo(),attitude=AttitudeInfo(),timestamp=bulkio.timestamp.create(),additional_info=[]):
        self.source_id = source_id
        self.rf_flow_id = rf_flow_id
        self.position = position
        self.cposition = cposition
        self.velocity = velocity
        self.acceleration = acceleration
        self.attitude = attitude
        self.timestamp = timestamp
        self.additional_info = additional_info


class frontend_tuner_allocation(object):
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
    
    def getProp(self):
        content = []
        for member in self.getMembers():
            content.append(CF.DataType(id=member[0],value=_any.to_any(member[1])))
        retval = CF.DataType(id=self.getId(),value=_any.to_any(content))
        return retval
                                                            
    def getMembers(self):
        return [("FRONTEND::tuner_allocation::tuner_type",self.tuner_type),("FRONTEND::tuner_allocation::allocation_id",self.allocation_id),("FRONTEND::tuner_allocation::center_frequency",self.center_frequency),("FRONTEND::tuner_allocation::bandwidth",self.bandwidth),("FRONTEND::tuner_allocation::bandwidth_tolerance",self.bandwidth_tolerance),("FRONTEND::tuner_allocation::sample_rate",self.sample_rate),("FRONTEND::tuner_allocation::sample_rate_tolerance",self.sample_rate_tolerance),("FRONTEND::tuner_allocation::device_control",self.device_control),("FRONTEND::tuner_allocation::group_id",self.group_id),("FRONTEND::tuner_allocation::rf_flow_id",self.rf_flow_id)]

class frontend_listener_allocation(object):
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
    
    def getProp(self):
        content = []
        for member in self.getMembers():
            content.append(CF.DataType(id=member[0],value=_any.to_any(member[1])))
        retval = CF.DataType(id=self.getId(),value=_any.to_any(content))
        return retval

    def getMembers(self):
        return [("FRONTEND::listener_allocation::existing_allocation_id",self.existing_allocation_id),("FRONTEND::listener_allocation::listener_allocation_id",self.listener_allocation_id)]

class default_frontend_tuner_status_struct_struct(object):
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
    
    def getProp(self):
        content = []
        for member in self.getMembers():
            content.append(CF.DataType(id=member[0],value=_any.to_any(member[1])))
        retval = CF.DataType(id=self.getId(),value=_any.to_any(content))
        return retval

    def getMembers(self):
        return [("FRONTEND::tuner_status::tuner_type",self.tuner_type),("FRONTEND::tuner_status::allocation_id_csv",self.allocation_id_csv),("FRONTEND::tuner_status::center_frequency",self.center_frequency),("FRONTEND::tuner_status::bandwidth",self.bandwidth),("FRONTEND::tuner_status::sample_rate",self.sample_rate),("FRONTEND::tuner_status::group_id",self.group_id),("FRONTEND::tuner_status::rf_flow_id",self.rf_flow_id),("FRONTEND::tuner_status::enabled",self.enabled)]
