/** @file scim_generic_table.cpp
 * Implementation of class GenericKeyIndexLib and GenericTablePhraseLib.
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
 * $Id: scim_generic_table.cpp,v 1.1.1.1 2005/05/24 23:37:49 liuspider Exp $
 *
 */

#define Uses_STL_FUNCTIONAL
#define Uses_STL_VECTOR
#define Uses_STL_IOSTREAM
#define Uses_STL_FSTREAM
#define Uses_STL_ALGORITHM
#define Uses_STL_UTILITY
#define Uses_STL_IOMANIP
#define Uses_C_STDIO
#define Uses_C_CTYPE
#define Uses_C_STDLIB
#define Uses_SCIM_UTILITY
#define Uses_SCIM_LOOKUP_TABLE

#include <scim.h>
#include "scim_generic_table.h"

using namespace scim;

/* Utility classes for sorting */
class GenericKeyIndexPairLessThanByKey
{
public:
    bool operator () (const GenericKeyIndexPair & lhs,
                      const GenericKeyIndexPair & rhs) const {
        return lhs.first < rhs.first;
    }
};

class GenericTablePhraseLessThanByIndex
{
    const GenericTablePhraseLib *m_lib;
public:
    GenericTablePhraseLessThanByIndex (const GenericTablePhraseLib *lib) : m_lib (lib) { }

    bool operator () (const GenericKeyIndexPair & lhs,
                      const GenericKeyIndexPair & rhs) const {
        return lhs.second < rhs.second;
    }
};

class GenericTablePhraseLessThanByLength
{
    const GenericTablePhraseLib *m_lib;
public:
    GenericTablePhraseLessThanByLength (const GenericTablePhraseLib *lib) : m_lib (lib) { }

    bool operator () (const GenericKeyIndexPair & lhs,
                      const GenericKeyIndexPair & rhs) const {
        if (m_lib->get_phrase_key_length (lhs.second) == 
            m_lib->get_phrase_key_length (rhs.second)) {
            uint32 llen = m_lib->get_phrase_length (lhs.second); 
            uint32 rlen = m_lib->get_phrase_length (rhs.second); 
            if (llen > rlen) return true;
            else if (llen < rlen) return false;
            else return m_lib->get_phrase_frequency (lhs.second) >
                        m_lib->get_phrase_frequency (rhs.second);
        }

        return  m_lib->get_phrase_key_length (lhs.second) <
                m_lib->get_phrase_key_length (rhs.second);
    }
};

class GenericTablePhraseLessThanByFrequency
{
    const GenericTablePhraseLib *m_lib;
public:
    GenericTablePhraseLessThanByFrequency (const GenericTablePhraseLib *lib) : m_lib (lib) {}

    bool operator () (const GenericKeyIndexPair & lhs,
                      const GenericKeyIndexPair & rhs) const {
        if (m_lib->get_phrase_key_length (lhs.second) == 
            m_lib->get_phrase_key_length (rhs.second))
            return  m_lib->get_phrase_frequency (lhs.second) >
                    m_lib->get_phrase_frequency (rhs.second);

        return  m_lib->get_phrase_key_length (lhs.second) <
                m_lib->get_phrase_key_length (rhs.second);
    }
};

class GenericTablePhraseLessThanByKeyLength
{
    const GenericTablePhraseLib *m_lib;
public:
    GenericTablePhraseLessThanByKeyLength (const GenericTablePhraseLib *lib) : m_lib (lib) {}

    bool operator () (const GenericKeyIndexPair & lhs,
                      const GenericKeyIndexPair & rhs) const {
        return  m_lib->get_phrase_key_length (lhs.second) <
                m_lib->get_phrase_key_length (rhs.second);
    }
};

class GenericKeyIndexPairEqualToByKey
{
public:
    bool operator () (const GenericKeyIndexPair & lhs,
                      const GenericKeyIndexPair & rhs) const {
        return lhs.first == rhs.first;
    }
};

/* Implementing of GenericKeyIndexLib */
GenericKeyIndexLib::GenericKeyIndexLib ()
{
    clear_all ();
}

GenericKeyIndexLib::GenericKeyIndexLib (const String &valid_chars)
{
    set_valid_chars (valid_chars);
}


bool
GenericKeyIndexLib::valid () const
{
    return  m_num_of_valid_chars > 0 &&
            m_max_key_length > 0 &&
            m_max_key_value > 1;
}

bool
GenericKeyIndexLib::set_valid_chars (const String &valid_chars, uint32 max_key_length)
{
    if (valid_chars.length () == 0)
        return false;

    clear_all ();

    for (String::const_iterator it=valid_chars.begin (); it!=valid_chars.end (); ++it) {
        if (*it > ' ') {
            ++m_num_of_valid_chars;
            m_char_value_table [*it] = m_num_of_valid_chars;
            m_value_char_table [m_num_of_valid_chars] = *it;
        }
    }

    if (m_num_of_valid_chars == 0)
        return false;

    // calculate m_max_key_length;
    int p = 0, m = m_num_of_valid_chars + 1;
    while (m > 0) {
        m /= 2;
        ++p;
    }

    m_max_key_length = sizeof (uint32) * 8 / p;

    if (max_key_length > 0 && max_key_length < m_max_key_length)
        m_max_key_length = max_key_length;

    m_max_key_value = 1;

    for (uint32 i=0; i<m_max_key_length; ++i)
        m_max_key_value *= (m_num_of_valid_chars+1);

    m_max_key_value -= 1;

    return true;
}

String
GenericKeyIndexLib::get_valid_chars () const
{
    String valid_chars;

    for (uint32 i=1; i<=m_num_of_valid_chars; ++i) {
        valid_chars += m_value_char_table [i];
    }

    return valid_chars;
}

bool
GenericKeyIndexLib::set_single_wildcards (const String &singles)
{
    bool ret = false;
    uint32 i;
    char ch;

    for (i=0; i<128; ++i) {
        if (m_char_value_table [i] == SCIM_GT_SINGLE_WILDCARD_VALUE)
            m_char_value_table [i] = 0;
    }

    //Used by find_key_indexes to simulate wildcard key.
    m_char_value_table [1] = SCIM_GT_SINGLE_WILDCARD_VALUE;

    for (i=0; i<singles.length (); ++i) {
        ch = singles [i];
        if (ch > ' ' && m_char_value_table [ch] == 0) {
            m_char_value_table [ch] = SCIM_GT_SINGLE_WILDCARD_VALUE;
            ret = true;
        }
    }

    return ret;
}

