#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <functional>
#include <memory>
#include <set>
#include <thread>

#include <zip.h>
#include <json-glib/json-glib.h>
#include <giomm/file.h>
#include <glibmm/i18n.h>
#include <glibmm/stringutils.h>
#include <glibmm/fileutils.h>

#include "updatework.h"
#include "launcherconfig.h"

class ReceiveBuff
{
public:
    ReceiveBuff()
    {
        constexpr const size_t presize = 1024;
        this->buff = ( char * )malloc( sizeof( char )*presize );
        this->allocate_size = presize;
        this->used_size = 0;
    }
    ~ReceiveBuff()
    {
        free( this->buff );
    }
    ReceiveBuff( const ReceiveBuff& rhs )
    {
        this->buff = ( char * )malloc( sizeof( char )*(rhs.allocate_size) );
        if ( this->buff == nullptr )
        {
            throw std::bad_alloc();
        }
        memccpy( this->buff , rhs.buff ,  sizeof( char ) , rhs.allocate_size );
        this->allocate_size = rhs.allocate_size;
        this->used_size = rhs.used_size;
    }
    ReceiveBuff& operator=( const ReceiveBuff& rhs )
    {
        if ( this == &rhs )
        {
            return *this;
        }
        free( this->buff );
        this->buff = ( char * )malloc( sizeof( char )*(rhs.allocate_size) );
        if ( this->buff == nullptr )
        {
            throw std::bad_alloc();
        }
        memccpy( this->buff , rhs.buff , sizeof( char ) , rhs.allocate_size );
        this->allocate_size = rhs.allocate_size;
        this->used_size = rhs.used_size;
    }
    ReceiveBuff( ReceiveBuff&& rhs ) noexcept( true ):
        buff( std::move(rhs.buff) ),
        allocate_size( std::move(rhs.allocate_size) ),
        used_size( std::move(rhs.used_size) )
    {
        ;
    }
    ReceiveBuff& operator=( ReceiveBuff&& rhs ) noexcept( true )
    {
        if ( this == &rhs )
        {
            return *this;
        }
        free( this->buff );
        this->buff = std::move( rhs.buff );
        this->allocate_size = std::move( rhs.allocate_size );
        this->used_size = std::move( rhs.used_size );
    }

    const char * get_buff( void )
    {
        return this->buff;
    }
    std::size_t get_used( void )
    {
        return this->used_size;
    }

    void append_data( const char * data , std::size_t data_size )
    {
        std::size_t need_size = used_size + data_size;
        if ( need_size > this->allocate_size )
        {
            while ( need_size > this->allocate_size )
            {
                this->allocate_size *= 2;
            }
            char * new_buff = static_cast<char *>( realloc( this->buff , this->allocate_size ) );
            if ( new_buff == nullptr )
            {
                //keep old buff content
                throw std::bad_alloc();
            }
            this->buff = new_buff;
        }
        memccpy( this->buff + used_size , data , sizeof( char ) , data_size );
        this->used_size += data_size;
    }
private:
    char * buff;
    std::size_t allocate_size;
    std::size_t used_size;
};

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
    progress_t * progress = static_cast<progress_t *>( client_ptr );
    if ( progress == nullptr || progress->work == nullptr )
    {
        return false;
    }
    //notify progress update
    {
        std::lock_guard<std::mutex> lock( progress->work->update_mutex );
        progress->work->prog_now = dlnow;
        progress->work->prog_total = dltotal;
    }
    progress->dispatcher.emit();
    return 0;
}

static std::size_t get_json_callback( char * content , std::size_t size , std::size_t element_number , void * save_ptr )
{
    std::size_t realsize = size*element_number;
    ReceiveBuff * ptr = static_cast<ReceiveBuff *>( save_ptr );
    try
    {
        ptr->append_data( content , realsize );
    }
    catch(const std::bad_alloc& e)
    {
        return 0;
    }
    return realsize;
}

