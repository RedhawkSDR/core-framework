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

# Ensure that sub-modules in this package will work by trying to import the
# jinja2 module. First, try to configure the path using setuptools, which works
# with Jinja2 installed from EPEL-6. If the import still fails for some reason,
# add the expected Python egg path to sys.path and try again.

from redhawk.codegen import versions
import sys

def import_fail(msg=''):
    raise ImportError('requires Jinja2 '+versions.jinja2+' or later'+msg)

def getdist(env):
    v_tuple = tuple(int(x) for x in versions.jinja2.split('.'))
    jinja_dists = env.__getitem__('Jinja2')
    version_found = None
    for dist in jinja_dists:
        if dist.has_version():
            chk_tuple = tuple(int(x) for x in dist.version.split('.'))
            if v_tuple <= chk_tuple:
                return dist,None
            else:
                version_found = dist.version
    return None, version_found
    

# Try to use setuptools to locate Jinja2 and configure the path
try:
    import pkg_resources
except ImportError:
    # setuptools is not installed; we'll import without a check
    pass
else:
    try:
        environment = pkg_resources.Environment()
        dist,msg = getdist(pkg_resources.Environment())
        if dist == None:
            import_fail(msg)
        pkg_resources.working_set.by_key['jinja2'] = dist
        sys.path.insert(0,dist.location)
    except pkg_resources.DistributionNotFound:
        import_fail()
    except pkg_resources.VersionConflict, e:
        import_fail(', but found ' + str(e.args[0]))

try:
    import jinja2
except ImportError:
    # With RHEL 5/6, Jinja2 2.6 is parallel-installed as an egg. We explicitly
    # add it to the path if present to ensure it gets picked up.
    import os
    import sys
    pyver = sys.version[:3]
    site_packages = os.path.join(sys.prefix, 'lib/python'+pyver, 'site-packages')
    eggfile = os.path.join(site_packages, 'Jinja2-'+versions.jinja2+'-py'+pyver+'.egg')
    if os.path.exists(eggfile):
        sys.path.insert(0, eggfile)
    try:
        import jinja2
    except ImportError:
        import_fail()
