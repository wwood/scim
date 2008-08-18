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

#define Uses_STL_AUTOPTR
#define Uses_STL_FUNCTIONAL
#define Uses_STL_VECTOR
#define Uses_STL_IOSTREAM
#define Uses_STL_FSTREAM
#define Uses_STL_ALGORITHM
#define Uses_STL_MAP
#define Uses_STL_UTILITY
#define Uses_STL_IOMANIP
#define Uses_C_STDIO

#define Uses_SCIM_UTILITY
#define Uses_SCIM_SERVER
#define Uses_SCIM_IMENGINE
#define Uses_SCIM_ICONV
#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_LOOKUP_TABLE
#define Uses_SCIM_DEBUG

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>

#ifdef HAVE_CONFIG_Hu
  #include <config.h>
#endif

#include <scim.h>
using namespace scim;
using namespace std;

#include "scim_generic_table.h"
#include "scim_ccin_imengine.h"

#include "imcontext.h"
#include "file_operation.h"
#include "pinyin_parse.h"
#include "glossary_lookup.h"
#include "glossary_adjust.h"
#include "ccinput.h"
#include "pinyin_table.c"

#ifndef INPUT_METHOD_NAME 
  #define INPUT_METHOD_NAME "ccinput"
#endif

#define _(String) dgettext(INPUT_METHOD_NAME,String)
#define N_(String) (String)

#define scim_module_init ccin_LTX_scim_module_init
#define scim_module_exit ccin_LTX_scim_module_exit
#define scim_imengine_module_init ccin_LTX_scim_imengine_module_init
#define scim_imengine_module_create_factory ccin_LTX_scim_imengine_module_create_factory

#ifndef SCIM_CCIN_SERVER_MODULE_DATADIR
  #define SCIM_CCIN_SERVER_MODULE_DATADIR "/usr/share/scim/ccinput"
#endif

#define SCIM_CCIN_SYSTEM_FREQUENCY_SAVE_NUMBER       100
#define SCIM_CCIN_USER_GLOSSARY_SAVE_NUMBER       10

//#define SCIM_TABLE_ICON_FILE  (SCIM_ICONDIR "/table.png")
#define SCIM_TABLE_ICON_FILE 

#define SCIM_TABLE_SAVE_PERIOD       300

#define SCIM_TABLE_MAX_TABLE_NUMBER  64
#define SCIM_TABLE_MAX_INPUTED_KEYS  20

#define SCIM_CONFIG_IMENGINE_CCIN_FULL_WIDTH_PUNCT_KEY   "/IMEngine/Chinese/CCIN/FullWidthPunctKey"
#define SCIM_CONFIG_IMENGINE_CCIN_FULL_WIDTH_LETTER_KEY  "/IMEngine/Chinese/CCIN/FullWidthLetterKey"
#define SCIM_CONFIG_IMENGINE_CCIN_MODE_SWITCH_KEY        "/IMEngine/Chinese/CCIN/ModeSwitchKey"
#define SCIM_CONFIG_IMENGINE_CCIN_SHOW_PROMPT            "/IMEngine/Chinese/CCIN/ShowPrompt"
#define SCIM_CONFIG_IMENGINE_CCIN_USER_TABLE_BINARY      "/IMEngine/Chinese/CCIN/UserTableBinary"
#define SCIM_CONFIG_IMENGINE_CCIN_LONG_PHRASE_FIRST      "/IMEngine/Chinese/CCIN/LongPhraseFirst"
#define SCIM_CONFIG_IMENGINE_CCIN_SHOW_KEY_HINT          "/IMEngine/Chinese/CCIN/ShowKeyHint"

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

#define SCIM_PROP_STATUS                                                     "/IMEngine/Chinese/CCIN/Status"
#define SCIM_PROP_LETTER                                                     "/IMEngine/Chinese/CCIN/Letter"
#define SCIM_PROP_PUNCT                                                      "/IMEngine/Chinese/CCIN/Punct"

#ifndef SCIM_ICONDIR
    #define SCIM_ICONDIR "/usr/share/scim/icons"
#endif

#define SCIM_FULL_LETTER_ICON                             (SCIM_ICONDIR "/full-letter.png")
#define SCIM_HALF_LETTER_ICON                             (SCIM_ICONDIR "/half-letter.png")
#define SCIM_FULL_PUNCT_ICON                              (SCIM_ICONDIR "/full-punct.png")
#define SCIM_HALF_PUNCT_ICON                              (SCIM_ICONDIR "/half-punct.png")


using namespace scim;
using std::cout;
using std::cerr;
using std::endl;

char user_input_buffer[MAX_CHAR_IN_ORIGIN_PINYIN_STRING];
int pinyin_parse_number;
u_short has_separator[9];
int first_time = 1;
uint32 fuzzy_pinyin = 0x0000;
int g_flag_is_GBK = 0;
ccinSyllable_t parsed_sp [MAX_SYLLABLE_IN_PHRASE];
//extern ccinSPMappingKey_t * g_sp_config_working;

static unsigned int _scim_number_of_tables = 0;

static Pointer <CcinIMEngineFactory> _scim_ccin_imengine_factories [1];

static std::vector<String> _scim_sys_ccin_list;
static std::vector<String> _scim_user_ccin_list;

static ConfigPointer _scim_config;

static Property _status_property (SCIM_PROP_STATUS, "");
static Property _letter_property (SCIM_PROP_LETTER, _("Full/Half Letter"));
static Property _punct_property  (SCIM_PROP_PUNCT, _("Full/Half Punct"));


static void
_get_table_list (std::vector<String> &table_list, const String &path)
{
    table_list.clear ();
    
    int i=0;
    DIR *dir = opendir (path.c_str ());
    if (dir != NULL) {
        struct dirent *file = readdir (dir);
        while (file != NULL) {
            struct stat filestat;
            String absfn = path + SCIM_PATH_DELIM_STRING + file->d_name;
            String fn = file->d_name;
            
            stat (absfn.c_str (), &filestat);
            if (S_ISREG (filestat.st_mode) && !strcmp (file->d_name, "ccinput.cfg"))
                table_list.push_back (absfn);

            file = readdir (dir);
            i++;
        }
        closedir (dir);
    }
}

extern "C" {
    void scim_module_init (void)
    {

    }

    void scim_module_exit (void)
    {
        for (int i=0; i<_scim_number_of_tables; ++i)
            _scim_ccin_imengine_factories [i].reset ();

        _scim_config.reset ();
    }

    unsigned int scim_imengine_module_init (const ConfigPointer &config)
    {
        _status_property.set_tip (_("The status of the current input method. Click to change it."));
        _letter_property.set_tip (_("The input mode of the letters. Click to toggle between half and full."));
        _punct_property.set_tip (_("The input mode of the puncutations. Click to toggle between half and full."));
        
        _status_property.set_label (" ");
        _letter_property.set_icon (SCIM_HALF_LETTER_ICON);
        _punct_property.set_icon (SCIM_HALF_PUNCT_ICON);
        
        _scim_config = config;

        _get_table_list (_scim_sys_ccin_list, SCIM_CCIN_SERVER_MODULE_DATADIR);
        _get_table_list (_scim_user_ccin_list, scim_get_home_dir () + SCIM_PATH_DELIM_STRING + ".scim" + SCIM_PATH_DELIM_STRING + "tables");
        
        _scim_number_of_tables = _scim_sys_ccin_list.size () + _scim_user_ccin_list.size ();

        return _scim_number_of_tables;
        return 1;
    }

    IMEngineFactoryPointer scim_imengine_module_create_factory (unsigned int server)
    {
        if (server >= _scim_number_of_tables) return 0;

        if (_scim_ccin_imengine_factories [server].null ()) {
            _scim_ccin_imengine_factories [server] = new CcinIMEngineFactory (_scim_config);

            if (server < _scim_sys_ccin_list.size ())
                _scim_ccin_imengine_factories [server]->load_table (_scim_sys_ccin_list [server], false);
            else
                _scim_ccin_imengine_factories [server]->load_table (_scim_user_ccin_list [server - _scim_sys_ccin_list.size ()], true);
            if (!_scim_ccin_imengine_factories [server]->valid ())
                _scim_ccin_imengine_factories [server].reset ();

        }
        return _scim_ccin_imengine_factories [server];

    }
}

// implementation of TableServer
CcinIMEngineFactory::CcinIMEngineFactory (const ConfigPointer &config)
    : m_config (config),
      m_is_user_table (false),
      m_show_prompt (false),
      m_show_key_hint (false),
      m_user_table_binary (false),
      m_modified (false),
      m_long_phrase_first (false),
      m_last_time ((time_t)0)
{
    init ();
}

