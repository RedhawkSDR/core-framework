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

from ossie.cf import StandardEvent

class DomainEventWriter:
    def __init__(self, publisher ):
        self.pub = publisher


    def __del__(self):
        self.pub = None


    def sendStateChangeEvent( self, prod, source, category, stateFrom, stateTo ):
        obj_evt = StandardEvent.StateChangeEventType( prod, source, category, stateFrom, stateTo )
        return self.sendStateChangeEventObject(obj_evt)

    def sendStateChangeEventObject( self, stateEventObject ):

        retval=-1
        if not isinstance( stateEvenObject, StandardEvent.StateChangeEventType ):
           return retval

        try:
            obj_evt = any.to_any( stateEventObject )
            if self.pub:
                self.pub.push(obj_evt) 
                
            retval=0
        except:
            pass

        return retval


    def sendAddedEvent( self, prod, source, source_name, obj, category ):
        obj_evt = StandardEvent.DomainManagementObjectAddedEventType( prod, source, source_name, category, obj )
        return  self.sendAddedEventObject(obj_evt)

    def sendAddedEventObject( self, addedEventObject ):

        retval=-1
        if not isinstance( addedEvenObject,  StandardEvent.DomainManagementObjectAddedEventType):
           return retval

        try:
            obj_evt = any.to_any( addedEventObject )
            if self.pub:
                self.pub.push(obj_evt) 
                
            retval=0
        except:
            pass


        return retval


    def sendRemovedEvent( self, prod, source, source_name, category ):
        obj_evt = StandardEvent.DomainManagementObjectRemovedEventType( prod, source, source_name, category )
        return  self.sendRemovedEventObject(obj_evt)

    def sendRemovedEventObject( self, removedEventObject ):
        retval=-1
        if not isinstance( removedEvenObject,  StandardEvent.DomainManagementObjectRemovedEventType ):
           return retval

        try:
            obj_evt = any.to_any( removedEventObject )
            if self.pub:
                self.pub.push(obj_evt) 
                
            retval=0
        except:
            pass

        return retval
