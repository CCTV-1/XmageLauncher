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
    Glib::ustring get_beta_version( void );
    Glib::ustring get_beta_path( void );
    Glib::ustring get_release_version( void );
    Glib::ustring get_release_path( void );
    bool          get_using_proxy( void );
    Glib::ustring get_proxy_scheme( void );
    Glib::ustring get_proxy_host( void );
    std::uint32_t get_proxy_port( void );
    std::uint32_t get_jvm_xms( void );
    std::uint32_t get_jvm_xmx( void );
    XmageType    get_update_source( void );

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
    Glib::ustring get_java_path( void );
    Glib::ustring get_beta_client( void );
    Glib::ustring get_beta_server( void );
    Glib::ustring get_release_client( void );
    Glib::ustring get_release_server( void );
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
}config_t;

#endif
