#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <syslog.h>
#include <curl/curl.h>
#include <json.h>

#include "webuiapi.h"

#define DBG(...) fprintf(stderr, __VA_ARGS__)

/* Global struct for holding configuration information */
static struct {
    char *uri;
    int port;
    char *username;
    char *password;
} cfg;

static CURL *connection = NULL;     /* Handle to the curl connection */
static char *token = NULL;          /* Our token provided by the webui */
static char url[2046];              /* Holds the url that build_url populates */

/* Initializes the curle connection */
void init_connection();

/* Performs an http request and returns the HTTP Status */
long perform_request();

/* Helper for building a url to an api resource */
char *build_url(int args, ...);

/* Gets a token from the webui api */
char *get_token();

/* Curl callback functions */
static size_t get_torrents_write_func(void *buffer, size_t size, size_t nmemb, char **json);
static size_t token_write_func(void *buffer, size_t size, size_t nmemb, char **token);

void webui_init(char *uri, char *username, char *password, int port)
{
    cfg.uri      = uri;
    cfg.port     = port;
    cfg.username = username;
    cfg.password = password;

    openlog("utorrent-notifier", LOG_PERROR, LOG_USER);
    init_connection();

    token = get_token();
}

void init_connection()
{
    connection = curl_easy_init();
    curl_easy_setopt(connection, CURLOPT_URL, cfg.uri);
    curl_easy_setopt(connection, CURLOPT_PORT, cfg.port);
    curl_easy_setopt(connection, CURLOPT_USERNAME, cfg.username);
    curl_easy_setopt(connection, CURLOPT_PASSWORD, cfg.password);
    curl_easy_setopt(connection, CURLOPT_COOKIEFILE, "");
}

torrent_info *webui_completed_torrents(torrent_info *current, torrent_info *last)
{
    // TODO: Efficient algorithm for finding completed torrents
    return NULL;
}

torrent_info *webui_new_torrents(torrent_info *current, torrent_info *last)
{
    // TODO: Efficient algorithm for finding new torrents
    return NULL;
}

torrent_info *webui_get_torrents()
{
    char *json;
    torrent_info *torrents = (torrent_info *)malloc(sizeof(torrent_info));
    torrents->next = NULL;

    curl_easy_setopt(connection, CURLOPT_URL, build_url(2, "/?list=1&token=", token));
    curl_easy_setopt(connection, CURLOPT_WRITEFUNCTION, get_torrents_write_func);
    curl_easy_setopt(connection, CURLOPT_WRITEDATA, &json);
    perform_request();

    // TODO: Parse JSON

    return torrents;
}

long perform_request()
{
    curl_easy_perform(connection);
    long http_status = 0;
    curl_easy_getinfo(connection, CURLINFO_RESPONSE_CODE, &http_status);

    switch (http_status) {
        case 401:
            syslog(LOG_PERROR, "401 Authorization denied. Check the username and password.");
            break;
        case 404:
            syslog(LOG_PERROR, "404 Not found. Check the URL.");
            break;
        default:
            break;
    }

    return http_status;
}

char *get_token()
{
    char *token = NULL;
    curl_easy_setopt(connection, CURLOPT_URL, build_url(1, "/token.html"));
    curl_easy_setopt(connection, CURLOPT_WRITEFUNCTION, token_write_func);
    curl_easy_setopt(connection, CURLOPT_WRITEDATA, &token);
    perform_request();
    return token;
}

char *build_url(int args, ...)
{
    char *arg = NULL;
    va_list ap;

    memset(url, 0, sizeof(url));
    strcpy(url, cfg.uri);

    va_start(ap, args);
    arg = va_arg(ap, char *);
    int i;
    for (i = 0; i < args; i++) {
        if (arg) {
            strcat(url, arg);
            arg = va_arg(ap, char *);
        }
    }
    va_end(ap);
    DBG("URL: %s\n", url);
    return url;
}

static size_t get_torrents_write_func(void *buffer, size_t size, size_t nmemb, char **json)
{
    *json = (char *)malloc(nmemb);
    memcpy(*json, buffer, nmemb);
    DBG("DATA: %s\n", *json);
    return nmemb;
}

static size_t token_write_func(void *buffer, size_t size, size_t nmemb, char **token)
{
    /* Strip the html */
    char *s = strchr((char *)buffer, '>'); s++; s = strchr(s, '>'); s++; /* Strip beginning html */
    char *e = strchr(s, '<'); /* Strip end html */
    *e = '\0'; /* Terminate the string */
    *token = (char *)malloc(strlen(s)+1);
    strcpy(*token, (const char*)s);
    DBG("TOKEN: %s\n", *token);
    return nmemb;
}
