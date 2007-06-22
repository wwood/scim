/** @file scim_m17n_imengine.cpp
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
 * $Id: scim_m17n_imengine.cpp,v 1.26 2007/01/11 00:44:33 suzhe Exp $
 */

#define Uses_SCIM_UTILITY
#define Uses_SCIM_IMENGINE
#define Uses_SCIM_LOOKUP_TABLE
#define Uses_SCIM_CONFIG_BASE
#define Uses_STL_MAP

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <scim.h>
#include "scim_m17n_imengine.h"

#define scim_module_init m17n_LTX_scim_module_init
#define scim_module_exit m17n_LTX_scim_module_exit
#define scim_imengine_module_init m17n_LTX_scim_imengine_module_init
#define scim_imengine_module_create_factory m17n_LTX_scim_imengine_module_create_factory

#define SCIM_PROP_STATUS                  "/IMEngine/M17N/Status"

#ifndef SCIM_M17N_ICON_FILE
    #define SCIM_M17N_ICON_FILE           (SCIM_ICONDIR "/scim-m17n.png")
#endif

struct M17NInfo {
    String        lang;
    String        name;
    String        uuid;
};

static std::vector <M17NInfo>                     __m17n_input_methods;
static std::map <MInputContext *, M17NInstance *> __m17n_input_contexts;

static CommonLookupTable                          __lookup_table;

static MConverter   *__m17n_converter = 0;

static M17NInstance * __find_instance (MInputContext *ic);

static MSymbol __key_to_symbol (const KeyEvent &key);

extern "C" {
    void scim_module_init (void)
    {
        __lookup_table.fix_page_size ();
    }

    void scim_module_exit (void)
    {
        M17N_FINI();
    }

    uint32 scim_imengine_module_init (const ConfigPointer &config)
    {
        SCIM_DEBUG_IMENGINE(1) << "Initialize M17N Engine.\n";

        MPlist *imlist, *elm;
        MSymbol utf8;

        size_t i;
        size_t count = 0;

        M17N_INIT();
        utf8 = msymbol("utf8");

        __m17n_converter = mconv_buffer_converter(utf8, NULL, 0);

        if (!__m17n_converter) return 0;

        imlist = mdatabase_list(msymbol("input-method"), Mnil, Mnil, Mnil);

        for (elm = imlist; elm && mplist_key(elm) != Mnil; elm = mplist_next(elm)) {
            MDatabase *mdb = (MDatabase *) mplist_value(elm);
            MSymbol *tag = mdatabase_tag(mdb);
            if (tag[1] != Mnil && tag[2] != Mnil) {
                const char *im_lang = msymbol_name (tag[1]);
                const char *im_name = msymbol_name (tag[2]);

                if (im_lang && strlen (im_lang) && im_name && strlen (im_name)) {
                    M17NInfo info;

                    SCIM_DEBUG_IMENGINE(1) << im_lang << "-" << im_name << "\n";

                    info.lang = String (im_lang);
                    info.name = String (im_name);

                    __m17n_input_methods.push_back (info);

                    count++;
                }
            }
        }

        if (imlist) m17n_object_unref(imlist);

        // Set uuids.
        for (i = 0; i < count; ++i)
            __m17n_input_methods [i].uuid = String ("IMEngine-M17N-" +
                                                    __m17n_input_methods [i].lang +
                                                    String ("-") +
                                                    __m17n_input_methods [i].name);

        return count;
    }

    IMEngineFactoryPointer scim_imengine_module_create_factory (uint32 engine)
    {
        if (engine >= __m17n_input_methods.size ()) return 0;

        M17NFactory *factory = 0;

        try {
            factory = new M17NFactory (__m17n_input_methods [engine].lang,
                                       __m17n_input_methods [engine].name,
                                       __m17n_input_methods [engine].uuid);
        } catch (...) {
            delete factory;
            factory = 0;
        }

        return factory;
    }
}

M17NFactory::M17NFactory (const String &lang,
                          const String &name,
                          const String &uuid)
    : m_im (0), m_lang (lang), m_name (name), m_uuid (uuid)
{
    SCIM_DEBUG_IMENGINE(1) << "Create M17N Factory :\n";
    SCIM_DEBUG_IMENGINE(1) << "  Lang : " << lang << "\n";
    SCIM_DEBUG_IMENGINE(1) << "  Name : " << name << "\n";
    SCIM_DEBUG_IMENGINE(1) << "  UUID : " << uuid << "\n";

    if (lang.length () >= 2)
        set_languages (lang);
}