void
CcinIMEngineFactory::init ()
{
    String str;
    String home_dir;

    if (!m_config.null ()) {
        m_config_fuzzy_Wang_Huang = m_config->read (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_WANG_HUANG_KEY), false);
        m_config_fuzzy_An_Ang = m_config->read (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_AN_ANG_KEY), false);
        m_config_fuzzy_En_Eng = m_config->read (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_EN_ENG_KEY), false);
        m_config_fuzzy_In_Ing = m_config->read (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_IN_ING_KEY), false);
        m_config_fuzzy_C_Ch = m_config->read (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_C_CH_KEY), false);
        m_config_fuzzy_S_Sh = m_config->read (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_S_SH_KEY), false);
        m_config_fuzzy_Z_Zh = m_config->read (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_Z_ZH_KEY), false);
        m_config_fuzzy_F_H = m_config->read (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_F_H_KEY), false);
        m_config_fuzzy_L_N = m_config->read (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_L_N_KEY), false);
        m_config_fuzzy_K_G = m_config->read (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_K_G_KEY), false);
        m_config_fuzzy_R_L = m_config->read (String (SCIM_CONFIG_IMENGINE_CCIN_FUZZY_R_L_KEY), false);

        if (m_config_fuzzy_Wang_Huang) fuzzy_pinyin |= FUZZY_SYLLABLE_1;
        if (m_config_fuzzy_An_Ang) fuzzy_pinyin |= FUZZY_FINAL_1;
        if (m_config_fuzzy_En_Eng) fuzzy_pinyin |= FUZZY_FINAL_2;
        if (m_config_fuzzy_In_Ing) fuzzy_pinyin |= FUZZY_FINAL_3;
        if (m_config_fuzzy_C_Ch) fuzzy_pinyin |= FUZZY_INITIAL_1;
        if (m_config_fuzzy_S_Sh) fuzzy_pinyin |= FUZZY_INITIAL_2;
        if (m_config_fuzzy_Z_Zh) fuzzy_pinyin |= FUZZY_INITIAL_3;
        if (m_config_fuzzy_F_H) fuzzy_pinyin |= FUZZY_INITIAL_4;
        if (m_config_fuzzy_L_N) fuzzy_pinyin |= FUZZY_INITIAL_5;
        if (m_config_fuzzy_K_G) fuzzy_pinyin |= FUZZY_INITIAL_6;
        if (m_config_fuzzy_R_L) fuzzy_pinyin |= FUZZY_INITIAL_7;
        m_config_shuangpin = m_config->read (String (SCIM_CONFIG_IMENGINE_CCIN_SHUANGPIN_KEY), false);
        m_config_GBK = m_config->read (String (SCIM_CONFIG_IMENGINE_CCIN_GBK_KEY), false);
        m_config_Shuangpin_Kind = m_config->read (String (SCIM_CONFIG_IMENGINE_CCIN_SHUANGPIN_KIND_KEY), String ("chinesestar"));

        //Read full width punctuation keys
        str = m_config->read (String (SCIM_CONFIG_IMENGINE_CCIN_FULL_WIDTH_PUNCT_KEY), String ("Control+period"));

        scim_string_to_key_list (m_full_width_punct_keys, str);

        //Read full width letter keys
        str = m_config->read (String (SCIM_CONFIG_IMENGINE_CCIN_FULL_WIDTH_LETTER_KEY), String ("Shift+space"));

        scim_string_to_key_list (m_full_width_letter_keys, str);

        //Read mode switch keys
        str = m_config->read (String (SCIM_CONFIG_IMENGINE_CCIN_MODE_SWITCH_KEY), String ("Alt+Shift_L,Alt+Shift_R,Shift+Shift_L+KeyRelease,Shift+Shift_R+KeyRelease"));

        scim_string_to_key_list (m_mode_switch_keys, str);

        m_show_prompt = m_config->read (String (SCIM_CONFIG_IMENGINE_CCIN_SHOW_PROMPT), false);

        m_show_key_hint = m_config->read (String (SCIM_CONFIG_IMENGINE_CCIN_SHOW_KEY_HINT), false);

        m_long_phrase_first = m_config->read (String (SCIM_CONFIG_IMENGINE_CCIN_LONG_PHRASE_FIRST), false);

        m_user_table_binary = m_config->read (String (SCIM_CONFIG_IMENGINE_CCIN_USER_TABLE_BINARY), false);
    }

    if (m_full_width_punct_keys.size () == 0)
        m_full_width_punct_keys.push_back (KeyEvent (SCIM_KEY_comma, SCIM_KEY_ControlMask));

    if (m_full_width_letter_keys.size () == 0)
        m_full_width_letter_keys.push_back (KeyEvent (SCIM_KEY_space, SCIM_KEY_ShiftMask));

    if (m_mode_switch_keys.size () == 0) {
        m_mode_switch_keys.push_back (KeyEvent (SCIM_KEY_Shift_L, SCIM_KEY_AltMask));
        m_mode_switch_keys.push_back (KeyEvent (SCIM_KEY_Shift_R, SCIM_KEY_AltMask));
        m_mode_switch_keys.push_back (KeyEvent (SCIM_KEY_Shift_L, SCIM_KEY_ShiftMask|SCIM_KEY_ReleaseMask));
        m_mode_switch_keys.push_back (KeyEvent (SCIM_KEY_Shift_R, SCIM_KEY_ShiftMask|SCIM_KEY_ReleaseMask));
    }

    m_last_time = time (NULL);

    m_select_keys = String ("1234567890");
	
	g_flag_is_GBK = m_config_GBK;
	
	ccin_set_sp_config (SP_CONFIG_ST);
	if (m_config_Shuangpin_Kind == "chinesestar")
		ccin_set_sp_config (SP_CONFIG_ST);
	else if (m_config_Shuangpin_Kind == "nature")
		ccin_set_sp_config (SP_CONFIG_ZR);
	else if (m_config_Shuangpin_Kind == "microsoft")
		ccin_set_sp_config (SP_CONFIG_MS);
	else if (m_config_Shuangpin_Kind == "ziguang")
		ccin_set_sp_config (SP_CONFIG_ZG);
	else if (m_config_Shuangpin_Kind == "abc")
		ccin_set_sp_config (SP_CONFIG_ZN);
	else if (m_config_Shuangpin_Kind == "liu")
		ccin_set_sp_config (SP_CONFIG_LS);

    first_time ++;
    ccin_open_imfactory ();
}

CcinIMEngineFactory::~CcinIMEngineFactory ()
{
    if (m_table.is_dynamic_adjust ()) {
        if (m_is_user_table)
            save_user_table ();
        else
            save_sys_table_freq ();
    }
    ccin_close_imfactory ();
}

WideString
CcinIMEngineFactory::get_name () const
{
    return m_table.get_name (scim_get_current_locale ());
}

WideString
CcinIMEngineFactory::get_authors () const
{
    return m_table.get_author ();
}

WideString
CcinIMEngineFactory::get_credits () const
{
    return WideString ();
}

WideString
CcinIMEngineFactory::get_help () const
{
    WideString help;

    String full_width_letter;
    String full_width_punct;
    String mode_switch;

    scim_key_list_to_string (full_width_letter, m_full_width_letter_keys);
    scim_key_list_to_string (full_width_punct, m_full_width_punct_keys);
    scim_key_list_to_string (mode_switch, m_mode_switch_keys);

    return utf8_mbstowcs (String (_("Hot Keys:\n\n  ")) +
                        full_width_letter + String (":\n") +
                        String (_("    switch between full/half width letter mode.\n\n  ")) +
                        full_width_punct + String (":\n") +
                        String (_("    switch between full/half width punctuation mode.\n\n  ")) +
                        mode_switch + String (":\n") +
                        String (_("    switch between Forward/Unforward mode.\n\n")) +
                        String (_("  Control+Down:\n    Move lookup cursor to next shorter phrase\n"
                                  "    Only available when LongPhraseFirst option is set.\n\n")) +
                        String (_("  Control+Up:\n    Move lookup cursor to previous longer phrase\n"
                                  "    Only available when LongPhraseFirst option is set.\n\n")) +
                        String (_("  Esc:\n    reset the input method.\n")));
    return help;
}

String
CcinIMEngineFactory::get_language () const
{
	    return scim_validate_language ("other");
}

String
CcinIMEngineFactory::get_uuid () const
{
    return m_table.get_uuid ();
}

String
CcinIMEngineFactory::get_icon_file () const
{
    String file = m_table.get_icon_file ();

    return file.length () ? file : String (SCIM_TABLE_ICON_FILE);
}

IMEngineInstancePointer
CcinIMEngineFactory::create_instance (const String& encoding, int id)
{
    return new CcinIMEngineInstance (this, encoding, id);
}

bool
CcinIMEngineFactory::load_table (const String &table_file, bool user_table)
{
    if (table_file.length ()) {
        m_table.load_lib (table_file);
        m_table_filename = table_file;
        m_is_user_table = user_table;

        if (!user_table)
            load_sys_table_freq ();

        set_locales (m_table.get_locales ());
        set_languages ("zh_CN,zh_TW,zh_HK,zh_SG");
        
        scim_string_to_key_list (m_pageup_keys, m_table.get_pageup_keys ());
        scim_string_to_key_list (m_pagedown_keys, m_table.get_pagedown_keys ());

        uint32 lookup_pagesize = m_table.get_select_keys ().length ();

        if (lookup_pagesize > SCIM_LOOKUP_TABLE_MAX_PAGESIZE)
            lookup_pagesize = SCIM_LOOKUP_TABLE_MAX_PAGESIZE;

        m_select_keys = m_table.get_select_keys ().substr (0, lookup_pagesize);

        if (!m_pageup_keys.size ())
            m_pageup_keys.push_back (KeyEvent (SCIM_KEY_comma));

        if (!m_pagedown_keys.size ())
            m_pagedown_keys.push_back (KeyEvent (SCIM_KEY_period));

        compact_memory ();

        return valid ();
    }
    return false;
}

