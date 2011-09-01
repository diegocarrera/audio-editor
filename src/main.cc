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

// global interfaces vars
gchar* ui_file[] =
  { "src/audio_player.ui", "src/audio_about.ui", "src/audio_fileChooser.ui" };

#define UI_MAIN 0
#define UI_ABOUT 1
#define UI_FILE_CHOOSER 2



//******************** class Sound **************************

class Sound
{
  public:
	Sound();

	void start_playing(double frequency);
	bool stop_playing();
		
  private:
	Glib::RefPtr<Gst::Pipeline> m_pipeline;
	Glib::RefPtr<Gst::Element> m_source;
	Glib::RefPtr<Gst::Element> m_sink;
};

  
Sound::Sound()
{
  m_pipeline = Gst::Pipeline::create("note");
  m_source = Gst::ElementFactory::create_element("audiotestsrc","source");
  m_sink = Gst::ElementFactory::create_element("autoaudiosink","output");
  m_pipeline->add(m_source);
  m_pipeline->add(m_sink);
  m_source->link(m_sink);
}
// class function to play a sound
void Sound::start_playing (double frequency)
{
  m_source->set_property("freq", frequency);
  m_pipeline->set_state(Gst::STATE_PLAYING);

  /* stop it after 200ms */
	Glib::signal_timeout().connect(sigc::mem_fun(*this, &Sound::stop_playing),200);
}

// class function to stop a sound
bool Sound::stop_playing()
{
  m_pipeline->set_state(Gst::STATE_NULL);
  return false;
}


//**********************************************



//******************** FUNCTIONS OF MAIN **************************

// function for a signal button  that play a sound
static void
on_button_clicked(double frequency, Sound* sound)
{
  sound->start_playing (frequency);
}

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


// function to choose and read structure of a wav file   
static void audio_open(){
  int result=0;
  Sound sound;	
  std::cout << "audio_open function activated" << std::endl;
  result=file_chooser();
	
 // char *filename = "/home/dcarrera/source/gtk3/audio-player/src/file.wav";

  if (result){
  
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

	  
  builder->get_widget("btnPlay", buttonPlay);
  buttonPlay->set_sensitive (true);

  builder->get_widget("btnZoomIn", buttonZoomIn);
  buttonZoomIn->set_sensitive (true);

  builder->get_widget("btnZoomOut", buttonZoomOut);
  buttonZoomOut->set_sensitive (true);

  builder->get_widget("btnEffect1", buttonEffect1);
  buttonEffect1->set_sensitive (true);

  builder->get_widget("btnEffect2", buttonEffect2);
  buttonEffect2->set_sensitive (true);
	  
  //outfile.save_wave_file(filename);
  buttonPlay->signal_clicked().connect (sigc::bind<double, Sound*>(sigc::ptr_fun(&on_button_clicked),
	                                 369.23, &sound));	  
  }
  
}

static void save_as ()
{

}

int
main(int argc, char *argv[])
{
  //local main vars 
  Gtk::Main kit(argc, argv);
  Gst::init (argc, argv);
  Gtk::MenuItem *quitMenuItem=0, *aboutMenuItem=0, *efectoMenuItem=0, 
			    *openMenuItem=0, *saveMenuItem=0;

  //  call MAIN UI
  builder = get_builder(UI_MAIN);
  Gtk::Window *main_win = 0;
  builder->get_widget("mainWindow", main_win);

  // signals for menu	
  if (main_win)
    {
      
      builder->get_widget("quit_item_menu",quitMenuItem);
      builder->get_widget("open_item_menu",openMenuItem);
      builder->get_widget("acerca_item_menu",aboutMenuItem);
      builder->get_widget("efecto1_item_menu",efectoMenuItem);
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

