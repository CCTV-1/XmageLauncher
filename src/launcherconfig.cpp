#include "launcherconfig.h"

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
        //config file don't exitst,fill default value
        //config.ini content like:
        //[Beta]
        //version=1.4.35.dev_2019-04-24_20-55
        //installed_path=betamage
        //[Release]
        //version=1.4.35V1
        //installed_path=releasemage
        //[Setting]
        //using_proxy=false
        //proxy_scheme=http
        //proxy_host=localhost
        //proxy_port=1080
        this->config_file.set_string( "Beta" , "version" , "1.4.35.dev_2019-04-24_20-55" );
        this->config_file.set_string( "Beta" , "installed_path" , "betamage" );
        this->config_file.set_string( "Release" , "version" , "1.4.35V1" );
        this->config_file.set_string( "Release" , "installed_path" , "releasemage" );
        this->config_file.set_boolean( "Setting" , "using_proxy" , false );
        this->config_file.set_string( "Setting" , "proxy_scheme" , "http" );
        this->config_file.set_string( "Setting" , "proxy_host" , "localhost" );
        this->config_file.set_integer( "Setting" , "proxy_port" , 1080 );
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
    this->release_client = this->config_file.get_string( "Release" , "installed_path" ) + "/mage-client/";
    this->release_server = this->config_file.get_string( "Release" , "installed_path" ) + "/mage-server/";
    this->using_proxy = this->config_file.get_boolean( "Setting" , "using_proxy" );
    this->proxy_scheme = this->config_file.get_string( "Setting" , "proxy_scheme" );
    this->proxy_host = this->config_file.get_string( "Setting" , "proxy_host" );
    this->proxy_port = this->config_file.get_integer( "Setting" , "proxy_port" );
}

LauncherConfig::~LauncherConfig()
{
    this->config_file.save_to_file( CONFIG_FILE_NAME );
}

#define getter(type,membername) const type& LauncherConfig::get_##membername() { return this->membername; }
    getter( Glib::ustring , java_path )
    getter( Glib::ustring , beta_version )
    getter( Glib::ustring , beta_path )
    getter( Glib::ustring , beta_client )
    getter( Glib::ustring , beta_server )
    getter( Glib::ustring , release_version )
    getter( Glib::ustring , release_path )
    getter( Glib::ustring , release_client )
    getter( Glib::ustring , release_server )
    getter( bool          , using_proxy )
    getter( Glib::ustring , proxy_scheme )
    getter( Glib::ustring , proxy_host )
    getter( std::uint32_t , proxy_port )
#undef getter
#define setter(type,membername) LauncherConfig& LauncherConfig::set_##membername( const type& _##membername ) { this->membername = _##membername; }
    setter( Glib::ustring , java_path )
    setter( Glib::ustring , beta_client )
    setter( Glib::ustring , beta_server )
    setter( Glib::ustring , release_client )
    setter( Glib::ustring , release_server )
#undef setter
#define setter(type,config_type,config_group,config_key,membername)\
    LauncherConfig& LauncherConfig::set_##membername( const type& _##membername )\
    {\
        this->membername = _##membername;\
        this->config_file.set_##config_type( #config_group , #config_key , _##membername );\
        return *this;\
    }
    setter( Glib::ustring , string  , Beta    , version        , beta_version )
    setter( Glib::ustring , string  , Beta    , installed_path , beta_path )
    setter( Glib::ustring , string  , Release , version        , release_version )
    setter( Glib::ustring , string  , Release , installed_path , release_path )
    setter( bool          , boolean , Setting , using_proxy    , using_proxy )
    setter( Glib::ustring , string  , Setting , proxy_scheme   , proxy_scheme )
    setter( Glib::ustring , string  , Setting , proxy_host     , proxy_host )
    setter( std::uint32_t , integer , Setting , proxy_port     , proxy_port )
#undef setter

LauncherConfig& LauncherConfig::get_config( void )
{
    static LauncherConfig config;
    return config;
}
