/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * main.c
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

#include <config.h>
#include <gtk/gtk.h>

/* translate libs*/
#include <glib/gi18n.h>


/* For testing propose use the local (not installed) ui file */
/* #define UI_FILE PACKAGE_DATA_DIR"/audio_player/ui/audio_player.ui" */
#define MAIN 0
#define ABOUT 1

#define UI_FILE "src/audio_player.ui"

/* about windows interface, created with glade3*/ 
#define UI_ABOUT "src/audio_about.ui"

gchar* ui_file[] = {"src/audio_player.ui", "src/audio_about.ui"};

/* Signal handlers */
/* Note: These may not be declared static because signal autoconnection
 * only works with non-static methods
 */

static GtkWidget *window = NULL;	

GtkBuilder *get_builder (gint ui)
{	
	GtkBuilder *builder = gtk_builder_new ();
	GError* error = NULL;
	if (!gtk_builder_add_from_file (builder, ui_file[ui], &error))
	{
		g_warning ("Couldn't load builder file: %s", error->message);
		g_error_free (error);
	}
	gtk_builder_connect_signals (builder, NULL);

	return builder;
}

G_MODULE_EXPORT void about (GtkWidget *widget, gpointer data)
{

  g_message("calling a builder function");
  // Get the window object from the ui file, ABOUT=1
  GtkBuilder *builder = get_builder(ABOUT);	
  g_message("builder loaded"); 	
  
  GtkDialog *about_dlg = GTK_DIALOG (gtk_builder_get_object (builder, "aboutWindow"));  	
  gtk_dialog_run(about_dlg);

  g_message("free builder-about");	
  gtk_widget_destroy(GTK_WIDGET(about_dlg));	
  
	
	
  //error en esta linea.. no se qporque
  //gtk_dialog_run (GTK_DIALOG (about_dlg));
	
  
	

}

G_MODULE_EXPORT  void onOpenImage(GtkWidget *widget, gpointer data)
{
	g_message("open");

	GtkWidget *image = GTK_WIDGET (data);
	GtkWidget *toplevel = gtk_widget_get_toplevel (image);
	GtkFileFilter *filter = gtk_file_filter_new ();
	GtkWidget *dialog = gtk_file_chooser_dialog_new (_("Open image"),
	                                                 GTK_WINDOW (toplevel),
	                                                 GTK_FILE_CHOOSER_ACTION_OPEN,
	                                                 GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
	                                                 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
	                                                 NULL);
	gtk_file_filter_add_pixbuf_formats (filter);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
	
	switch (gtk_dialog_run (GTK_DIALOG (dialog)))
	{
		case GTK_RESPONSE_ACCEPT:
		{
			gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
			gtk_image_set_from_file (GTK_IMAGE (image), filename);
			break;
		}
		default:
			break;
	}
	gtk_widget_destroy (dialog);
	
}


/*
G_MODULE_EXPORT void about2()
{
	printf("hola.about");

	GtkWidget *windowAbout;

	GError* error = NULL;

// Load UI from file 
	builder = gtk_builder_new ();
	if (!gtk_builder_add_from_file (builder, UI_ABOUT, &error))
	{
		g_warning ("Couldn't load builder file: %s", error->message);
		g_error_free (error);
		g_message("builder about NOT ok");
	}
	g_message("builder about ok");
	gtk_builder_connect_signals (builder, NULL);
	
	// Get the window object from the ui file 
	windowAbout = GTK_WIDGET (gtk_builder_get_object (builder, "window-about"));
	gtk_widget_get_toplevel(windowAbout);
	
	g_message("builder about LOADED");
	gtk_widget_destroy (windowAbout);
	g_message("window about destroy");
}
*/




/* Called when the window is closed */
G_MODULE_EXPORT  void destroy (GtkWidget *widget, gpointer data)
{
	gtk_main_quit ();
}


static GtkWidget* create_window (void)
{
	
	g_message("main-window");
	// Load UI from file 
	GtkBuilder *builder = get_builder(MAIN);
	g_message("get object from builder");
	// Get the window object from the ui file 
	window = GTK_WIDGET (gtk_builder_get_object (builder, "window"));
	//free builder
	g_object_unref (builder);

	g_message("window-main-loaded");
	return window;
}



int main (int argc, char *argv[])
{
 	GtkWidget *window = NULL;

#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif

	gtk_init (&argc, &argv);	
	window = create_window();	
	gtk_widget_show(window);
	
	gtk_main ();
	return 0;
}
