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
 * @brief This header describes about the interface of scim console.
 */

#ifndef SCIMCONSOLE_H_
#define SCIMCONSOLE_H_

#include <sys/time.h>

#define Uses_SCIM_BACKEND
#define Uses_SCIM_CONFIG
#define Uses_SCIM_EVENT
#define Uses_SCIM_PROPERTY
#define Uses_SCIM_TYPES
#include <scim.h>

#include "scim_console_common.h"

namespace scim_console
{

class Display;
class Helper;
class Panel;

/**
 * The public interface of scim console.
 */
class ScimConsole
{

public:

    /**
     * Destructor.
     */
    virtual ~ScimConsole ()
    {}

    /**
     * Quit scim console.
     * 
     * @param retval The retval of this process.
     */
    virtual void quit (int retval = 0) = 0;

    /**
     * Abort scim console immediately.
     * The retval of this process is set as -1.
     */
    virtual void abort () = 0;
    
    /**
     * Make an interruption for the main loop.
     *
     * @param time The longest time for the next interruption.
     */
    virtual void interrupt (const timeval &time) = 0;

    /**
     * Make an interruption for the main loop immediately.
     */
    virtual void interrupt () = 0;

    /**
     * Process a key event.
     *
     * @param key_event The key event.
     * @param hotkeys_enabled The hotkeys are enabled or not.
     * @return true if the key event is consumed.
     */
    virtual bool process_key_event (const scim::KeyEvent &key_event, bool hotkeys_enabled = true) = 0;

    /**
     * Handle the output from the child process.
     *
     * @param string The output string.
     */
    virtual void process_pty_output (const scim::String &string) = 0;

    /**
     * Forward a key event into the client process.
     * 
     * @param key_event The key event.
     */
    virtual void forward_key_event (const scim::KeyEvent &key_event) = 0;

    /**
     * Forward the input string into the cient process.
     * 
     * @param string The string to send the client.
     */
    virtual void forward_pty_input (const scim::String &string) = 0;

    /**
     * Update the display.
     */
    virtual void update_display () = 0;

    /**
     * Make a beep sound.
     */
    virtual void beep () = 0;

    /**
     * Register the set of properties.
     * 
     * @param properties The set of properties.
     */
    virtual void register_properties (const scim::PropertyList &properties) = 0;

    /**
     * Update a property.
     * 
     * @param property The property to update.
     */
    virtual void update_property (const scim::Property &property) = 0;

    /**
     * Register the set of properties for a helper.
     * 
     * @param properties The set of properties.
     */
    virtual void register_helper_properties (const scim::PropertyList &properties) = 0;

    /**
     * Update a property for a helper.
     * 
     * @param property The property to update.
     */
    virtual void update_helper_property (const scim::Property &property) = 0;

    /**
     * Get the backend for SCIM.
     *
     * @return The backend for SCIM.
     */
    virtual scim::BackEndPointer get_scim_backend () = 0;

    /**
     * Get the language for SCIM.
     *
     * @return The language for SCIM.
     */
    virtual const scim::String &get_scim_language () const = 0;
    
    /**
     * Get the configurationh for SCIM.
     * 
     * @return The configuration for SCIM.
     */
    virtual scim::ConfigPointer get_scim_config () = 0;

    /**
     * Get the keyboard layout of SCIM.
     *
     * @return The keyboard layout for SCIM.
     */
    virtual const scim::KeyboardLayout &get_scim_keyboard_layout () const = 0;
    
    /**
     * Get display.
     * 
     * @return The display.
     */
    virtual const Display &get_display () const = 0;

    /**
     * Get panel.
     * 
     * @return The panel.
     */
    virtual Panel &get_panel () = 0;
    
    /**
     * Get panel.
     * 
     * @return The panel.
     */
    virtual const Panel &get_panel () const = 0;
    
    /**
     * Accept a new helper.
     * 
     * @param helper The new helper.
     */
    virtual void accept_helper (Helper *helper) = 0;

    /**
     * Start a helper process.
     *
     * @param helper_uuid The UUID for the helper.
     */
    virtual void start_helper (const scim::String &helper_uuid) = 0;

    /**
     * Stop a helper process.
     *
     * @param helper_uuid The UUID for the helper.
     */
    virtual void stop_helper (const scim::String &helper_uuid) = 0;

    /**
     * Send an event for a helper program.
     *
     * @param helper_uuid The UUID for the helper program.
     * @param trans The event transaction. 
     */
    virtual void send_helper_event (const scim::String &helper_uuid, const scim::Transaction &trans) = 0;

    /**
     * Save the configuration for SCIM.
     */
    virtual void save_config () = 0;

    /**
     * Load the configuration for SCIM.
     */
    virtual void load_config () = 0;


protected:

    /**
     * Constructor.
     */
    ScimConsole ()
    {}

};

}
;

#endif                                            /*SCIMCONSOLE_H_*/
