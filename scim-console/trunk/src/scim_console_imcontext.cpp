/*
 * SCIM Console
 *
 * Copyright (c) 2006 Ryo Dairiki <ryo-dairiki@users.sourceforge.net>
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.*
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU Lesser General Public License for more details.*
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 */

/**
 * @file
 * @author Ryo Dairiki <ryo-dairiki@users.sourceforge.net>
 * @brief This file describes about the implementation of the IMContext class.
 */

#define Uses_SCIM_ATTRIBUTE
#define Uses_SCIM_BACKEND
#define Uses_SCIM_IMENGINE
#define Uses_SCIM_IMENGINE_MODULE
#define Uses_SCIM_UTILITY
#include <scim.h>

#include "scim_console_debug.h"
#include "scim_console_imcontext.h"

using namespace scim;

namespace scim_console
{
    
/**
 * The implementation class for IMContextes.
 */
class IMContextImpl: public IMContext
{

public:

    /* Public functions */

    /**
     * The construtor.
     * 
     * @param new_scim_console The instance of SCIM Console.
     */
    IMContextImpl (ScimConsole *new_scim_console);

    /**
     * The destructor.
     */
    ~IMContextImpl ();
    
    imcontext_id_t get_id () const;

    bool is_preedit_shown () const;

    int get_preedit_cursor_position () const;
    
    const scim::WideString &get_preedit_string () const;

    const scim::AttributeList &get_preedit_attributes () const;

    const scim::AttributeList &set_preedit_attributes () const;

    const scim::WideString &set_preedit_string () const;

    bool is_aux_shown () const;

    const scim::WideString &get_aux_string () const;
    
    const scim::AttributeList &get_aux_attributes () const;

    const scim::LookupTable &get_lookup_table () const;

    const scim::PanelFactoryInfo &get_imengine_info () const;
    
    const scim::String get_imengine_uuid () const;

    void change_imengine_by_uuid (const scim::String &factory_uuid);

    void change_imengine (scim::IMEngineFactoryPointer new_factory);

    void rotate_imengines (bool forward);

    bool process_key_event (const scim::KeyEvent &key_event);
    
    bool is_enabled () const;
    
    void set_enabled (bool status);

    bool is_lookup_table_shown () const;

    void lookup_table_page_up ();

    void lookup_table_page_down ();

    void lookup_table_select_candidate (int candidate_index);

    const scim::String get_help_string () const;

    void trigger_property (const scim::String &property);

    void process_helper_event (const scim::String &target_uuid, const scim::String &helper_uuid, const scim::Transaction &trans);

    /**
     * Set the visibility of the preedit string.
     *
     * @param shown Give true here to show the preedit.
     */
    void set_preedit_shown (bool shown);

    /**
     * Set the cursor position in the preedit string.
     *
     * @param cursor The new cursor position in the preedit string.
     */
    void set_preedit_cursor_position (int cursor);

    /**
     * Set the AttributeList for the preedit string.
     *
     * @param attrs The set of the new AttributeList for the preedit string.
     */
    void set_preedit_attributes (const scim::AttributeList &attrs);

    /**
     * Set the contents of the preedit.
     *
     * @param str The new contents for the preedit.
     */
    void set_preedit_string (const scim::WideString &str);

    /**
     * Update the preedit string.
     */
    void update_preedit ();

    /**
     * Set the visibility of the aux label.
     *
     * @param shown The new visibiliy of the aux label.
     */
    void set_aux_shown (bool shown);

    /**
     * Set the AttributeList for the aux string.
     *
     * @param attrs The set of the new AttributeList for the aux string.
     */
    void set_aux_attributes (const scim::AttributeList &attrs);

    /**
     * Set the contents of the aux label.
     *
     * @param aux The new contents of the aux label.
     */
    void set_aux_string (const scim::WideString &aux);

    /**
     * Update the aux string.
     */
    void update_aux ();

    /**
     * Commit a string.
     *
     * @param str The string to commit.
     */
    void commit_string (const scim::WideString &str);

