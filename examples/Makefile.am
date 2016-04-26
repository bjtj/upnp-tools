# SUBDIRS=res

noinst_PROGRAMS = discovery server

discovery_SOURCES = discovery.cpp
server_SOURCES = server.cpp

AM_CPPFLAGS =-I$(top_srcdir)
AM_CXXFLAGS = $(PTHREAD_CXXFLAGS) $(LIBOSLAYER_CFLAGS) $(LIBHTTP_SERVER_CFLAGS)
AM_LDFLAGS = $(PTHREAD_CFLAGS)
LDADD = ../libupnp-tools/libupnp-tools.la $(LIBOSLAYER_LIBS) $(LIBHTTP_SERVER_LIBS)
