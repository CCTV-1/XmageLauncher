#include "launcherconfig.h"

#include <functional>

//config.ini content like:
//[Beta]
//version=1.4.35.dev_2019-04-28_20-43
//installed_path=BetaXmage
//[Release]
//version=xmage_1.4.35V2
//installed_path=ReleaseXmage
//[Setting]
//using_proxy=false
//proxy_scheme=http
//proxy_host=localhost
//proxy_port=1080
//jvm_xms=256
//jvm_xmx=1024
//#0 -> XmageType::Beta 1->XmageType::Release
//active_xmage=1
constexpr const char * CONFIG_FILE_NAME = "config.conf";

LauncherConfig::LauncherConfig()
{
    this->config_file = Glib::KeyFile();
    try
    {
        this->config_file.load_from_file( CONFIG_FILE_NAME , Glib::KeyFileFlags::KEY_FILE_KEEP_COMMENTS | Glib::KeyFileFlags::KEY_FILE_KEEP_TRANSLATIONS );
    }
    //access file error or file parse error getter function set config to default value,constructor only output log.
    catch( const Glib::FileError& e )
    {
        g_log( __func__ , G_LOG_LEVEL_MESSAGE , "load config file faliure,exception type:Glib::FileError,code:%d" , e.code() );
    }
    catch( const Glib::KeyFileError& e )
    {
        g_log( __func__ , G_LOG_LEVEL_MESSAGE , "load config file faliure,exception type:Glib::FileError code:%d" , e.code() );
    }
}

LauncherConfig::~LauncherConfig()
{
    this->config_file.save_to_file( CONFIG_FILE_NAME );
}

Glib::ustring LauncherConfig::get_java_path()
{
    return Glib::find_program_in_path( "java" );
}
Glib::ustring LauncherConfig::get_beta_version()
{
    if ( this->config_file.has_key( "Beta" , "version" ) == false )
    {
        this->config_file.set_string( "Beta" , "version" , "1.4.35.dev_2019-04-28_20-43" );
    }
    return this->config_file.get_string( "Beta" , "version" );
}
Glib::ustring LauncherConfig::get_beta_path()
{
    if ( this->config_file.has_key( "Beta" , "installed_path" ) == false )
    {
        this->config_file.set_string( "Beta" , "installed_path" , "BetaXmage" );
    }
    return this->config_file.get_string( "Beta" , "installed_path" );
}
Glib::ustring LauncherConfig::get_release_version()
{
    if ( this->config_file.has_key( "Release" , "version" ) == false )
    {
        this->config_file.set_string( "Release" , "version" , "xmage_1.4.35V2" );
    }
    return this->config_file.get_string( "Release" , "version" );
}
Glib::ustring LauncherConfig::get_release_path()
{
    if ( this->config_file.has_key( "Release" , "installed_path" ) == false )
    {
        this->config_file.set_string( "Release" , "installed_path" , "ReleaseXmage" );
    }
    return this->config_file.get_string( "Release" , "installed_path" );
}
bool LauncherConfig::get_using_proxy()
{
    if ( this->config_file.has_key( "Setting" , "using_proxy" ) == false )
    {
        this->config_file.set_boolean( "Setting" , "using_proxy" , false );
    }
    return this->config_file.get_boolean( "Setting" , "using_proxy" );
}
Glib::ustring LauncherConfig::get_proxy_scheme()
{
    if ( this->config_file.has_key( "Setting" , "proxy_scheme" ) == false )
    {
        this->config_file.set_string( "Setting" , "proxy_scheme" , "http" );
    }
    return this->config_file.get_string( "Setting" , "proxy_scheme" );
}
Glib::ustring LauncherConfig::get_proxy_host()
{
    if ( this->config_file.has_key( "Setting" , "proxy_host" ) == false )
    {
        this->config_file.set_string( "Setting" , "proxy_host" , "localhost" );
    }
    return this->config_file.get_string( "Setting" , "proxy_host" );
}
std::uint32_t LauncherConfig::get_proxy_port()
{
    if ( this->config_file.has_key( "Setting" , "proxy_port" ) == false )
    {
        this->config_file.set_integer( "Setting" , "proxy_port" , 1080 );
    }
    return this->config_file.get_integer( "Setting" , "proxy_port" );
}
std::uint32_t LauncherConfig::get_jvm_xms()
{
    if ( this->config_file.has_key( "Setting" , "jvm_xms" ) == false )
    {
        this->config_file.set_integer( "Setting" , "jvm_xms" , 256 );
    }
    return this->config_file.get_integer( "Setting" , "jvm_xms" );
}
std::uint32_t LauncherConfig::get_jvm_xmx()
{
    if ( this->config_file.has_key( "Setting" , "jvm_xmx" ) == false )
    {
        this->config_file.set_integer( "Setting" , "jvm_xmx" , 1024 );
    }
    return this->config_file.get_integer( "Setting" , "jvm_xmx" );
}
XmageType LauncherConfig::get_active_xmage()
{
    if ( this->config_file.has_key( "Setting" , "active_xmage" ) == false )
    {
        this->config_file.set_integer( "Setting" , "active_xmage" , static_cast<int>( XmageType::Release ) );
    }
    return static_cast<XmageType>( this->config_file.get_integer( "Setting" , "active_xmage" ) );
}


