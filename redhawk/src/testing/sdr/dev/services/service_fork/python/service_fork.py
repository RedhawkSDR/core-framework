#!/usr/bin/env python3
#
# AUTO-GENERATED
#
# Source: service_fork.spd.xml

import sys, signal, copy, os
import logging

from ossie.cf import CF, CF__POA #@UnusedImport
from ossie.service import start_service
from omniORB import CORBA, URI, PortableServer

from ossie.cf import CF
from ossie.cf import CF__POA
import ossie.utils as _utils
from omniORB import any as _any

class service_fork(CF__POA.Application):

    def __init__(self, name="service_fork", execparams={}):
        self.name = name
        self._baseLog = logging.getLogger(self.name)
        self._log = logging.getLogger(self.name)
        sp = _utils.Popen(['python', '../../../dev/services/service_fork/python/loop.py'])
        self._pid = sp.pid

    def terminateService(self):
        pass

    def initialize(self):
        # TODO
        pass

    def releaseObject(self):
        # TODO
        pass

    def runTest(self, testid, testValues):
        # TODO
        pass

    def configure(self, configProperties):
        # TODO
        pass

    def query(self, configProperties):
        return [CF.DataType(id='child_pid', value=_any.to_any(self._pid)), CF.DataType(id='self_pid', value=_any.to_any(os.getpid()))]

    def initializeProperties(self, initialProperties):
        # TODO
        pass

    def registerPropertyListener(self, obj, prop_ids, interval):
        # TODO
        pass

    def unregisterPropertyListener(self, id):
        # TODO
        pass

    def getPort(self, name):
        # TODO
        pass

    def getPortSet(self):
        # TODO
        pass

    def retrieve_records(self, howMany, startingRecord):
        # TODO
        pass

    def retrieve_records_by_date(self, howMany, to_timeStamp):
        # TODO
        pass

    def retrieve_records_from_date(self, howMany, from_timeStamp):
        # TODO
        pass

    def getLogLevel(self, logger_id):
        # TODO
        pass

    def setLogLevel(self, logger_id, newLevel):
        # TODO
        pass

    def getNamedLoggers(self):
        # TODO
        pass

    def resetLog(self):
        # TODO
        pass

    def getLogConfig(self):
        # TODO
        pass

    def setLogConfig(self, config_contents):
        # TODO
        pass

    def setLogConfigURL(self, config_url):
        # TODO
        pass

    def start(self):
        # TODO
        pass

    def stop(self):
        # TODO
        pass

    def metrics(self, components, attributes):
        # TODO
        pass

    def _get_log_level(self):
        # TODO
        pass

    def _set_log_level(self, data):
        # TODO
        pass

    def _get_identifier(self):
        # TODO
        pass

    def _get_started(self):
        # TODO
        pass

    def _get_softwareProfile(self):
        # TODO
        pass

    def _get_registeredComponents(self):
        # TODO
        pass

    def _get_componentNamingContexts(self):
        # TODO
        pass

    def _get_componentProcessIds(self):
        # TODO
        pass

    def _get_componentDevices(self):
        # TODO
        pass

    def _get_componentImplementations(self):
        # TODO
        pass

    def _get_profile(self):
        # TODO
        pass

    def _get_name(self):
        # TODO
        pass

    def _get_aware(self):
        # TODO
        pass

    def _get_stopTimeout(self):
        # TODO
        pass

    def _set_stopTimeout(self, data):
        # TODO
        pass


if __name__ == '__main__':
    start_service(service_fork, thread_policy=PortableServer.SINGLE_THREAD_MODEL)
