#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <functional>
#include <filesystem>
#include <fstream>
#include <memory>
#include <thread>

#include <zip.h>
#include <jansson.h>
#include <glibmm/i18n.h>

#include "utilities.h"
#include "launcher.h"

typedef struct JsonBuff
{
    char * buff;
    std::size_t current_size;
}json_buff_t;

static Glib::ustring _proxy_desc;

static void common_curl_opt_set( std::shared_ptr<CURL> curl_handle )
{
    constexpr char user_agent[] = "XmageLauncher";
    curl_easy_setopt( curl_handle.get() , CURLOPT_USERAGENT , user_agent );
    curl_easy_setopt( curl_handle.get() , CURLOPT_VERBOSE , 0L );
    curl_easy_setopt( curl_handle.get() , CURLOPT_FOLLOWLOCATION , 1L );
    curl_easy_setopt( curl_handle.get() , CURLOPT_USE_SSL , 1L );
    if ( _proxy_desc.empty() == false )
    {
        curl_easy_setopt( curl_handle.get() , CURLOPT_PROXY , _proxy_desc.c_str() );
    }
}

static int download_description_callback( void * client_ptr , curl_off_t dltotal , curl_off_t dlnow , curl_off_t , curl_off_t )
{
    if ( client_ptr == nullptr )
        return 0;
    progress_t * progress = static_cast<progress_t *>( client_ptr );
    if ( progress == nullptr || progress->caller == nullptr || progress->work == nullptr )
    {
        return false;
    }
    //notify progress update
    {
        std::lock_guard<std::mutex> lock( progress->work->update_mutex );
        progress->work->prog_now = dlnow;
        progress->work->prog_total = dltotal;
    }
    progress->caller->info_notify();
    return 0;
}

static std::size_t get_json_callback( char * content , std::size_t size , std::size_t element_number , void * save_ptr )
{
    std::size_t realsize = size*element_number;
    json_buff_t * ptr = static_cast< json_buff_t * >( save_ptr );
    char * new_buff = static_cast<char *>( realloc( ptr->buff , ptr->current_size + sizeof( char )*realsize + 1 ) );
    ptr->buff = new_buff;
    memccpy( &( ptr->buff[ptr->current_size] ) , content , 1 , realsize );
    ptr->current_size += realsize;
    ptr->buff[ptr->current_size] = '\0';
    return realsize;
}