static std::shared_ptr<JsonParser> get_json( Glib::ustring url ) noexcept( false )
{
    std::string except_message( __func__ );

    std::int8_t re_try = 4;
    long default_timeout = 30L;
    ReceiveBuff json_buff;
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
        curl_easy_setopt( curl_handle.get() , CURLOPT_URL , url.c_str() );
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
                except_message += ":get '" + url + "' json failure,libcurl error message:";
                except_message += error_buff;
                throw std::runtime_error( except_message );
            }
            default_timeout += 10L;
            re_try--;
        }
    }
    while ( res != CURLE_OK );

    std::shared_ptr<JsonParser> root( json_parser_new() , g_object_unref );
    if ( !json_parser_load_from_data( root.get() , json_buff.get_buff() , json_buff.get_used() , nullptr ) )
    {
        except_message += ":network json:'";
        except_message += json_buff.get_buff();
        except_message += "' format does not meet expectations";
        throw std::invalid_argument( except_message );
    }

    return root;
}

static xmage_desc_t get_update_desc( const char * api_url , const char * url_jsonpath , const char * ver_jsonpath , std::function<void(xmage_desc_t&)> formatter = nullptr )
{
    std::shared_ptr<JsonParser> root = get_json( api_url );

    JsonNode * root_node = json_parser_get_root( root.get() );
    std::shared_ptr<JsonNode> urls_node( json_path_query( url_jsonpath , root_node , nullptr ), json_node_unref );
    if ( json_node_get_node_type( urls_node.get() ) != JsonNodeType::JSON_NODE_ARRAY )
    {
        throw std::runtime_error( std::string( "url json path" ) + url_jsonpath + "can't find" );
    }
    JsonArray * urls_arr = json_node_get_array( urls_node.get() );
    JsonNode * url_node = json_array_get_element( urls_arr , 0 );
    std::shared_ptr<JsonNode> vers_node( json_path_query( ver_jsonpath , root_node , nullptr ), json_node_unref );
    if ( json_node_get_node_type( vers_node.get() ) != JsonNodeType::JSON_NODE_ARRAY )
    {
        throw std::runtime_error( std::string( "version json path" ) + ver_jsonpath + "can't find" );
    }
    JsonArray * vers_arr = json_node_get_array( vers_node.get() );
    JsonNode * ver_node = json_array_get_element( vers_arr , 0 );

    xmage_desc_t raw_desc = { json_node_get_string( ver_node ) , json_node_get_string( url_node ) };
    if ( formatter != nullptr )
    {
        formatter( raw_desc );
    }
    return raw_desc;
}

static Glib::ustring get_installation_package_name( const xmage_desc_t& version_desc )
{
    return version_desc.version_name + ".zip";
}

static Glib::ustring get_download_temp_name( const xmage_desc_t& version_desc )
{
    return version_desc.version_name + ".dl";
}

