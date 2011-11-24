/* Copyright 2006 Florian Paul Schmidt. See the file LICENSE for license details */

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <gdk/gdkx.h>

#include <cstdlib>

#include <iostream>
#include <fstream>
#include <string>

#include <alsa/asoundlib.h>
#include <alsa/seq_event.h>

#include <lo/lo.h>

#include "kontroll.h"

#ifdef HAVE_LASH
#include <lash/lash.h>
#endif


/*
	some global variables. this is a simple app, so it's ok.
*/
std::string  file_name = "";
GladeXML    *xml;
snd_seq_t   *seq;
int          seq_port_id;
std::string dotfile_name;

#ifdef HAVE_LASH
lash_client_t *lash_client = 0;
#endif


void save_settings (std::string filename)
{
	std::cout << "saving to file: " << filename << std::endl;
	std::ofstream stream (filename.c_str());

	int x_cc, y_cc, x_channel, y_channel;
	float x_bottom, y_bottom, x_top, y_top;
	const char *x_url;
	const char *y_url;
	bool midi_on_off, osc_on_off;

	x_cc = (int)gtk_spin_button_get_value((GtkSpinButton*)glade_xml_get_widget(xml, "x_axis_cc_spin"));
	y_cc = (int)gtk_spin_button_get_value((GtkSpinButton*)glade_xml_get_widget(xml, "y_axis_cc_spin"));

	x_channel = (int)gtk_spin_button_get_value((GtkSpinButton*)glade_xml_get_widget(xml, "channel_x_spin"));
	y_channel = (int)gtk_spin_button_get_value((GtkSpinButton*)glade_xml_get_widget(xml, "channel_y_spin"));

	midi_on_off = gtk_toggle_button_get_active ((GtkToggleButton*)glade_xml_get_widget(xml, "midi_on_off_toggle"));

	x_url = gtk_entry_get_text((GtkEntry*)glade_xml_get_widget(xml, "osc_x_url_entry"));
	y_url = gtk_entry_get_text((GtkEntry*)glade_xml_get_widget(xml, "osc_y_url_entry"));

	x_bottom = gtk_spin_button_get_value((GtkSpinButton*)glade_xml_get_widget(xml, "osc_x_range_bottom"));
	x_top = gtk_spin_button_get_value((GtkSpinButton*)glade_xml_get_widget(xml, "osc_x_range_top"));
	y_bottom = gtk_spin_button_get_value((GtkSpinButton*)glade_xml_get_widget(xml, "osc_y_range_bottom"));
	y_top = gtk_spin_button_get_value((GtkSpinButton*)glade_xml_get_widget(xml, "osc_y_range_top"));

	osc_on_off = gtk_toggle_button_get_active ((GtkToggleButton*)glade_xml_get_widget(xml, "osc_on_off_toggle"));

	stream << x_cc << " "<< y_cc << " " << x_channel << " " << y_channel << " " << midi_on_off << " " << x_bottom << " " << x_top << " " << y_bottom << " " << y_top << " " << " " << x_url << " " << y_url << " " << osc_on_off << std::endl;
}