bool
GenericKeyIndexLib::set_multi_wildcards (const String &multis)
{
    bool ret = false;
    uint32 i;
    char ch;

    for (i=0; i<128; ++i) {
        if (m_char_value_table [i] == SCIM_GT_MULTI_WILDCARD_VALUE)
            m_char_value_table [i] = 0;
    }

    //Used by find_key_indexes to simulate wildcard key.
    m_char_value_table [2] = SCIM_GT_MULTI_WILDCARD_VALUE;

    for (i=0; i<multis.length (); ++i) {
        ch = multis [i];
        if (ch > ' ' && m_char_value_table [ch] == 0) {
            m_char_value_table [ch] = SCIM_GT_MULTI_WILDCARD_VALUE;
            ret = true;
        }
    }

    return ret;
}

String
GenericKeyIndexLib::get_single_wildcards () const
{
    String singles;

    for (int i= ' ' + 1; i<128; ++i)
        if (m_char_value_table [i] == SCIM_GT_SINGLE_WILDCARD_VALUE)
            singles += (char) i;

    return singles;
}

String
GenericKeyIndexLib::get_multi_wildcards () const
{
    String multis;

    for (int i= ' ' + 1; i<128; ++i)
        if (m_char_value_table [i] == SCIM_GT_MULTI_WILDCARD_VALUE)
            multis += (char) i;

    return multis;
}

bool
GenericKeyIndexLib::is_valid_char (char val) const
{
    if (val > ' ' &&
        (m_char_value_table [val] > 0 ||
         m_char_value_table [val] == SCIM_GT_SINGLE_WILDCARD_VALUE ||
         m_char_value_table [val] == SCIM_GT_MULTI_WILDCARD_VALUE))
        return true;
    return false;
}

bool
GenericKeyIndexLib::is_wildcard_char (char val) const
{
    if (val > ' ' &&
         (m_char_value_table [val] == SCIM_GT_SINGLE_WILDCARD_VALUE ||
          m_char_value_table [val] == SCIM_GT_MULTI_WILDCARD_VALUE))
        return true;
    return false;
}

bool
GenericKeyIndexLib::is_valid_key (const String &key) const
{
    if (key.length () == 0 || key.length () > m_max_key_length)
        return false;

    bool has_single_wildcard = false;
    bool has_multi_wildcard = false;
    bool no_more_single_wildcard = false;
    char ch;

    for (String::const_iterator i=key.begin (); i!=key.end (); ++i) {
        ch = *i;
        if (!is_valid_char (ch)) {
            return false;
        } else if (m_char_value_table [ch] == SCIM_GT_SINGLE_WILDCARD_VALUE) {
            if (has_multi_wildcard || no_more_single_wildcard)
                return false;
            has_single_wildcard = true;
        } else if (m_char_value_table [ch] == SCIM_GT_MULTI_WILDCARD_VALUE) {
            if (has_single_wildcard || has_multi_wildcard)
                return false;
            has_multi_wildcard = true;
        } else if (has_single_wildcard) {
            no_more_single_wildcard = true;
        }
    }

    return true;
}

bool
GenericKeyIndexLib::is_wildcard_key (const String &key) const
{
    for (String::const_iterator i=key.begin (); i!=key.end (); ++i) {
        if (m_char_value_table [(*i) & 0x7f] == SCIM_GT_SINGLE_WILDCARD_VALUE ||
            m_char_value_table [(*i) & 0x7f] == SCIM_GT_MULTI_WILDCARD_VALUE)
            return true;
    }
    return false;
}

bool
GenericKeyIndexLib::add_key_indexes (const std::vector <String> &keys,
                                         const std::vector <uint32> & indexes)
{
    if (keys.size () != indexes.size () || keys.size () == 0)
        return false;

    std::vector <std::pair <uint32, uint32> > val;

    m_keys.reserve (m_keys.size () + keys.size ());

    for (uint32 i=0; i<keys.size (); ++i) {
        if (is_valid_key (keys [i])) {
            compile_key (val, keys [i]);
            if (val.size () == 1 && val[0].first == val[0].second) {
                m_keys.push_back (GenericKeyIndexPair
                                    (val [0].first, indexes[i]));
            }
        }
    }

    std::sort (m_keys.begin (), m_keys.end (),
                GenericKeyIndexPairLessThanByKey ());

    return true;
}

bool
GenericKeyIndexLib::add_key_indexes (const std::vector <uint32> &keys,
                                         const std::vector <uint32> & indexes)
{
    if (keys.size () != indexes.size () || keys.size () == 0)
        return false;

    m_keys.reserve (m_keys.size () + keys.size ());

    for (uint32 i=0; i<keys.size (); ++i) {
        if (keys [i] > 0 && keys [i] <= m_max_key_value) {
            m_keys.push_back (GenericKeyIndexPair (keys [i], indexes[i]));
        }
    }

    std::sort (m_keys.begin (), m_keys.end (),
                GenericKeyIndexPairLessThanByKey ());

    return true;
}

bool
GenericKeyIndexLib::insert_key_index (const String &key, uint32 index)
{
    if (!is_valid_key (key) || is_wildcard_key (key))
        return false;

    std::vector <std::pair <uint32, uint32> > val;

    compile_key (val, key);

    if (val.size () == 0)
        return false;

    GenericKeyIndexPair keypair (val [0].first, index);

    GenericKeyIndexVector::iterator it =
        std::lower_bound (m_keys.begin (), m_keys.end (), keypair,
                        GenericKeyIndexPairLessThanByKey ());

    m_keys.insert (it, keypair); 
    return true;
}

bool
GenericKeyIndexLib::erase_key_index (const String &key, int index)
{
    if (!is_valid_key (key) || is_wildcard_key (key))
        return false;

    std::vector <std::pair <uint32, uint32> > val;

    compile_key (val, key);

    if (val.size () == 0)
        return false;

    GenericKeyIndexVector::iterator itl =
        std::lower_bound (m_keys.begin (), m_keys.end (),
                            GenericKeyIndexPair (val [0].first, 0),
                            GenericKeyIndexPairLessThanByKey ());

    GenericKeyIndexVector::iterator itu =
        std::upper_bound (m_keys.begin (), m_keys.end (), 
                            GenericKeyIndexPair (val [0].second, 0),
                            GenericKeyIndexPairLessThanByKey ());

    if (itl != m_keys.end ()) {
        if (index < 0) {
            m_keys.erase (itl, itu);
        } else {
            for (GenericKeyIndexVector::iterator it = itl; it!=itu; ++it)
                if (it->second == (uint32) index) {
                    m_keys.erase (it);
                    break;
                }
        }
        return true;
    } 
    return false;
}

