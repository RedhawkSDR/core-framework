#! /usr/bin/env python
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


from distutils.core import setup
import sys

try:
    import tabnanny
except ImportError:
    pass
else:
    import StringIO
    stdout, stderr = sys.stdout, sys.stderr
    sys.stdout = co = StringIO.StringIO()
    sys.stderr = ce = StringIO.StringIO()
    # Tabnanny doesn't provide any mechanism other than print outs so we have
    # to capture the output
    tabnanny.check("ossie")
    sys.stdout = stdout
    sys.stderr = stderr
    if len(co.getvalue().strip()) != 0:
        print "Incosistent tab usage:"
        print co.getvalue()
        sys.exit(-1)

ossiepy = ['ossie',
           'ossie/apps',
           'ossie/apps/qtbrowse',
           'ossie/apps/rhlauncher',
           'ossie/apps/rhlauncher/ui',
           'ossie/cf',
           'ossie/cf/CF',
           'ossie/cf/CF__POA',
           'ossie/cf/PortTypes',
           'ossie/cf/PortTypes__POA',
           'ossie/cf/StandardEvent',
           'ossie/cf/StandardEvent__POA',
           'ossie/cf/ExtendedEvent',
           'ossie/cf/ExtendedEvent__POA',
           'ossie/cf/ExtendedCF',
           'ossie/cf/ExtendedCF__POA',
           'ossie/cf/ExtendedCF/WKP',
           'ossie/cf/ExtendedCF__POA/WKP',
           'ossie/logger',
           'ossie/parsers',
           'ossie/utils',
           'ossie/utils/bluefile',
           'ossie/utils/bulkio',
           'ossie/utils/log4py',
           'ossie/utils/model',
           'ossie/utils/redhawk',
           'ossie/utils/sandbox',
           'ossie/utils/sb',
           'ossie/utils/sca',
           'ossie/utils/testing',
           'ossie/utils/tools',
           'redhawk']

version='1.10.1'

setup(
        name='ossiepy',
        version=version,
        description='OSSIE Python',
        packages=ossiepy,
        package_data={'ossie/apps/rhlauncher':['ui/*.ui',
                                               'ui/icons/*']},
        scripts=['ossie/utils/tools/prf2py.py',
                 'ossie/apps/qtbrowse/qtbrowse',
                 'ossie/apps/rhlauncher/rhlauncher',
                 'ossie/apps/scaclt',
                 'ossie/apps/py2prf',
                 'ossie/apps/eventviewer',
                 'ossie/apps/rh_net_diag',
                 'ossie/apps/cleanns',
                 'ossie/apps/cleanes',
                 'ossie/apps/cleanomni',
                 'ossie/apps/sdrlint'],
    )
