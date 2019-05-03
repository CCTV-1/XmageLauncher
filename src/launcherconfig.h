#pragma once
#ifndef LAUNCHERCONFIG_H
#define LAUNCHERCONFIG_H

#include <glibmm.h>

#include "commontype.h"

typedef class LauncherConfig
{
public:
    static LauncherConfig& get_config( void );
    ~LauncherConfig();
    LauncherConfig( const LauncherConfig& ) = delete;
    LauncherConfig( LauncherConfig&& ) = delete;
    LauncherConfig& operator=( const LauncherConfig& ) = delete;
    LauncherConfig& operator==( LauncherConfig&& ) = delete;

    //getter and setter
    const Glib::ustring& get_java_path( void );
    const Glib::ustring& get_beta_version( void );
    const Glib::ustring& get_beta_path( void );
    const Glib::ustring& get_release_version( void );
    const Glib::ustring& get_release_path( void );
    const bool         & get_using_proxy( void );
    const Glib::ustring& get_proxy_scheme( void );
    const Glib::ustring& get_proxy_host( void );
    const std::uint32_t& get_proxy_port( void );
    const std::uint32_t& get_jvm_xms( void );
    const std::uint32_t& get_jvm_xmx( void );
    const XmageType&     get_update_source( void );

    LauncherConfig& set_java_path( const Glib::ustring& );
    LauncherConfig& set_beta_version( const Glib::ustring& );
    LauncherConfig& set_beta_path( const Glib::ustring& );
    LauncherConfig& set_release_version( const Glib::ustring& );
    LauncherConfig& set_release_path( const Glib::ustring& );
    LauncherConfig& set_using_proxy( const bool& );
    LauncherConfig& set_proxy_scheme( const Glib::ustring& );
    LauncherConfig& set_proxy_host( const Glib::ustring& );
    LauncherConfig& set_proxy_port( const std::uint32_t& );
    LauncherConfig& set_jvm_xms( const std::uint32_t& );
    LauncherConfig& set_jvm_xmx( const std::uint32_t& );
    LauncherConfig& set_update_source( const XmageType& );

    //only getter
    const Glib::ustring& get_beta_client( void );
    const Glib::ustring& get_beta_server( void );
    const Glib::ustring& get_release_client( void );
    const Glib::ustring& get_release_server( void );
    //don't exitst
    //LauncherConfig& set_beta_client( const Glib::ustring& );
    //LauncherConfig& set_beta_server( const Glib::ustring& );
    //LauncherConfig& set_release_client( const Glib::ustring& );
    //LauncherConfig& set_release_server( const Glib::ustring& );

    //getter special case return a new string.
    Glib::ustring get_release_mage_version( void );
    Glib::ustring get_beta_mage_version( void );
private:
    LauncherConfig();

    Glib::KeyFile config_file;

    Glib::ustring java_path;
    Glib::ustring beta_version;
    Glib::ustring beta_path;
    Glib::ustring beta_client;
    Glib::ustring beta_server;
    Glib::ustring release_version;
    Glib::ustring release_path;
    Glib::ustring release_client;
    Glib::ustring release_server;
    bool using_proxy;
    Glib::ustring proxy_scheme;
    Glib::ustring proxy_host;
    std::uint32_t proxy_port;
    //jvm initial heap size.
    std::uint32_t jvm_xms;
    //jvm maximum heap size.
    std::uint32_t jvm_xmx;
    XmageType update_source;
}config_t;

#endif
