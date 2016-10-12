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
AM_LDFLAGS = -lrt 
AM_CPPFLAGS = $(OMNITHREAD_CFLAGS) -I $(top_srcdir)/common
reader_LDADD = $(OMNITHREAD_LIBS)
writer_LDADD = $(OMNITHREAD_LIBS)

noinst_PROGRAMS = reader writer

reader_SOURCES = reader.cpp control.cpp
writer_SOURCES = writer.cpp control.cpp
