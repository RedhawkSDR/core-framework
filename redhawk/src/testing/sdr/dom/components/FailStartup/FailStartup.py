#!/usr/bin/env python
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

#
from ossie.cf import CF, CF__POA
from omniORB import CORBA, URI
import sys, signal, copy
import CosNaming

class FailStartup(CF__POA.Resource):
    def __init__(self, execparams):
        self.execparams = execparams
        self._identifier = execparams["COMPONENT_IDENTIFIER"]

    def _get_identifier(self):
        return self._identifier

    def initialize(self):
        if self.execparams["FAIL_AT"] == "Initialize":
            raise StandardError

    def start(self):
        if self.execparams["FAIL_AT"] == "Start":
            raise StandardError

    def stop(self):
        pass # TODO: add your implementation here

    def releaseObject(self):
        pass # TODO: add your implementation here

    def getPort(self, name):
        raise CF.PortSupplier.UnknownPort() # TODO: add your implementation here

    def runTest(self, properties, testid):
        raise CF.TestableObject.UnknownTest("unknown test: %s" % str(testid)) # TODO: add your implementation here

    def configure(self, configProperties):
        if self.execparams["FAIL_AT"] == "Configure":
            raise StandardError

    def query(self, configProperties):
        return [] # TODO: add your implementation here

if __name__ == '__main__':
    # This code is boilerplate startup code for a component
    def __exit_handler(signum, frame):
        raise SystemExit

    # Turn the SCA execparams into a dictionary
    args = copy.copy(sys.argv[1:])
    execparams = {}
    while len(args) > 0:
        try:
            paramid = args.pop(0)
            paramvalue = args.pop(0)
            execparams[paramid] = paramvalue
        except IndexError:
            pass

    # Verify that the required parameters were provided
    for reqparam in ("NAMING_CONTEXT_IOR", "COMPONENT_IDENTIFIER", "NAME_BINDING"):
        if not execparams.has_key(reqparam):
            sys.exit(-1)

    print >> sys.stderr, "EXEC PARAMS", execparams
    orb = None
    signal.signal(signal.SIGINT, __exit_handler)
    signal.signal(signal.SIGTERM, __exit_handler)
    try:
        if execparams["FAIL_AT"] == "PreOrbInit":
            raise StandardError

        # ORB initialization
        orb = CORBA.ORB_init()
        obj_poa = orb.resolve_initial_references("RootPOA")
        poaManager = obj_poa._get_the_POAManager()
        poaManager.activate()

        if execparams["FAIL_AT"] == "PreServantCreation":
            raise StandardError

        # Create the component servant object
        component_Obj = FailStartup(execparams)
        component_Var = component_Obj._this()

        if execparams["FAIL_AT"] == "PreNameBinding":
            raise StandardError

        # Bind it to the naming context
        rootContext = orb.string_to_object(execparams['NAMING_CONTEXT_IOR'])
        if rootContext == None:
            raise SystemExit, "Failed to lookup naming context"
        rootContext = rootContext._narrow(CosNaming.NamingContext)
        name = URI.stringToName(execparams['NAME_BINDING'])
        rootContext.rebind(name, component_Var)

        if execparams["FAIL_AT"] == "PreOrbRun":
            raise StandardError
        orb.run()
    finally:
        if orb:
            orb.destroy()
