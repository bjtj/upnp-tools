TESTS=$(check_PROGRAMS)

check_PROGRAMS=test-basic test-network test-server test-ssdp test-notification test-resource-manager

test_basic_SOURCES = test-basic.cpp
test_basic_CPPFLAGS =-I$(top_srcdir)
test_basic_CXXFLAGS = $(PTHREAD_CXXFLAGS) $(LIBOSLAYER_CFLAGS) $(LIBHTTP_SERVER_CFLAGS)
test_basic_LDFLAGS = $(PTHREAD_CFLAGS)
test_basic_LDADD = ../libupnp-tools/libupnp-tools.la $(LIBOSLAYER_LIBS) $(LIBHTTP_SERVER_LIBS)

test_network_SOURCES = test-network.cpp
test_network_CPPFLAGS =-I$(top_srcdir)
test_network_CXXFLAGS = $(PTHREAD_CXXFLAGS) $(LIBOSLAYER_CFLAGS) $(LIBHTTP_SERVER_CFLAGS)
test_network_LDFLAGS = $(PTHREAD_CFLAGS)
test_network_LDADD = ../libupnp-tools/libupnp-tools.la $(LIBOSLAYER_LIBS) $(LIBHTTP_SERVER_LIBS)

test_server_SOURCES = test-server.cpp
test_server_CPPFLAGS =-I$(top_srcdir)
test_server_CXXFLAGS = $(PTHREAD_CXXFLAGS) $(LIBOSLAYER_CFLAGS) $(LIBHTTP_SERVER_CFLAGS)
test_server_LDFLAGS = $(PTHREAD_CFLAGS)
test_server_LDADD = ../libupnp-tools/libupnp-tools.la $(LIBOSLAYER_LIBS) $(LIBHTTP_SERVER_LIBS)

test_ssdp_SOURCES = test-ssdp.cpp
test_ssdp_CPPFLAGS =-I$(top_srcdir)
test_ssdp_CXXFLAGS = $(PTHREAD_CXXFLAGS) $(LIBOSLAYER_CFLAGS) $(LIBHTTP_SERVER_CFLAGS)
test_ssdp_LDFLAGS = $(PTHREAD_CFLAGS)
test_ssdp_LDADD = ../libupnp-tools/libupnp-tools.la $(LIBOSLAYER_LIBS) $(LIBHTTP_SERVER_LIBS)

test_notification_SOURCES = test-notification.cpp
test_notification_CPPFLAGS =-I$(top_srcdir)
test_notification_CXXFLAGS = $(PTHREAD_CXXFLAGS) $(LIBOSLAYER_CFLAGS) $(LIBHTTP_SERVER_CFLAGS)
test_notification_LDFLAGS = $(PTHREAD_CFLAGS)
test_notification_LDADD = ../libupnp-tools/libupnp-tools.la $(LIBOSLAYER_LIBS) $(LIBHTTP_SERVER_LIBS)

test_resource_manager_SOURCES = test-resource-manager.cpp
test_resource_manager_CPPFLAGS =-I$(top_srcdir)
test_resource_manager_CXXFLAGS = $(PTHREAD_CXXFLAGS) $(LIBOSLAYER_CFLAGS) $(LIBHTTP_SERVER_CFLAGS)
test_resource_manager_LDFLAGS = $(PTHREAD_CFLAGS)
test_resource_manager_LDADD = ../libupnp-tools/libupnp-tools.la $(LIBOSLAYER_LIBS) $(LIBHTTP_SERVER_LIBS)

