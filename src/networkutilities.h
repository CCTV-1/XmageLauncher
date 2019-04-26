#pragma once
#ifndef NETWORKUTILITIES_H
#define NETWORKUTILITIES_H

#include <cstdint>

#include <future>

#include <curl/curl.h>
#include <glibmm.h>

#include "commontype.h"

typedef struct ClientDescription
{
    Glib::ustring version_name;
    Glib::TimeVal create_time;
    std::size_t size;
    Glib::ustring download_url;
}client_desc_t;

typedef struct DownloadDescription
{
    curl_off_t total;
    curl_off_t now;
}download_desc_t;

//non-thread safe
bool network_utilities_initial( void );

//non-thread safe,existing network utilities thread not affected
//argument hostname: host name or dotted numerical IP address. A numerical IPv6 address must be written within [brackets].
bool set_proxy( curl_proxytype scheme , Glib::ustring hostname , std::uint32_t port );

std::shared_future<client_desc_t> get_last_version( XmageType type );

std::shared_future<bool> download_client( client_desc_t desc , download_desc_t * download_now = nullptr );

#endif