#include <string>
#include <filesystem>

#include <unistd.h>

#include <gtkmm/drawingArea.h>
#include <gtkmm/filechoosernative.h>
#include <gtkmm/liststore.h>
#include <gtkmm/messagedialog.h>
#include <glibmm/i18n.h>
#include <glibmm/ustring.h>
#include <glibmm/spawn.h>
#include <sigc++/signal.h>
#include <sigc++/connection.h>

#include <curl/curl.h>
#ifdef WIN32
#ifdef ERROR
//in msys2-mingw64,curl include ERROR macro(in wingdi.h),glibmm-2.68/iochannel.h have a enum class IOStatus::ERROR,so include curl after glibmm.
#undef ERROR
#endif
#endif

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
        this->set_draw_func(sigc::mem_fun(*this, &LauncherProgressBar::on_draw));
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
            this->queue_draw();
            return ;
        }
        if ( value >= 1.0 )
        {
            this->prog_value = 1.0;
            this->queue_draw();
            return ;
        }

        this->prog_value = value;
        this->queue_draw();
    }

    const double& get_progress_value( void ) const
    {
        return std::ref( this->prog_value );
    }

    void set_progress_info( Glib::ustring info )
    {
        this->prog_info = std::move( info );
        this->queue_draw();
    }

    const Glib::ustring& get_progress_info( void ) const
    {
        return std::ref( this->prog_info );
    }

protected:
    void measure_vfunc( Gtk::Orientation orientation , int , int& minimum , int& natural ,
        int& minimum_baseline , int& natural_baseline ) const override
    {
        if ( orientation == Gtk::Orientation::HORIZONTAL )
        {
            minimum = 30;
            natural = 35;
        }
        else
        {
            minimum = 20;
            natural = 25;
        }
        minimum_baseline = -1;
        natural_baseline = -1;
    }

    void on_draw( const Cairo::RefPtr<Cairo::Context>& cairo_context , int , int )
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
    }

private:
    double prog_value;

    Glib::ustring prog_info;
    Glib::RefPtr<Pango::Layout> layout;
};

class FolderChooserButton : public Gtk::Button
{
public:
    using selected_signal_t = sigc::signal<void(Glib::ustring)>;

    FolderChooserButton( BaseObjectType* cobject , const Glib::RefPtr<Gtk::Builder>& , Gtk::Window& parent_windw,
        Glib::ustring title , Glib::ustring default_filename = "" ):
        Gtk::Button( cobject ),
        filename(default_filename)
    {
        this->set_label(this->filename);
        this->set_expand();
        this->signal_clicked().connect(
            [ this ]()
            {
                this->chooserdialog->show();
            }
        );

        this->chooserdialog = Gtk::FileChooserNative::create( title , parent_windw , Gtk::FileChooser::Action::SELECT_FOLDER );
        this->chooserdialog->signal_response().connect(
            [ this ](int response_id)
            {
                if (response_id == Gtk::ResponseType::ACCEPT)
                {
                    this->set_filename( this->chooserdialog->get_file()->get_path() );
                }
                this->chooserdialog->hide();
            }
        );
    }

    void set_filename( const Glib::ustring& name )
    {
        this->filename = name;
        this->set_label(this->filename);
        this->selected_signal.emit( this->filename );
    }

    const Glib::ustring& get_filename() const
    {
        return this->filename;
    }

    selected_signal_t signal_selected()
    {
        return this->selected_signal;
    }

private:
    Glib::ustring filename;

    selected_signal_t selected_signal;
    Glib::RefPtr<Gtk::FileChooserNative> chooserdialog;
};

