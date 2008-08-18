/** @file scim_pinyin_imengine.cpp
 * implementation of class PinyinInstance.
 */

/*
 * Smart Pinyin Input Method
 * 
 * Copyright (c) 2005 James Su <suzhe@tsinghua.org.cn>
 *
 * $Id: scim_pinyin_imengine.cpp,v 1.8 2006/06/12 01:31:14 suzhe Exp $
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
#include <scim.h>
#include "scim_pinyin_private.h"
#include "scim_special_table.h"
#include "scim_phrase.h"
#include "scim_pinyin.h"
#include "scim_pinyin_phrase.h"
#include "scim_native_lookup_table.h"
#include "scim_pinyin_global.h"
#include "scim_pinyin_imengine.h"
#include "scim_pinyin_smart_match.h"
#include "scim_pinyin_imengine_config_keys.h"

#define scim_module_init pinyin_LTX_scim_module_init
#define scim_module_exit pinyin_LTX_scim_module_exit
#define scim_imengine_module_init pinyin_LTX_scim_imengine_module_init
#define scim_imengine_module_create_factory pinyin_LTX_scim_imengine_module_create_factory


#define SCIM_PROP_STATUS                            "/IMEngine/Pinyin/Status"
#define SCIM_PROP_LETTER                            "/IMEngine/Pinyin/Letter"
#define SCIM_PROP_PUNCT                             "/IMEngine/Pinyin/Punct"

#define SCIM_PROP_PINYIN_SCHEME                     "/IMEngine/Pinyin/PinyinScheme"
#define SCIM_PROP_PINYIN_SCHEME_QUAN_PIN            "/IMEngine/Pinyin/PinyinScheme/QuanPin"
#define SCIM_PROP_PINYIN_SCHEME_SP_STONE            "/IMEngine/Pinyin/PinyinScheme/SP-STONE"
#define SCIM_PROP_PINYIN_SCHEME_SP_ZRM              "/IMEngine/Pinyin/PinyinScheme/SP-ZRM"
#define SCIM_PROP_PINYIN_SCHEME_SP_MS               "/IMEngine/Pinyin/PinyinScheme/SP-MS"
#define SCIM_PROP_PINYIN_SCHEME_SP_ZIGUANG          "/IMEngine/Pinyin/PinyinScheme/SP-ZIGUANG"
#define SCIM_PROP_PINYIN_SCHEME_SP_ABC              "/IMEngine/Pinyin/PinyinScheme/SP-ABC"
#define SCIM_PROP_PINYIN_SCHEME_SP_LIUSHI           "/IMEngine/Pinyin/PinyinScheme/SP-LIUSHI"

#ifndef SCIM_PINYIN_DATADIR
    #define SCIM_PINYIN_DATADIR "/usr/share/scim/pinyin"
#endif

#define SCIM_PINYIN_MAX_USER_PHRASE_LENGTH   15
#define SCIM_PINYIN_MAX_PREEDIT_LENGTH       80
#define SCIM_PINYIN_MAX_SMART_MATCH_LEVEL    100

#ifndef SCIM_ICONDIR
    #define SCIM_ICONDIR "/usr/share/scim/icons"
#endif

#ifndef SCIM_SMART_PINYIN_ICON_FILE
    #define SCIM_SMART_PINYIN_ICON_FILE  (SCIM_ICONDIR "/smart-pinyin.png")
#endif

#define SCIM_FULL_LETTER_ICON                             (SCIM_ICONDIR "/full-letter.png")
#define SCIM_HALF_LETTER_ICON                             (SCIM_ICONDIR "/half-letter.png")
#define SCIM_FULL_PUNCT_ICON                              (SCIM_ICONDIR "/full-punct.png")
#define SCIM_HALF_PUNCT_ICON                              (SCIM_ICONDIR "/half-punct.png")

static IMEngineFactoryPointer _scim_pinyin_factory (0); 

static ConfigPointer _scim_config (0);

static Property _status_property   (SCIM_PROP_STATUS, "");
static Property _letter_property   (SCIM_PROP_LETTER, "");
static Property _punct_property    (SCIM_PROP_PUNCT, "");

static Property _pinyin_scheme_property     (SCIM_PROP_PINYIN_SCHEME, "全");
static Property _pinyin_quan_pin_property   (SCIM_PROP_PINYIN_SCHEME_QUAN_PIN,   "全拼");
static Property _pinyin_sp_stone_property   (SCIM_PROP_PINYIN_SCHEME_SP_STONE,   "双拼-中文之星/四通利方");
static Property _pinyin_sp_zrm_property     (SCIM_PROP_PINYIN_SCHEME_SP_ZRM,     "双拼-自然码");
static Property _pinyin_sp_ms_property      (SCIM_PROP_PINYIN_SCHEME_SP_MS,      "双拼-微软拼音");
static Property _pinyin_sp_ziguang_property (SCIM_PROP_PINYIN_SCHEME_SP_ZIGUANG, "双拼-紫光拼音");
static Property _pinyin_sp_abc_property     (SCIM_PROP_PINYIN_SCHEME_SP_ABC,     "双拼-智能ABC");
static Property _pinyin_sp_liushi_property  (SCIM_PROP_PINYIN_SCHEME_SP_LIUSHI,  "双拼-刘氏");

extern "C" {
    void scim_module_init (void)
    {
        bindtextdomain (GETTEXT_PACKAGE, SCIM_PINYIN_LOCALEDIR);
        bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    }

    void scim_module_exit (void)
    {
        _scim_pinyin_factory.reset ();
        _scim_config.reset ();
    }

    uint32 scim_imengine_module_init (const ConfigPointer &config)
    {
        _status_property.set_tip (_("Current input method state. Click to change it."));
        _letter_property.set_tip (_("Input mode of the letters. Click to toggle between half and full."));
        _letter_property.set_label (_("Full/Half Letter"));
        _punct_property.set_tip (_("Input mode of the puncutations. Click to toggle between half and full."));
        _punct_property.set_label (_("Full/Half Punct"));

        _status_property.set_label ("英");
        _letter_property.set_icon (SCIM_HALF_LETTER_ICON);
        _punct_property.set_icon (SCIM_HALF_PUNCT_ICON);

        _scim_config = config;
        return 1;
    }

    IMEngineFactoryPointer scim_imengine_module_create_factory (uint32 engine)
    {
        if (engine != 0) return IMEngineFactoryPointer (0);
        if (_scim_pinyin_factory.null ()) {
            PinyinFactory *factory = new PinyinFactory (_scim_config); 
            if (factory && factory->valid ())
                _scim_pinyin_factory = factory;
            else
                delete factory;
        }
        return _scim_pinyin_factory;
    }
}

// implementation of PinyinFactory
PinyinFactory::PinyinFactory (const ConfigPointer &config)
    : m_config (config),
      m_pinyin_parser (0),
      m_match_longer_phrase (false),
      m_auto_combine_phrase (false),
      m_auto_fill_preedit (false),
      m_always_show_lookup (false),
      m_show_all_keys (false),
      m_user_data_binary (true),
      m_valid (false),
      m_shuang_pin (false),
      m_shuang_pin_scheme (SCIM_SHUANG_PIN_STONE),
      m_last_time ((time_t)0),
      m_save_period ((time_t)300),
      m_dynamic_sensitivity (6),
      m_smart_match_level (20),
      m_max_user_phrase_length (SCIM_PINYIN_MAX_USER_PHRASE_LENGTH),
      m_max_preedit_length (SCIM_PINYIN_MAX_PREEDIT_LENGTH)
{
    set_languages ("zh_CN,zh_TW,zh_HK,zh_SG");

    m_valid = init ();

    m_reload_signal_connection = m_config->signal_connect_reload (slot (this, &PinyinFactory::reload_config));
}

bool
PinyinFactory::init ()
{
    String sys_phrase_lib            (String (SCIM_PINYIN_DATADIR) +
                                     String (SCIM_PATH_DELIM_STRING) +
                                     String ("phrase_lib"));

    String sys_pinyin_table            (String (SCIM_PINYIN_DATADIR) +
                                     String (SCIM_PATH_DELIM_STRING) + 
                                     String ("pinyin_table"));

    String sys_pinyin_phrase_lib    (String (SCIM_PINYIN_DATADIR) +
                                     String (SCIM_PATH_DELIM_STRING) +
                                     String ("pinyin_phrase_lib"));

    String sys_pinyin_phrase_index    (String (SCIM_PINYIN_DATADIR) +
                                     String (SCIM_PATH_DELIM_STRING) +
                                     String ("pinyin_phrase_index"));

    String sys_special_table        (String (SCIM_PINYIN_DATADIR) +
                                     String (SCIM_PATH_DELIM_STRING) +
                                     String ("special_table"));

    String home_dir                 (scim_get_home_dir ());

    String str;

    uint32 burst_stack_size = 128;

    bool tone                = false;
    bool dynamic_adjust        = true;
    bool incomplete            = true;

    bool ambiguities [SCIM_PINYIN_AmbLast + 2] =
        { false,  false,  false,  false,
          false,  false,  false,  false,
          false,  false,  false };

    char *amb_names [SCIM_PINYIN_AmbLast + 2] =
        { "Any",  "ZhiZi","ChiCi","ShiSi",
          "NeLe", "LeRi", "FoHe", "AnAng",
          "EnEng","InIng", 0};


    m_user_data_directory     = home_dir +
                                String (SCIM_PATH_DELIM_STRING) +
                                String (".scim") + 
                                String (SCIM_PATH_DELIM_STRING) +
                                String ("pinyin");

    m_user_pinyin_table       = m_user_data_directory +
                                String (SCIM_PATH_DELIM_STRING) +
                                String ("pinyin_table");

    m_user_phrase_lib         = m_user_data_directory +
                                String (SCIM_PATH_DELIM_STRING) +
                                String ("phrase_lib");

    m_user_pinyin_phrase_lib  = m_user_data_directory +
                                String (SCIM_PATH_DELIM_STRING) +
                                String ("pinyin_phrase_lib");

    m_user_pinyin_phrase_index= m_user_data_directory +
                                String (SCIM_PATH_DELIM_STRING) +
                                String ("pinyin_phrase_index");

    String user_special_table  (m_user_data_directory +
                                String (SCIM_PATH_DELIM_STRING) +
                                String ("special_table"));

    if (m_config) {
        // Load configurations.
        tone =
            m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_TONE),
                            false);
        incomplete =
            m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_INCOMPLETE),
                            true);
        dynamic_adjust =
            m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_DYNAMIC_ADJUST),
                            true);

        m_match_longer_phrase =
            m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_MATCH_LONGER_PHRASE),
                            false);
        m_auto_combine_phrase =
            m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_AUTO_COMBINE_PHRASE),
                            true);
        m_auto_fill_preedit =
            m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_AUTO_FILL_PREEDIT),
                            true);

        m_always_show_lookup =
            m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_ALWAYS_SHOW_LOOKUP),
                            true);
        m_show_all_keys =
            m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_SHOW_ALL_KEYS),
                            false);
        m_user_data_binary =
            m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_USER_DATA_BINARY),
                            true);

        m_save_period = (time_t)
            m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_SAVE_PERIOD),
                            300);
        m_dynamic_sensitivity =
            m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_DYNAMIC_SENSITIVITY),
                            8);
        m_smart_match_level =
            m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_SMART_MATCH_LEVEL),
                            20);
        m_max_user_phrase_length =
            m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_MAX_USER_PHRASE_LENGTH),
                            8);
        m_max_preedit_length =
            m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_MAX_PREEDIT_LENGTH),
                            64);
        burst_stack_size =
            m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_BURST_STACK_SIZE),
                            128);

        sys_pinyin_table =
            m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_SYSTEM_PINYIN_TABLE),
                            sys_pinyin_table);
        sys_phrase_lib =
            m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_SYSTEM_PHRASE_LIB),
                            sys_phrase_lib);
        sys_pinyin_phrase_lib =
            m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_SYSTEM_PINYIN_PHRASE_LIB),
                            sys_pinyin_phrase_lib);
        sys_pinyin_phrase_index =
            m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_SYSTEM_PINYIN_PHRASE_INDEX),
                            sys_pinyin_phrase_index);

        sys_special_table =
            m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_SYSTEM_SPECIAL_TABLE),
                            sys_special_table);

        m_user_pinyin_table =
            m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_USER_PINYIN_TABLE),
                            m_user_pinyin_table);
        m_user_phrase_lib =
            m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_USER_PHRASE_LIB),
                            m_user_phrase_lib);
        m_user_pinyin_phrase_lib =
            m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_USER_PINYIN_PHRASE_LIB),
                            m_user_pinyin_phrase_lib);
        m_user_pinyin_phrase_index =
            m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_USER_PINYIN_PHRASE_INDEX),
                            m_user_pinyin_phrase_index);
        user_special_table =
            m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_USER_SPECIAL_TABLE),
                            user_special_table);

        // Adjust configurations
        if (m_dynamic_sensitivity > 16)
            m_dynamic_sensitivity = 16;
        if (m_smart_match_level > SCIM_PINYIN_MAX_SMART_MATCH_LEVEL)
            m_smart_match_level = SCIM_PINYIN_MAX_SMART_MATCH_LEVEL;
        if (m_max_user_phrase_length > SCIM_PINYIN_MAX_USER_PHRASE_LENGTH)
            m_max_user_phrase_length = SCIM_PINYIN_MAX_USER_PHRASE_LENGTH;
        if (m_max_preedit_length > SCIM_PINYIN_MAX_PREEDIT_LENGTH)
            m_max_preedit_length = SCIM_PINYIN_MAX_PREEDIT_LENGTH;

        // Not absolute path, prepend home dir.
        if (m_user_pinyin_table.length () &&
            m_user_pinyin_table [0] != SCIM_PATH_DELIM)
            m_user_pinyin_table =
                home_dir + SCIM_PATH_DELIM_STRING + m_user_pinyin_table;

        if (m_user_phrase_lib.length () &&
            m_user_phrase_lib [0] != SCIM_PATH_DELIM)
            m_user_phrase_lib =
                home_dir + SCIM_PATH_DELIM_STRING + m_user_phrase_lib;

        if (m_user_pinyin_phrase_lib.length () &&
            m_user_pinyin_phrase_lib [0] != SCIM_PATH_DELIM)
            m_user_pinyin_phrase_lib =
                home_dir + SCIM_PATH_DELIM_STRING + m_user_pinyin_phrase_lib;

        if (m_user_pinyin_phrase_index.length () &&
            m_user_pinyin_phrase_index [0] != SCIM_PATH_DELIM)
            m_user_pinyin_phrase_index =
                home_dir + SCIM_PATH_DELIM_STRING + m_user_pinyin_phrase_index;

        // Read ambiguities config.
        for (int i = 0; i <= SCIM_PINYIN_AmbLast; i++) {
            String amb = String (SCIM_CONFIG_IMENGINE_PINYIN_AMBIGUITY) + 
                             String ("/") +
                             String (amb_names [i]);

            ambiguities [i] = m_config->read (amb, ambiguities [i]);
        }


        // Read and store hotkeys config.

        //Read full width punctuation keys
        str = m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_FULL_WIDTH_PUNCT_KEY),
                              String ("Control+period"));

        scim_string_to_key_list (m_full_width_punct_keys, str);

        //Read full width letter keys
        str = m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_FULL_WIDTH_LETTER_KEY),
                              String ("Shift+space"));

        scim_string_to_key_list (m_full_width_letter_keys, str);

        //Read mode switch keys
        str = m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_MODE_SWITCH_KEY),
                              String ("Alt+Shift_L+KeyRelease,") + 
                              String ("Alt+Shift_R+KeyRelease,") +
                              String ("Shift+Shift_L+KeyRelease,") +
                              String ("Shift+Shift_R+KeyRelease"));
        
        scim_string_to_key_list (m_mode_switch_keys, str);

        //Read chinese switch keys
        str = m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_CHINESE_SWITCH_KEY),
                              String ("Control+slash"));
        
        scim_string_to_key_list (m_chinese_switch_keys, str);

        //Read page up keys
        str = m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_PAGE_UP_KEY),
                              String ("comma,minus,bracketleft,Page_Up"));
        
        scim_string_to_key_list (m_page_up_keys, str);

        //Read page down keys
        str = m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_PAGE_DOWN_KEY),
                              String ("period,equal,bracketright,Page_Down"));
        
        scim_string_to_key_list (m_page_down_keys, str);

        //Read disable phrase keys
        str = m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_DISABLE_PHRASE_KEY),
                              String ("Control+d"));

        scim_string_to_key_list (m_disable_phrase_keys, str);

        // Load Pinyin Scheme config
        m_shuang_pin =
            m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_SHUANG_PIN),
                            m_shuang_pin);

        if (m_shuang_pin) {
            int tmp;
            tmp = m_config->read (String (SCIM_CONFIG_IMENGINE_PINYIN_SHUANG_PIN_SCHEME),
                                  (int) m_shuang_pin_scheme);

            if (tmp >=0 && tmp <= SCIM_SHUANG_PIN_LIUSHI)
                m_shuang_pin_scheme = static_cast<PinyinShuangPinScheme> (tmp);
        }
    }

    if (m_save_period > 0 && m_save_period < 30)
        m_save_period = 30;

    m_name = utf8_mbstowcs (_("Smart Pinyin"));

    m_pinyin_global.toggle_tone (tone);
    m_pinyin_global.toggle_incomplete (incomplete);
    m_pinyin_global.toggle_dynamic_adjust (dynamic_adjust);

    // Is there any ambiguities?
    if (ambiguities [0]) {
        for (int i=1; i <= SCIM_PINYIN_AmbLast; i++) 
            m_pinyin_global.toggle_ambiguity (static_cast<PinyinAmbiguity> (i), ambiguities [i]);
    } else {
        m_pinyin_global.toggle_ambiguity (SCIM_PINYIN_AmbAny, false);
    }

    m_pinyin_global.update_custom_settings ();

    if (m_full_width_punct_keys.size () == 0)
        m_full_width_punct_keys.push_back (KeyEvent (SCIM_KEY_comma, SCIM_KEY_ControlMask));

    if (m_full_width_letter_keys.size () == 0)
        m_full_width_letter_keys.push_back (KeyEvent (SCIM_KEY_space, SCIM_KEY_ShiftMask));

    if (m_mode_switch_keys.size () == 0) {
        m_mode_switch_keys.push_back (KeyEvent (SCIM_KEY_Shift_L, SCIM_KEY_AltMask | SCIM_KEY_ReleaseMask));
        m_mode_switch_keys.push_back (KeyEvent (SCIM_KEY_Shift_R, SCIM_KEY_AltMask | SCIM_KEY_ReleaseMask));
        m_mode_switch_keys.push_back (KeyEvent (SCIM_KEY_Shift_L, SCIM_KEY_ShiftMask | SCIM_KEY_ReleaseMask));
        m_mode_switch_keys.push_back (KeyEvent (SCIM_KEY_Shift_R, SCIM_KEY_ShiftMask | SCIM_KEY_ReleaseMask));
    }

    if (m_chinese_switch_keys.size () == 0)
        m_chinese_switch_keys.push_back (KeyEvent (SCIM_KEY_slash, SCIM_KEY_ControlMask));

    if (m_page_up_keys.size () == 0) {
        m_page_up_keys.push_back (KeyEvent (SCIM_KEY_comma, 0));
        m_page_up_keys.push_back (KeyEvent (SCIM_KEY_minus, 0));
        m_page_up_keys.push_back (KeyEvent (SCIM_KEY_bracketleft, 0));
        m_page_up_keys.push_back (KeyEvent (SCIM_KEY_Page_Up, 0));
    }

    if (m_page_down_keys.size () == 0) {
        m_page_down_keys.push_back (KeyEvent (SCIM_KEY_period, 0));
        m_page_down_keys.push_back (KeyEvent (SCIM_KEY_equal, 0));
        m_page_down_keys.push_back (KeyEvent (SCIM_KEY_bracketright, 0));
        m_page_down_keys.push_back (KeyEvent (SCIM_KEY_Page_Down, 0));
    }

    if (m_disable_phrase_keys.size () == 0)
        m_disable_phrase_keys.push_back (KeyEvent (SCIM_KEY_d, SCIM_KEY_ControlMask));

    if (!m_pinyin_global.load_pinyin_table (sys_pinyin_table.c_str (),
                                            m_user_pinyin_table.c_str ())) {
        SCIM_DEBUG_IMENGINE (1) << "Pinyin Table load failed!\n";
        return false;
    }

    if (!m_pinyin_global.load_sys_phrase_lib (sys_phrase_lib.c_str (),
                                              sys_pinyin_phrase_lib.c_str (),
                                              sys_pinyin_phrase_index.c_str ())) {
        SCIM_DEBUG_IMENGINE (1) << "System Phrase Library load failed!\n";
        return false;
    }

    if (!m_pinyin_global.load_user_phrase_lib (m_user_phrase_lib.c_str (),
                                               m_user_pinyin_phrase_lib.c_str (),
                                               m_user_pinyin_phrase_index.c_str ()))
        SCIM_DEBUG_IMENGINE (1) << "User Phrase Library load failed!\n";

    m_last_time = time (0);

    m_pinyin_global.get_user_phrase_lib ()->set_burst_stack_size (burst_stack_size);

    ifstream is_sys_special_table (sys_special_table.c_str ());
    ifstream is_user_special_table (user_special_table.c_str ());

    if (is_sys_special_table)
        m_special_table.load (is_sys_special_table);

    if (is_user_special_table)
        m_special_table.load (is_user_special_table);

    init_pinyin_parser ();

    return true;
}

void
PinyinFactory::init_pinyin_parser ()
{
    if (m_pinyin_parser)
        delete m_pinyin_parser;

    if (m_shuang_pin)
        m_pinyin_parser = new PinyinShuangPinParser (m_shuang_pin_scheme);
    else
        m_pinyin_parser = new PinyinDefaultParser ();
}

PinyinFactory::~PinyinFactory ()
{
    if (m_valid)
        save_user_library ();

    m_reload_signal_connection.disconnect ();
}

WideString
PinyinFactory::get_name () const
{
    return m_name;
}

WideString
PinyinFactory::get_authors () const
{
    return utf8_mbstowcs (
                String (_("Copyright (C) 2002, 2003 James Su <suzhe@tsinghua.org.cn>")));
}

WideString
PinyinFactory::get_credits () const
{
    return WideString ();
}

WideString
PinyinFactory::get_help () const
{
    String full_width_letter;
    String full_width_punct;
    String chinese_switch;
    String mode_switch;
    String disable_phrase;
    String page_up;
    String page_down;
    String help;

    scim_key_list_to_string (full_width_letter, m_full_width_letter_keys);
    scim_key_list_to_string (full_width_punct, m_full_width_punct_keys);
    scim_key_list_to_string (chinese_switch, m_chinese_switch_keys);
    scim_key_list_to_string (mode_switch, m_mode_switch_keys);
    scim_key_list_to_string (disable_phrase, m_disable_phrase_keys);
    scim_key_list_to_string (page_up, m_page_up_keys);
    scim_key_list_to_string (page_down, m_page_down_keys);

    help =  String (_("Hot Keys:")) + 
            String (_("\n\n  ")) + full_width_letter + String (_(":\n")) +
            String (_("    Switch between full/half width letter mode.")) + 
            String (_("\n\n  ")) + full_width_punct + String (_(":\n")) +
            String (_("    Switch between full/half width punctuation mode.")) +
            String (_("\n\n  ")) + chinese_switch + String (_(":\n")) +
            String (_("    Switch between Simplified/Traditional Chinese mode.")) +
            String (_("\n\n  ")) + mode_switch + String (_(":\n")) +
            String (_("    Switch between English/Chinese mode.")) +
            String (_("\n\n  ")) + page_up + String (_(":\n")) +
            String (_("    Page up in lookup table.")) +
            String (_("\n\n  ")) + page_down + String (_(":\n")) +
            String (_("    Page down in lookup table.")) +
            String (_("\n\n  ")) + disable_phrase + String (_(":\n")) +
            String (_("    Disable the selected user created phrase.")) +
            String (_("\n\n  Esc:\n"
                      "    Reset the input method.\n")) +
            String (_("\n\n  v:\n"
                      "    Enter the English input mode.\n"
                      "    Press Space or Return to commit\n"
                      "    the inputed string and exit this mode.")) +
            String (_("\n\n  i:\n"
                      "    Enter the special input mode. For example:\n"
                      "      Input \"idate\" will give you the\n"
                      "      string of the current date.\n"
                      "      Input \"imath\" will give you the\n"
                      "      common mathematic symbols.\n"
                      "    For more information about this mode,\n"
                      "    please refer to\n"
                      "    /usr/share/scim/pinyin/special_table"));

    return utf8_mbstowcs (help);
}

String
PinyinFactory::get_uuid () const
{
    return String ("05235cfc-43ce-490c-b1b1-c5a2185276ae");
}

String
PinyinFactory::get_icon_file () const
{
    return String (SCIM_SMART_PINYIN_ICON_FILE);
}

IMEngineInstancePointer
PinyinFactory::create_instance (const String& encoding, int id)
{
    return new PinyinInstance (this, &m_pinyin_global, encoding, id);
}

void
PinyinFactory::refresh ()
{
    if (m_save_period == 0)
        return;

    time_t cur_time = time (0);

    if (cur_time < m_last_time || cur_time - m_last_time > m_save_period) {
        m_last_time = cur_time;
        save_user_library ();
    }
}

void
PinyinFactory::save_user_library ()
{
    // First make the user data directory.
    if (access (m_user_data_directory.c_str (), R_OK | W_OK) != 0) {
        mkdir (m_user_data_directory.c_str (), S_IRUSR | S_IWUSR | S_IXUSR);
        if (access (m_user_data_directory.c_str (), R_OK | W_OK) != 0)
            return;
    }

    PinyinPhraseLib *lib = m_pinyin_global.get_user_phrase_lib ();

    if (lib) {
        lib->optimize_phrase_relation_map ();
        lib->optimize_phrase_frequencies ();
    }

    m_pinyin_global.save_pinyin_table (m_user_pinyin_table.c_str (), m_user_data_binary);
    m_pinyin_global.save_user_phrase_lib (m_user_phrase_lib.c_str (),
                                          m_user_pinyin_phrase_lib.c_str (),
                                          m_user_pinyin_phrase_index.c_str (),
                                          m_user_data_binary);
}

void
PinyinFactory::reload_config (const ConfigPointer &config)
{
    m_config = config;
    m_valid = init ();
}

// implementation of PinyinInstance
PinyinInstance::PinyinInstance (PinyinFactory *factory,
                                PinyinGlobal *pinyin_global,
                                const String& encoding,
                                int id)
    : IMEngineInstanceBase (factory, encoding, id),
      m_factory (factory),
      m_pinyin_global (pinyin_global),
      m_pinyin_table (0),
      m_sys_phrase_lib (0),
      m_user_phrase_lib (0),
      m_double_quotation_state (false),
      m_single_quotation_state (false),
      m_forward (false),
      m_focused (false),
      m_simplified (true),
      m_traditional (true),
      m_lookup_table_def_page_size (9),
      m_keys_caret (0),
      m_lookup_caret (0),
      m_client_encoding (encoding),
      m_iconv (encoding)
{
    m_full_width_punctuation [0] = true;
    m_full_width_punctuation [1] = false;

    m_full_width_letter [0] = false;
    m_full_width_letter [1] = false;

    if (m_factory->valid () && m_pinyin_global) {
        m_pinyin_table = m_pinyin_global->get_pinyin_table ();
        m_sys_phrase_lib = m_pinyin_global->get_sys_phrase_lib ();
        m_user_phrase_lib = m_pinyin_global->get_user_phrase_lib ();
    }

    if (encoding == "GBK" || encoding == "GB2312") {
        m_simplified = true;
        m_traditional = false;
        m_chinese_iconv.set_encoding ("GB2312");
    } else if (encoding == "BIG5" || encoding == "BIG5-HKSCS") {
        m_simplified = false;
        m_traditional = true;
        m_chinese_iconv.set_encoding ("BIG5");
    } else {
        m_simplified = true;
        m_traditional = true;
        m_chinese_iconv.set_encoding ("");
    }

    m_reload_signal_connection = m_factory->m_config->signal_connect_reload (slot (this, &PinyinInstance::reload_config));

    init_lookup_table_labels ();
}

PinyinInstance::~PinyinInstance ()
{
    m_reload_signal_connection.disconnect ();
}

bool
PinyinInstance::process_key_event (const KeyEvent& key)
{
    if (!m_focused || !m_pinyin_table || !m_sys_phrase_lib) return false;

    // capture the mode switch key events
    if (match_key_event (m_factory->m_mode_switch_keys, key)/* && 
        m_inputed_string.length () == 0*/) {
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

    // capture the chinese switch key events
    if (match_key_event (m_factory->m_chinese_switch_keys, key)) {
        trigger_property (SCIM_PROP_STATUS);
        m_prev_key = key;
        return true;
    }

    m_prev_key = key;

    if (key.is_key_release ())
        return true;

    if (!m_forward) {
        //reset key
        if (key.code == SCIM_KEY_Escape && key.mask == 0) {
            if (m_inputed_string.length () == 0 &&
                m_converted_string.length () == 0 &&
                m_preedit_string.length () == 0)
                return false;

            reset ();
            return true;
        }

        if (!m_factory->m_shuang_pin) {
            //go into english mode
            if ((!m_inputed_string.length () && key.code == SCIM_KEY_v && key.mask == 0) ||
                is_english_mode ()) {
                return english_mode_process_key_event (key);
            }
 
            //go into english mode
            if ((!m_inputed_string.length () &&
                 key.code == SCIM_KEY_i && key.mask == 0 &&
                 m_factory->m_special_table.valid ()) ||
                is_special_mode ()) {
                return special_mode_process_key_event (key);
            }
        }

        //caret left
        if (key.code == SCIM_KEY_Left && key.mask == 0)
            return caret_left ();

        //caret right
        if (key.code == SCIM_KEY_Right && key.mask == 0)
            return caret_right ();

        //caret home
        if (key.code == SCIM_KEY_Home && key.mask == 0)
            return caret_left (true);

        //caret end
        if (key.code == SCIM_KEY_End && key.mask == 0)
            return caret_right (true);

        //lookup table cursor up
        if (key.code == SCIM_KEY_Up && key.mask == 0)
            return lookup_cursor_up ();

        //lookup table cursor down
        if (key.code == SCIM_KEY_Down && key.mask == 0)
            return lookup_cursor_down ();

        //lookup table page up
        if (match_key_event (m_factory->m_page_up_keys, key)) {
            if (lookup_page_up ()) return true;
            return post_process (key.get_ascii_code ());
        }

        //lookup table page down
        if (match_key_event (m_factory->m_page_down_keys, key)) {
            if (lookup_page_down ()) return true;
            return post_process (key.get_ascii_code ());
        }

        //backspace key
        if (key.code == SCIM_KEY_BackSpace) {
            if (key.mask == SCIM_KEY_ShiftMask)
                return erase_by_key ();
            else if (key.mask == 0)
                return erase ();
        }

        //delete key
        if (key.code == SCIM_KEY_Delete) {
            if (key.mask == SCIM_KEY_ShiftMask)
                return erase_by_key (false);
            else if (key.mask == 0)
                return erase (false);
        }

        //select lookup table
        if (m_pinyin_global->use_tone ()) {
            if (((key.code >= SCIM_KEY_6 && key.code <= SCIM_KEY_9) || key.code == SCIM_KEY_0) && key.mask == 0) {
                int index;
                if (key.code == SCIM_KEY_0) index = 4;
                else index = key.code - SCIM_KEY_6;
                if (lookup_select (index)) return true;
            }
        } else {
            if (key.code >= SCIM_KEY_1 && key.code <= SCIM_KEY_9 && key.mask == 0) {
                int index = key.code - SCIM_KEY_1;
                if (lookup_select (index)) return true;
            }
        }

        //space hit
        if (key.code == SCIM_KEY_space && key.mask == 0)
            return space_hit ();

        //enter hit
        if (key.code == SCIM_KEY_Return && key.mask == 0)
            return enter_hit ();

        // disable phrase key
        if (match_key_event (m_factory->m_disable_phrase_keys, key)) {
            return disable_phrase();
        }

        if ((key.mask & (~ (SCIM_KEY_ShiftMask + SCIM_KEY_CapsLockMask))) == 0)
            return insert (key.get_ascii_code ());
    }

    if ((key.mask & (~ (SCIM_KEY_ShiftMask + SCIM_KEY_CapsLockMask))) == 0)
        return post_process (key.get_ascii_code ());

    return false;
}

