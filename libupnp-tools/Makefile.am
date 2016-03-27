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
	UPnPActionErrorCodes.hpp \
	UPnPActionHandler.hpp \
	UPnPActionInvoker.hpp \
	UPnPActionRequest.hpp \
	UPnPActionResponse.hpp \
	UPnPControlPoint.hpp \
	UPnPDeviceDeserializer.hpp \
	UPnPDeviceProfile.hpp \
	UPnPNotificationServer.hpp \
	UPnPEventSubscriber.hpp \
	UPnPEventSubscription.hpp \
	UPnPEventSubscriptionRegistry.hpp \
	UPnPModels.hpp \
	UPnPServer.hpp \
	UPnPServiceProfile.hpp \
	UPnPSession.hpp \
	UPnPSessionManager.hpp \
	Uuid.hpp \
	XmlUtils.hpp

libupnp_tools_la_SOURCES = \
	HttpUtils.cpp \
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
	UPnPActionErrorCodes.cpp \
	UPnPActionErrorCodes.hpp \
	UPnPActionHandler.hpp \
	UPnPActionInvoker.hpp \
	UPnPActionRequest.cpp \
	UPnPActionRequest.hpp \
	UPnPActionResponse.cpp \
	UPnPActionResponse.hpp \
	UPnPControlPoint.cpp \
	UPnPControlPoint.hpp \
	UPnPDeviceDeserializer.cpp \
	UPnPDeviceDeserializer.hpp \
	UPnPDeviceProfile.cpp \
	UPnPDeviceProfile.hpp \
	UPnPNotificationServer.cpp \
	UPnPNotificationServer.hpp \
	UPnPEventSubscriber.cpp \
	UPnPEventSubscriber.hpp \
	UPnPEventSubscription.cpp \
	UPnPEventSubscription.hpp \
	UPnPEventSubscriptionRegistry.cpp \
	UPnPEventSubscriptionRegistry.hpp \
	UPnPModels.cpp \
	UPnPModels.hpp \
	UPnPServer.cpp \
	UPnPServer.hpp \
	UPnPServiceProfile.hpp \
	UPnPSession.cpp \
	UPnPSession.hpp \
	UPnPSessionManager.cpp \
	UPnPSessionManager.hpp \
	Uuid.cpp \
	Uuid.hpp \
	XmlUtils.hpp


libupnp_tools_la_CXXFLAGS = $(PTHREAD_CXXFLAGS) $(LIBOSLAYER_CFLAGS) $(LIBHTTP_SERVER_CFLAGS)
libupnp_tools_la_LDFLAGS = $(PTHREAD_CFLAGS)
libupnp_tools_la_LIBADD = $(LIBOSLAYER_LIBS) $(LIBHTTP_SERVER_LIBS)
