## Process this file with automake to produce Makefile.in

## Created by Anjuta


uidir = $(datadir)/audio_player/ui
ui_DATA = audio_player.ui


AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(AUDIO_PLAYER_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = audio_player

audio_player_SOURCES = \
	wav_in.cc \
	wav_out.cc \
	main.cc

audio_player_LDFLAGS = 

audio_player_LDADD = $(AUDIO_PLAYER_LIBS)


EXTRA_DIST = $(ui_DATA)

# Remove ui directory on uninstall
uninstall-local:
	-rm -r $(uidir)
	-rm -r $(datadir)/audio_player

