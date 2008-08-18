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
 * @brief This header describes about the interface of the terminal display.
 */

#ifndef SCIMCONSOLEDISPLAY_H_
#define SCIMCONSOLEDISPLAY_H_

#define Uses_SCIM_TYPES
#include <scim.h>

#include "scim_console_common.h"
#include "scim_console_event_listener.h"
#include "scim_console_point.h"

namespace scim_console
{

class ScimConsole;

/**
 * The interface of the terminal display.
 */
class Display: public EventListener
{

public:

    /**
     * Alloc a display.
     *
     * @param scim_console The scim console.
     * @return The new display.
     */
    static Display *alloc (ScimConsole *scim_console);

    /**
     * Destructor.
     */
    virtual ~Display ()
    {}

    /**
     * Update this displays.
     *
     * @return RETVAL_SUCCEEDED if it succeeded.
     */
    virtual retval_t update () = 0;

    /**
     * Forward the output string.
     *
     * @return RETVAL_SUCCEEDED if it succeeded.
     */
    virtual retval_t forward_pty_output (const scim::String &output_string) = 0;

    /**
     * Make a beep sound.
     *
     * @return RETVAL_SUCCEEDED if it succeeded.
     */
    virtual retval_t beep () = 0;

    /**
     * Get the name of the display.
     * 
     * @return The name of the display.
     */
    virtual const scim::String &get_name () const = 0;

    /**
     * Get the tty number for the display.
     *
     * @return The tty number for the display.
     */
    virtual int get_tty_number () const = 0;

    /**
     * See if the display is active or not.
     *
     * @return true if the display is active.
     */
    virtual bool is_active () const = 0;

    /**
     * Get the location of the cursor.
     *
     * @param point The dest buffer for a pointer.
     * @return RETVAL_SUCCEEDED if it succeeded.
     */
    virtual retval_t get_cursor_location (Point &point) const = 0;

    /**
     * Get the width of the display.
     * 
     * @return The width of the display.
     */
    virtual size_t get_width () const = 0;

    /**
     * Get the height of the display.
     * 
     * @return The height of the display.
     */
    virtual size_t get_height () const = 0;

    /**
     * Get the width of the client display.
     * 
     * @return The width of the client display.
     */
    virtual size_t get_client_width () const = 0;

    /**
     * Get the height of the client display.
     * 
     * @return The height of the client display.
     */
    virtual size_t get_client_height () const = 0;

protected:

    /**
     * Constructor.
     */
    Display ()
    {}

};

};
#endif                                            /*SCIMCONSOLEDISPLAY_H_*/
