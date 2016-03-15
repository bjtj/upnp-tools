TESTS=$(check_PROGRAMS)

check_PROGRAMS=test-basic test-network

test_basic_SOURCES = test-basic.cpp
test_basic_CPPFLAGS =-I$(top_srcdir)
test_basic_CXXFLAGS = $(PTHREAD_CXXFLAGS) $(LIBOSLAYER_CFLAGS) $(LIBHTTP_SERVER_CFLAGS)
test_basic_LDFLAGS = $(PTHREAD_CFLAGS)
test_basic_LDADD = ../libupnp-tools/libupnp-tools.la $(LIBOSLAYER_LIBS) $(LIBHTTP_SERVER_LIBS)

test_network_SOURCES = test-network.cpp
test_network_CPPFLAGS =-I$(top_srcdir)
test_network_CXXFLAGS = $(PTHREAD_CXXFLAGS) $(LIBOSLAYER_CFLAGS) $(LIBHTTP_SERVER_CFLAGS)
test_network_LDFLAGS = $(PTHREAD_CFLAGS)
test_network_LDADD = ../libupnp-tools/libupnp-tools.la $(LIBOSLAYER_LIBS) $(LIBHTTP_SERVER_LIBS)
