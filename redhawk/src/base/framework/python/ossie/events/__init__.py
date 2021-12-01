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

import sys
import time as _time
import threading
import queue
import copy
import logging
import atexit

from omniORB import any, URI, CORBA
from ossie.cf import CF, CF__POA
from ossie.cf import ExtendedCF, ExtendedCF__POA
from ossie.cf import ExtendedEvent, ExtendedEvent__POA
from ossie.properties import struct_from_any, struct_to_any, props_to_any, prop_to_dict
from ossie.properties import simple_property, simpleseq_property, struct_property, structseq_property

import traceback
import CosEventComm__POA
import CosEventChannelAdmin, CosEventChannelAdmin__POA
import CosNaming
import CosLifeCycle

from .DomainEventWriter import *
from .Publisher import *
from .Subscriber import *
from .Manager import *


##
## Module constants for use when looking up well known channels
##
IDM_Channel_Spec="IDM_Channel"
ODM_Channel_Spec="ODM_Channel"

class PropertyEventSupplier(CF__POA.Port):
        
    def __init__(self, component, filter=None):
        self._component = component
        self._component._props.setPropertyChangeEvent(self.sendPropertyEvent)
        self._outPorts = {}
        self._proxy_consumer = None
        self._last_property_event_state = {}

    def sendEvent(self, event):
            self._component._log.debug("sendEvent %s %s", event, self._outPorts)
            for connectionId, connection in list(self._outPorts.items()):
                self._component._log.debug("Sending event to '%s'", connectionId)
                if 'proxy_consumer' not in connection:
                    continue
                try:
                    connection['proxy_consumer'].push(event)
                except:
                    self._component._log.warn("Unable to send PropertySetChangeEventType to '%s': %s", connectionId, sys.exc_info()[0])

    def sendPropertyEvent(self, id):
        self.sendPropertiesEvent((id,))

    def sendPropertiesEvent(self, ids=None):
        # Avoid marshalling overhead if there are no connections
        if not self._outPorts:
            self._component._log.debug("Skipping sendPropertiesEvent (no connections)")
            return

        if ids is None:
            ids = [prop.id_ for prop in list(self._component._props.values()) if prop.isSendEventChange()]
        self._component._log.debug("sendPropertiesEvent %s", ids)
        
        properties = []
        for id_ in ids:
            propDef = self._component._props.getPropDef(id_)
            if not propDef.isSendEventChange():
                self._component._log.warning("Send property event for non-eventable property %s", id_)
            v = propDef._toAny(propDef.get(self._component))
            properties.append(CF.DataType(id_, v))

        if len(properties) == 0:
            self._component._log.debug("Refusing to send empty property event")
        else:
            event = any.to_any(ExtendedEvent.PropertySetChangeEventType(self._component._id, self._component._name, properties))
            self.sendEvent(event)

    def sendChangedPropertiesEvent(self):
        eventable_ids = []
        for prop_id in list(self._component._props.keys()):
            prop_def = self._component._props.getPropDef(prop_id)
            if prop_def.isSendEventChange():
                newValue = self._component._props[prop_id]
                try:
                    oldValue = self._last_property_event_state[prop_id]
                    self._last_property_event_state[prop_id] = copy.deepcopy(newValue)
                except KeyError:
                    self._component._log.debug("Issuing event for the first time %s", prop_id)
                    self._last_property_event_state[prop_id] = copy.deepcopy(newValue)
                    eventable_ids.append(prop_id)
                else:
                    if prop_def.compareValues(oldValue, newValue):
                        self._component._log.debug("Issuing event for %s (%s != %s)", prop_id, oldValue, newValue)
                        eventable_ids.append(prop_id)
        self._component._log.debug("Eventing for properties %s", eventable_ids)
        self.sendPropertiesEvent(eventable_ids)

    def connectPort (self, connection, connectionId):
        try:
            channel = connection._narrow(CosEventChannelAdmin.EventChannel)
        except:
            self._component._log.warn("Could not narrow channel object: %s", sys.exc_info()[0])
            return
        self._outPorts[str(connectionId)] = self._connectSupplierToEventChannel(channel)

    def disconnectPort (self, connectionId):
        if str(connectionId) in self._outPorts:
            try:
                consumer = self._outPorts[connectionId]['proxy_consumer']
                consumer.disconnect_push_consumer()
            except:
                pass
            del self._outPorts[connectionId]
    
    def _connectSupplierToEventChannel(self, channel):
        connection = { 'port': channel }

        try:
            supplier_admin = channel.for_suppliers()
            proxy_consumer = supplier_admin.obtain_push_consumer()
            # Connect to the push supplier without a Supplier object; we will not
            # receive notification if we are disconnected.
            proxy_consumer.connect_push_supplier(None)
            connection['proxy_consumer'] = proxy_consumer
        except:
            self._component._log.warn("Failed to connect to event channel")

        return connection


