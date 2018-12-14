#{#
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
#}
#% set className = portgen.className()
class ${className}(${component.baseclass.name}.${portgen.templateClass()}):
    def __init__(self, parent, name):
        self.parent = parent
        self.name = name
        self.outConnections = {}
        self.port_lock = threading.Lock()

    def getConnectionIds(self):
        return self.outConnections.keys()

    def _evaluateRequestBasedOnConnections(self, __connection_id__, returnValue, inOut, out):
        if not __connection_id__ and len(self.outConnections) > 1:
            if (out or inOut or returnValue):
                raise PortCallError("Returned parameters require either a single connection or a populated __connection_id__ to disambiguate the call.", self.getConnectionIds())

        if len(self.outConnections) == 0:
            if (out or inOut or returnValue):
                raise PortCallError("No connections available.", self.getConnectionIds())
            else:
                if __connection_id__:
                    raise PortCallError("The requested connection id ("+__connection_id__+") does not exist.", self.getConnectionIds())
        if __connection_id__ and len(self.outConnections) > 0:
            foundConnection = False
            for connId, port in self.outConnections.items():
                if __connection_id__ == connId:
                    foundConnection = True
                    break
            if not foundConnection:
                raise PortCallError("The requested connection id ("+__connection_id__+") does not exist.", self.getConnectionIds())

    def connectPort(self, connection, connectionId):
        self.port_lock.acquire()
        try:
            port = connection._narrow(${portgen.corbaClass()})
            self.outConnections[str(connectionId)] = port
        finally:
            self.port_lock.release()

    def disconnectPort(self, connectionId):
        self.port_lock.acquire()
        try:
            self.outConnections.pop(str(connectionId), None)
        finally:
            self.port_lock.release()

    def _get_connections(self):
        self.port_lock.acquire()
        try:
            return [ExtendedCF.UsesConnection(name, port) for name, port in self.outConnections.iteritems()]
        finally:
            self.port_lock.release()
#{% for operation in portgen.operations() %}

#{% set arglist = ['self'] + operation.args %}
#{%  if arglist %}
#{%   if operation.is_attribute %}
    def ${operation.base_attribute}(self, __connection_id__ = ""):
#{%    if operation.returns %}
        return self.${operation.name}(__connection_id__)
#{%    else %}
        self.${operation.name}(__connection_id__)
#{%    endif %}

#{%   endif %}
    def ${operation.name}(${arglist|join(', ')}, __connection_id__ = ""):
#{%  else %}
#{%   if operation.is_attribute %}
    def ${operation.base_attribute}(__connection_id__ = ""):
#{%    if operation.returns %}
        return self.${operation.name}(__connection_id__)
#{%    else %}
        self.${operation.name}(__connection_id__)
#{%    endif %}

#{%   endif %}
    def ${operation.name}(__connection_id__ = ""):
#{%  endif %}
#{% if operation.returns|length > 1 %}
        retVal = []
#{% elif operation.returns|first == 'string' %}
        retVal = ""
#{% elif operation.returns %}
        retVal = None
#{% endif %}
        self.port_lock.acquire()

#{% set hasreturn = (operation.hasreturnType != 'void') %}
#{% if hasreturn %}
#{%     set returnstate='True' %}
#{% else %}
#{%     set returnstate='False' %}
#{% endif %}
#{% set hasout = operation.hasout %}
#{% if hasout %}
#{%     set _hasout='True' %}
#{% else %}
#{%     set _hasout='False' %}
#{% endif %}
#{% set hasinout = operation.hasinout %}
#{% if hasinout %}
#{%     set _hasinout='True' %}
#{% else %}
#{%     set _hasinout='False' %}
#{% endif %}

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, ${returnstate}, ${_hasinout}, ${_hasout})
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    try:
                        ${"retVal = " if operation.returns}port.${operation.name}(${operation.args|join(', ')})
                    except Exception:
                        self.parent._baseLog.exception("The call to ${operation.name} failed on port %s connection %s instance %s", self.name, connId, port)
                        raise
        finally:
            self.port_lock.release()
#{% if operation.returns %}

        return retVal
#{% endif %}
#{% endfor %}
