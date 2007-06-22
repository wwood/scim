/** @file scim_input_pad.cpp
 */

/*
 * Smart Common Input Method
 * 
 * Copyright (c) 2005 James Su <suzhe@tsinghua.org.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: scim_input_pad.cpp,v 1.12 2005/11/21 06:40:31 suzhe Exp $
 *
 */

#include <wctype.h>
#include <gtk/gtk.h>
#include <config.h>
#include "scim_input_group.h"

#define scim_module_init input_pad_LTX_scim_module_init
#define scim_module_exit input_pad_LTX_scim_module_exit
#define scim_helper_module_number_of_helpers input_pad_LTX_scim_helper_module_number_of_helpers
#define scim_helper_module_get_helper_info input_pad_LTX_scim_helper_module_get_helper_info
#define scim_helper_module_run_helper input_pad_LTX_scim_helper_module_run_helper

#define SCIM_INPUT_PAD_ICON                             (SCIM_ICONDIR "/input-pad.png")
#define SCIM_CONFIG_HELPER_INPUT_PAD_SHOW_PREVIEW       "/Helper/InputPad/ShowPreview"
#define SCIM_CONFIG_HELPER_INPUT_PAD_PREVIEW_SIZE       "/Helper/InputPad/PreviewSize"
#define SCIM_CONFIG_HELPER_INPUT_PAD_PREVIEW_DELAY      "/Helper/InputPad/PreviewDelay"
#define SCIM_CONFIG_HELPER_INPUT_PAD_CURRENT_GROUP      "/Helper/InputPad/CurrentGroup"
#define SCIM_CONFIG_HELPER_INPUT_PAD_CURRENT_TABLE      "/Helper/InputPad/CurrentTable"
#define SCIM_CONFIG_HELPER_INPUT_PAD_ENABLE_REPEAT      "/Helper/InputPad/EnableRepeat"
#define SCIM_CONFIG_HELPER_INPUT_PAD_REPEAT_DELAY       "/Helper/InputPad/RepeatDelay"
#define SCIM_CONFIG_HELPER_INPUT_PAD_REPEAT_FREQUENCY   "/Helper/InputPad/RepeatFrequency"
#define SCIM_CONFIG_HELPER_INPUT_PAD_MAIN_WINDOW_XPOS   "/Helper/InputPad/MainWindowXPos"
#define SCIM_CONFIG_HELPER_INPUT_PAD_MAIN_WINDOW_YPOS   "/Helper/InputPad/MainWindowYPos"

#define SCIM_RECENTLY_USED_PAD_FILE                     "/.scim/input-pad-recently-used.pad"

static GtkWidget *create_table_page (GtkWidget *table_widget, const InputTablePointer &table, size_t start, size_t n, bool recently);
static GtkWidget *create_group_page (const InputGroupPointer &group);
static GtkWidget *create_recently_used_group_page ();
static GtkWidget *create_group_notebook (const std::vector <InputGroupPointer> &groups);
static gboolean   main_window_delete_callback (GtkWidget *widget, GdkEvent *event, gpointer data);
static gboolean   create_table_idle_func (gpointer data);
static gboolean   show_preview_timeout (gpointer data);
static void       send_button_content (GtkButton *button);
static void       add_button_content_to_recently_used (GtkButton *button);
static void       button_pressed_callback (GtkButton *button, gpointer user_data);
static void       button_released_callback (GtkButton *button, gpointer user_data);
static void       options_button_clicked_callback (GtkButton *button, gpointer user_data);
static void       options_checkbox_toggled_callback (GtkToggleButton *togglebutton, gpointer user_data);
static gboolean   button_repeat_timeout (gpointer data);
static void       group_notebook_switch_page_callback (GtkNotebook *notebook, GtkNotebookPage *, guint page, gpointer data);
static void       table_notebook_switch_page_callback (GtkNotebook *notebook, GtkNotebookPage *, guint page, gpointer data);
static gboolean   button_crossing_callback (GtkWidget *button, GdkEventCrossing *event, gpointer data);
static gboolean   helper_agent_input_handler (GIOChannel *source, GIOCondition condition, gpointer user_data);
static void       slot_exit (const HelperAgent *, int ic, const String &uuid);
static void       slot_update_screen (const HelperAgent *, int ic, const String &uuid, int screen);
static void       slot_trigger_property (const HelperAgent *agent, int ic, const String &uuid, const String &property);
static void       run (const String &display);
static void       load_recently_used ();
static void       save_recently_used ();

static HelperAgent  helper_agent;
static GtkWidget   *main_window;
static GtkWidget   *main_notebook;

static GtkWidget   *preview_window;
static GtkWidget   *preview_label;

static GtkTooltips *tooltips;

static gint         preview_size = 80;
static gint         preview_delay = 1000;
static bool         show_preview = true;
static gint         current_group = 0;
static gint         current_table = 0;
static bool         enable_repeat = true;
static gint         repeat_delay  = 600;
static gint         repeat_frequency = 15;

static gint         main_window_xpos = 0;
static gint         main_window_ypos = 0;

static std::vector <InputGroupPointer> input_groups;

static InputTablePointer  recently_used_chars;
static InputTablePointer  recently_used_keyevents;
static GtkWidget         *recently_used_chars_vbox;
static GtkWidget         *recently_used_keyevents_vbox;

static HelperInfo  helper_info (String ("ff110940-b8f0-4062-9ff6-a84f4f3575c0"),
                                "",
                                String (SCIM_INPUT_PAD_ICON),
                                "",
                                SCIM_HELPER_STAND_ALONE|SCIM_HELPER_NEED_SCREEN_INFO);


