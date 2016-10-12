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

JAVA_LIBDIR = $(top_srcdir)/base/framework/java

all-local: TestJavaProps.jar

TestJavaProps.jar: TestJavaProps.java
	$(JAVAC) -source 1.5 -cp $(JAVA_LIBDIR)/CFInterfaces.jar:$(JAVA_LIBDIR)/log4j-1.2.15.jar:$(JAVA_LIBDIR)/ossie.jar TestJavaProps.java
	$(JAR) cf TestJavaProps.jar TestJavaProps.class
	$(JAR) uf TestJavaProps.jar TestJavaProps.java

clean-local:
	rm -f *.class
	rm -f TestJavaProps.jar

ossieName = TestJavaProps
noinst_PROGRAMS = TestJavaProps.jar

else

all-local:
	@echo "Java support disabled - TestJavaProps will not be compiled"

endif
