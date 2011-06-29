PROGRAM = utorrent-notifier

SOURCES = main.c webuiapi.c service.c \
		  services/syslog/syslog.c \
		  services/prowl/prowl.c \
		  services/boxcar/boxcar.c

CFLAGS = -ggdb -Wall $(shell pkg-config --cflags json) -I.
LIBS = -lcurl -lssl $(shell pkg-config --libs json libconfig)

all:
	gcc $(CFLAGS) -o $(PROGRAM) $(SOURCES) $(LIBS)

clean:
	rm -rf $(PROGRAM)
