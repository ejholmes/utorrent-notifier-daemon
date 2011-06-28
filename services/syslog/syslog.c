#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "service.h"

void syslog_setup(config_setting_t *settings);
void syslog_new_torrents(const torrent_info *torrents);
void syslog_completed_torrents(const torrent_info *torrents);

static struct service_info syslog_service = {
    .name = "Syslog",
    .config_key = "syslog",
    .setup_fn = syslog_setup,
    .new_torrents_fn = syslog_new_torrents,
    .completed_torrents_fn = syslog_completed_torrents
};
register_service(syslog_service);

void syslog_setup(config_setting_t *settings)
{
}

void syslog_new_torrents(const torrent_info *torrents)
{
    const torrent_info *torrent = NULL;
    for (torrent = torrents; torrent != NULL; torrent = torrent->next) {
        syslog(LOG_INFO, "Torrent added %s", torrent->name);
    }
}

void syslog_completed_torrents(const torrent_info *torrents)
{
    const torrent_info *torrent = NULL;
    for (torrent = torrents; torrent != NULL; torrent = torrent->next) {
        syslog(LOG_INFO, "Download complete: %s", torrent->name);
    }
}
