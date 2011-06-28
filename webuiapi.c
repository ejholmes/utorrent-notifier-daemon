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
#define STRFIELD(field, tmpvar, i) \
    tmpvar = (char *)FIELD(string, i); \
    field = (char *)malloc(strlen(tmpvar) + 1); \
    strcpy(field, tmpvar);
    

struct write_data {
    char *data;
    size_t size;
};

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

torrent_info *copy_torrent_info(torrent_info *torrent);

/* Curl callback functions */
static size_t get_torrents_write_func(void *buffer, size_t size, size_t nmemb, struct write_data *json);
static size_t token_write_func(void *buffer, size_t size, size_t nmemb, char **token);

void webui_init(char *uri, char *username, char *password, int port)
{
    curl_global_init(CURL_GLOBAL_NOTHING);

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
                int last_is_downloading = l->status & status_bit_started;
                int current_is_done = c->status & status_bit_loaded;

                if (last_is_downloading & current_is_done) {
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
    struct write_data json = {
        .data = NULL,
        .size = 0
    };
    torrent_info *head = NULL, *last = NULL;

    curl_easy_setopt(connection, CURLOPT_URL, build_url(2, "/?list=1&token=", token));
    curl_easy_setopt(connection, CURLOPT_WRITEFUNCTION, get_torrents_write_func);
    curl_easy_setopt(connection, CURLOPT_WRITEDATA, &json);
    perform_request();

    DBG("DATA: %s\n", json.data);

    struct json_object *obj;
    obj = json_tokener_parse(json.data);
    obj = json_object_object_get(obj, "torrents");

    int i;
    for (i = 0; i < json_object_array_length(obj); i++) {
        char *tmp = NULL;
        json_object *index = json_object_array_get_idx(obj, i);
        torrent_info *torrent = (torrent_info *)malloc(sizeof(torrent_info));

        STRFIELD(torrent->hash, tmp, 0);
        torrent->status                 =         FIELD(int, 1);
        STRFIELD(torrent->name, tmp, 2);
        torrent->size                   =         FIELD(double, 3);
        torrent->percent_progress       =         FIELD(int, 4);
        torrent->downloaded             =         FIELD(double, 5);
        torrent->uploaded               =         FIELD(double, 6);
        torrent->ratio                  =         FIELD(int, 7);
        torrent->upload_speed           =         FIELD(int, 8);
        torrent->download_speed         =         FIELD(int, 9);
        torrent->eta                    =         FIELD(int, 10);
        STRFIELD(torrent->label, tmp, 11);
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
            STRFIELD(torrent->torrent_source, tmp, 19);
            STRFIELD(torrent->rss_feed, tmp, 20);
            STRFIELD(torrent->status_string, tmp, 21);
        }

        torrent->next = NULL;

        if (!head)
            head = torrent;

        if (last)
            last->next = torrent;

        last = torrent;

        json_object_put(index);
    }
    json_object_put(obj);
    free(json.data);

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

torrent_info *copy_torrent_info(torrent_info *torrent)
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

long perform_request()
{
    curl_easy_perform(connection);
    long http_status = 0;
    curl_easy_getinfo(connection, CURLINFO_RESPONSE_CODE, &http_status);

    switch (http_status) {
        case 401:
            syslog(LOG_ERR, "401 Authorization denied. Check the username and password.");
            break;
        case 404:
            syslog(LOG_ERR, "404 Not found. Check the URL.");
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
    /* curl_easy_cleanup(connection); */
    /* init_connection(); */
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

static size_t get_torrents_write_func(void *buffer, size_t size, size_t nmemb, struct write_data *json)
{
    json->data = (char *)realloc(json->data, (size * nmemb) + json->size + 1);
    memcpy(&json->data[json->size], buffer, size * nmemb);
    json->size += size * nmemb;
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
