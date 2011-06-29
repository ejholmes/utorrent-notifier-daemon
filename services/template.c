#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "service.h"

void <service name>_torrent_added(const torrent_info *torrent);
void <service name>_torrent_complete(const torrent_info *torrent);

static struct service_info <service name>_service = {
    .name = "<service name>",
    .config_key = "<service key>",
    .torrent_added = <service name>_torrent_added,
    .torrent_complete = <service name>_torrent_complete
};
register_service(<service name>_service);

void <service name>_setup(config_setting_t *settings)
{
    if (settings != NULL) {
        // Grab settings
    }
    // Perform any initialization the service needs
}

void <service name>_torrent_added(const torrent_info *torrent)
{
    // Send notification
}

void <service name>_torrent_complete(const torrent_info *torrent)
{
    // Send notification
}
