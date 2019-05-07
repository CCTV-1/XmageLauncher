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
    Glib::ustring download_url;
}xmage_desc_t;

//non-thread safe
bool network_utilities_initial( void );

//non-thread safe,existing network utilities thread not affected
//argument hostname: host name or dotted numerical IP address. A numerical IPv6 address must be written within [brackets].
//scheme type see enum curl_proxytype
bool set_proxy( Glib::ustring scheme , Glib::ustring hostname , std::uint32_t port );

std::shared_future<xmage_desc_t> get_last_version( XmageType type );

std::shared_future<bool> download_xmage( xmage_desc_t desc , progress_t * download_now = nullptr );

Glib::ustring get_installation_package_name( xmage_desc_t desc );

Glib::ustring get_download_temp_name( xmage_desc_t desc );

#endif