M17NFactory::~M17NFactory ()
{
    if (m_im)
        minput_close_im (m_im);
}

bool
M17NFactory::load_input_method ()
{
    SCIM_DEBUG_IMENGINE(1) << "load_input_method(" << m_lang << "," << m_name << ")\n";

    if (m_im) return true;

    m_im = minput_open_im(msymbol (m_lang.c_str ()), msymbol (m_name.c_str ()), NULL);

    if (m_im) {
        m_im->driver.callback_list = M17NInstance::register_callbacks(m_im->driver.callback_list);
        return true;
    }

    return false;
}

WideString
M17NFactory::get_name () const
{
    return utf8_mbstowcs (m_lang + String ("-") + m_name);
}

WideString
M17NFactory::get_authors () const
{
    return WideString ();
}

WideString
M17NFactory::get_credits () const
{
    return WideString ();
}

WideString
M17NFactory::get_help () const
{
#if M17N_VERSION >= 10300
    MText *desc = minput_get_description (msymbol (m_lang.c_str ()), msymbol (m_name.c_str ()));
    if (desc) {
        int bufsize = mtext_len (desc) * 6;  // long enough
        char *buf = new char [ bufsize ];
        mconv_rebind_buffer(__m17n_converter, (unsigned char *) buf, bufsize);
        mconv_encode(__m17n_converter, desc);
        buf[__m17n_converter->nbytes] = 0;
        m17n_object_unref(desc);

        return utf8_mbstowcs (buf);
    }
#endif
    return WideString ();
}

String
M17NFactory::get_uuid () const
{
    return m_uuid;
}

String
M17NFactory::get_icon_file () const
{
#if M17N_VERSION >= 10300
    MPlist *l = minput_get_title_icon (msymbol (m_lang.c_str ()), msymbol (m_name.c_str ()));
    if (l) {
        char buf [256] = SCIM_M17N_ICON_FILE;
        MPlist *n = mplist_next (l);

        if (n && mplist_key (n) == Mtext) {
            MText *icon = (MText*) mplist_value (n);
            mconv_rebind_buffer(__m17n_converter, (unsigned char *) buf, 256);
            mconv_encode(__m17n_converter, icon);
            buf[__m17n_converter->nbytes] = 0;
        }

        m17n_object_unref (l);
        return String (buf);
    }
#endif
    return String (SCIM_M17N_ICON_FILE);
}

IMEngineInstancePointer
M17NFactory::create_instance (const String &encoding, int id)
{
    if (m_im || load_input_method ())
        return new M17NInstance (this, encoding, id);

    return new DummyIMEngineInstance(dynamic_cast<DummyIMEngineFactory*>(this), encoding, id);
}

M17NInstance::M17NInstance (M17NFactory  *factory,
                            const String &encoding,
                            int           id)
    : IMEngineInstanceBase (factory, encoding, id),
      m_ic (0),
      m_cap (0),
      m_block_preedit_op (false),
      m_pending_preedit_start (false),
      m_pending_preedit_draw (false),
      m_pending_preedit_done (false),
      m_preedit_showed (false)
{
    SCIM_DEBUG_IMENGINE(1) << "Create M17N Instance " << encoding << " " << id << "\n";

    if (factory->m_im) {
        SCIM_DEBUG_IMENGINE(2) << " Create minput instance.\n";
        m_ic = minput_create_ic (factory->m_im, NULL);
    }

    if (m_ic)
        __m17n_input_contexts [m_ic] = this;
}

M17NInstance::~M17NInstance ()
{
    SCIM_DEBUG_IMENGINE(1) << "Destroy M17N Instance " << get_id () << "\n";

    if (m_ic) {
        __m17n_input_contexts.erase (m_ic);
        minput_destroy_ic (m_ic);
    }
}

bool
M17NInstance::process_key_event (const KeyEvent& key)
{
    if (!m_ic) return false;

    if (key.is_key_release ()) return true;

    MSymbol m17n_key = __key_to_symbol (key.map_to_layout (SCIM_KEYBOARD_Default));

    if (m17n_key == Mnil) return false;

    return m17n_process_key (m17n_key);
}

