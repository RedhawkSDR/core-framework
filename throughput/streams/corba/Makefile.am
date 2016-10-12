#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK throughput.
#
# REDHAWK throughput is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK throughput is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
AM_CPPFLAGS = $(OMNIORB_CFLAGS) -I $(top_srcdir)/common

noinst_PROGRAMS = reader writer

reader_SOURCES = reader.cpp rawdataSK.cpp
writer_SOURCES = writer.cpp rawdataSK.cpp

reader_LDADD = $(OMNIORB_LIBS)
writer_LDADD = $(OMNIORB_LIBS)

BUILT_SOURCES = rawdataSK.cpp rawdata.h rawdata/__init__.py
CLEANFILES = $(BUILT_SOURCES) rawdata_idl.py

%SK.cpp %.h: idl/%.idl
	omniidl -bcxx -Wbh=.h -Wbs=SK.cpp $<

%/__init__.py: idl/%.idl
	omniidl -bpython -C ../.. -Wbpackage=streams.corba $<

clean-local:
	-rm -rf rawdata rawdata__POA