static xmage_desc_t get_last_release_version( void ) noexcept( false )
{
    std::string except_message( __func__ );
    //http://xmage.de/xmage/config.json
    //{
    //"java" : {
    //    "version": "1.8.0_201",
    //    "old_location": "http://download.oracle.com/otn-pub/java/jdk/8u201-b09/42970487e3af4f5aa5bca3f542482c60/jre-8u201-",
    //    "location": "http://xmage.today/java/jre-8u201-"
    //},
    //"XMage" : {
    //    "version": "1.4.35V1 (2019-04-24)",
    //    "location": "https://github.com/magefree/mage/releases/download/xmage_1.4.35V1/xmage_1.4.35V1.zip",
    //    "locations": ["http://xmage.de/files/xmage_1.4.35V1.zip"],
    //    "torrent": "",
    //    "images": "",
    //    "Launcher" : {
    //    "version": "0.3.8",
    //    "location": "http://bit.ly/xmageLauncher038"
    //    } 
    //}
    //}
    Glib::ustring api_url = "https://api.github.com/repos/magefree/mage/releases/latest";
    //{
    //    ... unimportant ...
    //    "name": "xmage_1.4.35V0",
    //    ... unimportant ...
    //    "assets": [
    //        {
    //            "name": "mage_1.4.35V0.zip"
    //            ... unimportant ...
    //            "browser_download_url": "https://github.com/magefree/mage/releases/download/xmage_1.4.35V0/xmage_1.4.35V0.zip"
    //        }
    //        {
    //            "name": "mage_1.4.35V0a.zip"
    //            ... unimportant ...
    //            "browser_download_url": "https://github.com/magefree/mage/releases/download/xmage_1.4.35V0/xmage_1.4.35V0a.zip"
    //        }
    //    ],
    //    ... unimportant ...
    //}

    std::int8_t re_try = 4;
    long default_timeout = 30L;
    json_buff_t json_buff = { nullptr , 0 };
    char error_buff[CURL_ERROR_SIZE];
    //c str* safe
    error_buff[0] = '\0';

    CURLcode res = CURLE_OK;
    std::shared_ptr<CURL> curl_handle( curl_easy_init() , curl_easy_cleanup );
    if ( curl_handle.get() == nullptr )
    {
        except_message += ":libcurl easy initial failure";
        throw std::runtime_error( except_message );
    }
    do
    {
        common_curl_opt_set( curl_handle );
        curl_easy_setopt( curl_handle.get() , CURLOPT_URL , api_url.c_str() );
        curl_easy_setopt( curl_handle.get() , CURLOPT_NOPROGRESS , 1L );
        curl_easy_setopt( curl_handle.get() , CURLOPT_TIMEOUT , default_timeout );
        curl_easy_setopt( curl_handle.get() , CURLOPT_WRITEFUNCTION , get_json_callback );
        curl_easy_setopt( curl_handle.get() , CURLOPT_WRITEDATA , &json_buff );
        curl_easy_setopt( curl_handle.get() , CURLOPT_ERRORBUFFER , error_buff );
        res = curl_easy_perform( curl_handle.get() );
        if ( res != CURLE_OK )
        {
            if ( re_try == 0 )
            {
                except_message += ":get last release version failure,libcurl error message:";
                except_message += error_buff;
                throw std::runtime_error( except_message );
            }
            default_timeout += 10L;
            re_try--;
        }
    }
    while ( res != CURLE_OK );

    json_error_t error;
    std::shared_ptr<json_t> root( json_loads( json_buff.buff , 0 , &error ) , json_decref );
    if ( root.get() == nullptr )
    {
        except_message += ":network json:'";
        except_message += json_buff.buff;
        except_message += "' format does not meet expectations";
        throw std::invalid_argument( except_message );
    }

    json_t * assets_array = json_object_get( root.get() , "assets" );
    if ( json_is_array( assets_array )  == false )
    {
        except_message += ":network json:'";
        except_message += json_buff.buff;
        except_message += "' node 'assets' format does not meet expectations";
        throw std::invalid_argument( except_message );
    }
    //last version:V0 V0a V0b ... 0Vz,json_array_size( assets_array ) ->C array index style
    json_t * last_assets = json_array_get( assets_array , json_array_size( assets_array ) - 1 );
    if ( json_is_object( last_assets ) == false )
    {
        except_message += ":network json:'";
        std::shared_ptr<char> jsons( json_dumps( last_assets , JSON_INDENT( 4 ) ) , free );
        except_message += jsons.get();
        except_message += "' element 0 format does not meet expectations";
        throw std::invalid_argument( except_message );
    }

    json_t * download_url = json_object_get( last_assets , "browser_download_url" );
    if ( json_is_string( download_url )  == false )
    {
        except_message += ":network json:'";
        std::shared_ptr<char> jsons( json_dumps( last_assets , JSON_INDENT( 4 ) ) , free );
        except_message += jsons.get();
        except_message += "' format does not meet expectations";
        throw std::invalid_argument( except_message );
    }
    json_t * version_name = json_object_get( last_assets , "name" );
    if ( json_is_string( version_name )  == false )
    {
        except_message += ":network json:'";
        except_message += json_buff.buff;
        except_message += "' node 'name' format does not meet expectations";
        throw std::invalid_argument( except_message );
    }
    Glib::ustring zip_name( json_string_value( version_name ) );
    Glib::ustring version = zip_name;
    if ( Glib::str_has_suffix( zip_name , ".zip" ) )
    {
        //".zip" exists NUL sizeof 5
        version = zip_name.substr( 0 , zip_name.size() - sizeof( ".zip" ) + 1 );
    }

    free( json_buff.buff );
    return { version , json_string_value( download_url ) };
}

