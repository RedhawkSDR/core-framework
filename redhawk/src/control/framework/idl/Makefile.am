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
%SK.cpp %.h %DynSK.cpp : ../../idl/ossie/CF/%.idl
	$(IDL) -I $(IDLDIR) -I $(IDLDIR)/COS -I ../../idl -bcxx -Wba -Wbd=DynSK.cpp -Wbh=.h -Wbs=SK.cpp -Wbkeep_inc_path $<
# omniORB uses the a ".hh" suffix for its COS header files, but omniidl doesn't
# know the difference, so we must modify generated headers to account for that.
	$(SED) 's|<COS/\(.*\)\.h>|<COS/\1.hh>|' $*.h > $*.h.tmp
	mv $*.h.tmp $*.h
	mkdir -p ../../include/ossie/CF
	cp $*.h ../../include/ossie/CF

clean-local:
	rm -f ../../include/ossie/CF/*.h

BUILT_SOURCES = cfSK.cpp \
		cfDynSK.cpp \
                PortTypesSK.cpp \
		PortTypesDynSK.cpp \
		StandardEventSK.cpp \
		StandardEventDynSK.cpp \
		AggregateDevicesSK.cpp \
		AggregateDevicesDynSK.cpp \
		ExtendedEventSK.cpp \
		ExtendedEventDynSK.cpp \
		QueryablePortSK.cpp \
		QueryablePortDynSK.cpp \
		WellKnownPropertiesSK.cpp \
		WellKnownPropertiesDynSK.cpp \
		sandboxSK.cpp \
		sandboxDynSK.cpp
CLEANFILES = $(BUILT_SOURCES)

noinst_LTLIBRARIES = libossieidl.la
libossieidl_la_SOURCES = $(BUILT_SOURCES)
libossieidl_la_CXXFLAGS = -Wall $(OMNIORB_CFLAGS) $(OMNITHREAD_CFLAGS) $(OMNIDYNAMIC_CFLAGS) -I../../include
libossieidl_la_LIBADD = $(OMNIORB_LIBS) $(OMNITHREAD_LIBS) $(OMNIDYNAMIC_LIBS)
libossieidl_la_LDFLAGS = -Wall