bool
M17NInstance::m17n_process_key (MSymbol key)
{
    SCIM_DEBUG_IMENGINE(2) << "process_key_event. " << msymbol_name (key) << "\n";

    char buf [1024];
    MText *produced;
    int ret;

    m_block_preedit_op = true;

    ret = minput_filter(m_ic, key, NULL);

    m_block_preedit_op = false;

    if (ret) {
        SCIM_DEBUG_IMENGINE(3) << "minput_filter returns 1\n";
        do_preedit_op ();
        return true;
    }

    produced = mtext();

    ret = minput_lookup(m_ic, key, NULL, produced);

    if (ret) {
        SCIM_DEBUG_IMENGINE(3) << "minput_lookup returns 1\n";
    }

    mconv_rebind_buffer(__m17n_converter, (unsigned char *) buf, 1024);
    mconv_encode(__m17n_converter, produced);
    buf[__m17n_converter->nbytes] = 0;
    m17n_object_unref(produced);

    if (strlen (buf)) {
        SCIM_DEBUG_IMENGINE(2) << "commit_string: " << buf << "\n";
        commit_string (utf8_mbstowcs (buf));
    }

    do_preedit_op ();

    return ret == 0;
}

void
M17NInstance::do_preedit_op ()
{
    if (m_block_preedit_op)
        return;

    if (m_pending_preedit_start) {
        preedit_start_cb (m_ic, Minput_preedit_start);
        m_pending_preedit_start = false;
    }

    if (m_pending_preedit_draw) {
        preedit_draw_cb (m_ic, Minput_preedit_draw);
        m_pending_preedit_draw = false;
    }

    if (m_pending_preedit_done) {
        preedit_done_cb (m_ic, Minput_preedit_done);
        m_pending_preedit_done = false;
    }
}

void
M17NInstance::move_preedit_caret (unsigned int pos)
{
}

void
M17NInstance::select_candidate (unsigned int item)
{
    if (item <= 10) {
        char buf [4];
        snprintf (buf, 4, "%d", (item + 1) % 10);
        m17n_process_key (msymbol (buf));
    }
}

void
M17NInstance::update_lookup_table_page_size (unsigned int page_size)
{
}

void
M17NInstance::lookup_table_page_up ()
{
    m17n_process_key (msymbol ("Up"));
}

void
M17NInstance::lookup_table_page_down ()
{
    m17n_process_key (msymbol ("Down"));
}

void
M17NInstance::reset ()
{
    SCIM_DEBUG_IMENGINE(2) << "reset.\n";
    minput_reset_ic(m_ic);

    hide_preedit_string ();
    hide_aux_string ();
    hide_lookup_table ();

    m_preedit_showed = false;
    m_pending_preedit_start = false;
    m_pending_preedit_draw = false;
    m_pending_preedit_done = false;
}

void
M17NInstance::focus_in ()
{
    SCIM_DEBUG_IMENGINE(2) << "focus_in.\n";

    PropertyList props;
    Property     prop (String (SCIM_PROP_STATUS),
                       String (""));

    prop.hide ();
    props.push_back (prop);

    register_properties (props);

#if M17N_VERSION >= 10300
    m17n_process_key (Minput_focus_in);
#else
    preedit_draw_cb (m_ic, Minput_preedit_draw);
    candidates_draw_cb (m_ic, Minput_candidates_draw);
#endif

    //FIXME: Minput_focus_in can't trigger status_draw_cb, so call it here.
    status_draw_cb (m_ic, Minput_status_draw);
}

void
M17NInstance::focus_out ()
{
    SCIM_DEBUG_IMENGINE(2) << "focus_out.\n";

#if M17N_VERSION >= 10300
    m17n_process_key (Minput_focus_out);
#else
    reset ();
#endif
}

void
M17NInstance::trigger_property (const String &property)
{
}

void
M17NInstance::update_client_capabilities (unsigned int cap)
{
    m_cap = cap;
}

static M17NInstance *
__find_instance (MInputContext *ic)
{
    std::map <MInputContext *, M17NInstance *>::iterator it =
        __m17n_input_contexts.find (ic);

    if (it != __m17n_input_contexts.end ())
        return it->second;

    return 0;
}