static bool download_update_callback( xmage_desc_t version_desc , progress_t * download_desc )
{
    std::shared_ptr<CURL> curl_handle( curl_easy_init() , curl_easy_cleanup );

    //output temp file,download success,rename to version.zip,to support check local zip continue download.
    Glib::ustring temp_name = get_download_temp_name( version_desc );
    std::shared_ptr<FILE> download_file( fopen( temp_name.c_str() , "wb+" ) , fclose );
    if ( download_file.get() == nullptr )
    {
        g_log( __func__ , G_LOG_LEVEL_WARNING , "open file:\'%s\' failure." , temp_name.c_str() );
        return false;
    }

    char error_buff[CURL_ERROR_SIZE];

    common_curl_opt_set( curl_handle );
    curl_easy_setopt( curl_handle.get() , CURLOPT_URL , version_desc.download_url.c_str() );
    curl_easy_setopt( curl_handle.get() , CURLOPT_NOPROGRESS , 0L );
    curl_easy_setopt( curl_handle.get() , CURLOPT_XFERINFOFUNCTION , download_description_callback );
    curl_easy_setopt( curl_handle.get() , CURLOPT_XFERINFODATA , download_desc );
    curl_easy_setopt( curl_handle.get() , CURLOPT_WRITEFUNCTION , fwrite );
    curl_easy_setopt( curl_handle.get() , CURLOPT_ERRORBUFFER , error_buff );
    curl_easy_setopt( curl_handle.get() , CURLOPT_WRITEDATA , download_file.get() );

    std::shared_ptr<CURLM> curlmulti_handle(
        [ curl_handle ]() -> CURLM *
        {
            CURLM * handle = curl_multi_init();
            curl_multi_add_handle( handle , curl_handle.get() );
            return handle;
        }(),
        [ curl_handle ]( CURLM * handle ) -> void
        {
            curl_multi_remove_handle( handle , curl_handle.get() );
            curl_multi_cleanup( handle );
        }
    );

    int running_handles;
    int repeats = 0;
    CURLMcode status_code = curl_multi_perform( curlmulti_handle.get() , &running_handles );
    if ( ( status_code != CURLM_OK ) && ( status_code != CURLM_CALL_MULTI_PERFORM ) )
    {
        g_log( __func__ , G_LOG_LEVEL_MESSAGE , "download url:\'%s\',error type:\'%s\',error message:\'%s\'" , 
                version_desc.download_url.c_str() , curl_multi_strerror( status_code ) , error_buff );
        return false;
    }
    
    while( running_handles )
    {
        int numfds;
        status_code = curl_multi_wait( curlmulti_handle.get() , nullptr , 0 , 100 , &numfds );
        if ( status_code != CURLM_OK )
        {
            g_log( __func__ , G_LOG_LEVEL_MESSAGE , "download url:\'%s\',error type:\'%s\',error message:\'%s\'" , 
                    version_desc.download_url.c_str() , curl_multi_strerror( status_code ) , error_buff );
            return false;
        }

        bool do_return;
        {
            std::lock_guard<std::mutex> lock( download_desc->work->update_mutex );
            do_return = !( download_desc->work->updating );
        }
        if ( do_return )
        {
            //stop update,update failure.
            return false;
        }

        if ( numfds == 0 )
        {
            repeats++;
            if ( repeats > 1 )
                g_usleep( 100 );
            //todo:add setting to launcher
            if ( repeats > 5*600 ) //600*100ms = 1min
            {
                g_log( __func__ , G_LOG_LEVEL_MESSAGE , "too long time not receiving data,disconnect,download failure." );
                return false;
            }
        }
        else
        {
            repeats = 0;
        }

        status_code = curl_multi_perform( curlmulti_handle.get() , &running_handles );
        if ( ( status_code != CURLM_OK ) && ( status_code != CURLM_CALL_MULTI_PERFORM ) )
        {
            g_log( __func__ , G_LOG_LEVEL_MESSAGE , "download url:\'%s\',error type:\'%s\',error message:\'%s\'" , 
                    version_desc.download_url.c_str() , curl_multi_strerror( status_code ) , error_buff );
            return false;
        }
    }

    CURLMsg * message = nullptr;
    do
    {
        int msgs_queue = 0;
        message = curl_multi_info_read( curlmulti_handle.get() , &msgs_queue );
        if ( ( message != nullptr ) && ( message->msg == CURLMSG_DONE ) )
        {
            if ( message->data.result != CURLE_OK )
            {
                g_log( __func__ , G_LOG_LEVEL_MESSAGE , "download url:\'%s\',error content:\'%s\',error code:\'%d\'" , 
                version_desc.download_url.c_str() , curl_easy_strerror( message->data.result ) , message->data.result );
                return false;
            }
        }
    }while( message != nullptr );

    return true;
}

