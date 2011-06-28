#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "service.h"

void service_setup(config_setting_t *settings);
void service_new_torrents(const torrent_info *torrents);
void service_completed_torrents(const torrent_info *torrents);

static struct service_info service = {
    .name = "Service Name",
    .config_key = "service",
    .setup_fn = service_setup,
    .new_torrents_fn = service_new_torrents,
    .completed_torrents_fn = service_completed_torrents
};
register_service(service);

void service_setup(config_setting_t *settings)
{
}

void service_new_torrents(const torrent_info *torrents)
{
    const torrent_info *torrent = NULL;
    for (torrent = torrents; torrent != NULL; torrent = torrent->next) {
    }
}

void service_completed_torrents(const torrent_info *torrents)
{
    const torrent_info *torrent = NULL;
    for (torrent = torrents; torrent != NULL; torrent = torrent->next) {
    }
}
