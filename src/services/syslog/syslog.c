#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "service.h"

static void torrent_added(const torrent_info *torrent);
static void torrent_complete(const torrent_info *torrent);

static struct service_info service = {
    .name = "Syslog",
    .config_key = "syslog",
    .torrent_added = torrent_added,
    .torrent_complete = torrent_complete
};
register_service(service);

static void torrent_added(const torrent_info *torrent)
{
    syslog(LOG_INFO, "Torrent added: %s", torrent->name);
}

static void torrent_complete(const torrent_info *torrent)
{
    syslog(LOG_INFO, "Download complete: %s", torrent->name);
}
