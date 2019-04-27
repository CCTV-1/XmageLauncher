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
    if ( network_utilities_initial() == false )
    {
        g_log( __func__ , G_LOG_LEVEL_ERROR , "network model initial fault" );
    }
    json_set_alloc_funcs( malloc , free );
    #ifdef _WIN32
    //JRE don't care Win64/32
    //#ifdef _WIN64
    //#else
    //#endif
        auto value = std::getenv("JAVA_HOME");
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

    set_proxy( curl_proxytype::CURLPROXY_HTTP , "localhost" , 1080 );
    auto desc = get_last_version( XmageType::Release );
    download_desc_t download_desc = { 0 , 0 };
    auto download_desc_ptr = &download_desc;
    auto app = Gtk::Application::create( argc , argv );
    auto builder = Gtk::Builder::create_from_resource( "/resources/Launcher.ui" );

    Gtk::Window * window = nullptr;
    builder->get_widget( "LauncherWindow" , window );

    Gtk::ProgressBar * download_info;
    builder->get_widget( "Progress" , download_info );
    window->show_all();

    Glib::signal_timeout().connect(
        [ window , desc , download_info , download_desc_ptr ]()
        {
            std::future_status desc_status = desc.wait_for( std::chrono::microseconds( 20 ) );
            if ( desc_status == std::future_status::ready )
            {
                client_desc_t client_desc = desc.get();
                g_log( __func__ , G_LOG_LEVEL_MESSAGE , "%s,%s" , client_desc.version_name.c_str() , client_desc.download_url.c_str() );
                auto download_status = download_client( client_desc , download_desc_ptr );
                Glib::signal_timeout().connect(
                    [ download_status , download_info , download_desc_ptr ]()
                    {
                        std::future_status desc_status = download_status.wait_for( std::chrono::microseconds( 20 ) );
                        download_info->property_text().set_value( std::to_string( download_desc_ptr->now ) + " / "  + std::to_string( download_desc_ptr->total ) );
                        if ( download_desc_ptr->total == 0 )
                            download_info->set_fraction( 0 );
                        else
                            download_info->set_fraction( download_desc_ptr->now/static_cast<gdouble>( download_desc_ptr->total ) );
                        if ( desc_status == std::future_status::ready )
                        {
                            if ( download_status.get() )
                            {
                                g_log( __func__ , G_LOG_LEVEL_MESSAGE , "download success" );
                            }
                            else
                            {
                                g_log( __func__ , G_LOG_LEVEL_MESSAGE , "download failure" );
                            }
                            return false;
                        }
                        return true;
                    } , 100
                );
                return false;
            }
            return true;
        }, 100
    );
    return app->run( *window );
}