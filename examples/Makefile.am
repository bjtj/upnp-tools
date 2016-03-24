# SUBDIRS=res

noinst_PROGRAMS = discovery server

discovery_SOURCES = discovery.cpp
discovery_CPPFLAGS =-I$(top_srcdir)
discovery_CXXFLAGS = $(PTHREAD_CXXFLAGS) $(LIBOSLAYER_CFLAGS) $(LIBHTTP_SERVER_CFLAGS)
discovery_LDFLAGS = $(PTHREAD_CFLAGS)
discovery_LDADD = ../libupnp-tools/libupnp-tools.la $(LIBOSLAYER_LIBS) $(LIBHTTP_SERVER_LIBS)

server_SOURCES = server.cpp
server_CPPFLAGS =-I$(top_srcdir)
server_CXXFLAGS = $(PTHREAD_CXXFLAGS) $(LIBOSLAYER_CFLAGS) $(LIBHTTP_SERVER_CFLAGS)
server_LDFLAGS = $(PTHREAD_CFLAGS)
server_LDADD = ../libupnp-tools/libupnp-tools.la $(LIBOSLAYER_LIBS) $(LIBHTTP_SERVER_LIBS)
