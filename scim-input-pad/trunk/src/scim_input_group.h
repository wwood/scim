/** @file scim_input_group.h
 */

/* 
 * Smart Common Input Method
 * 
 * Copyright (c) 2005 James Su <suzhe@tsinghua.org.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: scim_input_group.h,v 1.3 2005/06/25 03:47:23 suzhe Exp $
 */

#if !defined (__SCIM_INPUT_GROUP_H)
#define __SCIM_INPUT_GROUP_H

#define Uses_SCIM_UTILITY
#define Uses_SCIM_OBJECT
#define Uses_SCIM_POINTER
#define Uses_SCIM_EVENT
#define Uses_SCIM_HELPER
#define Uses_SCIM_CONFIG_BASE
#include <vector>
#include <scim.h>

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#if defined(HAVE_LIBINTL_H) && defined(ENABLE_NLS)
  #include <libintl.h>
  #define _(String) dgettext(GETTEXT_PACKAGE,String)
  #define N_(String) (String)
#else
  #define _(String) (String)
  #define N_(String) (String)
  #define bindtextdomain(Package,Directory)
  #define textdomain(domain)
  #define bind_textdomain_codeset(domain,codeset)
#endif

using namespace scim;

enum InputElementType
{
    INPUT_ELEMENT_NONE,
    INPUT_ELEMENT_STRING,
    INPUT_ELEMENT_KEY
};

struct InputElement
{
    InputElementType type;
    String           data;

    InputElement (InputElementType t = INPUT_ELEMENT_NONE, const String &d = "")
        : type (t), data (d) { }

    bool operator < (const InputElement & rhs) const {
        if (type < rhs.type || (type == rhs.type && data < rhs.data)) return true;
        return false;
    }

    bool operator == (const InputElement & rhs) const {
        return type == rhs.type && data == rhs.data;
    }
};

class InputTable : public ReferencedObject
{
    std::vector <InputElement> m_elements;
    String                     m_name;
    unsigned int               m_columns;

public:
    InputTable (const String &name = "", unsigned int columns= 5)
        : m_name (name), m_columns ((columns> 0)?columns:5) { }

    void set_name (const String &name) {
        m_name = name;
    }

    void set_columns (unsigned int columns) {
        if (columns > 0 && columns < 20) m_columns = columns;
    }

    void append_element (const InputElement &element) {
        m_elements.push_back (element);
    }

    void prepend_element (const InputElement &element) {
        m_elements.insert (m_elements.begin (), element);
    }

    void drop_oldest () {
        m_elements.pop_back ();
    }

    bool find_element (const InputElement &element) const {
        return std::find (m_elements.begin (), m_elements.end (), element) != m_elements.end ();
    }

    void delete_all_elements () {
        m_elements.clear ();
    }

    void clear () {
        m_elements.clear ();
        m_name = "";
    }

    const String& get_name () const {
        return m_name;
    }

    unsigned int get_columns () const {
        return m_columns;
    }

    size_t number_of_elements () const {
        return m_elements.size ();
    }

    const InputElement& get_element (size_t i) const {
        static InputElement none;
        if (i < m_elements.size ())
            return m_elements [i];
        return none;
    }
};

typedef Pointer <InputTable> InputTablePointer;

class InputGroup : public ReferencedObject
{
    std::vector <InputTablePointer> m_tables;
    String                          m_name;

public:
    InputGroup (const String &name = "") : m_name (name) { }

    void set_name (const String &name) {
        m_name = name;
    }

    void append_table (const InputTablePointer &table) {
        if (!table.null ()) m_tables.push_back (table);
    }

    void prepend_table (const InputTablePointer &table) {
        if (!table.null ()) m_tables.insert (m_tables.begin (), table);
    }

    void clear () {
        m_tables.clear ();
        m_name = "";
    }

    const String& get_name () const {
        return m_name;
    }

    size_t number_of_tables () const {
        return m_tables.size ();
    }

    InputTablePointer get_table (size_t i) const {
        if (i < m_tables.size ()) return m_tables [i];
        return InputTablePointer (0);
    }
};

typedef Pointer <InputGroup> InputGroupPointer;

int load_input_group_file (const String &file_name, std::vector <InputGroupPointer> &groups);

bool save_input_group_file (const String &file_name, const std::vector <InputGroupPointer> &groups);

int load_all_input_group_files (std::vector <InputGroupPointer> &groups);

#endif //__SCIM_INPUT_GROUP_H

/*
vi:ts=4:nowrap:ai:expandtab
*/

