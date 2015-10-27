noinst_PROGRAMS = ssdp-server cp

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
