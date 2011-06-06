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

#include "wav_in.h"
#include "wav_out.h"

#include "config.h"

#ifdef ENABLE_NLS
#  include <libintl.h>
#include <bits/stringfwd.h>
#endif

/* For testing propose use the local (not installed) ui file */
/* #define UI_FILE PACKAGE_DATA_DIR"/audio_player/ui/audio_player.ui" */
#define UI_FILE "src/audio_player.ui"

gchar* ui_file[] =
  { "src/audio_player.ui", "src/audio_about.ui" };

Gtk::MenuItem* quit_item_menu;
Gtk::AboutDialog *aboutDialog = NULL;

#define UI_MAIN 0
#define UI_ABOUT 1


static void
on_open_image(Gtk::Image* image)
{
  Gtk::FileChooserDialog dialog("Open image", Gtk::FILE_CHOOSER_ACTION_OPEN);
  dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

  Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
  filter->add_pixbuf_formats();
  dialog.add_filter(filter);

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

  // call function get_builder for MAIN ui
  builder = get_builder(UI_MAIN);
  Gtk::Window* main_win = 0;
  builder->get_widget("mainWindow", main_win);

  if (main_win)
    {
      Gtk::MenuItem* quitMenuItem=0, *aboutMenuItem=0, *efectoMenuItem;
      builder->get_widget("quit_item_menu",quitMenuItem);
      builder->get_widget("acerca_menu_item",aboutMenuItem);
      builder->get_widget("efecto1_menu_item",efectoMenuItem);
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

      kit.run(*main_win);
    }
  return 0;
}

