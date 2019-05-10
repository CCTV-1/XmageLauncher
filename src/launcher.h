#pragma once
#ifndef XMAGELAUNCHER_H
#define XMAGELAUNCHER_H

#include <thread>

#include <gtkmm.h>

#include "launcherconfig.h"
#include "utilities.h"

class XmageLauncher : public Gtk::Window
{
public:
    XmageLauncher( BaseObjectType* cobject , const Glib::RefPtr<Gtk::Builder>& builder );
    ~XmageLauncher();
    XmageLauncher( const XmageLauncher& ) = delete;
    XmageLauncher( XmageLauncher&& ) = delete;
    XmageLauncher& operator=( const XmageLauncher& ) = delete;
    XmageLauncher& operator==( XmageLauncher&& ) = delete;

    void launch_client( void );
    void launch_server( void );
    void disable_launch( void );
    void enable_launch( void );
    void show_setting( void );
    void close_setting( int );
    void do_update( void );
    void update_prog( void );
    void info_notify( void );
    config_t& get_config( void );

private:
    //keep ref to launcher deconstructor
    Glib::RefPtr<Gtk::Builder> launcher_builder;
    Gtk::Dialog * setting_dialog;
    config_t& config;

    std::thread update_process;
    Glib::Dispatcher update_dispatcher;
    UpdateWork update;
};

#endif
