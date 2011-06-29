uTorrent Notifier Daemon
========================
Lightweight C application for monitoring utorrent downloads.

Dependencies: [libconfig](http://www.hyperrealm.com/libconfig/), [libcurl](http://curl.haxx.se/libcurl/), [libjson](https://github.com/jehiah/json-c), [openssl](http://www.openssl.org/)

Install
-------

Download the most recent source from the downloads page.

```
$ tar xzvf utorrent-notifier-<version>.tar.gz
$ cd utorrent-notifier-<version>
$ ./configure
$ make && make install
```

Now edit `/etc/utorrent-notifier.cfg` to match your utorrent settings.
