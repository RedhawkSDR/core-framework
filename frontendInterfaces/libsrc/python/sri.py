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
from omniORB import CORBA
from omniORB import any as _any
from ossie.cf import CF
from bulkio.bulkioInterfaces import BULKIO

def create(stream_id, frontend_status, identifier = 'my_id', collector_frequency = -1.0 ):

    _sri = BULKIO.StreamSRI(hversion=1, xstart=0.0, xdelta=1.0, 
                              xunits=BULKIO.UNITS_TIME, subsize=0, ystart=0.0, ydelta=0.0, 
                              yunits=BULKIO.UNITS_NONE, mode=0, streamID=stream_id, blocking=False, keywords=[])
                              
    if frontend_status.sample_rate <= 0.0:
        _sri.xdelta =  1.0
    else:
        _sri.xdelta = 1/frontend_status.sample_rate
    if collector_frequency < 0:
        colFreq = frontend_status.center_frequency
    else:
        colFreq = float(collector_frequency)
    _col_rf = CF.DataType(id='COL_RF', value=_any.to_any(colFreq))
    _col_rf.value._t = CORBA.TC_double
    _sri.keywords.append(_col_rf)
    _chan_rf = CF.DataType(id='CHAN_RF', value=_any.to_any(frontend_status.center_frequency))
    _chan_rf.value._t = CORBA.TC_double
    _sri.keywords.append(_chan_rf)
    _rf_flow_id = CF.DataType(id='FRONTEND::RF_FLOW_ID', value=_any.to_any(frontend_status.rf_flow_id))
    _sri.keywords.append(_rf_flow_id)
    _bw_rf = CF.DataType(id='FRONTEND::BANDWIDTH', value=_any.to_any(frontend_status.bandwidth))
    _bw_rf.value._t = CORBA.TC_double
    _sri.keywords.append(_bw_rf)
    _dev_id = CF.DataType(id='FRONTEND::DEVICE_ID', value=_any.to_any(identifier))
    _sri.keywords.append(_dev_id)
    return _sri