XmageLauncher::XmageLauncher( BaseObjectType* cobject , const Glib::RefPtr<Gtk::Builder>& builder ):
    Gtk::Window( cobject ),
    launcher_builder( builder ),
    config( config_t::get_config() ),
    update_process(),
    update_dispatcher()
{
    if ( this->config.get_using_proxy() )
    {
        set_proxy( config.get_proxy_scheme() , config.get_proxy_host() , config.get_proxy_port() );
    }

    //main window button
    this->client_button = builder->get_widget<Gtk::Button>( "LauncherClient" );
    this->server_button = builder->get_widget<Gtk::Button>( "LauncherServer" );
    this->xmage_button = builder->get_widget<Gtk::Button>( "LauncherXmage" );

    //advanced setting dialog
    this->setting_dialog = builder->get_widget<Gtk::Dialog>( "SettingDialog" );
    this->setting_button = builder->get_widget<Gtk::Button>( "SettingButton" );
    this->using_mirror_button = builder->get_widget<Gtk::CheckButton>( "UsingMirror" );
    this->reset_button = builder->get_widget<Gtk::Button>( "ResetConfig" );

    //main window setting
    this->proxy_type = builder->get_widget<Gtk::ComboBox>( "ProxyType" );
    this->proxy_host = builder->get_widget<Gtk::Entry>( "ProxyHost" );
    this->proxy_port = builder->get_widget<Gtk::SpinButton>( "ProxyPort" );
    this->xms_opt = builder->get_widget<Gtk::SpinButton>( "XmsOpt" );
    this->xmx_opt = builder->get_widget<Gtk::SpinButton>( "XmxOpt" );
    this->release_path = Gtk::Builder::get_widget_derived<FolderChooserButton>( builder , "ReleaseMagePath" , *this ,  _("choose release xmage install path") );
    this->beta_path = Gtk::Builder::get_widget_derived<FolderChooserButton>( builder , "BetaMagePath" , *this , _("choose beta xmage install path") );
    this->active_xmage = builder->get_widget<Gtk::ComboBox>( "UpdateSource" );

    fill_setting_value();

    //main window button
    this->client_button->signal_clicked().connect( sigc::mem_fun( *this , &XmageLauncher::launch_client ) );
    this->server_button->signal_clicked().connect( sigc::mem_fun( *this , &XmageLauncher::launch_server ) );
    this->xmage_button->signal_clicked().connect(
        [ this ]()
        {
            this->launch_client();
            this->launch_server();
        }
    );

    //advanced setting dialog
    this->setting_dialog->add_button( _( "close menu" ) , 0 );
    this->setting_dialog->signal_response().connect( sigc::mem_fun( *this , &XmageLauncher::close_setting ) );
    this->setting_button->signal_clicked().connect( sigc::mem_fun( *this , &XmageLauncher::show_setting ) );
    this->using_mirror_button->set_active( this->config.get_using_mirror() );
    this->using_mirror_button->signal_toggled().connect(
        [ this ]()
        {
            this->config.set_using_mirror( this->using_mirror_button->get_active() );
        }
    );
    this->reset_button->signal_clicked().connect(
        [ this ]()
        {
            Gtk::MessageDialog confirm( Glib::ustring(_( "do you want reset config?" )) , false , Gtk::MessageType::ERROR , Gtk::ButtonsType::YES_NO , true );
            confirm.signal_response().connect([this]( int responseType )
            {
                if ( responseType != Gtk::ResponseType::YES )
                    return ;
                this->config.reset_config();
                fill_setting_value();
            });
            confirm.show();

        }
    );

    //main window setting
    this->proxy_type->signal_changed().connect(
        [ this ]()
        {
            Glib::ustring proxy_string = this->proxy_type->get_active_id();
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
    this->proxy_host->signal_changed().connect(
        [ this ]()
        {
            Glib::ustring host_string = this->proxy_host->get_text();
            this->config.set_proxy_host( host_string );
            if ( this->config.get_using_proxy() )
            {
                set_proxy( config.get_proxy_scheme() , config.get_proxy_host() , config.get_proxy_port() );
            }
        }
    );
    this->proxy_port->signal_value_changed().connect(
        [ this ]()
        {
            double port_value = this->proxy_port->get_value();
            this->config.set_proxy_port( port_value );
            if ( this->config.get_using_proxy() )
            {
                set_proxy( config.get_proxy_scheme() , config.get_proxy_host() , config.get_proxy_port() );
            }
        }
    );
    this->xms_opt->signal_value_changed().connect(
        [ this ]()
        {
            double xms_value = this->xms_opt->get_value();
            this->config.set_jvm_xms( xms_value );
        }
    );
    this->xmx_opt->signal_value_changed().connect(
        [ this ]()
        {
            double xmx_value = this->xmx_opt->get_value();
            this->config.set_jvm_xmx( xmx_value );
        }
    );
    release_path->signal_selected().connect(
        [ this ]( Glib::ustring path )
        {
            this->config.set_release_path( path );
        }
    );
    beta_path->signal_selected().connect(
        [ this ]( Glib::ustring path )
        {
            this->config.set_beta_path( path );
        }
    );
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
    this->active_xmage->set_model(typemodel);
    this->active_xmage->pack_start( type_col );
    this->active_xmage->property_active().set_value( static_cast<int>(config.get_active_xmage()) );
    this->active_xmage->signal_changed().connect(
        [ this ]()
        {
            int source = this->active_xmage->property_active();
            this->config.set_active_xmage( static_cast<XmageType>(source) );
            this->do_update();
        }
    );

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
    const Glib::ustring javaw_path = this->config.get_javaw_path();
    if (javaw_path.empty())
    {
        Gtk::MessageDialog confirm( Glib::ustring(_( "java runtime environment not found(need jre8 or jdk8)" )) , false , Gtk::MessageType::ERROR , Gtk::ButtonsType::OK , true );
        confirm.show();
        return ;
    }

    //"java -Xms1024m -Xmx1024m -XX:MaxPermSize=384m -XX:+UseConcMarkSweepGC -Dfile.encoding=UTF-8 
    //-XX:+CMSClassUnloadingEnabled -jar .\lib\mage-client-1.4.35.jar"
    Glib::ustring xms_opt = Glib::ustring::compose( "-Xms%1m" , this->config.get_jvm_xms() );
    Glib::ustring xmx_opt = Glib::ustring::compose( "-Xmx%1m" , this->config.get_jvm_xmx() );
    Glib::ustring jar_path = Glib::ustring::compose( "./lib/mage-client-%1.jar" , this->config.get_active_xmage_version() );
    std::vector<std::string> argvs({
        javaw_path , xms_opt , xmx_opt , 
        "-XX:MaxPermSize=384m" , "-XX:+UseConcMarkSweepGC" , "-Dfile.encoding=UTF-8",
        "-XX:+CMSClassUnloadingEnabled" , "-jar" , jar_path
    });
    Glib::ustring client_path = this->config.get_active_xmage_client();

    Glib::spawn_async_with_pipes( client_path.raw() , argvs );
}

void XmageLauncher::launch_server( void )
{
    const Glib::ustring java_path = this->config.get_java_path();
    if (java_path.empty())
    {
        Gtk::MessageDialog confirm( Glib::ustring(_( "java runtime environment not found(need jre8 or jdk8)" )) , false , Gtk::MessageType::ERROR , Gtk::ButtonsType::OK , true );
        confirm.show();
        return ;
    }

    //"java -Xms256M -Xmx512M -XX:MaxPermSize=256m -Djava.security.policy=./config/security.policy
    //-Djava.util.logging.config.file=./config/logging.config -Dlog4j.configuration=file:./config/log4j.properties -jar ./lib/mage-server-1.4.35.jar"
    Glib::ustring xms_opt = Glib::ustring::compose( "-Xms%1m" , this->config.get_jvm_xms() );
    Glib::ustring xmx_opt = Glib::ustring::compose( "-Xmx%1m" , this->config.get_jvm_xmx() );
    Glib::ustring jar_path = Glib::ustring::compose( "./lib/mage-server-%1.jar" , this->config.get_active_xmage_version() );
    std::vector<std::string> argvs({
        java_path , xms_opt , xmx_opt ,"-XX:MaxPermSize=384m" , "-Djava.security.policy=./config/security.policy", "-Dfile.encoding=UTF-8",
        "-Djava.util.logging.config.file=./config/logging.config" , "-Dlog4j.configuration=file:./config/log4j.properties"
        , "-jar" , jar_path
    });
    Glib::ustring server_path = this->config.get_active_xmage_serve();

    Glib::spawn_async_with_pipes( server_path.raw() , argvs );
}

void XmageLauncher::show_setting( void )
{
    setting_dialog->show();
}

void XmageLauncher::close_setting( int )
{
    setting_dialog->hide();
}

void XmageLauncher::disable_launch( void )
{
    this->client_button->set_sensitive( false );
    this->server_button->set_sensitive( false );
    this->xmage_button->set_sensitive( false );
}

void XmageLauncher::enable_launch( void )
{
    this->client_button->set_sensitive( true );
    this->server_button->set_sensitive( true );
    this->xmage_button->set_sensitive( true );
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

    LauncherProgressBar * update_progress_bar = Gtk::Builder::get_widget_derived<LauncherProgressBar>( launcher_builder , "ProgressBar" );
    
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

void XmageLauncher::fill_setting_value( void )
{
    if ( this->config.get_using_proxy() )
    {
        this->proxy_type->set_active_id( this->config.get_proxy_scheme() );
    }
    else
    {
        this->proxy_type->set_active_id( "None" );
    }

    this->proxy_host->set_text( this->config.get_proxy_host() );

    this->proxy_port->set_value( this->config.get_proxy_port() );
    this->xms_opt->set_value( this->config.get_jvm_xms() );
    this->xmx_opt->set_value( this->config.get_jvm_xmx() );
    this->release_path->set_filename( this->config.get_release_path() );
    this->beta_path->set_filename( this->config.get_beta_path() );
}
