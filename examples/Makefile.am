noinst_PROGRAMS = upnp-tools
upnp_tools_SOURCES = main.cpp
upnp_tools_CPPFLAGS =-I$(top_srcdir)/libupnp-tools
upnp_tools_CXXFLAGS = $(PTHREAD_CXXFLAGS) $(LIBOSLAYER_CFLAGS) $(LIBHTTP_SERVER_CFLAGS)
upnp_tools_LDFLAGS = $(PTHREAD_CFLAGS)
upnp_tools_LDADD = ../libupnp-tools/libupnp-tools.la $(LIBOSLAYER_LIBS) $(LIBHTTP_SERVER_LIBS)
