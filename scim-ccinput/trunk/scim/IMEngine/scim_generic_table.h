/** @file scim_generic_table.h
 * definition of GenericTableLib related classes.
 */

/* 
 * Smart Common Input Method
 * 
 * Copyright (c) 2002 James Su <suzhe@turbolinux.com.cn>
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 *
 * $Id: scim_generic_table.h,v 1.1.1.1 2005/05/24 23:37:49 liuspider Exp $
 */

#if !defined (__SCIM_GENERIC_TABLE__H)
#define __SCIM_GENERIC_TABLE_H

#define SCIM_GT_SINGLE_WILDCARD_VALUE    (-2)
#define SCIM_GT_MULTI_WILDCARD_VALUE    (-3)
#define SCIM_GT_MAX_PHRASE_LENGTH        31

using namespace scim;

//first = key value, second = phrase index
typedef std::pair <uint32, uint32> GenericKeyIndexPair;
typedef std::vector <GenericKeyIndexPair> GenericKeyIndexVector;

/**
 * class to store key-index std::pairs.
 *
 * the string keys are compiled to uint32 integers to store.
 * the integer keys combined with their indexes are stored into a std::vector.
 */
class GenericKeyIndexLib
{
    char   m_char_value_table [128];/**< char to value convert table */
    char   m_value_char_table [128];/**< value to char convert table */

    uint32 m_num_of_valid_chars;    /**< the number of all valid chars */

    uint32 m_max_key_length;        /**< the maximum length of valid key */
    uint32 m_max_key_value;            /**< the maximum value of the key number */

    GenericKeyIndexVector m_keys;    /**< the std::vector to store all of the key-index std::paires. */

public:
    GenericKeyIndexLib ();
    GenericKeyIndexLib (const String &valid_chars);

    /**
     * check if this key-index lib is valid
     */
    bool valid () const;

    /**
     * set the vaild input chars for this set of keys.
     *
     * @param valid_chars the std::vector contains the valid chars.
     * @return true if successfull.
     */
    bool set_valid_chars (const String &valid_chars, uint32 max_key_length = 0);

    /**
     * get a string contains all of the valid input chars.
     */
    String get_valid_chars () const;

    /**
     * set the valid single wildcard chars for this set of keys.
     *
     * @param single the single wildcard char.
     * @return true if successful.
     */
    bool set_single_wildcards (const String &singles);

    /**
     * set the valid multi wildcard chars for this set of keys.
     *
     * @param multi the multipl wildcard char.
     * @return true if successful.
     */
    bool set_multi_wildcards (const String &multis);

    /**
     * get the valid single wildcards.
     */
    String get_single_wildcards () const;
    
    /**
     * get the valid multi wildcards.
     */
    String get_multi_wildcards () const;

    /**
     * check if the given char is valid for this key system.
     */
    bool is_valid_char (char val) const; 

    /**
     * check if the given char is a wildcard char for this key system.
     */
    bool is_wildcard_char (char val) const; 

    /**
     * check if a given key is valid.
     */
    bool is_valid_key (const String &key) const; 

    /**
     * check if a given key is not defined.
     */
    bool is_defined_key (const String &key, bool auto_wildcard = false) const;

    /**
     * check if a given key has any wildcard chars.
     */
    bool is_wildcard_key (const String &key) const;

    /**
     * add many key-index std::paires into the library.
     */
    bool add_key_indexes (const std::vector <String> &keys, const std::vector <uint32> & indexes);

    bool add_key_indexes (const std::vector <uint32> &keys, const std::vector <uint32> & indexes);

    /**
     * insert a key-index std::pair into the library.
     */
    bool insert_key_index (const String &key, uint32 index);

    /**
     * delete a key-index std::pair, if index == -1 then
     * all the indexes relates to this key will be deleted.
     */
    bool erase_key_index (const String &key, int index = -1);

    /**
     * find all of the key-index std::paires associated to a key.
     *
     * @param key_indexes the std::vector to store the result.
     * @param key the key to be search.
     *
     * @return true if the result is not empty.
     */
    bool find_key_indexes (GenericKeyIndexVector & key_indexes, const String &key, bool auto_wildcard = false) const;

    /**
     * get the value of a string key.
     *
     * @return the value associated to the key, greater than zero.
     */
    uint32 key_to_value (const String &key) const;

    /**
     * get the string key of a key value.
     *
     * @param value the value to be translated, must be greater than zero.
     */
    String value_to_key (uint32 value) const;

