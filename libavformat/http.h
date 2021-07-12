/*
 * HTTP definitions
 * Copyright (c) 2010 Josh Allmann
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef AVFORMAT_HTTP_H
#define AVFORMAT_HTTP_H

#include "url.h"
#include "config.h"
#if CONFIG_ZLIB
#include <zlib.h>
#endif /* CONFIG_ZLIB */

#include "libavutil/avassert.h"
#include "libavutil/avstring.h"
#include "libavutil/bprint.h"
#include "libavutil/opt.h"
#include "libavutil/time.h"
#include "libavutil/parseutils.h"

#include "avformat.h"
#include "http.h"
#include "httpauth.h"
#include "internal.h"
#include "network.h"
#include "os_support.h"
#include "url.h"

#define BUFFER_SIZE   (MAX_URL_SIZE + HTTP_HEADERS_SIZE)
#define MAX_REDIRECTS 8
#define HTTP_SINGLE   1
#define HTTP_MUTLI    2
#define MAX_EXPIRY    19
#define WHITESPACES " \n\t\r"

#define HTTP_HEADERS_SIZE 4096

typedef enum {
    LOWER_PROTO,
    READ_HEADERS,
    WRITE_REPLY_HEADERS,
    FINISH
}HandshakeState;

typedef struct HTTPContext {
    const AVClass *class;
    URLContext *hd;
    unsigned char buffer[BUFFER_SIZE], *buf_ptr, *buf_end;
    int line_count;
    int http_code;
    /* Used if "Transfer-Encoding: chunked" otherwise -1. */
    uint64_t chunksize;
    int chunkend;
    uint64_t off, end_off, filesize;
    char *location;
    HTTPAuthState auth_state;
    HTTPAuthState proxy_auth_state;
    char *http_proxy;
    char *headers;
    char *mime_type;
    char *http_version;
    char *user_agent;
    char *referer;
    char *content_type;
    /* Set if the server correctly handles Connection: close and will close
     * the connection after feeding us the content. */
    int willclose;
    int seekable;           /**< Control seekability, 0 = disable, 1 = enable, -1 = probe. */
    int chunked_post;
    /* A flag which indicates if the end of chunked encoding has been sent. */
    int end_chunked_post;
    /* A flag which indicates we have finished to read POST reply. */
    int end_header;
    /* A flag which indicates if we use persistent connections. */
    int multiple_requests;
    uint8_t *post_data;
    int post_datalen;
    int is_akamai;
    int is_mediagateway;
    char *cookies;          ///< holds newline (\n) delimited Set-Cookie header field values (without the "Set-Cookie: " field name)
    /* A dictionary containing cookies keyed by cookie name */
    AVDictionary *cookie_dict;
    int icy;
    /* how much data was read since the last ICY metadata packet */
    uint64_t icy_data_read;
    /* after how many bytes of read data a new metadata packet will be found */
    uint64_t icy_metaint;
    char *icy_metadata_headers;
    char *icy_metadata_packet;
    AVDictionary *metadata;
#if CONFIG_ZLIB
    int compressed;
    z_stream inflate_stream;
    uint8_t *inflate_buffer;
#endif /* CONFIG_ZLIB */
    AVDictionary *chained_options;
    /* -1 = try to send if applicable, 0 = always disabled, 1 = always enabled */
    int send_expect_100;
    char *method;
    int reconnect;
    int reconnect_at_eof;
    int reconnect_on_network_error;
    int reconnect_streamed;
    int reconnect_delay_max;
    char *reconnect_on_http_error;
    int listen;
    char *resource;
    int reply_code;
    int is_multi_client;
    HandshakeState handshake_step;
    int is_connected_server;

    // BUPT
    uint64_t http_req_start;
    uint64_t http_req_end;
    uint64_t http_req_last;

    uint64_t internal_off;
    uint64_t internal_read_pos;
    uint64_t internal_max_size;
    uint8_t* internal_buffer;
    int init_off;
} HTTPContext;


/**
 * Initialize the authentication state based on another HTTP URLContext.
 * This can be used to pre-initialize the authentication parameters if
 * they are known beforehand, to avoid having to do an initial failing
 * request just to get the parameters.
 *
 * @param dest URL context whose authentication state gets updated
 * @param src URL context whose authentication state gets copied
 */
void ff_http_init_auth_state(URLContext *dest, const URLContext *src);

/**
 * Get the HTTP shutdown response status, be used after http_shutdown.
 *
 * @param h pointer to the resource
 * @return a negative value if an error condition occurred, 0
 * otherwise
 */
int ff_http_get_shutdown_status(URLContext *h);

/**
 * Send a new HTTP request, reusing the old connection.
 *
 * @param h pointer to the resource
 * @param uri uri used to perform the request
 * @return a negative value if an error condition occurred, 0
 * otherwise
 */
int ff_http_do_new_request(URLContext *h, const char *uri);

/**
 * Send a new HTTP request, reusing the old connection.
 *
 * @param h pointer to the resource
 * @param uri uri used to perform the request
 * @param options  A dictionary filled with HTTP options. On return
 * this parameter will be destroyed and replaced with a dict containing options
 * that were not found. May be NULL.
 * @return a negative value if an error condition occurred, 0
 * otherwise
 */
int ff_http_do_new_request2(URLContext *h, const char *uri, AVDictionary **options);

int ff_http_averror(int status_code, int default_averror);

#endif /* AVFORMAT_HTTP_H */