void
CcinIMEngineFactory::refresh ()
{
    if (!m_table.is_dynamic_adjust ())
        return;

    time_t cur_time = time (NULL);

    m_modified = true;

    if (cur_time < m_last_time || cur_time - m_last_time > SCIM_TABLE_SAVE_PERIOD) {
        m_last_time = cur_time;
        if (m_is_user_table)
            save_user_table ();
        else
            save_sys_table_freq ();
    }
}

void
CcinIMEngineFactory::save_user_table ()
{
    if (m_table_filename.length () && m_modified)
        m_table.save_lib (m_table_filename, "", m_user_table_binary);
}

String
CcinIMEngineFactory::get_sys_table_freq_file ()
{
    String fn, tf;
    size_t pos;

    if (m_table_filename.length ()) {
        pos = m_table_filename.rfind (SCIM_PATH_DELIM);

        if (pos != String::npos)
            tf = m_table_filename.substr (pos+1);
        else
            tf = m_table_filename;

        fn = scim_get_home_dir () + SCIM_PATH_DELIM_STRING + ".scim" + SCIM_PATH_DELIM_STRING + "tables";

        //Make the tables dir if it's not exist.
        if (access (fn.c_str (), R_OK | W_OK) != 0) {
            mkdir (fn.c_str (), S_IRUSR | S_IWUSR | S_IXUSR);
            if (access (fn.c_str (), R_OK | W_OK) != 0)
                return String ();
        }

        fn = fn + SCIM_PATH_DELIM_STRING + "frequencies";

        //Make the tables/frequencies dir if it's not exist.
        if (access (fn.c_str (), R_OK | W_OK) != 0) {
            mkdir (fn.c_str (), S_IRUSR | S_IWUSR | S_IXUSR);
            if (access (fn.c_str (), R_OK | W_OK) != 0)
                return String ();
        }

        fn = fn + SCIM_PATH_DELIM_STRING + tf + ".freq";
    }
    return fn;
}

void
CcinIMEngineFactory::load_sys_table_freq ()
{
    String fn = get_sys_table_freq_file ();

    if (fn.length ()) {
        std::ifstream is (fn.c_str ());
        if (is) m_table.input_phrase_frequencies (is);
    }
}

void
CcinIMEngineFactory::save_sys_table_freq ()
{
    if (m_modified) {
        String fn = get_sys_table_freq_file ();

        if (fn.length ()) {
            std::ofstream os (fn.c_str ());
            if (os) m_table.output_phrase_frequencies (os, m_user_table_binary);
        }
    }
}
                                                                 
void
CcinIMEngineFactory::compact_memory ()
{
    m_table.compact_memory ();

    std::vector<KeyEvent> (m_full_width_punct_keys).swap (m_full_width_punct_keys);
    std::vector<KeyEvent> (m_full_width_letter_keys).swap (m_full_width_letter_keys);
    std::vector<KeyEvent> (m_mode_switch_keys).swap (m_mode_switch_keys);
    std::vector<KeyEvent> (m_pageup_keys).swap (m_pageup_keys);
    std::vector<KeyEvent> (m_pagedown_keys).swap (m_pagedown_keys);
}


// implementation of CcinIMEngineInstance
CcinIMEngineInstance::CcinIMEngineInstance (CcinIMEngineFactory *factory,
                                            const String& encoding,
                                            int id)
    : IMEngineInstanceBase (factory, encoding, id),
      m_factory (factory),
      m_double_quotation_state (false),
      m_single_quotation_state (false),
      m_forward (false),
      m_focused (false),
      m_inputing_caret (0),
      m_inputing_key (0),
      m_inputed_word_number (0),
      m_inputed_user_word_number (0),
      m_iconv (encoding)
{
    im_context = ccin_initialize_context ();

    std::vector<KeyEvent> page_index_keys;

    m_full_width_punctuation [0] = true;
    m_full_width_punctuation [1] = false;

    m_full_width_letter [0] = false;
    m_full_width_letter [1] = false;

    for (uint32 i=0; i<m_factory->m_select_keys.length (); ++i)
        page_index_keys.push_back (KeyEvent (m_factory->m_select_keys [i], 0));

    m_lookup_table.set_page_size (m_factory->m_select_keys.length ());
    m_lookup_table.show_cursor ();
}

CcinIMEngineInstance::~CcinIMEngineInstance ()
{
    if (m_inputed_user_word_number) {
        ccin_save_user_glossary ();
        ccin_save_user_frequency ();
    }
    ccin_reset_context (im_context);
    free (im_context);
}

bool
CcinIMEngineInstance::process_key_event (const KeyEvent& key)
{
    IConvert conv;
    WideString res;
    String dst;

    if (!m_focused) {
		return false;
	}
    // capture the mode switch key events
    if (match_key_event (m_factory->m_mode_switch_keys, key)) {
        m_forward = !m_forward;
        refresh_all_properties ();
        reset ();
        m_prev_key = key;
        return true;
    }

    // toggle full width punctuation mode
    if (match_key_event (m_factory->m_full_width_punct_keys, key)) {
        trigger_property (SCIM_PROP_PUNCT);
        m_prev_key = key;
        return true;
    }

    // toggle full width letter mode
    if (match_key_event (m_factory->m_full_width_letter_keys, key)) {
        trigger_property (SCIM_PROP_LETTER);
        m_prev_key = key;
        return true;
    }

    //lookup table page up
    if (!m_forward && match_key_event (m_factory->m_pageup_keys, key)) {
        if (lookup_page_up ()) {
            m_prev_key = key;
            return true;
        }
    }

    //lookup table page down
    if (!m_forward && match_key_event (m_factory->m_pagedown_keys, key)) {
        if (lookup_page_down ()) {
            m_prev_key = key;
            return true;
        }
    }

    m_prev_key = key;

    if (key.is_key_release ())
        return true;

    //tab key
    if (key.code == SCIM_KEY_Tab && key.mask == 0)
        return post_process (key.get_ascii_code ());

    if (!m_forward) {
        //reset key
        if (key.code == SCIM_KEY_Escape && key.mask == 0) {
            if (m_inputed_keys.size () == 0) {
				m_translated_quanpin_string = "";
				return false;
			}
            reset ();
            return true;
        }

        //caret left
        if (key.code == SCIM_KEY_Left && key.mask == 0)
            return caret_left ();

        //caret right
        if (key.code == SCIM_KEY_Right && key.mask == 0)
            return caret_right ();

        //caret home
        if (key.code == SCIM_KEY_Home && key.mask == 0)
            return caret_home ();

        //caret end
        if (key.code == SCIM_KEY_End && key.mask == 0)
            return caret_end ();

        //lookup table cursor up
        if (key.code == SCIM_KEY_Up && key.mask == 0)
            return lookup_cursor_up ();

        //lookup table cursor down
        if (key.code == SCIM_KEY_Down && key.mask == 0)
            return lookup_cursor_down ();

        //lookup table cursor up to longer phrase
        if (key.code == SCIM_KEY_Up && key.mask == SCIM_KEY_ControlMask
            && m_factory->m_long_phrase_first)
            return lookup_cursor_up_to_longer ();

        //lookup table cursor down
        if (key.code == SCIM_KEY_Down && key.mask == SCIM_KEY_ControlMask
            && m_factory->m_long_phrase_first)
            return lookup_cursor_down_to_shorter ();

        //backspace key
        if (key.code == SCIM_KEY_BackSpace && key.mask == 0)
            return erase ();

        //delete key
        if (key.code == SCIM_KEY_Delete && key.mask == 0)
            return erase (false);

        //select lookup table
        uint32 pos = m_factory->m_table.get_select_key_pos (key.get_ascii_code ());
        if (pos != String::npos) {
            if (key.mask & (SCIM_KEY_ControlMask)) {
                return lookup_delete (key.get_ascii_code ());
            }
            else {
                return lookup_select (key.get_ascii_code ());
            }

        }
        else if (pos != String::npos && pos >= m_lookup_table.get_current_page_size ()) {
            return post_process (key.get_ascii_code ());
        }

        //space hit
        if (key.code == SCIM_KEY_space && key.mask == 0)
            return space_hit ();

        //enter hit
        if (key.code == SCIM_KEY_Return && key.mask == 0)
            return enter_hit ();

//        if ((key.mask & (~ (SCIM_KEY_ShiftMask + SCIM_KEY_LockMask))) == 0) {
        if ((key.mask & (~SCIM_KEY_ShiftMask)) == 0) {
            return insert (key.get_ascii_code ());
        }
    }
#if SCIM_1_2_API
    if ((key.mask & (~ (SCIM_KEY_ShiftMask + SCIM_KEY_LockMask))) == 0)
#else
    if ((key.mask & (~ (SCIM_KEY_ShiftMask + SCIM_KEY_CapsLockMask))) == 0)
#endif
        return post_process (key.get_ascii_code ());

    return false;
}

void
CcinIMEngineInstance::update_lookup_table_page_size (unsigned int page_size)
{
    if (page_size > 0)
        m_lookup_table.set_page_size (page_size);
}

