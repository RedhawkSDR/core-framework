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
%SK.cpp %.h %DynSK.cpp : $(top_srcdir)/idl/ossie/CF/%.idl
	$(AM_V_GEN)$(IDL) -I $(IDLDIR) -I $(IDLDIR)/COS -I $(top_srcdir)/idl -bcxx -Wba -Wbd=DynSK.cpp -Wbh=.h -Wbs=SK.cpp -Wbkeep_inc_path $<
# omniORB uses the a ".hh" suffix for its COS header files, but omniidl doesn't
# know the difference, so we must modify generated headers to account for that.
	$(AM_V_at)$(SED) 's|<COS/\(.*\)\.h>|<COS/\1.hh>|' $*.h > $*.h.tmp
	$(AM_V_at)mv $*.h.tmp $*.h
	$(AM_V_at)mkdir -p ../../include/ossie/CF
	$(AM_V_at)cp $*.h ../../include/ossie/CF

clean-local:
	rm -f ../../include/ossie/CF/*.h

install-data-hook :
	touch $(DESTDIR)$(pkgincludedir)/CF/*.h

BUILT_SOURCES =	cfSK.cpp \
		cfDynSK.cpp \
                DataTypeSK.cpp \
		DataTypeDynSK.cpp \
                PortSK.cpp \
		PortDynSK.cpp \
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
		sandboxDynSK.cpp \
		LogInterfacesSK.cpp \
		LogInterfacesDynSK.cpp \
		EventChannelManagerSK.cpp \
		EventChannelManagerDynSK.cpp

CLEANFILES = $(BUILT_SOURCES)


cfheaderdir = $(pkgincludedir)/CF
dist_cfheader_HEADERS = LogInterfaces.h EventChannelManager.h cf.h DataType.h Port.h PortTypes.h StandardEvent.h AggregateDevices.h ExtendedEvent.h QueryablePort.h WellKnownProperties.h sandbox.h
#nodist_pkginclude_HEADERS = cf.h PortTypes.h StandardEvent.h

lib_LTLIBRARIES = libossieidl.la
nodist_libossieidl_la_SOURCES = $(BUILT_SOURCES)
libossieidl_la_CXXFLAGS = -Wall $(OMNIORB_CFLAGS) $(OMNICOS_CFLAGS) $(OMNITHREAD_CFLAGS) $(OMNIDYNAMIC_CFLAGS) -I../../include
libossieidl_la_LIBADD = $(OMNIORB_LIBS) $(OMNITHREAD_LIBS) $(OMNIDYNAMIC_LIBS)
libossieidl_la_LDFLAGS = -Wall -version-info $(LIBOSSIECF_VERSION_INFO)
