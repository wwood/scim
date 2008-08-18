/*
 *  CCInput
 *
 *  Copyright (C) 2003, 2004 CCOSS, Inc.
 *
 *
 *  CCInput is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published
 *  by the Free Software Foundation; either version 2 of the License,
 *  or (at your option) any later version.
 *
 *  CCInput is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors: Lin ShiQun <linshiqun@ccoss.com.cn>
 *
 */

#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_UTILITY
#define Uses_SCIM_ICONV

#include <gtk/gtk.h>
#include "scim.h"
#include <gtk/scimkeyselection.h>

#include <stdio.h>
#include <iostream>

#ifndef INPUT_METHOD_NAME 
  #define INPUT_METHOD_NAME "ccinput"
#endif

#ifndef SCIM_ICONDIR
    #define SCIM_ICONDIR "/usr/share/scim/icons"
#endif

#define _(String) dgettext(INPUT_METHOD_NAME,String)
#define N_(String) (String)

#include <scim.h>
using namespace scim;
using namespace std;

#define scim_module_init ccin_imengine_setup_LTX_scim_module_init
#define scim_module_exit ccin_imengine_setup_LTX_scim_module_exit

#define scim_setup_module_create_ui       ccin_imengine_setup_LTX_scim_setup_module_create_ui
#define scim_setup_module_get_category    ccin_imengine_setup_LTX_scim_setup_module_get_category
#define scim_setup_module_get_name        ccin_imengine_setup_LTX_scim_setup_module_get_name
#define scim_setup_module_get_description   ccin_imengine_setup_LTX_scim_setup_module_get_description
#define scim_setup_module_load_config     ccin_imengine_setup_LTX_scim_setup_module_load_config
#define scim_setup_module_save_config     ccin_imengine_setup_LTX_scim_setup_module_save_config
#define scim_setup_module_query_changed   ccin_imengine_setup_LTX_scim_setup_module_query_changed


#define SCIM_CONFIG_IMENGINE_CCIN_FUZZY_WANG_HUANG_KEY    "/IMEngine/Chinese/CCIN/FuzzyWangHuangKey"
#define SCIM_CONFIG_IMENGINE_CCIN_FUZZY_AN_ANG_KEY    "/IMEngine/Chinese/CCIN/FuzzyAnAngKey"
#define SCIM_CONFIG_IMENGINE_CCIN_FUZZY_EN_ENG_KEY    "/IMEngine/Chinese/CCIN/FuzzyEnEngKey"
#define SCIM_CONFIG_IMENGINE_CCIN_FUZZY_IN_ING_KEY    "/IMEngine/Chinese/CCIN/FuzzyInIngKey"
#define SCIM_CONFIG_IMENGINE_CCIN_FUZZY_C_CH_KEY    "/IMEngine/Chinese/CCIN/FuzzyCChKey"
#define SCIM_CONFIG_IMENGINE_CCIN_FUZZY_S_SH_KEY    "/IMEngine/Chinese/CCIN/FuzzySShKey"
#define SCIM_CONFIG_IMENGINE_CCIN_FUZZY_Z_ZH_KEY    "/IMEngine/Chinese/CCIN/FuzzyZZhKey"
#define SCIM_CONFIG_IMENGINE_CCIN_FUZZY_F_H_KEY    "/IMEngine/Chinese/CCIN/FuzzyFHKey"
#define SCIM_CONFIG_IMENGINE_CCIN_FUZZY_L_N_KEY    "/IMEngine/Chinese/CCIN/FuzzyLNKey"
#define SCIM_CONFIG_IMENGINE_CCIN_FUZZY_K_G_KEY    "/IMEngine/Chinese/CCIN/FuzzyKGKey"
#define SCIM_CONFIG_IMENGINE_CCIN_FUZZY_R_L_KEY    "/IMEngine/Chinese/CCIN/FuzzyRLKey"

#define SCIM_CONFIG_IMENGINE_CCIN_SHUANGPIN_KEY    "/IMEngine/Chinese/CCIN/ShuangpinKey"
#define SCIM_CONFIG_IMENGINE_CCIN_SHUANGPIN_KIND_KEY    "/IMEngine/Chinese/CCIN/ShuangpinKindKey"
#define SCIM_CONFIG_IMENGINE_CCIN_GBK_KEY    "/IMEngine/Chinese/CCIN/GbkKey"

static GtkWidget * create_setup_window ();
static void        load_config (const ConfigPointer &config);
static void        save_config (const ConfigPointer &config);
static bool        query_changed ();

