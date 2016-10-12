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
AC_INIT({{component.name}}, {{component.version}})
AM_INIT_AUTOMAKE([nostdinc foreign])
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
LT_INIT([disable-static])

AC_CORBA_ORB
OSSIE_CHECK_OSSIE
RH_SOFTPKG_PREFIX([{{component.name}}],[{{component.impl.id}}])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Dependencies
PKG_CHECK_MODULES([REDHAWK], [ossie >= {{versions.redhawk}}])
#{% for dep in component.impl.softpkgdeps %}
RH_SOFTPKG_CXX([{{dep.spd}}]{{',[%s]' % dep.impl if dep.impl}})
#{% endfor %}
OSSIE_ENABLE_LOG4CXX
AX_BOOST_BASE([{{versions.boost}}])
AX_BOOST_SYSTEM
AX_BOOST_THREAD
AX_BOOST_REGEX

AC_CONFIG_FILES([Makefile {{component.pcfile}}])
AC_OUTPUT
