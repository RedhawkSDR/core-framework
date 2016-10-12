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
import threading
import Queue
import copy
import traceback
import logging

from omniORB import any, URI, CORBA
import CosEventComm__POA
import CosEventChannelAdmin, CosEventChannelAdmin__POA
import CosNaming
import CosLifeCycle

from  ossie.cf.CF import EventChannelManager
from  ossie.events  import Publisher
from  ossie.events  import Subscriber



class EM_Publisher(Publisher):
    def __init__(self, ecm, creg ):
        Publisher.__init__(self, creg.channel )
        self._ecm = ecm
        self._creg = creg

    def __del__(self):
        self.terminate()

    def terminate(self):
        if self._ecm:  self._ecm._unregister( self._creg )

class EM_Subscriber(Subscriber):
    def __init__(self, ecm, creg ):
        Subscriber.__init__(self, creg.channel )
        self._ecm = ecm
        self._creg = creg

    def __del__(self):
        self.terminate()

    def terminate(self):
        if self._ecm:  self._ecm._unregister( self._creg )


class Manager:
    _manager = None
    """This class provides the ability to manage event channels from Python"""
    def __init__(self, resource=None ):
        self._mgr_lock = threading.Lock()
        self._ecm = None
        self._logger = logging.getLogger("ossie.events.Manager")
        self._logger.setLevel(logging.INFO)
        self._allow = True
        self._registrations=[]
        if resource :
            try:
                self._logger.debug("Requesting Domain Manager Access....")
                dom = resource.getDomainManager()
                self._logger.debug("Requesting EventChannelManager Access....")
                self._ecm  = dom.getRef()._get_eventChannelMgr()
                self._logger.debug("Acquired reference to EventChannelManager")
            except:
                #print traceback.format_exc()
                self._logger.warn("EventChannelManager - unable to resolve DomainManager's EventChannelManager ")
                pass


    @staticmethod
    def GetManager( resource ):
        if Manager._manager == None :
            try:
                Manager._manager = Manager( resource )
            except:
                #print traceback.format_exc()
                logging.getLogger("ossie.events.Manager").warn("Unable to resolve Event Manager")
 
        return Manager._manager


    @staticmethod
    def Terminate() :
        # release all publishers and subscribers
        if Manager._manager:
            Manager._manager._terminate()


    def Publisher(self, channel_name, registrationId=""):

        self._mgr_lock.acquire()
        self._logger.debug("Requesting Publisher for Channel:" + str(channel_name) )
        pub = None
        try:
            if self._ecm:
                ereg = EventChannelManager.EventRegistration( channel_name = channel_name, reg_id = registrationId)

            
                self._logger.debug("Requesting Channel:" + str(channel_name) + " from Domain's EventChannelManager ")
                registration = self._ecm.registerResource( ereg )
                
                pub = EM_Publisher( self, registration )

                self._logger.debug("Channel:" + str(channel_name) + " Reg-Id:" + str(registration.reg.reg_id))
                
                self._registrations.append( registration )
        
        except:
            #print traceback.format_exc()
            self._logger.error("Unable to create Publisher for Channel:" + str(channel_name ))
        finally:
            self._mgr_lock.release()

        return pub


    def Subscriber(self, channel_name, registrationId=""):

        self._mgr_lock.acquire()
        self._logger.debug("Requesting Subscriber for Channel:" + str(channel_name) )
        sub = None
        try:
            if self._ecm:
                ereg = EventChannelManager.EventRegistration( channel_name = channel_name, reg_id = registrationId)
            
                self._logger.debug("Requesting Channel:" + str(channel_name) + " from Domain's EventChannelManager ")
                registration = self._ecm.registerResource( ereg )
                
                sub = EM_Subscriber( self, registration )

                self._logger.debug("Channel:" + str(channel_name) + " Reg-Id:" + str(registration.reg.reg_id))
                
                self._registrations.append( registration )
        
        except:
            #print traceback.format_exc()
            self._logger.error("Unable to create Subscriber for Channel:" + str(channel_name ))
        finally:
            self._mgr_lock.release()

        return sub


    def _terminate( self ) :
        
        self._mgr_lock.acquire()
        self._allow = False

        self._logger.debug( "Terminate All Registrations.: "  + str(len(self._registrations)) )
        for creg in self._registrations:
            if self._ecm != None:
                try :
                    self._logger.debug( "Unregister REG=ID:" + str(creg.reg.reg_id ) )
                    self._ecm.unregister( creg.reg )
                except:
                    self._logger.debug( "Unregister Failed for REG=ID:" + str(creg.reg.reg_id ) )

        # need to cleanup Publisher memory
        self._registrations[:] = []
        self._logger.debug( "Terminate Completed.");
        self._mgr_lock.release()

    def _unregister( self, reg ):
        self._logger.debug( "Unregister request .... allow: " + str(self._allow) + " reg-id:" +str(reg.reg.reg_id ) )

        if self._allow == False : return

        self._mgr_lock.acquire()

        for creg in self._registrations:
            if reg.reg.reg_id == creg.reg.reg_id :
                if self._ecm != None:
                    try :
                        # unregister from the Domain
                        self._logger.info( "UNREGISTER REG-ID:" + str(reg.reg.reg_id) +  " CHANNEL:"  + str(reg.reg.channel_name ) )
                        self._ecm.unregister( reg.reg );
                    except:
                        self._logger.error( "UNREGISTER FAILED, REG-ID:" + str(reg.reg.reg_id) +  " CHANNEL:"  + str(reg.reg.channel_name ) )
                                    
                self._registrations.remove(creg)
                break

        self._mgr_lock.release()
        self._logger.debug( "Unregister for ... reg-id:" +str(reg.reg.reg_id ) + " completed " )
    