bool
GenericKeyIndexLib::is_defined_key (const String &key, bool auto_wildcard) const
{
    if (!is_valid_key (key))
        return false;

    std::vector <std::pair <uint32, uint32> > val;
    uint32 interval;
    String nkey = key;
    GenericKeyIndexVector::const_iterator itl, itu;

    int find_times = 1;

    //If auto_wildcard is true and the key can be wildcarded,
    //then append '\002' (private multi wildcard) to the end of the key.

    if (auto_wildcard && !is_wildcard_key (nkey))
        find_times = 2;

    do {
        interval = compile_key (val, nkey);

        if (val.size () == 0 || interval == 0)
            return false;

        if (val.size () == 1 && val [0].first == val [0].second) {
            if (std::binary_search (m_keys.begin (), m_keys.end (),
                                    GenericKeyIndexPair (val [0].first, 0),
                                    GenericKeyIndexPairLessThanByKey ())) {
                return true;
            }
        } else {
            for (uint i=0; i<val.size (); ++i) {
                itl = std::lower_bound (m_keys.begin (), m_keys.end (),
                                        GenericKeyIndexPair (val [i].first, 0),
                                        GenericKeyIndexPairLessThanByKey ());
    
                itu = std::upper_bound (m_keys.begin (), m_keys.end (), 
                                        GenericKeyIndexPair (val [i].second, 0),
                                        GenericKeyIndexPairLessThanByKey ());
    
                if (itl != m_keys.end () && itl < itu) {
                    if (interval == 1) {
                        return true;
                    } else {
                        for (GenericKeyIndexVector::const_iterator it = itl; it!=itu; ++it)
                            if ((it->first - val [i].first) % interval == 0) {
                                return true;
                            }
                    }
                } 
            }
        }

        --find_times;

        if (find_times >= 1)
            nkey = nkey + "\002";

    } while (find_times >= 1);

    return false;
}

bool
GenericKeyIndexLib::find_key_indexes (GenericKeyIndexVector & key_indexes,
                                          const String &key,
                                          bool auto_wildcard) const
{
    if (!is_valid_key (key))
        return false;

    std::vector <std::pair <uint32, uint32> > val;
    uint32 interval;
    String nkey = key;
    int find_times = 1;
    GenericKeyIndexVector::const_iterator itl, itu;

    key_indexes.clear ();

    //If auto_wildcard is true and the key can be wildcarded,
    //then append '\002' (private multi wildcard) to the end of the key.

    if (auto_wildcard && !is_wildcard_key (nkey))
        find_times = 2;

    do {
        interval = compile_key (val, nkey);

        if (val.size () == 0 || interval == 0)
            break;

        for (uint i=0; i<val.size (); ++i) {
            itl = std::lower_bound (m_keys.begin (), m_keys.end (),
                                    GenericKeyIndexPair (val [i].first, 0),
                                    GenericKeyIndexPairLessThanByKey ());

            itu = std::upper_bound (m_keys.begin (), m_keys.end (), 
                                    GenericKeyIndexPair (val [i].second, 0),
                                    GenericKeyIndexPairLessThanByKey ());

            if (itl != m_keys.end ()) {
                if (interval == 1) {
                    for (GenericKeyIndexVector::const_iterator it = itl; it!=itu; ++it)
                        key_indexes.push_back (*it);
                } else {
                    for (GenericKeyIndexVector::const_iterator it = itl; it!=itu; ++it)
                        if ((it->first - val [i].first) % interval == 0)
                            key_indexes.push_back (*it);
                }
            } 
        }

        --find_times;

        if (find_times >= 1)
            nkey = nkey + "\002";

    } while (find_times >= 1);

    return key_indexes.size () > 0;
}

void
GenericKeyIndexLib::clear_all ()
{
    for (int i=0; i<128; ++i) {
        m_char_value_table [i] = 0;
        m_value_char_table [i] = 0;
    }

    m_num_of_valid_chars = 0;
    m_max_key_length     = 0;
    m_max_key_value      = 1;

    clear_keys ();
}

void
GenericKeyIndexLib::clear_keys ()
{
    GenericKeyIndexVector ().swap (m_keys);
}

void
GenericKeyIndexLib::compact_memory ()
{
    GenericKeyIndexVector (m_keys).swap (m_keys);
}

uint32
GenericKeyIndexLib::key_to_value (const String &key) const
{
    if (!is_valid_key (key) || is_wildcard_key (key))
        return 0;

    std::vector <std::pair <uint32, uint32> > val;
    compile_key (val, key);

    if (val.size () > 0)
        return val [0].first;

    return 0;
}

String
GenericKeyIndexLib::value_to_key (uint32 value) const
{
    String key;

    if (value == 0 || value > m_max_key_value)
        return key;

    uint32 base = m_num_of_valid_chars + 1;

    for (uint32 i=0; i<m_max_key_length && value > 0; ++i) {
        key = m_value_char_table [value % base] + key;
        value /= base;
    }
    return key;
}

