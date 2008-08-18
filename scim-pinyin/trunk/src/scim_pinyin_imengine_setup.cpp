/** @file scim_pinyin_imengine_setup.cpp
 * implementation of Setup Module of Pinyin IMEngine.
 */

/*
 * Smart Pinyin Input Method
 * 
 * Copyright (c) 2005 James Su <suzhe@tsinghua.org.cn>
 *
 * $Id: scim_pinyin_imengine_setup.cpp,v 1.3 2006/06/12 01:31:14 suzhe Exp $
 *
 */

#define Uses_SCIM_CONFIG_BASE

#include <gtk/gtk.h>

#include <scim.h>
#include <gtk/scimkeyselection.h>
#include "scim_pinyin_private.h"
#include "scim_pinyin.h"
#include "scim_pinyin_imengine_config_keys.h"

using namespace scim;

#define scim_module_init pinyin_imengine_setup_LTX_scim_module_init
#define scim_module_exit pinyin_imengine_setup_LTX_scim_module_exit

#define scim_setup_module_create_ui       pinyin_imengine_setup_LTX_scim_setup_module_create_ui
#define scim_setup_module_get_category    pinyin_imengine_setup_LTX_scim_setup_module_get_category
#define scim_setup_module_get_name        pinyin_imengine_setup_LTX_scim_setup_module_get_name
#define scim_setup_module_get_description pinyin_imengine_setup_LTX_scim_setup_module_get_description
#define scim_setup_module_load_config     pinyin_imengine_setup_LTX_scim_setup_module_load_config
#define scim_setup_module_save_config     pinyin_imengine_setup_LTX_scim_setup_module_save_config
#define scim_setup_module_query_changed   pinyin_imengine_setup_LTX_scim_setup_module_query_changed


static GtkWidget * __create_setup_window ();
static void        __load_config (const ConfigPointer &config);
static void        __save_config (const ConfigPointer &config);
static bool        __query_changed ();

// Module Interface.
extern "C" {
    void scim_module_init (void)
    {
        bindtextdomain (GETTEXT_PACKAGE, SCIM_PINYIN_LOCALEDIR);
        bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    }

    void scim_module_exit (void)
    {
    }

    GtkWidget * scim_setup_module_create_ui (void)
    {
        return __create_setup_window ();
    }

    String scim_setup_module_get_category (void)
    {
        return String ("IMEngine");
    }

    String scim_setup_module_get_name (void)
    {
        return String (_("Smart Pinyin"));
    }

    String scim_setup_module_get_description (void)
    {
        return String (_("An IMEngine module for Chinese which utilizes the pinyin input method."));
    }

    void scim_setup_module_load_config (const ConfigPointer &config)
    {
        __load_config (config);
    }

    void scim_setup_module_save_config (const ConfigPointer &config)
    {
        __save_config (config);
    }

    bool scim_setup_module_query_changed ()
    {
        return __query_changed ();
    }
} // extern "C"

// Internal data structure
struct KeyboardConfigData
{
    const char *key;
    const char *label;
    const char *title;
    const char *tooltip;
    GtkWidget  *entry;
    GtkWidget  *button;
    String      data;
};

// Internal data declaration.
static bool   __config_user_data_binary       = false;

static bool   __config_auto_combine_phrase    = true;
static bool   __config_auto_fill_preedit      = true;
static bool   __config_match_longer_phrase    = false;
static bool   __config_always_show_lookup     = true;
static bool   __config_show_all_keys          = false;
static bool   __config_dynamic_adjust         = true;

static int    __config_max_user_phrase_length = 8;
static int    __config_max_preedit_length     = 32;
static int    __config_smart_match_level      = 20;
static int    __config_burst_stack_size       = 128;
static int    __config_dynamic_sensitivity    = 6;
static int    __config_save_period            = 300;

static bool   __config_tone                   = false;
static bool   __config_incomplete             = true;

static bool   __config_ambiguities [SCIM_PINYIN_AmbLast+1] =
{
    false,
    false, false, false,
    false, false, false,
    false, false, false
};

static char * __ambiguity_config_keys [SCIM_PINYIN_AmbLast+1] =
{
    SCIM_CONFIG_IMENGINE_PINYIN_AMBIGUITY_ANY,
    SCIM_CONFIG_IMENGINE_PINYIN_AMBIGUITY_ZhiZi,
    SCIM_CONFIG_IMENGINE_PINYIN_AMBIGUITY_ChiCi,
    SCIM_CONFIG_IMENGINE_PINYIN_AMBIGUITY_ShiSi,
    SCIM_CONFIG_IMENGINE_PINYIN_AMBIGUITY_NeLe,
    SCIM_CONFIG_IMENGINE_PINYIN_AMBIGUITY_LeRi,
    SCIM_CONFIG_IMENGINE_PINYIN_AMBIGUITY_FoHe,
    SCIM_CONFIG_IMENGINE_PINYIN_AMBIGUITY_AnAng,
    SCIM_CONFIG_IMENGINE_PINYIN_AMBIGUITY_EnEng,
    SCIM_CONFIG_IMENGINE_PINYIN_AMBIGUITY_InIng
};

static char * __ambiguity_ui_strings [SCIM_PINYIN_AmbLast+1] =
{
    N_("A_mbiguities"),
    N_("_Zh and Z"),
    N_("_Ch and C"),
    N_("_Sh and S"),
    N_("_N and L"),
    N_("_L and R"),
    N_("_F and H"),
    N_("_An and Ang"),
    N_("_En and Eng"),
    N_("_In and Ing")
};

static bool __have_changed                 = false;

