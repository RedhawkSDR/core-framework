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

from xml.dom import minidom

_templateMapping = {
    'gov.redhawk.ide.codegen.jet.cplusplus.PullPortDataTemplate': 'redhawk.codegen.jinja.cpp.component.pull',
    'gov.redhawk.ide.codegen.jet.python.pattern.PullPortDataTemplate': 'redhawk.codegen.jinja.python.component.pull',
    'gov.redhawk.ide.codegen.jet.java.JavaGenerator': 'redhawk.codegen.jinja.java.component.pull',
    'gov.redhawk.ide.codegen.jet.python.pattern.MinimalServiceTemplate': 'redhawk.codegen.jinja.python.service',
    'gov.redhawk.ide.codegen.jet.java.MinimalServiceTemplate': 'redhawk.codegen.jinja.java.service'
}

def _xmlElementsToDict(node, tagname, key, value):
    return dict((str(n.getAttribute(key)), n.getAttribute(value)) for n in node.getElementsByTagName(tagname))

class ImplementationSettings(object):
    def __init__(self, name=None, outputDir=None, template=None, properties={}, crcs={}):
        self.name = name
        self.outputDir = outputDir
        self.template = template
        self.properties = properties
        self.generatedFileCRCs = crcs

    def override(self, other):
        if other.name is not None:
            self.name = other.name
        if other.outputDir is not None:
            self.outputDir = other.outputDir
        if other.template is not None:
            self.template = other.template
        self.properties.update(other.properties)
        self.generatedFileCRCs.update(other.generatedFileCRCs)

def _importWavedevImplSettings(node):
    if node.getAttribute('generatorId').startswith('gov.redhawk.ide.codegen.jinja') or node.getAttribute('generatorId').startswith('redhawk.codegen.jinja'):
        template = node.getAttribute('template')
    else:
        template = _templateMapping[node.getAttribute('template')]
    return ImplementationSettings(name=node.getAttribute('name'),
                                  outputDir=node.getAttribute('outputDir'),
                                  template=template,
                                  properties=_xmlElementsToDict(node, 'properties', 'id', 'value'),
                                  crcs=_xmlElementsToDict(node, 'generatedFileCRCs', 'file', 'crc'))

def importWavedevSettings(filename):
    dom = minidom.parse(filename)
    settings = {}
    for node in dom.getElementsByTagName('implSettings'):
        implId = node.getAttribute('key')
        settings[implId] = _importWavedevImplSettings(node.getElementsByTagName('value')[0])
    dom.unlink()
    return settings
