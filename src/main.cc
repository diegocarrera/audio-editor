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

#define UI_FILE "src/audio_player.ui"

gchar* ui_file[] =
  { "src/audio_player.ui", "src/audio_about.ui", "src/audio_fileChooser.ui" };

Gtk::MenuItem* quit_item_menu;
Gtk::AboutDialog *aboutDialog = NULL;
Gtk::FileChooserDialog *fileChooserDialog = NULL;

#define UI_MAIN 0
#define UI_ABOUT 1
#define UI_FILE_CHOOSER 2

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
  m_source = Gst::ElementFactory::create_element("audiotestsrc",
                                                 "source");
  m_sink = Gst::ElementFactory::create_element("autoaudiosink",
	                                             "output");
  m_pipeline->add(m_source);
  m_pipeline->add(m_sink);
  m_source->link(m_sink);
}

void Sound::start_playing (double frequency)
{
  m_source->set_property("freq", frequency);
  m_pipeline->set_state(Gst::STATE_PLAYING);

  /* stop it after 200ms */
	Glib::signal_timeout().connect(sigc::mem_fun(*this, &Sound::stop_playing),200);
}

bool Sound::stop_playing()
{
  m_pipeline->set_state(Gst::STATE_NULL);
  return false;
}

static void
on_button_clicked(double frequency, Sound* sound)
{
  sound->start_playing (frequency);

	
}
static void
on_open_image(Gtk::Image* image)
{
  Gtk::FileChooserDialog dialog("Open image", Gtk::FILE_CHOOSER_ACTION_OPEN);
  dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

  switch (dialog.run())
    {
  case Gtk::RESPONSE_ACCEPT:
    image->set(dialog.get_filename());
    break;
  default:
    break;
    }
  dialog.hide();
}



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


static void about ()
{

  std::cout << "about function activated" << std::endl;
  Glib::RefPtr<Gtk::Builder> aboutBuilder;
  aboutBuilder = get_builder(UI_ABOUT);
  aboutBuilder->get_widget("aboutWindow", aboutDialog);
  aboutDialog->run();
  aboutDialog->hide();	
}

static void file_chooser ()
{
  std::cout << "file Chooser function activated" << std::endl;
  Glib::RefPtr<Gtk::Builder> fileChooserBuilder;
  fileChooserBuilder = get_builder(UI_FILE_CHOOSER);
  fileChooserBuilder->get_widget("fileChooserWindow", fileChooserDialog);
  int result=fileChooserDialog->run();

  switch(result)
		
  {
    case(Gtk::RESPONSE_ACCEPT):
	{
      std::cout << "ENTRA" << std::endl;
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
	
}

static void
destroy()
{
  std::cout << "destroy function activated" << std::endl;
	Gtk::Main::quit();
}

static void
audio_open(){
  std::cout << "audio_open function activated" << std::endl;

  char *filename = "/home/dcarrera/source/gtk3/audio-player/src/file.wav";
  WAV_IN  audio(filename);
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
  outfile.save_wave_file("/home/dcarrera/source/gtk3/audio-player/src/outfile.wav");

}

int
main(int argc, char *argv[])
{
  //variables
  Glib::RefPtr<Gtk::Builder> builder;
  Gtk::Main kit(argc, argv);
  Gst::init (argc, argv);
  Sound sound;
  Gtk::Button* button;	

  // call function get_builder for MAIN ui
  builder = get_builder(UI_MAIN);
  Gtk::Window* main_win = 0;
  builder->get_widget("mainWindow", main_win);

  //btn for sound
  builder->get_widget("btnPlay", button);
  button->signal_clicked().connect (sigc::bind<double, Sound*>(sigc::ptr_fun(&on_button_clicked),
	                                 369.23, &sound));

 // builder->get_widget("btnLoad",button);
 // button->signal_clicked().connect (sigc::bind<Gtk::Image*>(sigc::ptr_fun(&on_open_image), image));
	
  if (main_win)
    {
      Gtk::MenuItem* quitMenuItem=0, *aboutMenuItem=0, *efectoMenuItem=0, *openMenuItem=0;
      builder->get_widget("quit_item_menu",quitMenuItem);
      builder->get_widget("open_item_menu",openMenuItem);
      builder->get_widget("acerca_item_menu",aboutMenuItem);
      builder->get_widget("efecto1_item_menu",efectoMenuItem);
      if(quitMenuItem)
        {
          quitMenuItem->signal_activate().connect(sigc::ptr_fun(&destroy));
        }
      if(aboutMenuItem)
        {
          aboutMenuItem->signal_activate().connect(sigc::ptr_fun(&about));
        }
      if(efectoMenuItem)
        {
          efectoMenuItem->signal_activate().connect(sigc::ptr_fun(&audio_open));
        }
      if(openMenuItem)
        {
          openMenuItem->signal_activate().connect(sigc::ptr_fun(&file_chooser));
        }		

      kit.run(*main_win);
    }
  return 0;
}

