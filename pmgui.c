/**
 * radeon-pm-gui: Power Management GUI for Radeon Graphics Cards in Linux
 * Copyright (C) 2012, Aaron Watry
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**
 * Title..: Radeon Power Management GUI
 * Author.: Aaron Watry (awatry@gmail.com)
 * Purpose: Provide GUI reporting/control of the sysfs interfaces exported by
 *          the radeon kernel module
 * Usage..: ./radeon-pm-gui
 * 
 * TODO...: - Report in disabled textbox if using dynpm or profile method
 *          - When profile, display current profile
 *          - Add temperature monitoring for cards that export the correct sysfs nodes
 *          - Add giant disclaimer to GUI
 *          - Add ability to switch between dynpm/profile
 *          - Add ability to select profile from available profiles
 *          - Use something like gksudo to request permissions to read/write stuff
 *          - If adequate permissions, display current/min/max core/memory clocks
 *          - Add ability to handle multiple cards
 *          - Add buttons to set ALL cards to a specific profile/method
 */

#include <gtk/gtk.h>
#include "pmlib.h"

//echo "profile" > / sys / class / drm / card0 / device / power_method
//echo "low" > / sys / class / drm / card0 / device / power_profile
//echo "dynpm" > /sys/class/drm/card0/device/power_method

//Card Type..: /sys/class/drm/card0/device/hwmon/hwmon1/name    "radeon"
//Temperature: /sys/class/drm/card0/device/hwmon/hwmon1/temp1_input
//Frequency (root only): /sys/kernel/debug/dri/0/radeon_pm_info

static pm_method_t curMethod;
static pm_profile_t curProfile;
static char* curCard = NULL;

//XXX: in the main() function, store an array of buttons so that function calls 
//     can swap the button statuses of all buttons
static GObject** guiWidgets = NULL;

static void changeCard(GtkComboBoxText* combo,
        gpointer data){
    
    //Get the current selected index, and then the card that goes with that index.
	gchar *cardName = gtk_combo_box_text_get_active_text( combo );

	g_print("changing card to %s\n", cardName);
	//XXX: Save the card name in curCard
	
	//XXX: Change the GUI elements for the current card's profile/method/temp
	
	//Free the name string
	g_free(cardName);
}

static void toggleLocked(GtkWidget *widget, gpointer data){
	//get current toggle status
	gboolean status = gtk_toggle_button_get_active( (GtkToggleButton*)widget );
	g_print("Toggle locked status:");
	g_print( (status == TRUE ? "ON" : "OFF") );
	g_print("\n");
}

static void changePMProfile(GtkWidget *widget,
        gpointer data){
    
    pm_profile_t newProfile = *(pm_profile_t*)data;
    
    if (curCard == NULL){
        curCard = "card0";
    }
    
    if (newProfile < 0 || newProfile >= MAX_PROFILE){
        g_error("Invalid new profile %d\n", newProfile);
        return;
    }
    
    if (canModifyPM()) {
        g_print("Setting profile %s\n", pm_profile_names[newProfile]);

        setMethod(curCard, PROFILE);
        setProfile(curCard, newProfile);
    } else {
        g_print("Insufficient permissions to set profile to %s\n", pm_profile_names[newProfile]);
    }
}

/**
 * 
 * @param widget
 * @param data
 */
static void
print_hello(GtkWidget *widget,
        gpointer data) {
    
    char **cards = getCards((char*) DEFAULT_DRM_DIR);
    if (cards == NULL) {
        g_printerr("Card list is empty.\n");
        return;
    }

    int idx = 0;
    while (cards[idx] != NULL) {
        g_print("Found card: %s\n", cards[idx]);

        g_print("method = %d\n", getMethod(cards[idx]));
        if (getMethod(cards[idx]) != METHOD_UNKNOWN) {
            g_print("profile = %d\n", getProfile(cards[idx]));
        }

        if (canModifyPM()) {
            g_print("Setting dynpm\n");
            setMethod(cards[idx], DYNPM);
            g_print("method = %d\n", getMethod(cards[idx]));
            if (getMethod(cards[idx]) != METHOD_UNKNOWN) {
                g_print("profile = %d\n", getProfile(cards[idx]));
            }

            g_print("Setting low profile\n");
            setMethod(cards[idx], PROFILE);
            setProfile(cards[idx], LOW);
            g_print("method = %d\n", getMethod(cards[idx]));
            if (getMethod(cards[idx]) != METHOD_UNKNOWN) {
                g_print("profile = %d\n", getProfile(cards[idx]));
            }
        }

        g_print("Current temperature: %d\n", getTemperature(cards[idx]));

        idx++;
    }
    freeCards(cards);

}

