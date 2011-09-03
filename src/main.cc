/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * main.cc
 * Copyright (C) Diego Carrera 2011 <diegocarrera2000@gmail.com>
 * 
 * audio-player is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * audio-player is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtkmm.h>
#include <iostream>
#include <gstreamermm.h>
#include <iomanip>

#include "wav_in.h"
#include "wav_out.h"

#include "config.h"

#ifdef ENABLE_NLS
#  include <libintl.h>
#include <bits/stringfwd.h>
#endif



// global vars
Glib::RefPtr<Gtk::Builder> builder;
Gtk::MenuItem *quit_item_menu;
Gtk::AboutDialog *aboutDialog = NULL;
Gtk::FileChooserDialog *fileChooserDialog = NULL;
std::string filename;
Gtk::Button *buttonPlay, *buttonEffect1, *buttonEffect2, *buttonZoomIn, *buttonZoomOut; 
Gtk::MenuItem *quitMenuItem=0, *aboutMenuItem=0, *openMenuItem=0, *saveMenuItem=0;

Glib::RefPtr<Glib::MainLoop> mainloop;
Glib::RefPtr<Gst::Pipeline> pipeline;
Glib::RefPtr<Gst::Element> decoder;
gulong data_probe_id = 0;


// global interfaces vars
gchar* ui_file[] =
  { "src/audio_player.ui", "src/audio_about.ui", "src/audio_fileChooser.ui" };

#define UI_MAIN 0
#define UI_ABOUT 1
#define UI_FILE_CHOOSER 2

//**********************************************
static bool on_timeout()
{
  Gst::Format fmt = Gst::FORMAT_TIME;
  gint64 pos = 0;
  gint64 len = 0;

  Glib::RefPtr<Gst::Query> query = Gst::QueryPosition::create(fmt);

  if(pipeline->query(query)
    && pipeline->query_duration(fmt, len))
  {
    // Cast query's RefPtr to RefPtr<Gst::QueryPosition> to parse the
    // pipeline's position query with the Gst::QueryPosition::parse() method
    Glib::RefPtr<Gst::QueryPosition> query_pos = Glib::RefPtr<Gst::QueryPosition>::cast_dynamic(query);
    if(query_pos)
      pos = query_pos->parse();

    std::cout << std::right << "Time: " << std::setfill('0') <<
      std::setw(3) << Gst::get_hours(pos) << ":" <<
      std::setw(2) << Gst::get_minutes(pos) << ":" <<
      std::setw(2) << Gst::get_seconds(pos) << "." <<
      std::setw(9) << std::left << Gst::get_fractional_seconds(pos);

    std::cout << std::right << "/" <<
      std::setw(3) << Gst::get_hours(len) << ":" <<
      std::setw(2) << Gst::get_minutes(len) << ":" <<
      std::setw(2) << Gst::get_seconds(len) << "." <<
      std::setw(9) << std::left << Gst::get_fractional_seconds(len) <<
      std::endl << std::flush;
  }

  return true;
}


// This function is used to receive asynchronous messages in the main loop.
static bool on_bus_message(const Glib::RefPtr<Gst::Bus>& /* bus */, const Glib::RefPtr<Gst::Message>& message)
{
  switch(message->get_message_type()) {
    case Gst::MESSAGE_EOS:
      std::cout << std::endl << "End of stream" << std::endl;
      mainloop->quit();
      return false;
    case Gst::MESSAGE_ERROR:
    {
      Glib::RefPtr<Gst::MessageError> msgError = Glib::RefPtr<Gst::MessageError>::cast_dynamic(message);
      if(msgError)
      {
        Glib::Error err;
        err = msgError->parse();
        std::cerr << "Error: " << err.what() << std::endl;
      }
      else
        std::cerr << "Error." << std::endl;

      mainloop->quit();
      return false;
    }
    default:
    {
      //std::cout << "debug: on_bus_message: unhandled message=" << G_OBJECT_TYPE_NAME(message->gobj()) << std::endl;
    }
      break;
  }

  return true;
}

static void on_parser_pad_added(const Glib::RefPtr<Gst::Pad>& newPad)
{
  // We can now link this pad with the audio decoder
  std::cout << "Dynamic pad created. Linking parser/decoder." << std::endl;
  Glib::RefPtr<Gst::Pad> sinkPad = decoder->get_static_pad("sink");
  Gst::PadLinkReturn ret = newPad->link(sinkPad);

  if (ret != Gst::PAD_LINK_OK && ret != Gst::PAD_LINK_WAS_LINKED)
  {
    std::cerr << "Linking of pads " << newPad->get_name() << " and " <<
      sinkPad->get_name() << " failed." << std::endl;
  }
}

static bool on_sink_pad_have_data(const Glib::RefPtr<Gst::Pad>& pad,
        const Glib::RefPtr<Gst::MiniObject>& data)
{
  std::cout << "Sink pad '" << pad->get_name() << "' has received data;";
  std::cout << " will now remove sink data probe id: " << data_probe_id << std::endl;
  pad->remove_data_probe(data_probe_id);
  return true;
}