static KeyboardConfigData __config_keyboards [] =
{
    {
        // key
        SCIM_CONFIG_IMENGINE_PINYIN_FULL_WIDTH_PUNCT_KEY,
        // label
        N_("Full width _punctuation:"),
        // title
        N_("Select full width puncutation keys"),
        // tooltip
        N_("Key events to switch full/half width punctuation input mode. "
           "Click on the right button edit."),
        // entry
        NULL,
        // button
        NULL,
        // data
        "Control+period"
    },
    {
        // key
        SCIM_CONFIG_IMENGINE_PINYIN_FULL_WIDTH_LETTER_KEY,
        // label
        N_("Full width _letter:"),
        // title
        N_("Select full width letter keys"),
        // tooltip
        N_("Key events to switch full/half width letter input mode. "
           "Click on the right button edit."),
        // entry
        NULL,
        // button
        NULL,
        // data
        "Shift+space"
    },
    {
        // key
        SCIM_CONFIG_IMENGINE_PINYIN_MODE_SWITCH_KEY,
        // label
        N_("_Mode switch:"),
        // title
        N_("Select mode switch keys"),
        // tooltip
        N_("Key events to change the current input mode. "
           "Click on the button on the right to edit it."),
        // entry
        NULL,
        // button
        NULL,
        // data
        "Alt+Shift_L+KeyRelease,"
        "Alt+Shift_R+KeyRelease,"
        "Shift+Shift_L+KeyRelease,"
        "Shift+Shift_R+KeyRelease"
    },
    {
        // key
        SCIM_CONFIG_IMENGINE_PINYIN_CHINESE_SWITCH_KEY,
        // label
        N_("_Chinese mode switch:"),
        // title
        N_("Select Chinese mode switch keys"),
        // tooltip
        N_("Key events to change the current Chinese input mode. "
           "Click on the right button edit."),
        // entry
        NULL,
        // button
        NULL,
        // data
        "Control+slash"
    },
    {
        // key
        SCIM_CONFIG_IMENGINE_PINYIN_PAGE_UP_KEY,
        // label
        N_("Page _up:"),
        // title
        N_("Select page up keys"),
        // tooltip
        N_("Key events to page up the lookup table. "
           "Click on the right button edit."),
        // entry
        NULL,
        // button
        NULL,
        // data
        "comma,minus,bracketleft,Page_Up"
    },
    {
        // key
        SCIM_CONFIG_IMENGINE_PINYIN_PAGE_DOWN_KEY,
        // label
        N_("Page dow_n:"),
        // title
        N_("Select page down keys"),
        // tooltip
        N_("Key events to page down the lookup table. "
           "Click on the right button edit."),
        // entry
        NULL,
        // button
        NULL,
        // data
        "period,equal,bracketright,Page_Down"
    },
    {
        // key
        SCIM_CONFIG_IMENGINE_PINYIN_DISABLE_PHRASE_KEY,
        // label
        N_("_Disable phrase:"),
        // title
        N_("Select disable phrase keys"),
        // tooltip
        N_("Key events to disable the currently selected user defined phrase. "
           "Click on the right button edit."),
        // entry
        NULL,
        // button
        NULL,
        // data
        "Control+d"
    },
    {
        // key
        NULL,
        // label
        NULL,
        // title
        NULL,
        // tooltip
        NULL,
        // entry
        NULL,
        // button
        NULL,
        // data
        ""
    },
};

static GtkWidget * __widget_user_data_binary       = 0;

static GtkWidget * __widget_auto_combine_phrase    = 0;
static GtkWidget * __widget_auto_fill_preedit      = 0;
static GtkWidget * __widget_match_longer_phrase    = 0;
static GtkWidget * __widget_always_show_lookup     = 0;
static GtkWidget * __widget_show_all_keys          = 0;
static GtkWidget * __widget_dynamic_adjust         = 0;

static GtkWidget * __widget_max_user_phrase_length = 0;
static GtkWidget * __widget_max_preedit_length     = 0;
static GtkWidget * __widget_smart_match_level      = 0;
static GtkWidget * __widget_burst_stack_size       = 0;
static GtkWidget * __widget_dynamic_sensitivity    = 0;
static GtkWidget * __widget_save_period            = 0;

static GtkWidget * __widget_tone                   = 0;
static GtkWidget * __widget_incomplete             = 0;

static GtkWidget * __widget_ambiguities [SCIM_PINYIN_AmbLast+1] = 
{
    NULL,
    NULL, NULL, NULL,
    NULL, NULL, NULL,
    NULL, NULL, NULL
};

static GtkTooltips * __widget_tooltips = 0;

// Common callbacks
static void
__on_default_editable_changed          (GtkEditable     *editable,
                                        gpointer         user_data);

static void
__on_default_toggle_button_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