    /**
     * Set the visibility of the lookup table.
     *
     * @param visible The visibility of the lookup table.
     */
    void set_lookup_table_shown (bool visible);

    /**
     * Update the lookup table.
     */
    void update_lookup_table ();

    /**
     * Set the lookup table.
     *
     * @param new_lookup_table The new lookup table.
     */
    void set_lookup_table (const scim::LookupTable &new_lookup_table);

    /**
     * Forward a key event into a client.
     * 
     * @param key_event The key event.
     */
    void forward_key_event (const scim::KeyEvent &key_event);

    /**
     * Register a set of properties.
     *
     * @param properties A set of properties.
     */
    void register_properties (const scim::PropertyList &properties);

    /**
     * Update a property.
     *
     * @param property The property to update.
     */
    void update_property (const scim::Property &property);

    /**
     * Make a beep sound.
     */
    void beep ();

    /**
     * Start a helper program.
     *
     * @param helper_uuid The UUID for the helper program.
     */
    void start_helper (const scim::String &helper_uuid);

    /**
     * Stop a helper program.
     *
     * @param helper_uuid The UUID for the helper program.
     */
    void stop_helper (const scim::String &helper_uuid);

    /**
     * Send an event for a helper program.
     *
     * @param helper_uuid The UUID for the helper program.
     * @param trans The event transaction. 
     */
    void send_helper_event (const scim::String &helper_uuid, const scim::Transaction &trans);

    /**
     * Get the surrounding text around the insertation cursor.
     *
     * @param text The buffer for the fetching text.
     * @param cursor The buffer for the fetching cursor position.
     * @param maxlen_before The maximum length of the surrounding text before the cursor.
     * @param maxlen_after The maximum length of the surrounding text after the cursor.
     * @return true if it succeeded to get it, otherwise it returns false.
     */
    bool get_surrounding_text (scim::WideString &text, int &cursor, int maxlen_before, int maxlen_after);

    /**
     * Delete a part of the surrounding text.
     * You must call get_surrounding_text () before using this;
     * The surrounding text in this function is what is given by the previous retval.
     *
     * @param offset The begining position of the part to delete in the surrounding text.
     * @param length The length of the part to delete in the surrounding text.
     * @return true if it succeeded to delete it, otherwise it returns false.
     */
    bool delete_surrounding_text (int offset, int length);

    /**
     * Replace the surrounding text.
     * You must call get_surrounding_text () before using this;
     * The surrounding text in this function is what is given by the previous retval.
     *
     * @param string The new surrounding text.
     * @param cursor_position The new cursor position in the surrounding text.
     * @return true if it succeeded to replace it, otherwise it returns false.
     */
    bool replace_surrounding_text (const scim::WideString &string, int cursor_position);

private:

	/**
	 * The instance of SCIM Console.
	 */
	ScimConsole *scim_console;

    /**
     * The id for the IMContext.
     */
    imcontext_id_t id;

    /**
     * The IM status.
     */
    bool enabled;

    /**
     * Whether the preedit is shown or not.
     */
    bool preedit_shown;

    /**
     * The preedit string.
     */
    scim::WideString preedit_string;

    /**
     * The AttributeList for the preedit string.
     */
    scim::AttributeList preedit_attributes;

    /**
     * The cursor position in the preedit string.
     */
    size_t preedit_cursor_position;

    /**
     * The visibility of the aux string.
     */
    bool aux_shown;

    /**
     * The contents of the aux label.
     */
    scim::WideString aux_string;

    /**
     * The set of the AttributeList for the aux string.
     */
    scim::AttributeList aux_attributes;

    /**
     * The visibility of the lookup table.
     */
    bool lookup_table_shown;

    /**
     * The lookup table.
     */
    scim::CommonLookupTable lookup_table;

    /**
     * The IMEngine.
     */
    scim::IMEngineInstancePointer imengine;
    
    /**
     * The factory info for the panel.
     */
    scim::PanelFactoryInfo factory_info;

    /**
     * The id for the IMEngine.
     */
    unsigned int imengine_id;