class GenericEventConsumer(CosEventComm__POA.PushConsumer):
    """A generic event consumer that passes all events to
    the provided target.  The filter, if provided will only
    pass events if the received event matches one of the
    type codes in the filter list
    """

    def __init__(self, on_push, on_disconnect=None, filter=None, keep_structs=False):
        self.__on_push = on_push
        self.__on_disconnect = on_disconnect
        self.__filter = filter
        self.__keep_structs = keep_structs

    def push(self, data):
        try:
            if self.__filter == None or data.typecode() in self.__filter:
                event = any.from_any(data, keep_structs=self.__keep_structs)
                self.__on_push(event, data.typecode())
        except:
            traceback.print_exc()

    def disconnect_push_consumer(self):
        if self.__on_disconnect != None:
            self.__on_disconnect()

class ChannelManager:
    """This class provides the ability to manage event channels from Python"""

    def __init__(self, orb, factoryName="EventChannelFactory"):
        # get access to an ORB reference
        self.orb = orb
        self.factoryName = factoryName
        # get access to the naming service's root context
        obj = self.orb.resolve_initial_references("NameService")
        self.rootContext = obj._narrow(CosNaming.NamingContext)

    def _getEventChannelName(self, eventChannelName, domainName=None):
        if domainName:
            return domainName + '/' + eventChannelName
        return eventChannelName

    def getEventChannelFactory(self):
        """
        Get the EventChannelFactory for this ORB. An initial reference of
        'EventService' is checked first, then the NameService is queried
        for the factory name. Returns None if one cannot be located.
        """
        factory=None
        try:
            factory = self.orb.resolve_initial_references("EventService")
        except:
            factory = None

        if not factory:
            eventFactoryName = URI.stringToName(self.factoryName)
            try:
                factory = self.rootContext.resolve(eventFactoryName)
            except:
                return None

        if factory:
            return factory._narrow(CosLifeCycle.GenericFactory)
        
        return None

    def createEventChannel(self, eventChannelName, domainName=None, force=False):
        """
        Creates an event channel, if it already exists return it. The
        event channel is bound into the name service as 'eventChannelName',
        in the context specified by 'domainName', or the root naming context
        if 'domainName' is not given.

        If force == True and the event channel already exists, destroy it and create a new one.
        """
        eventChannel = self.getEventChannel(eventChannelName, domainName)
        if eventChannel:
            if force or eventChannel._non_existent():
                self.destroyEventChannel(eventChannelName, domainName)
            else:
                return eventChannel._narrow(CosEventChannelAdmin.EventChannel)

        factory = self.getEventChannelFactory()
        if not factory:
            return None

        name = self._getEventChannelName(eventChannelName, domainName)
        key = [CosNaming.NameComponent("EventChannel","object interface")]
        if domainName != None:
            criteria = [CosLifeCycle.NameValuePair(name="InsName",value=any.to_any(domainName+'.'+eventChannelName))]
        else:
            criteria = [CosLifeCycle.NameValuePair(name="InsName",value=any.to_any(eventChannelName))]

        try:
            eventChannel = factory.create_object(key,criteria)._narrow(CosEventChannelAdmin.EventChannel)
        except:
            return None

        bindingName = URI.stringToName(name)
        self.rootContext.rebind(bindingName, eventChannel)

        return eventChannel

    def getEventChannel(self, eventChannelName, domainName=None):
        """
        Returns a reference to an existing event channel, or None if it
        cannot be located.
        """
        nameBinding = URI.stringToName(self._getEventChannelName(eventChannelName, domainName))
        try:
            channel = self.rootContext.resolve(nameBinding)
        except:
            return None

        try:
            if channel and not channel._non_existent():
                return channel._narrow(CosEventChannelAdmin.EventChannel)
        except CORBA.Exception:
            return None
        return None

    def destroyEventChannel(self, eventChannelName, domainName=None):
        """
        Destroys an existing event channel.
        """
        eventChannel = self.getEventChannel(eventChannelName, domainName)
        if not eventChannel:
            return

        # Forcibly unbind the name first.
        nameBinding = URI.stringToName(self._getEventChannelName(eventChannelName, domainName))
        try:
            self.rootContext.unbind(nameBinding)
        except:
            pass

        # Try to destroy the EventChannel, noting that it could be
        # unreachable even if it was found.
        try:
            # omniEvents requires a small pause to allow the EventChannel
            # to be fully destroyed, otherwise a subsequent creation may
            # fail (as though it still existed).
            eventChannel.destroy()
            while not eventChannel._non_existent():
                _time.sleep(0.1)
        except:
            pass
    
    def connectPushConsumer(self, channel, consumer):
        """A helper function to connect a consumer to a given channel"""
        if channel == None or consumer == None:
            raise ValueError("Both channel and consumer must be provided")
        consumer_admin = channel.for_consumers()
        proxy_supplier = consumer_admin.obtain_push_supplier()
        proxy_supplier.connect_push_consumer(consumer._this())


