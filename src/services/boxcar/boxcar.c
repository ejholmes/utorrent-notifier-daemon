#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <openssl/md5.h>

#include "service.h"

static void setup(config_setting_t *settings);
static void torrent_added(const torrent_info *torrent);
static void torrent_complete(const torrent_info *torrent);

static void send_message(char *message);

static struct service_info service = {
    .name = "Boxcar",
    .config_key = "boxcar",
    .setup = setup,
    .torrent_added = torrent_added,
    .torrent_complete = torrent_complete
};
register_service(service);

static const char *uri = "http://boxcar.io";
static const char *apikey = "";
static const char *email = "";
static char md5email[MD5_DIGEST_LENGTH * 2 + 1];

static void setup(config_setting_t *settings)
{
    if (settings != NULL) {
        config_setting_lookup_string(settings, "apikey", &apikey);
        config_setting_lookup_string(settings, "email", &email);

        unsigned char *md5bytes = MD5((unsigned char *)email, strlen(email), NULL);

        int i;
        for (i = 0; i < MD5_DIGEST_LENGTH; i++) {
            sprintf(&md5email[i*2], "%02x", md5bytes[i]);
        }
    }
}

static void torrent_added(const torrent_info *torrent)
{
    char message[2048];
    sprintf(message, "Torrent added: %s", torrent->name);
    send_message(message);
}

static void torrent_complete(const torrent_info *torrent)
{
    char message[2048];
    sprintf(message, "Torrent complete: %s", torrent->name);
    send_message(message);
}

static size_t ignore_result(void *buffer, size_t size, size_t nmemb, void *nothing)
{
    return size * nmemb;
}

static void send_message(char *message)
{
    CURL *connection = curl_easy_init();
    char url[2048];
    char post[2048];
    sprintf(url, "%s/devices/providers/%s/notifications", uri, apikey);
    sprintf(post, "email=%s&notification[message]=%s", md5email, curl_easy_escape(connection, message, 0));
    curl_easy_setopt(connection, CURLOPT_URL, url);
    curl_easy_setopt(connection, CURLOPT_POSTFIELDS, post);
    curl_easy_setopt(connection, CURLOPT_WRITEFUNCTION, ignore_result);
    if (curl_easy_perform(connection) != 0) {
        fprintf(stderr, "Boxcar: An error occurred\n");
    }
    curl_easy_cleanup(connection);
}
