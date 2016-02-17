# SUBDIRS=res

noinst_PROGRAMS = ssdpServerTest discovery

ssdpServerTest_SOURCES = ssdpServerTest.cpp
ssdpServerTest_CPPFLAGS =-I$(top_srcdir)
ssdpServerTest_CXXFLAGS = $(PTHREAD_CXXFLAGS) $(LIBOSLAYER_CFLAGS) $(LIBHTTP_SERVER_CFLAGS)
ssdpServerTest_LDFLAGS = $(PTHREAD_CFLAGS)
ssdpServerTest_LDADD = ../libupnp-tools/libupnp-tools.la $(LIBOSLAYER_LIBS) $(LIBHTTP_SERVER_LIBS)

discovery_SOURCES = discovery.cpp
discovery_CPPFLAGS =-I$(top_srcdir)
discovery_CXXFLAGS = $(PTHREAD_CXXFLAGS) $(LIBOSLAYER_CFLAGS) $(LIBHTTP_SERVER_CFLAGS)
discovery_LDFLAGS = $(PTHREAD_CFLAGS)
discovery_LDADD = ../libupnp-tools/libupnp-tools.la $(LIBOSLAYER_LIBS) $(LIBHTTP_SERVER_LIBS)
