#include <string>
#include <filesystem>

#include <unistd.h>

#include <curl/curl.h>
#include <gtkmm.h>
#include <glibmm/i18n.h>

#include "launcher.h"

class LauncherProgressBar : public Gtk::DrawingArea
{
public:
    LauncherProgressBar( BaseObjectType* cobject , const Glib::RefPtr<Gtk::Builder>& ):
        Gtk::DrawingArea( cobject ),
        prog_value( 0.0 ),
        prog_info()
    {
        this->layout = this->create_pango_layout( "0%" );
        this->set_size_request( 30 );
    }
    ~LauncherProgressBar()
    {
    }

    void puls_prog( double value )
    {
        if ( value <= 0 )
            return ;
        if ( value + this->prog_value <= 1.0 )
        {
            this->prog_value += value;
        }
        else
        {
            this->prog_value = 1.0;
        }
        
        this->queue_draw();
    }

    void set_progress_value( double value )
    {
        if ( value < 0.0 )
        {
            this->prog_value = 0.0;
            return ;
        }
        if ( value >= 1.0 )
        {
            this->prog_value = 1.0;
            return ;
        }

        this->prog_value = value;
    }

    const double& get_progress_value( void ) const
    {
        return std::ref( this->prog_value );
    }

    void set_progress_info( Glib::ustring info )
    {
        this->prog_info = std::move( info );
    }

    const Glib::ustring& get_progress_info( void ) const
    {
        return std::ref( this->prog_info );
    }

protected:
    void get_preferred_height_vfunc( int& minimum_width , int& natural_width ) const override
    {
        minimum_width = 30;
        natural_width = 35;
    }

    bool on_draw( const Cairo::RefPtr<Cairo::Context>& cairo_context ) override
    {
        const Gtk::Allocation allocation = get_allocation();
        constexpr double line_width = 2;
        cairo_context->set_line_width( line_width );

        //background color
        cairo_context->set_source_rgb( 204/255.0 , 204/255.0 , 204/255.0 );
        cairo_context->rectangle( 0 , 0 , allocation.get_width() , allocation.get_height() );
        cairo_context->fill_preserve();

        //bar box color
        cairo_context->set_source_rgb( 0/255.0 , 0/255.0 , 0/255.0 );
        cairo_context->rectangle( line_width/2 , line_width/2 , allocation.get_width() - line_width , allocation.get_height() - line_width );
        cairo_context->stroke();

        //progess color
        cairo_context->set_source_rgb( 25/255.0 , 189/255.0 , 155/255.0 );
        cairo_context->rectangle( line_width , line_width , ( allocation.get_width() - 2*line_width )*prog_value , allocation.get_height() - 2*line_width );
        cairo_context->fill_preserve();

        //porgress info
        int layout_width , layout_height;
        cairo_context->set_source_rgb( 0/255.0 , 0/255.0 , 0/255.0 );
        this->layout->set_text( this->prog_info );
        this->layout->get_pixel_size( layout_width , layout_height );
        cairo_context->move_to( ( allocation.get_width() - layout_width )/2 , ( allocation.get_height() - layout_height )/2 );
        this->layout->show_in_cairo_context( cairo_context );
        return true;
    }

private:
    double prog_value;

    Glib::ustring prog_info;
    Glib::RefPtr<Pango::Layout> layout;
};

static void fill_setting_value( Glib::RefPtr<Gtk::Builder>& builder , config_t& config )
{
    Gtk::ComboBox * proxy_type;
    builder->get_widget( "ProxyType" , proxy_type );
    if ( config.get_using_proxy() )
    {
        proxy_type->set_active_id( config.get_proxy_scheme() );
    }
    else
    {
        proxy_type->set_active_id( "None" );
    }

    Gtk::Entry * proxy_host;
    builder->get_widget( "ProxyHost" , proxy_host );
    proxy_host->set_text( config.get_proxy_host() );

    Gtk::SpinButton * proxy_port;
    builder->get_widget( "ProxyPort" , proxy_port );
    proxy_port->set_value( config.get_proxy_port() );

    Gtk::SpinButton * xms_opt;
    builder->get_widget( "XmsOpt" , xms_opt );
    xms_opt->set_value( config.get_jvm_xms() );

    Gtk::SpinButton * xmx_opt;
    builder->get_widget( "XmxOpt" , xmx_opt );
    xmx_opt->set_value( config.get_jvm_xmx() );

    Gtk::FileChooserButton * release_path;
    builder->get_widget( "ReleaseMagePath" , release_path );
    release_path->set_filename( config.get_release_path() );

    Gtk::FileChooserButton * beta_path;
    builder->get_widget( "BetaMagePath" , beta_path );
    beta_path->set_filename( config.get_beta_path() );
}

