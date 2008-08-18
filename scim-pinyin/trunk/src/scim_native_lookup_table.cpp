/** @file scim_lookup_table.cpp
 * implementation of class LookupTable.
 */

/*
 * Smart Pinyin Input Method
 * 
 * Copyright (c) 2005 James Su <suzhe@tsinghua.org.cn>
 *
 * $Id: scim_native_lookup_table.cpp,v 1.1.1.1 2005/01/06 13:30:58 suzhe Exp $
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

#include <scim.h>
#include "scim_pinyin_private.h"
#include "scim_phrase.h"
#include "scim_native_lookup_table.h"

//implementation of NativeLookupTable
NativeLookupTable::NativeLookupTable (int page_size)
    : LookupTable (page_size)
{
    std::vector <WideString> labels;
    char buf [2] = { 0, 0 };
    for (int i = 0; i < 9; ++i) {
        buf [0] = '1' + i;
        labels.push_back (utf8_mbstowcs (buf));
    }
    buf [0] = '0';
    labels.push_back (utf8_mbstowcs (buf));

    set_candidate_labels (labels);
}

bool
NativeLookupTable::append_entry (const WideString &entry)
{
    if (entry.length () > 0) {
        m_strings.push_back (entry);
        return true;
    }
    return false;
}

bool
NativeLookupTable::append_entry (const Phrase& entry)
{
    if (entry.is_enable ()) {
        m_phrases.push_back (entry);
        return true;
    }
    return false;
}

bool
NativeLookupTable::append_entry (const ucs4_t& entry)
{
    if (entry != 0) {
        m_chars.push_back (entry);
        return true;
    }
    return false;
}

WideString
NativeLookupTable::get_candidate (int index) const
{
    if (index < 0 || index >= (int) number_of_candidates ())
        return WideString ();

    if (index < (int) m_strings.size ()) {
        return m_strings [index];
    } else if (index < (int) (m_strings.size () + m_phrases.size ())) {
        return m_phrases [index - m_strings.size ()].get_content ();
    } else {
        return WideString (m_chars.begin () + index - m_strings.size () - m_phrases.size (), 
                        m_chars.begin () + index - m_strings.size () - m_phrases.size () + 1);
    }
    return WideString ();
}

AttributeList
NativeLookupTable::get_attributes (int index) const
{
    AttributeList attrs;

#if 0
    if (index >= 0 && index < (int) m_strings.size ())
        attrs.push_back (Attribute (0, m_strings [index].length (), SCIM_ATTR_FOREGROUND, SCIM_RGB_COLOR(32, 32, 255)));
#endif

    return attrs;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