uint32
GenericKeyIndexLib::compile_key (std::vector <std::pair <uint32, uint32> > &range_list, 
                                     const  String &key) const
{
    uint32 pos = 0;
    uint32 interval = 1;
    uint32 min_val = 0;
    uint32 max_val = 0;
    uint32 base = m_num_of_valid_chars + 1;
    uint32 power;
    uint32 i;

    bool has_multi_wildcard  = false;
    bool has_single_wildcard = false;

    char ch, bit;

    range_list.clear ();

    for (power = 0; power < m_max_key_length; ++power){
        min_val *= base;
        max_val *= base;

        ch  = key [pos];
        bit = m_char_value_table [ch];

        if (bit == SCIM_GT_MULTI_WILDCARD_VALUE) {
            has_multi_wildcard = true;
            min_val += 1;
            max_val += m_num_of_valid_chars;
            range_list.push_back (std::pair <uint32, uint32> (min_val, max_val));
            for (i=0; i<m_max_key_length-power-1; ++i) {
                range_list.push_back (std::pair <uint32, uint32> (
                                        range_list [i].first * base + 1,
                                        range_list [i].second * base + m_num_of_valid_chars));
            }
        } else if (bit == SCIM_GT_SINGLE_WILDCARD_VALUE) {
            has_single_wildcard = true;
            min_val += 1;
            max_val += m_num_of_valid_chars;
        } else if (pos < key.length ()){
            min_val += (uint32) bit;
            max_val += (uint32) bit;
            if (has_single_wildcard || has_multi_wildcard)
                interval *= base;
            if (has_multi_wildcard) {
                for (i=0; i<range_list.size (); ++i) {
                    range_list [i].first = range_list [i].first * base + (uint32) bit;
                    range_list [i].second = range_list [i].second * base + (uint32) bit;
                }
                if (range_list.back ().first > m_max_key_value)
                    range_list.pop_back ();
            }
        }

        ++pos;

        if (pos >= key.length ()) {
            if (!has_multi_wildcard)
                range_list.push_back (std::pair <uint32, uint32> (min_val, max_val));
            break;
        }
    }
#if 0
    std::cerr << "Key = " << key << " " << "interval =" << interval << std::endl;
    for (int i=0; i<range_list.size (); i++)
        std::cerr <<"   " << range_list [i].first << " " << range_list [i].second << std::endl;
#endif
    return interval;
}

// Implementation of GenericTablePhraseLib
static const char scim_generic_table_phrase_lib_text_header [] = "SCIM_Generic_Table_Phrase_Library_TEXT";
static const char scim_generic_table_phrase_lib_binary_header [] = "SCIM_Generic_Table_Phrase_Library_BINARY";
static const char scim_generic_table_phrase_lib_version [] = "VERSION_0_5";

static const char scim_generic_table_freq_lib_text_header [] = "SCIM_Generic_Table_Frequency_Library_TEXT";
static const char scim_generic_table_freq_lib_binary_header [] = "SCIM_Generic_Table_Frequency_Library_BINARY";
static const char scim_generic_table_freq_lib_version [] = "VERSION_0_3";

class _StringLessThanByFirstChar
{
public:
    bool operator () (const String & lhs, char rhs) const {
        return lhs [0] < rhs;
    }
    bool operator () (char lhs, const String & rhs) const {
        return lhs < rhs [0];
    }
    bool operator () (const String & lhs, const String & rhs) const {
        return lhs [0] < rhs [0];
    }
};

static inline String
_trim_blank (const String &str)
{
    String::size_type begin, len;

    begin = str.find_first_not_of (" \t\n\v");

    if (begin == String::npos)
        return String ();

    len = str.find_last_not_of (" \t\n\v");

    if (len != String::npos)
        len = len - begin + 1;

    return str.substr (begin, len);
}

static inline String
_get_param_portion (const String &str, const String &delim = "=")
{
    String ret = str;
    String::size_type pos = ret.find_first_of (String (" \t\v") + delim);

    if (pos != String::npos)
        ret.erase (pos, String::npos);

    return ret;
}

static inline String
_get_value_portion (const String &str, const String &delim = "=")
{
    String ret = str;
    String::size_type pos;

    pos = ret.find_first_of (delim);

    if (pos != String::npos)
        ret.erase (0, pos + 1);

    pos = ret.find_first_not_of(" \t\v");

    if (pos != String::npos)
        ret.erase (0, pos);

    pos = ret.find_last_not_of(" \t\v");

    if (pos != String::npos)
        ret.erase (pos + 1, String::npos);

    return ret;
}

static inline String  
_get_line (std::istream &is)
{
    char temp [1024];
    String res;

    while (1) {
        is.getline (temp, 1023);
        res = _trim_blank (String (temp));

        if (res.length () > 0 && res [0] != '#') return res;
        if (is.eof ()) return String ();
    }
}

static inline WideString
_hex_to_wide_string (const String &str)
{
    WideString ret;
    uint32 i = 0;

    while (i <= str.length () - 6 && str [i] == '0' && tolower (str [i+1]) == 'x') {
        ucs4_t wc = (ucs4_t) strtol (str.substr (i+2, 4).c_str (), NULL, 16);

        if (wc)
            ret.push_back (wc);

        i += 6;
    }
    return ret;
}

GenericTablePhraseLib::GenericTablePhraseLib (const String &libfile)
{
    load_lib (libfile);
}

GenericTablePhraseLib::GenericTablePhraseLib (std::istream &is)
{
    input (is);
}

bool
GenericTablePhraseLib::valid () const
{
    return  m_uuid.length () &&
            m_indexes.valid () &&
            m_locales.length () &&
            m_select_keys.length () &&
            m_default_name.length ();
}

