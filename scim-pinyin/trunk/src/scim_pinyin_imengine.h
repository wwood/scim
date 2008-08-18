/** @file scim_pinyin_imengine.h
 * definition of Pinyin IMEngine related classes.
 */

/* 
 * Smart Pinyin Input Method
 * 
 * Copyright (c) 2005 James Su <suzhe@tsinghua.org.cn>
 *
 * $Id: scim_pinyin_imengine.h,v 1.4 2005/08/15 08:56:00 suzhe Exp $
 */

#if !defined (__SCIM_PINYIN_SERVER_H)
#define __SCIM_PINYIN_SERVER_H

using namespace scim;

class PinyinFactory : public IMEngineFactoryBase
{
    PinyinGlobal   m_pinyin_global;

    SpecialTable   m_special_table;

    ConfigPointer  m_config;

    WideString     m_name;

    PinyinParser  *m_pinyin_parser;

    String         m_user_data_directory;
    String         m_user_phrase_lib;
    String         m_user_pinyin_table;
    String         m_user_pinyin_phrase_lib;
    String         m_user_pinyin_phrase_index;

    std::vector<KeyEvent>       m_full_width_punct_keys;
    std::vector<KeyEvent>       m_full_width_letter_keys;
    std::vector<KeyEvent>       m_mode_switch_keys;
    std::vector<KeyEvent>       m_chinese_switch_keys;
    std::vector<KeyEvent>       m_disable_phrase_keys;
    std::vector<KeyEvent>       m_page_up_keys;
    std::vector<KeyEvent>       m_page_down_keys;

    bool               m_match_longer_phrase;
    bool               m_auto_combine_phrase;
    bool               m_auto_fill_preedit;
    bool               m_always_show_lookup;
    bool               m_show_all_keys;

    bool               m_user_data_binary;

    bool               m_valid;

    time_t             m_last_time;
    time_t             m_save_period;

    bool               m_shuang_pin;

    PinyinShuangPinScheme m_shuang_pin_scheme;

    uint32             m_dynamic_sensitivity;
    uint32             m_smart_match_level;
    uint32             m_max_user_phrase_length;
    uint32             m_max_preedit_length;

    Connection         m_reload_signal_connection;

    friend class PinyinInstance;

public:
    PinyinFactory (const ConfigPointer &config);

    virtual ~PinyinFactory ();

    virtual WideString  get_name () const;
    virtual WideString  get_authors () const;
    virtual WideString  get_credits () const;
    virtual WideString  get_help () const;
    virtual String      get_uuid () const;
    virtual String      get_icon_file () const;

    virtual IMEngineInstancePointer create_instance (const String& encoding, int id = -1);

    void refresh ();

    bool valid () const { return m_valid; }

private:
    bool init ();

    void init_pinyin_parser ();

    void save_user_library ();

    void reload_config (const ConfigPointer &config);
};

class PinyinInstance : public IMEngineInstanceBase
{
    PinyinFactory       *m_factory;

    PinyinGlobal        *m_pinyin_global;
    PinyinTable         *m_pinyin_table;
    PinyinPhraseLib     *m_sys_phrase_lib;
    PinyinPhraseLib     *m_user_phrase_lib;

    bool                 m_double_quotation_state;
    bool                 m_single_quotation_state;

    bool                 m_full_width_punctuation [2];
    bool                 m_full_width_letter [2];
                     
    bool                 m_forward;
    bool                 m_focused;
    bool                 m_simplified;
    bool                 m_traditional;

    int                  m_lookup_table_def_page_size;

    int                  m_keys_caret;
    int                  m_lookup_caret;

    String               m_client_encoding;

    String               m_inputed_string;
 
    WideString           m_converted_string;
    WideString           m_preedit_string;
    WideString           m_auto_combined_string;

    KeyEvent             m_prev_key;

    NativeLookupTable    m_lookup_table;

    IConvert             m_iconv;
    IConvert             m_chinese_iconv;


    std::vector <PinyinParsedKey>          m_parsed_keys;

    std::vector < std::pair<int, int> >    m_keys_preedit_index;

    std::vector < std::pair<int, Phrase> > m_selected_phrases;

    std::vector < std::pair<int, WideString> > m_selected_strings;

    std::vector <CharVector>   m_chars_cache;

    std::vector <PhraseVector> m_phrases_cache;

    Connection         m_reload_signal_connection;

public:
    PinyinInstance (PinyinFactory *factory,
                    PinyinGlobal *pinyin_global,
                    const String& encoding,
                    int id = -1);
    virtual ~PinyinInstance ();

    virtual bool process_key_event (const KeyEvent& key);
    virtual void move_preedit_caret (unsigned int pos);
    virtual void select_candidate (unsigned int item);
    virtual void update_lookup_table_page_size (unsigned int page_size);
    virtual void lookup_table_page_up ();
    virtual void lookup_table_page_down ();
    virtual void reset ();
    virtual void focus_in ();
    virtual void focus_out ();
    virtual void trigger_property (const String &property);

private:
    void init_lookup_table_labels ();

    bool caret_left (bool home = false);
    bool caret_right (bool end = false);

    bool validate_insert_key (char key);

    bool insert (char key);
    bool erase (bool backspace = true);
    bool erase_by_key (bool backspace = true);
    bool space_hit ();
    bool enter_hit ();
    bool lookup_cursor_up ();
    bool lookup_cursor_down ();
    bool lookup_page_up ();
    bool lookup_page_down ();
    bool lookup_select (int index);
    bool post_process (char key);
    bool disable_phrase ();
    void lookup_to_converted (int index);
    void commit_converted ();

    Phrase add_new_phrase (const WideString &phrase, const PinyinParsedKeyVector &keys, bool refresh = false);

    void refresh_preedit_caret ();
    void refresh_preedit_string ();
    void refresh_lookup_table (int invalid_pos = 0, bool calc = true);
    void refresh_aux_string ();

    void initialize_all_properties ();
    void refresh_all_properties ();
    void refresh_status_property ();
    void refresh_letter_property ();
    void refresh_punct_property ();
    void refresh_pinyin_scheme_property ();

    void calc_parsed_keys ();
    void calc_keys_preedit_index ();
    void calc_preedit_string ();
    void calc_lookup_table (int invalid_pos = 0, WideString *combined = 0, PhraseVector *matched_phrases = 0);

    bool auto_fill_preedit (int invalid_pos = 0);

    bool has_unparsed_chars ();

    int  calc_inputed_caret ();
    int  calc_preedit_caret (); 

    int  inputed_caret_to_key_index (int caret);

    void store_selected_phrase (int caret, const Phrase &phrase, const WideString &total);
    void store_selected_string (int caret, const WideString &str, const WideString &total);

    void dynamic_adjust_selected ();
    void clear_selected (int clear_pos = 0);

    bool english_mode_process_key_event (const KeyEvent &key);

    void english_mode_refresh_preedit ();

    bool special_mode_process_key_event (const KeyEvent &key);

    void special_mode_refresh_preedit ();

    void special_mode_refresh_lookup_table ();

    bool special_mode_lookup_select (int index);

    bool match_key_event (const std::vector <KeyEvent>& keyvec, const KeyEvent& key);

    bool is_english_mode () const;

    bool is_special_mode () const;

    WideString convert_to_full_width (char key);

    void reload_config (const ConfigPointer &config);
};

#endif
/*
vi:expandtab:ts=4:nowrap:ai
*/

