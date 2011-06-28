#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "service.h"

void prowl_setup(config_setting_t *settings);
void prowl_new_torrents(const torrent_info *torrents);
void prowl_completed_torrents(const torrent_info *torrents);

static void prowl_add_message(char *message);

static struct service_info prowl = {
    .name = "Prowl",
    .config_key = "prowl",
    .setup_fn = prowl_setup,
    .new_torrents_fn = prowl_new_torrents,
    .completed_torrents_fn = prowl_completed_torrents
};
register_service(prowl);

static const char *addurl = "https://api.prowlapp.com/publicapi/add";
static const char *apikey = "";

void prowl_setup(config_setting_t *settings)
{
    config_setting_lookup_string(settings, "apikey", &apikey);
}

void prowl_new_torrents(const torrent_info *torrents)
{
    const torrent_info *torrent = NULL;
    for (torrent = torrents; torrent != NULL; torrent = torrent->next) {
        char message[1024];
        sprintf(message, "Torrent added: %s", torrent->name);
        prowl_add_message(message);
    }
}

void prowl_completed_torrents(const torrent_info *torrents)
{
    const torrent_info *torrent = NULL;
    for (torrent = torrents; torrent != NULL; torrent = torrent->next) {
        char message[1024];
        sprintf(message, "Download complete: %s", torrent->name);
        prowl_add_message(message);
    }
}

static void prowl_add_message(char *message)
{
    printf("%s\n", message);
}
