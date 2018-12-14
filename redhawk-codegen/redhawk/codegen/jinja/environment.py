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

import jinja2

class CodegenEnvironment(jinja2.Environment):
    def __init__(self, *args, **kwargs):
        super(CodegenEnvironment,self).__init__(*args, **kwargs)

        import filters
        self.filters['lines'] = filters.do_lines
        self.filters['prepend'] = filters.do_prepend
        self.filters['append'] = filters.do_append
        self.filters['relpath'] = filters.do_relpath
        self.filters['unique'] = filters.do_unique
        self.filters['filter'] = filters.do_filter
        self.filters['test'] = filters.do_test
        self.filters['quote'] = filters.do_quote
        self.filters['codealign'] = filters.do_codealign

        import tests
        self.tests['sometimes'] = tests.is_sometimes
        self.tests['always'] = tests.is_always
        self.tests['never'] = tests.is_never
        self.tests['resource'] = tests.is_resource
        self.tests['device'] = tests.is_device
        self.tests['loadabledevice'] = tests.is_loadabledevice
        self.tests['executabledevice'] = tests.is_executabledevice
        self.tests['aggregatedevice'] = tests.is_aggregatedevice
        self.tests['programmabledevice'] = tests.is_programmabledevice
        self.tests['service'] = tests.is_service
        self.tests['simple'] = tests.is_simple
        self.tests['simplesequence'] = tests.is_simplesequence
        self.tests['struct'] = tests.is_struct
        self.tests['structsequence'] = tests.is_structsequence
        self.tests['enumerated'] = tests.is_enumerated
        self.tests['provides'] = tests.is_provides
        self.tests['uses'] = tests.is_uses
        self.tests['bidir'] = tests.is_bidir
