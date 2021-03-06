#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <syslog.h>
#include <curl/curl.h>
#include <json.h>

#include "webuiapi.h"

#ifdef DEBUG
#define DBG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DBG(...)
#endif

#define FIELD(type, i) json_object_get_##type(json_object_array_get_idx(index, i))
#define STRFIELD(field, i) \
    { \
        char *tmp = (char *)FIELD(string, i); \
        field = (char *)malloc(strlen(tmp) + 1); \
        strcpy(field, tmp); \
    }
    

/* Global struct for holding configuration information */
static struct {
    char *uri;
    int port;
    char *username;
    char *password;
} cfg;

static CURL *connection = NULL;     /* Handle to the curl connection */
static char *token = NULL;          /* Our token provided by the webui */

/* Initializes the curle connection */
static void init_connection();

/* Performs an http request and returns the HTTP Status */
static long perform_request();

/* Helper for building a url to an api resource */
static char *build_url(int args, ...);

/* Gets a token from the webui api */
static char *get_token();

static torrent_info *copy_torrent_info(torrent_info *torrent);

/* Curl callback functions */
static size_t get_torrents_write_func(void *buffer, size_t size, size_t nmemb, char **json);
static size_t token_write_func(void *buffer, size_t size, size_t nmemb, char **token);

void webui_init(char *uri, char *username, char *password, int port)
{
    curl_global_init(CURL_GLOBAL_NOTHING);

    cfg.uri      = uri;
    cfg.port     = port;
    cfg.username = username;
    cfg.password = password;

    init_connection();
}

static void init_connection()
{
    if (connection) {
        DBG("Cleaning CURL connection\n");
        curl_easy_cleanup(connection);
    }
    connection = curl_easy_init();
    curl_easy_setopt(connection, CURLOPT_URL, cfg.uri);
    curl_easy_setopt(connection, CURLOPT_PORT, cfg.port);
    curl_easy_setopt(connection, CURLOPT_USERNAME, cfg.username);
    curl_easy_setopt(connection, CURLOPT_PASSWORD, cfg.password);
    curl_easy_setopt(connection, CURLOPT_COOKIEFILE, "");

    token = get_token();
}

torrent_info *webui_completed_torrents(torrent_info *current_list, torrent_info *last_list)
{
    torrent_info *head = NULL, *last = NULL;
    torrent_info *c, *l;

    /* Iterate through the lists and find any torrents that have completed */
    for (c = current_list; c != NULL; c = c->next) {
        for (l = last_list; l != NULL; l = l->next) {
            if (strcmp(l->hash, c->hash) == 0) {
                /* Determine if the torrent was downloading in the last set and
                 * is now done.*/
                int last_was_downloading = (l->percent_progress != 1000);
                int current_is_done = (c->percent_progress == 1000);

                if (last_was_downloading & current_is_done) {
                    torrent_info *current = copy_torrent_info(c);

                    if (!head)
                        head = current;

                    if (last)
                        last->next = current;

                    last = current;
                }
            }
        }
    }
    return head;
}

torrent_info *webui_new_torrents(torrent_info *current_list, torrent_info *last_list)
{
    torrent_info *head = NULL, *last = NULL;
    torrent_info *c, *l;

    /* Don't try to find new torrents when the last listign was NULL */
    if (!last_list)
        return NULL;

    /* Iterate through the lists and find any torrents that have completed */
    for (c = current_list; c != NULL; c = c->next) {
        int found = 0;

        /* Determine if the current torrent didn't exist in the last refresh */
        for (l = last_list; l != NULL; l = l->next) {
            if (strcmp(c->hash, l->hash) == 0)
                found = 1; /* Torrent existed in last refresh, so it's not new */
        }

        if (!found) {
            torrent_info *current = copy_torrent_info(c);

            if (!head)
                head = current;

            if (last)
                last->next = current;

            last = current;
        }
    }
    return head;
}

torrent_info *webui_get_torrents()
{
    /* char *json; */
    char *json = NULL;
    torrent_info *head = NULL, *last = NULL;

    curl_easy_setopt(connection, CURLOPT_URL, build_url(2, "/?list=1&token=", token));
    curl_easy_setopt(connection, CURLOPT_WRITEFUNCTION, get_torrents_write_func);
    curl_easy_setopt(connection, CURLOPT_WRITEDATA, &json);
    long http_status = perform_request();
    if (http_status != 200) {
        switch (http_status) {
            case 400:
                init_connection();
                return webui_get_torrents();
            default:
                return NULL;
        }
    }

    DBG("DATA: %s\n", json);

    struct json_object *root, *torrents;
    root = json_tokener_parse(json);
    if (!root)
        return NULL;
    torrents = json_object_object_get(root, "torrents");
    if (!torrents)
        return NULL;

    int i;
    for (i = 0; i < json_object_array_length(torrents); i++) {
        json_object *index = json_object_array_get_idx(torrents, i);
        torrent_info *torrent = (torrent_info *)malloc(sizeof(torrent_info));

        STRFIELD(torrent->hash, 0);
        torrent->status                 =         FIELD(int, 1);
        STRFIELD(torrent->name, 2);
        torrent->size                   =         FIELD(double, 3);
        torrent->percent_progress       =         FIELD(int, 4);
        torrent->downloaded             =         FIELD(double, 5);
        torrent->uploaded               =         FIELD(double, 6);
        torrent->ratio                  =         FIELD(int, 7);
        torrent->upload_speed           =         FIELD(int, 8);
        torrent->download_speed         =         FIELD(int, 9);
        torrent->eta                    =         FIELD(int, 10);
        STRFIELD(torrent->label, 11);
        torrent->peers_connected        =         FIELD(int, 12);
        torrent->peers_in_swarm         =         FIELD(int, 13);
        torrent->seeds_connected        =         FIELD(int, 14);
        torrent->seeds_in_swarm         =         FIELD(int, 15);
        torrent->availability           =         FIELD(double, 16);
        torrent->torrent_queue_order    =         FIELD(int, 17);
        torrent->remaining              =         FIELD(double, 18);
        torrent->torrent_source         = NULL;
        torrent->rss_feed               = NULL;
        torrent->status_string          = NULL;

        if (json_object_array_length(index) > 19) {
            STRFIELD(torrent->torrent_source, 19);
            STRFIELD(torrent->rss_feed, 20);
            STRFIELD(torrent->status_string, 21);
        }

        torrent->next = NULL;

        if (!head)
            head = torrent;

        if (last)
            last->next = torrent;

        last = torrent;

        json_object_put(index);
    }
    json_object_put(root);
    json_object_put(torrents);
    free(json);

    return head;
}

