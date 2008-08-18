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
 * @brief This header describes about the interface of interruption listeners.
 */

#ifndef SCIMCONSOLEINTERRUPTIONLISTENER_H_
#define SCIMCONSOLEINTERRUPTIONLISTENER_H_

#include <sys/time.h>

#include "scim_console_event_listener.h"

namespace scim_console
{


/**
 * The interface of interruption handlers.
 */
class InterruptionListener: public EventListener
{

    public:

        /**
         * Allocate an interruption listener.
         *
         * @return A new interruption listener.
         */
        static InterruptionListener *alloc ();

        /**
         * Destructor.
         */
        virtual ~InterruptionListener () {}

        /**
         * See if there is pending interruption.
         *
         * @return true if there is a pending interruption.
         */
        virtual bool has_interruption () const = 0;

        /**
         * Clear the pending interruption.
         */
        virtual void clear_interruption () = 0;

        /**
         * Get the remaining time for the next interruption.
         *
         * @return The remaining time for the next interruption.
         */
        virtual timeval get_remaining_time () const = 0;

        /**
         * Make an interruption.
         *
         * @param time The longest time for the next interruption.
         */
        virtual void interrupt (const timeval &time) = 0;

    protected:

        /**
         * Constructor.
         */
        InterruptionListener () {}

};

};

#endif              /*SCIMCONSOLEINTERRUPTIONLISTENER_H_*/