static void
__on_default_spin_button_changed       (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

static void
__on_default_key_selection_clicked     (GtkButton       *button,
                                        gpointer         user_data);


// Declaration of internal functions.
static void
__on_auto_combine_phrase_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

static void
__on_auto_fill_preedit_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

static void
__on_dynamic_adjust_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

static void
__on_ambiguities_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

static void
__setup_widget_value ();

// Function implementations.
static GtkWidget *
__create_setup_window ()
{
    static GtkWidget *window = 0;

    if (!window) {
        int i;
        GtkWidget *notebook;
        GtkWidget *table;
        GtkWidget *frame;
        GtkWidget *label;
        GtkWidget *vbox;
        GtkWidget *hbox;
        GtkWidget *separator;

        __widget_tooltips = gtk_tooltips_new ();

        // Create the Notebook.
        notebook = gtk_notebook_new ();
        gtk_widget_show (notebook);

        // Create the input configuration page.
        vbox = gtk_vbox_new (FALSE, 0);
        gtk_widget_show (vbox);
        gtk_container_add (GTK_CONTAINER (notebook), vbox);

        // Create the label for this note page.
        label = gtk_label_new (_("Input"));
        gtk_widget_show (label);
        gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), 0), label);

        // Create The table to hold the configurations.
        table = gtk_table_new (6, 3, FALSE);
        gtk_widget_show (table);
        gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);

        // Auto combine phrase
        __widget_auto_combine_phrase = gtk_check_button_new_with_mnemonic (_("Auto _combine phrase"));
        gtk_widget_show (__widget_auto_combine_phrase);
        gtk_table_attach (GTK_TABLE (table), __widget_auto_combine_phrase, 0, 1, 0, 1,
                            (GtkAttachOptions) (GTK_FILL),
                            (GtkAttachOptions) (0), 0, 0);
        gtk_container_set_border_width (GTK_CONTAINER (__widget_auto_combine_phrase), 2);

        // Auto fill preedit
        __widget_auto_fill_preedit = gtk_check_button_new_with_mnemonic (_("Auto _fill preedit"));
        gtk_widget_show (__widget_auto_fill_preedit);
        gtk_table_attach (GTK_TABLE (table), __widget_auto_fill_preedit, 0, 1, 1, 2,
                            (GtkAttachOptions) (GTK_FILL),
                            (GtkAttachOptions) (0), 0, 0);
        gtk_container_set_border_width (GTK_CONTAINER (__widget_auto_fill_preedit), 2);

        // Match longer phrase
        __widget_match_longer_phrase = gtk_check_button_new_with_mnemonic (_("_Match longer phrase"));
        gtk_widget_show (__widget_match_longer_phrase);
        gtk_table_attach (GTK_TABLE (table), __widget_match_longer_phrase, 0, 1, 2, 3,
                            (GtkAttachOptions) (GTK_FILL),
                            (GtkAttachOptions) (0), 0, 0);
        gtk_container_set_border_width (GTK_CONTAINER (__widget_match_longer_phrase), 2);

        // Always show lookup
        __widget_always_show_lookup = gtk_check_button_new_with_mnemonic (_("Always show lookup _table"));
        gtk_widget_show (__widget_always_show_lookup);
        gtk_table_attach (GTK_TABLE (table), __widget_always_show_lookup, 0, 1, 3, 4,
                            (GtkAttachOptions) (GTK_FILL),
                            (GtkAttachOptions) (0), 0, 0);
        gtk_container_set_border_width (GTK_CONTAINER (__widget_always_show_lookup), 2);

        // Show all keys
        __widget_show_all_keys = gtk_check_button_new_with_mnemonic (_("Show all _keys"));
        gtk_widget_show (__widget_show_all_keys);
        gtk_table_attach (GTK_TABLE (table), __widget_show_all_keys, 0, 1, 4, 5,
                            (GtkAttachOptions) (GTK_FILL),
                            (GtkAttachOptions) (0), 0, 0);
        gtk_container_set_border_width (GTK_CONTAINER (__widget_show_all_keys), 2);

        // Dynamic adjust
        __widget_dynamic_adjust = gtk_check_button_new_with_mnemonic (_("_Dynamic adjust"));
        gtk_widget_show (__widget_dynamic_adjust);
        gtk_table_attach (GTK_TABLE (table), __widget_dynamic_adjust, 0, 1, 5, 6,
                            (GtkAttachOptions) (GTK_FILL),
                            (GtkAttachOptions) (0), 0, 0);
        gtk_container_set_border_width (GTK_CONTAINER (__widget_dynamic_adjust), 2);

        // Max user phrase length
        hbox = gtk_hbox_new (FALSE, 0);
        gtk_widget_show (hbox);
        gtk_table_attach (GTK_TABLE (table), hbox, 2, 3, 0, 1,
                            (GtkAttachOptions) (GTK_FILL),
                            (GtkAttachOptions) (0), 0, 0);

        label = gtk_label_new (NULL);
        gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _("Max _user phrase length:"));
        gtk_widget_show (label);
        gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 2);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
        gtk_misc_set_padding (GTK_MISC (label), 2, 0);

        __widget_max_user_phrase_length = gtk_spin_button_new_with_range (2, 15, 1);
        gtk_widget_show (__widget_max_user_phrase_length);
        gtk_box_pack_start (GTK_BOX (hbox), __widget_max_user_phrase_length, FALSE, FALSE, 0);
        gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (__widget_max_user_phrase_length), TRUE);
        gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (__widget_max_user_phrase_length), TRUE);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON (__widget_max_user_phrase_length), 0);
        gtk_label_set_mnemonic_widget (GTK_LABEL (label), __widget_max_user_phrase_length);

        // Max preedit length
        hbox = gtk_hbox_new (FALSE, 0);
        gtk_widget_show (hbox);
        gtk_table_attach (GTK_TABLE (table), hbox, 2, 3, 1, 2,
                            (GtkAttachOptions) (GTK_FILL),
                            (GtkAttachOptions) (0), 0, 0);

        label = gtk_label_new (NULL);
        gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _("Max _preedit length:"));
        gtk_widget_show (label);
        gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 2);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
        gtk_misc_set_padding (GTK_MISC (label), 2, 0);

        __widget_max_preedit_length = gtk_spin_button_new_with_range (4, 80, 1);
        gtk_widget_show (__widget_max_preedit_length);
        gtk_box_pack_start (GTK_BOX (hbox), __widget_max_preedit_length, FALSE, FALSE, 0);
        gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (__widget_max_preedit_length), TRUE);
        gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (__widget_max_preedit_length), TRUE);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON (__widget_max_preedit_length), 0);
        gtk_label_set_mnemonic_widget (GTK_LABEL (label), __widget_max_preedit_length);

        // Smart match level
        hbox = gtk_hbox_new (FALSE, 0);
        gtk_widget_show (hbox);
        gtk_table_attach (GTK_TABLE (table), hbox, 2, 3, 2, 3,
                            (GtkAttachOptions) (GTK_FILL),
                            (GtkAttachOptions) (0), 0, 0);

        label = gtk_label_new (NULL);
        gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _("_Smart match level:"));
        gtk_widget_show (label);
        gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 2);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
        gtk_misc_set_padding (GTK_MISC (label), 2, 0);

        __widget_smart_match_level = gtk_spin_button_new_with_range (1, 100, 1);
        gtk_widget_show (__widget_smart_match_level);
        gtk_box_pack_start (GTK_BOX (hbox), __widget_smart_match_level, FALSE, FALSE, 0);
        gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (__widget_smart_match_level), TRUE);
        gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (__widget_smart_match_level), TRUE);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON (__widget_smart_match_level), 0);
        gtk_label_set_mnemonic_widget (GTK_LABEL (label), __widget_smart_match_level);

        // Burst stack size
        hbox = gtk_hbox_new (FALSE, 0);
        gtk_widget_show (hbox);
        gtk_table_attach (GTK_TABLE (table), hbox, 2, 3, 3, 4,
                            (GtkAttachOptions) (GTK_FILL),
                            (GtkAttachOptions) (0), 0, 0);

        label = gtk_label_new (NULL);
        gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _("_Burst stack size:"));
        gtk_widget_show (label);
        gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 2);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
        gtk_misc_set_padding (GTK_MISC (label), 2, 0);

        __widget_burst_stack_size = gtk_spin_button_new_with_range (0, 255, 1);
        gtk_widget_show (__widget_burst_stack_size);
        gtk_box_pack_start (GTK_BOX (hbox), __widget_burst_stack_size, FALSE, FALSE, 0);
        gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (__widget_burst_stack_size), TRUE);
        gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (__widget_burst_stack_size), TRUE);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON (__widget_burst_stack_size), 0);
        gtk_label_set_mnemonic_widget (GTK_LABEL (label), __widget_burst_stack_size);

        // Dynamic sensitivity
        hbox = gtk_hbox_new (FALSE, 0);
        gtk_widget_show (hbox);
        gtk_table_attach (GTK_TABLE (table), hbox, 2, 3, 4, 5,
                            (GtkAttachOptions) (GTK_FILL),
                            (GtkAttachOptions) (0), 0, 0);

        label = gtk_label_new (NULL);
        gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _("D_ynamic sensitivity:"));
        gtk_widget_show (label);
        gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 2);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
        gtk_misc_set_padding (GTK_MISC (label), 2, 0);

        __widget_dynamic_sensitivity = gtk_spin_button_new_with_range (0, 16, 1);
        gtk_widget_show (__widget_dynamic_sensitivity);
        gtk_box_pack_start (GTK_BOX (hbox), __widget_dynamic_sensitivity, FALSE, FALSE, 0);
        gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (__widget_dynamic_sensitivity), TRUE);
        gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (__widget_dynamic_sensitivity), TRUE);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON (__widget_dynamic_sensitivity), 0);
        gtk_label_set_mnemonic_widget (GTK_LABEL (label), __widget_dynamic_sensitivity);

        // Save period
        hbox = gtk_hbox_new (FALSE, 0);
        gtk_widget_show (hbox);
        gtk_table_attach (GTK_TABLE (table), hbox, 2, 3, 5, 6,
                            (GtkAttachOptions) (GTK_FILL),
                            (GtkAttachOptions) (0), 0, 0);

        label = gtk_label_new (NULL);
        gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _("Sa_ve period (s):"));
        gtk_widget_show (label);
        gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 2);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
        gtk_misc_set_padding (GTK_MISC (label), 2, 0);

        __widget_save_period = gtk_spin_button_new_with_range (30, 3600, 10);
        gtk_widget_show (__widget_save_period);
        gtk_box_pack_start (GTK_BOX (hbox), __widget_save_period, FALSE, FALSE, 0);
        gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (__widget_save_period), TRUE);
        gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (__widget_save_period), TRUE);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON (__widget_save_period), 0);
        gtk_label_set_mnemonic_widget (GTK_LABEL (label), __widget_save_period);

        separator = gtk_vseparator_new ();
        gtk_widget_show (separator);

        gtk_table_attach (GTK_TABLE (table), separator, 1, 2, 0, 6,
                            (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                            (GtkAttachOptions) (GTK_FILL), 8, 0);

        separator = gtk_hseparator_new ();
        gtk_widget_show (separator);
        gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, FALSE, 4);

        // User data binary
        __widget_user_data_binary = gtk_check_button_new_with_mnemonic (_("Save user data in binary _format"));
        gtk_widget_show (__widget_user_data_binary);
        gtk_box_pack_start (GTK_BOX (vbox), __widget_user_data_binary, FALSE, FALSE, 4);
        gtk_container_set_border_width (GTK_CONTAINER (__widget_user_data_binary), 2);

        separator = gtk_hseparator_new ();
        gtk_widget_show (separator);
        gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, FALSE, 4);

        // Create the pinyin configurations page
        vbox = gtk_vbox_new (FALSE, 0);
        gtk_widget_show (vbox);
        gtk_container_add (GTK_CONTAINER (notebook), vbox);

        // Create the label for this note page.
        label = gtk_label_new (_("Pinyin"));
        gtk_widget_show (label);
        gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), 1), label);

        hbox = gtk_hbox_new (FALSE, 0);
        gtk_widget_show (hbox);
        gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 4);

        // Use tone
        __widget_tone = gtk_check_button_new_with_mnemonic (_("_Use tone"));
        gtk_widget_show (__widget_tone);
        gtk_box_pack_start (GTK_BOX (hbox), __widget_tone, TRUE, TRUE, 4);
        gtk_container_set_border_width (GTK_CONTAINER (__widget_tone), 2);

        __widget_incomplete = gtk_check_button_new_with_mnemonic (_("Allow incomplete _pinyin"));
        gtk_widget_show (__widget_incomplete);
        gtk_box_pack_start (GTK_BOX (hbox), __widget_incomplete, TRUE, TRUE, 4);
        gtk_container_set_border_width (GTK_CONTAINER (__widget_incomplete), 2);

        separator = gtk_hseparator_new ();
        gtk_widget_show (separator);
        gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, FALSE, 4);

        // Ambiguities
        frame = gtk_frame_new (NULL);
        gtk_widget_show (frame);
        gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 2);
        gtk_container_set_border_width (GTK_CONTAINER (frame), 4);
        gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);

        __widget_ambiguities [0] = gtk_check_button_new_with_mnemonic (_(__ambiguity_ui_strings [0]));
        gtk_widget_show (__widget_ambiguities [0]);
        gtk_frame_set_label_widget (GTK_FRAME (frame), __widget_ambiguities [0]);

        table = gtk_table_new (3, 3, FALSE);
        gtk_widget_show (table);
        gtk_container_add (GTK_CONTAINER (frame), table);

        for (i=1; i <= SCIM_PINYIN_AmbLast; ++i) {
            __widget_ambiguities [i] = gtk_check_button_new_with_mnemonic (_(__ambiguity_ui_strings [i]));
            gtk_widget_show (__widget_ambiguities [i]);
            gtk_table_attach (GTK_TABLE (table), __widget_ambiguities [i],
                                (i-1) / 3, ((i-1) / 3) + 1, (i-1) % 3, ((i-1) % 3) + 1,
                                (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                                (GtkAttachOptions) (0), 4, 8);
            gtk_container_set_border_width (GTK_CONTAINER (__widget_ambiguities [i]), 2);
        }

        // Create the keyboard configurations page
        table = gtk_table_new (7, 3, FALSE);
        gtk_widget_show (table);
        gtk_container_add (GTK_CONTAINER (notebook), table);

        // Create the label for this note page.
        label = gtk_label_new (_("Keyboard"));
        gtk_widget_show (label);
        gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), 2), label);

        for (i = 0; __config_keyboards [i].key; ++ i) {
            label = gtk_label_new (NULL);
            gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _(__config_keyboards[i].label));
            gtk_widget_show (label);
            gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
                                (GtkAttachOptions) (GTK_FILL),
                                (GtkAttachOptions) (0), 4, 8);
            gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);

            __config_keyboards[i].entry = gtk_entry_new ();
            gtk_widget_show (__config_keyboards[i].entry);
            gtk_table_attach (GTK_TABLE (table), __config_keyboards[i].entry, 1, 2, i, i+1,
                                (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                                (GtkAttachOptions) (0), 0, 0);
            gtk_entry_set_editable (GTK_ENTRY (__config_keyboards[i].entry), FALSE);

            __config_keyboards[i].button = gtk_button_new_with_label ("...");
            gtk_widget_show (__config_keyboards[i].button);
            gtk_table_attach (GTK_TABLE (table), __config_keyboards[i].button, 2, 3, i, i+1,
                              (GtkAttachOptions) (GTK_FILL),
                              (GtkAttachOptions) (GTK_FILL), 4, 4);
            gtk_label_set_mnemonic_widget (GTK_LABEL (label), __config_keyboards[i].button);

        }

        // Connect all signals.
        g_signal_connect ((gpointer) __widget_auto_combine_phrase, "toggled",
                          G_CALLBACK (__on_auto_combine_phrase_toggled),
                          NULL);

        g_signal_connect ((gpointer) __widget_auto_fill_preedit, "toggled",
                          G_CALLBACK (__on_auto_fill_preedit_toggled),
                          NULL);

        g_signal_connect ((gpointer) __widget_match_longer_phrase, "toggled",
                          G_CALLBACK (__on_default_toggle_button_toggled),
                          &__config_match_longer_phrase);

        g_signal_connect ((gpointer) __widget_always_show_lookup, "toggled",
                          G_CALLBACK (__on_default_toggle_button_toggled),
                          &__config_always_show_lookup);

        g_signal_connect ((gpointer) __widget_show_all_keys, "toggled",
                          G_CALLBACK (__on_default_toggle_button_toggled),
                          &__config_show_all_keys);

        g_signal_connect ((gpointer) __widget_dynamic_adjust, "toggled",
                          G_CALLBACK (__on_dynamic_adjust_toggled),
                          NULL);

        g_signal_connect ((gpointer) __widget_max_user_phrase_length, "value_changed",
                          G_CALLBACK (__on_default_spin_button_changed),
                          &__config_max_user_phrase_length);

        g_signal_connect ((gpointer) __widget_max_preedit_length, "value_changed",
                          G_CALLBACK (__on_default_spin_button_changed),
                          &__config_max_preedit_length);

        g_signal_connect ((gpointer) __widget_smart_match_level, "value_changed",
                          G_CALLBACK (__on_default_spin_button_changed),
                          &__config_smart_match_level);

        g_signal_connect ((gpointer) __widget_burst_stack_size, "value_changed",
                          G_CALLBACK (__on_default_spin_button_changed),
                          &__config_burst_stack_size);

        g_signal_connect ((gpointer) __widget_dynamic_sensitivity, "value_changed",
                          G_CALLBACK (__on_default_spin_button_changed),
                          &__config_dynamic_sensitivity);

        g_signal_connect ((gpointer) __widget_save_period, "value_changed",
                          G_CALLBACK (__on_default_spin_button_changed),
                          &__config_save_period);

        g_signal_connect ((gpointer) __widget_user_data_binary, "toggled",
                          G_CALLBACK (__on_default_toggle_button_toggled),
                          &__config_user_data_binary);

        g_signal_connect ((gpointer) __widget_tone, "toggled",
                          G_CALLBACK (__on_default_toggle_button_toggled),
                          &__config_tone);

        g_signal_connect ((gpointer) __widget_incomplete, "toggled",
                          G_CALLBACK (__on_default_toggle_button_toggled),
                          &__config_incomplete);

        for (i = 0; i <= SCIM_PINYIN_AmbLast; ++i) {
            g_signal_connect ((gpointer) __widget_ambiguities [i], "toggled",
                                G_CALLBACK (__on_ambiguities_toggled),
                                (gpointer) i);
        }

        for (i = 0; __config_keyboards [i].key; ++ i) {
            g_signal_connect ((gpointer) __config_keyboards [i].button, "clicked",
                              G_CALLBACK (__on_default_key_selection_clicked),
                              &(__config_keyboards [i]));
            g_signal_connect ((gpointer) __config_keyboards [i].entry, "changed",
                              G_CALLBACK (__on_default_editable_changed),
                              &(__config_keyboards [i].data));
        }


        // Set all tooltips.
        gtk_tooltips_set_tip (__widget_tooltips, __widget_user_data_binary,
                              _("Store the user pinyin and phrase data in binary format, "
                                "this will increase the loading speed."), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_auto_combine_phrase,
                              _("Combine inputed phrases into one longer phrase automatically."), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_auto_fill_preedit,
                              _("Fill the preedit string automatically "
                                "by a smart matching algorithm."), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_match_longer_phrase,
                              _("Phrases which are longer than the inputed keys "
                                "can also be matched, this option is valid when "
                                "\"Auto combine phrase\" is checked."), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_always_show_lookup,
                              _("Lookup table will be always shown "
                                "when there are any candidate phrases."), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_show_all_keys,
                              _("All inputed keys will be shown, "
                                "this option is only valid when "
                                "\"Auto fill preedit\" is checked."), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_dynamic_adjust,
                              _("Phrase library will be adjusted dynamically "
                                "according to the inputed contents. "
                                "The changed part of the phrase library will be "
                                "stored in the user's local files."), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_max_user_phrase_length,
                              _("Maxmium length of the custom phrases created "
                                "by user. These phrases will be created automatically "
                                "when user inputs text."), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_max_preedit_length,
                              _("Maxmium length of the preedit string."), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_smart_match_level,
                              _("Level of the smart matching algorithm. "
                                "The larger the level the more accurate "
                                "the algorithm, but also slower."), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_burst_stack_size,
                              _("Size of the burst stack. "
                                "Newly inputed phrases will be placed onto the "
                                "burst stack. The phrases in this stack "
                                "have higher priority than others."), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_dynamic_sensitivity,
                              _("Sensitivity of the dynamic adjusting algorithm, "
                                "the higher the more sensitive."), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_save_period,
                              _("Time period, in seconds, to "
                                "save the user data."), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_tone,
                              _("Tone information will be used "
                                "in matching the pinyin key."), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_incomplete,
                              _("Pinyin keys which only have the initial part (Sheng Mu) "
                                "will be allowed. "), NULL);

        gtk_tooltips_set_tip (__widget_tooltips, __widget_ambiguities [0],
                              _("The following options control the ambiguous "
                                "behaviour of the pinyin matching algorithm, "
                                "useful if the user cannot distinguish between them."), NULL);

        for (i = 0; __config_keyboards [i].key; ++ i) {
            gtk_tooltips_set_tip (__widget_tooltips, __config_keyboards [i].entry,
                                  _(__config_keyboards [i].tooltip), NULL);
        }

        window = notebook;

        __setup_widget_value ();
    }

    return window;
}

