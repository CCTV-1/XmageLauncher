#include <string>
#include <filesystem>

#include <unistd.h>

#include <curl/curl.h>
#include <jansson.h>
#include <gtkmm.h>
#include <glibmm/i18n.h>

#include "launcher.h"

XmageLauncher::XmageLauncher( BaseObjectType* cobject , const Glib::RefPtr<Gtk::Builder>& builder ):
    Gtk::Window( cobject ),
    launcher_builder( builder ),
    config( config_t::get_config() ),
    update_process(),
    update_dispatcher()
{
    if ( this->config.get_using_proxy() )
    {
        set_proxy( config.get_proxy_scheme() , config.get_proxy_host() , config.get_proxy_port() );
    }

    //main window button
    Gtk::Button * client_button;
    builder->get_widget( "LauncherClient" , client_button );
    client_button->signal_clicked().connect( sigc::mem_fun( *this , &XmageLauncher::launch_client ) );
    Gtk::Button * server_button;
    builder->get_widget( "LauncherServer" , server_button );
    server_button->signal_clicked().connect( sigc::mem_fun( *this , &XmageLauncher::launch_server ) );
    Gtk::Button * xmage_button;
    builder->get_widget( "LauncherXmage" , xmage_button );
    xmage_button->signal_clicked().connect(
            [ this ]()
            {
                this->launch_client();
                this->launch_server();
            }
    );

    //Setting menu dialog
    builder->get_widget( "SettingDialog" , this->setting_dialog );
    this->setting_dialog->add_button( _( "close menu" ) , 0 );
    this->setting_dialog->signal_response().connect( sigc::mem_fun( *this , &XmageLauncher::close_setting ) );
    Gtk::Button * setting_button;
    builder->get_widget( "SettingButton" , setting_button );
    setting_button->signal_clicked().connect( sigc::mem_fun( *this , &XmageLauncher::show_setting ) );
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
            [ this , proxy_type ]()
            {
                Glib::ustring proxy_string = proxy_type->get_active_id();
                Glib::ustring diff_string = proxy_string.lowercase();
                if ( diff_string.compare( "none" ) == 0 )
                {
                    this->config.set_using_proxy( false );
                }
                else
                {
                    this->config.set_using_proxy( true );
                    this->config.set_proxy_scheme( proxy_string );
                }
            }
    );
    Gtk::Entry * proxy_host;
    builder->get_widget( "ProxyHost" , proxy_host );
    proxy_host->set_text( config.get_proxy_host() );
    proxy_host->signal_changed().connect(
        [ this , proxy_host ]()
        {
            Glib::ustring host_string = proxy_host->get_text();
            this->config.set_proxy_host( host_string );
        }
    );
    Gtk::SpinButton * proxy_port;
    builder->get_widget( "ProxyPort" , proxy_port );
    proxy_port->set_value( config.get_proxy_port() );
    proxy_port->signal_changed().connect(
        [ this , proxy_port ]()
        {
            double port_value = proxy_port->get_value();
            this->config.set_proxy_port( port_value );
        }
    );
    Gtk::SpinButton * xms_opt;
    builder->get_widget( "XmsOpt" , xms_opt );
    xms_opt->set_value( config.get_jvm_xms() );
    xms_opt->signal_changed().connect(
        [ this , xms_opt ]()
        {
            double xms_value = xms_opt->get_value();
            this->config.set_jvm_xms( xms_value );
        }
    );
    Gtk::SpinButton * xmx_opt;
    builder->get_widget( "XmxOpt" , xmx_opt );
    xmx_opt->set_value( this->config.get_jvm_xmx() );
    xmx_opt->signal_changed().connect(
        [ this , xmx_opt ]()
        {
            double xmx_value = xmx_opt->get_value();
            this->config.set_jvm_xmx( xmx_value );
        }
    );
    Gtk::FileChooserButton * release_path;
    builder->get_widget( "ReleaseMagePath" , release_path );
    release_path->set_filename( config.get_release_path() );
    release_path->signal_selection_changed().connect(
        [ this , release_path ]()
        {
            Glib::ustring new_release_path = release_path->get_filename();
            this->config.set_release_path( new_release_path );
        }
    );
    Gtk::FileChooserButton * beta_path;
    builder->get_widget( "BetaMagePath" , beta_path );
    beta_path->set_filename( this->config.get_beta_path() );
    beta_path->signal_selection_changed().connect(
        [ this , beta_path ]()
        {
            Glib::ustring new_beta_path = beta_path->get_filename();
            this->config.set_beta_path( new_beta_path );
        }
    );
    Gtk::ComboBox * active_xmage;
    builder->get_widget( "UpdateSource" , active_xmage );
    Glib::ustring source_string = xmagetype_to_string( this->config.get_active_xmage() );
    active_xmage->set_active_id( source_string );
    active_xmage->signal_changed().connect(
        [ this , active_xmage ]()
        {
            Glib::ustring source = active_xmage->get_active_id();
            this->config.set_active_xmage( string_to_xmagetype( source ) );
        }
    );

    this->show_all();

    update_dispatcher.connect( sigc::mem_fun( *this , &XmageLauncher::update_prog ) );
}