//only getter
Glib::ustring LauncherConfig::get_beta_client()
{
    return this->get_beta_path() + "/mage-client/";
}
Glib::ustring LauncherConfig::get_beta_server()
{
    return this->get_beta_path() + "/mage-server/";
}
Glib::ustring LauncherConfig::get_release_client()
{
    return this->get_release_path() + "/mage-client/";
}
Glib::ustring LauncherConfig::get_release_server()
{
    return this->get_release_path() + "/mage-server/";
}
Glib::ustring LauncherConfig::get_release_mage_version( void )
{
    //xmage_1.4.35V1 to 1.4.35
    Glib::ustring mage_version;
    Glib::ustring release_version = this->get_release_version();

    for (
        std::uint32_t i = sizeof( "xmage_" )/sizeof( char ) - 1;
        release_version[i] != 'V' && i <= release_version.size();
        i++
    )
    {
        mage_version += release_version[i];
    }
    return mage_version;
}
Glib::ustring LauncherConfig::get_beta_mage_version( void )
{
    //1.4.35.dev_2019-04-28_20-43 to 1.4.35
    Glib::ustring mage_version;
    Glib::ustring beta_version = this->get_beta_version();

    std::uint32_t dot_num = 0;
    for ( std::uint32_t i = 0 ; dot_num <=2 && i <= beta_version.size() ; i++ )
    {
        if ( beta_version[i] == '.' )
            dot_num++;
        mage_version += beta_version[i];
    }
    return mage_version;
}
Glib::ustring LauncherConfig::get_active_xmage_version( void )
{
    XmageType active_type = this->get_active_xmage();
    Glib::ustring version;
    switch ( active_type )
    {
        default:
        case XmageType::Release:
        {
            version = this->get_release_mage_version();
            break;
        }
        case XmageType::Beta:
        {
            version = this->get_beta_mage_version();
            break;
        }
    }

    return version;
}
Glib::ustring LauncherConfig::get_active_xmage_client( void )
{
    XmageType active_type = this->get_active_xmage();
    Glib::ustring version;
    switch ( active_type )
    {
        default:
        case XmageType::Release:
        {
            version = this->get_release_client();
            break;
        }
        case XmageType::Beta:
        {
            version = this->get_beta_client();
            break;
        }
    }

    return version;
}
Glib::ustring LauncherConfig::get_active_xmage_serve( void )
{
    XmageType active_type = this->get_active_xmage();
    Glib::ustring version;
    switch ( active_type )
    {
        default:
        case XmageType::Release:
        {
            version = this->get_release_server();
            break;
        }
        case XmageType::Beta:
        {
            version = this->get_beta_server();
            break;
        }
    }

    return version;
}
Glib::ustring LauncherConfig::get_active_xmage_path( void )
{
    XmageType active_type = this->get_active_xmage();
    Glib::ustring version;
    switch ( active_type )
    {
        default:
        case XmageType::Release:
        {
            version = this->get_release_path();
            break;
        }
        case XmageType::Beta:
        {
            version = this->get_beta_path();
            break;
        }
    }

    return version;
}

LauncherConfig& LauncherConfig::set_beta_version( const Glib::ustring& beta_version )
{
    this->config_file.set_string( "Beta" , "version" , beta_version );
    return *this;
}
LauncherConfig& LauncherConfig::set_beta_path( const Glib::ustring& beta_path )
{
    this->config_file.set_string( "Beta" , "installed_path" , beta_path );
    return *this;
}
LauncherConfig& LauncherConfig::set_release_version( const Glib::ustring& release_version )
{
    this->config_file.set_string( "Release" , "version" , release_version );
    return *this;
}
LauncherConfig& LauncherConfig::set_release_path( const Glib::ustring& release_path )
{
    this->config_file.set_string( "Release" , "installed_path" , release_path );
    return *this;
}
LauncherConfig& LauncherConfig::set_using_proxy( const bool& using_proxy )
{
    this->config_file.set_boolean( "Setting" , "using_proxy" , using_proxy );
    return *this;
}
LauncherConfig& LauncherConfig::set_proxy_scheme( const Glib::ustring& proxy_scheme )
{
    this->config_file.set_string( "Setting" , "proxy_scheme" , proxy_scheme );
    return *this;
}
LauncherConfig& LauncherConfig::set_proxy_host( const Glib::ustring& proxy_host )
{
    this->config_file.set_string( "Setting" , "proxy_host" , proxy_host );
    return *this;
}
LauncherConfig& LauncherConfig::set_proxy_port( const std::uint32_t& proxy_port )
{
    this->config_file.set_integer( "Setting" , "proxy_port" , proxy_port );
    return *this;
}
LauncherConfig& LauncherConfig::set_jvm_xms( const std::uint32_t& jvm_xms )
{
    this->config_file.set_integer( "Setting" , "jvm_xms" , jvm_xms );
    return *this;
}
LauncherConfig& LauncherConfig::set_jvm_xmx( const std::uint32_t& jvm_xmx )
{
    this->config_file.set_integer( "Setting" , "jvm_xmx" , jvm_xmx );
    return *this;
}
LauncherConfig& LauncherConfig::set_active_xmage( const XmageType& type )
{
    this->config_file.set_integer( "Setting" , "active_xmage" , static_cast<int>( type ) );
    return *this;
}

LauncherConfig& LauncherConfig::get_config( void )
{
    static LauncherConfig config;
    return config;
}
