#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "service.h"

void prowl_setup(config_setting_t *settings);
void prowl_torrent_added(const torrent_info *torrent);
void prowl_torrent_complete(const torrent_info *torrent);

static void prowl_add_message(char *message);

static struct service_info prowl = {
    .name = "Prowl",
    .config_key = "prowl",
    .setup = prowl_setup,
    .torrent_added  = prowl_torrent_added,
    .torrent_complete  = prowl_torrent_complete
};
register_service(prowl);

static const char *addurl = "https://api.prowlapp.com/publicapi/add";
static const char *apikey = "";

void prowl_setup(config_setting_t *settings)
{
    config_setting_lookup_string(settings, "apikey", &apikey);
}

void prowl_torrent_added(const torrent_info *torrent)
{
    char message[1024];
    sprintf(message, "Torrent added: %s", torrent->name);
    prowl_add_message(message);
}

void prowl_torrent_complete(const torrent_info *torrent)
{
    char message[1024];
    sprintf(message, "Download complete: %s", torrent->name);
    prowl_add_message(message);
}

static void prowl_add_message(char *message)
{
    printf("%s\n", message);
}