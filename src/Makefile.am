bin_PROGRAMS = utorrent-notifier

EXTRA_DIST = webuiapi.h service.h

AM_CFLAGS = $(LIBJSON_CFLAGS) $(LIBCONFIG_CFLAGS)
# utorrent_notifier_LDADD = /usr/lib/libjson.a /usr/lib/libconfig.a
utorrent_notifier_LDADD = $(LIBJSON_LIBS) $(LIBCONFIG_LIBS)
utorrent_notifier_SOURCES = main.c webuiapi.c service.c services/boxcar/boxcar.c services/prowl/prowl.c services/syslog/syslog.c
