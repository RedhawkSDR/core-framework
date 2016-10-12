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
import threading
import logging
import traceback

from omniORB import any, URI, CORBA
from ossie.cf import CF, CF__POA
import CosEventComm__POA
import CosEventChannelAdmin, CosEventChannelAdmin__POA


class Receiver(CosEventComm__POA.PushConsumer):
    def __init__(self):
        self._recv_disconnect = True
        self.logger = logging.getLogger("ossie.events.Subscriber.Receiver")
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

    def disconnect_push_consumer(self):
        self.logger.debug("Subscriber.Receiver handle disconnect_push_consumer")
        self._cond.acquire()
        self._recv_disconnect = True
        self._cond.notifyAll()
        self._cond.release()
    
    def wait_for_disconnect( self, wait_time=-1.0, retries=-1 ):
        try:
            self._cond.acquire()
            tries = retries
            while self._recv_disconnect == False:
                self.logger.debug("Subscriber.Reciever::wait_for_disconnect.... waiting for disconnect")
                if wait_time and wait_time > -1.0:
                    self._cond.wait( wait_time/1000.0 )
                    if tries == -1:
                        break
                    tries -= 1
                    if tries < 1:
                        self.logger.debug("Subscriber.Reciever::wait_for_disconnect.... out of retries...")
                        break;
                else:
                    self.logger.debug("Subscriber.Reciever::wait_for_disconnect.... wait forever..")
                    self._cond.wait()
        finally:
            self._cond.release()


class DefaultConsumer(Receiver):
    def __init__(self,parent):
        self.parent = parent
        Receiver.__init__(self)

    def push(self, data):
        if self.parent.dataArrivedCB != None:
            self.parent.logger.trace('Received (callback) DATA: ' + str(data))
            self.parent.dataArrivedCB( data )
        else:
            self.parent.logger.trace('Received (queue) DATA: ' + str(data))
            self.parent.events.put(data)
        

class Subscriber:
    def __init__(self, channel, dataArrivedCB=None):
        self.channel = channel
        self.proxy = None
        self.logger = logging.getLogger('ossie.events.Subscriber')
        self.dataArrivedCB=dataArrivedCB
        self.events = Queue.Queue()
        self.my_lock = threading.Lock()
        self.consumer = DefaultConsumer(self)
        self.connect()


    def __del__(self):
        self.logger.debug("Subscriber  DTOR START")
        if self.consumer and self.consumer.get_disconnect() == False:
            self.logger.debug("Subscriber::DTOR  DISCONNECT")
            self.disconnect()

        self.logger.debug("Subscriber::DTOR  DEACTIVATE CONSUMER")
        self.proxy = None
        self.consumer = None
        self.channel = None
                             
        self.logger.debug("Subscriber  DTOR END")

    def setDataArrivedCB(self, newCB=None ):
        self.dataArrivedCB= newCB

    def terminate(self):
        self.logger.debug("Subscriber::terminate START")
        if self.consumer and self.consumer.get_disconnect() == False:
            self.logger.debug("Subscriber::terminate DISCONNECT")
            self.disconnect()

        self.logger.debug("Subscriber::terminate  DEACTIVATE CONSUMER")
        self.proxy = None
        self.consumer = None
        self.channel = None
                             
        self.logger.debug("Subscriber::terminate END")

    def getData(self):
        retval=None
        if self.dataArrivedCB != None:
            return retval;
        
        try:
            tmp = self.events.get(False,.01)
            self.logger.debug('getData: ' + str(tmp))
            retval = any.from_any(tmp)
        except:
            #print traceback.print_exc()
            retval=None
        
        return retval


    def disconnect(self, retries=10, retry_wait=.01):
        if self.channel == None:
            return -1

        retval=0
        if self.proxy:
            retval=-1
            for x in range(retries):
                try:
                    self.logger.debug("Subscriber::disconnect  disconnect_push_supplier")
                    self.proxy.disconnect_push_supplier()
                    retval=0
                    break
                except CORBA.COMM_FAILURE:
                    pass
                time.sleep(retry_wait)

            if self.consumer:
               self.logger.debug("Subscriber::disconnect  Waiting for disconnect")
               self.consumer.wait_for_disconnect(.01,3)
               self.logger.debug("Subscriber::disconnect  recevied  disconnect")
           
        return retval


    def connect(self, retries=10, retry_wait=.01):
        
        if self.channel == None:
            return -1

        if self.proxy == None :
            for x in range(retries):
                try: 
                    admin = self.channel.for_consumers()
                    break
                except CORBA.COMM_FAILURE:
                    pass
                time.sleep(retry_wait)

            for x in range(retries):
                try: 
                    self.proxy = admin.obtain_push_supplier()
                    break
                except CORBA.COMM_FAILURE:
                    pass
            
                time.sleep(retry_wait)

        if self.proxy == None:
             return
        
        self.logger.debug("Subscriber::connect  Obtained Proxy for EventChannel")

        for x in range(retries):
            try: 
                self.proxy.connect_push_consumer( self.consumer._this() )
                self.consumer.reset()
                break
            except CORBA.BAD_PARAM:
                break
            except CORBA.COMM_FAILURE:
                pass
            except CosEventChannelAdmin.AlreadyConnected:
                self.consumer.reset()
                break
                
            time.sleep(retry_wait)

        self.logger.debug("Subscriber::connect  Connected to EventChannel")

