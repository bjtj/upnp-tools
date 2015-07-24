bin_PROGRAMS=upnp-tools
upnp_tools_SOURCES=Text.hpp Text.cpp \
	os.hpp os.cpp \
	SSDPServer.hpp SSDPServer.cpp \
	main.cpp
upnp_tools_CPPFLAGS=-I$(top_srcdir)/deps/http-server/include
upnp_tools_CXXFLAGS=$(PTHREAD_CXXFLAGS)
upnp_tools_LDFLAGS=$(PTHREAD_CFLAGS)
upnp_tools_LDADD=../deps/http-server/libhttp-server/libhttp-server.la
