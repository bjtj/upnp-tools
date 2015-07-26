noinst_PROGRAMS = upnp-tools
upnp_tools_SOURCES = main.cpp
upnp_tools_CPPFLAGS =-I$(top_srcdir)/libupnp-tools \
	-I$(top_srcdir)/deps/http-server/include \
	-I$(top_srcdir)/deps/oslayer/liboslayer
upnp_tools_CXXFLAGS = $(PTHREAD_CXXFLAGS)
upnp_tools_LDFLAGS = $(PTHREAD_CFLAGS)
upnp_tools_LDADD = ../libupnp-tools/libupnp-tools.la
#upnp_tools_LDADD=../deps/http-server/libhttp-server/libhttp-server.la ../deps/oslayer/liboslayer/liboslayer.la