void
CcinIMEngineInstance::move_preedit_caret (unsigned int pos)
{
    uint32 len = 0;
    uint32 i;

    for (i=0; i<m_converted_strings.size (); ++i) {
        if (pos >= len && pos < len + m_converted_strings [i].length ()) {
            m_inputing_key = i;
            m_inputing_caret = m_inputed_keys [i].length ();

            m_converted_strings.erase (m_converted_strings.begin () + i, m_converted_strings.end ());
            m_converted_pinyins.erase (m_converted_pinyins.begin () + i, m_converted_pinyins.end ());

            refresh_lookup_table ();
            refresh_preedit_string ();
            refresh_preedit_caret ();
            refresh_aux_string ();

            return;
        }
        len += m_converted_strings [i].length ();
    }

    if (m_converted_strings.size ()) {
        ++len;
        if (pos < len) ++pos;
    }

    for (uint32 i=m_converted_strings.size (); i<m_inputed_keys.size (); ++i) {
        if (pos >= len && pos <= len + m_inputed_keys [i].length ()) {
            m_inputing_key = i;
            m_inputing_caret = pos - len;

            refresh_preedit_caret ();
            refresh_aux_string ();
            return;
        }

        len += (m_inputed_keys [i].length () +1); 

    }
}

void
CcinIMEngineInstance::reset ()
{
    m_double_quotation_state = false;
    m_single_quotation_state = false;

    m_lookup_table.clear ();
    std::vector<String> ().swap (m_inputed_keys);

    std::vector<WideString> ().swap (m_converted_strings);
    std::vector<u_short> ().swap (m_converted_pinyins);

    GenericKeyIndexVector ().swap (m_lookup_key_indexes);
	m_translated_quanpin_string = "";

    m_inputing_caret = 0;
    m_inputing_key = 0;

    m_iconv.set_encoding (get_encoding ());

    hide_lookup_table ();
    hide_preedit_string ();
    hide_aux_string ();

    refresh_all_properties ();
}

void
CcinIMEngineInstance::focus_in ()
{
    m_focused = true;

    initialize_all_properties ();

    refresh_preedit_string ();
    refresh_preedit_caret ();

    if (m_lookup_table.number_of_candidates ()) {
        m_lookup_table.set_page_size (m_factory->m_select_keys.length ());
        update_lookup_table (m_lookup_table);
        show_lookup_table ();
    }

    refresh_aux_string ();
}

void
CcinIMEngineInstance::focus_out ()
{
    m_focused = false;
//    reset ();
}

void
CcinIMEngineInstance::trigger_property (const String &property)
{
    if (property == SCIM_PROP_STATUS) {
        m_forward = !m_forward;

        refresh_all_properties ();
        reset ();
                
    } else if (property == SCIM_PROP_LETTER) {
        m_full_width_letter [m_forward?1:0] =
            !m_full_width_letter [m_forward?1:0];

        refresh_letter_property ();

    } else if (property == SCIM_PROP_PUNCT) {
        m_full_width_punctuation [m_forward?1:0] =
            !m_full_width_punctuation [m_forward?1:0];

        refresh_punct_property ();
    }
}


bool
CcinIMEngineInstance::caret_left ()
{
    if (m_inputed_keys.size ()) {
        if (m_inputing_caret > 0) {
            --m_inputing_caret;
            refresh_preedit_caret ();
        } else if (m_inputing_key > 0) {
            if (m_converted_strings.size () >= m_inputing_key) {
                m_converted_strings.pop_back ();
                m_converted_pinyins.pop_back ();
                parse_pinyin_string ();
                refresh_preedit_string ();
                refresh_lookup_table ();
            }
            --m_inputing_key;
            m_inputing_caret = m_inputed_keys [m_inputing_key].length ();

            parse_pinyin_string ();
            refresh_preedit_caret ();
        }
        refresh_aux_string ();
        return true;
    }
    return false;
}

bool
CcinIMEngineInstance::caret_right ()
{
    if (m_inputed_keys.size ()) {
        if (m_inputing_caret < m_inputed_keys [m_inputing_key].size ()) {
            ++m_inputing_caret;
            refresh_preedit_caret ();
        } else if (m_inputing_key < m_inputed_keys.size () - 1) {
            ++m_inputing_key;
            m_inputing_caret = 0;
            refresh_preedit_caret ();
        }
        refresh_aux_string ();
        return true;
    }
    return false;
}

bool
CcinIMEngineInstance::caret_home ()
{
    if (m_inputed_keys.size ()) {
        if (m_converted_strings.size ()) {
            m_converted_strings.clear ();
            m_converted_pinyins.clear ();
            refresh_preedit_string ();
            refresh_lookup_table ();
        }

        m_inputing_key = 0;
        m_inputing_caret = 0;

        refresh_preedit_caret ();
        refresh_aux_string ();
        return true;
    }
    return false;
}

bool
CcinIMEngineInstance::caret_end ()
{
    if (m_inputed_keys.size ()) {
        m_inputing_key = m_inputed_keys.size () - 1;
        m_inputing_caret = m_inputed_keys [m_inputing_key].length ();

        refresh_preedit_caret ();
        refresh_aux_string ();
        return true;
    }
    return false;
}

bool
CcinIMEngineInstance::insert (char ch)
{

    if (ch == 0) return false;

    String newkey;
    uint32 old_inputing_key = m_inputing_key;
    bool insert_ok = false;

    String inputed_string = get_inputed_string ();
    if (inputed_string.length () >= MAX_CHAR_IN_ORIGIN_PINYIN_STRING-1) return true;

    if ((ch=='.') || (ch==',') || (ch=='[') || (ch==']') || (ch=='/') || (ch=='\\') || (ch==';')
        || (ch=='-') || (ch=='=') || (ch=='`') || (ch=='{') || (ch=='}') || (ch=='_') || (ch=='+') 
        || (ch=='!') || (ch=='@') || (ch=='#') || (ch=='$') || (ch=='%') 
        || (ch=='^') || (ch=='&') || (ch=='*') || (ch=='(') || (ch==')') 
        || (ch==':') || (ch=='\'') || (ch=='\"') || (ch=='<') || (ch=='>') || (ch=='?') || (ch=='|') || (ch=='~')) {
        if (m_inputed_keys.size () == 0) {
            return post_process (ch);
        }
    }
    

    if (m_inputed_keys.size ()) {

        newkey = m_inputed_keys [m_inputing_key];
        newkey.insert (newkey.begin () + m_inputing_caret, ch);
        m_inputed_keys [m_inputing_key] = newkey;

        m_global_inputing_caret = 1;
        for (uint32 i=0; i<m_inputing_key; i++) {
            m_global_inputing_caret += m_inputed_keys [i].length ();
        }
        m_global_inputing_caret += m_inputing_caret ;
        parse_pinyin_string ();
             
        for (uint32 i=0; i<m_inputed_keys.size (); i++) {
            m_global_inputing_caret -= m_inputed_keys [i].length ();
            if (m_global_inputing_caret == 0) {
                m_inputing_key = i;
                m_inputing_caret = m_inputed_keys [i].length ();
                insert_ok = true;
                break;
            }
            else if (m_global_inputing_caret < 0) {
                m_inputing_key = i;
                m_inputing_caret = m_inputed_keys [i].length () + m_global_inputing_caret;
                insert_ok = true;
                break;
            }
        }
    }
    else {
        m_translated_quanpin_string = "";
		newkey = String ();
        newkey.push_back (ch);

        m_inputed_keys.push_back (newkey);
        parse_pinyin_string();

        m_inputing_key = 0;
        m_inputing_caret = 1;
        insert_ok = true;
    }

    if (insert_ok) {
        //auto select
        if (old_inputing_key + 1 == m_inputing_key &&
            m_converted_strings.size () == old_inputing_key &&
            m_lookup_table.number_of_candidates () &&
            m_factory->m_table.is_auto_select ()) {
//            lookup_to_converted (m_lookup_table.get_cursor_pos ());
        }

        if (m_inputed_keys.size () > SCIM_TABLE_MAX_INPUTED_KEYS)
            commit_converted ();

        refresh_lookup_table ();
        refresh_preedit_string ();
        refresh_preedit_caret ();

        refresh_aux_string ();
        return true;
    } else if (m_inputed_keys.size ()) {
        return true;
    }
    return post_process (ch);
}

