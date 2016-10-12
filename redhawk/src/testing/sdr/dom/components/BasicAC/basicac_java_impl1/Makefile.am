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

# vim: noet: softtabstop=0
if HAVE_JAVASUPPORT

BasicAC_java_impl1.jar:
	mkdir -p bin
	find ./src -name "*.java" > fileList.txt
	$(JAVAC) -cp $(OSSIE_CLASSPATH):../../javaDep/javaDep/javaDep.jar -d bin @fileList.txt
	$(JAR) cf ./BasicAC_java_impl1.jar -C bin .
	rm fileList.txt

clean-local:
	rm -rf bin 

BasicAC_java_impl1_jar_SOURCES := $(shell find . -name "*.java")

ossieName = BasicAC_java_impl1
noinst_PROGRAMS = BasicAC_java_impl1.jar

else

all-local:
	@echo "Java support disabled - BasicAC_java_impl1 will not be compiled"

endif

