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
CFDIR = $(top_srcdir)/base

noinst_PROGRAMS = GPP

GPP_SOURCES = GPP.cpp \
GPP.h \
GPP_base.cpp \
GPP_base.h \
NicAllocator.cpp \
NicAllocator.h \
NicFacade.cpp \
NicFacade.h \
NicInterfaceFilter.cpp \
NicInterfaceFilter.h \
main.cpp \
reports/NicThroughputThresholdMonitor.cpp \
reports/NicThroughputThresholdMonitor.h \
reports/FreeMemoryThresholdMonitor.cpp \
reports/FreeMemoryThresholdMonitor.h \
reports/Reporting.h \
reports/ThresholdMonitor.h \
reports/SystemMonitorReporting.cpp \
reports/SystemMonitorReporting.h \
reports/CpuThresholdMonitor.cpp \
reports/CpuThresholdMonitor.h \
parsers/PidProcStatParser.cpp \
parsers/PidProcStatParser.h \
parsers/ProcStatFileParser.cpp \
parsers/ProcStatFileParser.h \
parsers/ProcStatParser.cpp \
parsers/ProcStatParser.h \
parsers/ProcMeminfoParser.cpp \
parsers/ProcMeminfoParser.h \
states/NicState.cpp \
states/NicState.h \
states/Limits.cpp \
states/Limits.h \
states/State.h \
states/CpuState.cpp \
states/CpuState.h \
states/ProcStat.cpp \
states/ProcStat.h \
states/ProcMeminfo.cpp \
states/ProcMeminfo.h \
statistics/CpuUsageAccumulator.cpp \
statistics/CpuUsageAccumulator.h \
statistics/CpuUsageStats.cpp \
statistics/CpuUsageStats.h \
statistics/NicAccumulator.cpp \
statistics/NicAccumulator.h \
statistics/Statistics.h \
struct_props.h \
utils/popen.cpp \
utils/popen.h \
utils/affinity.cpp \
utils/affinity.h \
utils/CmdlineExecutor.cpp \
utils/CmdlineExecutor.h \
utils/EnvironmentPathParser.cpp \
utils/EnvironmentPathParser.h \
utils/EventDispatcher.h \
utils/FileReader.cpp \
utils/FileReader.h \
utils/IOError.h \
utils/OverridableSingleton.h \
utils/ReferenceWrapper.h \
utils/SymlinkReader.cpp \
utils/SymlinkReader.h
GPP_CXXFLAGS = -Wall $(BOOST_CPPFLAGS) -I$(CFDIR)/include -I$(CFDIR)/include/ossie
GPP_LDADD = $(BOOST_REGEX_LIB) $(BOOST_SYSTEM_LIB) $(BOOST_FILESYSTEM_LIB) $(BOOST_THREAD_LIB) -lboost_iostreams $(OMNIDYNAMIC_LIBS) $(OMNICOS_LIBS) $(CFDIR)/framework/libossiecf.la $(CFDIR)/framework/idl/libossieidl.la