void dynpm(GtkWidget *widget, gpointer data){
    char **cards = getCards((char*) DEFAULT_DRM_DIR);
    if (cards == NULL) {
        g_printerr("Card list is empty.\n");
        return;
    }

    int idx = 0;
    while (cards[idx] != NULL) {
        g_print("Found card: %s\n", cards[idx]);

        if (canModifyPM()) {
            g_print("Setting dynpm\n");
            setMethod(cards[idx], DYNPM);
            g_print("method = %d\n", getMethod(cards[idx]));
            if (getMethod(cards[idx]) != METHOD_UNKNOWN) {
                g_print("profile = %d\n", getProfile(cards[idx]));
            }
        } else {
            g_print("Insufficient permission to modify PM method\n");
        }
        idx++;
    }
    freeCards(cards);
}

int main(int argc,
        char *argv[]) {
    GtkBuilder *builder;
    GObject *window;
    GObject *button;
    GObject *textView;
    GtkToggleButton *toggle;
    char **cardNames;
	int i;
	
    gtk_init(&argc, &argv);

    /* Construct a GtkBuilder instance and load our UI description */
    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "pmgui.ui", NULL);

    /* Connect signal handlers to the constructed widgets. */
    window = gtk_builder_get_object(builder, "window");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GObject *cards = gtk_builder_get_object(builder, "cmb_cards");
    g_signal_connect(cards, "changed", G_CALLBACK(changeCard), NULL);
	
	//Add all cards to combo box
	cardNames = getCards((char*) DEFAULT_DRM_DIR);
	if (cardNames != NULL){
		while (*cardNames != NULL){
			g_print("Adding card %s\n", *cardNames);
			//XXX: Add some identifying information to the card names (model/brand/etc).
			//XXX: During detection, store a chipset manufacturer (AMD/Nv) somewhere.
			//XXX: Either store a list of names, a list of descriptions, list of manufacturers, etc..
			//     Or use a standardized delimiter to make parsing easy.
			gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT( cards ), *cardNames );
			cardNames++;
		}
		freeCards(cardNames);
	}

    toggle = (GtkToggleButton*)gtk_builder_get_object(builder, "tgl_root");
    if (canModifyPM())
        gtk_toggle_button_set_active(toggle, TRUE);
    else
        gtk_toggle_button_set_active(toggle, FALSE);
	g_signal_connect(toggle, "toggled", G_CALLBACK(toggleLocked), NULL);

	//XXX: Change the dynpm/profile switch into a sliding toggle.
	//XXX: Move the profile selection to a visually distinct area from the dynpm/profile switch.
	button = gtk_builder_get_object(builder, "btn_dynpm");
    g_signal_connect(button, "clicked", G_CALLBACK(dynpm), NULL);

    button = gtk_builder_get_object(builder, "btn_default");
    g_signal_connect(button, "clicked", G_CALLBACK(changePMProfile), &pm_profiles[DEFAULT]);

    button = gtk_builder_get_object(builder, "btn_auto");
    g_signal_connect(button, "clicked", G_CALLBACK(changePMProfile), &pm_profiles[AUTO]);
    
    button = gtk_builder_get_object(builder, "btn_low");
    g_signal_connect(button, "clicked", G_CALLBACK(changePMProfile), &pm_profiles[LOW]);

    button = gtk_builder_get_object(builder, "btn_medium");
    g_signal_connect(button, "clicked", G_CALLBACK(changePMProfile), &pm_profiles[MEDIUM]);

    button = gtk_builder_get_object(builder, "btn_high");
    g_signal_connect(button, "clicked", G_CALLBACK(changePMProfile), &pm_profiles[HIGH]);

    button = gtk_builder_get_object(builder, "quit");
    g_signal_connect(button, "clicked", G_CALLBACK(gtk_main_quit), NULL);

    textView = gtk_builder_get_object(builder, "txt_temperature");
    g_signal_connect(textView, "move-cursor", G_CALLBACK(print_hello), NULL);
    
	//XXX: Set the current index/text for the combo box to the first card in the list.
	//XXX: use the changeCard callback
	
    gtk_main();

    return 0;
}
