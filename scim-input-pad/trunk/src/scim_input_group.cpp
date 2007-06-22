/** @file scim_input_group.cpp
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
 * $Id: scim_input_group.cpp,v 1.6 2005/11/21 06:40:30 suzhe Exp $
 *
 */

#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include "scim_input_group.h"

#ifndef SCIM_INPUT_PAD_DATADIR
#define SCIM_INPUT_PAD_DATADIR         "/usr/share/scim/input-pad"
#endif

#ifndef SCIM_INPUT_PAD_USER_DATADIR
#define SCIM_INPUT_PAD_USER_DATADIR    "/.scim/input-pad"
#endif

// Helper functions
static inline String
trim_blank (const String &str)
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
get_param_portion (const String &str, const String &delim = "=")
{
    String ret = str;
    String::size_type pos = ret.find_first_of (String (" \t\v") + delim);

    if (pos != String::npos)
        ret.erase (pos, String::npos);

    return ret;
}

static inline String
get_value_portion (const String &str, const String &delim = "=")
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
get_line (FILE *fp)
{
    char temp [4096];
    String res;

    while (fp && !feof (fp)) {
        if (!fgets (temp, 4096, fp)) break;

        res = trim_blank (String (temp));

        if (res.length () &&
            !(res.length () >= 3 && res.substr (0, 3) == String ("###")))
            return res;
    }

    return String ();
}

static const char scim_input_pad_header  [] = "SCIM_Input_Pad";
static const char scim_input_pad_version [] = "VERSION_1_0";

static InputTablePointer
load_one_table_from_file (FILE *fp)
{
    if (!fp || feof (fp)) return InputTablePointer (0);

    String str;
    std::vector <String> strvec;
    std::vector <String>::iterator it;

    InputTablePointer tbl = new InputTable ();

    String cur_lang = scim_get_locale_language (scim_get_current_locale ());

    while (!feof (fp)) {
        str = get_line (fp);

        if (str == String ("END_TABLE")) {
            if (tbl->number_of_elements ()) return tbl;
            return InputTablePointer (0);
        }

        if (str.length () > 5 && str.substr (0, 4) == "NAME") {
            String value, param;
            param = get_param_portion (str);
            value = get_value_portion (str);

            if (param == "NAME") {
                if (value.length () && tbl->get_name ().length () == 0) tbl->set_name (value);
            } else if (param.length () > 7 && param [4] == '[' && param [param.length () - 1] == ']') {
                String lang = param.substr (5, param.length () - 6); 
                if (lang.length () && scim_get_normalized_language (lang) == cur_lang && value.length ())
                    tbl->set_name (value);
            }

            continue;
        }

        if (str.length () > 8 && str.substr (0, 7) == "COLUMNS") {
            String value, param;
            param = get_param_portion (str);
            value = get_value_portion (str);

            if (param == "COLUMNS") {
                unsigned int columns = strtol (value.c_str (), 0, 10);
                tbl->set_columns (columns);
            }

            continue;
        }

        if (scim_split_string_list (strvec, str, ' ') > 0) {
            for (it = strvec.begin(); it != strvec.end (); ++it) {
                InputElement elm;

                if (it->length () > 1 && (*it)[0] == '*') {
                    elm.type = INPUT_ELEMENT_KEY;
                    elm.data = it->substr (1, it->length () - 1);
                } else if (it->length () > 2 && (*it)[0] == '0' && ((*it)[1] == 'x' || (*it)[1] == 'X')) {
                    ucs4_t wc = strtol (it->c_str () + 2, 0, 16);
                    if (wc > 0) {
                        char mb [8];
                        mb [utf8_wctomb ((unsigned char*)mb, wc, 8)] = 0;
                        elm.type = INPUT_ELEMENT_STRING;
                        elm.data = String (mb);
                    }
                } else if (it->length () > 0 && (*it)[0] != '*') {
                    elm.type = INPUT_ELEMENT_STRING;
                    elm.data = (*it);
                }

                if (elm.type != INPUT_ELEMENT_NONE || (it->length () == 1 && (*it)[0] == '*'))
                    tbl->append_element (elm);
            }
        }
    }

    return InputTablePointer (0);
}

