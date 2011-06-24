PROGRAM = utorrent-notifier
SOURCES = main.c webuiapi.c
CFLAGS = -ggdb -Wall $(shell pkg-config --cflags json)
LIBS = -lcurl $(shell pkg-config --libs json libconfig)

all:
	gcc $(CFLAGS) -o $(PROGRAM) $(SOURCES) $(LIBS)

clean:
	rm -rf $(PROGRAM)
