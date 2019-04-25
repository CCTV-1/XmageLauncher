#include <cstdlib>
#include <cstring>

#include <memory>

#include <curl/curl.h>
#include <jansson.h>

#include "networkutilities.h"

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

static client_desc_t get_last_version_callback( XmageType type ) noexcept( false )
{
    std::string except_message( __func__ );
    constexpr char user_agent[] = "XmageLauncher";
    Glib::ustring api_url;
    switch( type )
    {
        case XmageType::Beta:
        {
            break;
        }
        default:
        case XmageType::Release:
        {
            api_url = "https://api.github.com/repos/magefree/mage/releases/latest";
        }
    }

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
        curl_easy_setopt( curl_handle.get() , CURLOPT_URL , api_url.c_str() );
        curl_easy_setopt( curl_handle.get() , CURLOPT_USERAGENT , user_agent );
        curl_easy_setopt( curl_handle.get() , CURLOPT_VERBOSE , 0L );
        curl_easy_setopt( curl_handle.get() , CURLOPT_FOLLOWLOCATION , 1L );
        curl_easy_setopt( curl_handle.get() , CURLOPT_USE_SSL , 1L );
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
                except_message += ":get last version failure,libcurl error message:";
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
        except_message += "network json:'";
        except_message += json_buff.buff;
        except_message += "' format does not meet expectations";
        throw std::invalid_argument( except_message );
    }
    /* {
        ... unimportant ...
        "name": "xmage_1.4.35V0",
        ... unimportant ...
        "assets": [
            {
                ... unimportant ...
                "size": 148844576,
                ...  unimportant ...
                "created_at": "2019-04-23T21:30:45Z",
                "updated_at": "2019-04-23T21:34:32Z",
                "browser_download_url": "https://github.com/magefree/mage/releases/download/xmage_1.4.35V0/xmage_1.4.35V0.zip"
            }
        ],
        ... unimportant ...
    } */
    json_t * version_name = json_object_get( root.get() , "name" );
    if ( json_is_string( version_name )  == false )
    {
        except_message += "network json:'";
        except_message += json_buff.buff;
        except_message += "' node 'name' format does not meet expectations";
        throw std::invalid_argument( except_message );
    }

    json_t * assets_array = json_object_get( root.get() , "assets" );
    if ( json_is_array( assets_array )  == false )
    {
        except_message += "network json:'";
        except_message += json_buff.buff;
        except_message += "' node 'assets' format does not meet expectations";
        throw std::invalid_argument( except_message );
    }

    json_t * last_assets = json_array_get( assets_array , 0 );
    if ( json_is_object( last_assets ) == false )
    {
        except_message += "network json:'";
        std::shared_ptr<char> jsons( json_dumps( last_assets , JSON_INDENT( 4 ) ) , free );
        except_message += jsons.get();
        except_message += "' element 0 format does not meet expectations";
        throw std::invalid_argument( except_message );
    }

    json_t * created_time = json_object_get( last_assets , "created_at" );
    if ( json_is_string( created_time )  == false )
    {
        except_message += "network json:'";
        std::shared_ptr<char> jsons( json_dumps( last_assets , JSON_INDENT( 4 ) ) , free );
        except_message += jsons.get();
        except_message += "' node 'created_at' format does not meet expectations";
        throw std::invalid_argument( except_message );
    }

    json_t * client_size = json_object_get( last_assets , "size" );
    if ( json_is_integer( client_size )  == false )
    {
        except_message += "network json:'";
        std::shared_ptr<char> jsons( json_dumps( last_assets , JSON_INDENT( 4 ) ) , free );
        except_message += jsons.get();
        except_message += "' node 'size' format does not meet expectations";
        throw std::invalid_argument( except_message );
    }

    json_t * download_url = json_object_get( last_assets , "browser_download_url" );
    if ( json_is_string( download_url )  == false )
    {
        except_message += "network json:'";
        std::shared_ptr<char> jsons( json_dumps( last_assets , JSON_INDENT( 4 ) ) , free );
        except_message += jsons.get();
        except_message += "' format does not meet expectations";
        throw std::invalid_argument( except_message );
    }

    Glib::TimeVal time_val;
    time_val.assign_from_iso8601( json_string_value( created_time ) );
    client_desc_t last_version({
        json_string_value( version_name ) , time_val,
        static_cast<std::size_t>( json_integer_value( client_size ) ) , json_string_value( download_url )
    });

    free( json_buff.buff );
    return last_version;
}

static void download_client_callback( client_desc_t desc )
{
    ;
}

std::shared_future<client_desc_t> get_last_version( XmageType type )
{
    std::packaged_task<client_desc_t()> task( std::bind( get_last_version_callback , type ) );
    std::shared_future<client_desc_t> version_future = task.get_future();
    std::thread( std::move(task) ).detach();

    return version_future;
}

[[deprecated("unimplement")]]
std::shared_future<void> download_client( client_desc_t desc )
{
    std::packaged_task<void()> task( std::bind( download_client_callback , desc ) );
    std::shared_future<void> download_future = task.get_future();
    std::thread( std::move(task) ).detach();

    return download_future;
}