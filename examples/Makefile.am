noinst_PROGRAMS = ssdp
ssdp_SOURCES = ssdp.cpp
ssdp_CPPFLAGS =-I$(top_srcdir)/libupnp-tools
ssdp_CXXFLAGS = $(PTHREAD_CXXFLAGS) $(LIBOSLAYER_CFLAGS) $(LIBHTTP_SERVER_CFLAGS)
ssdp_LDFLAGS = $(PTHREAD_CFLAGS)
ssdp_LDADD = ../libupnp-tools/libupnp-tools.la $(LIBOSLAYER_LIBS) $(LIBHTTP_SERVER_LIBS)
