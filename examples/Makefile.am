SUBDIRS=res

noinst_PROGRAMS = ssdp-server cp server dialTest

ssdp_server_SOURCES = ssdp-server.cpp
ssdp_server_CPPFLAGS =-I$(top_srcdir)
ssdp_server_CXXFLAGS = $(PTHREAD_CXXFLAGS) $(LIBOSLAYER_CFLAGS) $(LIBHTTP_SERVER_CFLAGS)
ssdp_server_LDFLAGS = $(PTHREAD_CFLAGS)
ssdp_server_LDADD = ../libupnp-tools/libupnp-tools.la $(LIBOSLAYER_LIBS) $(LIBHTTP_SERVER_LIBS)

cp_SOURCES = cp.cpp
cp_CPPFLAGS =-I$(top_srcdir)
cp_CXXFLAGS = $(PTHREAD_CXXFLAGS) $(LIBOSLAYER_CFLAGS) $(LIBHTTP_SERVER_CFLAGS)
cp_LDFLAGS = $(PTHREAD_CFLAGS)
cp_LDADD = ../libupnp-tools/libupnp-tools.la $(LIBOSLAYER_LIBS) $(LIBHTTP_SERVER_LIBS)

server_SOURCES = server.cpp
server_CPPFLAGS =-I$(top_srcdir)
server_CXXFLAGS = $(PTHREAD_CXXFLAGS) $(LIBOSLAYER_CFLAGS) $(LIBHTTP_SERVER_CFLAGS)
server_LDFLAGS = $(PTHREAD_CFLAGS)
server_LDADD = ../libupnp-tools/libupnp-tools.la $(LIBOSLAYER_LIBS) $(LIBHTTP_SERVER_LIBS)

dialTest_SOURCES = dialTest.cpp
dialTest_CPPFLAGS =-I$(top_srcdir)
dialTest_CXXFLAGS = $(PTHREAD_CXXFLAGS) $(LIBOSLAYER_CFLAGS) $(LIBHTTP_SERVER_CFLAGS)
dialTest_LDFLAGS = $(PTHREAD_CFLAGS)
dialTest_LDADD = ../libupnp-tools/libupnp-tools.la $(LIBOSLAYER_LIBS) $(LIBHTTP_SERVER_LIBS)