    /**
     * The factory for the fallback IMEngine.
     */
    scim::IMEngineFactoryPointer fallback_imengine_factory;

    /**
     * The fallback IMEngine.
     */
    scim::IMEngineInstancePointer fallback_imengine;

};

/*
 * Static variables.
 */

/**
 * The IMContext currently focused.
 */
static IMContextImpl *focused_imcontext = NULL;

};

using std::cerr;
using std::endl;
using std::vector;

using namespace scim;
using namespace scim_console;

/*
 * Slot functions.
 */

/**
 * The slot for showing the preedit string.
 *
 * @param imengine The current IMEngine.
 */
static void slot_show_preedit (IMEngineInstanceBase *imengine)
{
    if (focused_imcontext == NULL) {
        cerr << "No imcontext is focused" << endl;
    } else {
        focused_imcontext->set_preedit_shown (true);
    }
}


/**
 * The slot for hiding the preedit string.
 *
 * @param imengine The current IMEngine.
 */
static void slot_hide_preedit (IMEngineInstanceBase *imengine)
{
    if (focused_imcontext == NULL) {
        cerr << "No imcontext is focused" << endl;
    } else {
        focused_imcontext->set_preedit_shown (false);
    }
}


/**
 * The slot for updating the preedit string.
 *
 * @param imengine The current IMEngine.
 * @param str The new string.
 * @param attrs The list of AttributeList for the preedit string.
 */
static void slot_update_preedit_string (IMEngineInstanceBase *imengine, const WideString &str, const AttributeList &attrs)
{
    if (focused_imcontext == NULL) {
        cerr << "No imcontext is focused" << endl;
    } else {
        focused_imcontext->set_preedit_string (str);
        focused_imcontext->set_preedit_attributes (attrs);
        focused_imcontext->update_preedit ();
    }
}


/**
 * The slot for moving the cursor in  the preedit string.
 *
 * @param imengine The current IMEngine.
 * @param caret The new cursor position.
 */
static void slot_update_preedit_cursor_position (IMEngineInstanceBase *imengine, int caret)
{
    if (focused_imcontext == NULL) {
        cerr << "No imcontext is focused" << endl;
    } else {
        focused_imcontext->set_preedit_cursor_position (caret);
    }
}


/**
 * The slot for showing the aux string.
 *
 * @param imengine The current IMEngine.
 */
static void slot_show_aux (IMEngineInstanceBase *imengine)
{
    if (focused_imcontext == NULL) {
        cerr << "No imcontext is focused" << endl;
    } else {
        focused_imcontext->set_aux_shown (true);
        focused_imcontext->update_aux ();
    }
}


/**
 * The slot for hiding the aux string.
 *
 * @param imengine The current IMEngine.
 */
static void slot_hide_aux (IMEngineInstanceBase *imengine)
{
    if (focused_imcontext == NULL) {
        cerr << "No imcontext is focused" << endl;
    } else {
        focused_imcontext->set_aux_shown (false);
        focused_imcontext->update_aux ();
    }
}


/**
 * The slot for updating the aux string.
 *
 * @param imengine The current IMEngine.
 * @param str The new string.
 * @param attrs The list of AttributeList for the aux string.
 */
static void slot_update_aux_string (IMEngineInstanceBase *imengine, const WideString &str, const AttributeList &attrs)
{
    if (focused_imcontext == NULL) {
        cerr << "No imcontext is focused" << endl;
    } else {
        focused_imcontext->set_aux_string (str);
        focused_imcontext->set_aux_attributes (attrs);
        focused_imcontext->update_aux ();
    }
}


/**
 * The slot for commiting the string.
 *
 * @param imengine The current IMEngine.
 * @param str The string to commit.
 */
static void slot_commit (IMEngineInstanceBase *imengine, const WideString &str)
{
    if (focused_imcontext == NULL) {
        cerr << "No imcontext is focused" << endl;
    } else {
        focused_imcontext->commit_string (str);
    }
}


/**
 * The slot for showing the lookup table.
 *
 * @param imengine The current IMEngine.
 */