void
PinyinInstance::select_candidate (unsigned int item)
{
    if (is_special_mode ())
        special_mode_lookup_select (item);
    else
        lookup_select (item);
}

void
PinyinInstance::update_lookup_table_page_size (unsigned int page_size)
{
    if (page_size > 0)
        m_lookup_table.set_page_size (page_size);
}

void
PinyinInstance::lookup_table_page_up ()
{
    lookup_page_up ();
}

void
PinyinInstance::lookup_table_page_down ()
{
    lookup_page_down ();
}

void
PinyinInstance::move_preedit_caret (unsigned int /*pos*/)
{
}

void
PinyinInstance::reset ()
{
    String encoding = get_encoding ();

    if (m_client_encoding != encoding) {
        m_client_encoding = encoding;
        m_iconv.set_encoding (encoding);

        if (encoding == "GBK" || encoding == "GB2312") {
            m_simplified = true;
            m_traditional = false;
            m_chinese_iconv.set_encoding ("GB2312");
        } else if (encoding == "BIG5" || encoding == "BIG5-HKSCS") {
            m_simplified = false;
            m_traditional = true;
            m_chinese_iconv.set_encoding ("BIG5");
        }
    }

    m_double_quotation_state = false;
    m_single_quotation_state = false;

    m_lookup_table.clear ();

    m_inputed_string = String ();
    
    m_converted_string = WideString ();
    m_preedit_string = WideString ();

    std::vector < std::pair <int, int> > ().swap (m_keys_preedit_index);
    std::vector <PinyinParsedKey> ().swap (m_parsed_keys);
    std::vector <PhraseVector> ().swap (m_phrases_cache);
    std::vector <CharVector> ().swap (m_chars_cache);

    clear_selected (0);

    m_keys_caret = 0;
    m_lookup_caret = 0;

    hide_lookup_table ();
    hide_preedit_string ();
    hide_aux_string ();

    refresh_all_properties ();
}