static void
__setup_widget_value ()
{
    int i;

    if (__widget_auto_combine_phrase) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_auto_combine_phrase),
            __config_auto_combine_phrase);
    }

    if (__widget_auto_fill_preedit) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_auto_fill_preedit),
            __config_auto_fill_preedit);

    }

    if (__widget_match_longer_phrase) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_match_longer_phrase),
            __config_match_longer_phrase);

        gtk_widget_set_sensitive (
            __widget_match_longer_phrase,
            !__config_auto_combine_phrase);
    }

    if (__widget_always_show_lookup) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_always_show_lookup),
            __config_always_show_lookup);

        gtk_widget_set_sensitive (
            __widget_always_show_lookup,
            __config_auto_fill_preedit);
    }

    if (__widget_show_all_keys) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_show_all_keys),
            __config_show_all_keys);

        gtk_widget_set_sensitive (
            __widget_show_all_keys,
            __config_auto_fill_preedit);
    }

    if (__widget_dynamic_adjust) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_dynamic_adjust),
            __config_dynamic_adjust);
    }

    if (__widget_max_user_phrase_length) {
        gtk_spin_button_set_value (
            GTK_SPIN_BUTTON (__widget_max_user_phrase_length),
            __config_max_user_phrase_length);
    }

    if (__widget_max_preedit_length) {
        gtk_spin_button_set_value (
            GTK_SPIN_BUTTON (__widget_max_preedit_length),
            __config_max_preedit_length);
    }

    if (__widget_smart_match_level) {
        gtk_spin_button_set_value (
            GTK_SPIN_BUTTON (__widget_smart_match_level),
            __config_smart_match_level);

        gtk_widget_set_sensitive (
            __widget_smart_match_level,
            __config_auto_fill_preedit | __config_auto_combine_phrase);
    }

    if (__widget_burst_stack_size) {
        gtk_spin_button_set_value (
            GTK_SPIN_BUTTON (__widget_burst_stack_size),
            __config_burst_stack_size);

        gtk_widget_set_sensitive (
            __widget_burst_stack_size,
            __config_dynamic_adjust);
    }

    if (__widget_dynamic_sensitivity) {
        gtk_spin_button_set_value (
            GTK_SPIN_BUTTON (__widget_dynamic_sensitivity),
            __config_dynamic_sensitivity);

        gtk_widget_set_sensitive (
            __widget_dynamic_sensitivity,
            __config_dynamic_adjust);
    }

    if (__widget_save_period) {
        gtk_spin_button_set_value (
            GTK_SPIN_BUTTON (__widget_save_period),
            __config_save_period);

        gtk_widget_set_sensitive (
            __widget_save_period,
            __config_dynamic_adjust);
    }

    if (__widget_user_data_binary) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_user_data_binary),
            __config_user_data_binary);
    }

    for (i = 0; __config_keyboards [i].key; ++ i) {
        if (__config_keyboards [i].entry) {
            gtk_entry_set_text (
                GTK_ENTRY (__config_keyboards [i].entry),
                __config_keyboards [i].data.c_str ());
        }
    }


    if (__widget_tone) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_tone),
            __config_tone);
    }

    if (__widget_incomplete) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_incomplete),
            __config_incomplete);
    }

    for (i = 0; i <= SCIM_PINYIN_AmbLast; ++ i) {
        if (__widget_ambiguities [i]) {
            gtk_toggle_button_set_active (
                GTK_TOGGLE_BUTTON (__widget_ambiguities [i]),
                __config_ambiguities [i]);

            if (i > 0) {
                gtk_widget_set_sensitive (
                    __widget_ambiguities [i],
                    __config_ambiguities [0]);
            }
        }
    }
}

