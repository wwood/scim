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
 * @brief This header describes about the public interface for the IMContext.
 */

#ifndef SCIMCONSOLEIMCONTEXT_H_
#define SCIMCONSOLEIMCONTEXT_H_

#include <vector>

#define Uses_SCIM_ATTRIBUTE
#define Uses_SCIM_IMENGINE
#define Uses_SCIM_PANEL
#define Uses_SCIM_TRANSACTION
#include <scim.h>

#include "scim_console.h"
#include "scim_console_common.h"

namespace scim_console
{

/**
 * The public interface of IMContexts.
 */
class IMContext
{

public:

    /**
     * Allocate an IMContext.
     * 
     * @return A new IMContext, otherwise it returns NULL. 
     */
    static IMContext *alloc (ScimConsole *scim_console);

    /**
     * Get the IMContext currently focused.
     * 
     * @return The focused IMContext.
     */
    static IMContext *get_focused_imcontext ();

    /**
     * Get the ID for the focused IMContext.
     * 
     * @return The ID for the focused IMContext.
     */
    static imcontext_id_t get_focused_imcontext_id ();

    /**
     * Destructor.
     */
    virtual ~IMContext ()
    {}

    /**
     * Get the ID for the IMContext.
     * 
     * @return The ID for the IMContext.
     */
    virtual imcontext_id_t get_id () const = 0;

    /**
     * Get the information of the current IMEngine.
     * 
     * @return The information of the current IMEngine.
     */
    virtual const scim::PanelFactoryInfo &get_imengine_info () const = 0;

    /**
     * Get the uuid for the current IMEngine.
     *
     * @return The UUID for the current IMEngine.
     */
    virtual const scim::String get_imengine_uuid () const = 0;

    /**
     * Change the IMEngine by uuid.
     *
     * @param The uuid for the IMEngine.
     */
    virtual void change_imengine_by_uuid (const scim::String &factory_uuid) = 0;

    /**
     * Change the IMEngine.
     *
     * @param The factory for the new IMEngine.
     */
    virtual void change_imengine (scim::IMEngineFactoryPointer new_factory) = 0;

    /**
     * Rotate IMEngines.
     * Choose the next or the previous IMEngine.
     *
     * @param forward Give true here to rotate forward, otherwise rotations goes backward.
     */
    virtual void rotate_imengines (bool forward) = 0;

    /**
     * Get if the preedit string is shown or not.
     *
     * @return true when the preedit is shown.
     */
    virtual bool is_preedit_shown () const = 0;

    /**
     * Get the cursor position in the preedit string.
     *
     * @return The cursor position in the preedit string.
     */
    virtual int get_preedit_cursor_position () const = 0;

    /**
     * Get the string for the preedit.
     *
     * @return The preedit string.
     */
    virtual const scim::WideString &get_preedit_string () const = 0;

    /**
     * Get the attributes of the preedit string.
     *
     * @return The attributes of the preedit string.
     */
    virtual const scim::AttributeList &get_preedit_attributes () const = 0;

    /**
     * Get if the aux string is shown or not.
     *
     * @return true when the aux is shown.
     */
    virtual bool is_aux_shown () const = 0;

    /**
     * Get the string for the aux.
     *
     * @return The aux string.
     */
    virtual const scim::WideString &get_aux_string () const = 0;

    /**
     * Get the attributes of the aux string.
     *
     * @return The attributes of the aux string.
     */
    virtual const scim::AttributeList &get_aux_attributes () const = 0;

    /**
     * Get the visibility of the lookp table.
     * 
     * @return The visibility of the lookup table.
     */
    virtual bool is_lookup_table_shown () const = 0;

    /**
     * Get the lookup table.
     *
     * @return The lookup table.
     */
    virtual const scim::LookupTable &get_lookup_table () const = 0;

    /**
     * Process a key event.
     *
     * @param key_event The key event.
     * @return true if the key event is consumed.
     */
    virtual bool process_key_event (const scim::KeyEvent &key_event) = 0;

    /**
     * See if the IM is enabled or not.
     *
     * @return true if the IM is enabled.
     */
    virtual bool is_enabled () const = 0;

    /**
     * Turn on or off the IME.
     *
     * @param status Give true here to turn on the IME.
     */
    virtual void set_enabled (bool status) = 0;

    /**
     * Page up the lookup table.
     */
    virtual void lookup_table_page_up () = 0;

    /**
     * Page down the lookup table.
     */
    virtual void lookup_table_page_down () = 0;

    /**
     * Select a candidate in the lookup table.
     * 
     * @param candidate_index The index of the candidate to select.
     */
    virtual void lookup_table_select_candidate (int candidate_index) = 0;

    /**
     * Get the help message.
     *
     * @return The help string.
     */
    virtual const scim::String get_help_string () const = 0;

    /**
     * Trigger a property.
     * 
     * @param property The name of the property to trigger.
     */
    virtual void trigger_property (const scim::String &property) = 0;

    /**
     * Process a helper event.
     * 
     * @param target_uuid The UUID for the target.
     * @param helper_uuid The UUID of the helper process.
     * @param trans The event transaction.
     */
    virtual void process_helper_event (const scim::String &target_uuid, const scim::String &helper_uuid, const scim::Transaction &trans) = 0;

protected:

    /**
     * Constructor.
     */
    IMContext ()
    {}

};

};
#endif                                            /*SCIMCONSOLEIMCONTEXT_H_*/
