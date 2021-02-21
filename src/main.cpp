#include <gtkmm/application.h>
#include <gtkmm/builder.h>
#include <glibmm/i18n.h>
#include <glibmm/ustring.h>

#include "launcher.h"

//in msys2-mingw64,curl include ERROR macro(in wingdi.h),glibmm-2.68/iochannel.h have a enum class IOStatus::ERROR,so include curl after glibmm.
#include <curl/curl.h>

int main ()
{
    bindtextdomain( "XmageLauncher" , "locale/" );
    bind_textdomain_codeset( "XmageLauncher" , "UTF-8" );
    textdomain( "XmageLauncher" );
    if ( curl_global_init( CURL_GLOBAL_ALL ) != 0 )
    {
        g_log( __func__ , G_LOG_LEVEL_ERROR , "network module initial faliure" );
    }

    auto app = Gtk::Application::create();
    Glib::RefPtr<Gtk::Builder> builder;
    try
    {
        builder = Gtk::Builder::create_from_resource( "/resources/Launcher.ui" );
    }
    catch (const Glib::Error& e)
    {
        g_log( __func__ , G_LOG_LEVEL_ERROR , "load UI file failure,error message:%s" , e.what());
        return -1;
    }
    XmageLauncher * launcher_window = Gtk::Builder::get_widget_derived<XmageLauncher>( builder, Glib::ustring("LauncherWindow") );
    app->signal_activate().connect(
        [app, launcher_window]()
        {
            app->add_window(*launcher_window);
            launcher_window->show();
        }
    );

    launcher_window->do_update();
    app->run();
    delete launcher_window;
    curl_global_cleanup();
    return 0;
}
