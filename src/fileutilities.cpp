#include <cstdlib>
#include <cstring>

#include <filesystem>
#include <thread>
#include <fstream>
#include <memory>

#include <jansson.h>
#include <zip.h>

#include "fileutilities.h"

static void unzip_client_callback( Glib::ustring client_zip_name , Glib::ustring unzip_path )
{
    std::filesystem::path client_zip( client_zip_name.raw() );
    std::filesystem::path unzip_dir( unzip_path.raw() );
    if ( std::filesystem::exists( client_zip ) == false )
        return ;

    if ( std::filesystem::exists( unzip_dir ) == false )
        std::filesystem::create_directories( unzip_dir );
    std::shared_ptr<zip_t> zip_ptr( zip_open( client_zip_name.c_str() , ZIP_RDONLY , nullptr ) , zip_close );
    zip_int64_t file_number = zip_get_num_entries( zip_ptr.get() , ZIP_FL_UNCHANGED );
    char * data_buff = nullptr;
    zip_uint64_t buff_size = 0;
    for( zip_int64_t i = 0 ; i < file_number ; i++ )
    {
        struct zip_stat file_stat;
        zip_stat_init( &file_stat );
        zip_stat_index( zip_ptr.get() , i , ZIP_FL_ENC_GUESS , &file_stat );
        fprintf( stdout , "unzip file :'%s' to:'%s/%s'\n" , file_stat.name , unzip_path.c_str() , file_stat.name );
        if ( file_stat.size > buff_size )
        {
            buff_size = file_stat.size;
            //discard old data
            free( data_buff );
            data_buff = static_cast<char *>( malloc( file_stat.size ) );
        }
        std::shared_ptr<zip_file_t> file_ptr( zip_fopen_index( zip_ptr.get() , i , ZIP_FL_COMPRESSED ) , zip_fclose );
        zip_fread( file_ptr.get() , data_buff , file_stat.size );
        std::ofstream unzip_file( unzip_path + std::string( "/" ) + file_stat.name , std::ios::binary );
        unzip_file.write( data_buff , buff_size );
    }
}

std::shared_future<void> unzip_client( Glib::ustring client_zip_name , Glib::ustring unzip_path )
{
    std::packaged_task<void()> task( std::bind( unzip_client_callback , client_zip_name , unzip_path ) );
    std::shared_future<void> unzip_future = task.get_future();
    std::thread( std::move(task) ).detach();

    return unzip_future;
}
