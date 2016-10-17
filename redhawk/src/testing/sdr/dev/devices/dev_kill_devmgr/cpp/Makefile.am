CFDIR = $(top_srcdir)/base

noinst_PROGRAMS = dev_kill_devmgr

dev_kill_devmgr_SOURCES = dev_kill_devmgr.cpp dev_kill_devmgr.h dev_kill_devmgr_base.cpp dev_kill_devmgr_base.h main.cpp
dev_kill_devmgr_CXXFLAGS = -Wall $(BOOST_CPPFLAGS) -I$(CFDIR)/include
dev_kill_devmgr_LDADD = $(BOOST_REGEX_LIB) $(BOOST_SYSTEM_LIB) $(BOOST_THREAD_LIB) $(OMNIDYNAMIC_LIBS) $(OMNICOS_LIBS) $(CFDIR)/framework/libossiecf.la $(CFDIR)/framework/idl/libossieidl.la