static void
__load_config (const ConfigPointer &config)
{
    if (!config.null ()) {
        int i;

        __config_auto_combine_phrase =
            config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_AUTO_COMBINE_PHRASE),
                          __config_auto_combine_phrase);
        __config_auto_fill_preedit =
            config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_AUTO_FILL_PREEDIT),
                          __config_auto_fill_preedit);
        __config_match_longer_phrase =
            config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_MATCH_LONGER_PHRASE),
                          __config_match_longer_phrase);
        __config_always_show_lookup =
            config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_ALWAYS_SHOW_LOOKUP),
                          __config_always_show_lookup);
        __config_show_all_keys =
            config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_SHOW_ALL_KEYS),
                          __config_show_all_keys);
        __config_dynamic_adjust =
            config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_DYNAMIC_ADJUST),
                          __config_dynamic_adjust);
        __config_user_data_binary=
            config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_USER_DATA_BINARY),
                          __config_user_data_binary);
        __config_max_user_phrase_length =
            config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_MAX_USER_PHRASE_LENGTH),
                          __config_max_user_phrase_length);
        __config_max_preedit_length=
            config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_MAX_PREEDIT_LENGTH),
                          __config_max_preedit_length);
        __config_smart_match_level=
            config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_SMART_MATCH_LEVEL),
                          __config_smart_match_level);
        __config_burst_stack_size=
            config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_BURST_STACK_SIZE),
                          __config_burst_stack_size);
        __config_dynamic_sensitivity =
            config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_DYNAMIC_SENSITIVITY),
                          __config_dynamic_sensitivity);
        __config_save_period =
            config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_SAVE_PERIOD),
                          __config_save_period);

        for (i = 0; __config_keyboards [i].key; ++ i) {
            __config_keyboards [i].data =
                config->read (String (__config_keyboards [i].key),
                              __config_keyboards [i].data);
        }

        __config_tone =
            config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_TONE),
                          __config_tone);

        __config_incomplete =
            config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_INCOMPLETE),
                          __config_incomplete);

        for (i = 0; i <= SCIM_PINYIN_AmbLast; ++ i) {
            __config_ambiguities [i] = 
                config->read (__ambiguity_config_keys [i], __config_ambiguities [i]);
        }

        __setup_widget_value ();
        __have_changed = false;
    }
}

