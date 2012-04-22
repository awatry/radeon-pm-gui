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
 * 
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

static int setDynpm(char *card) {
    return setMethod(card, DYNPM);
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

void lowProfile(GtkWidget *widget, gpointer data){
    char **cards = getCards((char*) DEFAULT_DRM_DIR);
    if (cards == NULL) {
        g_printerr("Card list is empty.\n");
        return;
    }

    int idx = 0;
    while (cards[idx] != NULL) {
        g_print("Found card: %s\n", cards[idx]);

        if (canModifyPM()) {
            g_print("Setting low profile\n");
            setMethod(cards[idx], PROFILE);
            setProfile(cards[idx], LOW);
            g_print("method = %d\n", getMethod(cards[idx]));
            if (getMethod(cards[idx]) != METHOD_UNKNOWN) {
                g_print("profile = %d\n", getProfile(cards[idx]));
            }
        }
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
    GObject *textBox;
    
    gtk_init(&argc, &argv);

    /* Construct a GtkBuilder instance and load our UI description */
    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "pmgui.ui", NULL);

    /* Connect signal handlers to the constructed widgets. */
    window = gtk_builder_get_object(builder, "window");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    button = gtk_builder_get_object(builder, "btn_low_profile");
    g_signal_connect(button, "clicked", G_CALLBACK(lowProfile), NULL);

    button = gtk_builder_get_object(builder, "btn_dynpm");
    g_signal_connect(button, "clicked", G_CALLBACK(dynpm), NULL);

    button = gtk_builder_get_object(builder, "quit");
    g_signal_connect(button, "clicked", G_CALLBACK(gtk_main_quit), NULL);

    textBox = gtk_builder_get_object(builder, "txt_temperature");
    g_signal_connect(textBox, "clicked", G_CALLBACK(print_hello), NULL);
    
    gtk_main();

    return 0;
}