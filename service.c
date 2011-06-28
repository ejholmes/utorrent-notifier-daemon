#include <stdio.h>
#include <stdlib.h>
#include <libconfig.h>

#include "webuiapi.h"
#include "service.h"

static struct {
    size_t num;
    struct service_info *services[256];
} svcs;

void add_service(struct service_info *service)
{
    svcs.services[svcs.num] = service;
    ++svcs.num;
}

void call_setup_fn(const config_t *config)
{
    int i;
    for (i = 0; i < svcs.num; i++) {
        int enabled = 0;
        config_setting_t *settings = config_lookup(config, svcs.services[i]->config_key);
        if (settings)
            config_setting_lookup_bool(settings, "enabled", &enabled);
        if (enabled)
            svcs.services[i]->setup_fn(settings);
    }
}

void call_new_torrents_fn(const torrent_info *torrents)
{
    int i;
    for (i = 0; i < svcs.num; i++) {
        svcs.services[i]->new_torrents_fn(torrents);
    }
}

void call_completed_torrents_fn(const torrent_info *torrents)
{
    int i;
    for (i = 0; i < svcs.num; i++) {
        svcs.services[i]->completed_torrents_fn(torrents);
    }
}