MPlist *
M17NInstance::register_callbacks(MPlist *callback_list)
{
    if(!callback_list)
        callback_list = mplist();
 
    mplist_put(callback_list, Minput_preedit_start, (void*)preedit_start_cb);
    mplist_put(callback_list, Minput_preedit_draw,  (void*)preedit_draw_cb);
    mplist_put(callback_list, Minput_preedit_done,  (void*)preedit_done_cb);
    mplist_put(callback_list, Minput_status_start,  (void*)status_start_cb);
    mplist_put(callback_list, Minput_status_draw,   (void*)status_draw_cb);
    mplist_put(callback_list, Minput_status_done,   (void*)status_done_cb);
    mplist_put(callback_list, Minput_candidates_start, (void*)candidates_start_cb);
    mplist_put(callback_list, Minput_candidates_draw,  (void*)candidates_draw_cb);
    mplist_put(callback_list, Minput_candidates_done,  (void*)candidates_done_cb);

#if M17N_VERSION >= 10300
    mplist_put(callback_list, Minput_get_surrounding_text,  (void*)get_surrounding_text_cb);
    mplist_put(callback_list, Minput_delete_surrounding_text,  (void*)delete_surrounding_text_cb);
#endif
 
    return callback_list;
}

void
M17NInstance::preedit_start_cb (MInputContext *ic, MSymbol command)
{
    M17NInstance *this_ptr = __find_instance (ic);

    if (this_ptr && !this_ptr->m_preedit_showed) {
        SCIM_DEBUG_IMENGINE(2) << "preedit_start_cb.\n";

        if (this_ptr->m_block_preedit_op) {
            this_ptr->m_pending_preedit_start = true;
            return;
        }

        this_ptr->show_preedit_string ();
        this_ptr->m_preedit_showed = true;
    }
}

void
M17NInstance::preedit_draw_cb  (MInputContext *ic, MSymbol command)
{
    M17NInstance *this_ptr = __find_instance (ic);

    if (this_ptr && ic->preedit) {
        SCIM_DEBUG_IMENGINE(2) << "preedit_draw_cb.\n";

        if (this_ptr->m_block_preedit_op) {
            this_ptr->m_pending_preedit_draw = true;
            return;
        }

        char buf[1024];
        mconv_rebind_buffer(__m17n_converter, (unsigned char *)buf, 1024);
        mconv_encode(__m17n_converter, ic->preedit);
        buf[__m17n_converter->nbytes] = 0;

        WideString wstr = utf8_mbstowcs (buf);
        if (wstr.length ()) {
            AttributeList attrs;

            if (ic->candidate_from < ic->candidate_to && ic->candidate_to <= wstr.length ())
                attrs.push_back (Attribute (ic->candidate_from,
                                            ic->candidate_to - ic->candidate_from,
                                            SCIM_ATTR_DECORATE,
                                            SCIM_ATTR_DECORATE_REVERSE));

            if (!this_ptr->m_preedit_showed) {
                this_ptr->show_preedit_string ();
                this_ptr->m_preedit_showed = true;
            }

            this_ptr->update_preedit_string (wstr, attrs);
            this_ptr->update_preedit_caret (ic->cursor_pos);
        } else {
            this_ptr->hide_preedit_string ();
            this_ptr->m_preedit_showed = false;
        }
    }
}

void
M17NInstance::preedit_done_cb  (MInputContext *ic, MSymbol command)
{
    M17NInstance *this_ptr = __find_instance (ic);

    if (this_ptr && this_ptr->m_preedit_showed) {
        SCIM_DEBUG_IMENGINE(2) << "preedit_done_cb.\n";

        if (this_ptr->m_block_preedit_op) {
            this_ptr->m_pending_preedit_done = true;
            return;
        }

        this_ptr->hide_preedit_string ();
        this_ptr->m_preedit_showed = false;
    }
}

void
M17NInstance::status_start_cb (MInputContext *ic, MSymbol command)
{
    M17NInstance *this_ptr = __find_instance (ic);

    if (this_ptr) {
        SCIM_DEBUG_IMENGINE(2) << "status_start_cb.\n";

        Property prop (String (SCIM_PROP_STATUS), String (""));

        this_ptr->update_property (prop);
    }
}

