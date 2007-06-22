/** @file scim_m17n_imengine.h
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
 * $Id: scim_m17n_imengine.h,v 1.6 2006/12/01 08:45:12 suzhe Exp $
 */

#if !defined (__SCIM_M17N_IMENGINE_H)
#define __SCIM_M17N_IMENGINE_H

#include <m17n.h>

#define M17N_VERSION (M17NLIB_MAJOR_VERSION * 10000 \
                     + M17NLIB_MINOR_VERSION * 100 \
                     + M17NLIB_PATCH_LEVEL)

using namespace scim;

class M17NFactory : public IMEngineFactoryBase
{
    MInputMethod *m_im;

    String        m_lang;
    String        m_name;
    String        m_uuid;

    friend class M17NInstance;

public:
    M17NFactory (const String &lang,
                 const String &name,
                 const String &uuid);

    virtual ~M17NFactory ();

    virtual WideString  get_name () const;
    virtual WideString  get_authors () const;
    virtual WideString  get_credits () const;
    virtual WideString  get_help () const;
    virtual String      get_uuid () const;
    virtual String      get_icon_file () const;

    virtual IMEngineInstancePointer create_instance (const String& encoding, int id = -1);

private:
    bool load_input_method ();
};

class M17NInstance : public IMEngineInstanceBase
{
    MInputContext    *m_ic;
    unsigned int      m_cap;

    bool              m_block_preedit_op;
    bool              m_pending_preedit_start;
    bool              m_pending_preedit_draw;
    bool              m_pending_preedit_done;

    bool              m_preedit_showed;

public:
    M17NInstance (M17NFactory *factory,
                 const String &encoding,
                 int           id = -1);

    virtual ~M17NInstance ();

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
    virtual void update_client_capabilities (unsigned int cap);

private:
    bool m17n_process_key (MSymbol key);

    void do_preedit_op ();

public:
    static MPlist * register_callbacks(MPlist *callback_list);

private:
    static void preedit_start_cb (MInputContext *ic, MSymbol command);
    static void preedit_draw_cb  (MInputContext *ic, MSymbol command);
    static void preedit_done_cb  (MInputContext *ic, MSymbol command);

    static void status_start_cb  (MInputContext *ic, MSymbol command);
    static void status_draw_cb   (MInputContext *ic, MSymbol command);
    static void status_done_cb   (MInputContext *ic, MSymbol command);

    static void candidates_start_cb (MInputContext *ic, MSymbol command);
    static void candidates_draw_cb  (MInputContext *ic, MSymbol command);
    static void candidates_done_cb  (MInputContext *ic, MSymbol command);

#if M17N_VERSION >= 10300
    static void get_surrounding_text_cb (MInputContext *ic, MSymbol command);
    static void delete_surrounding_text_cb (MInputContext *ic, MSymbol command);
#endif
};
#endif
/*
vi:ts=4:nowrap:ai:expandtab
*/
