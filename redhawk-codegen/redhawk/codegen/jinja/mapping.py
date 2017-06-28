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

import os
import warnings

from redhawk.codegen.model.properties import Kinds
from redhawk.codegen.model.softwarecomponent import ComponentTypes
from redhawk.codegen.lang.idl import IDLInterface
import redhawk.codegen.model.properties

class PropertyMapper(object):
    def _mapProperty(self, prop, propclass):
        propdict = {}
        propdict['class'] = propclass
        propdict['identifier'] = prop.identifier()
        if prop.hasName():
            propdict['name'] = prop.name()
        propdict['mode'] = prop.mode()
        propdict['units'] = prop.units()
        propdict['action'] = prop.action()
        propdict['kinds'] = prop.kinds()
        propdict['hasDescription'] = prop.hasDescription()
        propdict['description'] = prop.description()
        return propdict

    def _mapSimple(self, prop):
        propdict = self._mapProperty(prop, 'simple')
        propdict['type'] = prop.type()
        if prop.hasEnumerations():
            propdict['enums'] = [self._mapEnumeration(prop, l, v) for (l, v) in prop.enumerations()]
        propdict.update(self.mapSimpleProperty(prop))
        return propdict

    def mapSimpleProperty(self, prop):
        return {}

    def _mapEnumeration(self, prop, label, value):
        enumdict = {}
        enumdict['label'] = label
        enumdict['value'] = value
        enumdict.update(self.mapEnumeration(prop, label, value))
        return enumdict

    def mapEnumeration(self, prop, label, value):
        return {}

    def _mapSimpleSequence(self, prop):
        propdict = self._mapProperty(prop, 'simplesequence')
        propdict['type'] = prop.type()
        propdict.update(self.mapSimpleSequenceProperty(prop))
        return propdict

    def mapSimpleSequenceProperty(self, prop):
        return {}

    def _mapStruct(self, prop):
        propdict = self._mapProperty(prop, 'struct')
        fields = [self._mapField(f) for f in prop.fields()]
        propdict['fields'] = fields
        propdict.update(self.mapStructProperty(prop, fields))
        return propdict

    def mapStructProperty(self, prop, fields):
        return {}

    def _mapStructSequence(self, prop):
        propdict = self._mapProperty(prop, 'structsequence')
        structdef = self._mapStruct(prop.struct())
        if self.isConnectionTable(prop):
            structdef['builtin'] = True
        propdict['structdef'] = structdef
        propdict.update(self.mapStructSequenceProperty(prop, structdef))
        return propdict

    def mapStructSequenceProperty(self, prop, structdef):
        return {}

    def _mapField(self, prop):
        # NB: This does not support struct or struct sequences as fields;
        # however, if at some point the PRF is extended to add them, 
        if prop.isStruct():
            warnings.warn('Only simple and simplesequence properties may appear in a struct')
        elif prop.isSequence():
            return self._mapSimpleSequence(prop)
        else:
            return self._mapSimple(prop)

    def mapProperties(self, softpkg):
        simple = [self._mapSimple(s) for s in softpkg.getSimpleProperties()]
        simplesequence = [self._mapSimpleSequence(s) for s in softpkg.getSimpleSequenceProperties()]
        structs = [self._mapStruct(s) for s in softpkg.getStructProperties()]
        structsequence = [self._mapStructSequence(s) for s in softpkg.getStructSequenceProperties()]

        properties = simple+simplesequence+structs+structsequence
        structdefs = structs + [p['structdef'] for p in structsequence]
        events = [prop for prop in properties if Kinds.EVENT in prop['kinds']]
        messages = [prop for prop in properties if Kinds.MESSAGE in prop['kinds']]

        return {'properties': properties,
                'structdefs': structdefs,
                'events':     events,
                'messages':   messages}

    @staticmethod
    def isConnectionTable(prop):
        if prop.name() != 'connectionTable':
            return False
        if prop.struct().name() != 'connection_descriptor':
            return False
        fields = set(field.name() for field in prop.struct().fields())
        if 'connection_id' not in fields:
            return False
        if 'stream_id' not in fields:
            return False
        if 'port_name' not in fields:
            return False
        return True


class PortMapper(object):
    def mapPort(self, port, generator):
        portdict = {}
        portdict['name'] = port.name()
        portdict['repid'] = port.repid()
        portdict['types'] = port.types()
        portdict['hasDescription'] = port.hasDescription()
        port_description = port.description()
        if port_description != None:
            port_description = port_description.replace('\n','\\n')
        portdict['description'] = port_description
        if port.isProvides():
            direction = "provides"
        else:
            direction = "uses"
        portdict['direction'] = direction
        portdict['generator'] = generator
        portdict.update(self._mapPort(port, generator))
        return portdict

    def _mapPort(self, port):
        return {}

    def mapPorts(self, softpkg, portfactory):
        ports = [self.mapPort(p, portfactory.generator(p)) for p in softpkg.ports()]
        classnames = []
        generators = []
        for p in ports:
            if 'generator' in p:
               if p['generator'].className() not in classnames:
                   classnames.append(p['generator'].className())
                   generators.append(p['generator']) 
        return {'ports':          ports,
                'portgenerators': generators}