bool
GenericTablePhraseLib::input (std::istream &is)
{
    if (!is) return false;

    clear ();

    String temp;
    uint32 i;
    bool binary;

    // Check file header
    {
        temp = _get_line (is);
        if (temp == String (scim_generic_table_phrase_lib_text_header))
            binary = false;
        else if (temp == String (scim_generic_table_phrase_lib_binary_header))
            binary = true;
        else
            return false;

        temp = _get_line (is);

        if (temp != String (scim_generic_table_phrase_lib_version))
            return false;
    }

    std::vector <uint32> indexes;
    std::vector <String> keys;
    std::vector <uint32> key_values;
    std::vector <String> char_prompts;

    String phrasestr;
    String freqstr;
    uint32 freq;

    WideString wide_phrase;

    String paramstr;
    String valuestr;
    String valid_chars;
    String single_wildcards;
    String multi_wildcards;
    String split_chars;

    uint32 max_key_length = 0;

    //Read definitions
    if (_get_line (is) != String ("BEGIN_DEFINITION"))
        return false;

    while (1) {
        temp = _get_line (is);

        if (temp.length () == 0) return false;
        if (temp == String ("END_DEFINITION")) break;

        paramstr = _get_param_portion (temp);
        valuestr = _get_value_portion (temp);

        if (paramstr.length () == 0 || valuestr.length () == 0)
            return false;

        if (paramstr == "NAME") { // Get table default name.
            m_default_name = valuestr;
        } else if (paramstr.substr (0,5) == "NAME.") { //Get table name for each locales.
            m_lang_names.push_back (
                paramstr.substr (5,paramstr.length () - 5) + " = " + valuestr);
        } else if (paramstr == "UUID") {
            m_uuid = valuestr;
        } else if (paramstr == "ICON") {
            m_icon_file = valuestr;
        } else if (paramstr == "LOCALES") { //Get supported locales.
            m_locales = valuestr;
        } else if (paramstr == "AUTHOR") { //Get the table author.
            m_author = utf8_mbstowcs (valuestr);
        } else if (paramstr == "SERIAL_NUMBER") {
            m_serial_number = atoi (valuestr.c_str ());
        } else if (paramstr == "AUTO_SELECT") { //Get auto_select value.
            if (valuestr == "TRUE" || valuestr == "true" || valuestr == "True")
                m_auto_select = true;
            else
                m_auto_select = false;
        } else if (paramstr == "AUTO_WILDCARD") { //Get auto wildcard value.
            if (valuestr == "TRUE" || valuestr == "true" || valuestr == "True")
                m_auto_wildcard = true;
            else
                m_auto_wildcard = false;
        } else if (paramstr == "DYNAMIC_ADJUST") { //Get dynamic_adjust value.
            if (valuestr == "TRUE" || valuestr == "true" || valuestr == "True")
                m_dynamic_adjust = true;
            else
                m_dynamic_adjust = false;
        } else if (paramstr == "VALID_INPUT_CHARS") { //Get valid input chars.
            valid_chars = valuestr;
        } else if (paramstr == "SINGLE_WILDCARD_CHAR") { //Get single wildcard char.
            single_wildcards = valuestr;
        } else if (paramstr == "MULTI_WILDCARD_CHAR") { //Get multi wildcard char.
            multi_wildcards = valuestr;
        } else if (paramstr == "SELECT_KEYS") { //Get select keys.
            for (uint32 i=0; i<valuestr.length () && i<SCIM_LOOKUP_TABLE_MAX_PAGESIZE; ++i)
                if (valuestr [i] > ' ') m_select_keys += valuestr [i];
        } else if (paramstr == "SPLIT_CHARS") { //Get split chars.
            split_chars = valuestr;
        } else if (paramstr == "STATUS_PROMPT") {
            m_status_prompt = utf8_mbstowcs (valuestr);
        } else if (paramstr == "PAGE_UP_KEYS") {
            m_pageup_keys = valuestr;
        } else if (paramstr == "PAGE_DOWN_KEYS") {
            m_pagedown_keys = valuestr;
        } else if (paramstr == "MAX_KEY_LENGTH") {
            max_key_length = atoi (valuestr.c_str ());
        } else if (paramstr == "BEGIN_CHAR_PROMPTS_DEFINITION") { //Read char names.
            while (1) {
                temp = _get_line (is);

                if (temp == String ("END_CHAR_PROMPTS_DEFINITION"))
                    break;

                if (temp.length () < 3 || temp [1] != ' ')
                    return false;

                char_prompts.push_back (String (temp));
            }
        } else {
            return false;
        }
    }

    //Check inputed information.

    if (m_uuid.length () == 0)
        return false;

    if (m_indexes.set_valid_chars (valid_chars, max_key_length) == false) {
        clear ();
        return false;
    }

    m_indexes.set_single_wildcards (single_wildcards);
    m_indexes.set_multi_wildcards (multi_wildcards);

    for (i=0; i<char_prompts.size (); ++i) {
        if (m_indexes.is_valid_char (char_prompts [i][0]))
            m_char_prompts.push_back (char_prompts [i]);
    }

    if (m_default_name.length () == 0) {
        if (m_lang_names.size ()) {
            m_default_name = _get_value_portion (m_lang_names [0]);
        } else {
            return false;
        }
    }

    for (i=0; i<split_chars.length (); ++i)
        if (!m_indexes.is_valid_char (split_chars [i]))
            m_split_chars.push_back (split_chars [i]);

    //Read phrase table
    if (_get_line (is) != String ("BEGIN_TABLE"))
        return false;

    uint32 header;

    //Text format
    if (!binary){
        while (!is.eof ()) {
            temp = _get_line (is);

            if (temp.length () == 0) return false;
            if (temp == String ("END_TABLE")) break;

            paramstr = _get_param_portion (temp, " \t");
            valuestr = _get_value_portion (temp, " \t");

            if (paramstr.length () == 0 || valuestr.length () == 0)
                return false;

            //Split the phrase and it's frequency.
            phrasestr = _get_param_portion (valuestr, " \t");
            freqstr   = _get_value_portion (valuestr, " \t");

            if (phrasestr.length () >= 6 && phrasestr [0] == '0'
                && tolower (phrasestr[1]) == 'x')
                wide_phrase = _hex_to_wide_string (phrasestr);
            else
                wide_phrase = utf8_mbstowcs (phrasestr);

            if (wide_phrase.length ()) {
                //Get frequency
                if (freqstr.length () > 0)
                    freq = atoi (freqstr.c_str ());
                else
                    freq = 0;

                //Phrase length cannot greater than SCIM_GT_MAX_PHRASE_LENGTH 
                if (wide_phrase.length () > SCIM_GT_MAX_PHRASE_LENGTH)
                    wide_phrase = wide_phrase.substr (0, SCIM_GT_MAX_PHRASE_LENGTH);

                //store the key.
                keys.push_back (paramstr); 

                //store the index.
                indexes.push_back (m_content.size ()); 

                header = (wide_phrase.length () & 0x1f);           // phrase length
                header |= (((paramstr.length () - 1) & 0x7) << 5); // key length
                header |= ((freq & 0x3fffff) << 8);                // frequency
                header |= 0x80000000;                              // valid flag

                //store the length and frequency of the phrase.
                m_content.push_back (header); 

                //store the content of the phrase.
                m_content.insert (m_content.end (), wide_phrase.begin (), wide_phrase.end ());
            }
        }

        //Add key-index std::paires into m_indexes.
        m_indexes.add_key_indexes (keys, indexes);

    } else {
        char bytes [8];
        ucs4_t wc;
        uint32 num;

        is.read (bytes, 8);

        num = scim_bytestouint32 ((unsigned char*)bytes);
        m_content.reserve (scim_bytestouint32 ((unsigned char*) (bytes + 4)) + 1);

        indexes.reserve (num + 1);
        key_values.reserve (num + 1);

        for (i=0; i<num && !is.eof (); ++i) {
            is.read (bytes, 8);
            header = scim_bytestouint32 ((unsigned char*)bytes);
            key_values.push_back (scim_bytestouint32 ((unsigned char*) (bytes+4)));
            indexes.push_back (m_content.size ());

            //Clear the updated flag before push_back. 
            m_content.push_back (header & (~0x40000000));

            for (uint32 j=0; j<(header & 0x1f); ++j) {
                if ((wc = utf8_read_wchar (is)) == 0)
                    return false;
                m_content.push_back (wc);
            }
        }

        m_indexes.add_key_indexes (key_values, indexes);

        temp = _get_line (is);

        if (temp != String ("END_TABLE"))
            return false;
    }

    std::sort (m_char_prompts.begin (), m_char_prompts.end (),
                _StringLessThanByFirstChar ());

    std::sort (m_lang_names.begin (), m_lang_names.end ());

    return true;
}