void
PinyinInstance::focus_in ()
{
    m_focused = true;

    initialize_all_properties ();

    hide_preedit_string ();
    hide_aux_string ();

    init_lookup_table_labels ();

    if (!is_english_mode ()) {
        refresh_preedit_string ();
        refresh_preedit_caret ();
        refresh_aux_string ();

        if (m_lookup_table.number_of_candidates ()) {
            m_lookup_table.set_page_size (m_lookup_table_def_page_size);
            show_lookup_table ();
            update_lookup_table (m_lookup_table);
        }
    } else {
        english_mode_refresh_preedit ();
    }
}

void
PinyinInstance::focus_out ()
{
    m_focused = false;
}

void
PinyinInstance::trigger_property (const String &property)
{
    bool update_pinyin_scheme = false;

    if (property == SCIM_PROP_STATUS) {
        if (m_forward) {
            m_simplified = true;
            m_traditional = false;
            m_forward = false;
        } else if (!m_forward && m_simplified == true && m_traditional == false) {
            m_simplified = false;
            m_traditional = true;
        } else if (!m_forward && m_simplified == false && m_traditional == true) {
            m_simplified = true;
            m_traditional = true;
        } else if (!m_forward && m_simplified == true && m_traditional == true) {
            m_forward = true;
        }
 
        if (m_simplified && !m_traditional)
            m_chinese_iconv.set_encoding ("GB2312");
        else if (!m_simplified && m_traditional)
            m_chinese_iconv.set_encoding ("BIG5");
        else
            m_chinese_iconv.set_encoding ("");
 
        reset ();
    } else if (property == SCIM_PROP_LETTER) {
        int which = ((m_forward || is_english_mode ()) ? 1 : 0);
        m_full_width_letter [which] = !m_full_width_letter [which];

        refresh_letter_property ();
    } else if (property == SCIM_PROP_PUNCT) {
        int which = ((m_forward || is_english_mode ()) ? 1 : 0);
        m_full_width_punctuation [which] = !m_full_width_punctuation [which];

        refresh_punct_property ();
    } else if (property == SCIM_PROP_PINYIN_SCHEME_QUAN_PIN) {
        m_factory->m_shuang_pin = false;
        update_pinyin_scheme = true;
    } else if (property == SCIM_PROP_PINYIN_SCHEME_SP_STONE) {
        m_factory->m_shuang_pin = true;
        m_factory->m_shuang_pin_scheme = SCIM_SHUANG_PIN_STONE;
        update_pinyin_scheme = true;
    } else if (property == SCIM_PROP_PINYIN_SCHEME_SP_ZRM) {
        m_factory->m_shuang_pin = true;
        m_factory->m_shuang_pin_scheme = SCIM_SHUANG_PIN_ZRM;
        update_pinyin_scheme = true;
    } else if (property == SCIM_PROP_PINYIN_SCHEME_SP_MS) {
        m_factory->m_shuang_pin = true;
        m_factory->m_shuang_pin_scheme = SCIM_SHUANG_PIN_MS;
        update_pinyin_scheme = true;
    } else if (property == SCIM_PROP_PINYIN_SCHEME_SP_ZIGUANG) {
        m_factory->m_shuang_pin = true;
        m_factory->m_shuang_pin_scheme = SCIM_SHUANG_PIN_ZIGUANG;
        update_pinyin_scheme = true;
    } else if (property == SCIM_PROP_PINYIN_SCHEME_SP_ABC) {
        m_factory->m_shuang_pin = true;
        m_factory->m_shuang_pin_scheme = SCIM_SHUANG_PIN_ABC;
        update_pinyin_scheme = true;
    } else if (property == SCIM_PROP_PINYIN_SCHEME_SP_LIUSHI) {
        m_factory->m_shuang_pin = true;
        m_factory->m_shuang_pin_scheme = SCIM_SHUANG_PIN_LIUSHI;
        update_pinyin_scheme = true;
    }

    if (update_pinyin_scheme) {
        m_factory->init_pinyin_parser ();
        refresh_pinyin_scheme_property ();
        reset ();
        m_factory->m_config->write (String (SCIM_CONFIG_IMENGINE_PINYIN_SHUANG_PIN), m_factory->m_shuang_pin);
        m_factory->m_config->write (String (SCIM_CONFIG_IMENGINE_PINYIN_SHUANG_PIN_SCHEME), (int) m_factory->m_shuang_pin_scheme);
    }
}

