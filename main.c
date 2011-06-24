#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libconfig.h>

#include "webuiapi.h"

static config_t *config = NULL;

static const char *username = "";
static const char *password = "";
static const char *host = "http://localhost/gui";
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
    torrent_info *completed_torrents = NULL;
    torrent_info *new_torrents = NULL;

    /* current = webui_get_torrents(); */
    /* webui_free_torrent_info(current); */
    while (1) {
        webui_free_torrent_info(last);
        last = current;
        current = webui_get_torrents();
        completed_torrents = webui_completed_torrents(current, last);
        new_torrents = webui_new_torrents(current, last);
        
        // TODO: Send notifications
        
        webui_free_torrent_info(completed_torrents);
        webui_free_torrent_info(new_torrents);
        sleep(10);
    }

    return 0;
}