    /**
     * get the value of a char.
     */
    int char_to_value (char ch) const {
        return m_char_value_table [ch & 0x7f];
    }

    /**
     * get the corresponding char of a value.
     */
    char value_to_char (int value) const {
        return m_value_char_table [(uint32)(value & 0x7f)];
    }

    /**
     * clear everything.
     */
    void clear_all ();

    /**
     * clear all but the input chars and wildcard chars settings.
     */
    void clear_keys ();

    /**
     * free the unused memory.
     */
    void compact_memory ();

    /**
     * return the maximum allowed key length.
     */
    uint32 get_max_key_length () const { return m_max_key_length; }

    /**
     * return the maximum allowed key value.
     */
    uint32 get_max_key_value () const { return m_max_key_value; }

    /**
     * return the number of the keys.
     */
    uint32 number_of_keys () const { return m_keys.size (); }

    const GenericKeyIndexVector& get_all_key_indexes () const {
        return m_keys;
    }
private:
    /**
     * compile string key into uint32 integer keys,
     * if there are some wildcard chars, give out a fuzzy range
     * of the key.
     * 
     * @param range_list store the valid ranges, .first = min_val, .second = max_val.
     * @param key the key to be compiled.
     * @return the interval of the valid range.
     */
    uint32 compile_key (std::vector <std::pair <uint32, uint32> > &range_list, const String &key) const;
};

/**
 * class to store generic table phrases.
 *
 * The structure of m_content is:
 *
 *         Phrase 1             Phrase 2
 *  +------+------+------+...+------+------+--...
 *  |UINT32|UINT32|UINT32|   |UINT32|UINT32|UINT32
 *  |Header|PhraseContent|   |Header|PhraseContent
 *  +------+------+------+...+------+------+--...
 * 
 *  format of Header:
 *  The 31th bit must be set to one,
 *  indicating that this phrase is OK.
 *  The 30th bit indicates whether the phrase
 *  is updated.
 *  The 0-4 bits is phrase length
 *  The 5-7 bits is key length - 1
 *
 *  31                                  0
 *  +--------+--------+--------+--------+
 *  |FM  FREQ|AGE/FREQ|AGE/FREQ|KLNL-E-N|
 *  +--------+--------+--------+--------+
 */
class GenericTablePhraseLib
{
    GenericKeyIndexLib   m_indexes;

    String               m_uuid;

    String               m_icon_file;

    std::vector <ucs4_t> m_content;
    std::vector <String> m_char_prompts;

    WideString           m_author;
    WideString           m_status_prompt;

    String               m_locales;
    String               m_default_name;

    std::vector <String> m_lang_names;

    String               m_select_keys;
    String               m_split_chars;
    String               m_pageup_keys;
    String               m_pagedown_keys;

    bool                 m_auto_select;
    bool                 m_auto_wildcard;
    bool                 m_dynamic_adjust;

    GenericKeyIndexVector m_sorted_phrase_indexes;
    bool                  m_sorted_phrase_indexes_initialized;

    uint32                m_serial_number;

public:
    GenericTablePhraseLib (const String &libfile = "");
    GenericTablePhraseLib (std::istream &is);

    bool input (std::istream &is);
    bool output (std::ostream &os, bool binary = false) const;

    bool input_phrase_frequencies (std::istream &is);
    bool output_phrase_frequencies (std::ostream &os, bool binary = false) const;

    bool load_lib (const String &libfile, const String &freqfile = "");
    bool save_lib (const String &libfile, const String &freqfile = "", bool binary = false) const;

    bool valid () const;

    bool is_valid_char (char ch) const {
        return m_indexes.is_valid_char (ch);
    }

    bool is_wildcard_char (char ch) const {
        return m_indexes.is_wildcard_char (ch);
    }

    bool is_valid_key (const String &key) const {
        return m_indexes.is_valid_key (key);
    }

    bool is_defined_key (const String &key, bool maybe = true) const {
        return m_indexes.is_defined_key (key, maybe);
    }

    bool is_split_char (char ch) const {
        return m_split_chars.find (ch) != String::npos;
    }

    bool is_valid_phrase (uint32 index) const {
        return (index < m_content.size () - 1) && (m_content [index] & 0x80000000);
    }

    bool is_updated_phrase (uint32 index) const {
        return (index < m_content.size () - 1)
                && (m_content [index] & 0x80000000)
                && (m_content [index] & 0x40000000);
    }

    bool find_key_indexes (GenericKeyIndexVector &indexes, const String &key, bool sort_by_freq = true) const;

