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

from ossie.cf import CF__POA
from ossie import properties

__all__ = ('PropertyChangeListener',)

class PropertyChangeListener(CF__POA.PropertyChangeListener):
    '''
      Container to process property change events
      
      Example usage:
        def my_property_change_callback(event_id, registration_id, resource_id, properties, timestamp):
          print event_id, registration_id, resource_id, properties, timestamp
            
        myl = sb.PropertyChangeListener(defaultCallback=my_property_change_callback)
        regid=comp.registerPropertyListener( myl, ['prop1'], float(0.5))
          
      Note:
        - If no callbacks are provided, the contents of the event are directed to stdout
        - Multiple properties associated with the same callback are triggered together when receive in the same event
    '''
    def __init__(self, defaultCallback = None, changeCallbacks = {}):
        '''
          defaultCallback is triggered when no callback matches
          changeCallbacks is a dictionary of callbacks:
            key: id (string) to trigger the callback
            value: callback function
        '''
        self._changeCallbacks = changeCallbacks
        self._defaultCallback = defaultCallback

    def __del__(self):
        pass

    def updateCallback(self, _prop_id, _callback=None):
        '''
         _prop_id is the property id (string)
         _callback is a function of the form:
          fn(event_id, registration_id, resource_id, properties, timestamp)
        '''
        if type(_prop_id) != str:
            raise Exception('Invalid property id. It must be a strings')
        if _callback == None:
            if not self._changeCallbacks.has_key(_prop_id):
                raise Exception('Invalid key:', _prop_id)
            self._changeCallbacks.pop(_prop_id)
            return
        self._changeCallbacks.update({_prop_id:_callback})
    
    def getCallback(self, _prop_id=''):
        '''
         Set _prop_id for the unique key. Use an empty list to return all callbacks
        '''
        if _prop_id == []:
            return self._changeCallbacks
        if self._changeCallbacks.has_key(_prop_id):
            return self._changeCallbacks[_prop_id]
        raise Exception('Invalid property id. No callback registered under that id')
        
    def propertyChange(self, _propChEv) :
        if type(self._changeCallbacks) != dict:
            print 'Invalid change callbacks (must be dictionary with property id as a key and a callback function as a value). Printing received event', _propChEv

        _tmp_props = _propChEv.properties
        _triggers = {}
        
        for _prop_key in self._changeCallbacks:
            for _prop_idx in range(len(_tmp_props)):
                if _tmp_props[_prop_idx].id == _prop_key:
                    if not _triggers.has_key(self._changeCallbacks[_prop_key]):
                        _triggers[self._changeCallbacks[_prop_key]] = {}
                    _triggers[self._changeCallbacks[_prop_key]].update(properties.prop_to_dict(_tmp_props[_prop_idx]))
                    _tmp_props.pop(_prop_idx)
                    break
                    
        if len(_triggers) != 0:
            for _trigger in _triggers:
                _trigger(_propChEv.evt_id, _propChEv.reg_id, _propChEv.resource_id, _triggers[_trigger], _propChEv.timestamp)

        if len(_tmp_props) != 0:
            if self._defaultCallback:
                self._defaultCallback(_propChEv.evt_id, _propChEv.reg_id, _propChEv.resource_id, properties.props_to_dict(_tmp_props), _propChEv.timestamp)
                return
            print 'Property Change Event:'
            print ' event id:',_propChEv.evt_id
            print ' registration id:',_propChEv.reg_id
            print ' resource id:', _propChEv.resource_id
            print ' properties:', properties.props_to_dict(_tmp_props)
            print ' timestamp:', _propChEv.timestamp

