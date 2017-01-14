#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK code-generator.
#
# REDHAWK code-generator is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK code-generator is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

class GeneratorModule(object):
    def __init__(self, module):
        self.__module = module

    def factory(self, **opts):
        return self.__module.factory(**opts)

    def check(self):
        # If the module provides a check function, call it here
        if hasattr(self.__module, 'check'):
            return self.__module.check()
        else:
            return True

def importTemplate(template):
    """
    Imports a code generation module from the given fully-qualified name.
    """
    try:
        package = __import__(template)
    except Exception, e:
        print e
        raise

    # Since the module name probably has dots, get the most specific module
    # (e.g. 'component' from 'template.cpp.component').
    for name in template.split('.')[1:]:
        package = getattr(package, name)

    if not hasattr(package, 'factory'):
        raise TypeError('Invalid template ' + template)

    return GeneratorModule(package)
