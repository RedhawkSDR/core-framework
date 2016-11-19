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
ACLOCAL_AMFLAGS = -I m4 -I${OSSIEHOME}/share/aclocal/ossie
AUTOMAKE_OPTIONS = subdir-objects

check_cwd_java_jar_CLASSPATH = $(SOFTPKG_CLASSPATH):$(REDHAWK_CLASSPATH):

check_cwd_java.jar$(EXEEXT): $(check_cwd_java_jar_SOURCES)
	mkdir -p bin
	$(JAVAC) -cp $(check_cwd_java_jar_CLASSPATH) -g -d bin $(check_cwd_java_jar_SOURCES)
	$(JAR) cf ./check_cwd_java.jar -C bin .
	$(JAR) uf ./check_cwd_java.jar -C src .

clean-local:
	rm -rf bin 

check_cwd_java_jar_SOURCES := $(shell find ./src -name "*.java")

ossieName = check_cwd_java
noinst_PROGRAMS = check_cwd_java.jar