// Module Interface.
extern "C" {
    void scim_module_init (void)
    {
        bindtextdomain (INPUT_METHOD_NAME, SCIM_CCIN_LOCALEDIR);
        textdomain (INPUT_METHOD_NAME);
        bind_textdomain_codeset (INPUT_METHOD_NAME, "UTF-8");
    }

    void scim_module_exit (void)
    {
    }

    GtkWidget * scim_setup_module_create_ui (void)
    {
        return create_setup_window ();
    }

    String scim_setup_module_get_category (void)
    {
        return String ("IMEngine");
    }

    String scim_setup_module_get_name (void)
    {
 /*       setlocale (LC_ALL, "");
        bindtextdomain (INPUT_METHOD_NAME, "/usr/share/locale");
        textdomain (INPUT_METHOD_NAME);
        bind_textdomain_codeset (INPUT_METHOD_NAME, "UTF-8");*/

        return String (_("CC Input"));
    }

    String scim_setup_module_get_description (void)
    {
        return String (_("GongChuang, A IMEngine Module which uses Pinyin."));
    }

    void scim_setup_module_load_config (const ConfigPointer &config)
    {
        load_config (config);
    }

    void scim_setup_module_save_config (const ConfigPointer &config)
    {
        save_config (config);
    }

    bool scim_setup_module_query_changed ()
    {
        return query_changed ();
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

static bool    __config_Fuzzy_Wang_Huang = false;
static bool    __config_Fuzzy_An_Ang     = false;
static bool    __config_Fuzzy_En_Eng      = false;
static bool    __config_Fuzzy_In_Ing      = false;
static bool    __config_Fuzzy_C_Ch      = false;
static bool    __config_Fuzzy_S_Sh      = false;
static bool    __config_Fuzzy_Z_Zh      = false;
static bool    __config_Fuzzy_F_H       = false;
static bool    __config_Fuzzy_L_N      = false;
static bool    __config_Fuzzy_K_G      = false;
static bool    __config_Fuzzy_R_L      = false;

static bool    __config_shuangpin		  	= false;
static String  __config_Shuangpin_Kind		= "chinesestar";
static bool    __config_GBK		      		= false;


static GtkWidget *    __widget_Fuzzy_Wang_Huang = 0;
static GtkWidget *    __widget_Fuzzy_An_Ang     = 0;
static GtkWidget *    __widget_Fuzzy_En_Eng      = 0;
static GtkWidget *    __widget_Fuzzy_In_Ing      = 0;
static GtkWidget *    __widget_Fuzzy_C_Ch      = 0;
static GtkWidget *    __widget_Fuzzy_S_Sh      = 0;
static GtkWidget *    __widget_Fuzzy_Z_Zh      = 0;
static GtkWidget *    __widget_Fuzzy_F_H       = 0;
static GtkWidget *    __widget_Fuzzy_L_N      = 0;
static GtkWidget *    __widget_Fuzzy_K_G      = 0;
static GtkWidget *    __widget_Fuzzy_R_L      = 0;

// Internal data declaration.
static bool __config_show_prompt           = false;
static bool __config_show_key_hint         = false;
static bool __config_user_table_binary     = false;
static bool __config_long_phrase_first     = false;

static bool __have_changed                 = false;

static GtkWidget * __widget_show_prompt           = 0;
static GtkWidget * __widget_show_key_hint         = 0;
static GtkWidget * __widget_user_table_binary     = 0;
static GtkWidget * __widget_long_phrase_first     = 0;
static GtkWidget * __widget_full_width_punct_key  = 0;
static GtkWidget * __widget_full_width_letter_key = 0;
static GtkWidget * __widget_mode_switch_key       = 0;

static GtkWidget * __widget_quanpin_setup		  = 0;
static GtkWidget * __widget_shuangpin_setup		  = 0;

static GtkWidget * __widget_combo					  = 0;
static GtkWidget * __widget_GBK_setup		      = 0;
static GtkWidget * __widget_shuangpin_code_command =0;
static GtkWidget * __widget_show_shuangpin        = 0;


static bool is_shuangpin = true;  
static bool is_not_shuangpin = false;  
static GtkTooltips * __widget_tooltips            = 0;


// Declaration of internal functions.
static void
on_default_editable_changed          (GtkEditable     *editable,
                                      gpointer         user_data);

static void
on_default_toggle_button_toggled     (GtkToggleButton *togglebutton,
                                      gpointer         user_data);

static void
on_default_key_selection_clicked     (GtkButton       *button,
                                      gpointer         user_data);

static void
setup_widget_value ();

static void
on_default_radio_toggle_button_toggled(     GtkToggleButton *togglebutton,
                                      gpointer         user_data);

bool 
create_setup_fuzzy_window(GtkWidget *notebook);

bool
create_setup_sp_window(GtkWidget *notebook);

bool
create_setup_gbk_window(GtkWidget *notebook);

bool
show_shuangpin_code();

// Function implementations.
GtkWidget *
create_setup_window ()
{
    static GtkWidget *window = 0;
/*/
    setlocale (LC_ALL, "");
    bindtextdomain (INPUT_METHOD_NAME, "/usr/share/locale");
    textdomain (INPUT_METHOD_NAME);
    bind_textdomain_codeset (INPUT_METHOD_NAME, "UTF-8");
//*/
    if (!window) {
        GtkWidget *notebook;
        // Create the Notebook.
        notebook = gtk_notebook_new ();
        gtk_widget_show (notebook);

        create_setup_sp_window(notebook);
        create_setup_fuzzy_window(notebook);
//        create_setup_gbk_window(notebook);

        window = notebook;

        setup_widget_value ();
    }

    return window;
}

void
setup_widget_value ()
{
    if (__widget_Fuzzy_Wang_Huang) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_Fuzzy_Wang_Huang),
            __config_Fuzzy_Wang_Huang);
    }

    if (__widget_Fuzzy_An_Ang) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_Fuzzy_An_Ang),
            __config_Fuzzy_An_Ang);
    }

    if (__widget_Fuzzy_En_Eng) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_Fuzzy_En_Eng),
            __config_Fuzzy_En_Eng);
    }

    if (__widget_Fuzzy_In_Ing) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_Fuzzy_In_Ing),
            __config_Fuzzy_In_Ing);
    }

    if (__widget_Fuzzy_C_Ch) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_Fuzzy_C_Ch),
            __config_Fuzzy_C_Ch);
    }

    if (__widget_Fuzzy_S_Sh) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_Fuzzy_S_Sh),
            __config_Fuzzy_S_Sh);
    }

    if (__widget_Fuzzy_Z_Zh) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_Fuzzy_Z_Zh),
            __config_Fuzzy_Z_Zh);
    }

    if (__widget_Fuzzy_F_H) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_Fuzzy_F_H),
            __config_Fuzzy_F_H);
    }

    if (__widget_Fuzzy_L_N) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_Fuzzy_L_N),
            __config_Fuzzy_L_N);
    }

    if (__widget_Fuzzy_K_G) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_Fuzzy_K_G),
            __config_Fuzzy_K_G);
    }

    if (__widget_Fuzzy_R_L) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_Fuzzy_R_L),
            __config_Fuzzy_R_L);
    }

    if (__widget_shuangpin_setup) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_shuangpin_setup),
            __config_shuangpin);
    }
    if (__widget_quanpin_setup) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_quanpin_setup),
            !__config_shuangpin);
    }

    if (__widget_Fuzzy_R_L) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_Fuzzy_R_L),
            __config_Fuzzy_R_L);
    }
   
	if (__widget_GBK_setup) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_GBK_setup),
            __config_GBK);
    }
	
}