//Module Interface
extern "C" {
    void scim_module_init (void)
    {
        bindtextdomain (GETTEXT_PACKAGE, SCIM_INPUT_PAD_LOCALEDIR);
        bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");

        helper_info.name = String (_("Input Pad"));
        helper_info.description = String (_("An On Screen Input Pad to input some symbols easily."));
    }

    void scim_module_exit (void)
    {
    }

    unsigned int scim_helper_module_number_of_helpers (void)
    {
        return 1;
    }

    bool scim_helper_module_get_helper_info (unsigned int idx, HelperInfo &info)
    {
        if (idx == 0) {
            info = helper_info; 
            return true;
        }
        return false;
    }

    void scim_helper_module_run_helper (const String &uuid, const ConfigPointer &config, const String &display)
    {
        SCIM_DEBUG_MAIN(1) << "input_pad_LTX_scim_helper_module_run_helper ()\n";

        if (uuid == "ff110940-b8f0-4062-9ff6-a84f4f3575c0") {
            show_preview  = config->read (String (SCIM_CONFIG_HELPER_INPUT_PAD_SHOW_PREVIEW), (bool) show_preview);
            preview_size  = config->read (String (SCIM_CONFIG_HELPER_INPUT_PAD_PREVIEW_SIZE), (int) preview_size);
            preview_delay = config->read (String (SCIM_CONFIG_HELPER_INPUT_PAD_PREVIEW_DELAY), (int) preview_delay);
            current_group = config->read (String (SCIM_CONFIG_HELPER_INPUT_PAD_CURRENT_GROUP), (int) current_group);
            current_table = config->read (String (SCIM_CONFIG_HELPER_INPUT_PAD_CURRENT_TABLE), (int) current_table);
            enable_repeat = config->read (String (SCIM_CONFIG_HELPER_INPUT_PAD_ENABLE_REPEAT), (bool) enable_repeat);
            repeat_delay  = config->read (String (SCIM_CONFIG_HELPER_INPUT_PAD_REPEAT_DELAY), (int) repeat_delay);
            repeat_frequency = config->read (String (SCIM_CONFIG_HELPER_INPUT_PAD_REPEAT_FREQUENCY), (int) repeat_frequency);
            main_window_xpos = config->read (String (SCIM_CONFIG_HELPER_INPUT_PAD_MAIN_WINDOW_XPOS), (int) main_window_xpos);
            main_window_ypos = config->read (String (SCIM_CONFIG_HELPER_INPUT_PAD_MAIN_WINDOW_YPOS), (int) main_window_ypos);

            if (repeat_frequency == 0) repeat_frequency = 1;
            else if (repeat_frequency > 100) repeat_frequency = 100;

            run (display);

            config->write (String (SCIM_CONFIG_HELPER_INPUT_PAD_SHOW_PREVIEW), (bool) show_preview);
            config->write (String (SCIM_CONFIG_HELPER_INPUT_PAD_PREVIEW_SIZE), (int) preview_size);
            config->write (String (SCIM_CONFIG_HELPER_INPUT_PAD_PREVIEW_DELAY), (int) preview_delay);
            config->write (String (SCIM_CONFIG_HELPER_INPUT_PAD_CURRENT_GROUP), (int) current_group);
            config->write (String (SCIM_CONFIG_HELPER_INPUT_PAD_CURRENT_TABLE), (int) current_table);
            config->write (String (SCIM_CONFIG_HELPER_INPUT_PAD_ENABLE_REPEAT), (bool) enable_repeat);
            config->write (String (SCIM_CONFIG_HELPER_INPUT_PAD_REPEAT_DELAY), (int) repeat_delay);
            config->write (String (SCIM_CONFIG_HELPER_INPUT_PAD_REPEAT_FREQUENCY), (int) repeat_frequency);
            config->write (String (SCIM_CONFIG_HELPER_INPUT_PAD_MAIN_WINDOW_XPOS), (int) main_window_xpos);
            config->write (String (SCIM_CONFIG_HELPER_INPUT_PAD_MAIN_WINDOW_YPOS), (int) main_window_ypos);
        }

        SCIM_DEBUG_MAIN(1) << "exit input_pad_LTX_scim_helper_module_run_helper ()\n";
    }
}

static gboolean
helper_agent_input_handler (GIOChannel *source, GIOCondition condition, gpointer user_data)
{
    if (condition == G_IO_IN) {
        HelperAgent *agent = static_cast<HelperAgent *> (user_data);
        if (agent && agent->has_pending_event ())
            agent->filter_event ();
    } else if (condition == G_IO_ERR || condition == G_IO_HUP) {
        gtk_main_quit ();
    }
    return TRUE;
}

static void
slot_exit (const HelperAgent *, int ic, const String &uuid)
{
    gtk_main_quit ();
}

static void
slot_update_screen (const HelperAgent *, int ic, const String &uuid, int screen)
{
    if (gdk_display_get_n_screens (gdk_display_get_default ()) > screen) {
        GdkScreen *scr = gdk_display_get_screen (gdk_display_get_default (), screen);
        if (scr)
            gtk_window_set_screen (GTK_WINDOW (main_window), scr);
    }
}

static void
slot_trigger_property (const HelperAgent *agent, int ic, const String &uuid, const String &property)
{
    if (property == "/InputPad") {
        if (GTK_WIDGET_VISIBLE (main_window)) {
            gtk_window_get_position (GTK_WINDOW (main_window), &main_window_xpos, &main_window_ypos);
            gtk_widget_hide (main_window);
        } else {
            gtk_window_move (GTK_WINDOW (main_window), main_window_xpos, main_window_ypos);
            gtk_widget_show (main_window);
        }
    }
}

