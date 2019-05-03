#include "launcherconfig.h"

#include <functional>

constexpr const char * CONFIG_FILE_NAME = "config.conf";

LauncherConfig::LauncherConfig()
{
    this->config_file = Glib::KeyFile();
    try
    {
        this->config_file.load_from_file( CONFIG_FILE_NAME , Glib::KeyFileFlags::KEY_FILE_KEEP_COMMENTS | Glib::KeyFileFlags::KEY_FILE_KEEP_TRANSLATIONS );
    }
    catch( const Glib::FileError& e )
    {
        //config file don't exitst,fill default value.
        //if don't first get_type then set_type,keyfile don't exists key,throw Glib::KeyFileError.
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
        this->config_file.get_string( "Beta" , "version" );
        this->config_file.set_string( "Beta" , "version" , "1.4.35.dev_2019-04-28_20-43" );
        this->config_file.get_string( "Beta" , "installed_path" );
        this->config_file.set_string( "Beta" , "installed_path" , "BetaXmage" );
        this->config_file.get_string( "Release" , "version" );
        this->config_file.set_string( "Release" , "version" , "xmage_1.4.35V2" );
        this->config_file.get_string( "Release" , "installed_path" );
        this->config_file.set_string( "Release" , "installed_path" , "ReleaseXmage" );
        this->config_file.get_boolean( "Setting" , "using_proxy" );
        this->config_file.set_boolean( "Setting" , "using_proxy" , false );
        this->config_file.get_string( "Setting" , "proxy_scheme" );
        this->config_file.set_string( "Setting" , "proxy_scheme" , "http" );
        this->config_file.get_string( "Setting" , "proxy_host" );
        this->config_file.set_string( "Setting" , "proxy_host" , "localhost" );
        this->config_file.get_integer( "Setting" , "proxy_port" );
        this->config_file.set_integer( "Setting" , "proxy_port" , 1080 );
        this->config_file.get_integer( "Setting" , "jvm_xms" );
        this->config_file.set_integer( "Setting" , "jvm_xms" , 256 );
        this->config_file.get_integer( "Setting" , "jvm_xmx" );
        this->config_file.set_integer( "Setting" , "jvm_xmx" , 1024 );
        this->config_file.get_integer( "Setting" , "update_source" );
        this->config_file.set_integer( "Setting" , "update_source" , static_cast<int>( XmageType::Release ) );
        g_log( __func__ , G_LOG_LEVEL_MESSAGE , "Glib::FileError code:%d" , e.code() );
    }
    catch( const Glib::KeyFileError& e )
    {
        g_log( __func__ , G_LOG_LEVEL_MESSAGE , "Glib::FileError code:%d" , e.code() );
    }

    this->java_path = Glib::find_program_in_path( "java" );
    if ( this->java_path == "" )
    {
        g_log( __func__ , G_LOG_LEVEL_WARNING , "can not find java path" );
    }

    this->beta_version = this->config_file.get_string( "Beta" , "version" );
    this->beta_path = this->config_file.get_string( "Beta" , "installed_path" );
    this->beta_client = this->beta_path + "/mage-client/";
    this->beta_server = this->beta_path + "/mage-server/";
    this->release_version = this->config_file.get_string( "Release" , "version" );
    this->release_path = this->config_file.get_string( "Release" , "installed_path" );
    this->release_client = this->release_path  + "/mage-client/";
    this->release_server = this->release_path  + "/mage-server/";
    this->using_proxy = this->config_file.get_boolean( "Setting" , "using_proxy" );
    this->proxy_scheme = this->config_file.get_string( "Setting" , "proxy_scheme" );
    this->proxy_host = this->config_file.get_string( "Setting" , "proxy_host" );
    this->proxy_port = this->config_file.get_integer( "Setting" , "proxy_port" );
    this->jvm_xms = this->config_file.get_integer( "Setting" , "jvm_xms" );
    this->jvm_xmx = this->config_file.get_integer( "Setting" , "jvm_xmx" );
    this->update_source = static_cast<XmageType>( this->config_file.get_integer( "Setting" , "update_source" ) );
}

LauncherConfig::~LauncherConfig()
{
    this->config_file.save_to_file( CONFIG_FILE_NAME );
}

const Glib::ustring& LauncherConfig::get_java_path()
{
    return this->java_path;
}
const Glib::ustring& LauncherConfig::get_beta_version()
{
    return this->beta_version;
}
const Glib::ustring& LauncherConfig::get_beta_path()
{
    return this->beta_path;
}
const Glib::ustring& LauncherConfig::get_release_version()
{
    return this->release_version;
}
const Glib::ustring& LauncherConfig::get_release_path()
{
    return this->release_path;
}
const bool& LauncherConfig::get_using_proxy()
{
    return this->using_proxy;
}
const Glib::ustring& LauncherConfig::get_proxy_scheme()
{
    return this->proxy_scheme;
}
const Glib::ustring& LauncherConfig::get_proxy_host()
{
    return this->proxy_host;
}
const std::uint32_t& LauncherConfig::get_proxy_port()
{
    return this->proxy_port;
}
const std::uint32_t& LauncherConfig::get_jvm_xms()
{
    return this->jvm_xms;
}
const std::uint32_t& LauncherConfig::get_jvm_xmx()
{
    return this->jvm_xmx;
}
const XmageType& LauncherConfig::get_update_source()
{
    return this->update_source;
}