static void slot_show_lookup_table (IMEngineInstanceBase *imengine)
{
    if (focused_imcontext == NULL) {
        cerr << "No imcontext is focused" << endl;
    } else {
        focused_imcontext->set_lookup_table_shown (true);
        focused_imcontext->update_lookup_table ();
    }
}


/**
 * The slot for hiding the lookup table.
 *
 * @param imengine The current IMEngine.
 */
static void slot_hide_lookup_table (IMEngineInstanceBase *imengine)
{
    if (focused_imcontext == NULL) {
        cerr << "No imcontext is focused" << endl;
    } else {
        focused_imcontext->set_lookup_table_shown (false);
        focused_imcontext->update_lookup_table ();
    }
}


/**
 * The slot fot updating the lookup table.
 *
 * @param imengine The current IMEngine.
 * @param lookup_table The new lookup table.
 */
static void slot_update_lookup_table (IMEngineInstanceBase *imengine, const LookupTable &lookup_table)
{
    if (focused_imcontext == NULL) {
        cerr << "No imcontext is focused" << endl;
    } else {
        focused_imcontext->set_lookup_table (lookup_table);
    }
}


/**
 * The slot for forwarding key events into clients.
 * 
 * @param imengine The current IMEngine.
 * @param key_event The key event.
 */
static void slot_forward_key_event (IMEngineInstanceBase *imengine, const KeyEvent &key_event)
{
    if (focused_imcontext == NULL) {
        cerr << "No imcontext is focused" << endl;
    } else {
        focused_imcontext->forward_key_event (key_event);
    }
}

/**
 * The slot for registering a set of properties.
 *
 * @param imengine The current IMEngine.
 * @param properties A set of properties.
 */
static void slot_register_properties (IMEngineInstanceBase *imengine, const PropertyList &properties)
{
    if (focused_imcontext == NULL) {
        cerr << "No imcontext is focused" << endl;
    } else {
        focused_imcontext->register_properties (properties);
    }
}

/**
 * The slot for updating a property.
 *
 * @param imengine The current IMEngine.
 * @param propertie The property to update.
 */
static void slot_update_property (IMEngineInstanceBase *imengine, const Property &property)
{
    if (focused_imcontext == NULL) {
        cerr << "No imcontext is focused" << endl;
    } else {
        focused_imcontext->update_property (property);
    }
}

/**
 * The slot for making a beep sound.
 *
 * @param imengine The current IMEngine.
 */
static void slot_beep (IMEngineInstanceBase *imengine)
{
    if (focused_imcontext == NULL) {
        cerr << "No imcontext is focused" << endl;
    } else {
        focused_imcontext->beep ();
    }
}

/**
 * The slot for starting a helper program.
 *
 * @param imengine The current IMEngine.
 * @param helper_uuid The UUID for the helper program.
 */
static void slot_start_helper (IMEngineInstanceBase *imengine, const String &helper_uuid)
{
    if (focused_imcontext == NULL) {
        cerr << "No imcontext is focused" << endl;
    } else {
        focused_imcontext->start_helper (helper_uuid);
    }
}


/**
 * The slot for stopping a helper program.
 *
 * @param imengine The current IMEngine.
 * @param helper_uuid The UUID for the helper program.
 */
static void slot_stop_helper (IMEngineInstanceBase *imengine, const String &helper_uuid)
{
    if (focused_imcontext == NULL) {
        cerr << "No imcontext is focused" << endl;
    } else {
        focused_imcontext->stop_helper (helper_uuid);
    }
}


/**
 * The slot for sending an event for a helper program.
 *
 * @param imengine The current IMEngine.
 * @param helper_uuid The UUID for the helper program.
 * @param trans The event transaction.
 */
static void slot_send_helper_event (IMEngineInstanceBase *imengine, const String &helper_uuid, const Transaction &trans)
{
    if (focused_imcontext == NULL) {
        cerr << "No imcontext is focused" << endl;
    } else {
        focused_imcontext->send_helper_event (helper_uuid, trans);
    }
}