static void
send_button_content (GtkButton *button)
{
    if (helper_agent.get_connection_number () < 0) return;

    gpointer type = g_object_get_data (G_OBJECT (button), "element_type");

    if (type == (gpointer) INPUT_ELEMENT_STRING) {
        const gchar *str = gtk_button_get_label(GTK_BUTTON(button));
        if (str)
            helper_agent.commit_string (-1, "", scim::utf8_mbstowcs (str));
    } else if (type == (gpointer) INPUT_ELEMENT_KEY) {
        uint32 code = static_cast <uint32> (GPOINTER_TO_INT (g_object_get_data (G_OBJECT (button), "element_key_code")));
        uint16 mask = static_cast <uint16> (GPOINTER_TO_INT ((size_t)g_object_get_data (G_OBJECT (button), "element_key_mask")));
        KeyEvent key (code, (mask & ~SCIM_KEY_ReleaseMask));
        KeyEvent key_release (code, mask | SCIM_KEY_ReleaseMask);
        if (!key.empty ()) {
            helper_agent.send_key_event (-1, "", key);
            helper_agent.send_key_event (-1, "", key_release);
        }
    }
}

static void
add_button_content_to_recently_used (GtkButton *button)
{
    InputElement elm;
    GtkWidget *vbox = 0;

    elm.type = (InputElementType) GPOINTER_TO_INT(g_object_get_data (G_OBJECT (button), "element_type"));

    if (elm.type == INPUT_ELEMENT_STRING) {
        const gchar *str = gtk_button_get_label(GTK_BUTTON(button));
        if (str) {
            elm.data = String (str);
            if (!recently_used_chars->find_element (elm)) {
                recently_used_chars->prepend_element (elm);
                if (recently_used_chars->number_of_elements () > 50)
                    recently_used_chars->drop_oldest ();
                vbox = recently_used_chars_vbox;
            }
        }
    } else if (elm.type == INPUT_ELEMENT_KEY) {
        uint32 code = static_cast <uint32> (GPOINTER_TO_INT (g_object_get_data (G_OBJECT (button), "element_key_code")));
        uint16 mask = static_cast <uint16> (GPOINTER_TO_INT ((size_t)g_object_get_data (G_OBJECT (button), "element_key_mask")));
        KeyEvent key (code, (mask & ~SCIM_KEY_ReleaseMask));
        if (!key.empty ()) {
            elm.data = key.get_key_string ();
            if (!recently_used_keyevents->find_element (elm)) {
                recently_used_keyevents->prepend_element (elm);
                if (recently_used_keyevents->number_of_elements () > 20)
                    recently_used_keyevents->drop_oldest ();
                vbox = recently_used_keyevents_vbox;
            }
        }
    }

    // destroy the old table page.
    if (vbox) {
        GtkWidget  *widget;
        widget = (GtkWidget *) g_object_get_data (G_OBJECT (vbox), "table_root_widget");
        if (widget) gtk_widget_destroy (widget);
        g_object_set_data (G_OBJECT (vbox), "table_widget", 0);
        g_object_set_data (G_OBJECT (vbox), "table_viewport", 0);
        g_object_set_data (G_OBJECT (vbox), "table_root_widget", 0);
        g_object_set_data (G_OBJECT (vbox), "done", (gpointer) 0);
    }
}

static void
button_pressed_callback (GtkButton *button, gpointer recently)
{
    send_button_content (button);

    if (!recently) {
        add_button_content_to_recently_used (button);
    }

    if (enable_repeat) {
        guint id = g_timeout_add (repeat_delay, button_repeat_timeout, button);
        g_object_set_data (G_OBJECT (button), "button_repeat_timeout_id", (gpointer) id);
        g_object_set_data (G_OBJECT (button), "initial_pressed", (gpointer) 1);
    }
}

static void
button_released_callback (GtkButton *button, gpointer user_data)
{
    if (enable_repeat) {
        guint id = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (button), "button_repeat_timeout_id"));
        g_source_remove (id);
        g_object_set_data (G_OBJECT (button), "initial_pressed", (gpointer) 0);
    }
}

static gboolean
button_repeat_timeout (gpointer data)
{
    GtkButton *button = GTK_BUTTON (data);

    send_button_content (button);

    if (g_object_get_data (G_OBJECT (button), "initial_pressed") == (gpointer) 1) {
        guint id = g_timeout_add (1000/repeat_frequency, button_repeat_timeout, button);
        g_object_set_data (G_OBJECT (button), "button_repeat_timeout_id", (gpointer) id);
        g_object_set_data (G_OBJECT (button), "initial_pressed", (gpointer) 0);
        return FALSE;
    }
    return TRUE;
}

static gboolean
show_preview_timeout (gpointer data)
{
    GtkWidget *button = GTK_WIDGET (data);

    String str;
    WideString wstr;

    char buf [1024];
    const char *label = gtk_button_get_label (GTK_BUTTON (button));

    snprintf (buf, 1024, "<span font_desc=\"%d\">%s</span>\n", preview_size, label);

    str = String (buf);
    wstr = utf8_mbstowcs (label);

    for (size_t i = 0; i < wstr.length (); ++i) {
        snprintf (buf, 1024, (wstr [i] > 0xFFFF) ? "U+%06X " : "U+%04X ", wstr [i]);
        str += String (buf);
    }

    gtk_label_set_markup (GTK_LABEL (preview_label), str.c_str ());

    gint x, y;
    GtkRequisition requisition;
    GdkScreen *screen;
    GdkScreen *pointer_screen;
    gint scrw, scrh;

    gtk_widget_size_request (preview_window, &requisition);
    gtk_window_resize (GTK_WINDOW(preview_window), requisition.width, requisition.height);

    screen = gtk_widget_get_screen (button);
    gdk_display_get_pointer (gdk_screen_get_display (screen),
                             &pointer_screen, &x, &y, NULL);


    scrw = gdk_screen_get_width (screen);
    scrh = gdk_screen_get_height (screen);

    if (pointer_screen != screen) {
        x = MAX (0, (scrw - requisition.width) / 2);
        y = MAX (0, (scrh - requisition.height) / 2);
    }

    gtk_window_move (GTK_WINDOW (preview_window), x + 2, y + 2);
    gtk_widget_show (preview_window);

    return FALSE;
}