//only setter
const Glib::ustring& LauncherConfig::get_beta_client()
{
    return this->beta_client;
}
const Glib::ustring& LauncherConfig::get_beta_server()
{
    return this->beta_server;
}
const Glib::ustring& LauncherConfig::get_release_client()
{
    return this->release_client;
}
const Glib::ustring& LauncherConfig::get_release_server()
{
    return this->release_server;
}
Glib::ustring LauncherConfig::get_release_mage_version( void )
{
    //xmage_1.4.35V1 to 1.4.35
    Glib::ustring mage_version;

    for (
        std::uint32_t i = sizeof( "xmage_" )/sizeof( char ) - 1;
        this->release_version[i] != 'V' && i <= this->release_version.size();
        i++
    )
    {
        mage_version += this->release_version[i];
    }
    return mage_version;
}
Glib::ustring LauncherConfig::get_beta_mage_version( void )
{
    //1.4.35.dev_2019-04-28_20-43 to 1.4.35
    Glib::ustring mage_version;

    std::uint32_t dot_num = 0;
    for ( std::uint32_t i = 0 ; dot_num <=2 && i <= this->release_version.size() ; i++ )
    {
        if ( this->beta_version[i] == '.' )
            dot_num++;
        mage_version += this->beta_version[i];
    }
    return mage_version;
}

LauncherConfig& LauncherConfig::set_beta_version( const Glib::ustring& _beta_version )
{
	this->beta_version = _beta_version;
    this->config_file.set_string( "Beta" , "version" , _beta_version );
    return *this;
}
LauncherConfig& LauncherConfig::set_beta_path( const Glib::ustring& _beta_path )
{
	this->beta_path = _beta_path;
    this->config_file.set_string( "Beta" , "installed_path" , _beta_path );
    return *this;
}
LauncherConfig& LauncherConfig::set_release_version( const Glib::ustring& _release_version )
{
	this->release_version = _release_version;
    this->config_file.set_string( "Release" , "version" , _release_version );
    return *this;
}
LauncherConfig& LauncherConfig::set_release_path( const Glib::ustring& _release_path )
{
	this->release_path = _release_path;
    this->config_file.set_string( "Release" , "installed_path" , _release_path );
    return *this;
}
LauncherConfig& LauncherConfig::set_using_proxy( const bool& _using_proxy )
{
	this->using_proxy = _using_proxy;
    this->config_file.set_boolean( "Setting" , "using_proxy" , _using_proxy );
    return *this;
}
LauncherConfig& LauncherConfig::set_proxy_scheme( const Glib::ustring& _proxy_scheme )
{
	this->proxy_scheme = _proxy_scheme;
    this->config_file.set_string( "Setting" , "proxy_scheme" , _proxy_scheme );
    return *this;
}
LauncherConfig& LauncherConfig::set_proxy_host( const Glib::ustring& _proxy_host )
{
	this->proxy_host = _proxy_host;
    this->config_file.set_string( "Setting" , "proxy_host" , _proxy_host );
    return *this;
}
LauncherConfig& LauncherConfig::set_proxy_port( const std::uint32_t& _proxy_port )
{
	this->proxy_port = _proxy_port;
    this->config_file.set_integer( "Setting" , "proxy_port" , _proxy_port );
    return *this;
}
LauncherConfig& LauncherConfig::set_jvm_xms( const std::uint32_t& _jvm_xms )
{
	this->jvm_xms = _jvm_xms;
    this->config_file.set_integer( "Setting" , "jvm_xms" , _jvm_xms );
    return *this;
}
LauncherConfig& LauncherConfig::set_jvm_xmx( const std::uint32_t& _jvm_xmx )
{
	this->jvm_xmx = _jvm_xmx;
    this->config_file.set_integer( "Setting" , "jvm_xmx" , _jvm_xmx );
    return *this;
}
LauncherConfig& LauncherConfig::set_java_path( const Glib::ustring& _java_path )
{
    this->java_path = _java_path;
    return *this;
}
LauncherConfig& LauncherConfig::set_update_source( const XmageType& type )
{
    this->update_source = type;
    this->config_file.set_integer( "Setting" , "update_source" , static_cast<int>( type ) );
    return *this;
}

LauncherConfig& LauncherConfig::get_config( void )
{
    static LauncherConfig config;
    return config;
}