static xmage_desc_t get_last_beta_version( void ) noexcept( false )
{
    std::string except_message( __func__ );
    Glib::ustring api_url = "http://xmage.today/config.json";

    std::int8_t re_try = 4;
    long default_timeout = 30L;
    json_buff_t json_buff = { nullptr , 0 };
    char error_buff[CURL_ERROR_SIZE];
    //c str* safe
    error_buff[0] = '\0';

    CURLcode res = CURLE_OK;
    std::shared_ptr<CURL> curl_handle( curl_easy_init() , curl_easy_cleanup );
    if ( curl_handle.get() == nullptr )
    {
        except_message += ":libcurl easy initial failure";
        throw std::runtime_error( except_message );
    }
    do
    {
        common_curl_opt_set( curl_handle );
        curl_easy_setopt( curl_handle.get() , CURLOPT_URL , api_url.c_str() );
        curl_easy_setopt( curl_handle.get() , CURLOPT_NOPROGRESS , 1L );
        curl_easy_setopt( curl_handle.get() , CURLOPT_TIMEOUT , default_timeout );
        curl_easy_setopt( curl_handle.get() , CURLOPT_WRITEFUNCTION , get_json_callback );
        curl_easy_setopt( curl_handle.get() , CURLOPT_WRITEDATA , &json_buff );
        curl_easy_setopt( curl_handle.get() , CURLOPT_ERRORBUFFER , error_buff );
        res = curl_easy_perform( curl_handle.get() );
        if ( res != CURLE_OK )
        {
            if ( re_try == 0 )
            {
                except_message += ":get last beta version failure,libcurl error message:";
                except_message += error_buff;
                throw std::runtime_error( except_message );
            }
            default_timeout += 10L;
            re_try--;
        }
    }
    while ( res != CURLE_OK );

    //{
    //    "java": {
    //        "version": "1.8.0_201",
    //        "location": "http://xmage.today/java/jre-8u201-"
    //    },
    //    "XMage": {
    //        "version": "1.4.35.dev_2019-04-24_20-55",
    //        "location": "http://xmage.today/files/mage-update_1.4.35.dev_2019-04-24_20-55.zip",
    //        "locations": [],
    //        "full": "http://xmage.today/files/mage-full_1.4.35.dev_2019-04-24_20-55.zip",
    //        "torrent": "",
    //        "images": "",
    //        "Launcher": {
    //            "version": "0.3.8",
    //            "location": "http://bit.ly/xmageLauncher038"
    //        }
    //    }
    //}

    json_error_t error;
    std::shared_ptr<json_t> root( json_loads( json_buff.buff , 0 , &error ) , json_decref );
    if ( root.get() == nullptr )
    {
        except_message += ":network json:'";
        except_message += json_buff.buff;
        except_message += "' format does not meet expectations";
        throw std::invalid_argument( except_message );
    }

    json_t * client_node = json_object_get( root.get() , "XMage" );
    if ( json_is_object( client_node )  == false )
    {
        except_message += ":network json:'";
        std::shared_ptr<char> jsons( json_dumps( client_node , JSON_INDENT( 4 ) ) , free );
        except_message += jsons.get();
        except_message += "' node 'XMage' format does not meet expectations";
        throw std::invalid_argument( except_message );
    }

    json_t * version = json_object_get( client_node , "version" );
    if ( json_is_string( version )  == false )
    {
        except_message += ":network json:'";
        std::shared_ptr<char> jsons( json_dumps( client_node , JSON_INDENT( 4 ) ) , free );
        except_message += jsons.get();
        except_message += "' node 'version' format does not meet expectations";
        throw std::invalid_argument( except_message );
    }

    json_t * download_url = json_object_get( client_node , "full" );
    if ( json_is_string( download_url )  == false )
    {
        except_message += ":network json:'";
        std::shared_ptr<char> jsons( json_dumps( client_node , JSON_INDENT( 4 ) ) , free );
        except_message += jsons.get();
        except_message += "' format does not meet expectations";
        throw std::invalid_argument( except_message );
    }

    free( json_buff.buff );
    return { json_string_value( version ) , json_string_value( download_url ) };
}

