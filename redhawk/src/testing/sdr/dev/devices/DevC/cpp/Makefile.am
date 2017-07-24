
CFDIR = $(top_srcdir)/base

noinst_PROGRAMS = DevC

DevC_SOURCES = DevC.cpp DevC.h DevC_base.cpp DevC_base.h main.cpp
DevC_CXXFLAGS = -Wall $(BOOST_CPPFLAGS) -I$(CFDIR)/include
DevC_LDADD = $(BOOST_REGEX_LIB) $(BOOST_SYSTEM_LIB) $(BOOST_THREAD_LIB) $(OMNIDYNAMIC_LIBS) $(OMNICOS_LIBS) $(CFDIR)/framework/libossiecf.la $(CFDIR)/framework/idl/libossieidl.la

