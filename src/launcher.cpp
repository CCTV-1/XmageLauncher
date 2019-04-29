#include <string>
#include <filesystem>

#include <unistd.h>

#include <curl/curl.h>
#include <jansson.h>
#include <gtkmm.h>

#include "commontype.h"
#include "networkutilities.h"
#include "fileutilities.h"
#include "launcherconfig.h"

enum class OperatorSystemType:std::uint32_t
{
    Windows,
    Linux,
    Macos,
};

bool launch_client( config_t& config , XmageType type )
{
    //"java -Xms1024m -Xmx1024m -XX:MaxPermSize=384m -XX:+UseConcMarkSweepGC -XX:+CMSClassUnloadingEnabled -jar .\lib\mage-client-1.4.35.jar"
    Glib::ustring version;
    if ( type == XmageType::Release )
        version = config.get_release_mage_version();
    else
        version = config.get_beta_mage_version();

    std::vector<Glib::ustring> argvs({
        config.get_java_path() , "-Xms1024m" , "-Xmx1024m" , "-XX:MaxPermSize=384m" , "-XX:+UseConcMarkSweepGC" ,
        "-XX:+CMSClassUnloadingEnabled" , "-jar" , "./lib/mage-client-" + version + ".jar"
    });
    Glib::ustring client_path;
    if ( type == XmageType::Release )
        client_path = config.get_release_client();
    else
        client_path = config.get_beta_client();
    Glib::spawn_async_with_pipes( client_path , argvs );
    return true;
}

bool launch_server( config_t& config , XmageType type )
{
    //"java -Xms256M -Xmx512M -XX:MaxPermSize=256m -Djava.security.policy=./config/security.policy -Djava.util.logging.config.file=./config/logging.config -Dlog4j.configuration=file:./config/log4j.properties -jar ./lib/mage-server-1.4.35.jar"
    Glib::ustring version;
    if ( type == XmageType::Release )
        version = config.get_release_mage_version();
    else
        version = config.get_beta_mage_version();

    std::vector<Glib::ustring> argvs({
        config.get_java_path() , "-Xms1024m" , "-Xmx1024m" , "-XX:MaxPermSize=384m" , "-Djava.security.policy=./config/security.policy"
        "-Djava.util.logging.config.file=./config/logging.config" , "-Dlog4j.configuration=file:./config/log4j.properties" , "-jar" , "./lib/mage-server-" + version + ".jar"
    });
    Glib::ustring server_path;
    if ( type == XmageType::Release )
        server_path = config.get_release_server();
    else
        server_path = config.get_beta_server();
    Glib::spawn_async_with_pipes( server_path , argvs );
    return true;
}

//read/apply config
auto& launcher_initial( void )
{
    json_set_alloc_funcs( malloc , free );
    if ( network_utilities_initial() == false )
    {
        g_log( __func__ , G_LOG_LEVEL_ERROR , "network module initial fault" );
    }

    auto & config = config_t::get_config();
    if ( config.get_using_proxy() )
    {
        set_proxy( config.get_proxy_scheme() , config.get_proxy_host() , config.get_proxy_port() );
    }
    return config;
}

void launcher_cleanup( void )
{
    curl_global_cleanup();
}

int main ( int argc , char * argv[] )
{
    auto& config = launcher_initial();

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
        [ window , desc , download_info , &config , download_desc_ptr ]()
        {
            std::future_status desc_status = desc.wait_for( std::chrono::microseconds( 20 ) );
            if ( desc_status == std::future_status::ready )
            {
                client_desc_t client_desc = desc.get();
                g_log( __func__ , G_LOG_LEVEL_MESSAGE , "installed xmage version:%s" , config.get_release_version().c_str() );
                g_log( __func__ , G_LOG_LEVEL_MESSAGE , "last xmage '%s',download url:'%s'" , client_desc.version_name.c_str() , client_desc.download_url.c_str() );
                if ( client_desc.version_name.compare( config.get_release_version() ) )
                {
                    g_log( __func__ , G_LOG_LEVEL_MESSAGE , "%s" , "exitst new xmage,now download." );
                }
                else
                {
                    g_log( __func__ , G_LOG_LEVEL_MESSAGE , "%s" , "not exitst new xmage." );
                    return false;
                }

                std::shared_future<bool> download_status;
                if ( std::filesystem::is_regular_file( client_desc.version_name.raw() + ".zip" ) == false )
                {
                    download_status = download_client( client_desc , download_desc_ptr );
                }
                Glib::signal_timeout().connect(
                    [ client_desc , download_status , download_info , download_desc_ptr , &config ]()
                    {
                        //may continue download
                        if ( download_status.valid() )
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
                                    g_log( __func__ , G_LOG_LEVEL_MESSAGE , "download success,install..." );
                                else
                                {
                                    g_log( __func__ , G_LOG_LEVEL_MESSAGE , "download failure" );
                                    return false;
                                }
                            }
                            
                            return true;
                        }
                        else
                        {
                            g_log( __func__ , G_LOG_LEVEL_MESSAGE , "exitst new xmage zip,continue download,install..." );
                        }

                        auto unzip_future = unzip_client( client_desc.version_name + ".zip" ,  config.get_release_path() );
                        Glib::signal_timeout().connect(
                            [ client_desc , unzip_future , &config ]()
                            {
                                std::future_status unzip_status = unzip_future.wait_for( std::chrono::microseconds( 20 ) );
                                if ( unzip_status == std::future_status::ready )
                                {
                                    g_log( __func__ , G_LOG_LEVEL_MESSAGE , "install success" );
                                    config.set_release_version( client_desc.version_name );
                                    launch_client( config , XmageType::Release );
                                    return false;
                                }
                                return true;
                            } , 100
                        );

                        return false;
                    } , 100
                );
                return false;
            }
            return true;
        }, 100
    );
    return app->run( *window );
}