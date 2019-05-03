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

bool launch_client( config_t& config , XmageType type )
{
    //"java -Xms1024m -Xmx1024m -XX:MaxPermSize=384m -XX:+UseConcMarkSweepGC -XX:+CMSClassUnloadingEnabled -jar .\lib\mage-client-1.4.35.jar"
    Glib::ustring version;
    if ( type == XmageType::Release )
        version = config.get_release_mage_version();
    else
        version = config.get_beta_mage_version();

    Glib::ustring xms_opt = Glib::ustring( "-Xms" ) + std::to_string( config.get_jvm_xms() ) + "m";
    Glib::ustring xmx_opt = Glib::ustring( "-Xmx" ) + std::to_string( config.get_jvm_xmx() ) + "m";
    std::vector<Glib::ustring> argvs({
        config.get_java_path() , xms_opt , xmx_opt , 
        "-XX:MaxPermSize=384m" , "-XX:+UseConcMarkSweepGC" ,
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

    Glib::ustring xms_opt = Glib::ustring( "-Xms" ) + std::to_string( config.get_jvm_xms() ) + "m";
    Glib::ustring xmx_opt = Glib::ustring( "-Xmx" ) + std::to_string( config.get_jvm_xmx() ) + "m";
    std::vector<Glib::ustring> argvs({
        config.get_java_path() , xms_opt , xmx_opt ,"-XX:MaxPermSize=384m" , "-Djava.security.policy=./config/security.policy",
        "-Djava.util.logging.config.file=./config/logging.config" , "-Dlog4j.configuration=file:./config/log4j.properties"
        , "-jar" , "./lib/mage-server-" + version + ".jar"
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

    //main window button
    Gtk::Button * client_button;
    builder->get_widget( "LauncherClient" , client_button );
    client_button->signal_clicked().connect(
        [ &config ]()
        {
            launch_client( config , XmageType::Release );
        }
    );
    Gtk::Button * server_button;
    builder->get_widget( "LauncherServer" , server_button );
    server_button->signal_clicked().connect(
        [ &config ]()
        {
            launch_server( config , XmageType::Release );
        }
    );
    Gtk::Button * xmage_button;
    builder->get_widget( "LauncherXmage" , xmage_button );
    xmage_button->signal_clicked().connect(
        [ &config ]()
        {
            launch_client( config , XmageType::Release );
            launch_server( config , XmageType::Release );
        }
    );
    //main window progress
    Gtk::ProgressBar * progrees_bar;
    builder->get_widget( "Progress" , progrees_bar );
    Gtk::Label * progrees_target;
    builder->get_widget( "ProgressTarget" , progrees_target );
    Gtk::Label * progrees_value;
    builder->get_widget( "ProgressValue" , progrees_value );
    Gtk::Dialog * setting_dialog;
    builder->get_widget( "SettingDialog" , setting_dialog );


    Gtk::Button * setting_button;
    builder->get_widget( "SettingButton" , setting_button );
    setting_button->signal_clicked().connect(
        [ setting_dialog ]()
        {
            setting_dialog->run();
        }
    );

    //Setting menu dialog
    Gtk::ComboBox * proxy_type;
    builder->get_widget( "ProxyType" , proxy_type );
    if ( config.get_using_proxy() )
    {
        proxy_type->set_active_id( config.get_proxy_scheme() );
    }
    else
    {
        proxy_type->set_active_id( "None" );
    }
    proxy_type->signal_changed().connect(
        [ &config , proxy_type ]()
        {
            Glib::ustring proxy_string = proxy_type->get_active_id();
            Glib::ustring diff_string = proxy_string.lowercase();
            if ( diff_string.compare( "none" ) == 0 )
            {
                config.set_using_proxy( false );
            }
            else
            {
                config.set_using_proxy( true );
                config.set_proxy_scheme( proxy_string );
            }
        }
    );
    Gtk::Entry * proxy_host;
    builder->get_widget( "ProxyHost" , proxy_host );
    proxy_host->set_text( config.get_proxy_host() );
    proxy_host->signal_changed().connect(
        [ &config , proxy_host ]()
        {
            Glib::ustring host_string = proxy_host->get_text();
            config.set_proxy_host( host_string );
        }
    );
    Gtk::SpinButton * proxy_port;
    builder->get_widget( "ProxyPort" , proxy_port );
    proxy_port->set_value( config.get_proxy_port() );
    proxy_port->signal_changed().connect(
        [ &config , proxy_port ]()
        {
            double port_value = proxy_port->get_value();
            config.set_proxy_port( port_value );
        }
    );
    Gtk::SpinButton * xms_opt;
    builder->get_widget( "XmsOpt" , xms_opt );
    xms_opt->set_value( config.get_jvm_xms() );
    xms_opt->signal_changed().connect(
        [ &config , xms_opt ]()
        {
            double xms_value = xms_opt->get_value();
            config.set_jvm_xms( xms_value );
        }
    );
    Gtk::SpinButton * xmx_opt;
    builder->get_widget( "XmxOpt" , xmx_opt );
    xmx_opt->set_value( config.get_jvm_xmx() );
    xmx_opt->signal_changed().connect(
        [ &config , xmx_opt ]()
        {
            double xmx_value = xmx_opt->get_value();
            config.set_proxy_port( xmx_value );
        }
    );
    Gtk::FileChooserButton * release_path;
    builder->get_widget( "ReleaseMagePath" , release_path );
    release_path->set_filename( config.get_release_path() );
    release_path->signal_selection_changed().connect(
        [ &config , release_path ]()
        {
            Glib::ustring new_release_path = release_path->get_filename();
            config.set_release_path( new_release_path );
        }
    );
    Gtk::FileChooserButton * beta_path;
    builder->get_widget( "BetaMagePath" , beta_path );
    beta_path->set_filename( config.get_beta_path() );
    beta_path->signal_selection_changed().connect(
        [ &config , beta_path ]()
        {
            Glib::ustring new_beta_path = beta_path->get_filename();
            config.set_beta_path( new_beta_path );
        }
    );
    Gtk::ComboBox * update_source;
    builder->get_widget( "UpdateSource" , update_source );
    //update_source->set_active_id( "Release" );
    update_source->signal_changed().connect(
        [ &config , update_source ]()
        {
            Glib::ustring source = update_source->get_active_id();
            g_log( __func__ , G_LOG_LEVEL_MESSAGE , "source:'%s'" , source.c_str() );
        }
    );

    window->show_all();

    Glib::signal_timeout().connect(
        [ window , desc , progrees_bar , progrees_target , progrees_value , &config , download_desc_ptr ]()
        {
            std::future_status desc_status = desc.wait_for( std::chrono::microseconds( 20 ) );
            if ( desc_status == std::future_status::ready )
            {
                progrees_target->set_label( "check update" );
                client_desc_t client_desc;
                try
                {
                    client_desc = desc.get();
                }
                catch ( const std::exception& e )
                {
                    //get update information failure
                    g_log( __func__ , G_LOG_LEVEL_MESSAGE , "%s" , e.what() );
                    return false;
                }
                g_log( __func__ , G_LOG_LEVEL_MESSAGE , "installed xmage version:'%s'." , config.get_release_version().c_str() );
                g_log( __func__ , G_LOG_LEVEL_MESSAGE , "last xmage version:'%s',download url:'%s'." , client_desc.version_name.c_str() , client_desc.download_url.c_str() );
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
                    [ client_desc , download_status , progrees_bar , progrees_target , progrees_value , download_desc_ptr , &config ]()
                    {
                        progrees_target->set_label( "download update" );
                        //may continue download
                        if ( download_status.valid() )
                        {
                            std::future_status desc_status = download_status.wait_for( std::chrono::microseconds( 20 ) );
                            progrees_value->set_label( std::to_string( download_desc_ptr->now ) + " / "  + std::to_string( download_desc_ptr->total ) );
                            if ( download_desc_ptr->total == 0 )
                                progrees_bar->set_fraction( 0 );
                            else
                                progrees_bar->set_fraction( download_desc_ptr->now/static_cast<gdouble>( download_desc_ptr->total ) );
                            if ( desc_status == std::future_status::ready )
                            {
                                if ( download_status.get() )
                                {
                                    progrees_target->set_label( "download success,install..." );
                                }
                                else
                                {
                                    progrees_target->set_label( "download failure" );
                                    return false;
                                }
                            }
                            else
                            {
                                return true;
                            }
                        }
                        else
                        {
                            g_log( __func__ , G_LOG_LEVEL_MESSAGE , "exitst new xmage zip,continue download,install..." );
                        }

                        progrees_bar->set_fraction( 0 );
                        progrees_value->set_label( "0/0" );
                        auto unzip_future = unzip_client( client_desc.version_name + ".zip" ,  config.get_release_path() );
                        Glib::signal_timeout().connect(
                            [ progrees_target , client_desc , unzip_future , &config ]()
                            {
                                progrees_target->set_label( "install update" );
                                std::future_status unzip_status = unzip_future.wait_for( std::chrono::microseconds( 20 ) );
                                if ( unzip_status == std::future_status::ready )
                                {
                                    progrees_target->set_label( "install success" );
                                    config.set_release_version( client_desc.version_name );
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