void
M17NInstance::status_draw_cb (MInputContext *ic, MSymbol command)
{
    M17NInstance *this_ptr = __find_instance (ic);

    if (this_ptr && ic->status) {
        SCIM_DEBUG_IMENGINE(2) << "status_draw_cb.\n";

        char buf[1024];
        mconv_rebind_buffer(__m17n_converter, (unsigned char *)buf, 1024);
        mconv_encode(__m17n_converter, ic->status);
        buf[__m17n_converter->nbytes] = 0;

        Property prop (String (SCIM_PROP_STATUS), String (buf));

        this_ptr->update_property (prop);
    }
}

void
M17NInstance::status_done_cb (MInputContext *ic, MSymbol command)
{
    M17NInstance *this_ptr = __find_instance (ic);

    if (this_ptr) {
        SCIM_DEBUG_IMENGINE(2) << "status_done_cb.\n";

        Property prop (String (SCIM_PROP_STATUS), String (""));
        prop.hide ();

        this_ptr->update_property (prop);
    }
}

void
M17NInstance::candidates_start_cb (MInputContext *ic, MSymbol command)
{
    M17NInstance *this_ptr = __find_instance (ic);

    if (this_ptr) {
        SCIM_DEBUG_IMENGINE(2) << "candidates_start_cb.\n";
        this_ptr->show_lookup_table ();
    }
}

void
M17NInstance::candidates_draw_cb (MInputContext *ic, MSymbol command)
{
    M17NInstance *this_ptr = __find_instance (ic);

    if (this_ptr) {
        __lookup_table.clear ();

        SCIM_DEBUG_IMENGINE(2) << "candidates_draw_cb.\n";
        SCIM_DEBUG_IMENGINE(3) << "candidate_index = " << ic->candidate_index << "\n";
        SCIM_DEBUG_IMENGINE(3) << "candidate_from  = " << ic->candidate_from << "\n";
        SCIM_DEBUG_IMENGINE(3) << "candidate_to    = " << ic->candidate_to << "\n";
        SCIM_DEBUG_IMENGINE(3) << "candidate_show  = " << ic->candidate_show << "\n";

        if (ic->candidate_list && ic->candidate_show) {
            MPlist *group;
            MText  *mt;
            char buf [1024];
            int i, len, cur, page;
            bool page_up, page_down;
            WideString wstr;

            i = 0;
            page = 0;
            group = ic->candidate_list;
            while (1) {
                if (mplist_key (group) == Mtext)
                    len = mtext_len ((MText *) mplist_value (group));
                else
                    len = mplist_length ((MPlist*) mplist_value (group));

                if (i + len > ic->candidate_index)
                    break;
                i += len;
                group = mplist_next (group);

                ++page;
            }

            cur = ic->candidate_index - i;

            page_up = (page > 0);
            page_down = ((page + 1) < mplist_length (ic->candidate_list));

            if (page_up)
                __lookup_table.append_candidate (0x3000);

            if (mplist_key (group) == Mtext) {
                mt = (MText *) mplist_value (group);
                mconv_rebind_buffer(__m17n_converter, (unsigned char *)buf, 1024);
                mconv_encode(__m17n_converter, mt);
                buf[__m17n_converter->nbytes] = 0;
                wstr = utf8_mbstowcs (buf);

                for (i = 0; i < wstr.length (); ++i)
                    __lookup_table.append_candidate (wstr [i]);

                if (page_up) {
                    __lookup_table.set_page_size (1);
                    __lookup_table.page_down ();
                }

                __lookup_table.set_page_size (wstr.length ());
            } else {
                MPlist *pl;

                len = 0;
                for (pl = (MPlist *) mplist_value (group); mplist_key (pl) != Mnil; pl = mplist_next (pl)) {
                    mt = (MText *) mplist_value (pl);
                    mconv_rebind_buffer(__m17n_converter, (unsigned char *)buf, 1024);
                    mconv_encode(__m17n_converter, mt);
                    buf[__m17n_converter->nbytes] = 0;
                    wstr = utf8_mbstowcs (buf);

                    __lookup_table.append_candidate (wstr);
                    ++ len;
                }

                if (page_up) {
                    __lookup_table.set_page_size (1);
                    __lookup_table.page_down ();
                }

                __lookup_table.set_page_size (len);
            }

            if (page_down)
                __lookup_table.append_candidate (0x3000);

            __lookup_table.set_cursor_pos_in_current_page (cur);
            __lookup_table.show_cursor ();

            this_ptr->update_lookup_table (__lookup_table);
            this_ptr->show_lookup_table ();
        } else {
            this_ptr->hide_lookup_table ();
        }
    }
}