static gboolean
button_crossing_callback (GtkWidget *button, GdkEventCrossing *event, gpointer data)
{
    if (!show_preview) return FALSE;

    if (event->type == GDK_ENTER_NOTIFY) {
        guint id = g_timeout_add (preview_delay, show_preview_timeout, button);
        g_object_set_data (G_OBJECT (button), "preview_timeout_id", (gpointer) id);
    } else {
        if (GTK_WIDGET_VISIBLE (preview_window)) {
            gtk_widget_hide (preview_window);
        } else {
            guint id = GPOINTER_TO_INT(g_object_get_data (G_OBJECT (button), "preview_timeout_id"));
            g_source_remove (id);
        }
    }
    return FALSE;
}

static GtkWidget *
create_table_page (GtkWidget *table_widget, const InputTablePointer &table, size_t start, size_t n, bool recently)
{
    GtkWidget *button;

    if (table.null () || table->number_of_elements () == 0) return 0;

    if (!table_widget) {
        unsigned int rows;

        rows = table->number_of_elements () / table->get_columns ();
        if (table->number_of_elements () % table->get_columns ()) rows ++;

        table_widget = gtk_table_new (rows, table->get_columns (), TRUE);
        gtk_table_set_row_spacings(GTK_TABLE(table_widget), 0);
        gtk_table_set_col_spacings(GTK_TABLE(table_widget), 0);
        gtk_widget_show (table_widget);
    }

    size_t i = 0;

    if (n == 0) n = table->number_of_elements ();

    for (i = start; i < table->number_of_elements () && n > 0; ++i, --n) {
        InputElement elm = table->get_element (i);

        if (elm.type == INPUT_ELEMENT_NONE) continue;

        int row = i / table->get_columns ();
        int col = i % table->get_columns ();

        button = 0;
        bool preview = false;

        if (elm.type == INPUT_ELEMENT_STRING) {
            button = gtk_button_new_with_label (elm.data.c_str ());
            g_object_set_data (G_OBJECT (button), "element_type", (gpointer) INPUT_ELEMENT_STRING);
            preview = true;
        } else if (elm.type == INPUT_ELEMENT_KEY) {
            KeyEvent key (elm.data);
            if (!key.empty ()) {
                String label;
                ucs4_t wc = key.get_unicode_code ();
                if (key.mask == 0 && wc > 0x20 && (key.code < 0xF000 || key.code > 0x01000000)) {
                    char mb [8];
                    mb [utf8_wctomb ((unsigned char *)mb, wc, 8)] = 0;
                    label = String (mb);
                    preview = true;
                } else {
                    label = elm.data;
                }
                button = gtk_button_new_with_label (label.c_str ());
                g_object_set_data (G_OBJECT (button), "element_type", (gpointer) INPUT_ELEMENT_KEY);
                g_object_set_data (G_OBJECT (button), "element_key_code", (gpointer) ((size_t) key.code));
                g_object_set_data (G_OBJECT (button), "element_key_mask", (gpointer) ((size_t) key.mask));
            }
        }

        if (button) {
            gtk_widget_show (button);
            g_signal_connect (G_OBJECT (button), "pressed", G_CALLBACK (button_pressed_callback), (gpointer) recently);
            g_signal_connect (G_OBJECT (button), "released", G_CALLBACK (button_released_callback), (gpointer) recently);

            if (preview) {
                g_signal_connect (G_OBJECT (button), "enter-notify-event", G_CALLBACK (button_crossing_callback), 0);
                g_signal_connect (G_OBJECT (button), "leave-notify-event", G_CALLBACK (button_crossing_callback), 0);
            }
            gtk_table_attach_defaults (GTK_TABLE (table_widget), button, col, col+1, row, row+1);
        }
    }

    return table_widget;
}

static void
set_table_vbox_width (GtkWidget *vbox)
{
    GtkWidget *viewport = (GtkWidget *) g_object_get_data (G_OBJECT (vbox), "table_viewport");
    GtkWidget *widget = (GtkWidget *) g_object_get_data (G_OBJECT (vbox), "table_widget");

    GtkRequisition req;

    gtk_widget_size_request (widget, &req);
    gtk_widget_set_size_request (viewport, req.width + 2, -1);
}

static void
group_notebook_switch_page_callback (GtkNotebook *notebook, GtkNotebookPage *, guint page, gpointer data)
{
    GtkWidget *tablebook = gtk_notebook_get_nth_page (notebook, page);

    gint curtable = gtk_notebook_get_current_page (GTK_NOTEBOOK (tablebook));

    GtkWidget *vbox = gtk_notebook_get_nth_page (GTK_NOTEBOOK (tablebook), curtable);

    current_group = page;

    if (g_object_get_data (G_OBJECT (vbox), "done") == (gpointer) 1) {
        set_table_vbox_width (vbox);
        return;
    }

    g_idle_add (create_table_idle_func, (gpointer) ((page << 16) | (curtable & 0xFFFF)));
}

static void
table_notebook_switch_page_callback (GtkNotebook *notebook, GtkNotebookPage *, guint page, gpointer data)
{
    gint       grouppage = gtk_notebook_get_current_page (GTK_NOTEBOOK (main_notebook));

    GtkWidget *vbox = gtk_notebook_get_nth_page (notebook, page);

    current_table = page;

    if (g_object_get_data (G_OBJECT (vbox), "done") == (gpointer) 1) {
        set_table_vbox_width (vbox);
        return;
    }

    g_idle_add (create_table_idle_func, (gpointer) ((grouppage << 16) | (page & 0xFFFF)));
}