void webui_free_torrent_info(torrent_info *torrents)
{
    torrent_info *torrent = torrents;
    while (torrent != NULL) {
        free(torrent->hash);
        free(torrent->name);
        free(torrent->label);
        free(torrent->torrent_source);
        free(torrent->rss_feed);
        free(torrent->status_string);

        torrent_info *temp = torrent;
        torrent = torrent->next;
        free(temp);
    }
}

static torrent_info *copy_torrent_info(torrent_info *torrent)
{
    torrent_info *copy = (torrent_info *)malloc(sizeof(torrent_info));
    memcpy(copy, torrent, sizeof(torrent_info));

    copy->hash           = NULL;
    copy->name           = NULL;
    copy->label          = NULL;
    copy->torrent_source = NULL;
    copy->rss_feed       = NULL;
    copy->status_string  = NULL;
    copy->next           = NULL;

    if (torrent->hash) {
        copy->hash           = (char *)malloc(strlen(torrent->hash) + 1);
        strcpy(copy->hash, torrent->hash);
    }
    if (torrent->name) {
        copy->name           = (char *)malloc(strlen(torrent->name) + 1);
        strcpy(copy->name, torrent->name);
    }
    if (torrent->label) {
        copy->label          = (char *)malloc(strlen(torrent->label) + 1);
        strcpy(copy->label, torrent->label);
    }
    if (torrent->torrent_source) {
        copy->torrent_source = (char *)malloc(strlen(torrent->torrent_source) + 1);
        strcpy(copy->torrent_source, torrent->torrent_source);
    }
    if (torrent->rss_feed) {
        copy->rss_feed       = (char *)malloc(strlen(torrent->rss_feed) + 1);
        strcpy(copy->rss_feed, torrent->rss_feed);
    }
    if (torrent->status_string) {
        copy->status_string  = (char *)malloc(strlen(torrent->status_string) + 1);
        strcpy(copy->status_string, torrent->status_string);
    }


    return copy;
}

static long perform_request()
{
    curl_easy_perform(connection);
    long http_status = 0;
    curl_easy_getinfo(connection, CURLINFO_RESPONSE_CODE, &http_status);

    switch (http_status) {
        case 200:
            break;
        case 400:
            syslog(LOG_ERR, "400 Bad Request.");
            break;
        case 401:
            syslog(LOG_ERR, "401 Unauthorized. Check the username and password.");
            break;
        case 404:
            syslog(LOG_ERR, "404 Not Found. Check the URL.");
            break;
        case 503:
            syslog(LOG_ERR, "503 Service Unavailable. Check the URL.");
            break;
        default:
            syslog(LOG_ERR, "Got status code %ld\n", http_status);
            break;
    }

    return http_status;
}

static char *get_token()
{
    char *token = NULL;
    curl_easy_setopt(connection, CURLOPT_URL, build_url(1, "/token.html"));
    curl_easy_setopt(connection, CURLOPT_WRITEFUNCTION, token_write_func);
    curl_easy_setopt(connection, CURLOPT_WRITEDATA, &token);
    perform_request();
    return token;
}

static char *build_url(int args, ...)
{
    static char *url = NULL;
    char *arg = NULL;
    va_list ap;

    url = (char *)realloc(url, strlen(cfg.uri) + 1);
    strcpy(url, cfg.uri);

    va_start(ap, args);
    arg = va_arg(ap, char *);
    int i;
    for (i = 0; i < args; i++) {
        if (arg) {
            url = (char *)realloc(url, strlen(url) + strlen(arg) + 1);
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
    int current_size = 0;
    if (*json)
        current_size = strlen(*json);
    *json = (char *)realloc(*json, strlen((char *)buffer) + current_size + 1);
    char *ptr = *json+current_size;
    strcpy(ptr, (char *)buffer);
    return size * nmemb;
}

static size_t token_write_func(void *buffer, size_t size, size_t nmemb, char **token)
{
    /* Strip the html */
    char *s = strchr((char *)buffer, '>'); s++; s = strchr(s, '>'); s++; /* Strip beginning html */
    char *e = strchr(s, '<'); /* Strip end html */
    *e = '\0'; /* Terminate the string */
    *token = (char *)malloc(strlen(s)+1);
    strcpy(*token, (const char*)s);
    return size * nmemb;
}
