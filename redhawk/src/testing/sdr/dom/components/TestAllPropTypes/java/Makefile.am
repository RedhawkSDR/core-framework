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

 
TestAllPropTypes.jar: $(TestAllPropTypes_jar_SOURCES)
	mkdir -p bin
	$(JAVAC) -cp ../../../../../../base/framework/java/CFInterfaces.jar:../../../../../../base/framework/java/log4j-1.2.15.jar:../../../../../../base/framework/java/ossie.jar -d bin $(TestAllPropTypes_jar_SOURCES)
	$(JAR) cf ./TestAllPropTypes.jar -C bin .

clean-local:
	rm -rf bin

ossieName = TestAllPropTypes
bindir = $(prefix)/dom/components/TestAllPropTypes/java/
noinst_PROGRAMS = TestAllPropTypes.jar
TestAllPropTypes_jar_SOURCES := $(shell find ./src -name "*.java")

xmldir = $(prefix)/dom/components/TestAllPropTypes/

domdir = $(prefix)/dom/components/TestAllPropTypes/java/
