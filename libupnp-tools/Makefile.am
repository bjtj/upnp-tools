lib_LTLIBRARIES = libupnp-tools.la

libupnp_toolsincdir = ${includedir}/upnp-tools
libupnp_toolsinc_HEADERS = SSDPServer.hpp

libupnp_tools_la_SOURCES = Text.hpp Text.cpp \
	SSDPServer.hpp SSDPServer.cpp

libupnp_tools_la_CXXFLAGS = $(PTHREAD_CXXFLAGS) $(LIBOSLAYER_CFLAGS) $(LIBHTTP_SERVER_CFLAGS)
libupnp_tools_la_LDFLAGS = $(PTHREAD_CFLAGS)
libupnp_tools_la_LIBADD = $(LIBOSLAYER_LIBS) $(LIBHTTP_SERVER_LIBS)