static bool install_update_callback( Glib::ustring install_packge_name , Glib::ustring install_dir_path , progress_t * progress ) noexcept( false )
{
    if ( progress == nullptr || progress->work == nullptr )
    {
        return false;
    }

    auto install_packge = Gio::File::create_for_path( install_packge_name );
    auto install_dir = Gio::File::create_for_path( install_dir_path );
    if ( install_packge->query_exists() == false )
    {
        return false;
    }
    if ( install_dir->query_exists() == false )
    {
        install_dir->make_directory_with_parents();
    }

    std::shared_ptr<zip_t> zip_ptr( zip_open( install_packge_name.c_str() , ZIP_RDONLY , nullptr ) , zip_close );
    zip_int64_t file_number = zip_get_num_entries( zip_ptr.get() , ZIP_FL_UNCHANGED );
    {
        std::lock_guard<std::mutex> lock( progress->work->update_mutex );
        progress->work->prog_total = file_number;
    }
    progress->dispatcher.emit();

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

        Glib::ustring unzip_path( file_stat.name );
        auto target_path = Gio::File::create_for_path( install_dir_path + "/" + unzip_path );
        if ( unzip_path[unzip_path.size()-1] == '/' )
        {
            //avoid file_set_contents() write root/dir/ to root/dir
            //if write to root/dir,block write root/dir/file exception handler mkdir(root/dir/) failure.
            Gio::FileType type = target_path->query_file_type();
            if ( type == Gio::FileType::UNKNOWN )
            {
                target_path->make_directory_with_parents();
            }
            //if exists root/dir remove it.
            else if ( type != Gio::FileType::DIRECTORY )
            {
                target_path->remove();
                target_path->make_directory_with_parents();
            }

            //update progress
            {
                std::lock_guard<std::mutex> lock( progress->work->update_mutex );
                progress->work->prog_now = i + 1;
            }
            progress->dispatcher.emit();
            continue;
        }

        try
        {
            Glib::file_set_contents( target_path->get_path() , data_buff.get() , file_stat.size );
        }
        catch ( const Glib::FileError& e )
        {
            auto code = e.code();
            if ( code == Glib::FileError::Code::ACCESS_DENIED )
            {
                if ( target_path->query_file_type() != Gio::FileType::DIRECTORY )
                    g_log( __func__ , G_LOG_LEVEL_MESSAGE , "access file:'%s failure',Glib::FileError code:%d" , target_path->get_path().c_str() , code );
            }
            else if ( code == Glib::FileError::Code::NO_SUCH_ENTITY )
            {
                auto parent_dir = target_path->get_parent();

                //zip_get_num_entries ->exists 
                Gio::FileType type = parent_dir->query_file_type();
                if ( type == Gio::FileType::UNKNOWN )
                {
                    parent_dir->make_directory_with_parents();
                }

                try
                {
                    Glib::file_set_contents( target_path->get_path() , data_buff.get() , file_stat.size );
                }
                catch ( const Glib::FileError& e )
                {
                    g_log( __func__ , G_LOG_LEVEL_MESSAGE , "retry unzip file:'%s' failure,Glib::FileError code:%d" , target_path->get_path().c_str() , code );
                }
            }
            else
            {
                g_log( __func__ , G_LOG_LEVEL_MESSAGE , "unzip file:'%s failure',Glib::FileError code:%d" , target_path->get_path().c_str() , code );
            }
        }

        {
            std::lock_guard<std::mutex> lock( progress->work->update_mutex );
            progress->work->prog_now = i + 1;
        }
        progress->dispatcher.emit();
    }

    return true;
}

