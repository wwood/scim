/** @file scim_uim_imengine.h
 */

/* 
 * Smart Common Input Method
 * 
 * Copyright (c) 2004 James Su <suzhe@tsinghua.org.cn>
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
 * $Id: scim_uim_imengine.h,v 1.7 2007/04/10 04:36:37 makeinu Exp $
 */

#if !defined (__SCIM_UIM_IMENGINE_H)
#define __SCIM_UIM_IMENGINE_H

#include <uim.h>

using namespace scim;

class UIMFactory : public IMEngineFactoryBase
{
    String m_name;
    String m_uuid;

    friend class UIMInstance;

public:
    UIMFactory (const String &name,
                const String &lang,
                const String &uuid);

    virtual ~UIMFactory ();

    virtual WideString  get_name () const;
    virtual WideString  get_authors () const;
    virtual WideString  get_credits () const;
    virtual WideString  get_help () const;
    virtual String      get_uuid () const;
    virtual String      get_icon_file () const;

    virtual IMEngineInstancePointer create_instance (const String& encoding, int id = -1);
};

class UIMInstance : public IMEngineInstanceBase
{
    uim_context       m_uc;

    WideString        m_preedit_string;
    AttributeList     m_preedit_attrs;
    int               m_preedit_caret;

    CommonLookupTable m_lookup_table;

    bool              m_show_lookup_table;

    PropertyList      m_properties;

public:
    UIMInstance (UIMFactory   *factory,
                 const String &uim_name,
                 const String &encoding,
                 int           id = -1);

    virtual ~UIMInstance ();

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
    static int convert_keycode (int keycode);
    static int convert_keymask (int mask);

    static void uim_commit_cb            (void *ptr, const char *str);

    static void uim_preedit_clear_cb     (void *ptr);
    static void uim_preedit_pushback_cb  (void *ptr, int attr, const char *str);
    static void uim_preedit_update_cb    (void *ptr);

    static void uim_prop_list_update_cb  (void *ptr, const char *str);
    static void uim_prop_label_update_cb (void *ptr, const char *str);

    static void uim_cand_activate_cb     (void *ptr, int nr, int display_limit);
    static void uim_cand_select_cb       (void *ptr, int index);
    static void uim_cand_shift_page_cb   (void *ptr, int dir);
    static void uim_cand_deactive_cb     (void *ptr);
};
#endif
/*
vi:ts=4:nowrap:ai:expandtab
*/