class SoftpkgMapper(object):
    def setImplementation(self, impl=None):
        pass

    def mapSoftpkg(self, softpkg):
        component = {}
        component['name'] = softpkg.name()
        component['basename'] = softpkg.basename()
        component['version'] = softpkg.version()
        component['type'] = softpkg.type()
        component['sdrpath'] = softpkg.getSdrPath()
        component['artifacttype'] = self.artifactType(softpkg)

        # XML profile
        component['profile'] = { 'spd': softpkg.spdFile() }
        if softpkg.scdFile():
            component['profile']['scd'] = softpkg.scdFile()
        if softpkg.prfFile():
            component['profile']['prf'] = softpkg.prfFile()

        return component

    def mapComponent(self, softpkg):
        component = self.mapSoftpkg(softpkg)
        component.update(self._mapComponent(softpkg))
        return component

    def _mapComponent(self, softpkg):
        return {}

    def mapImplementation(self, impl):
        impldict = {}
        impldict['id'] = impl.identifier()
        impldict['entrypoint'] = impl.entrypoint()
        impldict['softpkgdeps'] = [self._mapSoftpkgDependency(dep) for dep in impl.softpkgdeps()]
        impldict['outputdir'] = impl.entrypoint()
        impldict.update(self._mapImplementation(impl))
        return impldict

    def _mapImplementation(self, implementation):
        return {}

    def _mapSoftpkgDependency(self, dependency):
        depdict = {}
        depdict['spd'] = dependency.spdfile
        basename = os.path.basename(dependency.spdfile)
        dirname = dependency.spdfile[:dependency.spdfile.find(basename)]
        depdict['name'] = basename.replace('.spd.xml','')
        if dirname[:6]=='/deps/':
            ns_name=dirname[6:]
            if ns_name != '/':
                depdict['name'] = ns_name.replace('/','.')[:-1]
        if dependency.impl:
            depdict['impl'] = dependency.impl
        return depdict

    def getInterfaceNamespaces(self, softpkg):
        if not softpkg.descriptor():
            return
        # The CF interfaces are already assumed as part of REDHAWK
        seen = set(['CF', 'ExtendedCF', 'ExtendedEvent'])
        for interface in softpkg.descriptor().interfaces():
            namespace = IDLInterface(interface.repid).namespace()
            # Assume that omg.org interfaces are available as part of the ORB.
            if namespace in seen or namespace.startswith('omg.org/Cos'):
                continue
            seen.add(namespace)
            yield namespace
            if namespace == 'FRONTEND':
                if 'BULKIO' not in seen:
                    seen.add('BULKIO')
                    yield 'BULKIO'

    def artifactType(self, softpkg):
        if softpkg.type() == ComponentTypes.RESOURCE:
            return 'component'
        elif softpkg.type() in (ComponentTypes.DEVICE,
                                ComponentTypes.LOADABLEDEVICE,
                                ComponentTypes.EXECUTABLEDEVICE):
            return 'device'
        elif softpkg.type() == ComponentTypes.SERVICE:
            return 'service'
        elif softpkg.type() == ComponentTypes.SHAREDPACKAGE:
            return 'sharedpackage'
        else:
            raise ValueError, 'Unsupported software component type', softpkg.type()


class ProjectMapper(SoftpkgMapper):
    def mapImplementation(self, impl, generator):
        impldict = {}
        impldict['id'] = impl.identifier()
        impldict['entrypoint'] = impl.entrypoint()
        impldict['localfile'] = impl.localfile()
        impldict['language'] = impl.programminglanguage()
        impldict['generator'] = generator
        impldict['softpkgdeps'] = [self._mapSoftpkgDependency(dep) for dep in impl.softpkgdeps()]
        if generator is not None:
            outputdir = generator.getOutputDir()
        else:
            # NB: Fall back to inferring the output directory from the entry
            #     point
            outputdir = os.path.dirname(impl.entrypoint())
        impldict['outputdir'] = outputdir
        impldict.update(self._mapImplementation(impl, generator))
        return impldict

    def _mapImplementation(self, impl, generator):
        return {}

    def mapProject(self, softpkg, generators):
        project = self.mapComponent(softpkg)
        impls = [self.mapImplementation(impl, generators.get(impl.identifier(), None)) for impl in softpkg.implementations()]
        project['implementations'] = impls
        project['languages'] = set(impl['language'] for impl in impls)
        project['subdirs'] = [impl['outputdir'] for impl in impls]
        return project


class ComponentMapper(SoftpkgMapper):
    def mapComponent(self, softpkg):
        component = self.mapSoftpkg(softpkg)
        component['license'] = None
        if softpkg.descriptor():
            if softpkg.descriptor().supports('IDL:CF/AggregateDevice:1.0'):
                component['aggregate'] = True

        component.update(self._mapComponent(softpkg))
        return component

    def _mapComponent(self, softpkg):
        return {}

    def hasMultioutPort(self, softpkg):
        for prop in softpkg.getStructSequenceProperties():
            if PropertyMapper.isConnectionTable(prop):
                return True
        return False
