
ACLOCAL_AMFLAGS = -I m4 -I${OSSIEHOME}/share/aclocal/ossie
AUTOMAKE_OPTIONS = subdir-objects

lib_LTLIBRARIES = libossiecluster.la

xmldir = $(prefix)
dist_xml_DATA = 

distclean-local:
	rm -rf m4
	rm -rf .deps/ .libs/ Makefile Makefile.in *m4 autom4te.cache/ config.* configure depcomp install-sh libossiecluster* libtool ltmain.sh missing

libossiecluster_la_SOURCES = clustermgr.cpp {{cluster}}Resolver.cpp
libossiecluster_la_LIBADD = $(REDHAWK_LIBS) -lyaml-cpp
libossiecluster_la_CPPFLAGS = -I . -I $(srcdir)/include -I../../../include/ossie/cluster $(REDHAWK_CFLAGS) $(BOOST_CPPFLAGS)
libossiecluster_la_CXXFLAGS = -Wall
