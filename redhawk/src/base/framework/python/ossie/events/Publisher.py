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
import time
import Queue
import copy
import logging
import traceback
import threading

from omniORB import any, URI, CORBA
import CosEventComm__POA
import CosEventChannelAdmin, CosEventChannelAdmin__POA


class Receiver(CosEventComm__POA.PushSupplier):
    def __init__(self):
        self._recv_disconnect = True
        self.logger = logging.getLogger("ossie.events.Publisher.Receiver")
        self._lock = threading.Lock()
        self._cond = threading.Condition(self._lock)


    def __del__(self):
        self._cond.acquire()
        self._recv_disconnect = True
        self._cond.notifyAll()
        self._cond.release()
    
    def get_disconnect(self):
        return self._recv_disconnect

    def reset(self):
        self._recv_disconnect = False

    def disconnect_push_supplier(self):
        self.logger.debug("Publisher.Reciever handle disconnect_push_supplier")
        self._cond.acquire()
        self._recv_disconnect = True
        self._cond.notifyAll()
        self._cond.release()
    
    def wait_for_disconnect( self, wait_time=-1.0, retries=-1 ):
        try:
            self._cond.acquire()
            tries = retries
            while self._recv_disconnect == False:
                self.logger.debug("Publisher.Reciever .... waiting for disconnect")
                if wait_time > -1.0:
                    self._cond.wait( wait_time/1000.0 )
                    if tries == -1:
                        break
                    tries -= 1
                    if tries < 1:
                        break;
                else:
                    self._cond.wait()
        finally:
            self._cond.release()
        

class DefaultReceiver(Receiver):
    def __init__(self,parent):
        self.parent = parent
        Receiver.__init__(self)

class Publisher:
    def __init__(self, channel ):
        self.channel = channel
        self.proxy = None
        self.logger = logging.getLogger("ossie.events.Publisher")
        self.disconnectReceiver = DefaultReceiver(self)

            
        self.connect()

    def __del__(self):
        self.logger.debug("Publisher  DTOR START")
        if self.disconnectReceiver and self.disconnectReceiver.get_disconnect() == False:
            self.logger.debug("Publisher::DTOR  DISCONNECT")
            self.disconnect()
        
        self.logger.debug("Publisher::DTOR  DEACTIVATE")
        self.disconnectReceiver=None
        self.proxy=None
        self.channel=None
        self.logger.debug("Publisher  DTOR END")


    def terminate(self):
        self.logger.debug("Publisher::terminate START")
        if self.disconnectReceiver and self.disconnectReceiver.get_disconnect() == False:
            self.logger.debug("Publisher::terminate  DISCONNECT")
            self.disconnect()
        
        self.logger.debug("Publisher::terminate  DEACTIVATE")
        self.disconnectReceiver=None
        self.proxy=None
        self.channel=None
        self.logger.debug("Publisher::terminate END")

        if self.proxy:
            for x in range(10):
                try:
                    self.proxy.disconnect_push_consumer()
                    break
                except CORBA.COMM_FAILURE:
                    pass
                time.sleep(.01)

        self.proxy = None
        self.supplier = None
        self.channel = None


    def push(self,data ):
        retval=0
        edata=data
        if not isinstance(data, CORBA.Any):
            edata = any.to_any(data)
                            
        try:
            if self.proxy != None:
                self.proxy.push(edata)
        except:
            #traceback.print_exc()
            retval=-1
        
        return retval


    def disconnect(self, retries=10, retry_wait=.01):
        retval=0
        if self.channel == None:
            return retval

        if self.proxy:
            retval=-1
            for x in range(retries):
                try:
                    self.proxy.disconnect_push_consumer()
                    retval=0
                    break
                except CORBA.COMM_FAILURE:
                    self.logger.error("Publisher ::disconnect, Caught COMM_FAILURE, Retrying.")
                    pass
                time.sleep(retry_wait)

            if self.disconnectReceiver:
                self.logger.debug("Publisher ::disconnect, Waiting for disconnect.......")
                self.disconnectReceiver.wait_for_disconnect( .01, 3)
                self.logger.debug("Publisher ::disconnect, received disconnect.......")

        return retval


    def connect(self, retries=10, retry_wait=.01):
        retval=-1
        
        if self.channel == None:
            return retval
    
        if self.proxy == None:
            self.logger.debug("Getting supplier object")
            for x in range(retries):
                try: 
                    self.logger.debug("Getting supplier object" + str(self.channel) )
                    supplier_admin = self.channel.for_suppliers()
                    break
                except CORBA.COMM_FAILURE:
                    pass
                time.sleep(retry_wait)

            for x in range(retries):
                try: 
                    self.proxy = supplier_admin.obtain_push_consumer()
                    break
                except CORBA.COMM_FAILURE:
                    pass
                time.sleep(retry_wait)

        self.logger.debug("Publisher Checking proxy...")
        if self.proxy != None:
            self.logger.debug("Publisher Connect receiver to  EventChannel ")
            for x in range(retries):
                try: 
                    self.proxy.connect_push_supplier( self.disconnectReceiver._this() )
                    self.disconnectReceiver.reset()
                    retval=0
                    break
                except CORBA.BAD_PARAM:
                    break
                except CORBA.COMM_FAILURE:
                    pass
                except CosEventChannelAdmin.AlreadyConnected:
                    self.logger.debug("Publisher Already connected to  EventChannel ")
                    self.disconnectReceiver.reset()
                    retval=0
                    break
                time.sleep(retry_wait)

        return retval
