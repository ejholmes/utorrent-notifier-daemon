#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "service.h"

void syslog_torrent_added(const torrent_info *torrent);
void syslog_torrent_complete(const torrent_info *torrent);

static struct service_info syslog_service = {
    .name = "Syslog",
    .config_key = "syslog",
    .torrent_added = syslog_torrent_added,
    .torrent_complete = syslog_torrent_complete
};
register_service(syslog_service);

void syslog_torrent_added(const torrent_info *torrent)
{
    syslog(LOG_INFO, "Torrent added: %s", torrent->name);
}

void syslog_torrent_complete(const torrent_info *torrent)
{
    syslog(LOG_INFO, "Download complete: %s", torrent->name);
}
