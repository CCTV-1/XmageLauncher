#pragma once
#ifndef NETWORKUTILITIES_H
#define NETWORKUTILITIES_H

#include <cstdint>

#include <future>

#include <glibmm.h>

#include "commontype.h"

typedef struct JsonBuff
{
    char * buff;
    std::size_t current_size;
}json_buff_t;

typedef struct ClientDescription
{
    Glib::ustring version_name;
    Glib::TimeVal create_time;
    std::size_t size;
    Glib::ustring download_url;
}client_desc_t;

std::shared_future<client_desc_t> get_last_version( XmageType type );

std::shared_future<void> download_client( client_desc_t desc );

#endif