/**
 * The slot for getting the surrounding text.
 *
 * @param imengine The current IMEngine.
 * @param text The buffer for the fetching text.
 * @param cursor The buffer for fetching cursor position in the surrounding text.
 * @param maxlen_before The maximum length of the surrounding text before the cursor.
 * @param maxlen_after The maximum length of the surrounding text after the cursor.
 * @return true if it succeeded, otherwise it returns false.
 */
bool slot_get_surrounding_text (IMEngineInstanceBase *imengine, WideString &text, int &cursor, int maxlen_before, int maxlen_after)
{
    if (focused_imcontext == NULL) {
        cerr << "No imcontext is focused" << endl;
        return false;
    } else {
        return focused_imcontext->get_surrounding_text (text, cursor, maxlen_before, maxlen_after);
    }
}


/**
 * The slot for deleting the surrounding text.
 *
 * @param imengine The current IMEngine.
 * @param offset The begining offset of the part of the surrounding text.
 * @param offset The length of the part of the surrounding text.
 * @return true if it succeeded, otherwise it returns false.
 */
bool slot_delete_surrounding_text (IMEngineInstanceBase *imengine, int offset, int length)
{
    if (focused_imcontext == NULL) {
        cerr << "No imcontext is focused" << endl;
        return false;
    } else {
        return focused_imcontext->delete_surrounding_text (offset, length);
    }
}


/**
 * The slot for deleting the surrounding text.
 *
 * @param offset The begining offset of the part of the surrounding text.
 * @param offset The length of the part of the surrounding text.
 * @return true if it succeeded, otherwise it returns false.
 */
bool slot_replace_surrounding_text (const WideString &string, int cursor_position)
{
    if (focused_imcontext == NULL) {
        cerr << "No imcontext is focused" << endl;
        return false;
    } else {
        return focused_imcontext->replace_surrounding_text (string, cursor_position);
    }
}

/**
 * The slot for commit fallback strings.
 *
 * @param imengine The imengine.
 * @param commit_str The string to commit.
 */
void slot_fallback_commit (IMEngineInstanceBase *imengine, const WideString &commit_str)
{
    if (focused_imcontext == NULL) {
        cerr << "No imcontext is focused" << endl;
    } else {
        focused_imcontext->commit_string (commit_str);
    }
}


/*
 * Implementations.
 */
IMContext *IMContext::alloc (ScimConsole *scim_console)
{
	if (focused_imcontext != NULL) {
		cerr << "Another instance of IMContext already exists" << endl;
		return NULL;
	}
	
	focused_imcontext = new IMContextImpl (scim_console);
	return focused_imcontext;
}

IMContext *IMContext::get_focused_imcontext ()
{
    return focused_imcontext;
}


imcontext_id_t IMContext::get_focused_imcontext_id ()
{
    if (focused_imcontext == NULL) {
        return -1;
    } else {
        return focused_imcontext->get_id ();
    }
}

IMContextImpl::IMContextImpl (ScimConsole *new_scim_console): scim_console (new_scim_console), id (0), enabled (false), 
preedit_cursor_position (0), preedit_shown (false), aux_shown (false), lookup_table_shown (false), imengine_id (0)
{
    BackEndPointer scim_backend = scim_console->get_scim_backend ();
    fallback_imengine_factory = scim_backend->get_factory (SCIM_COMPOSE_KEY_FACTORY_UUID);
    if (fallback_imengine_factory.null ()) fallback_imengine_factory = new DummyIMEngineFactory ();

    fallback_imengine = fallback_imengine_factory->create_instance (String ("UTF-8"), 0);
    fallback_imengine->signal_connect_commit_string (slot (slot_fallback_commit));

    set_enabled (false);
}


IMContextImpl::~IMContextImpl ()
{
    focused_imcontext = NULL;
}


imcontext_id_t IMContextImpl::get_id () const
{
    return id;
}


const String IMContextImpl::get_imengine_uuid () const
{
    return imengine->get_factory_uuid ();
}