static void
save_one_table_to_file (FILE *fp, const InputTablePointer &table)
{
    if (!fp || table.null ()) return;

    fprintf (fp, "BEGIN_TABLE\n");
    fprintf (fp, "NAME=%s\n", table->get_name ().c_str ());
    fprintf (fp, "COLUMNS=%u\n\n", table->get_columns ());

    for (size_t i = 0; i < table->number_of_elements (); ++i) {
        const InputElement &elm = table->get_element (i);

        if (elm.type == INPUT_ELEMENT_STRING)
            fprintf (fp, "%s ", elm.data.c_str ());
        else if (elm.type == INPUT_ELEMENT_KEY)
            fprintf (fp, "*%s ", elm.data.c_str ());
        else
            fprintf (fp, "* ");

        if ((i + 1) % table->get_columns () == 0)
            fprintf (fp, "\n");
    }

    fprintf (fp, "\nEND_TABLE\n\n");
}

static InputGroupPointer
load_one_group_from_file (FILE *fp)
{
    if (!fp || feof (fp)) return InputGroupPointer (0);

    String str, value, param;
    String cur_lang = scim_get_locale_language (scim_get_current_locale ());

    InputGroupPointer grp = new InputGroup ();

    while (!feof (fp)) {
        str = get_line (fp);
        if (str == String ("BEGIN_TABLE")) {
            InputTablePointer tbl = load_one_table_from_file (fp);
            if (!tbl.null () && tbl->number_of_elements () > 0)
                grp->append_table (tbl);

            continue;
        }

        if (str == String ("END_GROUP")) {
            if (grp->number_of_tables ()) return grp;
            return InputGroupPointer (0);
        }

        param = get_param_portion (str);
        value = get_value_portion (str);

        if (param == "NAME") {
            if (value.length () && grp->get_name ().length () == 0) grp->set_name (value);
        } else if (param.length () > 7 && param.substr (0,5) == "NAME[" && param [param.length () - 1] == ']') {
            String lang = param.substr (5, param.length () - 6); 
            if (lang.length () && scim_get_normalized_language (lang) == cur_lang && value.length ())
                grp->set_name (value);
        }
    }
    return InputGroupPointer (0);
}

static void
save_one_group_to_file (FILE *fp, const InputGroupPointer &group)
{
    if (!fp || group.null ()) return;

    fprintf (fp, "BEGIN_GROUP\n");
    fprintf (fp, "NAME=%s\n\n", group->get_name ().c_str ());

    for (size_t i = 0; i < group->number_of_tables (); ++i) {
        save_one_table_to_file (fp, group->get_table (i));
    }

    fprintf (fp, "END_GROUP\n\n");
}

int
load_input_group_file (const String &file_name, std::vector <InputGroupPointer> &groups)
{
    FILE *fp = fopen (file_name.c_str (), "rb");

    if (!fp) return 0;

    if (get_line (fp) != String (scim_input_pad_header) ||
        get_line (fp) != String (scim_input_pad_version))
        return 0;

    while (!feof (fp)) {
        if (get_line (fp) == String ("BEGIN_GROUP")) {
            InputGroupPointer grp = load_one_group_from_file (fp);
            if (!grp.null () && grp->number_of_tables () > 0) {
                groups.push_back (grp);
            }
        }
    }
    return groups.size ();
}

bool
save_input_group_file (const String &file_name, const std::vector <InputGroupPointer> &groups)
{
    if (groups.size () == 0) return false;

    FILE *fp = fopen (file_name.c_str (), "wb");

    if (!fp) return false;

    fprintf (fp, "%s\n", scim_input_pad_header);
    fprintf (fp, "%s\n\n", scim_input_pad_version);

    for (size_t i = 0; i < groups.size (); ++i) {
        save_one_group_to_file (fp, groups [i]);
    }

    fclose (fp);

    return true;
}

static void
get_input_group_file_list (std::vector<String> &file_list, const String &path)
{
    file_list.clear ();

    DIR *dir = opendir (path.c_str ());
    if (dir != NULL) {
        struct dirent *file = readdir (dir);
        while (file != NULL) {
            struct stat filestat;
            String absfn = path + SCIM_PATH_DELIM_STRING + file->d_name;
            stat (absfn.c_str (), &filestat);

            if (S_ISREG (filestat.st_mode))
                file_list.push_back (absfn);

            file = readdir (dir);
        }
        closedir (dir);        
    }
}

int
load_all_input_group_files (std::vector <InputGroupPointer> &groups)
{
    std::vector<String> file_list;

    groups.clear ();

    get_input_group_file_list (file_list, SCIM_INPUT_PAD_DATADIR);

    for (size_t i = 0; i < file_list.size (); ++i) {
        load_input_group_file (file_list [i], groups);
    }

    get_input_group_file_list (file_list, scim_get_home_dir () + SCIM_INPUT_PAD_USER_DATADIR);

    for (size_t i = 0; i < file_list.size (); ++i)
        load_input_group_file (file_list [i], groups);

    return groups.size ();
}

/*
vi:ts=4:nowrap:ai:expandtab
*/