static void
__save_config (const ConfigPointer &config)
{
    if (!config.null ()) {
        int i;

        config->write (String (SCIM_CONFIG_IMENGINE_PINYIN_AUTO_COMBINE_PHRASE),
                      __config_auto_combine_phrase);
        config->write (String (SCIM_CONFIG_IMENGINE_PINYIN_AUTO_FILL_PREEDIT),
                      __config_auto_fill_preedit);
        config->write (String (SCIM_CONFIG_IMENGINE_PINYIN_MATCH_LONGER_PHRASE),
                      __config_match_longer_phrase);
        config->write (String (SCIM_CONFIG_IMENGINE_PINYIN_ALWAYS_SHOW_LOOKUP),
                      __config_always_show_lookup);
        config->write (String (SCIM_CONFIG_IMENGINE_PINYIN_SHOW_ALL_KEYS),
                      __config_show_all_keys);
        config->write (String (SCIM_CONFIG_IMENGINE_PINYIN_DYNAMIC_ADJUST),
                      __config_dynamic_adjust);
        config->write (String (SCIM_CONFIG_IMENGINE_PINYIN_USER_DATA_BINARY),
                      __config_user_data_binary);
        config->write (String (SCIM_CONFIG_IMENGINE_PINYIN_MAX_USER_PHRASE_LENGTH),
                      __config_max_user_phrase_length);
        config->write (String (SCIM_CONFIG_IMENGINE_PINYIN_MAX_PREEDIT_LENGTH),
                      __config_max_preedit_length);
        config->write (String (SCIM_CONFIG_IMENGINE_PINYIN_SMART_MATCH_LEVEL),
                      __config_smart_match_level);
        config->write (String (SCIM_CONFIG_IMENGINE_PINYIN_BURST_STACK_SIZE),
                      __config_burst_stack_size);
        config->write (String (SCIM_CONFIG_IMENGINE_PINYIN_DYNAMIC_SENSITIVITY),
                      __config_dynamic_sensitivity);
        config->write (String (SCIM_CONFIG_IMENGINE_PINYIN_SAVE_PERIOD),
                      __config_save_period);

        for (i = 0; __config_keyboards [i].key; ++ i) {
            config->write (String (__config_keyboards [i].key),
                          __config_keyboards [i].data);
        }

        config->write (String (SCIM_CONFIG_IMENGINE_PINYIN_TONE),
                      __config_tone);

        config->write (String (SCIM_CONFIG_IMENGINE_PINYIN_INCOMPLETE),
                      __config_incomplete);

        for (int i = 0; i <= SCIM_PINYIN_AmbLast; ++ i) {
            config->write (__ambiguity_config_keys [i], __config_ambiguities [i]);
        }

        __have_changed = false;
    }
}