static void playWav ()
{
  std::cout << "Wav to play: " <<  filename << std::endl;

  mainloop = Glib::MainLoop::create();

  // Create the pipeline:
  pipeline = Gst::Pipeline::create("audio-player");

  // Create the elements:
  // filsrc reads the file from disk:
  Glib::RefPtr<Gst::Element> source = Gst::ElementFactory::create_element("filesrc");
  if(!source)
    std::cerr << "filesrc element could not be created." << std::endl;

  // wavparse into elementary streams (audio and video):
  Glib::RefPtr<Gst::Element> parser = Gst::ElementFactory::create_element("wavparse");
  if(!parser)
    std::cerr << "oggdemux element could not be created." << std::endl;

  // identity decodes a vorbis (audio) stream:
 decoder = Gst::ElementFactory::create_element("identity");
  if(!decoder)
    std::cerr << "vorbisdec element could not be created." << std::endl;

  // audioconvert converts raw audio to a format which can be used by the next element
  Glib::RefPtr<Gst::Element> conv = Gst::ElementFactory::create_element("audioconvert");
  if(!conv)
    std::cerr << "audioconvert element could not be created." << std::endl;

  // Outputs sound to an ALSA audio device
  Glib::RefPtr<Gst::Element> sink = Gst::ElementFactory::create_element("alsasink");
  if(!sink)
    std::cerr << "alsasink element could not be created." << std::endl;

  //Check that the elements were created:
  if(!pipeline || !source || !parser || !decoder || !conv || !sink)
  {
    std::cerr << "One element could not be created" << std::endl;
    return;
  }

  Glib::RefPtr<Gst::Pad> pad = sink->get_static_pad("sink");
  if(pad)
    data_probe_id = pad->add_data_probe( sigc::ptr_fun(&on_sink_pad_have_data) );
	
  //std::cout << "sink data probe id = " << data_probe_id << std::endl;

  source->set_property("location", filename);

  // Get the bus from the pipeline, 
  // and add a bus watch to the default main context with the default priority:
  Glib::RefPtr<Gst::Bus> bus = pipeline->get_bus();
  bus->add_watch( sigc::ptr_fun(&on_bus_message) );


  // Put all the elements in a pipeline:
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
#endif
    pipeline->add(source)->add(parser)->add(decoder)->add(conv)->add(sink);
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << "Error while adding elements to the pipeline: " << ex.what() << std::endl;
    return;
  }
#endif

  // Link the elements together:
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
#endif
    source->link(parser);

    // We cannot link the parser and decoder yet, 
    // because the parser uses dynamic pads.
    // So we do it later in a pad-added signal handler:
    parser->signal_pad_added().connect( sigc::ptr_fun(&on_parser_pad_added) );

    decoder->link(conv)->link(sink);
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  }
  catch(const std::runtime_error& ex)
  {
    std::cout << "Exception while linking elements: " << ex.what() << std::endl;
  }
#endif

  // Call on_timeout function at a 200ms
  // interval to regularly print the position of the stream
  Glib::signal_timeout().connect(sigc::ptr_fun(&on_timeout), 200);

  // Now set the whole pipeline to playing and start the main loop:
  std::cout << "Setting to PLAYING." << std::endl;
  pipeline->set_state(Gst::STATE_PLAYING);
  std::cout << "Running." << std::endl;
  mainloop->run();

  // Clean up nicely:
  std::cout << "Returned. Stopping playback." << std::endl;
  pipeline->set_state(Gst::STATE_NULL);	

  return;
}





//******************** GTK BASIC FUNCTIONS OF MAIN CLASS **************************


// function to builder a UI
static Glib::RefPtr<Gtk::Builder>
get_builder(gint ui)
{
  std::cout << "builder function activated" << std::endl;
  Glib::RefPtr<Gtk::Builder> builder;
  try
    {
      builder = Gtk::Builder::create_from_file(ui_file[ui]);
    }
  catch (const Glib::FileError & ex)
    {
      std::cerr << ex.what() << std::endl;
      //return 1;
    }
  catch (const Gtk::BuilderError& ex)
    {
      std::cerr << "BuilderError: " << ex.what() << std::endl;
      //return 1;
    }

  //gtk_builder_connect_signals (builder, NULL);
  return builder;
}

// function to get & close "about UI"
static void about ()
{

  std::cout << "about function activated" << std::endl;
  Glib::RefPtr<Gtk::Builder> aboutBuilder;
  aboutBuilder = get_builder(UI_ABOUT);
  aboutBuilder->get_widget("aboutWindow", aboutDialog);
  aboutDialog->run();
  aboutDialog->hide();	
}