void
M17NInstance::candidates_done_cb (MInputContext *ic, MSymbol command)
{
    M17NInstance *this_ptr = __find_instance (ic);

    if (this_ptr) {
        SCIM_DEBUG_IMENGINE(2) << "candidates_done_cb.\n";
        this_ptr->hide_lookup_table ();
    }
}

#if M17N_VERSION >= 10300
void
M17NInstance::get_surrounding_text_cb (MInputContext *ic, MSymbol command)
{
    M17NInstance *this_ptr = __find_instance (ic);

    if (this_ptr && (this_ptr->m_cap & SCIM_CLIENT_CAP_SURROUNDING_TEXT)) {
        SCIM_DEBUG_IMENGINE(2) << "get_surrounding_text_cb.\n";
        if (ic->plist && mplist_key (ic->plist) == Minteger) {
            int len = (int) ((size_t) mplist_value (ic->plist));
            int cursor;
            WideString wstr;
            MText *txt = mtext ();
            if (this_ptr->get_surrounding_text (wstr, cursor, (len < 0) ? (-len) : 0, (len > 0) ? len : 0)) {
                for (WideString::const_iterator i = wstr.begin (); i != wstr.end (); ++i)
                    mtext_cat_char (txt, (int) *i);
            }
            mplist_set (ic->plist, Mtext, txt);
            m17n_object_unref (txt);
        }
    }
}

void
M17NInstance::delete_surrounding_text_cb (MInputContext *ic, MSymbol command)
{
    M17NInstance *this_ptr = __find_instance (ic);

    if (this_ptr) {
        SCIM_DEBUG_IMENGINE(2) << "delete_surrounding_text_cb.\n";
        if (ic->plist && mplist_key (ic->plist) == Minteger) {
            int len = (int) ((size_t) mplist_value (ic->plist));
            // FIXME: There is no way to inform m17n whether it's failed or not.
            this_ptr->delete_surrounding_text ((len < 0) ? len : 0, (len > 0) ? len : (-len));
        }
    }
}
#endif

static MSymbol
__key_to_symbol (const KeyEvent &key)
{
    int mask = 0;
    String keysym;

    if (key.code >= SCIM_KEY_space && key.code <= SCIM_KEY_asciitilde) {
        int c = key.code;

        if ((c == SCIM_KEY_space) &&
            key.is_shift_down ())
            mask |= SCIM_KEY_ShiftMask;

        if (key.is_control_down ()) {
            if (c >= 'a' && c <= 'z')
                c += 'A' - 'a';
            mask |= SCIM_KEY_ControlMask;
        }

        keysym.push_back (c);
    } else if (key.code >= SCIM_KEY_Shift_L && key.code <= SCIM_KEY_Hyper_R) {
        return Mnil;
    } else {
        if (!scim_key_to_string (keysym, KeyEvent (key.code, 0)))
            return Mnil;
        if (key.is_control_down ())
            mask |= SCIM_KEY_ControlMask;
        if (key.is_shift_down ())
            mask |= SCIM_KEY_ShiftMask;
    }

    if (key.is_meta_down ())
        mask |= SCIM_KEY_MetaMask;

    if (key.is_alt_down ())
        mask |= SCIM_KEY_AltMask;

    if (!keysym.length ()) return Mnil;

    if (mask & SCIM_KEY_ShiftMask)
        keysym = String ("S-") + keysym;
    if (mask & SCIM_KEY_ControlMask)
        keysym = String ("C-") + keysym;
    if (mask & SCIM_KEY_MetaMask)
        keysym = String ("M-") + keysym;
    if (mask & SCIM_KEY_AltMask)
        keysym = String ("A-") + keysym;
    if (mask & SCIM_KEY_SuperMask)
        keysym = String ("s-") + keysym;
    if (mask & SCIM_KEY_HyperMask)
        keysym = String ("H-") + keysym;

    return msymbol (keysym.c_str ());
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