bool
GenericTablePhraseLib::output (std::ostream &os, bool binary) const
{
    if (!valid () || !os) return false;

    uint32 i;
    String tmp;
    GenericKeyIndexVector::const_iterator it;
    GenericKeyIndexVector indexes;

    if (binary)
        os << scim_generic_table_phrase_lib_binary_header << "\n";
    else
        os << scim_generic_table_phrase_lib_text_header << "\n";

    os << scim_generic_table_phrase_lib_version << "\n";

    os << "# Begin Table definition.\n";
    os << "BEGIN_DEFINITION\n";

    os << "UUID = " << m_uuid << "\n";

    if (m_icon_file.length ())
        os << "ICON = " << m_icon_file << "\n";

    os << "SERIAL_NUMBER = " << m_serial_number << "\n";

    if (m_default_name.length () == 0)
        os << "# ";
    os << "NAME = " << m_default_name << "\n";

    for (i=0; i<m_lang_names.size (); ++i)
        os << "NAME." << m_lang_names [i] << "\n";

    if (m_locales.length () == 0)
        os << "# ";
    os << "LOCALES = " << m_locales << "\n";

    if (m_author.length () == 0)
        os << "# ";
    os << "AUTHOR = " << utf8_wcstombs (m_author) << "\n";

    os << "STATUS_PROMPT = " << utf8_wcstombs (m_status_prompt) << "\n";

    os << "AUTO_SELECT = " << (m_auto_select?"TRUE":"FALSE") << "\n";
    os << "AUTO_WILDCARD = " << (m_auto_wildcard?"TRUE":"FALSE") << "\n";
    os << "DYNAMIC_ADJUST = " << (m_dynamic_adjust?"TRUE":"FALSE") << "\n";

    os << "VALID_INPUT_CHARS = " << m_indexes.get_valid_chars () << "\n";

    tmp = m_indexes.get_single_wildcards (); 
    if (tmp.length () == 0)
        os << "# ";
    os << "SINGLE_WILDCARD_CHAR = " << tmp << "\n";

    tmp = m_indexes.get_multi_wildcards (); 
    if (tmp.length () == 0)
        os << "# ";
    os << "MULTI_WILDCARD_CHAR = " << tmp << "\n";

    if (m_select_keys.length () == 0)
        os << "# ";
    os << "SELECT_KEYS = " << m_select_keys << "\n";

    if (m_split_chars.length () == 0)
        os << "# ";
    os << "SPLIT_CHARS = " << m_split_chars << "\n";

    os << "MAX_KEY_LENGTH = " << m_indexes.get_max_key_length () << "\n";

    if (m_pageup_keys.length () == 0)
        os << "# ";
    os << "PAGE_UP_KEYS = " << m_pageup_keys << "\n";

    if (m_pagedown_keys.length () == 0)
        os << "# ";
    os << "PAGE_DOWN_KEYS = " << m_pagedown_keys << "\n";

    if (m_char_prompts.size () > 0) {
        os << "BEGIN_CHAR_PROMPTS_DEFINITION\n";
        for (i=0; i<m_char_prompts.size (); ++i)
            os << m_char_prompts [i] << "\n";
        os << "END_CHAR_PROMPTS_DEFINITION\n";
    }

    os << "END_DEFINITION\n\n";

    os << "# Begin Table data.\n";

    os << "BEGIN_TABLE\n";

    indexes = m_indexes.get_all_key_indexes ();

    sort_indexes_by_index (indexes);

    if (!binary) {
        for (it = indexes.begin (); it != indexes.end (); ++it) {

            os << m_indexes.value_to_key (it->first) << "\t"
                << utf8_wcstombs (get_phrase (it->second)) << "\t"
                << get_phrase_frequency (it->second) << "\n";
        }
    } else {
        char bytes [8];
        std::vector <ucs4_t>::const_iterator cit;
        uint32 header;

        scim_uint32tobytes ((unsigned char*)bytes, m_indexes.number_of_keys ());
        scim_uint32tobytes ((unsigned char*)bytes+4, m_content.size ());

        os.write (bytes, 8);

        cit = m_content.begin ();
        for (it = indexes.begin (); it != indexes.end (); ++it) {
            header = (uint32) *(cit + it->second);
            scim_uint32tobytes ((unsigned char*)bytes, header);
            scim_uint32tobytes ((unsigned char*)bytes+4, it->first);
            os.write (bytes, 8);

            for (i=0; i<(header & 0x1f); ++i)
                utf8_write_wchar (os, *(cit+it->second+i+1));
        }
    }
    os << "END_TABLE\n";
    return true;
}

