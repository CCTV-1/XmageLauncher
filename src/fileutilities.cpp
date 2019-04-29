#include <cstdlib>
#include <cstring>

#include <filesystem>
#include <thread>
#include <fstream>
#include <memory>

//#include <giomm/file.h>
#include <jansson.h>
#include <zip.h>

#include "fileutilities.h"

static void unzip_client_callback( Glib::ustring client_zip_name , Glib::ustring unzip_path ) noexcept( false )
{
    std::filesystem::path client_zip( client_zip_name.raw() );
    std::filesystem::path unzip_dir( unzip_path.raw() );
    if ( std::filesystem::exists( client_zip ) == false )
    {
        return ;
    }

    if ( std::filesystem::exists( unzip_dir ) == false )
        std::filesystem::create_directories( unzip_dir );
    std::shared_ptr<zip_t> zip_ptr( zip_open( client_zip_name.c_str() , ZIP_RDONLY , nullptr ) , zip_close );
    zip_int64_t file_number = zip_get_num_entries( zip_ptr.get() , ZIP_FL_UNCHANGED );
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
    }
}

std::shared_future<void> unzip_client( Glib::ustring client_zip_name , Glib::ustring unzip_path ) noexcept( false )
{
    std::packaged_task<void()> task( std::bind( unzip_client_callback , client_zip_name , unzip_path ) );
    std::shared_future<void> unzip_future = task.get_future();
    std::thread( std::move(task) ).detach();

    return unzip_future;
}
