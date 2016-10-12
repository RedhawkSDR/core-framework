CFDIR = $(top_srcdir)/base

noinst_PROGRAMS = comp

comp_SOURCES = comp.cpp comp.h comp_base.cpp comp_base.h main.cpp
comp_CXXFLAGS = -Wall $(BOOST_CPPFLAGS) -I$(CFDIR)/include
comp_LDADD = $(BOOST_REGEX_LIB) $(BOOST_SYSTEM_LIB) $(BOOST_THREAD_LIB) $(OMNIDYNAMIC_LIBS) $(OMNICOS_LIBS) $(CFDIR)/framework/libossiecf.la $(CFDIR)/framework/idl/libossieidl.la