void
PinyinInstance::refresh_aux_string ()
{
    if (m_factory->m_auto_fill_preedit) {
        WideString aux;
        AttributeList attrs;

        if (m_factory->m_show_all_keys) {
            for (size_t i = 0; i < m_parsed_keys.size (); ++i) {
                WideString key = utf8_mbstowcs (m_parsed_keys [i].get_key_string());
                if (m_lookup_caret == i) {
                    attrs.push_back (
                        Attribute (aux.length (),
                                   key.length (),
                                   SCIM_ATTR_DECORATE, SCIM_ATTR_DECORATE_REVERSE));
                }
                aux += key;
                aux.push_back (0x20);
            }
        } else {
            int i;
            if (m_parsed_keys.size () == 0) {
                aux = utf8_mbstowcs (m_inputed_string);
            } else if (m_keys_caret >= m_parsed_keys.size ()) {
                int parsed_len = m_parsed_keys.back ().get_end_pos ();
                for (i=parsed_len; i<(int) m_inputed_string.length (); ++i) {
                    aux += (ucs4_t) (m_inputed_string [i]);
                }
            } else {
                for (i=m_parsed_keys [m_keys_caret].get_pos ();
                     i < m_parsed_keys [m_keys_caret].get_end_pos (); ++i)
                    aux += (ucs4_t) (m_inputed_string [i]);
            }
    
            if (m_parsed_keys.size () &&
                m_keys_caret > 0 &&
                m_keys_caret <= m_parsed_keys.size ()) {
                aux.insert (aux.begin (), (ucs4_t) 0x0020);
    
                for (i=m_parsed_keys [m_keys_caret - 1].get_end_pos () - 1;
                     i >= m_parsed_keys [m_keys_caret - 1].get_pos (); --i)
                    aux = (ucs4_t) (m_inputed_string [i]) + aux;
            }
        }

        if (aux.length ()) {
            update_aux_string (aux, attrs);
            show_aux_string ();
        } else {
            hide_aux_string ();
        }

        return;
    }
}

void
PinyinInstance::initialize_all_properties ()
{
    PropertyList proplist;

    proplist.push_back (_pinyin_scheme_property);
    proplist.push_back (_pinyin_quan_pin_property);
    proplist.push_back (_pinyin_sp_stone_property);
    proplist.push_back (_pinyin_sp_zrm_property);
    proplist.push_back (_pinyin_sp_ms_property);
    proplist.push_back (_pinyin_sp_ziguang_property);
    proplist.push_back (_pinyin_sp_abc_property);
    proplist.push_back (_pinyin_sp_liushi_property);
    proplist.push_back (_status_property);
    proplist.push_back (_letter_property);
    proplist.push_back (_punct_property);

    register_properties (proplist);
    refresh_all_properties ();
    refresh_pinyin_scheme_property ();
}

void
PinyinInstance::refresh_all_properties ()
{
    refresh_status_property ();
    refresh_letter_property ();
    refresh_punct_property ();
}

void
PinyinInstance::refresh_status_property ()
{
    if (is_english_mode () || m_forward)
        _status_property.set_label ("英");
    else if (m_traditional && !m_simplified)
        _status_property.set_label ("繁");
    else if (m_simplified && !m_traditional)
        _status_property.set_label ("简");
    else
        _status_property.set_label ("中");

    update_property (_status_property);
}

void
PinyinInstance::refresh_letter_property ()
{
    _letter_property.set_icon (
        m_full_width_letter [(m_forward || is_english_mode ())?1:0] ? SCIM_FULL_LETTER_ICON : SCIM_HALF_LETTER_ICON);

    update_property (_letter_property);
}

void
PinyinInstance::refresh_punct_property ()
{
    _punct_property.set_icon (
        m_full_width_punctuation [(m_forward || is_english_mode ())?1:0] ? SCIM_FULL_PUNCT_ICON : SCIM_HALF_PUNCT_ICON);

    update_property (_punct_property);
}

void
PinyinInstance::refresh_pinyin_scheme_property ()
{
    String tip;

    if (m_factory->m_shuang_pin) {
        switch (m_factory->m_shuang_pin_scheme) {
            case SCIM_SHUANG_PIN_STONE:
                tip = _pinyin_sp_stone_property.get_label ();
                break;
            case SCIM_SHUANG_PIN_ZRM:
                tip = _pinyin_sp_zrm_property.get_label ();
                break;
            case SCIM_SHUANG_PIN_MS:
                tip = _pinyin_sp_ms_property.get_label ();
                break;
            case SCIM_SHUANG_PIN_ZIGUANG:
                tip = _pinyin_sp_ziguang_property.get_label ();
                break;
            case SCIM_SHUANG_PIN_ABC:
                tip = _pinyin_sp_abc_property.get_label ();
                break;
            case SCIM_SHUANG_PIN_LIUSHI:
                tip = _pinyin_sp_liushi_property.get_label ();
                break;
        }
        _pinyin_scheme_property.set_label ("双");
    } else {
        tip = _pinyin_quan_pin_property.get_label ();
        _pinyin_scheme_property.set_label ("全");
    }

    _pinyin_scheme_property.set_tip (tip);
    update_property (_pinyin_scheme_property);
}

void
PinyinInstance::init_lookup_table_labels ()
{
    std::vector<WideString> candidate_labels;
    char buf [2] = { 0, 0 };

    if (m_pinyin_global->use_tone ()) {
        for (int i=5; i<9; i++) {
            buf [0] = '1' + i;
            candidate_labels.push_back (utf8_mbstowcs (buf));
        }

        buf [0] = '0';
        candidate_labels.push_back (utf8_mbstowcs (buf));
    } else {
        for (int i=0; i<9; i++) {
            buf [0] = '1' + i;
            candidate_labels.push_back (utf8_mbstowcs (buf));
        }
    }

    m_lookup_table_def_page_size = (int) candidate_labels.size ();

    m_lookup_table.set_page_size (m_lookup_table_def_page_size);
    m_lookup_table.set_candidate_labels (candidate_labels);
    m_lookup_table.show_cursor ();
}