bool
GenericTablePhraseLib::input_phrase_frequencies (std::istream &is)
{
    if (!is) return false;

    String temp;
    uint32 i;
    bool binary;

    // Check file header
    {
        temp = _get_line (is);
        if (temp == String (scim_generic_table_freq_lib_text_header))
            binary = false;
        else if (temp == String (scim_generic_table_freq_lib_binary_header))
            binary = true;
        else
            return false;

        temp = _get_line (is);

        if (temp != String (scim_generic_table_freq_lib_version))
            return false;
    }

    String paramstr;
    String valuestr;

    uint32 freq;
    uint32 index;

    //Read definitions
    if (_get_line (is) != String ("BEGIN_DEFINITION"))
        return false;

    while (1) {
        temp = _get_line (is);

        if (temp.length () == 0) return false;
        if (temp == String ("END_DEFINITION")) break;

        paramstr = _get_param_portion (temp);
        valuestr = _get_value_portion (temp);

        if (paramstr.length () == 0 || valuestr.length () == 0)
            return false;

        if (paramstr == "NAME") { // Get table default name.
            // Check if the table name is OK.
            if (m_default_name != valuestr)
                return false;
        } else if (paramstr == "UUID") {
            if (m_uuid != valuestr)
                return false;
        } else if (paramstr == "SERIAL_NUMBER") {
            // Check if the serial number is OK.
            if (m_serial_number != atoi (valuestr.c_str ()))
                return false;
        } else {
            return false;
        }
    }

    //Read phrase table
    if (_get_line (is) != String ("BEGIN_FREQUENCY_TABLE"))
        return false;

    //Text format
    if (!binary){
        while (!is.eof ()) {
            temp = _get_line (is);

            if (temp.length () == 0) return false;
            if (temp == String ("END_FREQUENCY_TABLE")) break;

            paramstr = _get_param_portion (temp, " \t");
            valuestr = _get_value_portion (temp, " \t");

            if (paramstr.length () == 0 || valuestr.length () == 0)
                return false;

            //Split the phrase and it's frequency.
            index = atoi (paramstr.c_str ());
            freq  = atoi (valuestr.c_str ());

            set_phrase_frequency (index, freq);
        }
    } else {
        char bytes [8];

        while (!is.eof ()) {
            is.read (bytes, 8);

            index = scim_bytestouint32 ((unsigned char*)bytes);
            freq  = scim_bytestouint32 ((unsigned char*)(bytes+4));

            // End of file.
            if (index == (~0) && freq == (~0))
                break;

            set_phrase_frequency (index, freq);
        }

        temp = _get_line (is);

        if (temp != String ("END_FREQUENCY_TABLE"))
            return false;
    }

    return true;
}

bool
GenericTablePhraseLib::output_phrase_frequencies (std::ostream &os, bool binary) const
{
    if (!valid () || !os) return false;

    uint32 i;
    String tmp;
    GenericKeyIndexVector::const_iterator it;

    if (binary)
        os << scim_generic_table_freq_lib_binary_header << "\n";
    else
        os << scim_generic_table_freq_lib_text_header << "\n";

    os << scim_generic_table_freq_lib_version << "\n";

    os << "# Begin Table definition.\n";
    os << "BEGIN_DEFINITION\n";

    os << "UUID = " << m_uuid << "\n";

    os << "SERIAL_NUMBER = " << m_serial_number << "\n";

    if (m_default_name.length () > 0)
        os << "NAME = " << m_default_name << "\n";

    os << "END_DEFINITION\n\n";

    os << "# Begin Frequency Table data.\n";

    os << "BEGIN_FREQUENCY_TABLE\n";
    if (!binary) {
        for (it = m_indexes.get_all_key_indexes ().begin ();
            it != m_indexes.get_all_key_indexes ().end (); ++it) {
            if (is_updated_phrase (it->second)) {
                os << it->second << "\t"
                    << get_phrase_frequency (it->second) << "\n";
            }
        }
    } else {
        char bytes [8];
        std::vector <ucs4_t>::const_iterator cit;
        uint32 header;

        for (it = m_indexes.get_all_key_indexes ().begin ();
            it != m_indexes.get_all_key_indexes ().end (); ++it) {
            if (is_updated_phrase (it->second)) {
                scim_uint32tobytes ((unsigned char*)bytes, it->second);
                scim_uint32tobytes ((unsigned char*)bytes+4, get_phrase_frequency (it->second));
                os.write (bytes, 8);
            }
        }

        //Write the EOF signature
        scim_uint32tobytes ((unsigned char*)bytes, ~0);
        scim_uint32tobytes ((unsigned char*)bytes+4, ~0);
    }
    os << "END_FREQUENCY_TABLE\n";
    return true;
}

bool
GenericTablePhraseLib::load_lib (const String &libfile, const String &freqfile)
{
    std::ifstream is (libfile.c_str ()), isfreq (freqfile.c_str ());
    if (!is) return false;
    if (input (is)) {
        input_phrase_frequencies (isfreq);
        return true;
    }
    return false;
}

bool
GenericTablePhraseLib::save_lib (const String &libfile, const String &freqfile, bool binary) const
{
    std::ofstream os (libfile.c_str ()), osfreq (freqfile.c_str ());
    if (!os) return false;
    if (osfreq) output_phrase_frequencies (osfreq, binary);
    return output (os, binary);
}

bool
GenericTablePhraseLib::find_key_indexes (GenericKeyIndexVector &indexes,
                                             const String &key,
                                             bool sort_by_freq) const
{
    if (!valid ()) return false;
    bool ret = m_indexes.find_key_indexes (indexes, key, m_auto_wildcard);
    if (ret) {

// FIXME: don't erase duplicated phrases, this should be done within the tables.
#if 0
        sort_indexes_by_phrase (indexes);

        indexes.erase (std::unique (indexes.begin (), indexes.end (),
                            GenericTablePhraseEqualToByPhrase (this)),
                        indexes.end ());
#endif
        if (sort_by_freq)
            sort_indexes_by_frequency (indexes);
        else
            sort_indexes_by_length (indexes);
    }
    return ret;
}

bool
GenericTablePhraseLib::find_phrase_indexes (GenericKeyIndexVector &indexes,
                                         const WideString & phrase)
{
    if (!valid ()) return false;

    if (!m_sorted_phrase_indexes_initialized)
        initialize_sorted_phrase_indexes ();

    GenericKeyIndexVector::const_iterator lb, ub;

    indexes.clear ();

    lb = std::lower_bound  (m_sorted_phrase_indexes.begin (),
                            m_sorted_phrase_indexes.end (),
                            phrase,
                            GenericTablePhraseLessThanByPhrase (this));

    if (lb != m_sorted_phrase_indexes.end ()) {
        ub = std::upper_bound  (m_sorted_phrase_indexes.begin (),
                                m_sorted_phrase_indexes.end (),
                                phrase,
                                GenericTablePhraseLessThanByPhrase (this));

        indexes = GenericKeyIndexVector (lb, ub);
    }

    return indexes.size () != 0;
}

WideString
GenericTablePhraseLib::get_key_prompt (const String &key) const
{
    WideString prompt;

    for (uint32 i=0; i<key.length (); ++i)
        prompt += get_char_prompt (key [i]);

    return prompt;
}