static GtkWidget *
create_group_page (const InputGroupPointer &group)
{
    GtkWidget *notebook;
    GtkWidget *vbox;
  
    if (group.null () || group->number_of_tables () == 0) return 0;

    notebook = gtk_notebook_new ();
    gtk_notebook_set_scrollable (GTK_NOTEBOOK(notebook), TRUE);
    gtk_notebook_popup_enable (GTK_NOTEBOOK(notebook));
    gtk_notebook_set_show_border (GTK_NOTEBOOK(notebook), FALSE);

    for (size_t i = 0; i < group->number_of_tables (); ++i) {
        InputTablePointer table = group->get_table (i);
        if (!table.null () && table->number_of_elements () > 0) {
            vbox = gtk_vbox_new (FALSE, 10);
            gtk_widget_show (vbox);
            g_object_set_data (G_OBJECT (vbox), "table_pointer", (gpointer) table.get ());
            g_object_set_data (G_OBJECT (vbox), "table_widget", 0);
            gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                                      vbox,
                                      gtk_label_new (table->get_name ().c_str ()));
        }
    }

    gtk_widget_show (notebook);
    g_signal_connect (G_OBJECT (notebook), "switch-page", G_CALLBACK (table_notebook_switch_page_callback), 0);

    return notebook;
}

static GtkWidget *
create_recently_used_group_page ()
{
    GtkWidget *notebook;
    GtkWidget *vbox;

    notebook = gtk_notebook_new ();
    gtk_notebook_set_scrollable (GTK_NOTEBOOK(notebook), TRUE);
    gtk_notebook_popup_enable (GTK_NOTEBOOK(notebook));
    gtk_notebook_set_show_border (GTK_NOTEBOOK(notebook), FALSE);

    // Recently used chars
    vbox = gtk_vbox_new (FALSE, 10);
    gtk_widget_show (vbox);
    g_object_set_data (G_OBJECT (vbox), "table_pointer", (gpointer) recently_used_chars.get ());
    g_object_set_data (G_OBJECT (vbox), "table_widget", 0);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                              vbox,
                              gtk_label_new (_(recently_used_chars->get_name ().c_str ())));

    g_object_set_data (G_OBJECT (vbox), "recently", (gpointer) 1);
    recently_used_chars_vbox = vbox;

    // Recently used KeyEvents
    vbox = gtk_vbox_new (FALSE, 10);
    gtk_widget_show (vbox);
    g_object_set_data (G_OBJECT (vbox), "table_pointer", (gpointer) recently_used_keyevents.get ());
    g_object_set_data (G_OBJECT (vbox), "table_widget", 0);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                              vbox,
                              gtk_label_new (_(recently_used_keyevents->get_name ().c_str ())));

    g_object_set_data (G_OBJECT (vbox), "recently", (gpointer) 1);
    recently_used_keyevents_vbox = vbox;

    gtk_widget_show (notebook);
    g_signal_connect (G_OBJECT (notebook), "switch-page", G_CALLBACK (table_notebook_switch_page_callback), 0);

    return notebook;
}

static GtkWidget *
create_group_notebook (const std::vector <InputGroupPointer> &groups)
{
    GtkWidget *notebook;

    if (groups.size () == 0) return 0;

    notebook = gtk_notebook_new();
    gtk_notebook_set_scrollable (GTK_NOTEBOOK(notebook), TRUE);
    gtk_notebook_popup_enable (GTK_NOTEBOOK(notebook));

    for (size_t i = 0; i < groups.size (); ++i) {
        if (groups [i]->number_of_tables () > 0) {
            gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                                      create_group_page (groups [i]),
                                      gtk_label_new (groups [i]->get_name ().c_str ()));
        }
    }

    // Add recently used group page
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                              create_recently_used_group_page (),
                              gtk_label_new (_("Recently Used")));

    gtk_widget_show (notebook);
    g_signal_connect (G_OBJECT (notebook), "switch-page", G_CALLBACK (group_notebook_switch_page_callback), 0);

    return notebook;
}

static gboolean
create_table_idle_func (gpointer data)
{
    gint       grouppage = gtk_notebook_get_current_page (GTK_NOTEBOOK (main_notebook));
    GtkWidget *tablebook = gtk_notebook_get_nth_page (GTK_NOTEBOOK (main_notebook), grouppage);
    gint       tablepage = gtk_notebook_get_current_page (GTK_NOTEBOOK (tablebook));
    GtkWidget  *vbox     = gtk_notebook_get_nth_page (GTK_NOTEBOOK (tablebook), tablepage);

    if (data != (gpointer) (grouppage << 16 | (tablepage & 0xFFFF)))
        return FALSE;

    if (g_object_get_data (G_OBJECT (vbox), "done") == (gpointer) 1)
        return FALSE;

    gpointer recently = g_object_get_data (G_OBJECT (vbox), "recently");

    InputTable *table = static_cast <InputTable *> (g_object_get_data (G_OBJECT (vbox), "table_pointer"));
    GtkWidget  *widget = (GtkWidget *) g_object_get_data (G_OBJECT (vbox), "table_widget");

    if (!table || table->number_of_elements () == 0) return FALSE;

    size_t start = 0;
    if (widget == 0) {
        GtkWidget *scroll;
        GtkWidget *subbox;
        GtkWidget *viewport;

        scroll = gtk_scrolled_window_new (NULL, NULL);
        gtk_widget_show (scroll);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
                                        GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
        gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scroll), 
                                             GTK_SHADOW_NONE);
        gtk_box_pack_start (GTK_BOX (vbox), scroll, TRUE, TRUE, 0);

        subbox = gtk_vbox_new (FALSE, 0);
        gtk_widget_show (subbox);

        viewport = gtk_viewport_new (NULL, NULL);
        gtk_widget_show (viewport);

        gtk_container_add (GTK_CONTAINER(scroll), viewport);
        gtk_container_add (GTK_CONTAINER(viewport), subbox);

        widget = create_table_page (0, table, 0, 1, recently);
        gtk_box_pack_start (GTK_BOX (subbox), widget, FALSE, FALSE, 0);

        g_object_set_data (G_OBJECT (vbox), "table_widget", (gpointer) widget);
        g_object_set_data (G_OBJECT (vbox), "table_viewport", (gpointer) viewport);
        g_object_set_data (G_OBJECT (vbox), "table_root_widget", (gpointer) scroll);
    } else {
        start = (size_t) g_object_get_data (G_OBJECT (vbox), "table_start");
        if (start < table->number_of_elements ())
            create_table_page (widget, table, start, 1, recently);
    }

    start ++;

    if (start >= table->number_of_elements ()) {
        set_table_vbox_width (vbox);
        g_object_set_data (G_OBJECT (vbox), "done", (gpointer) 1);
        return FALSE;
    } 

    g_object_set_data (G_OBJECT (vbox), "table_start", (gpointer) start);

    return TRUE;
}