# Track MessageConsumerPorts that have been started, to ensure that the thread
# is terminated on exit and avoid exception messages
_consumers = []
def _cleanup_consumers():
    # Iterate through a copy to avoid ordering problems at shutdown when
    # consumers try to de-register themselves
    for consumer in _consumers[:]:
        consumer.terminate()
atexit.register(_cleanup_consumers)

class MessageConsumerPort(ExtendedEvent__POA.MessageEvent, threading.Thread):
    class Consumer_i(CosEventChannelAdmin__POA.ProxyPushConsumer):
        def __init__(self, parent, instance_id):
            self.supplier = None
            self.parent = parent
            self.instance_id = instance_id
            self.existence_lock = threading.Lock()

        def push(self, data):
            self.parent.actionQueue.put(('message',data))
        
        def connect_push_supplier(self, supplier):
            self.supplier = supplier
            
        def disconnect_push_consumer(self):
            self.existence_lock.acquire()
            try:
                self.supplier.disconnect_push_supplier()
            except:
                pass
            self.parent.actionQueue.put(('destroy',self.instance_id))
            self.existence_lock.release()
    
    class SupplierAdmin_i(CosEventChannelAdmin__POA.SupplierAdmin):
        def __init__(self, parent):
            self.parent = parent
            self.instance_counter = 0
    
        def obtain_push_consumer(self):
            self.instance_counter += 1
            self.parent.consumer_lock.acquire()
            self.parent.consumers[self.instance_counter] = self.parent.Consumer_i(self.parent,self.instance_counter)
            objref = self.parent.consumers[self.instance_counter]._this()
            self.parent.consumer_lock.release()
            return objref

    def __init__(self, thread_sleep=0.1, parent=None, storeMessages = False):
        self.consumer_lock = threading.Lock()
        threading.Thread.__init__(self)
        self._terminateMe=False
        self._pauseMe=True
        self.state = threading.Condition()
        self.setDaemon(True)
        self.actionQueue = queue.Queue()
        self.thread_sleep = thread_sleep
        self._messages = {}
        self._allMsg = []
        self._connections = {}
        self.consumers = {}
        self.supplier_admin = self.SupplierAdmin_i(self)
        self._parent_comp = parent
        self._storeMessages = storeMessages
        self._storedMessages = []
        self.startPort()


    def registerMessage(self, msgid, msgstruct, callback):
        if msgid != None:
            self._messages[msgid] = (msgstruct, callback)
        else:
            self._allMsg.append((msgstruct, callback))

    # CF.Port
    def connectPort (self, connection, connectionId):
        try:
            channel = connection._narrow(CosEventChannelAdmin.EventChannel)
        except:
            print("WARNING: Could not narrow channel object: %s" % (sys.exc_info()[0]))
            print(connection._NP_RepositoryId)
            return
        self._connections[str(connectionId)] = self._connectConsumerToEventChannel(channel, str(connectionId))

    def disconnectPort (self, connectionId):
        if str(connectionId) in self._connections:
            supplier = self._connections[connectionId]['proxy_supplier']
            del self._connections[connectionId]
            supplier.disconnect_push_supplier()
    
    def _connectConsumerToEventChannel(self, channel, connectionId):
        connection = { 'port': channel }

        consumer_admin = channel.for_consumers()
        proxy_supplier = consumer_admin.obtain_push_supplier()
        connection['proxy_supplier'] = proxy_supplier
        self.consumer_lock.acquire()
        self.consumers[connectionId] = self.Consumer_i(self,connectionId)
        proxy_supplier.connect_push_consumer(self.consumers[connectionId]._this())
        self.consumer_lock.release()

        return connection
    
    # CosEventChannelAdmin.EventChannel
    def for_suppliers(self):
        return self.supplier_admin._this()

    def startPort(self):
        if not self.is_alive():
            self._terminateMe = False
            self.start()
        self._pauseMe = False
        try:
           self.state.acquire()
           self.state.notify()
        finally:
           self.state.release()

    def stopPort(self):
        self._pauseMe=True


    def terminate(self, timeout=None):
        self._pauseMe=False
        self._terminateMe = True
        try:
           self.state.acquire()
           self.state.notify()
        finally:
           self.state.release()
        try:
           self.join(timeout)
        except:
            pass

    def run(self):
        # Start tracking this thread
        _consumers.append(self)
        try:
            self._run()
        finally:
            # Stop tracking this thread
            _consumers.remove(self)

    def getMessages(self):
        retval = copy.deepcopy(self._storedMessages)
        self._storedMessages = []
        return retval

    def _run(self):
        while not self._terminateMe:
            while not self.actionQueue.empty():
                action, payload = self.actionQueue.get()
                if action == 'destroy':
                    self.consumer_lock.acquire()
                    for consumer in self.consumers:
                        if consumer == payload:
                            consumer = self.consumers.pop(consumer)
                            consumer.existence_lock.acquire()
                            consumer.existence_lock.release()
                            del consumer
                            break
                    self.consumer_lock.release()
                elif action == 'message':
                    values = payload.value(CF._tc_Properties)
                    if values is None:
                        print('WARNING: Unrecognized message type', payload.typecode())
                    for value in values:
                        id = value.id
                        if id in self._messages:
                            msgstruct, callback = self._messages[id]
                            tmp_value = struct_from_any(value.value, msgstruct, strictComplete=False)
                            try:
                                callback(id, tmp_value)
                            except Exception as e:
                                print("Callback for message "+str(id)+" failed with exception: "+str(e))
                        for allMsg in self._allMsg:
                            if self._storeMessages:
                                self._storedMessages.append(prop_to_dict(value))
                            callback = allMsg[1]
                            try:
                                callback(id, value)
                            except Exception as e:
                                print("Callback for message "+str(id)+" failed with exception: "+str(e))
            else:
                _time.sleep(self.thread_sleep)

