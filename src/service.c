#include <stdio.h>
#include <stdlib.h>
#include <libconfig.h>

#include "webuiapi.h"
#include "service.h"

static struct {
    size_t num;
    struct service_info **services;
} svcs;

void add_service(struct service_info *service)
{
    svcs.services = (struct service_info **)realloc(svcs.services, sizeof(struct service_info *));
    svcs.services[svcs.num] = service;
    ++svcs.num;
}

void call_setup_fns(const config_t *config)
{
    int i;
    for (i = 0; i < svcs.num; i++) {
        int enabled = 1;
        config_setting_t *settings = config_lookup(config, svcs.services[i]->config_key);
        if (settings)
            config_setting_lookup_bool(settings, "enabled", &enabled);
        if (enabled && svcs.services[i]->setup != NULL)
            svcs.services[i]->setup(settings);
    }
}

void call_torrent_added_fns(const torrent_info *torrents)
{
    int i;
    for (i = 0; i < svcs.num; i++) {
        const torrent_info *torrent = NULL;
        for (torrent = torrents; torrent != NULL; torrent = torrent->next){
            if (svcs.services[i]->torrent_added != NULL)
                svcs.services[i]->torrent_added(torrents);
        }
    }
}

void call_torrent_complete_fns(const torrent_info *torrents)
{
    int i;
    for (i = 0; i < svcs.num; i++) {
        const torrent_info *torrent = NULL;
        for (torrent = torrents; torrent != NULL; torrent = torrent->next){
            if (svcs.services[i]->torrent_complete != NULL)
                svcs.services[i]->torrent_complete(torrents);
        }
    }
}
