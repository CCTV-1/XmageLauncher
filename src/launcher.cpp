#include <string>

#include <unistd.h>

#include <curl/curl.h>
#include <jansson.h>
#include <gtkmm.h>

#include "commontype.h"
#include "networkutilities.h"
#include "fileutilities.h"

enum class OperatorSystemType:std::uint32_t
{
    Windows,
    Linux,
    Macos,
};

bool launch_client( XmageType )
{
    //"start javaw -Xms1024m -Xmx1024m -XX:MaxPermSize=384m -XX:+UseConcMarkSweepGC -XX:+CMSClassUnloadingEnabled -jar .\lib\mage-client-1.4.35.jar"
    return true;
}

bool launch_server( XmageType )
{
    //"java -Xms256M -Xmx512M -XX:MaxPermSize=256m -Djava.security.policy=./config/security.policy -Djava.util.logging.config.file=./config/logging.config -Dlog4j.configuration=file:./config/log4j.properties -jar ./lib/mage-server-1.4.35.jar"
    return true;
}

void launcher_initial( void )
{
    if ( network_utilities_initial() )
    {
        //G_LOG_LEVEL_ERROR call exit* function
        g_log( __func__ , G_LOG_LEVEL_ERROR , "network model initial fault" );
    }
    json_set_alloc_funcs( malloc , free );
    #ifdef _WIN32
    //JRE don't care Win64/32
    //#ifdef _WIN64
    //#else
    //#endif
        std::getenv("JAVA_HOME");
    #elif __APPLE__
        #include "TargetConditionals.h"
        #if TARGET_OS_MAC
        //Xmage don't support the following architecture
        //#elif TARGET_OS_IPHONE
        //#elif TARGET_IPHONE_SIMULATOR
        #else
        #   error "unsupported architecture"
        #endif
    #elif __linux__
        // linux
    //Xmage don't support the following architecture
    //#elif __unix__
    //#elif defined(_POSIX_VERSION)
    #else
    #   error "unsupported architecture"
    #endif
}

void launcher_cleanup( void )
{
    curl_global_cleanup();
}

int main ( int argc , char * argv[] )
{
    launcher_initial();

    auto desc = get_last_version( XmageType::Release );

    auto app = Gtk::Application::create( argc , argv );
    auto builder = Gtk::Builder::create_from_resource( "/resources/Launcher.ui" );

    Gtk::Window * window = nullptr;
    builder->get_widget( "LauncherWindow" , window );

    Glib::signal_timeout().connect(
        [ window , desc ]()
        {
            std::future_status desc_status = desc.wait_for( std::chrono::microseconds( 20 ) );
            if ( desc_status == std::future_status::ready )
            {
                client_desc_t client_desc = desc.get();
                g_log( __func__ , G_LOG_LEVEL_MESSAGE , "%s,%s" , client_desc.version_name.c_str() , client_desc.download_url.c_str() );
                return false;
            }
            return true;
        }, 100
    );
    return app->run( *window );
}