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
 * @brief This header describes about the interface of key event readers.
 */

#ifndef SCIMCONSOLEKEYEVENTREADER_H_
#define SCIMCONSOLEKEYEVENTREADER_H_

#define Uses_SCIM_EVENT
#include <scim.h>

#include "scim_console.h"
#include "scim_console_common.h"
#include "scim_console_event_listener.h"

namespace scim_console
{

/**
 * The public interface of key event readers.
 */
class KeyEventReader: public EventListener
{

    public:

        /**
         * Allocate a new KeyEventReader.
         *
         * @param scim_console The ScimConsole.
         * @return The new KeyEventReader.
         */
        static KeyEventReader *alloc (ScimConsole *scim_console);

        /**
         * Destructor.
         */
        virtual ~KeyEventReader () {}

#if 0
        /**
         * Get the layout for this keyboard.
         *
         * @return The keyboard layout.
         */
        virtual scim::KeyboardLayout get_keyboard_layout () const = 0;

        /**
         * Set the layout for this keyboard.
         *
         * @param new_keyboard_layout The keyboard layout.
         */
        virtual void set_keyboard_layout (scim::KeyboardLayout new_keyboard_layout) = 0;

        /**
         * Guess the keyboard layout.
         */
        virtual void guess_keyboard_layout () = 0;
#endif

    protected:

        /**
         * Constructor.
         */
        KeyEventReader () {}

};

};

#endif              /*SCIMCONSOLEKEYEVENTREADER_H_*/