void IMContextImpl::change_imengine_by_uuid (const String &factory_uuid)
{
    IMEngineFactoryPointer factory = scim_console->get_scim_backend ()->get_factory (factory_uuid);
    change_imengine (factory);
}


void IMContextImpl::change_imengine (IMEngineFactoryPointer new_factory)
{
    if (imengine != NULL) {
        imengine->focus_out ();
        imengine->reset ();
    }

    BackEndPointer scim_backend = scim_console->get_scim_backend ();
    scim_backend->set_default_factory (scim_console->get_scim_language (), new_factory->get_uuid ());
    imengine = new_factory->create_instance ("UTF-8", imengine_id);
    ++imengine_id;

    factory_info = PanelFactoryInfo (new_factory->get_uuid (), utf8_wcstombs (new_factory->get_name ()), new_factory->get_language (), new_factory->get_icon_file ());

    if (imengine != NULL) {
        imengine->signal_connect_show_preedit_string (slot (slot_show_preedit));
        imengine->signal_connect_show_aux_string (slot (slot_show_aux));
        imengine->signal_connect_show_lookup_table (slot (slot_show_lookup_table));

        imengine->signal_connect_hide_preedit_string (slot (slot_hide_preedit));
        imengine->signal_connect_hide_aux_string (slot (slot_hide_aux));
        imengine->signal_connect_hide_lookup_table (slot (slot_hide_lookup_table));

        imengine->signal_connect_update_preedit_caret (slot (slot_update_preedit_cursor_position));
        imengine->signal_connect_update_preedit_string (slot (slot_update_preedit_string));
        imengine->signal_connect_update_aux_string (slot (slot_update_aux_string));
        imengine->signal_connect_update_lookup_table (slot (slot_update_lookup_table));
        imengine->signal_connect_commit_string (slot (slot_commit));
        imengine->signal_connect_forward_key_event (slot (slot_forward_key_event));

        imengine->signal_connect_register_properties (slot (slot_register_properties));
        imengine->signal_connect_update_property (slot (slot_update_property));

        imengine->signal_connect_beep (slot (slot_beep));

        imengine->signal_connect_start_helper (slot (slot_start_helper));
        imengine->signal_connect_stop_helper (slot (slot_stop_helper));
        imengine->signal_connect_send_helper_event (slot (slot_send_helper_event));

        imengine->signal_connect_get_surrounding_text (slot (slot_get_surrounding_text));
        imengine->signal_connect_delete_surrounding_text (slot (slot_delete_surrounding_text));
    }
    
    preedit_string.clear ();
    preedit_attributes.clear ();
    preedit_cursor_position = 0;
    preedit_shown = false;

    enabled = true;

    imengine->focus_in ();
}


void IMContextImpl::rotate_imengines (bool forward)
{
    BackEndPointer scim_backend = scim_console->get_scim_backend ();

    IMEngineFactoryPointer new_factory;
    if (forward) {
        new_factory = scim_backend->get_next_factory ("", "UTF-8", imengine->get_factory_uuid ());
    } else {
        new_factory = scim_backend->get_previous_factory ("", "UTF-8", imengine->get_factory_uuid ());
    }
    change_imengine (new_factory);
}


void IMContextImpl::set_preedit_shown (bool shown)
{
    preedit_shown = shown;
    if (!preedit_shown) {
        preedit_cursor_position = 0;
        preedit_string.clear ();
        preedit_attributes.clear ();
    }
}


bool IMContextImpl::is_preedit_shown () const
{
    return preedit_shown;
}


int IMContextImpl::get_preedit_cursor_position () const
{
    return preedit_cursor_position;
}


void IMContextImpl::set_preedit_cursor_position (int cursor_position)
{
    preedit_cursor_position = cursor_position;
}

void IMContextImpl::set_preedit_string (const WideString &new_preedit_string)
{
    preedit_string = new_preedit_string;
}

const WideString &IMContextImpl::get_preedit_string () const
{
    return preedit_string;
}

void IMContextImpl::update_preedit ()
{
    scim_console->update_display ();
}

const AttributeList &IMContextImpl::get_preedit_attributes () const
{
    return preedit_attributes;
}