void
load_config (const ConfigPointer &config)
{
    if (!config.null ()) {
        __config_Fuzzy_Wang_Huang =
            config->read (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_WANG_HUANG_KEY),
                          __config_Fuzzy_Wang_Huang);
        __config_Fuzzy_An_Ang =
            config->read (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_AN_ANG_KEY),
                          __config_Fuzzy_An_Ang);
        __config_Fuzzy_En_Eng =
            config->read (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_EN_ENG_KEY),
                          __config_Fuzzy_En_Eng);
        __config_Fuzzy_In_Ing =
            config->read (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_IN_ING_KEY),
                          __config_Fuzzy_In_Ing);
        __config_Fuzzy_C_Ch =
            config->read (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_C_CH_KEY),
                          __config_Fuzzy_C_Ch);
        __config_Fuzzy_S_Sh =
            config->read (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_S_SH_KEY),
                          __config_Fuzzy_S_Sh);
        __config_Fuzzy_Z_Zh =
            config->read (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_Z_ZH_KEY),
                          __config_Fuzzy_Z_Zh);
        __config_Fuzzy_F_H =
            config->read (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_F_H_KEY),
                          __config_Fuzzy_F_H);
        __config_Fuzzy_L_N =
            config->read (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_L_N_KEY),
                          __config_Fuzzy_L_N);
        __config_Fuzzy_K_G =
            config->read (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_K_G_KEY),
                          __config_Fuzzy_K_G);
        __config_Fuzzy_R_L =
            config->read (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_R_L_KEY),
                          __config_Fuzzy_R_L);
        __config_shuangpin =
            config->read (String (SCIM_CONFIG_IMENGINE_CCIN_SHUANGPIN_KEY),
                          __config_shuangpin);
    	__config_Shuangpin_Kind =
            config->read (String (SCIM_CONFIG_IMENGINE_CCIN_SHUANGPIN_KIND_KEY),
                          __config_Shuangpin_Kind);
        __config_GBK =
            config->read (String (SCIM_CONFIG_IMENGINE_CCIN_GBK_KEY),
                          __config_GBK);

		setup_widget_value ();

		String str	= __config_Shuangpin_Kind;
		if (!strcmp (str.c_str(), "chinesestar")){
			__config_Shuangpin_Kind = _("chinesestar");
		}else if (!strcmp (str.c_str(), "nature")){
			__config_Shuangpin_Kind = _("nature");
		}else if (!strcmp (str.c_str(), "microsoft")){
			__config_Shuangpin_Kind = _("microsoft");
		}else if (!strcmp (str.c_str(), "ziguang")){
			__config_Shuangpin_Kind = _("ziguang");
		}else if (!strcmp (str.c_str(), "abc")){
			__config_Shuangpin_Kind = _("abc");
		}else if (!strcmp (str.c_str(), "liu")){
			__config_Shuangpin_Kind = _("liu");
		}
		gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (__widget_combo)->entry), (const char*)(__config_Shuangpin_Kind.c_str()) );

        __have_changed = false;
    }
}

