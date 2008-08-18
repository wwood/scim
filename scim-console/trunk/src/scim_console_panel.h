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
 * @brief This header describes about the public interface for the panel.
 */

#ifndef SCIMCONSOLEPANEL_H_
#define SCIMCONSOLEPANEL_H_

#include "scim_console_common.h"

namespace scim_console
{

class Menu;
class ScimConsole;

/**
 * The public interface of the panel.
 */
class Panel
{

public:

    /**
     * Allocate a panel.
     *
     * @param scim_console The SCIM Console.
     * @return The new instance of the panel.
     */
    static Panel *alloc (ScimConsole *scim_console);

    /**
     * Destructor.
     */
    virtual ~Panel ()
    {}
    
    /**
     * See if the popup is shown or not.
     *
     * @return true if the popup is shown.
     */
    virtual bool is_popup_shown () const = 0;
    
    /**
     * Get the message shown in the popup.
     * 
     * @return The popup string.
     */
    virtual const scim::String &get_popup_string () const = 0;
    
    /**
     * Get the menu of the panel.
     * 
     * @return The menu of the panel.
     */
    virtual const Menu &get_menu () const = 0;
    
    /**
     * Register properties.
     *
     * @param properties The properties to register.
     * @return RETVAL_SUCCEEDED if it succeeded, otherwise it returns RETVAL_FAILED.
     */
    virtual retval_t register_properties (const scim::PropertyList &properties) = 0;
    
    /**
     * Deregister properties.
     *
     * @param properties The properties to register.
     * @return RETVAL_SUCCEEDED if it succeeded, otherwise it returns RETVAL_FAILED.
     */
    virtual retval_t deregister_properties (const scim::PropertyList &properties) = 0;

    /**
     * Update a property.
     *
     * @param property The property to update.
     * @return RETVAL_SUCCEEDED if it succeeded, otherwise it returns RETVAL_FAILED.
     */
    virtual retval_t update_property (const scim::Property &property) = 0;
    
    /**
     * Update the panel.
     * 
     * @return RETVAL_SUCCEEDED if it succeeded, otherwise it returns RETVAL_FAILED.
     */
    virtual retval_t update () = 0;

    /**
     * Check if the panel has been updated.
     *
     * @return true if the panel has been updated.
     */
    virtual bool has_update () const = 0;

    /**
     * Clear update flag.
     */
    virtual void clear_update () = 0;

    /**
     * Request to show the help message.
     * 
     * @return RETVAL_SUCCEEDED if it succeeded to show that.
     */
    virtual retval_t request_help () = 0;

    /**
     * Request to show the factory menu.
     * 
     * @return RETVAL_SUCCEEDED if it succeeded to show that.
     */
    virtual retval_t request_factory_menu () = 0;
    
    /**
     * The menu hotkey is pressed.
     *
     * @return true if the key event is consumed.
     */
    virtual bool process_hotkey_menu () = 0;
    
    /**
     * The ok hotkey is pressed.
     *
     * @return true if the key event is consumed.
     */
    virtual bool process_hotkey_ok () = 0;
    
    /**
     * The cancel hotkey is pressed.
     *
     * @return true if the key event is consumed.
     */
    virtual bool process_hotkey_cancel () = 0;
    
    /**
     * The previous hotkey is pressed.
     *
     * @return true if the key event is consumed.
     */
    virtual bool process_hotkey_previous () = 0;
    
    /**
     * The next hotkey is pressed.
     *
     * @return true if the key event is consumed.
     */
    virtual bool process_hotkey_next () = 0;

protected:


    /**
     * Constructor.
     */
    Panel ()
    {}

}
;

};

#endif              /*SCIMCONSOLEPANEL_H_*/
