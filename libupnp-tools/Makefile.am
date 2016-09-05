lib_LTLIBRARIES = libupnp-tools.la

libupnp_toolsincdir = ${includedir}/libupnp-tools
libupnp_toolsinc_HEADERS = \
	HttpUtils.hpp \
	NetworkStateManager.hpp \
	NetworkUtil.hpp \
	SSDPEventListener.hpp \
	SSDPHeader.hpp \
	SSDPMsearchSender.hpp \
	SSDPMulticastListener.hpp \
	SSDPMulticastSender.hpp \
	SSDPServer.hpp \
	SharedUPnPDeviceList.hpp \
	UPnPActionErrorCodes.hpp \
	UPnPActionInvoker.hpp \
	UPnPActionRequest.hpp \
	UPnPActionRequestHandler.hpp \
	UPnPActionResponse.hpp \
	UPnPControlPoint.hpp \
	UPnPDeviceDeserializer.hpp \
	UPnPDeviceProfile.hpp \
	UPnPDeviceProfileBuilder.hpp \
	UPnPDeviceProfileSession.hpp \
	UPnPDeviceProfileSessionManager.hpp \
	UPnPDeviceSerializer.hpp \
	UPnPEventReceiver.hpp \
	UPnPEventSubscriber.hpp \
	UPnPEventSubscription.hpp \
	UPnPModels.hpp \
	UPnPPropertyManager.hpp \
	UPnPResourceManager.hpp \
	UPnPServer.hpp \
	UPnPServiceProfile.hpp \
	XmlUtils.hpp

libupnp_tools_la_SOURCES = \
	HttpUtils.cpp \
	HttpUtils.hpp \
	NetworkStateManager.cpp \
	NetworkStateManager.hpp \
	NetworkUtil.cpp \
	NetworkUtil.hpp \
	SSDPEventListener.hpp \
	SSDPHeader.cpp \
	SSDPHeader.hpp \
	SSDPMsearchSender.cpp \
	SSDPMsearchSender.hpp \
	SSDPMulticastListener.cpp \
	SSDPMulticastListener.hpp \
	SSDPMulticastSender.cpp \
	SSDPMulticastSender.hpp \
	SSDPServer.cpp \
	SSDPServer.hpp \
	SharedUPnPDeviceList.cpp \
	SharedUPnPDeviceList.hpp \
	UPnPActionErrorCodes.cpp \
	UPnPActionErrorCodes.hpp \
	UPnPActionInvoker.hpp \
	UPnPActionRequest.cpp \
	UPnPActionRequest.hpp \
	UPnPActionRequestHandler.hpp \
	UPnPActionResponse.cpp \
	UPnPActionResponse.hpp \
	UPnPControlPoint.cpp \
	UPnPControlPoint.hpp \
	UPnPDeviceDeserializer.cpp \
	UPnPDeviceDeserializer.hpp \
	UPnPDeviceProfile.cpp \
	UPnPDeviceProfile.hpp \
	UPnPDeviceProfileBuilder.cpp \
	UPnPDeviceProfileBuilder.hpp \
	UPnPDeviceProfileSession.cpp \
	UPnPDeviceProfileSession.hpp \
	UPnPDeviceProfileSessionManager.cpp \
	UPnPDeviceProfileSessionManager.hpp \
	UPnPDeviceSerializer.cpp \
	UPnPDeviceSerializer.hpp \
	UPnPEventReceiver.cpp \
	UPnPEventReceiver.hpp \
	UPnPEventSubscriber.cpp \
	UPnPEventSubscriber.hpp \
	UPnPEventSubscription.cpp \
	UPnPEventSubscription.hpp \
	UPnPModels.cpp \
	UPnPModels.hpp \
	UPnPPropertyManager.cpp \
	UPnPPropertyManager.hpp \
	UPnPResourceManager.cpp \
	UPnPResourceManager.hpp \
	UPnPServer.cpp \
	UPnPServer.hpp \
	UPnPServiceProfile.hpp \
	XmlUtils.hpp

libupnp_tools_la_CXXFLAGS = $(PTHREAD_CXXFLAGS) $(LIBOSLAYER_CFLAGS) $(LIBHTTP_SERVER_CFLAGS)
libupnp_tools_la_LDFLAGS = $(PTHREAD_CFLAGS)
libupnp_tools_la_LIBADD = $(LIBOSLAYER_LIBS) $(LIBHTTP_SERVER_LIBS)
