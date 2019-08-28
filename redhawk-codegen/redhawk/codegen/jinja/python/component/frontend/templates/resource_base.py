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
#% extends "pull/resource_base.py"
#{% block basefeiimports %}
import frontend
from frontend import FRONTEND
#{% if 'FrontendTuner' in component.implements %}
from ossie.properties import struct_to_props
BOOLEAN_VALUE_HERE=False
#{% endif %}
#{% endblock %}
#{% block baseadditionalimports %}
from omniORB import any as _any
#{% endblock %}
#{% block extensions %}
#{% for prop in component.properties if prop.name == "frontend_tuner_status" %}
        # Rebind tuner status property with custom struct definition
#{%  if 'ScanningTuner' in component.implements %}
        frontend_tuner_status = FrontendScannerDevice.frontend_tuner_status.rebind()
#{%  else %}
        frontend_tuner_status = FrontendTunerDevice.frontend_tuner_status.rebind()
#{%  endif %}
        frontend_tuner_status.structdef = frontend_tuner_status_struct_struct
#{% endfor %}

#{% if 'FrontendTuner' in component.implements %}
        def frontendTunerStatusChanged(self,oldValue, newValue):
            pass

        def getTunerStatus(self,allocation_id):
            tuner_id = self.getTunerMapping(allocation_id)
            if tuner_id < 0:
                raise FRONTEND.FrontendException(("ERROR: ID: " + str(allocation_id) + " IS NOT ASSOCIATED WITH ANY TUNER!"))
            _props = self.query([CF.DataType(id='FRONTEND::tuner_status',value=_any.to_any(None))])
            return _props[0].value._v[tuner_id]._v

        def assignListener(self,listen_alloc_id, allocation_id):
            # find control allocation_id
            existing_alloc_id = allocation_id
            if self.listeners.has_key(existing_alloc_id):
                existing_alloc_id = self.listeners[existing_alloc_id]
            self.listeners[listen_alloc_id] = existing_alloc_id

#{% if component.hasmultioutport %}
            old_table = self.connectionTable
            new_entries = []
            for entry in self.connectionTable:
                if entry.connection_id == existing_alloc_id:
                    tmp = bulkio.connection_descriptor_struct()
                    tmp.connection_id = listen_alloc_id
                    tmp.stream_id = entry.stream_id
                    tmp.port_name = entry.port_name
                    new_entries.append(tmp)

            for new_entry in new_entries:
                foundEntry = False
                for entry in self.connectionTable:
                    if entry.connection_id == new_entry.connection_id and \
                       entry.stream_id == entry.stream_id and \
                       entry.port_name == entry.port_name:
                        foundEntry = True
                        break

                if not foundEntry:
                    self.connectionTable.append(new_entry)

            self.connectionTableChanged(old_table, self.connectionTable)
#{% endif %}

#{% if component.hasmultioutport %}
        def connectionTableChanged(self, oldValue, newValue):
#{%   for port in component.ports if port.multiout %}
            self.${port.pyname}.updateConnectionFilter(newValue)
#{%   endfor %}
#{% endif %}

        def removeListener(self,listen_alloc_id):
            if self.listeners.has_key(listen_alloc_id):
                del self.listeners[listen_alloc_id]

#{% if component.hasmultioutport %}
#{%   for port_out in component.ports if port_out.multiout %}
            # Check to see if port "${port_out.pyname}" has a connection for this listener
            tmp = self.${port_out.pyname}._get_connections()
            for i in range(len(self.${port_out.pyname}._get_connections())):
                connection_id = tmp[i].connectionId
                if connection_id == listen_alloc_id:
                    self.${port_out.pyname}.disconnectPort(connection_id)
#{%   endfor %}

            old_table = self.connectionTable
            for entry in list(self.connectionTable):
                if entry.connection_id == listen_alloc_id:
                    self.connectionTable.remove(entry)

            self.connectionTableChanged(old_table, self.connectionTable)
#{% endif %}

#{% endif %}
#{% if component.hasmultioutport %}

        def removeAllocationIdRouting(self,tuner_id):
            allocation_id = self.getControlAllocationId(tuner_id)
            old_table = self.connectionTable
            for entry in list(self.connectionTable):
                if entry.connection_id == allocation_id:
                    self.connectionTable.remove(entry)

            for key,value in self.listeners.items():
                if (value == allocation_id):
                    for entry in list(self.connectionTable):
                        if entry.connection_id == key:
                            self.connectionTable.remove(entry)

            self.connectionTableChanged(old_table, self.connectionTable)

        def removeStreamIdRouting(self,stream_id, allocation_id):
            old_table = self.connectionTable
            for entry in list(self.connectionTable):
                if allocation_id == "":
                    if entry.stream_id == stream_id:
                        self.connectionTable.remove(entry)
                else:
                    if entry.stream_id == stream_id and entry.connection_id == allocation_id:
                        self.connectionTable.remove(entry)

            for key,value in self.listeners.items():
                if (value == allocation_id):
                    for entry in list(self.connectionTable):
                        if entry.connection_id == key and entry.stream_id == stream_id:
                            self.connectionTable.remove(entry)

            self.connectionTableChanged(old_table, self.connectionTable)

        def matchAllocationIdToStreamId(self,allocation_id, stream_id, port_name):
            if port_name != "":
                for entry in list(self.connectionTable):
                    if entry.port_name != port_name:
                        continue
                    if entry.stream_id != stream_id:
                        continue
                    if entry.connection_id != allocation_id:
                        continue
                    # all three match. This is a repeat
                    return

                old_table = self.connectionTable;
                tmp = bulkio.connection_descriptor_struct()
                tmp.connection_id = allocation_id
                tmp.port_name = port_name
                tmp.stream_id = stream_id
                self.connectionTable.append(tmp)
                self.connectionTableChanged(old_table, self.connectionTable)
                return

            old_table = self.connectionTable;
            tmp = bulkio.connection_descriptor_struct()
#{%   for port in component.ports if port.multiout %}
            tmp.connection_id = allocation_id
            tmp.port_name = "${port.portname}"
            tmp.stream_id = stream_id
            self.connectionTable.append(tmp)
#{%   endfor %}
            self.connectionTableChanged(old_table, self.connectionTable)
#{% else %}
        def removeAllocationIdRouting(self,tuner_id):
            pass
#{% endif %}
#{% endblock %}
