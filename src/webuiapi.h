#ifndef WEBUIAPI_H
#define WEBUIAPI_H

#ifdef __cplusplus
extern "C" {
#endif

enum status_bits_t {
    status_bit_started = 1,
    status_bit_checking = 2,
    status_bit_start_after_check = 4,
    status_bit_checked = 8,
    status_bit_error = 16,
    status_bit_paused = 32,
    status_bit_queued = 64,
    status_bit_loaded = 128
};

/* Structure of the torrent info returned by the webui api */
typedef struct torrent_info_t {
    char *hash;             /* Torrent hash */
    char *name;             /* Torrent name */
    char *label;            /* Label assigned to this torrent */
    char *torrent_source;
    char *rss_feed;
    char *status_string;

    int status;
    int percent_progress;
    int ratio;
    int upload_speed;
    int download_speed;
    int eta;
    int peers_connected;
    int peers_in_swarm;
    int seeds_connected;
    int seeds_in_swarm;
    int torrent_queue_order;

    double size;
    double downloaded;
    double uploaded;
    double availability;
    double remaining;

    struct torrent_info_t *next;
} torrent_info;

/* Initializes the connection to the webui api */
void webui_init();

/* Returns a linked list to all the current torrents */
torrent_info *webui_get_torrents();

/* Frees a linked list of torrent_info */
void webui_free_torrent_info(torrent_info *torrents);

/* Returns a linked list of completed torrents. NULL if there are none */
torrent_info *webui_completed_torrents(torrent_info *current, torrent_info *last);

/* Returns a linked list of newly added torrents. NULL if there are none */
torrent_info *webui_new_torrents(torrent_info *current, torrent_info *last);

#ifdef __cplusplus
}
#endif 

#endif // WEBUIAPI_H