void
save_config (const ConfigPointer &config)
{
//    if (!config.null ()) 
	{
        config->write (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_WANG_HUANG_KEY),
                          __config_Fuzzy_Wang_Huang);
        config->write (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_AN_ANG_KEY),
                          __config_Fuzzy_An_Ang);
        config->write (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_EN_ENG_KEY),
                          __config_Fuzzy_En_Eng);
        config->write (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_IN_ING_KEY),
                          __config_Fuzzy_In_Ing);
        config->write (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_C_CH_KEY),
                          __config_Fuzzy_C_Ch);
        config->write (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_S_SH_KEY),
                          __config_Fuzzy_S_Sh);
        config->write (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_Z_ZH_KEY),
                          __config_Fuzzy_Z_Zh);
        config->write (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_F_H_KEY),
                          __config_Fuzzy_F_H);
        config->write (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_L_N_KEY),
                          __config_Fuzzy_L_N);
        config->write (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_K_G_KEY),
                          __config_Fuzzy_K_G);
        config->write (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_R_L_KEY),
                          __config_Fuzzy_R_L);
        config->write (String (SCIM_CONFIG_IMENGINE_CCIN_SHUANGPIN_KEY),
                          __config_shuangpin);
//        __config_Shuangpin_Kind = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (__widget_combo)->entry));
		String str	= gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (__widget_combo)->entry));
		if (!strcmp (str.c_str(), _("chinesestar"))){
			__config_Shuangpin_Kind = "chinesestar";
		}else if (!strcmp (str.c_str(), _("nature"))){
			__config_Shuangpin_Kind = "nature";
		}else if (!strcmp (str.c_str(), _("microsoft"))){
			__config_Shuangpin_Kind = "microsoft";
		}else if (!strcmp (str.c_str(), _("ziguang"))){
			__config_Shuangpin_Kind = "ziguang";
		}else if (!strcmp (str.c_str(), _("abc"))){
			__config_Shuangpin_Kind = "abc";
		}else if (!strcmp (str.c_str(), _("liu"))){
			__config_Shuangpin_Kind = "liu";
		}
		config->write (String (SCIM_CONFIG_IMENGINE_CCIN_SHUANGPIN_KIND_KEY),
                          __config_Shuangpin_Kind);
        config->write (String (SCIM_CONFIG_IMENGINE_CCIN_GBK_KEY),
                          __config_GBK);

        __have_changed = false;
    }
}

bool
query_changed ()
{
    return true;
//    return __have_changed;
}

static void
on_default_editable_changed (GtkEditable *editable,
                             gpointer     user_data)
{
    String *str = static_cast <String *> (user_data);

    if (str) {
        *str = String (gtk_entry_get_text (GTK_ENTRY (editable)));
        __have_changed = true;
    }
}