bool
PinyinInstance::caret_left (bool home)
{
    if (m_inputed_string.length ()) {
        if (m_keys_caret > 0) {
            if (home) m_keys_caret = 0;
            else m_keys_caret --;

            if (m_keys_caret <= (int) m_converted_string.length () &&
                m_keys_caret <= (int) m_parsed_keys.size ()) {
                m_lookup_caret = m_keys_caret;
                refresh_preedit_string ();
                refresh_lookup_table (-1);
            }
            refresh_aux_string ();

            refresh_preedit_caret ();
        } else {
            return caret_right (true);
        }
        return true;
    }
    return false;
}

bool
PinyinInstance::caret_right (bool end)
{
    if (m_inputed_string.length ()) {
        if (m_keys_caret <= (int) m_parsed_keys.size ()) {
            if (end) {
                if (has_unparsed_chars ())
                    m_keys_caret = m_parsed_keys.size () + 1;
                else
                    m_keys_caret = m_parsed_keys.size ();
            } else {
                m_keys_caret ++;
            }

            if (!has_unparsed_chars () && m_keys_caret > (int) m_parsed_keys.size ())
                return caret_left (true);

            if (m_keys_caret <= (int) m_converted_string.length () &&
                m_keys_caret <= (int) m_parsed_keys.size ()) {
                m_lookup_caret = m_keys_caret;
                refresh_preedit_string ();
                refresh_lookup_table (-1);
            }
            refresh_aux_string ();

            refresh_preedit_caret ();
        } else {
            return caret_left (true);
        }
        return true;
    }
    return false;
}

bool
PinyinInstance::validate_insert_key (char key)
{
    if (m_pinyin_global->use_tone () && key >= '1' && key <= '5')
        return true;

    if (m_factory->m_shuang_pin && key == ';')
        return true;

    if ((key >= 'a' && key <= 'z') || key == '\'')
        return true;

    return false;
}

bool
PinyinInstance::insert (char key)
{
    if (key == 0) return false;

    std::vector <PinyinParsedKey> old_parsed_keys = m_parsed_keys;
    String old_inputed_string = m_inputed_string;
    PinyinKeyExactEqualTo equal;

    if (validate_insert_key (key)) {
        int inputed_caret = calc_inputed_caret ();

        if ((m_parsed_keys.size () != 0 &&
             m_inputed_string.length () - m_parsed_keys.back ().get_end_pos () > SCIM_PINYIN_KEY_MAXLEN) ||
            (m_parsed_keys.size () == 0 &&
             m_inputed_string.length () > SCIM_PINYIN_KEY_MAXLEN))
            return true;

        if (inputed_caret == 0 && ((key >= '1' && key <= '5') || key == '\'' || key == ';'))
            return post_process (key);

        String::iterator i = m_inputed_string.begin () + inputed_caret;

        if (key != '\'') {
            m_inputed_string.insert (i, key);
        } else {
            // Don't insert more than one split chars in one place.
            if ((i != m_inputed_string.begin () && *(i-1) == '\'') ||
                (i != m_inputed_string.end () && *i == '\''))
                return true;
            m_inputed_string.insert (i, key);
        }

        calc_parsed_keys ();

        if (m_parsed_keys.size () > m_factory->m_max_preedit_length) {
            m_inputed_string = old_inputed_string;
            m_parsed_keys = old_parsed_keys;
            return true;
        }

        unsigned int len;
        for (len = 0; len < m_parsed_keys.size () && len < old_parsed_keys.size (); ++ len) {
            if (!equal (m_parsed_keys [len], old_parsed_keys [len]))
                break;
        }

        if (m_converted_string.length () > len)
            m_converted_string.erase (m_converted_string.begin () + len, m_converted_string.end ());

        m_keys_caret = inputed_caret_to_key_index (inputed_caret + 1);

        if (m_keys_caret <= (int) m_converted_string.length ())
            m_lookup_caret = m_keys_caret;

        else if (m_lookup_caret > (int) m_converted_string.length ())
            m_lookup_caret = m_converted_string.length ();

        bool calc_lookup = auto_fill_preedit (len);

        calc_keys_preedit_index ();
        refresh_preedit_string ();
        refresh_preedit_caret ();
        refresh_aux_string ();
        refresh_lookup_table (len, calc_lookup);
        return true;
    }

    return post_process (key);
}

bool
PinyinInstance::erase (bool backspace)
{
    if (m_inputed_string.length ()) {
        std::vector <PinyinParsedKey> old_parsed_keys = m_parsed_keys;

        int inputed_caret = calc_inputed_caret ();

        if (!backspace && inputed_caret < (int) m_inputed_string.length ())
            inputed_caret ++;

        if (inputed_caret > 0) {
            m_inputed_string.erase (inputed_caret - 1, 1);

            calc_parsed_keys ();

            m_keys_caret = inputed_caret_to_key_index (inputed_caret - 1);

            unsigned int len;
            PinyinKeyExactEqualTo equal;

            for (len = 0; len < m_parsed_keys.size () && len < old_parsed_keys.size (); len ++) {
                if (!equal (m_parsed_keys [len], old_parsed_keys [len]))
                    break;
            }

            if (m_converted_string.length () > len)
                m_converted_string.erase (m_converted_string.begin () + len, m_converted_string.end ());

            if (m_keys_caret <= (int) m_converted_string.length () &&
                m_lookup_caret > m_keys_caret)
                m_lookup_caret = m_keys_caret;
            else if (m_lookup_caret > (int) m_converted_string.length ())
                m_lookup_caret = m_converted_string.length ();

            bool calc_lookup = auto_fill_preedit (len);

            calc_keys_preedit_index ();
            refresh_preedit_string ();
            refresh_preedit_caret ();
            refresh_aux_string ();
            refresh_lookup_table (len, calc_lookup);
        }

        return true;
    }
    return false;
}

bool
PinyinInstance::erase_by_key (bool backspace)
{
    if (m_inputed_string.length ()) {
        // If there is no completed key, then call ordinary erase function
        // to erase a char.
        if (!m_parsed_keys.size ())
            return erase (backspace);

        // If the caret is at the end of parsed keys and there are unparsed
        // chars left, and it's not backspace then call erase.
        if (has_unparsed_chars () && m_keys_caret >= m_parsed_keys.size ()) {
            String unparsed_chars = m_inputed_string.substr (m_parsed_keys.back ().get_end_pos ());

            // Only one unparsed split char, delete it directly
            if (unparsed_chars.length () == 1 && unparsed_chars [0] == '\'') {
                m_inputed_string.erase (m_inputed_string.begin () +
                                        m_parsed_keys.back ().get_end_pos ());
            } else if ((m_keys_caret > m_parsed_keys.size ()) ||
                        (m_keys_caret == m_parsed_keys.size () && !backspace)) {
                return erase (backspace);
            }

            m_keys_caret = m_parsed_keys.size ();
        }

        // If the caret is at the beginning of the preedit string and
        // it's a backspace, then do nothing.
        if (backspace && !m_keys_caret)
            return true;

        int inputed_caret = m_keys_caret;

        if (!backspace && inputed_caret < (int) m_parsed_keys.size())
            ++ inputed_caret;

        if (inputed_caret > 0) {
            -- inputed_caret;

            int erased_pos = m_parsed_keys [inputed_caret].get_pos ();
            int erased_len = m_parsed_keys [inputed_caret].get_length ();

            m_inputed_string.erase (erased_pos, erased_len);

            // Check whether should insert a split char between the previous and the next key.
            if (erased_pos && erased_pos < m_inputed_string.length ()) {
                if (m_inputed_string [erased_pos-1] != '\'' &&
                    m_inputed_string [erased_pos] != '\'') {
                    m_inputed_string.insert (m_inputed_string.begin () + erased_pos, '\'');
                    erased_len --;
                } else if (m_inputed_string [erased_pos-1] == '\'' &&
                    m_inputed_string [erased_pos] == '\'') {
                    m_inputed_string.erase (m_inputed_string.begin () + erased_pos);
                    erased_len ++;
                }
            }

            m_parsed_keys.erase (m_parsed_keys.begin () + inputed_caret);

            for (int i = inputed_caret; i < m_parsed_keys.size (); ++ i) {
                m_parsed_keys [i].set_pos (m_parsed_keys [i].get_pos () - erased_len);
            }

            m_keys_caret = inputed_caret;

            if (m_converted_string.length () > inputed_caret)
                m_converted_string.erase (inputed_caret, 1);

            if (m_keys_caret <= (int) m_converted_string.length () &&
                m_lookup_caret > m_keys_caret)
                m_lookup_caret = m_keys_caret;
            else if (m_lookup_caret > (int) m_converted_string.length ())
                m_lookup_caret = m_converted_string.length ();

            bool calc_lookup = auto_fill_preedit (inputed_caret);

            calc_keys_preedit_index ();
            refresh_preedit_string ();
            refresh_preedit_caret ();
            refresh_aux_string ();
            refresh_lookup_table (inputed_caret, calc_lookup);
        }

        return true;
    }
    return false;
}

bool
PinyinInstance::space_hit ()
{
    if (m_inputed_string.length ()) {
        if (m_converted_string.length () == 0 && m_lookup_table.number_of_candidates () == 0)
            return true;

        if (m_lookup_table.number_of_candidates () &&
            (m_converted_string.length () <= m_parsed_keys.size () ||
             m_keys_caret == m_lookup_caret))
            lookup_to_converted (m_lookup_table.get_cursor_pos ());

        int invalid_pos = -1;

        if (m_converted_string.length () >= m_parsed_keys.size ()) {
            if (!m_factory->m_auto_fill_preedit || m_lookup_caret == m_parsed_keys.size ()) {
                commit_converted ();
                invalid_pos = 0;
            } else {
                m_keys_caret = m_lookup_caret = m_parsed_keys.size ();
            }
        }

        bool calc_lookup = auto_fill_preedit (invalid_pos);

        calc_keys_preedit_index ();
        refresh_preedit_string ();
        refresh_preedit_caret ();
        refresh_aux_string ();
        refresh_lookup_table (invalid_pos, calc_lookup);

        return true;
    }
    return post_process (' ');
}

bool
PinyinInstance::enter_hit ()
{
    if (m_inputed_string.length ()) {
        WideString str = utf8_mbstowcs (m_inputed_string);
        reset ();
        commit_string (str);
        return true;
    }
    return false;
}

bool
PinyinInstance::lookup_cursor_up ()
{
    if (m_inputed_string.length () && m_lookup_table.number_of_candidates ()) {
        m_lookup_table.cursor_up ();
        m_lookup_table.set_page_size (m_lookup_table_def_page_size);
        update_lookup_table (m_lookup_table);
        return true;
    }
    return false;
}

bool
PinyinInstance::lookup_cursor_down ()
{
    if (m_inputed_string.length () && m_lookup_table.number_of_candidates ()) {
        m_lookup_table.cursor_down ();
        m_lookup_table.set_page_size (m_lookup_table_def_page_size);
        update_lookup_table (m_lookup_table);
        return true;
    }
    return false;
}

bool
PinyinInstance::lookup_page_up ()
{
    if (m_inputed_string.length () && m_lookup_table.number_of_candidates ()) {
        m_lookup_table.page_up ();
        m_lookup_table.set_page_size (m_lookup_table_def_page_size);
        update_lookup_table (m_lookup_table);
        return true;
    }
    return false;
}

