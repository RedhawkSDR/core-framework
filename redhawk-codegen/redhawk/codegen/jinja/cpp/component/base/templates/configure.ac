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
#{% block license %}
#{# Allow child templates to include license #}
#{% endblock %}
#{% block acInit %}
AC_INIT({{component.name}}, {{component.version}})
AM_INIT_AUTOMAKE([nostdinc foreign])
AC_CONFIG_MACRO_DIR([m4])
#{% if component.impl.module %}
LT_INIT([dlopen])
#{% endif %}
#{% endblock %}

#{% block acChecks %}
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
#{% endblock %}

#{% block acOssie %}
AC_CORBA_ORB
OSSIE_CHECK_OSSIE
OSSIE_SDRROOT_AS_PREFIX
#{% endblock %}

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Dependencies
#{% block pkgConfigPath %}
#{% endblock %}
#{% block projectDeps %}
PKG_CHECK_MODULES([PROJECTDEPS], [ossie >= {{versions.redhawk}} omniORB4 >= {{versions.omniORB4}}#{% endblock %}])
#{% block interfaceDeps %}
#{% if component.interfacedeps %}
PKG_CHECK_MODULES([INTERFACEDEPS], [{{component.interfacedeps|join(', ')}}])
#{% endif %}
#{% endblock %}
#{% block softpkgDeps %}
#{% for dep in component.impl.softpkgdeps %}
RH_SOFTPKG_CXX([{{dep.spd}}]{{',[%s]' % dep.impl if dep.impl}})
#{% endfor %}
#{% endblock %}
#{% block coreDeps %}
OSSIE_ENABLE_LOG4CXX
AX_BOOST_BASE([{{versions.boost}}])
AX_BOOST_SYSTEM
AX_BOOST_THREAD
AX_BOOST_REGEX
#{% endblock %}

#{% block acConfig %}
AC_CONFIG_FILES([
#%- filter trim|lines|join(' \\\n')|indent(17)
Makefile
#%- block files
#% endblock
#% endfilter
])
AC_OUTPUT
#{% endblock %}
