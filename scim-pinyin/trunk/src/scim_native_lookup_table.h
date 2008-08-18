/** @file scim_native_lookup_table.h
 * definition of NativeLookupTable classes.
 */

/*
 * Smart Pinyin Input Method
 * 
 * Copyright (c) 2005 James Su <suzhe@tsinghua.org.cn>
 *
 * $Id: scim_native_lookup_table.h,v 1.1.1.1 2005/01/06 13:30:58 suzhe Exp $
 */

#if !defined (__SCIM_NATIVE_LOOKUP_TABLE_H)
#define __SCIM_NATIVE_LOOKUP_TABLE_H

using namespace scim;
using namespace std;

/**
 * lookup table class used by SCIM servers itself.
 *
 * The internal of lookup table is divided into two part,
 * one is a set of phrases, another is a set of chars.
 * These phrases and chars are combined into one index range,
 * and can be accessed by unified index.
 */
class NativeLookupTable : public LookupTable
{
    vector<WideString>  m_strings;
    PhraseVector        m_phrases;
    vector<ucs4_t>      m_chars;

public:
    NativeLookupTable (int page_size = 10);

    virtual void clear () {
        LookupTable::clear ();
        vector <WideString> ().swap (m_strings);
        PhraseVector ().swap (m_phrases);
        vector <ucs4_t> ().swap (m_chars);
    }

    /**
     * return the total number of phrases and chars.
     */
    virtual uint32 number_of_candidates () const {
        return (m_strings.size () + m_phrases.size () + m_chars.size ());
    }

    /**
     * get the content of a phrase or a char.
     *
     * @param buf the buffer to store the content.
     * @param index the unified index in the lookup table.
     *
     * @return the number of wide chars actually written into the buffer.
     */
    virtual WideString get_candidate (int index) const;

    virtual AttributeList get_attributes (int index) const;

public:
    /**
     * append a string into the string vector.
     */

    bool append_entry (const WideString &entry);

    /**
     * append a phrase into the phrase vector.
     */
    bool append_entry (const Phrase& entry);
    
    /**
     * append a char into the char vector.
     */
    bool append_entry (const ucs4_t& entry);

    /**
     * return true if the entry at index is a string.
     */
    bool is_string (int index) const {
        if (index >= 0 && index < (int) m_strings.size ())
            return true;
        return false;
    }
    
    bool is_string_in_page (int page_index) const {
        if (page_index >= 0 && page_index < get_current_page_size ())
            return is_string (page_index + get_current_page_start ());
        return false;
    }

    /**
     * return true if the entry at index is a phrase.
     */
    bool is_phrase (int index) const {
        if (index >= (int) m_strings.size () &&
            index < (int) (m_phrases.size () + m_strings.size ()))
            return true;
        return false;
    }

    bool is_phrase_in_page (int page_index) const {
        if (page_index >= 0 && page_index < get_current_page_size ())
            return is_phrase (page_index + get_current_page_start ());
        return false;
    }

    /**
     * return true if the entry at index is a char.
     */
    bool is_char (int index) const {
        if (index >= (int) (m_strings.size () + m_phrases.size ()) &&
            index < (int) number_of_candidates ())
            return true;
        return false;
    }

    bool is_char_in_page (int page_index) const {
        if (page_index >= 0 && page_index < get_current_page_size ())
            return is_char (page_index + get_current_page_start ());
        return false;
    }

    WideString get_string (int index) const {
        if (is_string (index))
            return m_strings [index];
        return WideString ();
    }

    WideString get_string_in_page (int page_index) const {
        if (page_index >=0 && page_index < get_current_page_size ())
            return get_string (page_index + get_current_page_start ());
        return WideString ();
    }

    /**
     * return the phrase at index, if it's not a phrase, a empty phrase
     * will be returned.
     */
    Phrase get_phrase (int index) const {
        if (is_phrase (index))
            return m_phrases [index - m_strings.size ()];
        return Phrase ();
    }

    Phrase get_phrase_in_page (int page_index) const {
        if (page_index >=0 && page_index < get_current_page_size ())
            return get_phrase (page_index + get_current_page_start ());
        return Phrase ();
    }

    /**
     * return the char at index, if it's not a char, zero will be returned.
     */
    ucs4_t get_char (int index) const {
        if (is_char (index))
            return m_chars [index - m_strings.size () - m_phrases.size ()];
        return 0;
    }

    ucs4_t get_char_in_page (int page_index) const {
        if (page_index >= 0 && page_index < get_current_page_size ())
            return get_char (page_index + get_current_page_start ());
        return 0;
    }
};

#endif

/*
vi:ts=4:nowrap:ai:expandtab
*/
