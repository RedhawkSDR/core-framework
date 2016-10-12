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
#% set target = automake.canonicalName(component.jarfile)
#% set componentdir = component.name.replace('.','/')
#% set classpath = component.interfacejars
ACLOCAL_AMFLAGS = -I m4 -I${OSSIEHOME}/share/aclocal/ossie
AUTOMAKE_OPTIONS = subdir-objects

{{target}}_CLASSPATH = $(SOFTPKG_CLASSPATH):$(REDHAWK_CLASSPATH):{{classpath|prepend('$(OSSIE_HOME)/lib/')|join(':')}}

{{component.jarfile}}$(EXEEXT): $({{target}}_SOURCES)
	mkdir -p bin
	$(JAVAC) -cp $({{target}}_CLASSPATH) -g -d bin $({{target}}_SOURCES)
	$(JAR) cf ./{{component.jarfile}} -C bin .
	$(JAR) uf ./{{component.jarfile}} -C src .

clean-local:
	rm -rf bin

distclean-local:
	rm -rf m4
	rm -f config.*
	rm -rf autom4te.cache
	rm -f acinclude.m4
	rm -f aclocal.m4
	rm -f configure
	rm -f depcomp
	rm -f install-sh
	rm -f ltmain.sh
	rm -f Makefile.in
	rm -f missing
	rm -rf .deps

#{% set outputdir = generator.getOutputDir() %}
ossieName = {{component.name}}
bindir = $(prefix)/{{component.sdrpath}}/{{componentdir}}/{{outputdir}}/
bin_PROGRAMS = {{component.jarfile}}
{{target}}_SOURCES := $(shell find ./src -name "*.java")

xmldir = $(prefix)/{{component.sdrpath}}/{{componentdir}}/
dist_xml_DATA = {{component.profile.values()|relpath(outputdir)|join(' ')}}

#{% set sdrbranch = component.sdrpath.split('/')[0] %}
{{sdrbranch}}dir = $(prefix)/{{component.sdrpath}}/{{componentdir}}/{{outputdir}}/
dist_{{sdrbranch}}_SCRIPTS = {{component.impl.entrypoint|relpath(outputdir)}}
