lib_LTLIBRARIES = libupnp-tools.la

libupnp_toolsincdir = ${includedir}/libupnp-tools
libupnp_toolsinc_HEADERS = \
	HttpUtils.hpp \
	NetworkUtil.hpp \
	SSDPEventHandler.hpp \
	SSDPHeader.hpp \
	SSDPMsearchSender.hpp \
	SSDPMulticastListener.hpp \
	SSDPServer.hpp \
	UPnPActionInvoker.hpp \
	UPnPControlPoint.hpp \
	UPnPModels.hpp \
	UPnPSession.hpp \
	UPnPSessionManager.hpp \
	Uuid.hpp \
	XmlUtils.hpp


libupnp_tools_la_SOURCES = \
	HttpUtils.hpp \
	NetworkUtil.cpp \
	NetworkUtil.hpp \
	SSDPEventHandler.hpp \
	SSDPHeader.cpp \
	SSDPHeader.hpp \
	SSDPMsearchSender.cpp \
	SSDPMsearchSender.hpp \
	SSDPMulticastListener.cpp \
	SSDPMulticastListener.hpp \
	SSDPServer.cpp \
	SSDPServer.hpp \
	UPnPActionInvoker.hpp \
	UPnPControlPoint.cpp \
	UPnPControlPoint.hpp \
	UPnPModels.hpp \
	UPnPSession.hpp \
	UPnPSessionManager.hpp \
	Uuid.cpp \
	Uuid.hpp \
	XmlUtils.hpp


libupnp_tools_la_CXXFLAGS = $(PTHREAD_CXXFLAGS) $(LIBOSLAYER_CFLAGS) $(LIBHTTP_SERVER_CFLAGS)
libupnp_tools_la_LDFLAGS = $(PTHREAD_CFLAGS)
libupnp_tools_la_LIBADD = $(LIBOSLAYER_LIBS) $(LIBHTTP_SERVER_LIBS)