bool set_proxy( Glib::ustring scheme , Glib::ustring hostname , std::uint32_t port )
{
    Glib::ustring proxy_desc;
    Glib::ustring diff_scheme = scheme.lowercase();

    const std::set<Glib::ustring> scheme_map({
        "http"   ,
        "https"  ,
        "socks4" ,
        "socks4a",
        "socks5" ,
        "socks5h",
    });

    if ( scheme_map.find( diff_scheme ) == scheme_map.end() )
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

static std::shared_future<xmage_desc_t> get_last_version( XmageType type )
{
    config_t& config = config_t::get_config();
    bool using_mirror = config.get_using_mirror();
    std::function<xmage_desc_t()> get_version_func;
    if ( type == XmageType::Release )
    {
        if ( using_mirror )
        {
            //{
            //  "java" : {
            //      "version": "1.8.0_201",
            //      "old_location": "http://download.oracle.com/otn-pub/java/jdk/8u201-b09/42970487e3af4f5aa5bca3f542482c60/jre-8u201-",
            //      "location": "http://xmage.today/java/jre-8u201-"
            //  },
            //  "XMage" : {
            //      "version": "1.4.35V1 (2019-04-24)",
            //      "location": "https://github.com/magefree/mage/releases/download/xmage_1.4.35V1/xmage_1.4.35V1.zip",
            //      "locations": ["http://xmage.de/files/xmage_1.4.35V1.zip"],
            //      "torrent": "",
            //      "images": "",
            //      "Launcher" : {
            //      "version": "0.3.8",
            //      "location": "http://bit.ly/xmageLauncher038"
            //      } 
            //  }
            //}
            get_version_func = std::bind( get_update_desc , "http://xmage.de/xmage/config.json" , "$.XMage.locations[0]" , "$.XMage.version",
                []( xmage_desc_t& raw )
                {
                    //"1.4.35V1 (2019-04-24)",
                    Glib::ustring version_time = raw.version_name;
                    std::size_t space_index = 0;
                    for ( std::size_t i = 0 ; i < version_time.size() ; i++ )
                    {
                        if ( version_time[i] == ' ' )
                        {
                            space_index = i;
                            break;
                        }
                    }
                    //1.4.35V1
                    raw.version_name = version_time.substr( 0 , space_index );
                }
            );
        }
        else
        {
            //{
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
            get_version_func = std::bind( get_update_desc , "https://api.github.com/repos/magefree/mage/releases/latest" , "$.assets[0].browser_download_url" , "$.assets[0].name",
                []( xmage_desc_t& raw )
                {
                    if ( Glib::str_has_prefix( raw.version_name , "xmage_" ) )
                    {
                        //"xmage_" exists NUL sizeof 6
                        raw.version_name = raw.version_name.substr( sizeof( "xmage_" ) - 1 , raw.version_name.size() );
                    }
                    if ( Glib::str_has_suffix( raw.version_name , ".zip" ) )
                    {
                        //".zip" exists NUL sizeof 5
                        raw.version_name = raw.version_name.substr( 0 , raw.version_name.size() - ( sizeof( ".zip" ) - 1 ) );
                    }
                }
            );
        }
    }
    else
    {
        //{
        //    "java": {
        //        "version": "1.8.0_201",
        //        "location": "http://xmage.today/java/jre-8u201-"
        //    },
        //    "XMage": {
        //        "version": "1.4.48-dev (2021-05-05 14-05)",
        //        "location": "http://beta.xmage.today/files/mage-update_1.4.48-dev_2021-05-05_14-05.zip",
        //        "locations": [],
        //        "full": "http://beta.xmage.today/files/mage-full_1.4.48-dev_2021-05-05_14-05.zip",
        //        "torrent": "",
        //        "images": "",
        //        "Launcher": {
        //            "version": "0.3.8",
        //            "location": "http://bit.ly/xmageLauncher038"
        //        }
        //    }
        //}
        get_version_func = std::bind( get_update_desc , "http://xmage.today/config.json" , "$.XMage.full" , "$.XMage.version" , nullptr );
    }

    std::packaged_task<xmage_desc_t()> task( get_version_func );
    std::shared_future<xmage_desc_t> version_future = task.get_future();
    std::thread( std::move(task) ).detach();

    return version_future;
}

static std::shared_future<bool> download_update( xmage_desc_t version_desc , progress_t * download_desc )
{
    std::packaged_task<bool()> task( std::bind( download_update_callback , version_desc , download_desc ) );
    std::shared_future<bool> download_future = task.get_future();
    std::thread( std::move(task) ).detach();

    return download_future;
}

static std::shared_future<bool> install_update( Glib::ustring install_packge_name , Glib::ustring install_dir_path , progress_t * progress )
{
    std::packaged_task<bool()> task( std::bind( install_update_callback , install_packge_name , install_dir_path , progress ) );
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
            str = _( "Release" );
            break;
        case XmageType::Beta:
            str = _( "Beta" );
            break;
    }

    return str;
}

UpdateWork::UpdateWork():
    update_mutex(),
    updating( false ),
    prog_now(),
    prog_total(),
    prog_info()
{
    ;
}

