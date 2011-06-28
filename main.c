#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <libconfig.h>

#include "webuiapi.h"
#include "service.h"

void call_setup_fns(const config_t *config);
void call_torrent_added_fns(const torrent_info *torrents);
void call_torrent_complete_fns(const torrent_info *torrents);

static config_t *config = NULL;

static const char *username = "";
static const char *password = "";
static const char *host = "http://localhost/gui";
static int port = 8080;
static int refresh_interval = 1;

void print_torrent_info(torrent_info *torrents)
{
    printf("Torrents: \n");
    for (torrents = torrents; torrents != NULL; torrents = torrents->next) {
        printf("%s\n", torrents->name);
    }
}

int main(int argc, char *argv[])
{
    openlog("utorrent-notifier", LOG_PERROR, LOG_USER);

    config = (config_t *)malloc(sizeof(config_t));
    FILE *fp = fopen("/etc/utorrent-notifier", "r");

    if (fp) {
        if (config_read(config, fp) == CONFIG_TRUE) {
            config_lookup_string(config, "username", &username);
            config_lookup_string(config, "password", &password);
            config_lookup_string(config, "host", &host);
            config_lookup_int(config, "port", &port);
            config_lookup_int(config, "refresh_interval", &refresh_interval);
        }
        else {
            printf("%s\n", config_error_text(config));
        }
    }

    webui_init(host, username, password, port);
    call_setup_fns(config);
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
        print_torrent_info(current);
        if (!last)
            continue;
        completed_torrents = webui_completed_torrents(current, last);
        new_torrents = webui_new_torrents(current, last);
        
        call_torrent_added_fns(new_torrents);
        call_torrent_complete_fns(completed_torrents);
        
        webui_free_torrent_info(completed_torrents);
        webui_free_torrent_info(new_torrents);
        sleep(refresh_interval);
    }

    closelog();

    return 0;
}