static bool
__query_changed ()
{
    return __have_changed;
}

static void
__on_default_editable_changed          (GtkEditable     *editable,
                                        gpointer         user_data)
{
    String *str = static_cast <String *> (user_data);

    if (str) {
        *str = String (gtk_entry_get_text (GTK_ENTRY (editable)));
        __have_changed = true;
    }
}

static void
__on_default_toggle_button_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    bool *toggle = static_cast<bool*> (user_data);

    if (toggle) {
        *toggle = gtk_toggle_button_get_active (togglebutton);
        __have_changed = true;
    }
}

static void
__on_default_spin_button_changed       (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
    int *value = static_cast <int *> (user_data);

    if (value) {
        *value = gtk_spin_button_get_value_as_int (spinbutton);
        __have_changed = true;
    }
}

static void
__on_default_key_selection_clicked     (GtkButton       *button,
                                        gpointer         user_data)
{
    KeyboardConfigData *data = static_cast <KeyboardConfigData *> (user_data);

    if (data) {
        GtkWidget *dialog = scim_key_selection_dialog_new (_(data->title));
        gint result;

        scim_key_selection_dialog_set_keys (
            SCIM_KEY_SELECTION_DIALOG (dialog),
            data->data.c_str ());

        result = gtk_dialog_run (GTK_DIALOG (dialog));

        if (result == GTK_RESPONSE_OK) {
            const gchar *keys = scim_key_selection_dialog_get_keys (
                            SCIM_KEY_SELECTION_DIALOG (dialog));

            if (!keys) keys = "";

            if (String (keys) != data->data)
                gtk_entry_set_text (GTK_ENTRY (data->entry), keys);
        }

        gtk_widget_destroy (dialog);
    }
}