WideString
GenericTablePhraseLib::get_char_prompt (char ch) const
{
    std::vector <String>::const_iterator it = 
        std::lower_bound (m_char_prompts.begin (), m_char_prompts.end (),
                            ch, _StringLessThanByFirstChar ());

    if (it != m_char_prompts.end () && (*it) [0] == ch)
        return utf8_mbstowcs (it->substr (2, it->length () - 2));

    String str;
    str.push_back (ch);

    return utf8_mbstowcs (str);
}

WideString
GenericTablePhraseLib::get_phrase (uint32 index) const
{
    if (index < m_content.size () - 1) {
        uint32 len = m_content [index] & 0x1f;
        if (len < SCIM_GT_MAX_PHRASE_LENGTH)
            return WideString (
                        m_content.begin () + index + 1,
                        m_content.begin () + index + len + 1);
    }
    return WideString ();
}

WideString
GenericTablePhraseLib::get_name (const String &locale) const
{
    if (locale.length () == 0)
        return utf8_mbstowcs (m_default_name);

    String lang, param, value;
    String::size_type dot;

    dot = locale.find_first_of ('.');
    if (dot != String::npos)
        lang = locale.substr (0, dot);
    else
        lang = locale;

    for (uint32 i=0; i<m_lang_names.size (); ++i) {
        param = _get_param_portion (m_lang_names [i]);
        value = _get_value_portion (m_lang_names [i]);
        if ((param.length () > lang.length () && param.substr (0, lang.length ()) == lang) ||
            (param.length () < lang.length () && lang.substr (0, param.length ()) == param) ||
            (param == lang))
            return utf8_mbstowcs (value);
    }
    return utf8_mbstowcs (m_default_name);
}

void
GenericTablePhraseLib::set_phrase_frequency (uint32 index, uint32 freq)
{
    if (is_valid_phrase (index) && get_phrase_frequency (index) != freq) {
        // Update the frequency and set updated flag 0x40000000.
        m_content [index] &= (~0x3fffff00);
        m_content [index] |= (((freq & 0x3fffff) << 8) | 0xC0000000);
    }
}

uint32
GenericTablePhraseLib::get_max_phrase_length () const
{
    GenericKeyIndexVector::const_iterator it;
    uint32 maxlen = 0;
    uint32 len;

    for (it  = m_indexes.get_all_key_indexes ().begin ();
         it != m_indexes.get_all_key_indexes ().end ();
         ++ it) {
        len = get_phrase_length (it->second);
        if (maxlen < len) maxlen = len;
    }
    return maxlen;
}

void
GenericTablePhraseLib::clear ()
{
    std::vector <ucs4_t> ().swap (m_content);
    std::vector <String> ().swap (m_char_prompts);
    std::vector <String> ().swap (m_lang_names);
    m_indexes.clear_all ();
    m_locales = String ();
    m_select_keys = String ();
    m_split_chars = String ();
    m_default_name = String ();

    m_serial_number = 0;

    m_auto_select = false;
    m_auto_wildcard = false;
    m_dynamic_adjust = true;

    GenericKeyIndexVector ().swap (m_sorted_phrase_indexes);
    m_sorted_phrase_indexes_initialized = false;
}

void
GenericTablePhraseLib::compact_memory ()
{
    m_indexes.compact_memory ();
    std::vector <ucs4_t> (m_content).swap (m_content);
    std::vector <String> (m_char_prompts).swap (m_char_prompts);
    std::vector <String> (m_lang_names).swap (m_lang_names);
}

int
GenericTablePhraseLib::compare_phrase (uint32 lhs, uint32 rhs) const
{
    uint32 llen, rlen;

    llen = (m_content [lhs] & 0x1f);
    rlen = (m_content [rhs] & 0x1f);

    if (llen < rlen) return -1;
    else if (llen > rlen) return 1;
    else {
        lhs ++;
        rhs ++;
        int val;
        for (int i=0; i<llen; ++i) {
            val = m_content [lhs + i] - m_content [rhs + i];
            if (val != 0) return val;
        }
    }
    return 0;
}

int
GenericTablePhraseLib::compare_phrase (uint32 lhs, const WideString & rhs) const
{
    uint32 llen, rlen;

    llen = (m_content [lhs] & 0x1f);
    rlen = rhs.length ();

    if (llen < rlen) return -1;
    else if (llen > rlen) return 1;
    else {
        lhs ++;
        int val;
        for (int i=0; i<llen; ++i) {
            val = m_content [lhs + i] - rhs [i];
            if (val != 0) return val;
        }
    }
    return 0;
}

int
GenericTablePhraseLib::compare_phrase (const WideString & lhs, uint32 rhs) const
{
    uint32 llen, rlen;

    llen = lhs.length ();
    rlen = (m_content [rhs] & 0x1f);

    if (llen < rlen) return -1;
    else if (llen > rlen) return 1;
    else {
        rhs ++;
        int val;
        for (int i=0; i<llen; ++i) {
            val = lhs[i] - m_content [rhs + i];
            if (val != 0) return val;
        }
    }
    return 0;
}

void 
GenericTablePhraseLib::sort_indexes_by_frequency (GenericKeyIndexVector &indexes) const
{
    std::sort (indexes.begin (), indexes.end (),
                GenericTablePhraseLessThanByFrequency (this));
}

void
GenericTablePhraseLib::sort_indexes_by_length (GenericKeyIndexVector &indexes) const
{
    std::sort (indexes.begin (), indexes.end (),
                GenericTablePhraseLessThanByLength (this));
}

void
GenericTablePhraseLib::sort_indexes_by_phrase (GenericKeyIndexVector &indexes) const
{
    std::sort (indexes.begin (), indexes.end (),
                GenericTablePhraseLessThanByPhrase (this));
}

void
GenericTablePhraseLib::sort_indexes_by_index (GenericKeyIndexVector &indexes) const
{
    std::sort (indexes.begin (), indexes.end (),
                GenericTablePhraseLessThanByIndex (this));
}

void
GenericTablePhraseLib::initialize_sorted_phrase_indexes ()
{
    if (!m_sorted_phrase_indexes_initialized) {
        m_sorted_phrase_indexes = m_indexes.get_all_key_indexes ();
        m_sorted_phrase_indexes_initialized = true;
        sort_indexes_by_phrase (m_sorted_phrase_indexes);
    }
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