static void
options_checkbox_toggled_callback (GtkToggleButton *togglebutton, gpointer user_data)
{
    GtkWidget *widget = GTK_WIDGET (user_data);

    gtk_widget_set_sensitive (widget, gtk_toggle_button_get_active (togglebutton));
}

static void
options_button_clicked_callback (GtkButton *button, gpointer user_data)
{
    GtkWidget *dialog;
    GtkWidget *show_preview_checkbox;
    GtkWidget *enable_repeat_checkbox;
    GtkWidget *repeat_delay_spin_button;
    GtkWidget *repeat_frequency_spin_button;
    GtkWidget *preview_size_spin_button;
    GtkWidget *preview_delay_spin_button;
    GtkWidget *clear_recently_used_checkbox;
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *separator;
 
    dialog = gtk_dialog_new_with_buttons (_("Options"), NULL,
                                          GTK_DIALOG_MODAL,
                                          GTK_STOCK_OK,
                                          GTK_RESPONSE_ACCEPT,
                                          GTK_STOCK_CANCEL,
                                          GTK_RESPONSE_REJECT,
                                          NULL);
    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

    // Show preview
    show_preview_checkbox = gtk_check_button_new_with_mnemonic (_("_Show Preview"));
    gtk_widget_show (show_preview_checkbox);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), show_preview_checkbox, FALSE, FALSE, 4);
    gtk_tooltips_set_tip (tooltips, show_preview_checkbox,
                          _("Whether to show preview of a character "
                            "in a popup window with larger size."
                            "If it's enabled, then a preview window will be showed "
                            "if mouse pointer is moved onto the button and stay for "
                            "a short while."), 0);

    // Preview size
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox, FALSE, FALSE, 4);

    label = gtk_label_new (NULL);
    gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _("_Preview Size (pt):"));
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 2);
    gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
    gtk_misc_set_padding (GTK_MISC (label), 2, 0);

    preview_size_spin_button = gtk_spin_button_new_with_range (10, 200, 2);
    gtk_widget_show (preview_size_spin_button);
    gtk_box_pack_start (GTK_BOX (hbox), preview_size_spin_button, FALSE, FALSE, 0);
    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (preview_size_spin_button), TRUE);
    gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (preview_size_spin_button), TRUE);
    gtk_spin_button_set_digits (GTK_SPIN_BUTTON (preview_size_spin_button), 0);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), preview_size_spin_button);
    gtk_tooltips_set_tip (tooltips, preview_size_spin_button,
                          _("The size of preview, in points."), 0);

    // Preview delay
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox, FALSE, FALSE, 4);

    label = gtk_label_new (NULL);
    gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _("Preview _Delay (ms):"));
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 2);
    gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
    gtk_misc_set_padding (GTK_MISC (label), 2, 0);

    preview_delay_spin_button = gtk_spin_button_new_with_range (100, 5000, 10);
    gtk_widget_show (preview_delay_spin_button);
    gtk_box_pack_start (GTK_BOX (hbox), preview_delay_spin_button, FALSE, FALSE, 0);
    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (preview_delay_spin_button), TRUE);
    gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (preview_delay_spin_button), TRUE);
    gtk_spin_button_set_digits (GTK_SPIN_BUTTON (preview_delay_spin_button), 0);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), preview_delay_spin_button);
    gtk_tooltips_set_tip (tooltips, preview_delay_spin_button,
                          _("The delay between moving mouse pointer onto "
                            "the button and showing preview. In milliseconds."), 0);

    separator = gtk_hseparator_new ();
    gtk_widget_show (separator);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), separator, FALSE, FALSE, 0);

    // Enable repeat
    enable_repeat_checkbox = gtk_check_button_new_with_mnemonic (_("_Enable Repeat"));
    gtk_widget_show (enable_repeat_checkbox);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), enable_repeat_checkbox, FALSE, FALSE, 4);
    gtk_tooltips_set_tip (tooltips, enable_repeat_checkbox,
                          _("Whether to enable repeatedly committing. "
                            "If it's enabled, then the character or key event "
                            "will be committed to client repeatedly if you hold "
                            "the button pressed."), 0);

    // Repeat delay
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox, FALSE, FALSE, 4);

    label = gtk_label_new (NULL);
    gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _("_Repeat Delay (ms):"));
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 2);
    gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
    gtk_misc_set_padding (GTK_MISC (label), 2, 0);

    repeat_delay_spin_button = gtk_spin_button_new_with_range (100, 1000, 10);
    gtk_widget_show (repeat_delay_spin_button);
    gtk_box_pack_start (GTK_BOX (hbox), repeat_delay_spin_button, FALSE, FALSE, 0);
    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (repeat_delay_spin_button), TRUE);
    gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (repeat_delay_spin_button), TRUE);
    gtk_spin_button_set_digits (GTK_SPIN_BUTTON (repeat_delay_spin_button), 0);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), repeat_delay_spin_button);
    gtk_tooltips_set_tip (tooltips, repeat_delay_spin_button,
                          _("The delay between pressing down the button "
                            "and starting repeatedly committing.In milliseconds."), 0);

    // Repeat frequency
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox, FALSE, FALSE, 4);

    label = gtk_label_new (NULL);
    gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _("Repeat _Frequency (times/sec):"));
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 2);
    gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
    gtk_misc_set_padding (GTK_MISC (label), 2, 0);

    repeat_frequency_spin_button = gtk_spin_button_new_with_range (1, 100, 1);
    gtk_widget_show (repeat_frequency_spin_button);
    gtk_box_pack_start (GTK_BOX (hbox), repeat_frequency_spin_button, FALSE, FALSE, 0);
    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (repeat_frequency_spin_button), TRUE);
    gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (repeat_frequency_spin_button), TRUE);
    gtk_spin_button_set_digits (GTK_SPIN_BUTTON (repeat_frequency_spin_button), 0);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), repeat_frequency_spin_button);
    gtk_tooltips_set_tip (tooltips, repeat_frequency_spin_button,
                          _("The frequency of repeatedly committing. In times/second."), 0);

    separator = gtk_hseparator_new ();
    gtk_widget_show (separator);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), separator, FALSE, FALSE, 0);

    // Show preview
    clear_recently_used_checkbox = gtk_check_button_new_with_mnemonic (_("_Clear Recently Used history"));
    gtk_widget_show (clear_recently_used_checkbox);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), clear_recently_used_checkbox, FALSE, FALSE, 4);
    gtk_tooltips_set_tip (tooltips, clear_recently_used_checkbox,
                          _("If it's checked, then the history of recently used characters and key events will be cleared."), 0);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_preview_checkbox), show_preview);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (preview_size_spin_button), preview_size);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (preview_delay_spin_button), preview_delay);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (enable_repeat_checkbox), enable_repeat);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (repeat_delay_spin_button), repeat_delay);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (repeat_frequency_spin_button), repeat_frequency);

    g_signal_connect (G_OBJECT(show_preview_checkbox), "toggled",
                      G_CALLBACK (options_checkbox_toggled_callback),
                      preview_size_spin_button);
    g_signal_connect (G_OBJECT(show_preview_checkbox), "toggled",
                      G_CALLBACK (options_checkbox_toggled_callback),
                      preview_delay_spin_button);

    g_signal_connect (G_OBJECT(enable_repeat_checkbox), "toggled",
                      G_CALLBACK (options_checkbox_toggled_callback),
                      repeat_delay_spin_button);
    g_signal_connect (G_OBJECT(enable_repeat_checkbox), "toggled",
                      G_CALLBACK (options_checkbox_toggled_callback),
                      repeat_frequency_spin_button);

    gint result = gtk_dialog_run (GTK_DIALOG (dialog));

    if (result == GTK_RESPONSE_ACCEPT) {
        show_preview = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (show_preview_checkbox));
        preview_size = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (preview_size_spin_button));
        preview_delay = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (preview_delay_spin_button));
        enable_repeat = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (enable_repeat_checkbox));
        repeat_delay = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (repeat_delay_spin_button));
        repeat_frequency = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (repeat_frequency_spin_button));

        // Clear recently used pages.
        if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (clear_recently_used_checkbox))) {
            recently_used_chars->delete_all_elements ();
            recently_used_keyevents->delete_all_elements ();

            GtkWidget  *widget;

            widget = (GtkWidget *) g_object_get_data (G_OBJECT (recently_used_chars_vbox), "table_root_widget");
            if (widget) gtk_widget_destroy (widget);
            g_object_set_data (G_OBJECT (recently_used_chars_vbox), "table_widget", 0);
            g_object_set_data (G_OBJECT (recently_used_chars_vbox), "table_viewport", 0);
            g_object_set_data (G_OBJECT (recently_used_chars_vbox), "table_root_widget", 0);
            g_object_set_data (G_OBJECT (recently_used_chars_vbox), "done", (gpointer) 0);

            widget = (GtkWidget *) g_object_get_data (G_OBJECT (recently_used_keyevents_vbox), "table_root_widget");
            if (widget) gtk_widget_destroy (widget);
            g_object_set_data (G_OBJECT (recently_used_keyevents_vbox), "table_widget", 0);
            g_object_set_data (G_OBJECT (recently_used_keyevents_vbox), "table_viewport", 0);
            g_object_set_data (G_OBJECT (recently_used_keyevents_vbox), "table_root_widget", 0);
            g_object_set_data (G_OBJECT (recently_used_keyevents_vbox), "done", (gpointer) 0);
        }
    }

    gtk_widget_destroy (dialog);
}