bool
CcinIMEngineInstance::erase (bool backspace)
{
    if (m_inputed_keys.size ()) {
        if (backspace && (m_inputing_key > 0 || m_inputing_caret > 0)) {
            if (m_inputing_caret > 0) {
                --m_inputing_caret;
                m_inputed_keys [m_inputing_key].erase (m_inputing_caret, 1);
            } else {
                if (m_inputed_keys [m_inputing_key].length () == 0)
                    m_inputed_keys.erase (m_inputed_keys.begin () + m_inputing_key);

                --m_inputing_key;
                m_inputing_caret = m_inputed_keys [m_inputing_key].length ();

                if (m_inputing_caret > 0) {
                    --m_inputing_caret;
                    m_inputed_keys [m_inputing_key].erase (m_inputing_caret, 1);
                }
            }

            if (m_inputed_keys [m_inputing_key].length () == 0) {
                m_inputed_keys.erase (m_inputed_keys.begin () + m_inputing_key);

                if (m_inputing_key > 0) {
                    --m_inputing_key;
                    m_inputing_caret = m_inputed_keys [m_inputing_key].length ();
                }
            }
        } else if (!backspace) {
            if (m_inputing_caret < m_inputed_keys [m_inputing_key].length ()) {
                m_inputed_keys [m_inputing_key].erase (m_inputing_caret, 1);
            }
            if (m_inputed_keys [m_inputing_key].length () == 0) {
                m_inputed_keys.erase (m_inputed_keys.begin () + m_inputing_key);
            }
            if (m_inputing_key == m_inputed_keys.size () && m_inputing_key > 0) {
                --m_inputing_key;
                m_inputing_caret = m_inputed_keys [m_inputing_key].length ();
            }
        } else {
            return true;
        }

        if (m_inputed_keys.size () == 1 && m_inputed_keys [0].length () == 0) {
            m_inputed_keys.clear ();
            m_inputing_key = 0;
            m_inputing_caret = 0;
        }

        if (m_converted_strings.size () > m_inputing_key) {
            m_converted_strings.erase (m_converted_strings.begin () + m_inputing_key, m_converted_strings.end ());
            m_converted_pinyins.erase (m_converted_pinyins.begin () + m_inputing_key, m_converted_pinyins.end ());
            refresh_lookup_table ();
        } else if (m_converted_strings.size () == m_inputing_key) {
            refresh_lookup_table ();
        }


        m_global_inputing_caret = 1;
        for (uint32 i=0; i<m_inputing_key; i++) {
            m_global_inputing_caret += m_inputed_keys [i].length ();
        }
        m_global_inputing_caret += m_inputing_caret ;
        m_global_inputing_caret -=1;

        parse_pinyin_string ();

        for (uint32 i=0; i<m_inputed_keys.size (); i++) {
            m_global_inputing_caret -= m_inputed_keys [i].length ();
            if (m_global_inputing_caret == 0) {
                m_inputing_key = i;
                m_inputing_caret = m_inputed_keys [i].length ();
                break;
            }
            else if (m_global_inputing_caret < 0) {
                m_inputing_key = i;
                m_inputing_caret = m_inputed_keys [i].length () + m_global_inputing_caret;
                break;
            }
            else if ((m_global_inputing_caret > 0) && (i==m_inputed_keys.size () - 1)) {
                m_inputing_key = m_inputed_keys.size () - 1;
                m_inputing_caret = m_inputed_keys [i].length () ;
                break;
            }
        }

        refresh_lookup_table ();
        refresh_preedit_string ();
        refresh_preedit_caret ();

        refresh_aux_string ();
        return true;
    }                                    


    return false;
}

bool
CcinIMEngineInstance::space_hit ()
{
    if (m_inputed_keys.size ()) {
        if (m_converted_strings.size () == 0 && m_lookup_table.number_of_candidates () == 0)
            return true;

        if (m_lookup_table.number_of_candidates () && m_converted_strings.size () < m_inputed_keys.size ())
            lookup_to_converted (m_lookup_table.get_cursor_pos ());

        if (m_converted_strings.size () >= m_inputed_keys.size () ||
            m_lookup_table.number_of_candidates () == 0)
            commit_converted ();

        parse_pinyin_string();

        refresh_lookup_table ();
        refresh_preedit_string ();
        refresh_preedit_caret ();
        refresh_aux_string ();

        return true;
    }
    return post_process (' ');
}

bool
CcinIMEngineInstance::enter_hit ()
{
    if (m_inputed_keys.size ()) {
        WideString str;

        for (uint32 i=0; i<m_inputed_keys.size (); ++i)
            str += utf8_mbstowcs (m_inputed_keys [i]);

        reset ();
        commit_string (str);
        return true;
    }
    return false;
}

bool
CcinIMEngineInstance::lookup_cursor_up ()
{
    if (m_inputed_keys.size () && m_lookup_table.number_of_candidates ()) {
        m_lookup_table.cursor_up ();
        m_lookup_table.set_page_size (m_factory->m_select_keys.length ());
        update_lookup_table (m_lookup_table);
        refresh_aux_string ();
        return true;
    }
    return false;
}

bool
CcinIMEngineInstance::lookup_cursor_down ()
{
    if (m_inputed_keys.size () && m_lookup_table.number_of_candidates ()) {
        m_lookup_table.cursor_down ();
        m_lookup_table.set_page_size (m_factory->m_select_keys.length ());
        update_lookup_table (m_lookup_table);
        refresh_aux_string ();
        return true;
    }
    return false;
}

bool
CcinIMEngineInstance::lookup_cursor_up_to_longer ()
{
    if (m_inputed_keys.size () && m_lookup_table.number_of_candidates ()) {
        //Get current lookup table cursor
        uint32 cursor = m_lookup_table.get_cursor_pos ();
        //Get current phrase length
        uint32 curlen = m_factory->m_table.get_phrase_length (m_lookup_key_indexes [cursor].second);

        do {
            m_lookup_table.cursor_up ();
            cursor = m_lookup_table.get_cursor_pos ();
            if (curlen < m_factory->m_table.get_phrase_length (m_lookup_key_indexes [cursor].second))
                break;
        } while (cursor);

        m_lookup_table.set_page_size (m_factory->m_select_keys.length ());
        update_lookup_table (m_lookup_table);
        refresh_aux_string ();
        return true;
    }
    return false;
}

bool
CcinIMEngineInstance::lookup_cursor_down_to_shorter ()
{
    if (m_inputed_keys.size () && m_lookup_table.number_of_candidates ()) {
        uint32 candidates = m_lookup_table.number_of_candidates ();
        //Get current lookup table cursor
        uint32 cursor = m_lookup_table.get_cursor_pos ();
        //Get current phrase length
        uint32 curlen = m_factory->m_table.get_phrase_length (m_lookup_key_indexes [cursor].second);

        do {
            m_lookup_table.cursor_down ();
            cursor = m_lookup_table.get_cursor_pos ();
            if (curlen > m_factory->m_table.get_phrase_length (m_lookup_key_indexes [cursor].second))
                break;
        } while (cursor < candidates - 1);

        m_lookup_table.set_page_size (m_factory->m_select_keys.length ());
        update_lookup_table (m_lookup_table);
        refresh_aux_string ();
        return true;
    }
    return false;
}
                                                                 
bool
CcinIMEngineInstance::lookup_page_up ()
{
    if (m_inputed_keys.size () && m_lookup_table.number_of_candidates ()) {
        m_lookup_table.page_up ();
        m_lookup_table.set_page_size (m_factory->m_select_keys.length ());
        update_lookup_table (m_lookup_table);
        refresh_aux_string ();
        return true;
    }
    return false;
}

bool
CcinIMEngineInstance::lookup_page_down ()
{
    if (m_inputed_keys.size () && m_lookup_table.number_of_candidates ()) {
        m_lookup_table.page_down ();         


        m_lookup_table.set_page_size (m_factory->m_select_keys.length ());
        update_lookup_table (m_lookup_table);
        refresh_aux_string ();
        return true;
    }
    return false;
}

bool
CcinIMEngineInstance::lookup_select (char key)
{
    if (m_inputed_keys.size ()) {

        if (m_lookup_table.number_of_candidates () == 0) {
            return insert (key);
        }
        if ( key - '0' > m_lookup_table.get_current_page_size ())
            return insert (key);
            
        uint32 index;

        index = m_factory->m_table.get_select_key_pos (key);

        index += m_lookup_table.get_current_page_start ();

        lookup_to_converted (index);

        parse_pinyin_string();
        refresh_preedit_string ();
        refresh_lookup_table ();
        refresh_preedit_caret ();
        refresh_aux_string ();

        if (m_converted_strings.size () >= m_inputed_keys.size ())
            commit_converted ();

        parse_pinyin_string();
        refresh_preedit_string ();
        refresh_lookup_table ();
        refresh_preedit_caret ();
        refresh_aux_string ();

        return true;
    }

    return post_process (key);
}

bool
CcinIMEngineInstance::post_process (char key)
{
    if (m_inputed_keys.size () > 0) return true;

    if ((ispunct (key) && m_full_width_punctuation [m_forward?1:0]) ||
        ((isalnum (key) || key == ' ' || key == '\t') && m_full_width_letter [m_forward?1:0])) {
        WideString str;
        if (key == '\t')
            str.push_back ('\t');
        else if (key == '.')
            str.push_back (0x3002);
        else if (key == '\\')
            str.push_back (0x3001);
        else if (key == '^') {
            str.push_back (0x2026);
            str.push_back (0x2026);
        } else if (key == '\"') {
            if (!m_double_quotation_state)
                str.push_back (0x201c);
            else
                str.push_back (0x201d);
            m_double_quotation_state = !m_double_quotation_state;
        } else if (key == '\'') {
            if (!m_single_quotation_state)
                str.push_back (0x2018);
            else
                str.push_back (0x2019);
            m_single_quotation_state = !m_single_quotation_state;
        } else if (key == '<')
            str.push_back (0x300a);
        else if (key == '>')
            str.push_back (0x300b);
        else if (key == '$')
            str.push_back (0xffe5);
        else {
            str.push_back (scim_wchar_to_full_width (key));
        }

        commit_string (str);
        return true;
    }

    return false;
}