static void
on_default_toggle_button_toggled (GtkToggleButton *togglebutton,
                                  gpointer         user_data)
{
    bool *toggle = static_cast<bool*> (user_data);

    if (toggle) {
        *toggle = gtk_toggle_button_get_active (togglebutton);
        __have_changed = true;
    }
}

static void
on_default_radio_toggle_button_toggled (GtkToggleButton *togglebutton,
                                  gpointer         user_data)
{
    bool *toggle = static_cast<bool*> (user_data);

/*    if (!__have_changed && (*toggle)!=__config_shuangpin) {
        *toggle = gtk_toggle_button_get_active (togglebutton);
        __have_changed = true;
    }
*/
	if (__config_shuangpin != *toggle)
		__config_shuangpin = *toggle;

	cout << "__config_shuangpin_toggle==" << __config_shuangpin << endl;
	if (__config_shuangpin){
		gtk_widget_set_sensitive(__widget_combo, true);
		gtk_widget_set_sensitive(__widget_shuangpin_code_command, true);
	}else{
		gtk_widget_set_sensitive(__widget_combo, false);
		gtk_widget_set_sensitive(__widget_shuangpin_code_command, false);
	}
}

static void
on_default_key_selection_clicked (GtkButton *button,
                                  gpointer   user_data) 
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

bool
show_shuangpin_code(){
    GtkWidget *frame;
    GtkWidget *vbox;
    GtkWidget *image;
	String pinyin_setup_str;
	__widget_show_shuangpin = gtk_dialog_new_with_buttons (_("shuangpin kind"),
                                NULL,
                                GtkDialogFlags (0),
                                GTK_STOCK_OK,
                                GTK_RESPONSE_OK,
			                    NULL);

	gtk_widget_show (__widget_show_shuangpin);

	String str	= gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (__widget_combo)->entry));
	if (!strcmp (str.c_str(), _("chinesestar"))){
		str = "sp_st.png";
		pinyin_setup_str = _("chinesestar");
	}else if (!strcmp (str.c_str(), _("nature"))){
		str = "sp_zr.png";
		pinyin_setup_str = _("nature");
	}else if (!strcmp (str.c_str(), _("microsoft"))){
		str = "sp_ms.png";
		pinyin_setup_str = _("microsoft");
	}else if (!strcmp (str.c_str(), _("ziguang"))){
		str = "sp_zg.png";
		pinyin_setup_str = _("ziguang");
	}else if (!strcmp (str.c_str(), _("abc"))){
		str = "sp_zn.png";
		pinyin_setup_str = _("abc");
	}else if (!strcmp (str.c_str(), _("liu"))){
		str = "sp_ls.png";
		pinyin_setup_str = _("liu");
	}

	str = string(SCIM_ICONDIR) + "/" + str;
	
	pinyin_setup_str += _("shuangpin kind");
	frame = gtk_frame_new (pinyin_setup_str.c_str());
    gtk_widget_show (frame);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 4);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (__widget_show_shuangpin)->vbox),
                        frame, TRUE, TRUE, 0);

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);
    gtk_container_add (GTK_CONTAINER (frame), vbox);

	// 	image = gtk_image_new_from_file("/usr/share/scim/ccinput/ccinput.png");
 	image = gtk_image_new_from_file (str.c_str());
    gtk_box_pack_start (GTK_BOX (vbox), image, TRUE, TRUE, 0);
    gtk_widget_show (image);    

    g_signal_connect_swapped (GTK_OBJECT (__widget_show_shuangpin), 
                                  "response", 
                                  G_CALLBACK (gtk_widget_hide),
                                  GTK_OBJECT (__widget_show_shuangpin));

    g_signal_connect_swapped (GTK_OBJECT (__widget_show_shuangpin), 
                                  "delete_event", 
                                  G_CALLBACK (gtk_widget_hide_on_delete),
                                  GTK_OBJECT (__widget_show_shuangpin));

	gtk_widget_show (__widget_show_shuangpin);

	gint result = gtk_dialog_run (GTK_DIALOG (__widget_show_shuangpin));
    gtk_widget_destroy (__widget_show_shuangpin);
    return 1;

}

static void
on_combo_activate (GtkCombo *combo,
                   gpointer         user_data)
{
//	__config_Shuangpin_Kind = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (__widget_combo)->entry));
//    __have_changed = true;
}

