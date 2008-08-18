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

#if !defined (__SCIM_CCIN_SERVER_H)
#define __SCIM_CCIN_SERVER_H

/* phrase frequency cannot larger than this value (2^23 - 1) */
#define SCIM_GT_MAX_PHRASE_FREQ            8388607

/* when a phrase is input,
 * increase the freq by ((max_freq - cur_freq) >> delta)
 */
#define SCIM_GT_PHRASE_FREQ_DELTA_SHIFT    10

#include "ccinput.h"

using namespace scim;

class CcinIMEngineFactory : public IMEngineFactoryBase
{
    GenericTablePhraseLib m_table;

    ConfigPointer         m_config;

    std::vector<KeyEvent> m_full_width_punct_keys;
    std::vector<KeyEvent> m_full_width_letter_keys;
    std::vector<KeyEvent> m_mode_switch_keys;
    std::vector<KeyEvent> m_pageup_keys;
    std::vector<KeyEvent> m_pagedown_keys;

    String                m_table_filename;

    String                m_select_keys;

    bool                  m_is_user_table;

    bool                  m_config_fuzzy_Wang_Huang ;
    bool                  m_config_fuzzy_An_Ang;
    bool                  m_config_fuzzy_En_Eng;
    bool                  m_config_fuzzy_In_Ing;
    bool                  m_config_fuzzy_C_Ch;
    bool                  m_config_fuzzy_S_Sh;
    bool                  m_config_fuzzy_Z_Zh;
    bool                  m_config_fuzzy_F_H;
    bool                  m_config_fuzzy_L_N;
    bool                  m_config_fuzzy_K_G;
    bool                  m_config_fuzzy_R_L;

    bool                  m_config_shuangpin;
    String                m_config_Shuangpin_Kind;
    bool                  m_config_GBK;

    bool                  m_show_prompt;

    bool                  m_show_key_hint;

    bool                  m_user_table_binary;

    bool                  m_modified;

    bool                  m_long_phrase_first;

    time_t                m_last_time;

    friend class CcinIMEngineInstance;

public:
    CcinIMEngineFactory (const ConfigPointer &config);

    virtual ~CcinIMEngineFactory ();

    virtual WideString  get_name () const;
    virtual WideString  get_authors () const;
    virtual WideString  get_credits () const;
    virtual WideString  get_help () const;
    virtual String      get_uuid () const;
    virtual String      get_icon_file () const;
    virtual String      get_language () const;
	    
    bool load_table (const String &table_file, bool user_table=false);

    bool valid () const {
        return m_table.valid ();
    }

    virtual IMEngineInstancePointer create_instance (const String& encoding, int id = -1);

private:

    void init ();
    void save_user_table ();

    String get_sys_table_freq_file ();

    void load_sys_table_freq ();
    void save_sys_table_freq ();

    void refresh ();
    void compact_memory ();
};

class CcinIMEngineInstance : public IMEngineInstanceBase
{
    ccinIMContext_t * im_context;

    Pointer <CcinIMEngineFactory>  m_factory;

    bool m_double_quotation_state;
    bool m_single_quotation_state;

    bool m_full_width_punctuation [2];
    bool m_full_width_letter [2];

    bool m_forward;
    bool m_focused;

    uint32             m_inputed_word_number;
    uint32             m_inputed_user_word_number;

    String              m_translated_quanpin_string;
    std::vector<String> m_inputed_keys;
    std::vector<WideString> m_converted_strings;
    std::vector<u_short> m_converted_pinyins;

    int m_global_inputing_caret;
    uint32             m_inputing_caret;
    uint32             m_inputing_key;
    WideString          m_last_converted_string;

    CommonLookupTable      m_lookup_table;
    GenericKeyIndexVector  m_lookup_key_indexes;

    IConvert m_iconv;

    KeyEvent m_prev_key;

public:
    CcinIMEngineInstance (CcinIMEngineFactory *factory,
                         const String& encoding,
                         int id = -1);
    virtual ~CcinIMEngineInstance ();

    virtual bool process_key_event (const KeyEvent& key);
    virtual void move_preedit_caret (unsigned int pos);
    virtual void update_lookup_table_page_size (unsigned int page_size);
    virtual void reset ();
    virtual void focus_in ();
    virtual void focus_out ();

    virtual void lookup_table_page_up ();
    virtual void lookup_table_page_down ();
    virtual void trigger_property (const String &property);
    virtual void select_candidate (unsigned int index);
private:
    bool caret_left ();
    bool caret_right ();
    bool caret_home ();
    bool caret_end ();
    bool insert (char key);
    bool erase (bool backspace = true);
    bool space_hit ();
    bool enter_hit ();
    bool lookup_cursor_up ();
    bool lookup_cursor_down ();
    bool lookup_cursor_up_to_longer ();
    bool lookup_cursor_down_to_shorter ();
    bool lookup_page_up ();
    bool lookup_page_down ();
    bool lookup_select (char key);
    bool post_process (char key);
    void lookup_to_converted (int index);
    void commit_converted ();

    void refresh_preedit_caret ();
    void refresh_status_string ();
    void refresh_preedit_string ();
    void refresh_lookup_table ();
    void refresh_aux_string ();

    void initialize_all_properties ();
    void refresh_all_properties ();
    void refresh_status_property ();
    void refresh_letter_property ();
    void refresh_punct_property ();
                    
    
    bool match_key_event (const std::vector<KeyEvent>& keyvec, const KeyEvent& key);
    bool parse_pinyin_string ();
    bool create_lookup_table ();
    bool display_debug_info ();
    String get_inputed_string ();
    String get_parse_string ();
    bool add_user_phrase (WideString &ret);
    bool lookup_delete (char key);
    uint32 get_inputed_string_length ();
};

#endif
/*
vi:ts=4:nowrap:ai:expandtab
*/