// function to get & close "choose UI"" 
static int file_chooser ()
{
  int sw=0;
  //get builder for UI_FILE_CHOOSER
  std::cout << "file Chooser function activated" << std::endl;
  Glib::RefPtr<Gtk::Builder> fileChooserBuilder;
  fileChooserBuilder = get_builder(UI_FILE_CHOOSER);
  fileChooserBuilder->get_widget("fileChooserWindow", fileChooserDialog);  

  //Add button accept & cancel
  fileChooserDialog->add_button(Gtk::Stock::OPEN,Gtk::RESPONSE_ACCEPT);
  fileChooserDialog->add_button(Gtk::Stock::CANCEL,Gtk::RESPONSE_CANCEL);

  //Add filters, so that only certain file types can be selected:
  Glib::RefPtr<Gtk::FileFilter> filter_wav = Gtk::FileFilter::create();
  filter_wav->set_name("Wav files");
  filter_wav->add_mime_type("audio/wav");
  fileChooserDialog->add_filter(filter_wav);	
	
  int result=fileChooserDialog->run();

  //get event for button on FileChooserDialog
  switch(result)	
  {
    case(Gtk::RESPONSE_ACCEPT):
	{
	  filename = fileChooserDialog->get_filename();
	  std::cout << "File selected: " <<  filename << std::endl;
	  sw=1;
	  break;
	}
    case(Gtk::RESPONSE_CANCEL):
    {
      std::cout << "Cancel clicked." << std::endl;
      break;
    }		  
	default:
	{
      printf("value es:%i\n",result);
	  std::cout << "Unexpected button clicked." << std::endl;
	  break;
	}	  
  }
  fileChooserDialog->hide();
  return sw;
}

// function to exit and kill application 
static void
destroy()
{
  std::cout << "destroy function activated" << std::endl;
  Gtk::Main::quit();
}

static void printer()
{
  std::cout << "imprime activated" << std::endl;	
}


// function to choose and read structure of a wav file   
static void audio_open(){
  int result=0;
  std::cout << "audio_open function activated" << std::endl;
  result=file_chooser();
	
  if (result){
  
  char *cpFilename = new char[filename.size()+1];
  strcpy(cpFilename,filename.c_str());

  WAV_IN  audio(cpFilename);
  double sampleRate = audio.get_sample_rate_hz();
  unsigned int bitsPerSample = audio.get_bits_per_sample();
  unsigned int channels = audio.get_num_channels();

  saveMenuItem->set_sensitive();
  buttonPlay->set_sensitive (); 
  buttonZoomIn->set_sensitive ();
  buttonZoomOut->set_sensitive ();
  buttonEffect1->set_sensitive ();
  buttonEffect2->set_sensitive ();


  }
}

static void save_as ()
{
  char *cpFilename = new char[filename.size()+1];
  strcpy(cpFilename,filename.c_str());
	
  WAV_IN  audio(cpFilename);
  double sampleRate = audio.get_sample_rate_hz();
  unsigned int bitsPerSample = audio.get_bits_per_sample();
  unsigned int channels = audio.get_num_channels();	

  WAV_OUT outfile(sampleRate, bitsPerSample, channels);
  while(audio.more_data_available()) {
    double data = audio.read_current_input();
    // If this were a real application, you'd probably do something
    //  to the data here.
    outfile.write_current_output(data);
  }
	
  outfile.save_wave_file (cpFilename);
}

int
main(int argc, char *argv[])
{
  //local main vars 
  Gtk::Main kit(argc, argv);
  Gst::init (argc, argv);

  //  call MAIN UI
  builder = get_builder(UI_MAIN);
  Gtk::Window *main_win = 0;
  builder->get_widget("mainWindow", main_win);

  
  builder->get_widget("btnPlay", buttonPlay);
  buttonPlay->signal_clicked().connect(sigc::ptr_fun(&playWav));	
           
  builder->get_widget("btnZoomIn", buttonZoomIn);
  buttonZoomIn->signal_clicked().connect(sigc::ptr_fun(&printer));

  builder->get_widget("btnZoomOut", buttonZoomOut);
  buttonZoomOut->signal_clicked().connect(sigc::ptr_fun(&printer));

  builder->get_widget("btnEffect1", buttonEffect1);
  buttonEffect1->signal_clicked().connect(sigc::ptr_fun(&printer));
	  
  builder->get_widget("btnEffect2", buttonEffect2);
  buttonEffect2->signal_clicked().connect(sigc::ptr_fun(&printer));
	      
	
  // signals for menu	
  if (main_win)
    {
      
      builder->get_widget("quit_item_menu",quitMenuItem);
      builder->get_widget("open_item_menu",openMenuItem);
      builder->get_widget("acerca_item_menu",aboutMenuItem);
	  builder->get_widget("save_item_menu",saveMenuItem);
		
      if(openMenuItem)
      {
        openMenuItem->signal_activate().connect(sigc::ptr_fun(&audio_open));
      }		
      if(saveMenuItem)
      {
        saveMenuItem->signal_activate().connect(sigc::ptr_fun(&save_as));				
      }
      if(quitMenuItem)
      {
        quitMenuItem->signal_activate().connect(sigc::ptr_fun(&destroy));
      }
      if(aboutMenuItem)
      {
        aboutMenuItem->signal_activate().connect(sigc::ptr_fun(&about));
      }

      kit.run(*main_win);
    }
  return 0;
}