void load_settings (std::string filename)
{
	std::cout << "loading from file: " << filename << std::endl;
	std::ifstream stream (filename.c_str());

	if (!stream.good())
	{
		std::cout << "stream for loading not good. returning..." << std::endl;
		return;
	}

	int x_cc, y_cc, x_channel, y_channel;
	float x_bottom, y_bottom, x_top, y_top;
	std::string x_url;
	std::string y_url;
	bool midi_on_off, osc_on_off;

	stream >> x_cc >> y_cc >> x_channel >> y_channel >> midi_on_off >> x_bottom >> x_top >> y_bottom >> y_top >> x_url >> y_url >> osc_on_off;

	gtk_spin_button_set_value((GtkSpinButton*)glade_xml_get_widget(xml, "x_axis_cc_spin"), x_cc);
	gtk_spin_button_set_value((GtkSpinButton*)glade_xml_get_widget(xml, "y_axis_cc_spin"), y_cc);

	gtk_spin_button_set_value((GtkSpinButton*)glade_xml_get_widget(xml, "channel_x_spin"), x_channel);
	gtk_spin_button_set_value((GtkSpinButton*)glade_xml_get_widget(xml, "channel_y_spin"), y_channel);

	gtk_toggle_button_set_active ((GtkToggleButton*)glade_xml_get_widget(xml, "midi_on_off_toggle"), midi_on_off);
	gtk_toggle_button_set_active ((GtkToggleButton*)glade_xml_get_widget(xml, "osc_on_off_toggle"), osc_on_off);

	gtk_entry_set_text((GtkEntry*)glade_xml_get_widget(xml, "osc_x_url_entry"), x_url.c_str());
	gtk_entry_set_text((GtkEntry*)glade_xml_get_widget(xml, "osc_y_url_entry"), y_url.c_str());

	gtk_spin_button_set_value((GtkSpinButton*)glade_xml_get_widget(xml, "osc_x_range_bottom"), x_bottom);
	gtk_spin_button_set_value((GtkSpinButton*)glade_xml_get_widget(xml, "osc_x_range_top"), x_top);
	gtk_spin_button_set_value((GtkSpinButton*)glade_xml_get_widget(xml, "osc_y_range_bottom"), y_bottom);
	gtk_spin_button_set_value((GtkSpinButton*)glade_xml_get_widget(xml, "osc_y_range_top"), y_top);

}
/*
	the gtk signal handlers
*/
extern "C"
{
	void on_new_activate (GtkWidget *widget, gpointer user_data)
	{
		// std::cout << "file -> new activated" << std::endl;
	}

	void on_open_activate (GtkWidget *widget, gpointer user_data)
	{
		std::cout << "file -> open activated" << std::endl;
		GtkWidget *dialog;

		dialog = gtk_file_chooser_dialog_new ("Open File", (GtkWindow*)glade_xml_get_widget(xml, "mainwindow"), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

		if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
		{
			char *filename;
			filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

			std::cout << "selected: " << filename << std::endl;
			file_name = filename;

			load_settings (file_name);

			g_free (filename);
		}

		gtk_widget_destroy (dialog);


	}

	void on_save_as_activate (GtkWidget *widget, gpointer user_data)
	{
		// std::cout << "file -> save as activated" << std::endl;
		GtkWidget *dialog;

		dialog = gtk_file_chooser_dialog_new ("Save File As..", (GtkWindow*)glade_xml_get_widget(xml, "mainwindow"), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);

		if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
		{
			char *filename;
			filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

			// std::cout << "selected: " << filename << std::endl;
			file_name = filename;

			save_settings (file_name);

			g_free (filename);
		}

		gtk_widget_destroy (dialog);
	}

	void on_save_activate (GtkWidget *widget, gpointer user_data)
	{
		// std::cout << "file -> save activated" << std::endl;

		if (file_name == "")
			on_save_as_activate (widget, user_data);

		save_settings (file_name);
	}

	void on_quit_activate (GtkWidget *widget, gpointer user_data)
	{
		// std::cout << "file -> quit activated" << std::endl;
		save_settings (dotfile_name);
		gtk_main_quit ();
	}

	void on_about_activate (GtkWidget *widget, gpointer user_data)
	{
		// std::cout << "help -> about activated" << std::endl;
		gtk_widget_show (glade_xml_get_widget(xml, "about_dialog"));
	}

	void on_mainwindow_destroy_event (GtkWidget *widget, gpointer user_data) 
	{
		// std::cout << "destroy event. exiting" << std::endl;
		save_settings (dotfile_name);
		gtk_main_quit ();
	}

	void on_mainwindow_delete_event (GtkWidget *widget, gpointer user_data) 
	{
		// std::cout << "delete event. exiting" << std::endl;
		save_settings (dotfile_name);
		gtk_main_quit ();
	}
}

void setup_alsa_port ()
{
	if (snd_seq_open (&seq, "default", SND_SEQ_OPEN_OUTPUT, 0) < 0) 
	{
		std::cout << "Error opening ALSA sequencer." << std::endl;

		exit (EXIT_FAILURE);
	}

	snd_seq_set_client_name(seq, "Kontroll");
	
	if ((seq_port_id = snd_seq_create_simple_port (seq, "Output",
		SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
		SND_SEQ_PORT_TYPE_APPLICATION)) < 0) 
	{
		std::cout << "Error creating sequencer port." << std::endl;

		exit (EXIT_FAILURE);
	}
}

/*
	value needs to be between 0 and 1
*/
void send_controller_msg (double value, int cc, int channel)
{
	snd_seq_event_t ev;

	snd_seq_ev_clear(&ev);

	snd_seq_ev_set_direct(&ev);
	snd_seq_ev_set_subs(&ev);
	snd_seq_ev_set_source(&ev, seq_port_id);

	ev.type = SND_SEQ_EVENT_CONTROLLER;

	// correct by -1, because UI shows standard midi usage
	ev.data.control.channel = channel - 1;
	// (int)gtk_spin_button_get_value((GtkSpinButton*)glade_xml_get_widget(xml, "channel_spin")) - 1;

	ev.data.control.param = cc - 1;	
	ev.data.control.value = (int)(128.0 * value);

	snd_seq_event_output_direct(seq, &ev);
	snd_seq_drain_output(seq);
}

/* x and y will be between 0 and 1 */
void send_osc_msg (float x, float y)
{
	lo_address address_x =  lo_address_new_from_url (gtk_entry_get_text((GtkEntry*)glade_xml_get_widget(xml, "osc_x_url_entry")));

	lo_address address_y =  lo_address_new_from_url (gtk_entry_get_text((GtkEntry*)glade_xml_get_widget(xml, "osc_y_url_entry")));

	char *x_path = lo_url_get_path (gtk_entry_get_text((GtkEntry*)glade_xml_get_widget(xml, "osc_x_url_entry")));
	char *y_path = lo_url_get_path (gtk_entry_get_text((GtkEntry*)glade_xml_get_widget(xml, "osc_y_url_entry")));

	float x_bottom = gtk_spin_button_get_value((GtkSpinButton*)glade_xml_get_widget(xml, "osc_x_range_bottom"));
	float x_top = gtk_spin_button_get_value((GtkSpinButton*)glade_xml_get_widget(xml, "osc_x_range_top"));
	float y_bottom = gtk_spin_button_get_value((GtkSpinButton*)glade_xml_get_widget(xml, "osc_y_range_bottom"));
	float y_top = gtk_spin_button_get_value((GtkSpinButton*)glade_xml_get_widget(xml, "osc_y_range_top"));

	if (lo_send (address_x, x_path, "f", x_bottom + (x_top - x_bottom)*x) == -1)
		std::cout << "whoops OSC not worky!#!@!" <<std::endl;

	if (lo_send (address_y, y_path, "f", y_bottom + (y_top - y_bottom)*y) == -1)
		std::cout << "whoops OSC not worky!#!@!" <<std::endl;

	lo_address_free (address_x);
	lo_address_free (address_y);

	free (x_path);
	free (y_path);
}

#ifdef HAVE_LASH
gboolean query_lash_events (gpointer data)
{
	lash_event_t* lash_event;
	lash_config_t* lash_config;

	char filename [1024];

	while ((lash_event = lash_get_event(lash_client))) {
		switch (lash_event_get_type(lash_event)) {
			case LASH_Save_File:
				snprintf (filename, 1023, "%s/beef", lash_event_get_string (lash_event));
				save_settings (filename);
				lash_send_event (lash_client, lash_event);
			break;

			case LASH_Restore_File:
				snprintf (filename, 1023, "%s/beef", lash_event_get_string (lash_event));
				load_settings (filename);
				lash_send_event (lash_client, lash_event);
			break;

			case LASH_Quit:
				printf("- LASH asked us to quit!\n");
				// quit = 1; 
				lash_event_destroy(lash_event);

				gtk_main_quit();
				break;

			default:
				printf("- Got unhandled LASH event - ignoring\n");
			}
	}

	while ((lash_config = lash_get_config(lash_client))) {
			lash_config_destroy(lash_config);
	}

	return TRUE;
}
#endif


gboolean query_mouse_position (gpointer data)
{
	static gint mouse_x;
	static gint mouse_y;
	static GdkModifierType mouse_mask;

	gint x,y;
	GdkModifierType mask;

	gdk_window_get_pointer (gdk_screen_get_root_window (gdk_display_get_screen (gdk_display_get_default (),0)), &x, &y, &mask);

	if (x != mouse_x || y != mouse_y)
	{
		/*
			Ok, something happened, so let's send a midi event.
			We normalize the mouse x and y values to the 0..1 interval.
		*/
		if (gtk_toggle_button_get_active ((GtkToggleButton*)glade_xml_get_widget(xml, "midi_on_off_toggle")))
		{
			int cc, channel;

			cc = (int)gtk_spin_button_get_value((GtkSpinButton*)glade_xml_get_widget(xml, "x_axis_cc_spin"));
			channel = (int)gtk_spin_button_get_value((GtkSpinButton*)glade_xml_get_widget(xml, "channel_x_spin"));

			send_controller_msg ((float)x/(float)gdk_screen_width (), cc,channel); 

			cc = (int)gtk_spin_button_get_value((GtkSpinButton*)glade_xml_get_widget(xml, "y_axis_cc_spin"));
			channel = (int)gtk_spin_button_get_value((GtkSpinButton*)glade_xml_get_widget(xml, "channel_y_spin"));

			send_controller_msg ((float)y/(float)gdk_screen_height (), cc, channel);
		}
		
		if (gtk_toggle_button_get_active ((GtkToggleButton*)glade_xml_get_widget(xml, "osc_on_off_toggle")))
		{
			send_osc_msg ((float)x/(float)gdk_screen_width (), (float)y/(float)gdk_screen_height ()); 
		}
	}

	mouse_x = x;
	mouse_y = y;
	mouse_mask = mask;
	// std::cout << "query mouse" << std::endl;
	return TRUE;
}

int main (int argc, char *argv[])
{
	std::cout << "Kontroll - (c) 2006 - Florian Paul Schmidt" << std::endl;

	dotfile_name = std::string (getenv("HOME")) + std::string ("/.kontroll");

	setup_alsa_port ();

	gtk_init (&argc, &argv);
	glade_init ();

	xml = glade_xml_new (PREFIX "/share/kontroll/kontroll.glade",  NULL, NULL);

	/* connect signal handlers */
	glade_xml_signal_autoconnect (xml);
	
	/* 
		this periodically called function will check for mouse
		movement.
	*/
	gtk_timeout_add (10, query_mouse_position, 0);

	load_settings (dotfile_name);

#ifdef HAVE_LASH
	lash_event_t *lash_event;

	lash_client = lash_init(lash_extract_args(&argc, &argv), "kontroll",
	                        LASH_Config_File, LASH_PROTOCOL(2, 0));
	if (lash_client)
		std::cout << "- Connected to LASH server" << std::endl;
	else
		std::cout << "- Failed to connect to LASH server" << std::endl;

	if (lash_client)
	{
		lash_jack_client_name(lash_client, "kontroll");

		lash_alsa_client_id (lash_client, seq_port_id);

		lash_event = lash_event_new_with_type(LASH_Client_Name);
		lash_event_set_string(lash_event, "kontroll");
		lash_send_event(lash_client, lash_event);
		gtk_timeout_add (500, query_lash_events, 0);
	}
#endif  


	gtk_main ();

}
