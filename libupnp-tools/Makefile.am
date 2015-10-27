lib_LTLIBRARIES = libupnp-tools.la

libupnp_toolsincdir = ${includedir}/upnp-tools
libupnp_toolsinc_HEADERS = \
	SSDPServer.hpp \
	StringReader.hpp \
	Timer.hpp \
	UPnP.hpp \
	UPnPAction.hpp \
	UPnPActionArgument.hpp \
	UPnPActionArgumentDirection.hpp \
	UPnPControlPoint.hpp \
	UPnPDevice.hpp \
	UPnPDeviceMaker.hpp \
	UPnPDevicePool.hpp \
	UPnPServer.hpp \
	UPnPService.hpp \
	UPnPServiceMaker.hpp \
	UPnPStateVariable.hpp \
	XmlDocument.hpp \
	XmlDocumentPrinter.hpp \
	XmlDomParser.hpp \
	XmlNodeFinder.hpp

libupnp_tools_la_SOURCES = \
	SSDPServer.cpp \
	SSDPServer.hpp \
	StringReader.cpp \
	StringReader.hpp \
	Timer.cpp \
	Timer.hpp \
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
	UPnPDeviceMaker.cpp \
	UPnPDeviceMaker.hpp \
	UPnPDevicePool.cpp \
	UPnPDevicePool.hpp \
	UPnPServer.cpp \
	UPnPServer.hpp \
	UPnPService.cpp \
	UPnPService.hpp \
	UPnPServiceMaker.cpp \
	UPnPServiceMaker.hpp \
	UPnPStateVariable.cpp \
	UPnPStateVariable.hpp \
	XmlDocument.cpp \
	XmlDocument.hpp \
	XmlDocumentPrinter.cpp \
	XmlDocumentPrinter.hpp \
	XmlDomParser.cpp \
	XmlDomParser.hpp \
	XmlNodeFinder.cpp \
	XmlNodeFinder.hpp


libupnp_tools_la_CXXFLAGS = $(PTHREAD_CXXFLAGS) $(LIBOSLAYER_CFLAGS) $(LIBHTTP_SERVER_CFLAGS)
libupnp_tools_la_LDFLAGS = $(PTHREAD_CFLAGS)
libupnp_tools_la_LIBADD = $(LIBOSLAYER_LIBS) $(LIBHTTP_SERVER_LIBS)