XmageLauncher::~XmageLauncher()
{
    if ( this->update_process.joinable() )
        this->update_process.detach();
    delete setting_dialog;
}

void XmageLauncher::launch_client( void )
{
        //"java -Xms1024m -Xmx1024m -XX:MaxPermSize=384m -XX:+UseConcMarkSweepGC -XX:+CMSClassUnloadingEnabled -jar .\lib\mage-client-1.4.35.jar"
        Glib::ustring version = this->config.get_active_xmage_version();

        Glib::ustring xms_opt = Glib::ustring( "-Xms" ) + std::to_string( this->config.get_jvm_xms() ) + "M";
        Glib::ustring xmx_opt = Glib::ustring( "-Xmx" ) + std::to_string( this->config.get_jvm_xmx() ) + "M";
        std::vector<Glib::ustring> argvs({
            this->config.get_java_path() , xms_opt , xmx_opt , 
            "-XX:MaxPermSize=384m" , "-XX:+UseConcMarkSweepGC" ,
            "-XX:+CMSClassUnloadingEnabled" , "-jar" , "./lib/mage-client-" + version + ".jar"
        });
        Glib::ustring client_path = this->config.get_active_xmage_client();

        Glib::spawn_async_with_pipes( client_path , argvs );
}

void XmageLauncher::launch_server( void )
{
        //"java -Xms256M -Xmx512M -XX:MaxPermSize=256m -Djava.security.policy=./config/security.policy -Djava.util.logging.config.file=./config/logging.config -Dlog4j.configuration=file:./config/log4j.properties -jar ./lib/mage-server-1.4.35.jar"
        Glib::ustring version = this->config.get_active_xmage_version();

        Glib::ustring xms_opt = Glib::ustring( "-Xms" ) + std::to_string( this->config.get_jvm_xms() ) + "M";
        Glib::ustring xmx_opt = Glib::ustring( "-Xmx" ) + std::to_string( this->config.get_jvm_xmx() ) + "M";
        std::vector<Glib::ustring> argvs({
            this->config.get_java_path() , xms_opt , xmx_opt ,"-XX:MaxPermSize=384m" , "-Djava.security.policy=./config/security.policy",
            "-Djava.util.logging.config.file=./config/logging.config" , "-Dlog4j.configuration=file:./config/log4j.properties"
            , "-jar" , "./lib/mage-server-" + version + ".jar"
        });
        Glib::ustring server_path = this->config.get_active_xmage_serve();

        Glib::spawn_async_with_pipes( server_path , argvs );
}

void XmageLauncher::show_setting( void )
{
    setting_dialog->show_all();
}

void XmageLauncher::close_setting( int )
{
        setting_dialog->hide();
}

void XmageLauncher::disable_launch( void )
{
    Gtk::Button * client_button;
    this->launcher_builder->get_widget( "LauncherClient" , client_button );
    client_button->set_sensitive( true );
    Gtk::Button * server_button;
    this->launcher_builder->get_widget( "LauncherServer" , server_button );
    server_button->set_sensitive( true );
    Gtk::Button * xmage_button;
    this->launcher_builder->get_widget( "LauncherXmage" , xmage_button );
    xmage_button->set_sensitive( true );
}

void XmageLauncher::enable_launch( void )
{
    Gtk::Button * client_button;
    this->launcher_builder->get_widget( "LauncherClient" , client_button );
    client_button->set_sensitive( false );
    Gtk::Button * server_button;
    this->launcher_builder->get_widget( "LauncherServer" , server_button );
    server_button->set_sensitive( false );
    Gtk::Button * xmage_button;
    this->launcher_builder->get_widget( "LauncherXmage" , xmage_button );
    xmage_button->set_sensitive( false );
}

void XmageLauncher::do_update( void )
{
    this->update_process = std::thread(
        [ this ]()
        {
            this->update.do_update( this );
        }
    );
}

