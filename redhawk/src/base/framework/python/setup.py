#! /usr/bin/env python3
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

from setuptools import setup, find_packages
from distutils import sysconfig
import sys

try:
    import tabnanny
except ImportError:
    pass
else:
    import io
    stdout, stderr = sys.stdout, sys.stderr
    sys.stdout = co = io.StringIO()
    sys.stderr = ce = io.StringIO()
    # Tabnanny doesn't provide any mechanism other than print outs so we have
    # to capture the output
    tabnanny.check("ossie")
    sys.stdout = stdout
    sys.stderr = stderr
    if len(co.getvalue().strip()) != 0:
        print("Incosistent tab usage:")
        print(co.getvalue())
        sys.exit(-1)

import ossie.version

setup(name='ossiepy',
      version=ossie.version.__version__,
      description='OSSIE Python3',
      author="Redhawk Deployer",
      author_email="redhawksdr@redhawksdr.org",
      classifiers=[
          'Development Status :: 5  - Production/Stable',
          'License:: OSI Approved :: GNU LESSER GENERAL PUBLIC LICENSE',
          'Programming Language :: Python :: 3',
          'Programming Language :: Python :: 3.6',
          "Operating System :: OS Independent",          
          ],

      packages=find_packages(include=[ "ossie", "ossie.*", "redhawk", "redhawk.*"]),
    
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
      entry_points={'redhawk.sandbox.helpers':['SoundSink=ossie.utils.sb.audio:SoundSink',
                                               'LinePlot=ossie.utils.sb.plots:LinePlot',
                                               'LinePSD=ossie.utils.sb.plots:LinePSD',
                                               'RasterPlot=ossie.utils.sb.plots:RasterPlot',
                                               'RasterPSD=ossie.utils.sb.plots:RasterPSD',
                                               'XYPlot=ossie.utils.sb.plots:XYPlot']},
      install_requires=[
          'PyQt5',
          'jinja2',
          'numpy',
          'lxml',
          ]
      )
