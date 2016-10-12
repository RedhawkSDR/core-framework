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

OSSIE_CHECK_OSSIE
OSSIE_SDRROOT_AS_PREFIX

PKG_CHECK_MODULES([OSSIE], [ossie >= {{versions.redhawk}}])

RH_JAVA_HOME
RH_PROG_JAVAC([{{versions.java}}])
RH_PROG_JAR

RH_PKG_CLASSPATH([REDHAWK], [ossie])
#{% if component.interfacedeps %}
PKG_CHECK_MODULES([INTERFACEDEPS], [{{component.interfacedeps|join(', ')}}])
#{% endif %}
#{% for dep in component.impl.softpkgdeps %}
RH_SOFTPKG_JAVA([{{dep.spd}}]{{',[%s]' % dep.impl if dep.impl}})
#{% endfor %}

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
