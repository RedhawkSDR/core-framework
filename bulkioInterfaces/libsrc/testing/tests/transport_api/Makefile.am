
TESTS = test_transport
check_PROGRAMS = $(TESTS)
all: $(check_PROGRAMS)

test_transport_SOURCES = test_transport.cpp
test_transport_SOURCES += custom_transport.cpp
test_transport_SOURCES += test_factory.cpp
test_transport_SOURCES += test_out_manager.cpp
test_transport_SOURCES += test_in_manager.cpp
test_transport_SOURCES += test_cf_port.cpp
test_transport_SOURCES += setup_utils.cpp
test_transport_CXXFLAGS = -I $(BULKIO_CPP_LIBDIR)/vita49 $(BULKIO_CFLAGS) $(CUSTOM_CFLAGS) $(BOOST_CPPFLAGS) $(OSSIE_CFLAGS) $(CPPUNIT_CFLAGS) -ggdb
test_transport_LDADD = $(BULKIO_LIBS) $(TRANSPORT_LIBS) $(BOOST_LDFLAGS) $(BOOST_SYSTEM_LIB) $(BOOST_REGEX_LIB) $(OSSIE_LIBS) $(CPPUNIT_LIBS) $(LOG4CXX_LIBS)
