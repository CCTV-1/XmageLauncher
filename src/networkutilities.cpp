#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <functional>
#include <memory>

#include <jansson.h>

#include "networkutilities.h"

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
    
    download_desc_t * desc = static_cast<download_desc_t *>( client_ptr );
    desc->total = dltotal;
    desc->now = dlnow;
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
    //            "name": "mage_1.4.35V0"
    //            ... unimportant ...
    //            "browser_download_url": "https://github.com/magefree/mage/releases/download/xmage_1.4.35V0/xmage_1.4.35V0.zip"
    //        }
    //        {
    //            "name": "mage_1.4.35V0a"
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

    free( json_buff.buff );
    return { json_string_value( version_name ) , json_string_value( download_url ) };
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

static bool download_client_callback( xmage_desc_t client_desc , download_desc_t * download_desc )
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

std::shared_future<bool> download_xmage( xmage_desc_t client_desc , download_desc_t * download_desc )
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
