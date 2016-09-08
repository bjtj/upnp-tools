TESTS = $(check_PROGRAMS)

check_PROGRAMS = test-basic test-network test-server test-ssdp test-notification test-resource-manager test-control-point test-serialize test-device-profile-builder test-uuid test-cache

test_basic_SOURCES = test-basic.cpp
test_network_SOURCES = test-network.cpp
test_server_SOURCES = test-server.cpp
test_ssdp_SOURCES = test-ssdp.cpp
test_notification_SOURCES = test-notification.cpp
test_resource_manager_SOURCES = test-resource-manager.cpp
test_control_point_SOURCES = test-control-point.cpp
test_serialize_SOURCES = test-serialize.cpp
test_device_profile_builder_SOURCES = test-device-profile-builder.cpp
test_uuid_SOURCES = test-uuid.cpp
test_cache_SOURCES = test-cache.cpp

AM_CPPFLAGS =-I$(top_srcdir)
AM_CXXFLAGS = $(PTHREAD_CXXFLAGS) $(LIBOSLAYER_CFLAGS) $(LIBHTTP_SERVER_CFLAGS) -DDATA_PATH="\"$(srcdir)/data\""
AM_LDFLAGS = $(PTHREAD_CFLAGS)
LDADD = ../libupnp-tools/libupnp-tools.la $(LIBOSLAYER_LIBS) $(LIBHTTP_SERVER_LIBS)
