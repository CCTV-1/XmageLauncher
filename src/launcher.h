#pragma once
#ifndef XMAGELAUNCHER_H
#define XMAGELAUNCHER_H

#include <thread>

#include <glibmm/dispatcher.h>
#include <glibmm/refptr.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/combobox.h>
#include <gtkmm/dialog.h>
#include <gtkmm/builder.h>
#include <gtkmm/entry.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/window.h>

#include "launcherconfig.h"
#include "updatework.h"

class FolderChooserButton;

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
    void update_widgets( void );
    void fill_setting_value( void );

private:
    //keep ref to launcher deconstructor
    Glib::RefPtr<Gtk::Builder> launcher_builder;

    Gtk::CheckButton * using_mirror_button;
    Gtk::ComboBox * active_xmage;
    Gtk::ComboBox * proxy_type;
    Gtk::Button * client_button;
    Gtk::Button * server_button;
    Gtk::Button * xmage_button;
    Gtk::Button * setting_button;
    Gtk::Button * reset_button;
    Gtk::SpinButton * proxy_port;
    Gtk::SpinButton * xms_opt;
    Gtk::SpinButton * xmx_opt;
    Gtk::Entry * proxy_host;
    Gtk::Dialog * setting_dialog;    
    FolderChooserButton * beta_path;
    FolderChooserButton * release_path;

    config_t& config;

    std::thread update_process;
    Glib::Dispatcher update_dispatcher;
    UpdateWork update;
};

#endif
