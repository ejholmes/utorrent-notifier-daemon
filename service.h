#ifndef __SERVICE_H__
#define __SERVICE_H__ 

#include <libconfig.h>

#include "webuiapi.h"

/* Pass a service_info struct to this to register it to receive events */
#define register_service(svc_info) \
    static void __attribute__((constructor)) init() \
    { \
        add_service(&svc_info); \
    }

struct service_info {
    char name[256]; /* Name of the service */
    char config_key[256]; /* Group that the configuration is under in the configuration file */
    void (*setup_fn)(config_setting_t *); /* Function called at the start of the application */
    void (*new_torrents_fn)(const torrent_info *); /* Called when a torrent(s) is added */
    void (*completed_torrents_fn)(const torrent_info *); /* Called when a torrent(s) is completed */
};

void add_service(struct service_info *service);

#endif /* __SERVICE_H__ */