bool 
create_setup_fuzzy_window(GtkWidget *notebook){
	GtkWidget *table;
    GtkWidget *vbox;
    GtkWidget *vbox2;
    GtkWidget *vbox3;
    GtkWidget *vbox4;
    GtkWidget *hbox;
    GtkWidget *frame;
    GtkWidget *frame2;
    GtkWidget *frame3;
    GtkWidget *label;
    int i;

    __widget_tooltips = gtk_tooltips_new ();

    hbox = gtk_hbox_new (FALSE, 8);
    gtk_widget_show (hbox);
    gtk_container_add (GTK_CONTAINER (notebook), hbox);

    frame = gtk_frame_new (_("fuzzy_initial"));
    gtk_widget_show (frame);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 4);
    gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 0);

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
    gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);

    frame2 = gtk_frame_new (_("fuzzy_final"));
    gtk_widget_show (frame2);
    gtk_container_set_border_width (GTK_CONTAINER (frame2), 4);
    gtk_box_pack_start (GTK_BOX (vbox), frame2, TRUE, TRUE, 0);

    frame3 = gtk_frame_new (_("other"));
    gtk_widget_show (frame3);
    gtk_container_set_border_width (GTK_CONTAINER (frame3), 4);
    gtk_box_pack_start (GTK_BOX (vbox), frame3, TRUE, TRUE, 0);

    vbox2 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox2);
    gtk_container_add (GTK_CONTAINER (frame), vbox2);

    vbox3 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox3);
    gtk_container_add (GTK_CONTAINER (frame2), vbox3);

    vbox4 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox4);
    gtk_container_add (GTK_CONTAINER (frame3), vbox4);


        // Create the label for this note page.