bool
PinyinInstance::lookup_page_down ()
{
    if (m_inputed_string.length () && m_lookup_table.number_of_candidates ()) {
        m_lookup_table.page_down ();
        m_lookup_table.set_page_size (m_lookup_table_def_page_size);
        update_lookup_table (m_lookup_table);
        return true;
    }
    return false;
}

bool
PinyinInstance::lookup_select (int index)
{
    if (m_inputed_string.length ()) {
        if (m_lookup_table.number_of_candidates () == 0)
            return true;

        index += m_lookup_table.get_current_page_start ();

        lookup_to_converted (index);

        int invalid_pos = -1;

        if (m_converted_string.length () >= m_parsed_keys.size () &&
            m_lookup_caret == (int) m_converted_string.length ()) {
            commit_converted ();
            invalid_pos = 0;
        }

        bool calc_lookup = auto_fill_preedit (invalid_pos);

        calc_keys_preedit_index ();
        refresh_preedit_string ();
        refresh_preedit_caret ();
        refresh_aux_string ();
        refresh_lookup_table (invalid_pos, calc_lookup);

        return true;
    }

    return false;
}

bool
PinyinInstance::post_process (char key)
{
    if (m_inputed_string.length () > 0) {
        if (m_converted_string.length () == m_parsed_keys.size () && !has_unparsed_chars ()) {
            commit_converted ();
            calc_keys_preedit_index ();
            refresh_preedit_string ();
            refresh_preedit_caret ();
            refresh_aux_string ();
            refresh_lookup_table (0);
        } else {
            return true;
        }
    }

    if ((ispunct (key) && m_full_width_punctuation [m_forward?1:0]) ||
        ((isalnum (key) || key == 0x20) && m_full_width_letter [m_forward?1:0])) {
        commit_string (convert_to_full_width (key));
        return true;
    }

    return false;
}

void
PinyinInstance::lookup_to_converted (int index)
{
    if (index < 0 || index >= (int) m_lookup_table.number_of_candidates ())
        return;

    WideString str = m_lookup_table.get_candidate (index);

    if (m_lookup_caret < (int) m_converted_string.length ()) {
        m_converted_string.erase (m_lookup_caret, min (str.length (), m_converted_string.length () - m_lookup_caret));
    }

    m_converted_string.insert (m_lookup_caret, str);

    if (m_pinyin_global && m_pinyin_global->use_dynamic_adjust ()) {
        if (m_lookup_table.is_string (index))
            store_selected_string (m_lookup_caret, str, m_converted_string);
        else if (m_lookup_table.is_phrase (index))
            store_selected_phrase (m_lookup_caret, m_lookup_table.get_phrase (index), m_converted_string);
        else {
            Phrase phrase;
            if (m_user_phrase_lib && m_user_phrase_lib->valid ())
                phrase = m_user_phrase_lib->find (str);

            if (!phrase.valid () && m_sys_phrase_lib && m_sys_phrase_lib->valid ())
                phrase = m_sys_phrase_lib->find (str);

            if (phrase.valid ())
                store_selected_phrase (m_lookup_caret, phrase, m_converted_string);
        }
    }

    m_lookup_caret += str.length ();

    if (m_keys_caret < m_lookup_caret)
        m_keys_caret = m_lookup_caret;
}

void
PinyinInstance::commit_converted ()
{
    if (m_converted_string.length ()) {

        // Clear the preedit string to avoid screen blinking
        update_preedit_string (WideString (), AttributeList ());

        commit_string (m_converted_string);

        if (m_pinyin_global && m_pinyin_global->use_dynamic_adjust ()) {
            dynamic_adjust_selected ();
            add_new_phrase (m_converted_string, m_parsed_keys);
            clear_selected (0);
            m_factory->refresh ();
        }

        if (m_converted_string.length () <= m_parsed_keys.size ()) {
            m_keys_caret -= m_converted_string.length ();
            m_inputed_string.erase (0, m_parsed_keys [m_converted_string.length () - 1].get_end_pos ());
        } else {
            m_keys_caret -= m_parsed_keys.size ();
            m_inputed_string.erase (0, m_parsed_keys.back ().get_end_pos ());
        }

        if (m_keys_caret < 0) m_keys_caret = 0;

        m_converted_string = WideString ();
        m_lookup_caret = 0;

        calc_parsed_keys ();
    }
}

Phrase
PinyinInstance::add_new_phrase (const WideString &phrase, const PinyinParsedKeyVector &pkeys, bool refresh)
{
    Phrase newph;

#if ENABLE_DEBUG
    std::cerr << "Add New Phrase: " << utf8_wcstombs (phrase) << " (";
    for (unsigned int i=0; i<pkeys.size (); ++i) {
        std::cerr << pkeys [i] << " ";
    }
    std::cerr << ")\n";
#endif

    if (m_user_phrase_lib && m_user_phrase_lib->valid () && phrase.length () >= 1) {
        /*
         * First check if the phrase is already in user phrase library,
         * if not, then add it, otherwise refresh it.
         */
        newph = m_user_phrase_lib->find (phrase);
        if (!newph.valid () || !newph.is_enable ()) {
            PinyinKeyVector keys;
            for (PinyinParsedKeyVector::const_iterator i=pkeys.begin (); i!=pkeys.end (); i++)
                keys.push_back (*i);

            /*
             * If the phrase is in sys phrase library, copy it to user phrase lib.
             */
            Phrase oldph; 

            if (m_sys_phrase_lib && m_sys_phrase_lib->valid ())
                oldph = m_sys_phrase_lib->find (phrase);

            if (oldph.valid ()) {
                newph = m_user_phrase_lib->append (oldph, keys);
            } else if (phrase.length () <= m_factory->m_max_user_phrase_length) {

                newph = m_user_phrase_lib->append (phrase, keys);

                if (newph.valid () && newph.is_enable ()) {
                    // Give an initial frequency to the new phrase.
                    uint32 freq = 0;
                    if (m_pinyin_table) {
                        for (unsigned int i = 0; i < newph.length (); ++ i)
                            freq += m_pinyin_table->get_char_frequency (newph [i], pkeys [i]);
                        freq /= (1 << (2 * newph.length () - 1));
                    }

                    newph.set_frequency (freq + 1);
                }
            }
        }
    }

    if (newph.valid () && newph.is_enable () && refresh) {
        if (newph.length () > 1)
            newph.refresh (SCIM_PHRASE_MAX_FREQ_BITS - m_factory->m_dynamic_sensitivity);
        else if (m_pinyin_table)
            m_pinyin_table->refresh (newph [0], 31 - m_factory->m_dynamic_sensitivity, pkeys [0]);
    }

    return newph;
}

void
PinyinInstance::refresh_preedit_string ()
{
    calc_preedit_string ();

    if (m_preedit_string.length ()) {
        AttributeList attrs;

        if (m_lookup_caret >= 0 && m_lookup_caret < (int) m_keys_preedit_index.size ()) {
            attrs.push_back (
                    Attribute(m_keys_preedit_index [m_lookup_caret].first,
                              m_keys_preedit_index [m_lookup_caret].second -
                                m_keys_preedit_index [m_lookup_caret].first,
                              SCIM_ATTR_DECORATE, SCIM_ATTR_DECORATE_REVERSE));
        }

        update_preedit_string (m_preedit_string, attrs);
        show_preedit_string ();
    } else {
        hide_preedit_string ();
    }
}

void
PinyinInstance::refresh_lookup_table (int invalid_pos, bool calc)
{
    if (calc) calc_lookup_table (invalid_pos);

    if (m_lookup_table.number_of_candidates ()) {
        if (m_factory->m_always_show_lookup ||
            !m_factory->m_auto_fill_preedit ||
            m_lookup_caret == m_keys_caret) {
            update_lookup_table (m_lookup_table);
            show_lookup_table ();
            return;
        }
    }
    hide_lookup_table ();
}

void
PinyinInstance::refresh_preedit_caret ()
{
    if (m_inputed_string.length ()) {
        int caret = calc_preedit_caret ();
        update_preedit_caret (caret);
    }
}

void
PinyinInstance::calc_parsed_keys ()
{
    m_factory->m_pinyin_parser->parse (m_pinyin_global->get_pinyin_validator (),
                                       m_parsed_keys,
                                       m_inputed_string.c_str ());
}

void
PinyinInstance::calc_lookup_table (int invalid_pos,
                                         WideString *combined,
                                         PhraseVector *matched_phrases)
{
    m_lookup_table.clear ();
    m_lookup_table.set_page_size (m_lookup_table_def_page_size);

    if (combined) *combined = WideString ();
    if (matched_phrases) matched_phrases->clear ();

    if (m_parsed_keys.size () == 0) return;

    WideString str; 
    bool       longer_phrase = false;

    PinyinParsedKeyVector::const_iterator begin   = m_parsed_keys.begin () + m_lookup_caret;
    PinyinParsedKeyVector::const_iterator end     = m_parsed_keys.end ();
    PinyinParsedKeyVector::const_iterator invalid;

    if (begin >= end) return;

    if (invalid_pos >= 0)
        invalid = m_parsed_keys.begin () + invalid_pos;
    else
        invalid = end;

    if (m_factory->m_match_longer_phrase && !m_factory->m_auto_combine_phrase && (end - begin) > 4)
        longer_phrase = true;

    scim_pinyin_update_matches_cache (
                m_chars_cache,
                m_phrases_cache,
                m_parsed_keys.begin (),
                m_parsed_keys.end (),
                invalid,
                m_pinyin_table,
                m_user_phrase_lib,
                m_sys_phrase_lib,
                &m_iconv,
                (m_simplified && m_traditional)? 0 : (&m_chinese_iconv),
                false,
                longer_phrase);

    // If auto_combine_phrase is enable, then use smart match func
    // to create the content of lookup table
    if ((m_factory->m_auto_combine_phrase ||
         (m_factory->m_auto_fill_preedit && combined))) {

        WideString acp;
        WideString first;
 
        PhraseVector mphrases;

        acp = scim_pinyin_smart_match (
                mphrases,
                m_chars_cache.begin () + m_lookup_caret,
                m_phrases_cache.begin () + m_lookup_caret,
                begin,
                end,
                m_pinyin_table,
                m_user_phrase_lib,
                m_sys_phrase_lib,
                m_factory->m_smart_match_level,
                &m_iconv,
                (m_simplified && m_traditional)? 0 : (&m_chinese_iconv));

        if (m_phrases_cache [m_lookup_caret].size ()) {
            first = m_phrases_cache [m_lookup_caret] [0].get_content ();
        } else if (m_chars_cache [m_lookup_caret].size ()) {
            first += m_chars_cache [m_lookup_caret] [0];
        }

        if (m_factory->m_auto_combine_phrase && first != acp && acp.length ()) {
            m_lookup_table.append_entry (acp);
        }

        if (combined) {
            *combined = acp;
        }

        if (matched_phrases) {
            matched_phrases->swap (mphrases);
        }
    }

    if (!m_phrases_cache [m_lookup_caret].size () || !m_chars_cache [m_lookup_caret].size ()) {
        scim_pinyin_search_matches (
                m_chars_cache [m_lookup_caret],
                m_phrases_cache [m_lookup_caret],
                begin,
                end,
                m_pinyin_table,
                m_user_phrase_lib,
                m_sys_phrase_lib,
                &m_iconv,
                (m_simplified && m_traditional)? 0 : (&m_chinese_iconv),
                true,
                longer_phrase);
    }

    if (m_phrases_cache [m_lookup_caret].size ()) {
        for (PhraseVector::iterator i = m_phrases_cache [m_lookup_caret].begin ();
                i != m_phrases_cache [m_lookup_caret].end (); ++ i)
            m_lookup_table.append_entry (*i);
    } 

    if (m_chars_cache [m_lookup_caret].size ()) {
        for (CharVector::iterator i = m_chars_cache [m_lookup_caret].begin ();
                i != m_chars_cache [m_lookup_caret].end (); ++ i)
            m_lookup_table.append_entry (*i);
    } 
}