static gboolean
main_window_delete_callback (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    gtk_window_get_position (GTK_WINDOW (widget), &main_window_xpos, &main_window_ypos);
    return FALSE;
}

static void
create_main_window ()
{
    GtkWidget *vbox;
    GtkWidget *button;
    GtkWidget *separator;

    tooltips = gtk_tooltips_new ();

    main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_accept_focus (GTK_WINDOW (main_window), FALSE);
    g_signal_connect (G_OBJECT(main_window), "destroy", G_CALLBACK (gtk_main_quit), NULL);
    g_signal_connect (G_OBJECT(main_window), "delete_event", G_CALLBACK (main_window_delete_callback), NULL);

    vbox = gtk_vbox_new (FALSE, 4);
    gtk_widget_show (vbox);
    gtk_container_add(GTK_CONTAINER(main_window), vbox);

    button = gtk_button_new_with_label (_("Options"));
    g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (options_button_clicked_callback), 0);
    gtk_widget_show (button);
    gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 4);
    gtk_tooltips_set_tip (tooltips, button, _("Open the options dialog."), 0);

    separator = gtk_hseparator_new ();
    gtk_widget_show (separator);
    gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, FALSE, 0);


    main_notebook = create_group_notebook (input_groups);
    gtk_box_pack_start (GTK_BOX (vbox), main_notebook, TRUE, TRUE, 4);

    if (current_group >= gtk_notebook_get_n_pages (GTK_NOTEBOOK (main_notebook)))
        current_group = 0;

    GtkWidget *tablebook = gtk_notebook_get_nth_page (GTK_NOTEBOOK (main_notebook), current_group);

    if (current_table >= gtk_notebook_get_n_pages (GTK_NOTEBOOK (tablebook)))
        current_table = 0;

    gtk_notebook_set_current_page (GTK_NOTEBOOK (main_notebook),  current_group);
    gtk_notebook_set_current_page (GTK_NOTEBOOK (tablebook), current_table);

    table_notebook_switch_page_callback (GTK_NOTEBOOK (tablebook), 0, current_table, 0);

    GtkWidget *frame;
    preview_window = gtk_window_new (GTK_WINDOW_POPUP);
    preview_label  = gtk_label_new (0);
    gtk_label_set_justify (GTK_LABEL (preview_label), GTK_JUSTIFY_CENTER);
    gtk_widget_show (preview_label);
    frame = gtk_frame_new (0);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_widget_show (frame);
    gtk_container_add(GTK_CONTAINER(preview_window), frame);
    gtk_container_add(GTK_CONTAINER(frame), preview_label);

    gint scrh = gdk_screen_get_height (gtk_widget_get_screen (main_window));
    gint scrw = gdk_screen_get_width  (gtk_widget_get_screen (main_window));

    gtk_widget_set_size_request (GTK_WIDGET (main_window), -1, scrh / 2);

    GtkRequisition requisition;
    gtk_widget_size_request (GTK_WIDGET (main_window), &requisition);

    if (main_window_xpos < 0)
        main_window_xpos  =0;
    else  if(main_window_xpos > scrw - requisition.width)
        main_window_xpos = scrw - requisition.width;
                    
    if (main_window_ypos < 0)
        main_window_ypos  =0;
    else  if(main_window_ypos > scrh - requisition.height)
        main_window_ypos = scrh - requisition.height;

    gtk_window_move (GTK_WINDOW (main_window), main_window_xpos, main_window_ypos);
    gtk_widget_show (main_window);
}