//        label = gtk_label_new (_("Generic"));
    label = gtk_label_new (_("Fuzzy_Pinyin"));
    gtk_widget_show (label);
    gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), 1), label);

    hbox = gtk_hbox_new (FALSE, 8);
    gtk_widget_show (hbox);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

    __widget_Fuzzy_Wang_Huang = gtk_check_button_new_with_mnemonic (_("Fuzzy_Wang_Huang"));
    gtk_widget_show (__widget_Fuzzy_Wang_Huang);
    gtk_box_pack_start (GTK_BOX (vbox4), __widget_Fuzzy_Wang_Huang, FALSE, FALSE, 1);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_Fuzzy_Wang_Huang), 1);

    __widget_Fuzzy_An_Ang = gtk_check_button_new_with_mnemonic (_("Fuzzy_An_Ang"));
    gtk_widget_show (__widget_Fuzzy_An_Ang);
    gtk_box_pack_start (GTK_BOX (vbox3), __widget_Fuzzy_An_Ang, FALSE, FALSE, 1);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_Fuzzy_An_Ang), 1);

    __widget_Fuzzy_En_Eng = gtk_check_button_new_with_mnemonic (_("Fuzzy_En_Eng"));
    gtk_widget_show (__widget_Fuzzy_En_Eng);
    gtk_box_pack_start (GTK_BOX (vbox3), __widget_Fuzzy_En_Eng, FALSE, FALSE, 1);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_Fuzzy_En_Eng), 1);

    __widget_Fuzzy_In_Ing = gtk_check_button_new_with_mnemonic (_("Fuzzy_In_Ing"));
    gtk_widget_show (__widget_Fuzzy_In_Ing);
    gtk_box_pack_start (GTK_BOX (vbox3), __widget_Fuzzy_In_Ing, FALSE, FALSE, 1);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_Fuzzy_In_Ing), 1);

    __widget_Fuzzy_C_Ch = gtk_check_button_new_with_mnemonic (_("Fuzzy_C_Ch"));
    gtk_widget_show (__widget_Fuzzy_C_Ch);
    gtk_box_pack_start (GTK_BOX (vbox2), __widget_Fuzzy_C_Ch, FALSE, FALSE, 1);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_Fuzzy_C_Ch), 1);

    __widget_Fuzzy_S_Sh = gtk_check_button_new_with_mnemonic (_("Fuzzy_S_Sh"));
    gtk_widget_show (__widget_Fuzzy_S_Sh);
    gtk_box_pack_start (GTK_BOX (vbox2), __widget_Fuzzy_S_Sh, FALSE, FALSE, 1);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_Fuzzy_S_Sh), 1);

    __widget_Fuzzy_Z_Zh = gtk_check_button_new_with_mnemonic (_("Fuzzy_Z_Zh"));
    gtk_widget_show (__widget_Fuzzy_Z_Zh);
    gtk_box_pack_start (GTK_BOX (vbox2), __widget_Fuzzy_Z_Zh, FALSE, FALSE, 1);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_Fuzzy_Z_Zh), 1);

    __widget_Fuzzy_F_H = gtk_check_button_new_with_mnemonic (_("Fuzzy_F_H"));
    gtk_widget_show (__widget_Fuzzy_F_H);
    gtk_box_pack_start (GTK_BOX (vbox2), __widget_Fuzzy_F_H, FALSE, FALSE, 1);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_Fuzzy_F_H), 1);

    __widget_Fuzzy_L_N = gtk_check_button_new_with_mnemonic (_("Fuzzy_L_N"));
    gtk_widget_show (__widget_Fuzzy_L_N);
    gtk_box_pack_start (GTK_BOX (vbox2), __widget_Fuzzy_L_N, FALSE, FALSE, 1);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_Fuzzy_L_N), 1);

    __widget_Fuzzy_K_G = gtk_check_button_new_with_mnemonic (_("Fuzzy_K_G"));
    gtk_widget_show (__widget_Fuzzy_K_G);
    gtk_box_pack_start (GTK_BOX (vbox2), __widget_Fuzzy_K_G, FALSE, FALSE, 1);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_Fuzzy_K_G), 1);

    __widget_Fuzzy_R_L = gtk_check_button_new_with_mnemonic (_("Fuzzy_R_L"));
    gtk_widget_show (__widget_Fuzzy_R_L);
    gtk_box_pack_start (GTK_BOX (vbox2), __widget_Fuzzy_R_L, FALSE, FALSE, 1);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_Fuzzy_R_L), 1);

        
    // Connect all signals.
    g_signal_connect ((gpointer) __widget_Fuzzy_Wang_Huang, "toggled",
                          G_CALLBACK (on_default_toggle_button_toggled),
                          &__config_Fuzzy_Wang_Huang);

    g_signal_connect ((gpointer) __widget_Fuzzy_An_Ang, "toggled",
                          G_CALLBACK (on_default_toggle_button_toggled),
                          &__config_Fuzzy_An_Ang);

    g_signal_connect ((gpointer) __widget_Fuzzy_En_Eng, "toggled",
                          G_CALLBACK (on_default_toggle_button_toggled),
                          &__config_Fuzzy_En_Eng);

    g_signal_connect ((gpointer) __widget_Fuzzy_In_Ing, "toggled",
                          G_CALLBACK (on_default_toggle_button_toggled),
                          &__config_Fuzzy_In_Ing);

    g_signal_connect ((gpointer) __widget_Fuzzy_C_Ch, "toggled",
                          G_CALLBACK (on_default_toggle_button_toggled),
                          &__config_Fuzzy_C_Ch);

    g_signal_connect ((gpointer) __widget_Fuzzy_S_Sh, "toggled",
                          G_CALLBACK (on_default_toggle_button_toggled),
                          &__config_Fuzzy_S_Sh);

    g_signal_connect ((gpointer) __widget_Fuzzy_Z_Zh, "toggled",
                          G_CALLBACK (on_default_toggle_button_toggled),
                          &__config_Fuzzy_Z_Zh);

    g_signal_connect ((gpointer) __widget_Fuzzy_F_H, "toggled",
                          G_CALLBACK (on_default_toggle_button_toggled),
                          &__config_Fuzzy_F_H);

    g_signal_connect ((gpointer) __widget_Fuzzy_L_N, "toggled",
                          G_CALLBACK (on_default_toggle_button_toggled),
                          &__config_Fuzzy_L_N);

    g_signal_connect ((gpointer) __widget_Fuzzy_K_G, "toggled",
                          G_CALLBACK (on_default_toggle_button_toggled),
                          &__config_Fuzzy_K_G);

    g_signal_connect ((gpointer) __widget_Fuzzy_R_L, "toggled",
                          G_CALLBACK (on_default_toggle_button_toggled),
                          &__config_Fuzzy_R_L);

    // Set all tooltips.
    gtk_tooltips_set_tip (__widget_tooltips, __widget_show_prompt,
                              _("If this option is checked, "
                                "the key prompt of the currently selected phrase "
                                "will be shown."), NULL);

    gtk_tooltips_set_tip (__widget_tooltips, __widget_show_key_hint,
                              _("If this option is checked, "
                                "the remaining keystrokes of the phrases"
                                "will be shown on the lookup table."), NULL);

    gtk_tooltips_set_tip (__widget_tooltips, __widget_user_table_binary,
                              _("If this option is checked, "
                                "the user table will be stored with binary format, "
                                "this will increase the loading speed."), NULL);

    gtk_tooltips_set_tip (__widget_tooltips, __widget_long_phrase_first,
                              _("If this option is checked, "
                                "the longer phrase will be shown "
                                "in front of others. "), NULL);


}