static void
__on_auto_combine_phrase_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    __config_auto_combine_phrase = gtk_toggle_button_get_active (togglebutton);

    if (__widget_match_longer_phrase) {
        gtk_widget_set_sensitive (
            __widget_match_longer_phrase,
            !__config_auto_combine_phrase);
    }

    if (__widget_smart_match_level) {
        gtk_widget_set_sensitive (
            __widget_smart_match_level,
            __config_auto_fill_preedit | __config_auto_combine_phrase);
    }

    __have_changed = true;
}

static void
__on_auto_fill_preedit_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    __config_auto_fill_preedit = gtk_toggle_button_get_active (togglebutton);

    if (__widget_always_show_lookup) {
        gtk_widget_set_sensitive (
            __widget_always_show_lookup,
            __config_auto_fill_preedit);
    }

    if (__widget_show_all_keys) {
        gtk_widget_set_sensitive (
            __widget_show_all_keys,
            __config_auto_fill_preedit);
    }

    if (__widget_smart_match_level) {
        gtk_widget_set_sensitive (
            __widget_smart_match_level,
            __config_auto_fill_preedit | __config_auto_combine_phrase);
    }

    __have_changed = true;
}

static void
__on_dynamic_adjust_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    __config_dynamic_adjust = gtk_toggle_button_get_active (togglebutton);

    if (__widget_burst_stack_size) {
        gtk_widget_set_sensitive (
            __widget_burst_stack_size,
            __config_dynamic_adjust);
    }

    if (__widget_dynamic_sensitivity) {
        gtk_widget_set_sensitive (
            __widget_dynamic_sensitivity,
            __config_dynamic_adjust);
    }

    if (__widget_save_period) {
        gtk_widget_set_sensitive (
            __widget_save_period,
            __config_dynamic_adjust);
    }

    __have_changed = true;
}

static void
__on_ambiguities_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    size_t index = (size_t) user_data;

    if (index >= 0 && index <= SCIM_PINYIN_AmbLast) {
        __config_ambiguities [index] = gtk_toggle_button_get_active (togglebutton);

        if (index == 0) {
            for (int i = 1; i <= SCIM_PINYIN_AmbLast; ++ i)
                gtk_widget_set_sensitive (__widget_ambiguities [i], __config_ambiguities [0]);
        }

        __have_changed = true;
    }
}

/*
vi:ts=4:nowrap:expandtab
*/