XmageLauncher::XmageLauncher( BaseObjectType* cobject , const Glib::RefPtr<Gtk::Builder>& builder ):
    Gtk::Window( cobject ),
    launcher_builder( builder ),
    config( config_t::get_config() ),
    update_process(),
    update_dispatcher()
{
    fill_setting_value( this->launcher_builder , this->config );
    if ( this->config.get_using_proxy() )
    {
        set_proxy( config.get_proxy_scheme() , config.get_proxy_host() , config.get_proxy_port() );
    }

    //main window button
    Gtk::Button * client_button;
    builder->get_widget( "LauncherClient" , client_button );
    client_button->signal_clicked().connect( sigc::mem_fun( *this , &XmageLauncher::launch_client ) );
    Gtk::Button * server_button;
    builder->get_widget( "LauncherServer" , server_button );
    server_button->signal_clicked().connect( sigc::mem_fun( *this , &XmageLauncher::launch_server ) );
    Gtk::Button * xmage_button;
    builder->get_widget( "LauncherXmage" , xmage_button );
    xmage_button->signal_clicked().connect(
        [ this ]()
        {
            this->launch_client();
            this->launch_server();
        }
    );

    //advanced setting dialog
    builder->get_widget( "SettingDialog" , this->setting_dialog );
    this->setting_dialog->add_button( _( "close menu" ) , 0 );
    this->setting_dialog->signal_response().connect( sigc::mem_fun( *this , &XmageLauncher::close_setting ) );
    Gtk::Button * setting_button;
    builder->get_widget( "SettingButton" , setting_button );
    setting_button->signal_clicked().connect( sigc::mem_fun( *this , &XmageLauncher::show_setting ) );
    Gtk::CheckButton * using_mirror_button;
    builder->get_widget( "UsingMirror" , using_mirror_button );
    using_mirror_button->set_active( this->config.get_using_mirror() );
    using_mirror_button->signal_toggled().connect(
        [ this , using_mirror_button ]()
        {
            this->config.set_using_mirror( using_mirror_button->get_active() );
        }
    );
    Gtk::Button * reset_button;
    builder->get_widget( "ResetConfig" , reset_button );
    reset_button->signal_clicked().connect(
        [ this ]()
        {
            Gtk::MessageDialog confirm( Glib::ustring(_( "do you want reset config?" )) , false , Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
            confirm.set_position( Gtk::WindowPosition::WIN_POS_CENTER_ALWAYS );
            if ( confirm.run() != Gtk::RESPONSE_YES )
                return ;
            this->config.reset_config();
            fill_setting_value( this->launcher_builder , this->config );
        }
    );

    //main window setting
    Gtk::ComboBox * proxy_type;
    builder->get_widget( "ProxyType" , proxy_type );
    proxy_type->signal_changed().connect(
        [ this , proxy_type ]()
        {
            Glib::ustring proxy_string = proxy_type->get_active_id();
            Glib::ustring diff_string = proxy_string.lowercase();
            if ( diff_string.compare( "none" ) == 0 )
            {
                this->config.set_using_proxy( false );
            }
            else
            {
                this->config.set_using_proxy( true );
                this->config.set_proxy_scheme( proxy_string );
            }
            if ( this->config.get_using_proxy() )
            {
                set_proxy( config.get_proxy_scheme() , config.get_proxy_host() , config.get_proxy_port() );
            }
        }
    );
    Gtk::Entry * proxy_host;
    builder->get_widget( "ProxyHost" , proxy_host );
    proxy_host->signal_changed().connect(
        [ this , proxy_host ]()
        {
            Glib::ustring host_string = proxy_host->get_text();
            this->config.set_proxy_host( host_string );
            if ( this->config.get_using_proxy() )
            {
                set_proxy( config.get_proxy_scheme() , config.get_proxy_host() , config.get_proxy_port() );
            }
        }
    );
    Gtk::SpinButton * proxy_port;
    builder->get_widget( "ProxyPort" , proxy_port );
    proxy_port->signal_value_changed().connect(
        [ this , proxy_port ]()
        {
            double port_value = proxy_port->get_value();
            this->config.set_proxy_port( port_value );
            if ( this->config.get_using_proxy() )
            {
                set_proxy( config.get_proxy_scheme() , config.get_proxy_host() , config.get_proxy_port() );
            }
        }
    );
    Gtk::SpinButton * xms_opt;
    builder->get_widget( "XmsOpt" , xms_opt );
    xms_opt->signal_value_changed().connect(
        [ this , xms_opt ]()
        {
            double xms_value = xms_opt->get_value();
            this->config.set_jvm_xms( xms_value );
        }
    );
    Gtk::SpinButton * xmx_opt;
    builder->get_widget( "XmxOpt" , xmx_opt );
    xmx_opt->signal_value_changed().connect(
        [ this , xmx_opt ]()
        {
            double xmx_value = xmx_opt->get_value();
            this->config.set_jvm_xmx( xmx_value );
        }
    );
    Gtk::FileChooserButton * release_path;
    builder->get_widget( "ReleaseMagePath" , release_path );
    release_path->signal_selection_changed().connect(
        [ this , release_path ]()
        {
            Glib::ustring new_release_path = release_path->get_filename();
            this->config.set_release_path( new_release_path );
        }
    );
    Gtk::FileChooserButton * beta_path;
    builder->get_widget( "BetaMagePath" , beta_path );
    beta_path->signal_selection_changed().connect(
        [ this , beta_path ]()
        {
            Glib::ustring new_beta_path = beta_path->get_filename();
            this->config.set_beta_path( new_beta_path );
        }
    );
    Gtk::ComboBox * active_xmage;
    builder->get_widget( "UpdateSource" , active_xmage );
    Gtk::TreeModelColumnRecord type_colrec;
    Gtk::TreeModelColumn<std::uint32_t> index_col;
    Gtk::TreeModelColumn<Glib::ustring> type_col;
    type_colrec.add( type_col );
    type_colrec.add( index_col );
    auto typemodel = Gtk::ListStore::create(type_colrec);
    for ( std::uint8_t i = std::uint8_t(XmageType::Beta) ; i <= std::uint8_t(XmageType::Release) ; i++ )
    {
        Gtk::TreeModel::Row row = *(typemodel->append());
        row[index_col] = i;
        row[type_col] = xmagetype_to_string( static_cast<XmageType>(i) );
    }
    active_xmage->set_model(typemodel);
    active_xmage->pack_start( type_col );
    active_xmage->property_active().set_value( static_cast<int>(config.get_active_xmage()) );
    active_xmage->signal_changed().connect(
        [ this , active_xmage ]()
        {
            int source = active_xmage->property_active();
            this->config.set_active_xmage( static_cast<XmageType>(source) );
            this->do_update();
        }
    );

    this->show_all();

    update_dispatcher.connect( sigc::mem_fun( *this , &XmageLauncher::update_widgets ) );
}


XmageLauncher::~XmageLauncher()
{
    if ( this->update_process.joinable() )
    {
        this->update.stop_update();
        this->update_process.join();
    }
    delete setting_dialog;
}

void XmageLauncher::launch_client( void )
{
        //"java -Xms1024m -Xmx1024m -XX:MaxPermSize=384m -XX:+UseConcMarkSweepGC -XX:+CMSClassUnloadingEnabled -jar .\lib\mage-client-1.4.35.jar"
        Glib::ustring version = this->config.get_active_xmage_version();

        Glib::ustring xms_opt = Glib::ustring::compose( "-Xms%1m" , this->config.get_jvm_xms() );
        Glib::ustring xmx_opt = Glib::ustring::compose( "-Xmx%1m" , this->config.get_jvm_xmx() );
        std::vector<Glib::ustring> argvs({
            this->config.get_javaw_path() , xms_opt , xmx_opt , 
            "-XX:MaxPermSize=384m" , "-XX:+UseConcMarkSweepGC" ,
            "-XX:+CMSClassUnloadingEnabled" , "-jar" , "./lib/mage-client-" + version + ".jar"
        });
        Glib::ustring client_path = this->config.get_active_xmage_client();

        Glib::spawn_async_with_pipes( client_path , argvs );
}

void XmageLauncher::launch_server( void )
{
        //"java -Xms256M -Xmx512M -XX:MaxPermSize=256m -Djava.security.policy=./config/security.policy -Djava.util.logging.config.file=./config/logging.config -Dlog4j.configuration=file:./config/log4j.properties -jar ./lib/mage-server-1.4.35.jar"
        Glib::ustring version = this->config.get_active_xmage_version();

        Glib::ustring xms_opt = Glib::ustring::compose( "-Xms%1m" , this->config.get_jvm_xms() );
        Glib::ustring xmx_opt = Glib::ustring::compose( "-Xmx%1m" , this->config.get_jvm_xmx() );
        std::vector<Glib::ustring> argvs({
            this->config.get_java_path() , xms_opt , xmx_opt ,"-XX:MaxPermSize=384m" , "-Djava.security.policy=./config/security.policy",
            "-Djava.util.logging.config.file=./config/logging.config" , "-Dlog4j.configuration=file:./config/log4j.properties"
            , "-jar" , "./lib/mage-server-" + version + ".jar"
        });
        Glib::ustring server_path = this->config.get_active_xmage_serve();

        Glib::spawn_async_with_pipes( server_path , argvs );
}

void XmageLauncher::show_setting( void )
{
    setting_dialog->show_all();
}

void XmageLauncher::close_setting( int )
{
    setting_dialog->hide();
}

void XmageLauncher::disable_launch( void )
{
    Gtk::Button * client_button;
    this->launcher_builder->get_widget( "LauncherClient" , client_button );
    client_button->set_sensitive( false );
    Gtk::Button * server_button;
    this->launcher_builder->get_widget( "LauncherServer" , server_button );
    server_button->set_sensitive( false );
    Gtk::Button * xmage_button;
    this->launcher_builder->get_widget( "LauncherXmage" , xmage_button );
    xmage_button->set_sensitive( false );
}

void XmageLauncher::enable_launch( void )
{
    Gtk::Button * client_button;
    this->launcher_builder->get_widget( "LauncherClient" , client_button );
    client_button->set_sensitive( true );
    Gtk::Button * server_button;
    this->launcher_builder->get_widget( "LauncherServer" , server_button );
    server_button->set_sensitive( true );
    Gtk::Button * xmage_button;
    this->launcher_builder->get_widget( "LauncherXmage" , xmage_button );
    xmage_button->set_sensitive( true );
}

void XmageLauncher::do_update( void )
{
    //stop old update thread
    if ( this->update_process.joinable() )
    {
        this->update.stop_update();
        this->update_process.join();
    }
    //new update thread
    this->update_process = std::thread(
        [ this ]()
        {
            this->update.do_update( this->update_dispatcher );
        }
    );
}

void XmageLauncher::update_widgets( void )
{
    bool update_end;
    std::int64_t now = 0 , total = 0;
    Glib::ustring info;

    LauncherProgressBar * update_progress_bar = nullptr;
    this->launcher_builder->get_widget_derived( "ProgressBar" , update_progress_bar );
    
    this->update.get_data( update_end , now , total , info );
    if ( update_end )
    {
        this->enable_launch();
    }
    else
    {
        this->disable_launch();
    }
    if ( total > 0 )
        info += ":" + std::to_string( now ) + " / "  + std::to_string( total );
    update_progress_bar->set_progress_info( info );
    update_progress_bar->set_progress_value( total ? now/static_cast<gdouble>( total ) : 0 );

    this->queue_draw();
}