bool
create_setup_sp_window(GtkWidget *notebook){
	GtkWidget *table;
    GtkWidget *vbox;
    GtkWidget *vbox2;
    GtkWidget *vbox3;
    GtkWidget *vbox4;
    GtkWidget *hbox;
    GtkWidget *frame;
    GtkWidget *frame2;
    GtkWidget *frame3;
    GtkWidget *label;
    GSList *group;
    GtkWidget *radiobutton;
    int i;

    vbox2 = gtk_vbox_new (FALSE, 8);
    gtk_widget_show (vbox2);
    gtk_container_add (GTK_CONTAINER (notebook), vbox2);

	frame = gtk_frame_new (_("pinyin_setup"));
    gtk_widget_show (frame);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 4);
    gtk_box_pack_start (GTK_BOX (vbox2), frame, FALSE, FALSE, 0);

    vbox = gtk_vbox_new (FALSE, 8);
    gtk_widget_show (vbox);
    gtk_container_add (GTK_CONTAINER (frame), vbox);

	label = gtk_label_new (_("regular"));
    gtk_widget_show (label);
    gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), 0), label);

     __widget_quanpin_setup = gtk_radio_button_new_with_label (NULL, _("quanpin"));
    gtk_box_pack_start (GTK_BOX (vbox), __widget_quanpin_setup, FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_quanpin_setup), 1);
    gtk_widget_show (__widget_quanpin_setup);

    group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (__widget_quanpin_setup));
    __widget_shuangpin_setup = gtk_radio_button_new_with_label (group, _("shuangpin"));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (__widget_quanpin_setup), TRUE);
    gtk_box_pack_start (GTK_BOX (vbox), __widget_shuangpin_setup, FALSE, FALSE, 0);
    gtk_widget_show (__widget_shuangpin_setup);

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox);
    gtk_container_add (GTK_CONTAINER (vbox), hbox);

	__widget_combo = gtk_combo_new ();
	gtk_widget_show (__widget_combo);
	GList *glist = NULL;
	glist = g_list_append (glist, _("chinesestar"));
    glist = g_list_append (glist, _("nature"));
    glist = g_list_append (glist, _("microsoft")); 
    glist = g_list_append (glist, _("ziguang")); 
    glist = g_list_append (glist, _("abc"));
    glist = g_list_append (glist, _("liu"));
    gtk_combo_set_popdown_strings (GTK_COMBO (__widget_combo), glist);
    gtk_container_add (GTK_CONTAINER (hbox), __widget_combo);

	__widget_shuangpin_code_command = gtk_button_new_with_label (_("Show ..."));
	gtk_widget_show (__widget_shuangpin_code_command);
    gtk_box_pack_start (GTK_BOX (hbox), __widget_shuangpin_code_command, FALSE, FALSE, 1);

//setup gbk
    __widget_GBK_setup = gtk_check_button_new_with_mnemonic (_("GBK_setup"));
    gtk_widget_show (__widget_GBK_setup);
    gtk_box_pack_start (GTK_BOX (vbox2), __widget_GBK_setup, FALSE, FALSE, 1);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_GBK_setup), 1);
	
	if (!__config_shuangpin){
		gtk_widget_set_sensitive(__widget_combo, false);
		gtk_widget_set_sensitive(__widget_shuangpin_code_command, false);
	}

// Connect all signals.
	g_signal_connect (G_OBJECT (GTK_COMBO (__widget_combo)), "activate",
    	                  G_CALLBACK (on_combo_activate), NULL);
	
    g_signal_connect ((gpointer) __widget_quanpin_setup, "toggled",
                          G_CALLBACK (on_default_radio_toggle_button_toggled),
                          &is_not_shuangpin);

    g_signal_connect ((gpointer) __widget_shuangpin_setup, "toggled",
                          G_CALLBACK (on_default_radio_toggle_button_toggled),
                          &is_shuangpin);

	g_signal_connect ((gpointer) __widget_shuangpin_code_command, "clicked",
                          G_CALLBACK (show_shuangpin_code),
                          &__config_shuangpin);

    g_signal_connect ((gpointer) __widget_GBK_setup, "toggled",
                          G_CALLBACK (on_default_toggle_button_toggled),
                          &__config_GBK);
}

/*
vi:ts=4:nowrap:expandtab
*/
