lib_LTLIBRARIES = libupnp-tools.la

libupnp_toolsincdir = ${includedir}/upnp-tools
libupnp_toolsinc_HEADERS = \
						SSDPServer.hpp \
						UPnP.hpp \
						UPnPAction.hpp \
						UPnPActionArgument.hpp \
						UPnPActionArgumentDirection.hpp \
						UPnPControlPoint.hpp \
						UPnPDevice.hpp \
						UPnPDevicePool.hpp \
						UPnPServer.hpp \
						UPnPService.hpp \
						UPnPStateVariable.hpp

libupnp_tools_la_SOURCES = \
						SSDPServer.cpp \
						SSDPServer.hpp \
						UPnP.hpp \
						UPnPAction.cpp \
						UPnPAction.hpp \
						UPnPActionArgument.cpp \
						UPnPActionArgument.hpp \
						UPnPActionArgumentDirection.hpp \
						UPnPControlPoint.cpp \
						UPnPControlPoint.hpp \
						UPnPDevice.cpp \
						UPnPDevice.hpp \
						UPnPDevicePool.cpp \
						UPnPDevicePool.hpp \
						UPnPServer.hpp \
						UPnPService.cpp \
						UPnPService.hpp \
						UPnPStateVariable.cpp \
						UPnPStateVariable.hpp

libupnp_tools_la_CXXFLAGS = $(PTHREAD_CXXFLAGS) $(LIBOSLAYER_CFLAGS) $(LIBHTTP_SERVER_CFLAGS)
libupnp_tools_la_LDFLAGS = $(PTHREAD_CFLAGS)
libupnp_tools_la_LIBADD = $(LIBOSLAYER_LIBS) $(LIBHTTP_SERVER_LIBS)