void IMContextImpl::set_preedit_attributes (const AttributeList &new_attributes)
{
    preedit_attributes = new_attributes;
}


bool IMContextImpl::is_aux_shown () const
{
    return aux_shown;
}


void IMContextImpl::set_aux_shown (bool shown)
{
    aux_shown = shown;
}


const WideString &IMContextImpl::get_aux_string () const
{
    return aux_string;
}


void IMContextImpl::set_aux_string (const WideString &string)
{
    aux_string = string;
}


const AttributeList &IMContextImpl::get_aux_attributes () const
{
    return aux_attributes;
}


void IMContextImpl::set_aux_attributes (const AttributeList &new_attributes)
{
    aux_attributes = new_attributes;
}


void IMContextImpl::update_aux ()
{
    //cerr << "Not implemented: IMContextImpl::update_aux" << endl;
}


void IMContextImpl::commit_string (const WideString &str)
{
    scim_console->forward_pty_input (utf8_wcstombs (str));
}


const LookupTable &IMContextImpl::get_lookup_table () const
{
    return lookup_table;
}

void IMContextImpl::update_lookup_table ()
{
    scim_console->update_display ();
}

bool IMContextImpl::is_lookup_table_shown () const
{
    return lookup_table_shown;
}

void IMContextImpl::set_lookup_table_shown (bool visible)
{
    lookup_table_shown = visible;
}

void IMContextImpl::set_lookup_table (const LookupTable &new_lookup_table)
{
    lookup_table.clear ();
    lookup_table.set_page_size (new_lookup_table.get_page_size ());
    lookup_table.show_cursor (new_lookup_table.is_cursor_visible ());
    lookup_table.fix_page_size (new_lookup_table.is_page_size_fixed ());
    for (int i = 0; i < new_lookup_table.number_of_candidates (); ++i) {
        const WideString &label = new_lookup_table.get_candidate (i);
        const AttributeList &attributes = new_lookup_table.get_attributes (i);
        lookup_table.append_candidate (label, attributes);
    }
    lookup_table.set_cursor_pos (new_lookup_table.get_cursor_pos ());
}

void IMContextImpl::lookup_table_page_up ()
{
    imengine->lookup_table_page_up ();
}

void IMContextImpl::lookup_table_page_down ()
{
    imengine->lookup_table_page_down ();
}

void IMContextImpl::lookup_table_select_candidate (int candidate)
{
    imengine->select_candidate (candidate);
}

void IMContextImpl::forward_key_event (const KeyEvent &key_event)
{
    scim_console->forward_key_event (key_event);
}


void IMContextImpl::beep ()
{
    scim_console->beep ();
}

void IMContextImpl::start_helper (const String &helper_uuid)
{
    scim_console->start_helper (helper_uuid);
}

void IMContextImpl::stop_helper (const String &helper_uuid)
{
    scim_console->stop_helper (helper_uuid);
}

void IMContextImpl::process_helper_event (const String &imengine_uuid, const String &helper_uuid, const Transaction &trans)
{
    if (imengine->get_factory_uuid () == imengine_uuid)
        imengine->process_helper_event (helper_uuid, trans);
}

void IMContextImpl::send_helper_event (const String &helper_uuid, const Transaction &trans)
{
    scim_console->send_helper_event (helper_uuid, trans);
}

bool IMContextImpl::get_surrounding_text (WideString &text, int &cursor, int maxlen_before, int maxlen_after)
{
    return false;
}

bool IMContextImpl::delete_surrounding_text (int offset, int length)
{
    return false;
}

bool IMContextImpl::replace_surrounding_text (const WideString &string, int cursor_position)
{
    return false;
}


bool IMContextImpl::is_enabled () const
{
    return enabled;
}