void
CcinIMEngineInstance::lookup_to_converted (int index)
{
    if (index < 0 || index >= m_lookup_table.number_of_candidates ())
        return;

    WideString str = m_lookup_table.get_candidate (index);
    uint32 phrase  = m_lookup_key_indexes [index].second;
    uint32 freq    = m_factory->m_table.get_phrase_frequency (phrase);

    int node_num;
    ccinGBWordInfoList_t *gb_word_node_ptr;
    ccinGBKWordInfoList_t *gbk_word_node_ptr;
    ccinPhraseTwoWordInfoList_t *two_word_node_ptr;
    ccinPhraseThreeWordInfoList_t *three_word_node_ptr;
    ccinPhraseFourWordInfoList_t *four_word_node_ptr;
    ccinLongPhraseInfoList_t    *above_four_node_ptr;
    switch(str.length()){
    case 1:
        node_num = index - im_context->lookup_result->lookup_above_four_num;
        node_num -= im_context->lookup_result->lookup_four_word_num;
        node_num -= im_context->lookup_result->lookup_three_word_num;
        node_num -= im_context->lookup_result->lookup_two_word_num;
        if(node_num < im_context->lookup_result->lookup_word_gb_num)
        {
            gb_word_node_ptr = im_context->lookup_result->lookup_word_gb_list;
            while(node_num--)
            {
                gb_word_node_ptr = gb_word_node_ptr->next;
            }
            ccin_phrase_freq_adjust((ccinGBWordInfo *)gb_word_node_ptr->lookup_word_gb,WORD_GB);
            ccin_phrase_freq_adjust_again((ccinGBWordInfo *)gb_word_node_ptr->lookup_word_gb,WORD_GB);
            m_converted_pinyins.push_back (((ccinGBWordInfo *)gb_word_node_ptr->lookup_word_gb)->pinyin_key);
        }
        else {
            node_num -= im_context->lookup_result->lookup_word_gb_num;
            gbk_word_node_ptr = im_context->lookup_result->lookup_word_gbk_list;
            while(node_num--)
            {
                gbk_word_node_ptr = gbk_word_node_ptr->next;
            }
            m_converted_pinyins.push_back (((ccinGBKWordInfo *)gbk_word_node_ptr->lookup_word_gbk)->pinyin_key);
        }
        break;
    case 2:
        node_num = index - im_context->lookup_result->lookup_above_four_num;
        node_num -= im_context->lookup_result->lookup_four_word_num;
        node_num -= im_context->lookup_result->lookup_three_word_num;
        two_word_node_ptr = im_context->lookup_result->lookup_two_word_list;
        while(node_num--)
        {
            two_word_node_ptr = two_word_node_ptr->next;
        }
        ccin_phrase_freq_adjust((ccinPhraseTwoWordInfo *)two_word_node_ptr->lookup_phrase_word_two,PHRASE_TWO);
        ccin_phrase_freq_adjust_again((ccinPhraseTwoWordInfo *)two_word_node_ptr->lookup_phrase_word_two,PHRASE_TWO);
        m_converted_pinyins.push_back (((ccinPhraseTwoWordInfo *)two_word_node_ptr->lookup_phrase_word_two)->pinyin_key[0]);
        m_converted_pinyins.push_back (((ccinPhraseTwoWordInfo *)two_word_node_ptr->lookup_phrase_word_two)->pinyin_key[1]);
        break;
    case 3:
        node_num = index - im_context->lookup_result->lookup_above_four_num;
        node_num -= im_context->lookup_result->lookup_four_word_num;
        three_word_node_ptr = im_context->lookup_result->lookup_three_word_list;
        while(node_num--)
        {
            three_word_node_ptr = three_word_node_ptr->next;
        }
        ccin_phrase_freq_adjust((ccinPhraseThreeWordInfo *)three_word_node_ptr->lookup_phrase_word_three,PHRASE_THREE);
        ccin_phrase_freq_adjust_again((ccinPhraseThreeWordInfo *)three_word_node_ptr->lookup_phrase_word_three,PHRASE_THREE);
        m_converted_pinyins.push_back (((ccinPhraseThreeWordInfo *)three_word_node_ptr->lookup_phrase_word_three)->pinyin_key[0]);
        m_converted_pinyins.push_back (((ccinPhraseThreeWordInfo *)three_word_node_ptr->lookup_phrase_word_three)->pinyin_key[1]);
        m_converted_pinyins.push_back (((ccinPhraseThreeWordInfo *)three_word_node_ptr->lookup_phrase_word_three)->pinyin_key[2]);
        break;
    case 4:
        node_num = index - im_context->lookup_result->lookup_above_four_num;
        four_word_node_ptr = im_context->lookup_result->lookup_four_word_list;
        while(node_num--)
        {
            four_word_node_ptr = four_word_node_ptr->next;
        }
        ccin_phrase_freq_adjust((ccinPhraseFourWordInfo *)four_word_node_ptr->lookup_phrase_word_four,PHRASE_FOUR);
        ccin_phrase_freq_adjust_again((ccinPhraseFourWordInfo *)four_word_node_ptr->lookup_phrase_word_four,PHRASE_FOUR);
        m_converted_pinyins.push_back (((ccinPhraseFourWordInfo *)four_word_node_ptr->lookup_phrase_word_four)->pinyin_key[0]);
        m_converted_pinyins.push_back (((ccinPhraseFourWordInfo *)four_word_node_ptr->lookup_phrase_word_four)->pinyin_key[1]);
        m_converted_pinyins.push_back (((ccinPhraseFourWordInfo *)four_word_node_ptr->lookup_phrase_word_four)->pinyin_key[2]);
        m_converted_pinyins.push_back (((ccinPhraseFourWordInfo *)four_word_node_ptr->lookup_phrase_word_four)->pinyin_key[3]);
        break;
    default:
        node_num = index;
        above_four_node_ptr = im_context->lookup_result->lookup_above_four_list;
        while(node_num--)
        {
            above_four_node_ptr = above_four_node_ptr->next;
        }
        for (uint32 i=0; i<above_four_node_ptr->lookup_long_phrase->word_number; i++) {
            m_converted_pinyins.push_back (((ccinLongPhraseInfo *)above_four_node_ptr->lookup_long_phrase)->pinyin_key[i]);
        }
        break;
    }

    m_inputed_word_number++;
    if (m_inputed_word_number > SCIM_CCIN_SYSTEM_FREQUENCY_SAVE_NUMBER) {
        ccin_save_system_frequency ();
        m_inputed_word_number = 0;
    }

    m_last_converted_string = str;
    for (uint32 i=0; i<str.length(); i++) {
        m_converted_strings.push_back (str.substr(i, 1));
    }
    if (m_inputing_key < m_converted_strings.size ()) {
        m_inputing_key = m_converted_strings.size ();
        m_inputing_caret = 0;
    }
    if (m_converted_strings.size () >= m_inputed_keys.size ()) {
        m_inputing_key = 0;
        m_inputing_caret = 0;      
    }
}

void
CcinIMEngineInstance::commit_converted ()
{
    if (m_converted_strings.size ()) {
        WideString res;

        for (uint32 i=0; i<m_converted_strings.size (); ++i)
            res += m_converted_strings [i];

        commit_string (res);

        if (add_user_phrase (res)) {
            m_inputed_user_word_number++;
            if (m_inputed_user_word_number > SCIM_CCIN_USER_GLOSSARY_SAVE_NUMBER) {
                ccin_save_user_frequency ();
                ccin_save_user_glossary ();
                m_inputed_user_word_number = 0;
            }
        }
                
        m_inputed_keys.erase (m_inputed_keys.begin (), m_inputed_keys.begin () + m_converted_strings.size ());
        m_inputing_key -= m_converted_strings.size ();

        if (m_inputed_keys.size () == 1 && m_inputed_keys [0].length () == 0) {
            m_inputed_keys.clear ();
            m_inputing_key = 0;
            m_inputing_caret = 0;
        }

        m_converted_strings.clear ();
        m_converted_pinyins.clear ();
   }
}

void
CcinIMEngineInstance::refresh_preedit_string ()
{
    WideString preedit_string;
    uint32 i;
    uint32 keypos = 0;

    for (i = 0; i<m_converted_strings.size (); ++i) {
        preedit_string += m_converted_strings [i];
        keypos += m_converted_strings [i].length ();
    }

    if (preedit_string.length ()) {
        preedit_string.push_back ((ucs4_t)' ');
        ++keypos;
    }

    for (i=m_converted_strings.size (); i<m_inputed_keys.size (); ++i) {
        preedit_string += utf8_mbstowcs (m_inputed_keys [i]);
        preedit_string.push_back ((ucs4_t)' ');
    }

    if (preedit_string.length())
        preedit_string += utf8_mbstowcs (m_translated_quanpin_string);

    if (preedit_string.length () == 0) {
        hide_preedit_string ();
        return;
    }

    int start;
    int end;

    if (m_converted_strings.size () < m_inputed_keys.size ()) {
        start = (int) keypos;
        end = (int) keypos + m_inputed_keys [m_converted_strings.size ()].length ();
    } else {
        start = -1;
        end = -1;
    }

    AttributeList attrs;
    attrs.push_back (Attribute(start, end, SCIM_ATTR_DECORATE, SCIM_ATTR_DECORATE_HIGHLIGHT));

    update_preedit_string (preedit_string, attrs);
    show_preedit_string ();
}

void
CcinIMEngineInstance::refresh_lookup_table ()
{
    GenericKeyIndexVector phrases;
    WideString str;

    m_lookup_table.set_page_size (m_factory->m_select_keys.length ());

    create_lookup_table ();

    if (m_lookup_table.number_of_candidates ()) {
        update_lookup_table (m_lookup_table);
        show_lookup_table ();
        return;
    }

    hide_lookup_table ();
}

void
CcinIMEngineInstance::refresh_preedit_caret ()
{
    if (m_inputed_keys.size ()) {
        uint32 keypos = 0;
        uint32 i;
        for (i=0; i<m_converted_strings.size (); ++i)
            keypos += m_converted_strings [i].length ();

        if (keypos) ++keypos;

        for (i=m_converted_strings.size (); i<m_inputed_keys.size () && i<m_inputing_key; ++i)
            keypos += (m_inputed_keys [i].length () + 1);

        int caret = (int) keypos + m_inputing_caret;

        update_preedit_caret (caret);
    }
}

