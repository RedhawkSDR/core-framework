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

import logging

from ossie.cf import StandardEvent, CF
from ossie import events
from ossie.utils.uuid import uuid4
from ossie.utils.notify import notification
from ossie.utils import weakobj

log = logging.getLogger(__name__)

class ChannelListener(object):
    def __del__(self):
        try:
            self.disconnect()
        except:
            pass
        try:
            poa = self.__consumer._default_POA()
            poa.deactivate_object(poa.servant_to_id(self.__consumer))
        except:
            pass

    def push(self, event, typecode):
        pass

    def connect(self, domain):
        log.trace('Connecting %s', self.CHANNEL_NAME)
        # Generate a unique identifier for each event channel connection to prevent
        # collisions if multiple instances are running.
        self.__identifier = str(uuid4())
        self.__consumer = events.GenericEventConsumer(weakobj.boundmethod(self.push), keep_structs=True)
        self.__domainManager = domain
        self.__domainManager.registerWithEventChannel(self.__consumer._this(), self.__identifier, self.CHANNEL_NAME)

    def disconnect(self):
        log.trace('Disconnecting %s', self.CHANNEL_NAME)
        self.__domainManager.unregisterFromEventChannel(self.__identifier, self.CHANNEL_NAME)

class ODMListener(ChannelListener):
    CHANNEL_NAME = 'ODM_Channel'

    @notification
    def deviceManagerAdded(self, event):
        """
        A device manager was added.
        """
        log.trace('deviceManagerAdded %s', event)

    @notification
    def deviceManagerRemoved(self, event):
        """
        A device manager was removed.
        """
        log.trace('deviceManagerRemoved %s', event)

    @notification
    def deviceAdded(self, event):
        """
        A device was added.
        """
        log.trace('deviceAdded %s', event)

    @notification
    def deviceRemoved(self, event):
        """
        A device was removed.
        """
        log.trace('deviceRemoved %s', event)

    @notification
    def applicationAdded(self, event):
        """
        An application was added.
        """
        log.trace('applicationAdded %s', event)

    @notification
    def applicationRemoved(self, event):
        """
        An application was removed.
        """
        log.trace('applicationRemoved %s', event)

    @notification
    def applicationFactoryAdded(self, event):
        """
        An application factory was added.
        """
        log.trace('applicationFactoryAdded %s', event)

    @notification
    def applicationFactoryRemoved(self, event):
        """
        An application factory was removed.
        """
        log.trace('applicationFactoryRemoved %s', event)

    @notification
    def serviceAdded(self, event):
        """
        A service was added.
        """
        log.trace('serviceAdded %s', event)

    @notification
    def serviceRemoved(self, event):
        """
        A service was removed.
        """
        log.trace('serviceRemoved %s', event)

    def __init__(self):
        self.__handlers = {
            StandardEvent._tc_DomainManagementObjectAddedEventType: {
                StandardEvent.DEVICE_MANAGER: ODMListener.deviceManagerAdded,
                StandardEvent.DEVICE: ODMListener.deviceAdded,
                StandardEvent.APPLICATION_FACTORY: ODMListener.applicationFactoryAdded,
                StandardEvent.APPLICATION: ODMListener.applicationAdded,
                StandardEvent.SERVICE: ODMListener.serviceAdded
            },
            StandardEvent._tc_DomainManagementObjectRemovedEventType: {
                StandardEvent.DEVICE_MANAGER: ODMListener.deviceManagerRemoved,
                StandardEvent.DEVICE: ODMListener.deviceRemoved,
                StandardEvent.APPLICATION_FACTORY: ODMListener.applicationFactoryRemoved,
                StandardEvent.APPLICATION: ODMListener.applicationRemoved,
                StandardEvent.SERVICE: ODMListener.serviceRemoved
            },
        }

    def push(self, event, typecode):
        try:
            self.__handlers[typecode][event.sourceCategory](self, event)
        except KeyError:
            log.debug('Unhandled ODM message %s', event)


class IDMListener(ChannelListener):
    CHANNEL_NAME = 'IDM_Channel'

    # Mapping of event states to device states
    EVENT_MAP = { StandardEvent.IDLE          : CF.Device.IDLE,
                  StandardEvent.ACTIVE        : CF.Device.ACTIVE,
                  StandardEvent.BUSY          : CF.Device.BUSY,
                  StandardEvent.LOCKED        : CF.Device.LOCKED,
                  StandardEvent.UNLOCKED      : CF.Device.UNLOCKED,
                  StandardEvent.SHUTTING_DOWN : CF.Device.SHUTTING_DOWN }

    @notification
    def administrativeStateChanged(self, deviceId, stateChangeFrom, stateChangeTo):
        """
        The administrative state of a device changed.
        """
        log.trace('administrativeStateChanged %s %s->%s', deviceId, stateChangeFrom, stateChangeTo)

    @notification
    def operationalStateChanged(self, deviceId, stateChangeFrom, stateChangeTo):
        """
        The operational state of a device changed.
        """
        log.trace('operationalStateChanged %s %s->%s', deviceId, stateChangeFrom, stateChangeTo)

    @notification
    def usageStateChanged(self, deviceId, stateChangeFrom, stateChangeTo):
        """
        The usage state of a device changed.
        """
        log.trace('usageStateChanged %s %s->%s', deviceId, stateChangeFrom, stateChangeTo)

    def __init__(self):
        self.__handlers = {
            StandardEvent.ADMINISTRATIVE_STATE_EVENT: IDMListener.administrativeStateChanged,
            StandardEvent.OPERATIONAL_STATE_EVENT: IDMListener.operationalStateChanged,
            StandardEvent.USAGE_STATE_EVENT: IDMListener.usageStateChanged,
        }

    def push(self, event, typecode):
        if typecode != StandardEvent._tc_StateChangeEventType:
            log.debug('Unhandled IDM message %s', event)
            return
        deviceId = event.sourceId
        stateChangeFrom = IDMListener.EVENT_MAP[event.stateChangeFrom]
        stateChangeTo = IDMListener.EVENT_MAP[event.stateChangeTo]
        self.__handlers[event.stateChangeCategory](self, deviceId, stateChangeFrom, stateChangeTo)