void IMContextImpl::set_enabled (bool status)
{
    enabled = status;
    if (imengine != NULL) {
        imengine->focus_out ();
        imengine->reset ();
    }

    BackEndPointer scim_backend = scim_console->get_scim_backend ();
    const String scim_language = scim_console->get_scim_language ();
    if (enabled) {
        IMEngineFactoryPointer factory = scim_backend->get_default_factory (scim_language, "UTF-8");
        if (factory != NULL) {
            imengine = factory->create_instance ("UTF-8", imengine_id);
            ++imengine_id;
        }
    } else {
        imengine = NULL;
    }

    if (imengine == NULL) {
        imengine = fallback_imengine;
        factory_info = PanelFactoryInfo (String (""), String (_ ("English/Keyboard")), String ("C"), String (SCIM_KEYBOARD_ICON_FILE));
    } else {
        IMEngineFactoryPointer factory = scim_backend->get_factory (imengine->get_factory_uuid ());
        factory_info = PanelFactoryInfo (factory->get_uuid (), utf8_wcstombs (factory->get_name ()), factory->get_language (), factory->get_icon_file ());
        
        imengine->signal_connect_show_preedit_string (slot (slot_show_preedit));
        imengine->signal_connect_show_aux_string (slot (slot_show_aux));
        imengine->signal_connect_show_lookup_table (slot (slot_show_lookup_table));

        imengine->signal_connect_hide_preedit_string (slot (slot_hide_preedit));
        imengine->signal_connect_hide_aux_string (slot (slot_hide_aux));
        imengine->signal_connect_hide_lookup_table (slot (slot_hide_lookup_table));

        imengine->signal_connect_update_preedit_caret (slot (slot_update_preedit_cursor_position));
        imengine->signal_connect_update_preedit_string (slot (slot_update_preedit_string));
        imengine->signal_connect_update_aux_string (slot (slot_update_aux_string));
        imengine->signal_connect_update_lookup_table (slot (slot_update_lookup_table));
        imengine->signal_connect_commit_string (slot (slot_commit));
        imengine->signal_connect_forward_key_event (slot (slot_forward_key_event));

        imengine->signal_connect_register_properties (slot (slot_register_properties));
        imengine->signal_connect_update_property (slot (slot_update_property));

        imengine->signal_connect_beep (slot (slot_beep));

        imengine->signal_connect_start_helper (slot (slot_start_helper));
        imengine->signal_connect_stop_helper (slot (slot_stop_helper));
        imengine->signal_connect_send_helper_event (slot (slot_send_helper_event));

        imengine->signal_connect_get_surrounding_text (slot (slot_get_surrounding_text));
        imengine->signal_connect_delete_surrounding_text (slot (slot_delete_surrounding_text));
    }
        
    imengine->focus_in ();
}


bool IMContextImpl::process_key_event (const scim::KeyEvent &key_event)
{
    dout (6) << "IMContextImpl::process_key_event ()" << endl;
    if (enabled) {
        return imengine->process_key_event (key_event);
    } else {
        return false;
    }
}

const String IMContextImpl::get_help_string () const
{
    String help_string = String (_ ("Smart Common Input Method platform ")) +
                         String (SCIM_VERSION) +
                         String (_ ("\n(C) 2002-2005 James Su <suzhe@tsinghua.org.cn>\n\n"));

    IMEngineFactoryPointer factory = scim_console->get_scim_backend ()->get_factory (imengine->get_factory_uuid ());
    if (!factory.null ()) {
        help_string += utf8_wcstombs (factory->get_name ());
        help_string += String (_ (":\n\n"));

        help_string += utf8_wcstombs (factory->get_authors ());
        help_string += String (_ ("\n\n"));

        help_string += utf8_wcstombs (factory->get_help ());
        help_string += String (_ ("\n\n"));

        help_string += utf8_wcstombs (factory->get_credits ());
    }
    return help_string;
}

void IMContextImpl::trigger_property (const String &property)
{
    imengine->trigger_property (property);
}

const PanelFactoryInfo &IMContextImpl::get_imengine_info () const
{
    return factory_info;
}

void IMContextImpl::register_properties (const PropertyList &properties)
{
    scim_console->register_properties (properties);
}

void IMContextImpl::update_property (const Property &property)
{
    scim_console->update_property (property);
}