void
CcinIMEngineInstance::refresh_aux_string ()
{
    WideString prompt;

    if (!m_factory->m_show_prompt) return;

    if (m_inputed_keys.size () == 0) {
        hide_aux_string ();
        return;
    }
    prompt = m_factory->m_table.get_key_prompt (m_inputed_keys [m_inputing_key]);

    if (m_lookup_table.number_of_candidates ()) {
        prompt += utf8_mbstowcs (" <");
        prompt += utf8_mbstowcs (m_factory->m_table.get_key_string (
                    m_lookup_key_indexes [m_lookup_table.get_cursor_pos ()].first));
        prompt += utf8_mbstowcs (">");
    }
    update_aux_string (prompt);
//    show_aux_string ();
}

void
CcinIMEngineInstance::initialize_all_properties ()
{
    PropertyList proplist;
    proplist.push_back (_status_property);
    proplist.push_back (_letter_property);
    proplist.push_back (_punct_property);

    register_properties (proplist);
    refresh_all_properties ();
}

void
CcinIMEngineInstance::refresh_all_properties ()
{
    refresh_status_property ();
    refresh_letter_property ();
    refresh_punct_property ();
}

void
CcinIMEngineInstance::refresh_status_property ()
{
    setlocale (LC_ALL, "");
    bindtextdomain (INPUT_METHOD_NAME, "/usr/share/locale");
    textdomain (INPUT_METHOD_NAME);
    bind_textdomain_codeset (INPUT_METHOD_NAME, "UTF-8");

    static String status_forward = _("En");

    if (m_focused) {
        if (m_forward)
            _status_property.set_label (status_forward);
        else
            _status_property.set_label ( utf8_wcstombs (m_factory->m_table.get_status_prompt ()));
    }

    update_property (_status_property);
                       
}

void
CcinIMEngineInstance::refresh_letter_property ()
{
    _letter_property.set_icon (
        m_full_width_letter [m_forward?1:0] ? SCIM_FULL_LETTER_ICON : SCIM_HALF_LETTER_ICON);

    update_property (_letter_property);
}

void
CcinIMEngineInstance::refresh_punct_property ()
{
    _punct_property.set_icon (
        m_full_width_punctuation [m_forward?1:0] ? SCIM_FULL_PUNCT_ICON : SCIM_HALF_PUNCT_ICON);

    update_property (_punct_property);
}


bool
CcinIMEngineInstance::match_key_event (const std::vector<KeyEvent>& keyvec,
                                      const KeyEvent& key)
{
    std::vector<KeyEvent>::const_iterator kit; 

    for (kit = keyvec.begin (); kit != keyvec.end (); ++kit) {
        if (key.code == kit->code && key.mask == kit->mask)
            if (!(key.mask & SCIM_KEY_ReleaseMask) || m_prev_key.code == key.code)
                return true;
    }
    return false;
}

bool
CcinIMEngineInstance::parse_pinyin_string ()
{
    uint32 ret = 0;
    String parse_string;

    ccin_reset_context (im_context);	
    parse_string = get_parse_string ();
    im_context->current_total_pinyin_length = parse_string.length();

    if (im_context->current_total_pinyin_length >= MAX_CHAR_IN_ORIGIN_PINYIN_STRING)
    {
        ccin_reset_context (im_context);	//not necessary in fact
    }

    strncpy ( (char *)(im_context->origin_pinyin_buffer), parse_string.c_str (), im_context->current_total_pinyin_length);
    im_context->origin_pinyin_buffer [im_context->current_total_pinyin_length] = '\0';
//    pinyin_parse_number = ccin_parse_pinyin((char *)im_context->origin_pinyin_buffer,
//                                    im_context->current_total_pinyin_length,
//                                    im_context->pinyin_syllable_buffer,
//                                    has_separator,
//                                    fuzzy_pinyin);
	if (m_factory->m_config_shuangpin == FALSE)	{
		pinyin_parse_number =
			ccin_parse_pinyin ((char *)im_context->origin_pinyin_buffer,
							   im_context->current_total_pinyin_length,
							   im_context->pinyin_syllable_buffer,
							   has_separator, fuzzy_pinyin);
	}
	else{
		pinyin_parse_number =
			ccin_parse_shuangpin ((char *)im_context->origin_pinyin_buffer,
							 	  im_context->current_total_pinyin_length,
								  parsed_sp,
								  im_context->pinyin_syllable_buffer,
								  has_separator, fuzzy_pinyin);
	}

    if (pinyin_parse_number == -1)
    {
        ccin_reset_context (im_context);	//not necessary in fact

        String illegal_key;
        for (uint32 i=0; i<m_inputed_keys.size (); i++) {
           illegal_key += m_inputed_keys [i];
        }
        reset();
        m_inputed_keys.push_back (illegal_key);
        m_inputing_key = 0;
        m_inputing_caret = m_global_inputing_caret;

//       refresh_lookup_table ();
        refresh_preedit_string ();
        refresh_preedit_caret ();
        refresh_aux_string ();
        return false;
    }

    if (pinyin_parse_number == -2)
    {
        uint32 len1;
        uint32 len2;

        len1 = get_inputed_string_length ();

        if (m_converted_strings.size () >= m_inputed_keys.size ()) return false;
        uint32 times = m_inputed_keys.size ();
        for (uint32 i=m_converted_strings.size (); i<times; i++) {
            m_inputed_keys.pop_back ();
        }

		if (m_factory->m_config_shuangpin == FALSE)	{
	        for ( uint32 i=1; i<=9; i++)
    	    {
        	    String str = (const char*) (im_context->pinyin_syllable_buffer[9-i]);

            	if (has_separator[9-i]) {
                	str += "'";
            	}
	            m_inputed_keys.push_back (str);
    	    }
		}else{
	        for ( uint32 i=1; i<=9; i++)
    	    {
        	    String str = (const char*) (parsed_sp[9-i]);

            	if (has_separator[9-i]) {
                	str += "'";
            	}
	            m_inputed_keys.push_back (str);
    	    }
			m_translated_quanpin_string = "  [";
			for ( uint32 i=1; i<=9; i++)
	    	{
    	    	String str = (const char*) (im_context->pinyin_syllable_buffer[9-i]);

//        		if (has_separator[pinyin_parse_number-i]) {
//          	  	str += "'";
//        		}
	        	m_translated_quanpin_string += str;
		        if (i!=9) m_translated_quanpin_string += " ";
    		}
			m_translated_quanpin_string += "]";
		}
			
        len2 = get_inputed_string_length ();
        m_global_inputing_caret = m_global_inputing_caret + len2 - len1;
        ccin_find_matching_results (im_context->pinyin_syllable_buffer, 9, im_context->lookup_result,fuzzy_pinyin);
        return true;
    }

    if (m_converted_strings.size () >= m_inputed_keys.size ()) return false;
    uint32 times = m_inputed_keys.size ();
    for (uint32 i=m_converted_strings.size (); i<times; i++) {
        m_inputed_keys.pop_back ();
    }
    
	if (m_factory->m_config_shuangpin == FALSE)	{
		for ( uint32 i=1; i<=pinyin_parse_number; i++)
	    {
    	    String str = (const char*) (im_context->pinyin_syllable_buffer[pinyin_parse_number-i]);

        	if (has_separator[pinyin_parse_number-i]) {
            	str += "'";
        	}
	        m_inputed_keys.push_back (str);
    	}
	}else{
		for ( uint32 i=1; i<=pinyin_parse_number; i++)
	    {
    	    String str = (const char*) (parsed_sp[pinyin_parse_number-i]);

        	if (has_separator[pinyin_parse_number-i]) {
            	str += "'";
        	}
	        m_inputed_keys.push_back (str);
    	}

		m_translated_quanpin_string = "  [";
		for ( uint32 i=1; i<=pinyin_parse_number; i++)
	    {
    	    String str = (const char*) (im_context->pinyin_syllable_buffer[pinyin_parse_number-i]);

//        	if (has_separator[pinyin_parse_number-i]) {
//            	str += "'";
//        	}
	        m_translated_quanpin_string += str;
	        if (i!=pinyin_parse_number) m_translated_quanpin_string += " ";
    	}
		m_translated_quanpin_string += "]";
	}

    ccin_find_matching_results (im_context->pinyin_syllable_buffer, pinyin_parse_number, im_context->lookup_result,fuzzy_pinyin);
    return true;
}