class MessageSupplierPort(ExtendedCF__POA.QueryablePort):
    class Supplier_i(CosEventComm__POA.PushSupplier):
        def disconnect_push_supplier(self):
            pass

    def __init__(self):
        self._messages = {}
        self._connections = {}
        self.portInterfaceAccess = threading.Lock()
        self._port_log = logging.getLogger("MessageSupplierPort")

    def registerMessage(self, msgid, msgstruct, callback):
        self._messages[msgid] = (msgstruct, callback)

    def connectPort (self, connection, connectionId):
        self.portInterfaceAccess.acquire()
        try:
            channel = connection._narrow(CosEventChannelAdmin.EventChannel)
        except:
            print("WARNING: Could not narrow channel object: %s" % (sys.exc_info()[0]))
            print(connection._NP_RepositoryId)
            self.portInterfaceAccess.release()
            return
        self._connections[str(connectionId)] = self._connectSupplierToEventChannel(channel)
        self.portInterfaceAccess.release()


    def disconnectPort (self, connectionId):
        self.portInterfaceAccess.acquire()
        try:
            if str(connectionId) in self._connections:
                consumer = self._connections[connectionId]['proxy_consumer']
                consumer.disconnect_push_consumer()
                del self._connections[connectionId]
        except:
            self.portInterfaceAccess.release()
            raise
        self.portInterfaceAccess.release()
    
    def _connectSupplierToEventChannel(self, channel):
        connection = { 'port': channel }

        supplier_admin = channel.for_suppliers()
        proxy_consumer = supplier_admin.obtain_push_consumer()
        connection['proxy_consumer'] = proxy_consumer
        connection['supplier'] = self.Supplier_i()
        proxy_consumer.connect_push_supplier(connection['supplier']._this())

        return connection

    # CosEventComm.PushSupplier delegation
    def push(self, data, connectionId=None):
        """
        Sends pre-serialized messages.

        Args:
            data:         Messages serialized to a CORBA.Any
            connectionId: Target connection (default: all).

        Raises:
            ValueError: If connectionId is given and does not match any
                        connection.
        """

        try:
            self._push( data, connectionId )
        except CORBA.MARSHAL:
            self._port_log.warn("Could not deliver the message. Maximum message size exceeded")


    # CosEventComm.PushSupplier delegation
    def _push(self, data, connectionId=None):
        """
        Sends pre-serialized messages.

        Args:
            data:         Messages serialized to a CORBA.Any
            connectionId: Target connection (default: all).

        Raises:
            ValueError: If connectionId is given and does not match any
                        connection.
        """
        with self.portInterfaceAccess:
            self._checkConnectionId(connectionId)

            for identifier, connection in self._connections.items():
                if not self._isConnectionSelected(identifier, connectionId):
                    continue

                try:
                    connection['proxy_consumer'].push(data)
                except CORBA.MARSHAL as e:
                    raise e
                except:
                    self._port_log.warn("WARNING: Unable to send data to " + identifier)


    def sendMessage(self, data_struct, connectionId=None):
        """
        Sends a single message.

        Args:
            data_struct:  Message structure or CORBA.Any to send.
            connectionId: Target connection (default: all).

        Raises:
            ValueError: If connectionId is given and does not match any
                        connection.
        """
        if not isinstance(data_struct, CORBA.Any):
            outgoing = [CF.DataType(id=data_struct.getId(),value=struct_to_any(data_struct))]
            outmsg = props_to_any(outgoing)
        else:
            outmsg = data_struct
        self.push(outmsg, connectionId)

    def sendMessages(self, data_structs, connectionId=None):
        """
        Sends a list of messages.

        Args:
            data_structs: Sequence of messages to send.
            connectionId: Target connection (default: all).

        Raises:
            ValueError: If connectionId is given and does not match any
                        connection.
        """
        outgoing = []
        msgid=None
        for msg in data_structs:
            msgid=msg.getId()
            outgoing.append(CF.DataType(id=msg.getId(),value=struct_to_any(msg)))
        outmsg = props_to_any(outgoing)

        try:
            # try to push entire message set 
            self._push(outmsg, connectionId)
        except CORBA.MARSHAL:
            if len(data_structs) == 1:
                self._port_log.warn("Could not deliver the message id="+str(msgid)+". Maximum message size exceeded")
            else:
                self._port_log.warn("Could not deliver the message. Maximum message size exceeded, trying individually")
                # try resending individually
                for msg in data_structs:
                    outm = props_to_any([CF.DataType(id=msg.getId(),value=struct_to_any(msg))])
                    try:
                        self._push(outm,connectionId)
                    except CORBA.MARSHAL:
                        self._port_log.warn("Could not deliver the message id="+str(msg.getId())+". Maximum message size exceeded")
                        break
                    except:
                        print("WARNING: Unable to send data to",connection)

    def disconnect_push_supplier(self):
        pass

    def _get_connections(self):
        return [ExtendedCF.UsesConnection(k, v['port']) for k, v in self._connections.items()]

    def _isConnectionSelected(self, connectionId, targetId):
        if not targetId:
            return True
        return connectionId == targetId

    def _checkConnectionId(self, connectionId):
        if connectionId and not connectionId in self._connections:
            raise ValueError("invalid connection '"+connectionId+"'")
