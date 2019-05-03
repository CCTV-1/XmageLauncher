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
        this->config_file.set_string( "Beta" , "version" , "1.4.35.dev_2019-04-28_20-43" );
        this->config_file.set_string( "Beta" , "installed_path" , "BetaXmage" );
        this->config_file.set_string( "Release" , "version" , "xmage_1.4.35V2" );
        this->config_file.set_string( "Release" , "installed_path" , "ReleaseXmage" );
        this->config_file.set_boolean( "Setting" , "using_proxy" , false );
        this->config_file.set_string( "Setting" , "proxy_scheme" , "http" );
        this->config_file.set_string( "Setting" , "proxy_host" , "localhost" );
        this->config_file.set_integer( "Setting" , "proxy_port" , 1080 );
        this->config_file.set_integer( "Setting" , "jvm_xms" , 256 );
        this->config_file.set_integer( "Setting" , "jvm_xmx" , 1024 );
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
}

LauncherConfig::~LauncherConfig()
{
    this->config_file.save_to_file( CONFIG_FILE_NAME );
}

#define getter(type,membername) const type& LauncherConfig::get_##membername() { return this->membername; }
    getter( Glib::ustring , java_path )
    getter( Glib::ustring , beta_version )
    getter( Glib::ustring , beta_path )
    getter( Glib::ustring , release_version )
    getter( Glib::ustring , release_path )
    getter( bool          , using_proxy )
    getter( Glib::ustring , proxy_scheme )
    getter( Glib::ustring , proxy_host )
    getter( std::uint32_t , proxy_port )
    getter( std::uint32_t , jvm_xms )
    getter( std::uint32_t , jvm_xmx )

    //only setter
    getter( Glib::ustring , beta_client )
    getter( Glib::ustring , beta_server )
    getter( Glib::ustring , release_client )
    getter( Glib::ustring , release_server )
#undef getter
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
    setter( std::uint32_t , integer , Setting , jvm_xms        , jvm_xms )
    setter( std::uint32_t , integer , Setting , jvm_xmx        , jvm_xmx )
#undef setter

//getter special case
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

//setter special case
LauncherConfig& LauncherConfig::set_java_path( const Glib::ustring& _java_path )
{
    this->java_path = _java_path;
    return *this;
}

LauncherConfig& LauncherConfig::get_config( void )
{
    static LauncherConfig config;
    return config;
}