void
PinyinInstance::calc_keys_preedit_index ()
{
    m_keys_preedit_index.clear ();

    int numkeys = (int) m_parsed_keys.size ();
    int numconvs = (int) m_converted_string.length ();
    int i;
    int len;

    std::pair <int, int> kpi;

    //Insert position index for converted string
    for (i = 0; i < numconvs; i++) {
        kpi.first = i;
        kpi.second = i+1;
        m_keys_preedit_index.push_back (kpi);
    }

    len = numconvs;

    for (i = numconvs; i < numkeys; i++) {
        kpi.first = len;
        kpi.second = len + m_parsed_keys [i].get_length ();
        len += (m_parsed_keys [i].get_length () + 1);
        m_keys_preedit_index.push_back (kpi);
    }
}

void
PinyinInstance::calc_preedit_string ()
{
    m_preedit_string = WideString ();

    if (m_inputed_string.length () == 0) {
        return;
    }

    WideString unparsed_string;
    unsigned int i;

    m_preedit_string = m_converted_string;

    for (i = m_converted_string.length (); i<m_parsed_keys.size (); i++) {
        int endpos = m_parsed_keys [i].get_end_pos ();
        for (int j = m_parsed_keys [i].get_pos (); j<endpos; j++) {
            m_preedit_string += ((ucs4_t) m_inputed_string [j]);
        }
        m_preedit_string += (ucs4_t) 0x0020;
    }

    if (m_parsed_keys.size () == 0) {
        unparsed_string = utf8_mbstowcs (m_inputed_string);
    } else {
        int parsed_len = m_parsed_keys.back ().get_end_pos ();
        for (i=parsed_len; i<m_inputed_string.length (); i++) {
            unparsed_string += (ucs4_t) (m_inputed_string [i]);
        }
    }

    if (unparsed_string.length ())
        m_preedit_string += unparsed_string;
}

int
PinyinInstance::calc_inputed_caret ()
{
    int caret;
    if (m_keys_caret <= 0)
        caret = 0;
    else if (m_keys_caret < (int) m_parsed_keys.size ()) {
        caret = m_parsed_keys [m_keys_caret].get_pos ();
    } else if (m_keys_caret == (int) m_parsed_keys.size ()) {
        caret = m_parsed_keys [m_keys_caret-1].get_end_pos ();
        if (caret < (int)m_inputed_string.length () && m_inputed_string [caret] == '\'')
            caret ++;
    } else {
        caret = m_inputed_string.length ();
    }

    return caret;
}

int
PinyinInstance::calc_preedit_caret ()
{
    int caret;
    if (m_keys_caret <= 0)
        caret = 0;
    else if (m_keys_caret < (int)m_keys_preedit_index.size ()) {
        caret = m_keys_preedit_index [m_keys_caret].first;
    } else if (m_keys_caret == (int)m_keys_preedit_index.size ()) {
        caret = m_keys_preedit_index [m_keys_caret-1].second;
    } else {
        caret = m_preedit_string.length ();
    }
    return caret;
}

int
PinyinInstance::inputed_caret_to_key_index (int caret)
{
    if (m_parsed_keys.size () == 0) {
        if (caret > 0) return 1;
        else return 0;
    }

    for (int i=0; i<(int)m_parsed_keys.size (); i++) {
        if (caret >= m_parsed_keys [i].get_pos () && caret < m_parsed_keys [i].get_end_pos ())
            return i;
    }

    if (caret == m_parsed_keys.back ().get_end_pos ())
        return m_parsed_keys.size ();

    return m_parsed_keys.size () + 1;
}

bool
PinyinInstance::has_unparsed_chars ()
{
    if (m_inputed_string.length () == 0) 
        return false;
    if (m_parsed_keys.size () == 0)
        return true;
    if (m_parsed_keys.back ().get_end_pos () < (int)m_inputed_string.length ())
        return true;
    return false;
}

bool
PinyinInstance::match_key_event (const std::vector <KeyEvent>& keyvec, const KeyEvent& key)
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
PinyinInstance::disable_phrase ()
{
    if (m_lookup_table.number_of_candidates () == 0 ||
        m_user_phrase_lib == 0 || !m_user_phrase_lib->valid ())
        return false;

    WideString str = m_lookup_table.get_candidate (m_lookup_table.get_cursor_pos ());

    if (str.length () >= 2) {
        Phrase phrase = m_user_phrase_lib->find (str);
        if (phrase.valid () && phrase.is_enable ()) {
            phrase.disable ();
            bool calc_lookup = auto_fill_preedit (-1);
            calc_keys_preedit_index ();
            refresh_preedit_string ();
            refresh_preedit_caret ();
            refresh_aux_string ();
            refresh_lookup_table (-1, calc_lookup);
        }
    }
    return true;
}

bool
PinyinInstance::auto_fill_preedit (int invalid_pos)
{
    if (m_factory->m_auto_fill_preedit) {

        WideString combined;
        PhraseVector matched_phrases;

        calc_lookup_table (invalid_pos, &combined, &matched_phrases);

        if ((int) m_converted_string.length () > m_lookup_caret)
            m_converted_string.erase (m_converted_string.begin () + m_lookup_caret, m_converted_string.end ());

        m_converted_string += combined;

        clear_selected (m_lookup_caret);

        size_t start = 0;

        for (size_t i = 0; i < matched_phrases.size (); ++ i) {
            if (matched_phrases [i].length () >= 1) {
                store_selected_phrase (start + m_lookup_caret,
                                       matched_phrases [i],
                                       m_converted_string);
                start += matched_phrases [i].length ();
            } else {
                start ++;
            }
        }

        return false;
    }
    return true;
}

void
PinyinInstance::store_selected_phrase (int caret, const Phrase &phrase, const WideString &total)
{
    if (!phrase.length ()) return;

    unsigned int i;

    std::vector < std::pair<int, WideString> > tmp_strings;
    std::vector < std::pair<int, Phrase> > tmp_phrases;

    for (i = 0; i < m_selected_strings.size (); ++ i) {
        int old_start = m_selected_strings [i].first;
        int old_end = m_selected_strings [i].first + m_selected_strings [i].second.length ();
        int new_end = caret + phrase.length ();

        if (old_end <= caret || new_end <= old_start)
            tmp_strings.push_back (m_selected_strings [i]);
        else if (old_start <= caret && old_end >= new_end)
            tmp_strings.push_back (std::make_pair (old_start, total.substr (old_start, old_end - old_start)));
        else if (caret <= old_start && old_end > new_end)
            tmp_strings.push_back (std::make_pair (new_end, total.substr (new_end, old_end - new_end)));
        else if (old_start < caret && old_end <= new_end)
            tmp_strings.push_back (std::make_pair (old_start, total.substr (old_start, caret - old_start)));
    }

    for (i = 0; i < m_selected_phrases.size (); ++ i) {
        if ((m_selected_phrases [i].first + m_selected_phrases [i].second.length ()) <= caret ||
            (caret + phrase.length ()) <= m_selected_phrases [i].first)
            tmp_phrases.push_back (m_selected_phrases [i]);
    }

    tmp_phrases.push_back (std::make_pair (caret, phrase));

    m_selected_strings.swap (tmp_strings);
    m_selected_phrases.swap (tmp_phrases);
}

void
PinyinInstance::store_selected_string (int caret, const WideString &str, const WideString &total)
{
    unsigned int i;

    std::vector < std::pair<int, WideString> > tmp_strings;
    std::vector < std::pair<int, Phrase> > tmp_phrases;

    for (i = 0; i < m_selected_strings.size (); ++ i) {
        int old_start = m_selected_strings [i].first;
        int old_end = m_selected_strings [i].first + m_selected_strings [i].second.length ();
        int new_end = caret + str.length ();

        if (old_end <= caret || new_end <= old_start)
            tmp_strings.push_back (m_selected_strings [i]);
        else if (old_start <= caret && old_end >= new_end)
            tmp_strings.push_back (std::make_pair (old_start, total.substr (old_start, old_end - old_start)));
        else if (caret <= old_start && old_end > new_end)
            tmp_strings.push_back (std::make_pair (new_end, total.substr (new_end, old_end - new_end)));
        else if (old_start < caret && old_end <= new_end)
            tmp_strings.push_back (std::make_pair (old_start, total.substr (old_start, caret - old_start)));
    }

    for (i = 0; i < m_selected_phrases.size (); ++ i) {
        if ((m_selected_phrases [i].first + m_selected_phrases [i].second.length ()) <= caret ||
            (caret + str.length ()) <= m_selected_phrases [i].first)
            tmp_phrases.push_back (m_selected_phrases [i]);
    }

    tmp_strings.push_back (std::make_pair (caret, str));

    m_selected_strings.swap (tmp_strings);
    m_selected_phrases.swap (tmp_phrases);
}

void
PinyinInstance::dynamic_adjust_selected ()
{
    if (m_user_phrase_lib && m_user_phrase_lib->valid ()) {
        PinyinParsedKeyVector keys;
        Phrase newph;

        std::vector <std::pair <int, Phrase> > added_phrases;

        unsigned int i;

        for (i = 0; i < m_selected_phrases.size (); ++ i) {
            if (m_selected_phrases [i].first + m_selected_phrases [i].second.length () <=
                m_parsed_keys.size ()) {
                keys = PinyinParsedKeyVector (
                            m_parsed_keys.begin () + m_selected_phrases [i].first,
                            m_parsed_keys.begin () + m_selected_phrases [i].first +
                                m_selected_phrases [i].second.length ());
                newph = add_new_phrase (m_selected_phrases [i].second.get_content (), keys, true);

                if (newph.valid ()) {
                    added_phrases.push_back (std::make_pair (m_selected_phrases [i].first, newph));
                }
            }
        }

        for (i = 0; i < m_selected_strings.size (); ++ i) {
            if (m_selected_strings [i].first + m_selected_strings [i].second.length () <=
                m_parsed_keys.size ()) {
                keys = PinyinParsedKeyVector (
                            m_parsed_keys.begin () + m_selected_strings [i].first,
                            m_parsed_keys.begin () + m_selected_strings [i].first +
                                m_selected_strings [i].second.length ());

                if (keys.size ()) {
                    newph = add_new_phrase (m_selected_strings [i].second, keys, true);
                    if (newph.valid ()) {
                        added_phrases.push_back (std::make_pair (m_selected_strings [i].first, newph));
                    }
                }
            }
        }

        // Try to find out and add new inputed phrase
        WideString phrase;
        unsigned int last_start = 0;

        //sort by caret
        std::sort (added_phrases.begin (), added_phrases.end ());

#if ENABLE_DEBUG
        std::cerr << "Added new phrases :\n";
        for (i=0; i<added_phrases.size (); ++ i) {
            std::cerr << added_phrases [i].first << " = ";
            std::cerr << utf8_wcstombs (added_phrases [i].second.get_content ()) << "\n";
        }
#endif

        unsigned int num = added_phrases.size ();

        // Combine several single chars to phrases.
        for (i = 1; i < num; ++ i) {
            if (added_phrases [i-1].first + 1 != added_phrases [i].first ||
                added_phrases [i].second.length () != 1) {

                if (added_phrases [i-1].second.length () == 1) {
                    phrase += added_phrases [i-1].second.get_content ();
                    if (phrase.length () > 1) {
                        keys = PinyinParsedKeyVector (
                                m_parsed_keys.begin () + added_phrases [last_start].first,
                                m_parsed_keys.begin () + added_phrases [last_start].first +
                                    phrase.length ());

                        newph = add_new_phrase (phrase, keys, true);
                        if (newph.valid ()) {
                            added_phrases.push_back (
                                std::make_pair (added_phrases [last_start].first, newph));
                        }
                    }
                }
                phrase.clear ();
                last_start = i;
            } else {
                phrase += added_phrases [i-1].second.get_content ();
            }
        }

        if (phrase.length () >= 1 && added_phrases [num-1].second.length () == 1) {
            phrase += added_phrases [num-1].second.get_content ();
            keys = PinyinParsedKeyVector (
                    m_parsed_keys.begin () + added_phrases [last_start].first,
                    m_parsed_keys.begin () + added_phrases [last_start].first +
                        phrase.length ());
            newph = add_new_phrase (phrase, keys, true);
            if (newph.valid ()) {
                added_phrases.push_back (
                    std::make_pair (added_phrases [last_start].first, newph));
            }
        }

        // refresh phrases relation data
        for (i = 1; i < added_phrases.size (); ++ i) {
            if (added_phrases [i-1].first + added_phrases [i-1].second.length () == added_phrases [i].first &&
                (added_phrases [i-1].second.length () + added_phrases [i].second.length () > 2))
                m_user_phrase_lib->refresh_phrase_relation (
                    added_phrases [i-1].second,
                    added_phrases [i].second,
                    16);
        }
    }
}