static void
load_recently_used ()
{
    String fn = scim_get_home_dir () + String (SCIM_RECENTLY_USED_PAD_FILE);

    std::vector <InputGroupPointer> groups;

    if (load_input_group_file (fn, groups) > 0 &&
        groups [0]->get_name () == "Recently Used" &&
        groups [0]->number_of_tables () > 0) {
        InputTablePointer tb;
        for (size_t i = 0; i < groups [0]->number_of_tables (); ++i) {
            tb = groups [0]->get_table (i); 
            if (tb->get_name () == N_("Characters"))
                recently_used_chars = tb;
            else if (tb->get_name () == N_("KeyEvents"))
                recently_used_keyevents = tb;
        }
    }

    if (recently_used_chars.null ())
        recently_used_chars = new InputTable ("Characters", 5);
    if (recently_used_keyevents.null ())
        recently_used_keyevents = new InputTable ("KeyEvents", 1);
}

static void
save_recently_used ()
{
    String fn = scim_get_home_dir () + String (SCIM_RECENTLY_USED_PAD_FILE);

    InputGroupPointer grp = new InputGroup ("Recently Used");

    grp->append_table (recently_used_chars);
    grp->append_table (recently_used_keyevents);

    std::vector <InputGroupPointer> groups;

    groups.push_back (grp);

    save_input_group_file (fn, groups);
}

void run (const String &display)
{
    char **argv = new char * [4];
    int    argc = 3;

    argv [0] = "input-pad";
    argv [1] = "--display";
    argv [2] = const_cast<char *> (display.c_str ());
    argv [3] = 0;
 
    if (load_all_input_group_files (input_groups) == 0) return;

    setenv ("DISPLAY", display.c_str (), 1);

    gtk_init(&argc, &argv);
  
    load_recently_used ();

    create_main_window ();

    helper_agent.signal_connect_exit (slot (slot_exit));
	helper_agent.signal_connect_update_screen (slot (slot_update_screen));
    helper_agent.signal_connect_trigger_property (slot (slot_trigger_property));

    int fd = helper_agent.open_connection (helper_info, display);
    GIOChannel *ch = g_io_channel_unix_new (fd);

    if (fd >= 0 && ch) {
        Property prop ("/InputPad", _("Input Pad"), SCIM_INPUT_PAD_ICON, _("Show/Hide Input Pad."));
	    PropertyList props;

	    props.push_back (prop);

	    helper_agent.register_properties (props);

        g_io_add_watch (ch, G_IO_IN,  helper_agent_input_handler, (gpointer) &helper_agent);
        g_io_add_watch (ch, G_IO_ERR, helper_agent_input_handler, (gpointer) &helper_agent);
        g_io_add_watch (ch, G_IO_HUP, helper_agent_input_handler, (gpointer) &helper_agent);
    }

    gtk_main ();

    save_recently_used ();
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
