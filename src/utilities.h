#pragma once
#ifndef NETWORKUTILITIES_H
#define NETWORKUTILITIES_H

#include <cstdint>

#include <thread>
#include <future>

#include <curl/curl.h>
#include <glibmm.h>

typedef struct VersionDescription
{
    Glib::ustring version_name;
    Glib::ustring download_url;
}xmage_desc_t;

enum class XmageType:std::uint8_t
{
    Beta,
    Release
};

struct UpdateWork
{
    //struct default public
    UpdateWork();
    ~UpdateWork() = default;
    void do_update( Glib::Dispatcher& dispatcher );
    void stop_update( void );
    void get_data( bool& update_end , std::int64_t& now , std::int64_t& total , Glib::ustring& info );
    
    mutable std::mutex update_mutex;
    bool updating;
    std::int64_t prog_now;
    std::int64_t prog_total;
    Glib::ustring prog_info;
};

typedef struct UpdateProgress
{
    Glib::Dispatcher& dispatcher;
    UpdateWork * work;
}progress_t;

Glib::ustring xmagetype_to_string( const XmageType& type );

//non-thread safe,existing network thread not affected
//argument hostname: host name or dotted numerical IP address. A numerical IPv6 address must be written within [brackets].
//scheme type see enum curl_proxytype
bool set_proxy( Glib::ustring scheme , Glib::ustring hostname , std::uint32_t port );

std::shared_future<xmage_desc_t> get_last_version( XmageType type );

std::shared_future<bool> download_update( xmage_desc_t desc , progress_t * download_now = nullptr );

std::shared_future<bool> install_update( Glib::ustring install_packge_name , Glib::ustring install_dir_path , progress_t * progress );

Glib::ustring get_installation_package_name( xmage_desc_t desc );

Glib::ustring get_download_temp_name( xmage_desc_t desc );

#endif