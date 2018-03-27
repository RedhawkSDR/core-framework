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

from ossie.utils.log4py import logging

_log = logging.getLogger(__name__)
_plugins = None

def _load_plugins():
    global _plugins
    if _plugins is not None:
        return

    _plugins = []
    import pkg_resources
    for entry_point in pkg_resources.iter_entry_points('redhawk.sandbox.helpers'):
        _log.trace("Loading plugin '%s'", entry_point.name)
        try:
            plugin = entry_point.load()
        except:
            # Ignore errors in plugin load
            _log.exception("Failed to load plugin '%s'", entry_point.name)
            continue
        _plugins.append((entry_point.name, plugin))


def plugins():
    _load_plugins()
    return _plugins[:]