void XmageLauncher::update_prog( void )
{
    std::int64_t now = 0 , total = 0;
    Glib::ustring info;
    Gtk::ProgressBar * progress_bar = nullptr;
    Gtk::Label * progress_target = nullptr;
    Gtk::Label * progress_value;

    this->launcher_builder->get_widget( "Progress" , progress_bar );
    this->launcher_builder->get_widget( "ProgressTarget" , progress_target );
    this->launcher_builder->get_widget( "ProgressValue" , progress_value );
    this->update.get_data( now , total , info );
    progress_target->set_label( info );
    progress_value->set_label( std::to_string( now ) + " / "  + std::to_string( total ) );
    if ( total == 0 )
        progress_bar->set_fraction( 0 );
    else
        progress_bar->set_fraction( now/static_cast<gdouble>( total ) );
}

void XmageLauncher::info_notify( void )
{
    this->update_dispatcher.emit();
}

config_t& XmageLauncher::get_config( void )
{
    return std::ref( this->config );
}

/* void update_xmage_callback( config_t& config , XmageType type , progress_t * progress , Gtk::Label * progress_label )
{
    auto update_future = get_last_version( type );
    xmage_desc_t update_desc;
    try
    {
        update_desc = update_future.get();
    }
    catch ( const std::exception& e )
    {
        //get update information failure
        g_log( __func__ , G_LOG_LEVEL_MESSAGE , "%s" , e.what() );
        return ;
    }
    Glib::ustring version;
    if ( type == XmageType::Release )
    {
        version = config.get_release_version();
    }
    else
    {
        version = config.get_beta_version();
    }

    g_log( __func__ , G_LOG_LEVEL_MESSAGE , _( "last xmage version:'%s',download url:'%s'." ) , update_desc.version_name.c_str() , update_desc.download_url.c_str() );
    if ( update_desc.version_name.compare( version ) )
    {
        g_log( __func__ , G_LOG_LEVEL_MESSAGE , "%s" , _( "exist new xmage,now download." ) );
    }
    else
    {
        progress_label->set_label( _( "no need to update" ) );
        return ;
    }

    std::shared_future<bool> download_future;
    //if exists
    if ( std::filesystem::is_regular_file( get_installation_package_name( update_desc ).raw() ) == false )
    {
        download_future = download_xmage( update_desc , progress );
        progress_label->set_label( _(" download update" ) );
        if ( download_future.get() )
        {
            progress_label->set_label( _( "download success,install..." ) );
            std::filesystem::rename( get_download_temp_name( update_desc ).raw() , get_installation_package_name( update_desc ).raw() );
        }
        else
        {
            progress_label->set_label( _( "download failure" ) );
            return ;
        }
    }
    else
    {
        g_log( __func__ , G_LOG_LEVEL_MESSAGE , _( "using local installation package" ) );
    }
    Glib::ustring install_path;
    if ( type == XmageType::Release )
    {
        install_path = config.get_release_path();
    }
    else
    {
        install_path = config.get_beta_path();
    }
    auto install_future = install_xmage( get_installation_package_name( update_desc ) ,  install_path , progress );
    progress_label->set_label( _( "install update" ) );
    if ( install_future.get() == false )
    {
        progress_label->set_label( _( "install faliure" ) );
        return ;
    }
    progress_label->set_label( _( "install success" ) );
    if ( type == XmageType::Release )
        config.set_release_version( update_desc.version_name );
    else
        config.set_beta_version( update_desc.version_name );
    std::filesystem::remove( get_installation_package_name( update_desc ).raw() );
} */

//    curl_off_t dlnow;
//    curl_off_t dltotal;
//    Glib::Dispatcher progress_dispatcher;
//    progress_dispatcher.connect(
//        [ &dlnow , &dltotal , progress_value , progress_bar ]()
//        {
//            progress_value->set_label( std::to_string( dlnow ) + " / "  + std::to_string( dltotal ) );
//            if ( dltotal == 0 )
//                progress_bar->set_fraction( 0 );
//            else
//                progress_bar->set_fraction( dlnow/static_cast<gdouble>( dltotal ) );
//        }
//    );
//    progress_t download_desc = { &dlnow , &dltotal , &progress_dispatcher };
//    XmageType type = config.get_active_xmage();
//    auto download_desc_ptr = &download_desc;
//    std::thread update_thread( update_xmage_callback , std::ref( config ) , type , download_desc_ptr ,  progress_target );
//    update_thread.detach(); */

