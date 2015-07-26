lib_LTLIBRARIES = libupnp-tools.la

libupnp_toolsincdir = ${includedir}/upnp-tools
libupnp_toolsinc_HEADERS = SSDPServer.hpp

libupnp_tools_la_SOURCES = Text.hpp Text.cpp \
	SSDPServer.hpp SSDPServer.cpp
libupnp_tools_la_CPPFLAGS = -I$(top_srcdir)/deps/http-server/include -I$(top_srcdir)/deps/oslayer/liboslayer
libupnp_tools_la_CXXFLAGS = $(PTHREAD_CXXFLAGS)
libupnp_tools_la_LDFLAGS = $(PTHREAD_CFLAGS)
libupnp_tools_la_LIBADD = ../deps/http-server/libhttp-server/libhttp-server.la ../deps/oslayer/liboslayer/liboslayer.la
