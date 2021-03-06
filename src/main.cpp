#include <curl/curl.h>
#include <gtkmm.h>
#include <glibmm/i18n.h>

#include "launcher.h"

int main ( int argc , char * argv[] )
{
    bindtextdomain( "XmageLauncher" , "locale/" );
    bind_textdomain_codeset( "XmageLauncher" , "UTF-8" );
    textdomain( "XmageLauncher" );
    if ( curl_global_init( CURL_GLOBAL_ALL ) != 0 )
    {
        g_log( __func__ , G_LOG_LEVEL_ERROR , "network module initial faliure" );
    }

    auto app = Gtk::Application::create( argc , argv );
    auto builder = Gtk::Builder::create_from_resource( "/resources/Launcher.ui" );
    XmageLauncher * launcher_window;
    builder->get_widget_derived( "LauncherWindow" , launcher_window );

    launcher_window->do_update();
    app->run( *launcher_window );
    delete launcher_window;
    curl_global_cleanup();
    return 0;
}