bool
CcinIMEngineInstance::create_lookup_table() {

    m_lookup_table.clear ();

    u_char buffer_for_display[sizeof(ccinHanziChar_t)*9+1];	//3*9=27

//    ccinLongPhraseInfoList_t * list;
    for (ccinLongPhraseInfoList_t * list = im_context->lookup_result->lookup_above_four_list; list!=NULL; list=list->next)
    {
        memcpy (buffer_for_display, list->lookup_long_phrase->phrase, list->lookup_long_phrase->word_number * sizeof(ccinHanziChar_t));
        buffer_for_display[list->lookup_long_phrase->word_number*sizeof(ccinHanziChar_t)] = '\0';

        String tmp = (String)((const char *)buffer_for_display);
        WideString key = utf8_mbstowcs(tmp);
        m_lookup_table.append_candidate (key);
        m_lookup_key_indexes.push_back (std::pair <uint32, uint32> (0,0));
    }

//    ccinPhraseFourWordInfoList_t * list4;
    for (ccinPhraseFourWordInfoList_t * list4 = im_context->lookup_result->lookup_four_word_list; list4!=NULL; list4=list4->next)
    {
        memcpy (buffer_for_display, list4->lookup_phrase_word_four->phrase, 4 * sizeof(ccinHanziChar_t));
        buffer_for_display[4*sizeof(ccinHanziChar_t)] = '\0';

        String tmp = (String)((const char *)buffer_for_display);
        WideString key = utf8_mbstowcs(tmp);
        m_lookup_table.append_candidate (key);
        m_lookup_key_indexes.push_back (std::pair <uint32, uint32> (0,0));
    }

//    ccinPhraseThreeWordInfoList_t * list;
    for (ccinPhraseThreeWordInfoList_t * list = im_context->lookup_result->lookup_three_word_list; list!=NULL; list=list->next)
    {
        memcpy (buffer_for_display, list->lookup_phrase_word_three->phrase, 3 * sizeof(ccinHanziChar_t));
        buffer_for_display[3*sizeof(ccinHanziChar_t)] = '\0';

        String tmp = (String)((const char *)buffer_for_display);
        WideString key = utf8_mbstowcs(tmp);
        m_lookup_table.append_candidate (key);
        m_lookup_key_indexes.push_back (std::pair <uint32, uint32> (0,0));
    }

//    ccinPhraseTwoWordInfoList_t * list;
    for (ccinPhraseTwoWordInfoList_t * list = im_context->lookup_result->lookup_two_word_list; list!=NULL; list=list->next)
    {
        memcpy (buffer_for_display, list->lookup_phrase_word_two->phrase, 2 * sizeof(ccinHanziChar_t));
        buffer_for_display[2*sizeof(ccinHanziChar_t)] = '\0';

        String tmp = (String)((const char *)buffer_for_display);
        WideString key = utf8_mbstowcs(tmp);
        m_lookup_table.append_candidate (key);
        m_lookup_key_indexes.push_back (std::pair <uint32, uint32> (0,0));
    }

//    ccinGBWordInfoList_t * list;
    for (ccinGBWordInfoList_t * list = im_context->lookup_result->lookup_word_gb_list; list!=NULL; list=list->next)
    {
        memcpy (buffer_for_display, list->lookup_word_gb->word, sizeof(ccinHanziChar_t));
        buffer_for_display[sizeof(ccinHanziChar_t)] = '\0';

        String tmp = (String)((const char *)buffer_for_display);
        WideString key = utf8_mbstowcs(tmp);
        m_lookup_table.append_candidate (key);
        m_lookup_key_indexes.push_back (std::pair <uint32, uint32> (0,0));
    }

//    ccinGBKWordInfoList_t * list;
    for (ccinGBKWordInfoList_t * list = im_context->lookup_result->lookup_word_gbk_list; list!=NULL; list=list->next)
    {
        memcpy (buffer_for_display, list->lookup_word_gbk->word, sizeof(ccinHanziChar_t));
        buffer_for_display[sizeof(ccinHanziChar_t)] = '\0';

        String tmp = (String)((const char *)buffer_for_display);
        WideString key = utf8_mbstowcs(tmp);
        m_lookup_table.append_candidate (key);
        m_lookup_key_indexes.push_back (std::pair <uint32, uint32> (0,0));
    }

}

bool
CcinIMEngineInstance::display_debug_info ()
{
    for (uint32 i=0; i<m_inputed_keys.size () ; i++) {
        cout << "m_inputed_keys [" << i << "] =" << m_inputed_keys[i] << flush << endl;
    }
    for (uint32 i=0; i<m_converted_strings.size () ; i++) {
        cout << "m_converted_strings [" << i << "] =" << utf8_wcstombs (m_converted_strings[i]) << flush << endl;
    }
    cout << "m_inputing_caret=  " << m_inputing_caret << flush <<endl;
    cout << "m_inputing_key=  " << m_inputing_key << flush << endl;
    cout << endl;
    return true;
}

String
CcinIMEngineInstance::get_inputed_string () {
    String res;
//    for (uint32 i=0; i<1; ++i)
    for (uint32 i=0; i<m_inputed_keys.size (); ++i)
        res += m_inputed_keys [i];
    return res;
}

String
CcinIMEngineInstance::get_parse_string () {
    String res;
    for (uint32 i=m_converted_strings.size (); i<m_inputed_keys.size (); ++i)
        res += m_inputed_keys [i];
    return res;
}

bool
CcinIMEngineInstance::add_user_phrase (WideString &ret) {

    if (m_converted_strings.size () <=1)
        return false;
    else if (m_last_converted_string.length () >= ret.length ())
        return false;
    else {
        uint32 len = ret.length ();
        u_char* buffer_for_display = new u_char [sizeof(ccinHanziChar_t)*len+1];
        u_short* pinyin_key = new u_short [len+1];
        std::vector <u_short>::iterator iter;

        uint32 i=0;
        for (iter=m_converted_pinyins.begin (); iter != m_converted_pinyins.end (); iter++) {
            pinyin_key[i] = *iter;
            i++;
        }

        String str = utf8_wcstombs (ret);
        memcpy (buffer_for_display, str.c_str (), sizeof (ccinHanziChar_t) *len );
        buffer_for_display[sizeof (ccinHanziChar_t)*len] = '\0';

        ccin_add_user_phrase (len, (ccinHanziChar_t *)buffer_for_display, pinyin_key);

        free(buffer_for_display);
        free(pinyin_key);
        return true;
    }
}

bool
CcinIMEngineInstance::lookup_delete (char key)
{
    if (m_inputed_keys.size ()) {
        if (m_lookup_table.number_of_candidates () == 0)
            return true;
        if ( key - '0' > m_lookup_table.get_current_page_size ())
            return false;

        int ret;
        uint32 node_num;
        node_num = m_factory->m_table.get_select_key_pos (key);
        node_num += m_lookup_table.get_current_page_start ();

        WideString str = m_lookup_table.get_candidate (node_num);

        ccinGBWordInfoList_t *gb_word_node_ptr;
        ccinGBKWordInfoList_t *gbk_word_node_ptr;
        ccinPhraseTwoWordInfoList_t *two_word_node_ptr;
        ccinPhraseThreeWordInfoList_t *three_word_node_ptr;
        ccinPhraseFourWordInfoList_t *four_word_node_ptr;
        ccinLongPhraseInfoList_t    *above_four_node_ptr;
        switch(str.length()){
        case 1:
            break;
        case 2:
            node_num = node_num - im_context->lookup_result->lookup_above_four_num;
            node_num -= im_context->lookup_result->lookup_four_word_num;
            node_num -= im_context->lookup_result->lookup_three_word_num;
            two_word_node_ptr = im_context->lookup_result->lookup_two_word_list;
            while(node_num--)
            {
               two_word_node_ptr = two_word_node_ptr->next;
            }
			ret = ccin_del_user_phrase(PHRASE_TWO, (ccinPhraseTwoWordInfo *)two_word_node_ptr->lookup_phrase_word_two);
            break;
        case 3:
            node_num = node_num - im_context->lookup_result->lookup_above_four_num;
            node_num -= im_context->lookup_result->lookup_four_word_num;
            three_word_node_ptr = im_context->lookup_result->lookup_three_word_list;
            while(node_num--)
            {
                three_word_node_ptr = three_word_node_ptr->next;
            }
            ret = ccin_del_user_phrase(PHRASE_THREE, (ccinPhraseThreeWordInfo *)three_word_node_ptr->lookup_phrase_word_three);
            break;
        case 4:
            node_num = node_num - im_context->lookup_result->lookup_above_four_num;
            four_word_node_ptr = im_context->lookup_result->lookup_four_word_list;
            while(node_num--)
            {
                four_word_node_ptr = four_word_node_ptr->next;
            }
            ret = ccin_del_user_phrase(PHRASE_FOUR, (ccinPhraseFourWordInfo *)four_word_node_ptr->lookup_phrase_word_four);
            break;
        default:
            node_num = node_num;
            above_four_node_ptr = im_context->lookup_result->lookup_above_four_list;
            while(node_num--)
            {
                above_four_node_ptr = above_four_node_ptr->next;
            }
            ret = ccin_del_user_phrase(PHRASE_LONG, (ccinLongPhraseInfoList_t *)above_four_node_ptr->lookup_long_phrase);
            break;
            
        }
        parse_pinyin_string();
//        refresh_preedit_string ();

#if 0
        char buf[255];
        ccinPhraseTwoWordInfoList_t * tmp;
        tmp = im_context->lookup_result->lookup_two_word_list;
        while(tmp) {
            bzero(buf,255);
            strncpy(buf,(char*)tmp->lookup_phrase_word_two->phrase,6);
            tmp = tmp->next;
        }
#endif

        refresh_lookup_table ();

        return true;
    }

    return true;
}

uint32
CcinIMEngineInstance::get_inputed_string_length () {
    uint32 len = 0;
    for (uint32 i=0; i<m_inputed_keys.size (); ++i) {
        len += m_inputed_keys [i].length ();
    }
    return len;
}

void
CcinIMEngineInstance::lookup_table_page_up ()
{
}

void
CcinIMEngineInstance::lookup_table_page_down ()
{
}

void
CcinIMEngineInstance::select_candidate (unsigned int index)
{
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