void
PinyinInstance::clear_selected (int clear_pos)
{
    if (!clear_pos) {
        std::vector < std::pair<int, WideString> > ().swap (m_selected_strings);
        std::vector < std::pair<int, Phrase> > ().swap (m_selected_phrases);
    } else {
        size_t i;
        std::vector < std::pair<int, WideString> > tmp_strings;
        std::vector < std::pair<int, Phrase> > tmp_phrases;

        for (i = 0; i < m_selected_strings.size (); ++ i) {
            if (m_selected_strings [i].first + m_selected_strings [i].second.length () <= clear_pos)
                tmp_strings.push_back (m_selected_strings [i]);
        }

        for (i = 0; i < m_selected_phrases.size (); ++ i) {
            if (m_selected_phrases [i].first + m_selected_phrases [i].second.length () <= clear_pos)
                tmp_phrases .push_back (m_selected_phrases [i]);
        }

        m_selected_strings.swap (tmp_strings);
        m_selected_phrases.swap (tmp_phrases);
    }
}

WideString
PinyinInstance::convert_to_full_width (char key)
{
    WideString str;
    if (key == '.')
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
    } else if (key == '<' && !m_forward) {
        str.push_back (0x300A);
    } else if (key == '>' && !m_forward) {
        str.push_back (0x300B);
    } else if (key == '$') {
        str.push_back (0xFFE5);
    } else if (key == '_') {
        str.push_back (0x2014);
        str.push_back (0x2014);
    } else {
        str.push_back (scim_wchar_to_full_width (key));
    }

    return str;
}

bool
PinyinInstance::english_mode_process_key_event (const KeyEvent &key)
{
    //Start forward mode
    if (!m_inputed_string.length () && key.code == SCIM_KEY_v && key.mask == 0) {
        m_inputed_string.push_back ('v');
        m_converted_string.push_back ('v');
        refresh_all_properties ();
    }
    //backspace key
    else if ((key.code == SCIM_KEY_BackSpace ||
              key.code == SCIM_KEY_Delete) &&
             key.mask == 0) {
        m_converted_string.erase (m_converted_string.length () - 1);
        if (m_converted_string.length () <= 1)
            m_converted_string.clear ();
    }
    //enter or space hit, commit string
    else if ((key.code == SCIM_KEY_Return || key.code == SCIM_KEY_space) &&
             (key.mask & (~ (SCIM_KEY_ShiftMask + SCIM_KEY_CapsLockMask))) == 0) {
        WideString str = m_converted_string.substr (1);
        if (str.length ())
            commit_string (str);
        m_converted_string.clear ();
    }
    //insert normal keys
    else if ((key.mask & (~ (SCIM_KEY_ShiftMask + SCIM_KEY_CapsLockMask))) == 0) {
        char ch = key.get_ascii_code ();
        if ((ispunct (ch) && m_full_width_punctuation [1]) ||
            (isalnum (ch) && m_full_width_letter [1])) {
            m_converted_string += convert_to_full_width (ch);
        } else if (ch) {
            ucs4_t wc;
            utf8_mbtowc (&wc, (unsigned char*) &ch, 1);
            m_converted_string += wc;
        } else {
            return true;
        }
    }
    //forward special keys directly.
    else if ((key.mask & (~ (SCIM_KEY_ShiftMask + SCIM_KEY_CapsLockMask)))) 
        return false;

    if (m_converted_string.length ()) {
        english_mode_refresh_preedit ();
    } else {
        reset ();
    }

    return true;
}

void
PinyinInstance::english_mode_refresh_preedit ()
{
    WideString str = m_converted_string.substr (1);
    if (str.length ()) {
        update_preedit_string (str);
        update_preedit_caret (str.length ());
        show_preedit_string ();
    } else {
        hide_preedit_string ();
    }
}

bool
PinyinInstance::is_english_mode () const
{
    if (m_inputed_string.length () && m_inputed_string [0] == 'v' &&
        m_converted_string.length () && m_converted_string [0] == 'v')
        return true;
    return false;
}

bool
PinyinInstance::special_mode_process_key_event (const KeyEvent &key)
{
    //Start forward mode
    if (!m_inputed_string.length () && key.code == SCIM_KEY_i && key.mask == 0) {
        m_inputed_string.push_back ('i');
        m_converted_string.push_back ('i');
        special_mode_refresh_preedit ();
        special_mode_refresh_lookup_table ();
        return true;
    }

    //lookup table cursor up
    if (key.code == SCIM_KEY_Up && key.mask == 0)
        return lookup_cursor_up ();

    //lookup table cursor down
    if (key.code == SCIM_KEY_Down && key.mask == 0)
        return lookup_cursor_down ();

    //lookup table page up
    if (match_key_event (m_factory->m_page_up_keys, key))
        if (lookup_page_up ()) return true;

    //lookup table page down
    if (match_key_event (m_factory->m_page_down_keys, key))
        if (lookup_page_down ()) return true;

    //select lookup table
    if (m_pinyin_global->use_tone ()) {
        if (((key.code >= SCIM_KEY_6 && key.code <= SCIM_KEY_9) || key.code == SCIM_KEY_0) && key.mask == 0) {
            int index;
            if (key.code == SCIM_KEY_0) index = 4;
            else index = key.code - SCIM_KEY_6;
            if (special_mode_lookup_select (index)) return true;
        }
    } else {
        if (key.code >= SCIM_KEY_1 && key.code <= SCIM_KEY_9 && key.mask == 0) {
            int index = key.code - SCIM_KEY_1;
            if (special_mode_lookup_select (index)) return true;
        }
    }

    //backspace key
    if ((key.code == SCIM_KEY_BackSpace ||
         key.code == SCIM_KEY_Delete) &&
         key.mask == 0) {
        m_inputed_string.erase (m_inputed_string.length () - 1);
        m_converted_string.erase (m_converted_string.length () - 1);
    }
    //enter or space hit, commit string
    else if ((key.code == SCIM_KEY_space || key.code == SCIM_KEY_Return) &&
             (key.mask & (~ (SCIM_KEY_ShiftMask + SCIM_KEY_CapsLockMask))) == 0) {

        if (m_lookup_table.number_of_candidates ())
            commit_string (m_lookup_table.get_candidate (m_lookup_table.get_cursor_pos ()));
        else
            commit_string (m_converted_string);

        m_inputed_string.clear ();
        m_converted_string.clear ();
    }
    //insert normal keys
    else if ((key.mask & (~ (SCIM_KEY_ShiftMask + SCIM_KEY_CapsLockMask))) == 0 &&
              key.code != 0 &&
              m_inputed_string.length () <= m_factory->m_special_table.get_max_key_length ()) {
        char ch = key.get_ascii_code ();

        if (ch) {
            m_inputed_string += ch;
            m_converted_string += (ucs4_t) ch;
        } else {
            return true;
        }
    }
    //forward special keys directly.
    else if ((key.mask & (~ (SCIM_KEY_ShiftMask + SCIM_KEY_CapsLockMask)))) 
        return false;

    if (m_inputed_string.length ()) {
        special_mode_refresh_preedit ();
        special_mode_refresh_lookup_table ();
    } else {
        reset ();
    }

    return true;
}

void
PinyinInstance::special_mode_refresh_preedit ()
{
    if (m_converted_string.length ()) {
        update_preedit_string (m_converted_string);
        update_preedit_caret (m_converted_string.length ());
        show_preedit_string ();
    } else {
        hide_preedit_string ();
    }
}

void
PinyinInstance::special_mode_refresh_lookup_table ()
{
    m_lookup_table.clear ();
    m_lookup_table.set_page_size (m_lookup_table_def_page_size);

    if (m_inputed_string.length () > 1) {
        std::vector<WideString> result;
        String key = m_inputed_string.substr (1);

        if (m_factory->m_special_table.find (result, key) > 0) {
            for (std::vector<WideString>::iterator it = result.begin (); it != result.end (); ++ it) {
                if (m_iconv.test_convert (*it))
                    m_lookup_table.append_entry (*it);
            }
            if (m_lookup_table.number_of_candidates ()) {
                show_lookup_table ();
                update_lookup_table (m_lookup_table);
                return;
            }
        }
    }
    hide_lookup_table ();
}

bool
PinyinInstance::special_mode_lookup_select (int index)
{
    if (m_inputed_string.length () && m_lookup_table.number_of_candidates ()) {

        index += m_lookup_table.get_current_page_start ();

        WideString str = m_lookup_table.get_candidate (index);
        if (str.length ()) {
            commit_string (str);
        }
        reset ();
        return true;
    }

    return false;
}

bool
PinyinInstance::is_special_mode () const
{
    if (m_inputed_string.length () && m_inputed_string [0] == 'i' &&
        m_converted_string.length () && m_converted_string [0] == 'i')
        return true;
    return false;
}

void
PinyinInstance::reload_config (const ConfigPointer &config)
{
    reset ();
    if (m_factory->valid () && m_pinyin_global) {
        m_pinyin_table = m_pinyin_global->get_pinyin_table ();
        m_sys_phrase_lib = m_pinyin_global->get_sys_phrase_lib ();
        m_user_phrase_lib = m_pinyin_global->get_user_phrase_lib ();
    } else {
        m_pinyin_table = 0;
        m_sys_phrase_lib = 0;
        m_user_phrase_lib = 0;
    }
}


/*
vi:ts=4:nowrap:ai:expandtab
*/