static bool download_client_callback( xmage_desc_t client_desc , progress_t * download_desc )
{
    std::shared_ptr<CURL> curl_handle( curl_easy_init() , curl_easy_cleanup );

    //output temp file,download success,rename to version.zip,to support check local zip continue download.
    Glib::ustring temp_name = get_download_temp_name( client_desc );
    std::shared_ptr<FILE> download_file( fopen( temp_name.c_str() , "wb+" ) , fclose );
    if ( download_file.get() == nullptr )
    {
        g_log( __func__ , G_LOG_LEVEL_WARNING , "open file:\'%s\' failure." , temp_name.c_str() );
        return false;
    }

    long default_timeout = 60*60L;
    char error_buff[CURL_ERROR_SIZE];
    CURLcode res = CURLE_OK;

    common_curl_opt_set( curl_handle );
    curl_easy_setopt( curl_handle.get() , CURLOPT_URL , client_desc.download_url.c_str() );
    curl_easy_setopt( curl_handle.get() , CURLOPT_TIMEOUT , default_timeout );
    curl_easy_setopt( curl_handle.get() , CURLOPT_NOPROGRESS , 0L );
    if ( download_desc != nullptr )
        curl_easy_setopt( curl_handle.get() , CURLOPT_XFERINFOFUNCTION , download_description_callback );
        curl_easy_setopt( curl_handle.get() , CURLOPT_XFERINFODATA , download_desc );
    curl_easy_setopt( curl_handle.get() , CURLOPT_WRITEFUNCTION , fwrite );
    curl_easy_setopt( curl_handle.get() , CURLOPT_ERRORBUFFER , error_buff );
    curl_easy_setopt( curl_handle.get() , CURLOPT_WRITEDATA , download_file.get() );

    res = curl_easy_perform( curl_handle.get() );
    if ( res != CURLE_OK )
    {
        g_log( __func__ , G_LOG_LEVEL_MESSAGE , "download url:\'%s\',error type:\'%s\',error message:\'%s\'" , 
                client_desc.download_url.c_str() , curl_easy_strerror( res ) , error_buff );
        return false;
    }

    return true;
}

