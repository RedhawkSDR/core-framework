CFDIR = $(top_srcdir)/base

noinst_PROGRAMS = cpp_wb_receiver

include $(srcdir)/Makefile.am.ide
cpp_wb_receiver_SOURCES = $(redhawk_SOURCES_auto)
cpp_wb_receiver_CXXFLAGS = -Wall $(BOOST_CPPFLAGS) -I$(CFDIR)/include -I$(CFDIR)/include/ossie
cpp_wb_receiver_LDADD = $(BOOST_REGEX_LIB) $(BOOST_SYSTEM_LIB) $(BOOST_THREAD_LIB) $(OMNIDYNAMIC_LIBS) $(OMNICOS_LIBS) $(CFDIR)/framework/libossiecf.la $(CFDIR)/framework/idl/libossieidl.la