    bool find_phrase_indexes (GenericKeyIndexVector &indexes, const WideString &phrase);

    String get_key_string (uint32 value) const {
        return m_indexes.value_to_key (value);
    }

    WideString get_key_prompt (const String &key) const;

    WideString get_char_prompt (char ch) const;

    WideString get_phrase (uint32 index) const;

    WideString get_name (const String &locale) const;

    uint32 get_phrase_length (uint32 index) const {
        if (is_valid_phrase (index))
            return (uint32) (m_content [index] & 0x1f);
        return 0;
    }

    uint32 get_phrase_frequency (uint32 index) const {
        if (is_valid_phrase (index))
            return (uint32) ((m_content [index] >> 8) & 0x3fffff);
        return 0;
    }

    uint32 get_phrase_key_length (uint32 index) const {
        if (is_valid_phrase (index))
            return (uint32) ((m_content [index] >> 5) & 0x07) + 1;
        return 0;
    }

    const String & get_uuid () const {
        return m_uuid;
    }

    const String & get_icon_file () const {
        return m_icon_file;
    }

    const WideString & get_author () const {
        return m_author;
    }

    const WideString & get_status_prompt () const {
        return m_status_prompt;
    }

    const String & get_locales () const {
        return m_locales;
    }

    const String & get_select_keys () const {
        return m_select_keys;
    }

    const String & get_pageup_keys () const {
        return m_pageup_keys;
    }

    const String & get_pagedown_keys () const {
        return m_pagedown_keys;
    }

    size_t get_select_key_pos (char ch) const {
        return m_select_keys.find (ch);
    }

    uint32 get_max_key_length () const {
        return m_indexes.get_max_key_length ();
    }

    uint32 number_of_keys () const {
        return m_indexes.number_of_keys ();
    }

    uint32 get_max_phrase_length () const;

    bool is_auto_select () const { return m_auto_select; }

    /**
     * Whether adjust phrase frequencies dynamically.
     */
    bool is_dynamic_adjust () const { return m_dynamic_adjust; }

    void set_phrase_frequency (uint32 index, uint32 freq);

    void clear ();

    void compact_memory ();

private:
    friend class GenericTablePhraseLessThanByPhrase;
    friend class GenericTablePhraseEqualToByPhrase;

    int compare_phrase (uint32 lhs, uint32 rhs) const;
    int compare_phrase (uint32 lhs, const WideString & rhs) const;
    int compare_phrase (const WideString & lhs, uint32 rhs) const;

    void initialize_sorted_phrase_indexes ();

    void sort_indexes_by_frequency (GenericKeyIndexVector &indexes) const;
    void sort_indexes_by_length (GenericKeyIndexVector &indexes) const;
    void sort_indexes_by_phrase (GenericKeyIndexVector &indexes) const;
    void sort_indexes_by_index (GenericKeyIndexVector &indexes) const;
};

class GenericTablePhraseLessThanByPhrase
{
    const GenericTablePhraseLib *m_lib;
public:
    GenericTablePhraseLessThanByPhrase (const GenericTablePhraseLib *lib) : m_lib (lib) {}

    bool operator () (const GenericKeyIndexPair & lhs,
                      const GenericKeyIndexPair & rhs) const {
        return  m_lib->compare_phrase (lhs.second, rhs.second) < 0;
    }
    bool operator () (const GenericKeyIndexPair & lhs, const WideString & rhs) const {
        return  m_lib->compare_phrase (lhs.second, rhs) < 0;
    }
    bool operator () (const WideString & lhs, const GenericKeyIndexPair & rhs) const {
        return  m_lib->compare_phrase (lhs, rhs.second) < 0;
    }
};

class GenericTablePhraseEqualToByPhrase
{
    const GenericTablePhraseLib *m_lib;
public:
    GenericTablePhraseEqualToByPhrase (const GenericTablePhraseLib *lib) : m_lib (lib) {}

    bool operator () (const GenericKeyIndexPair & lhs,
                      const GenericKeyIndexPair & rhs) const {
        return  m_lib->compare_phrase (lhs.second, rhs.second) == 0;
    }
    bool operator () (const GenericKeyIndexPair & lhs, const WideString & rhs) const {
        return  m_lib->compare_phrase (lhs.second, rhs) == 0;
    }
    bool operator () (const WideString & lhs, const GenericKeyIndexPair & rhs) const {
        return  m_lib->compare_phrase (lhs, rhs.second) == 0;
    }
};

#endif

/*
vi:ts=4:nowrap:ai:expandtab
*/
