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

bin_SCRIPTS = redhawk-softpkg
bin_PROGRAMS = redhawk-shminfo redhawk-shmclean

OSSIE_LIBS = $(top_builddir)/base/framework/libossiecf.la $(top_builddir)/base/framework/idl/libossieidl.la

redhawk_shminfo_SOURCES = src/shminfo.cpp src/ShmVisitor.cpp src/ShmVisitor.h
redhawk_shminfo_CXXFLAGS = -Wall $(OSSIE_CFLAGS)
redhawk_shminfo_LDFLAGS = $(OSSIE_LIBS)

redhawk_shmclean_SOURCES = src/shmclean.cpp src/ShmVisitor.cpp src/ShmVisitor.h
redhawk_shmclean_CXXFLAGS = -Wall $(OSSIE_CFLAGS)
redhawk_shmclean_LDFLAGS = $(OSSIE_LIBS) -lrt
