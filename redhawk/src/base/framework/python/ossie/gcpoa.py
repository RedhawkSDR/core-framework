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

import time, threading
from omniORB import CORBA, PortableServer, PortableServer__POA

class SingletonThread(threading.Thread):
    __instance = None

    @staticmethod 
    def getInstance():
        """ Static access method. """
        if SingletonThread.__instance == None:
            SingletonThread()
        return SingletonThread.__instance

    def __init__(self, delay):
        threading.Thread.__init__(self)
        self.setDaemon(True)
        self._delay = delay
        self.contexts = []
        self.instanceMutex_ = threading.Lock()
        self.stop_signal = threading.Event()
        if SingletonThread.__instance != None:
            raise Exception("SingletonThread is a singleton")
        else:
            SingletonThread.__instance = self

    def stop(self):
        self.stop_signal.set()

    def addContext(self, context):
        self.contexts.append(context)

    def releaseContext(self, context):
        try:
            self.contexts.remove(context)
        except:
            print 'Unable to remove context'

    def run(self):
        while not self.stop_signal.isSet():
            with self.instanceMutex_:
                for context in self.contexts:
                    context.gc_sweep()
            time.sleep(self._delay)

class POACreator(PortableServer__POA.AdapterActivator):
    def __init__(self):
        pass

    def unknown_adapter(self, parent, name):
        parent_name_obj = parent.the_name
        parent_name = str(parent_name_obj)
        child_name = name

        if (child_name == "Iterators"):
            try:
                child = createGCPOA(parent, child_name)
            except PortableServer.POA.AdapterAlreadyExists:
                return False
            except PortableServer.POA.InvalidPolicy:
                sys.exit(-1)
            return True

        return False

class ServantEntry:
    def __init__(self, servant, ttl, destroy, last_access):
        self.servant = servant
        self.ttl = ttl
        self.destroy = destroy
        self.last_access = last_access

class GCContext:
    def gc_sweep(self):
        pass

class GCServantLocator(PortableServer__POA.ServantLocator, GCContext):
    def __init__(self):
        self.activeMap_ = {}
        self.activeMapMutex_ = threading.Lock()
        processing_thread = SingletonThread(0.1)
        processing_thread.addContext(self)
        processing_thread.start()

    def __del__(self):
        pass

    def preinvoke(self, oid, adapter, operation, the_cookie=0):
        with self.activeMapMutex_:
            if not self.activeMap_.has_key(oid):
                raise CORBA.OBJECT_NOT_EXIST()

            servant = self.activeMap_[oid].servant
            if self.activeMap_[oid].destroy == operation:
                self.activeMap_.pop(oid)

            self.activeMap_[oid].last_access = time.time()
            return servant, the_cookie

    def postinvoke(self, oid, adapter, operation, the_cookie, the_servant):
        pass

    def register_servant(self, oid, servant, ttl, destroy):
        with self.activeMapMutex_:
            if self.activeMap_.has_key(oid):
                raise PortableServer.POA.ObjectAlreadyActive()

            self.activeMap_[oid] = ServantEntry(servant, ttl, destroy, time.time())

    def gc_sweep(self):
        with self.activeMapMutex_:
            keys_to_remove = []
            for _iter in self.activeMap_:
                age = time.time() - self.activeMap_[_iter].last_access
                if age > self.activeMap_[_iter].ttl:
                    keys_to_remove.append(_iter)
            for key in keys_to_remove:
                self.activeMap_.pop(key)

def createGCPOA(parent, name):
    policy_list = []
    policy_list.append(parent.create_servant_retention_policy(PortableServer.NON_RETAIN))
    policy_list.append(parent.create_request_processing_policy(PortableServer.USE_SERVANT_MANAGER))

    poa_mgr = parent.the_POAManager
    poa = parent.create_POA(name, poa_mgr, policy_list)

    for ii in range(len(policy_list)):
        policy_list[ii].destroy()

    manager = GCServantLocator()
    manager_ref = manager._this()
    poa.set_servant_manager(manager_ref)

    return poa

def activateGCObject(poa, servant, ttl=60, destroy="destroy"):
    manager = poa.get_servant_manager()
    orb = CORBA.ORB_init()
    mgr_servant = orb.resolve_initial_references("RootPOA").reference_to_servant(manager);
    if mgr_servant == None:
        raise PortableServer.POA.WrongPolicy()

    obj = poa.create_reference(servant._NP_RepositoryId)
    oid = poa.reference_to_id(obj)
    mgr_servant.register_servant(oid, servant, ttl, destroy)
    return obj