static bool install_xmage_callback( Glib::ustring client_zip_name , Glib::ustring unzip_path , progress_t * progress ) noexcept( false )
{
    if ( progress == nullptr || progress->caller == nullptr || progress->work == nullptr )
    {
        return false;
    }

    std::filesystem::path client_zip( client_zip_name.raw() );
    std::filesystem::path unzip_dir( unzip_path.raw() );
    if ( std::filesystem::exists( client_zip ) == false )
    {
        return false;
    }

    if ( std::filesystem::exists( unzip_dir ) == false )
        std::filesystem::create_directories( unzip_dir );
    std::shared_ptr<zip_t> zip_ptr( zip_open( client_zip_name.c_str() , ZIP_RDONLY , nullptr ) , zip_close );
    zip_int64_t file_number = zip_get_num_entries( zip_ptr.get() , ZIP_FL_UNCHANGED );
    {
        std::lock_guard<std::mutex> lock( progress->work->update_mutex );
        progress->work->prog_now = file_number;
    }

    zip_uint64_t buff_size = 1024 * 8 * sizeof( char );
    std::shared_ptr<char> data_buff( static_cast<char *>( calloc( buff_size , sizeof( char ) ) ) , free );
    for( zip_int64_t i = 0 ; i < file_number ; i++ )
    {
        struct zip_stat file_stat;
        zip_stat_init( &file_stat );
        zip_stat_index( zip_ptr.get() , i , ZIP_FL_ENC_GUESS , &file_stat );
        if ( file_stat.size > buff_size )
        {
            buff_size = file_stat.size;
            //discard old data
            data_buff = std::shared_ptr<char>( static_cast<char *>( calloc( buff_size , sizeof( char ) ) ) , free );
        }
        std::shared_ptr<zip_file_t> file_ptr( zip_fopen_index( zip_ptr.get() , i , ZIP_FL_UNCHANGED ) , zip_fclose );
        zip_fread( file_ptr.get() , data_buff.get() , file_stat.size );
        std::filesystem::path target_path;
        try
        {
            target_path = std::filesystem::path( unzip_path.raw() + std::string( "/" ) + file_stat.name );
            Glib::file_set_contents( target_path.string() , data_buff.get() , file_stat.size );
        }
        catch ( const Glib::FileError& e )
        {
            auto code = e.code();
            if (
                code == Glib::FileError::Code::NO_SUCH_ENTITY ||
                code == Glib::FileError::Code::ACCESS_DENIED
            )
            {
                std::filesystem::path parent_dir = target_path.parent_path();
                std::filesystem::create_directories( parent_dir );
            }
            else
            {
                g_log( __func__ , G_LOG_LEVEL_MESSAGE , "unzip file:'%s',Glib::FileError code:%d" , target_path.string().c_str() , code );
            }
        }
        //file name encoding not supported
        catch ( const std::exception& e )
        {
            g_log( __func__ , G_LOG_LEVEL_MESSAGE , "file name:'%s/%s' encoding not supported" , unzip_path.c_str() , file_stat.name );
        }
        {
            std::lock_guard<std::mutex> lock( progress->work->update_mutex );
            progress->work->prog_now = i + 1;
        }
        progress->caller->info_notify();
    }

    return true;
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

bool network_utilities_initial( void )
{
    return ( curl_global_init( CURL_GLOBAL_ALL ) == 0 );
}

bool set_proxy( Glib::ustring scheme , Glib::ustring hostname , std::uint32_t port )
{
    Glib::ustring proxy_desc;
    Glib::ustring diff_scheme = scheme.lowercase();

    if (
        diff_scheme != "http"    &&
        diff_scheme != "https"   &&
        diff_scheme != "socks4"  &&
        diff_scheme != "socks4a" &&
        diff_scheme != "socks5"  &&
        diff_scheme != "socks5h"
    )
    {
        return false;
    }

    proxy_desc += scheme;
    proxy_desc += "://";

    if ( hostname.empty() )
        return false;
    proxy_desc += hostname;

    if ( port >= 65535 )
        return false;
    
    proxy_desc += ":";
    proxy_desc += std::to_string( port );

    _proxy_desc = std::move( proxy_desc );
    return true;
}

std::shared_future<xmage_desc_t> get_last_version( XmageType type )
{
    std::function<xmage_desc_t()> get_version_func;
    if ( type == XmageType::Release )
        get_version_func = std::bind( get_last_release_version );
    else
        get_version_func = std::bind( get_last_beta_version );

    std::packaged_task<xmage_desc_t()> task( get_version_func );
    std::shared_future<xmage_desc_t> version_future = task.get_future();
    std::thread( std::move(task) ).detach();

    return version_future;
}

std::shared_future<bool> download_xmage( xmage_desc_t client_desc , progress_t * download_desc )
{
    std::packaged_task<bool()> task( std::bind( download_client_callback , client_desc , download_desc ) );
    std::shared_future<bool> download_future = task.get_future();
    std::thread( std::move(task) ).detach();

    return download_future;
}

Glib::ustring get_installation_package_name( xmage_desc_t client_desc )
{
    return client_desc.version_name + ".zip";
}

Glib::ustring get_download_temp_name( xmage_desc_t client_desc )
{
    return client_desc.version_name + ".dl";
}

std::shared_future<bool> install_xmage( Glib::ustring client_zip_name , Glib::ustring unzip_path , progress_t * progress ) noexcept( false )
{
    std::packaged_task<bool()> task( std::bind( install_xmage_callback , client_zip_name , unzip_path , progress ) );
    std::shared_future<bool> unzip_future = task.get_future();
    std::thread( std::move(task) ).detach();

    return unzip_future;
}

Glib::ustring xmagetype_to_string( const XmageType& type )
{
    Glib::ustring str;
    switch ( type )
    {
        default:
        case XmageType::Release:
            str = "Release";
            break;
        case XmageType::Beta:
            str = "Beta";
            break;
    }

    return str;
}

XmageType string_to_xmagetype( const Glib::ustring& str )
{
    XmageType type = XmageType::Release;
    Glib::ustring diff_str = str.lowercase();
    if ( diff_str == "release" )
    {
        type = XmageType::Release;
    }
    if ( diff_str == "beta" )
    {
        type = XmageType::Beta;
    }

    return type;
}

UpdateWork::UpdateWork():
    update_mutex(),
    prog_now(),
    prog_total(),
    prog_info()
{
    ;
}

void UpdateWork::do_update( XmageLauncher * caller )
{
    config_t& config = caller->get_config();
    XmageType type = config.get_active_xmage();
    progress_t progress = { caller , this };
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
        //progress_label->set_label( _( "no need to update" ) );
        {
            std::lock_guard<std::mutex> lock( this->update_mutex );
            this->prog_info = _( "no need to update" );
        }
        caller->info_notify();
        return ;
    }

    std::shared_future<bool> download_future;
    //if exists
    if ( std::filesystem::is_regular_file( get_installation_package_name( update_desc ).raw() ) == false )
    {
        download_future = download_xmage( update_desc , &progress );//progress );
        //progress_label->set_label( _(" download update" ) );
        {
            std::lock_guard<std::mutex> lock( this->update_mutex );
            this->prog_info = _( "download update" );
        }
        caller->info_notify();
        if ( download_future.get() )
        {
            //progress_label->set_label( _( "download success,install..." ) );
            {
                std::lock_guard<std::mutex> lock( this->update_mutex );
                this->prog_info = _( "download success,install..." );
            }
            caller->info_notify();
            std::filesystem::rename( get_download_temp_name( update_desc ).raw() , get_installation_package_name( update_desc ).raw() );
        }
        else
        {
            //progress_label->set_label( _( "download failure" ) );
            {
                std::lock_guard<std::mutex> lock( this->update_mutex );
                this->prog_info = _( "download failure" );
            }
            caller->info_notify();
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
    auto install_future = install_xmage( get_installation_package_name( update_desc ) ,  install_path , &progress );//progress );
    /* progress_label->set_label( _( "install update" ) ); */
    {
        std::lock_guard<std::mutex> lock( this->update_mutex );
        this->prog_info = _( "install update" );
    }
    caller->info_notify();
    if ( install_future.get() == false )
    {
        //progress_label->set_label( _( "install faliure" ) );
        {
            std::lock_guard<std::mutex> lock( this->update_mutex );
            this->prog_info = _( "install faliure" );
        }
        caller->info_notify();
        return ;
    }
    //progress_label->set_label( _( "install success" ) );
    {
        std::lock_guard<std::mutex> lock( this->update_mutex );
        this->prog_info = _( "install success" );
    }
    caller->info_notify();
    if ( type == XmageType::Release )
        config.set_release_version( update_desc.version_name );
    else
        config.set_beta_version( update_desc.version_name );
    std::filesystem::remove( get_installation_package_name( update_desc ).raw() );


    g_log( __func__ , G_LOG_LEVEL_MESSAGE , "update thread enter" );
    {
        std::lock_guard<std::mutex> lock( this->update_mutex );
        this->prog_now = 233;
        this->prog_total = 233333;
        this->prog_info = "test";
    }
    caller->info_notify();
}

void UpdateWork::get_data( std::int64_t& now , std::int64_t& total , Glib::ustring& info )
{
    std::lock_guard<std::mutex> lock( this->update_mutex );
    now = this->prog_now;
    total = this->prog_total;
    info = this->prog_info;
}
