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

def CodegenPackageLoader(package):
    module = __import__(package)
    for name in package.split('.')[1:]:
        module = getattr(module, name)
    if hasattr(module, "loader"):
        return module.loader
    else:
        return jinja2.PackageLoader(package)

def CodegenLoader(default, depends=None):
    default = jinja2.PackageLoader(default)
    if not depends:
        return default
    prefix = jinja2.PrefixLoader(dict((n, CodegenPackageLoader(p)) for n, p in depends.iteritems()))
    return jinja2.ChoiceLoader([prefix, default])
