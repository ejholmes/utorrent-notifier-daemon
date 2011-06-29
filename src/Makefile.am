bin_PROGRAMS = utorrent-notifier

AM_CFLAGS = $(LIBJSON_CFLAGS) $(LIBCONFIG_CFLAGS)
utorrent_notifier_LDADD = $(LIBJSON_LIBS) $(LIBCONFIG_LIBS)
utorrent_notifier_SOURCES = main.c webuiapi.c service.c services/boxcar/boxcar.c services/prowl/prowl.c services/syslog/syslog.c
