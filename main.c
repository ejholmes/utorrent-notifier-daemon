#include <stdio.h>
#include <stdlib.h>
#include <libconfig.h>

#include "webuiapi.h"

static config_t *config = NULL;

const char *username = NULL, *password = NULL, *host = NULL;
int port = 8080;

int main(int argc, char *argv[])
{
    config = (config_t *)malloc(sizeof(config_t));
    FILE *fp = fopen("/etc/utorrent-notifier", "r");

    if (fp) {
        if (config_read(config, fp) == CONFIG_TRUE) {
            config_lookup_string(config, "username", &username);
            config_lookup_string(config, "password", &password);
            config_lookup_string(config, "host", &host);
            config_lookup_int(config, "port", &port);
        }
        else {
            printf("%s\n", config_error_text(config));
        }
    }

    webui_init(host, username, password, port);
    torrent_info *last = NULL;
    torrent_info *current = NULL;

    current = webui_get_torrents();

    return 0;
}
