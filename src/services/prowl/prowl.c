#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

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

static const char *addurl = "http://api.prowlapp.com/publicapi/add";
static const char *apikey = "";

void prowl_setup(config_setting_t *settings)
{
    if (settings != NULL) {
        config_setting_lookup_string(settings, "apikey", &apikey);
    }
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

static size_t ignore_result(void *buffer, size_t size, size_t nmemb, void *nothing)
{
    return size * nmemb;
}

static void prowl_add_message(char *message)
{
    CURL *connection = curl_easy_init();
    static char post[2048];
    sprintf(post, "apikey=%s&application=utorrent-notifier&event=%s&description=%s", 
            apikey,
            curl_easy_escape(connection, message, 0),
            "");
    curl_easy_setopt(connection, CURLOPT_URL, addurl);
    curl_easy_setopt(connection, CURLOPT_POSTFIELDS, post);
    curl_easy_setopt(connection, CURLOPT_WRITEFUNCTION, ignore_result);
    if (curl_easy_perform(connection) != 0) {
        fprintf(stderr, "Prowl: An error occurred\n");
    }
    curl_easy_cleanup(connection);
}
