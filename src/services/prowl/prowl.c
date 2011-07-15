#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "service.h"

static void setup(config_setting_t *settings);
static void torrent_added(const torrent_info *torrent);
static void torrent_complete(const torrent_info *torrent);

static void add_message(char *message);

static struct service_info prowl = {
    .name = "Prowl",
    .config_key = "prowl",
    .setup = setup,
    .torrent_added  = torrent_added,
    .torrent_complete  = torrent_complete
};
register_service(prowl);

static const char *addurl = "http://api.prowlapp.com/publicapi/add";
static const char *apikey = "";

static void setup(config_setting_t *settings)
{
    if (settings != NULL) {
        config_setting_lookup_string(settings, "apikey", &apikey);
    }
}

static void torrent_added(const torrent_info *torrent)
{
    char message[1024];
    sprintf(message, "Torrent added: %s", torrent->name);
    add_message(message);
}

static void torrent_complete(const torrent_info *torrent)
{
    char message[1024];
    sprintf(message, "Download complete: %s", torrent->name);
    add_message(message);
}

static size_t ignore_result(void *buffer, size_t size, size_t nmemb, void *nothing)
{
    return size * nmemb;
}

static void add_message(char *message)
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
