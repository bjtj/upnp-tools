noinst_PROGRAMS = controller server ssdp-discover ssdp-server dial-discover dial-server msearch

if WITH_NCURSES
noinst_PROGRAMS += tui-controller
endif

#

controller_SOURCES = controller.cpp
server_SOURCES = server.cpp
ssdp_discover_SOURCES = ssdp-discover.cpp
ssdp_server_SOURCES = ssdp-server.cpp
dial_discover_SOURCES = dial-discover.cpp
dial_server_SOURCES = dial-server.cpp
msearch_SOURCES = msearch.cpp


if WITH_NCURSES
tui_controller_SOURCES = tui-controller.cpp
endif

#

AM_CPPFLAGS =-I$(top_srcdir)
AM_CXXFLAGS = $(PTHREAD_CXXFLAGS) $(LIBOSLAYER_CFLAGS) $(LIBHTTP_SERVER_CFLAGS) -DDATA_PATH="\"$(srcdir)/data\""
AM_LDFLAGS = $(PTHREAD_CFLAGS)
LDADD = ../libupnp-tools/libupnp-tools.la $(LIBOSLAYER_LIBS) $(LIBHTTP_SERVER_LIBS)

if WITH_NCURSES
AM_CXXFLAGS += $(NCURSES_CFLAGS)
LDADD += ${NCURSES_LIBS}
endif

EXTRA_DIST=data/dial.xml