void UpdateWork::do_update( Glib::Dispatcher& dispatcher )
{
    //start update,disable launch button
    {
        std::lock_guard<std::mutex> lock( this->update_mutex );
        this->prog_info = _( "check for updates" );
    }

    //return ; call deconstruct,don't need call dispatcher.emit();
    std::shared_ptr<void> lock_launch(
        [ this , &dispatcher ]()
        {
            {
                std::lock_guard<std::mutex> lock( this->update_mutex );
                this->updating = true;
            }
            dispatcher.emit();
            return nullptr;
        }(),
        [ this , &dispatcher ]( void * )
        {
            {
                std::lock_guard<std::mutex> lock( this->update_mutex );
                this->prog_total = 0;
                this->updating = false;
            }
            dispatcher.emit();
        }
    );

    config_t& config = config_t::get_config();
    XmageType type = config.get_active_xmage();
    progress_t progress = { dispatcher , this };

    auto update_future = get_last_version( type );
    xmage_desc_t update_desc;

    while ( update_future.wait_for( std::chrono::microseconds( 200 ) ) != std::future_status::ready )
    {
        bool do_return = false;
        {
            std::lock_guard<std::mutex> lock( this->update_mutex );
            do_return = !( this->updating );
        }
        if ( do_return )
        {
            return ;
        }
    }

    try
    {
        update_desc = update_future.get();
    }
    catch ( const std::exception& e )
    {
        //get update information failure
        g_log( __func__ , G_LOG_LEVEL_MESSAGE , "%s" , e.what() );
        //don't need stop request,check update failure,to return end update.
        {
            std::lock_guard<std::mutex> lock( this->update_mutex );
            this->prog_info = _( "check update failure" );
        }
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
    if ( update_desc.version_name.compare( version ) > 0 )
    {
        g_log( __func__ , G_LOG_LEVEL_MESSAGE , "%s" , _( "exist new xmage,now download." ) );
    }
    else
    {
        {
            std::lock_guard<std::mutex> lock( this->update_mutex );
            this->prog_info = _( "no need to update" );
        }
        return ;
    }

    std::shared_future<bool> download_future;
    //if exists
    auto install_package = Gio::File::create_for_path( get_installation_package_name( update_desc ) );
    if ( install_package->query_exists() == false )
    {
        download_future = download_update( update_desc , &progress );
        {
            std::lock_guard<std::mutex> lock( this->update_mutex );
            this->prog_info = _( "download update" );
        }
        dispatcher.emit();
        while ( download_future.wait_for( std::chrono::microseconds( 200 ) ) != std::future_status::ready )
        {
            bool do_return = false;
            {
                std::lock_guard<std::mutex> lock( this->update_mutex );
                do_return = !( this->updating );
            }
            if ( do_return )
            {
                //wait download thread exit
                download_future.wait();
                return ;
            }
        }

        if ( download_future.get() )
        {
            {
                std::lock_guard<std::mutex> lock( this->update_mutex );
                this->prog_now = 0;
                this->prog_total = 0;
                this->prog_info = _( "download success" );
            }
            dispatcher.emit();
            auto download_temp_file = Gio::File::create_for_path( get_download_temp_name( update_desc ) );
            download_temp_file->move( install_package );
        }
        else
        {
            {
                std::lock_guard<std::mutex> lock( this->update_mutex );
                this->prog_now = 0;
                this->prog_total = 0;
                this->prog_info = _( "download failure" );
            }
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
    {
        std::lock_guard<std::mutex> lock( this->update_mutex );
        this->prog_info = _( "install update" );
    }
    auto install_future = install_update( get_installation_package_name( update_desc ) ,  install_path , &progress );
    dispatcher.emit();
    while ( install_future.wait_for( std::chrono::microseconds( 200 ) ) != std::future_status::ready )
    {
        bool do_return = false;
        {
            std::lock_guard<std::mutex> lock( this->update_mutex );
            do_return = !( this->updating );
        }
        if ( do_return )
        {
            return ;
        }
    }
    if ( install_future.get() == false )
    {
        {
            std::lock_guard<std::mutex> lock( this->update_mutex );
            this->prog_now = 0;
            this->prog_total = 0;
            this->prog_info = _( "install failure" );
        }
        return ;
    }

    {
        std::lock_guard<std::mutex> lock( this->update_mutex );
        this->prog_now = 0;
        this->prog_total = 0;
        this->prog_info = _( "install success" );
    }
    dispatcher.emit();

    if ( type == XmageType::Release )
    {
        config.set_release_version( update_desc.version_name );
    }
    else
    {
        config.set_beta_version( update_desc.version_name );
    }
    install_package->remove();
}

void UpdateWork::stop_update( void )
{
    std::lock_guard<std::mutex> lock( this->update_mutex );
    this->updating = false;
}

void UpdateWork::get_data( bool& update_end , std::int64_t& now , std::int64_t& total , Glib::ustring& info )
{
    std::lock_guard<std::mutex> lock( this->update_mutex );
    update_end = !this->updating;
    now = this->prog_now;
    total = this->prog_total;
    info = this->prog_info;
}
