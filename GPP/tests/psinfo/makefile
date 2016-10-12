

## control for numa build
NUMA_CFLAGS=-DHAVE_LIBNUMA=1
NUMA_LIBS=-lnuma

src_topdir=../../cpp
CXX=g++
GDEBUG=
CXXFLAGS=$(GDEBUG) -I$(src_topdir)/parsers -I$(src_topdir)/states -I$(src_topdir) $(NUMA_CFLAGS)
OBJS=psinfo.o  $(src_topdir)/parsers/ProcStatParser.o $(src_topdir)/states/ProcStat.o  $(src_topdir)/parsers/ProcMeminfoParser.o $(src_topdir)/states/ProcMeminfo.o $(src_topdir)/statistics/CpuUsageStats.o
#non-mt LIBS+=$(NUMA_LIBS) -lboost_date_time -lboost_system -lboost_thread
#mt LIBS+=$(NUMA_LIBS) -lboost_date_time-mt -lboost_system-mt -lboost_thread-mt
LIBS+=$(NUMA_LIBS) -lboost_date_time -lboost_system -lboost_thread

all: psinfo

clean:
	rm *.o psinfo

psinfo: $(OBJS)
	$(CXX) -o psinfo $(GDEBUG) $(OBJS) $(LIBS)

.cc.o:
	$(CXX) $(CXXFLAGS) $< -c -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) $< -c -o $@
