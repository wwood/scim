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
 * @brief This header describes about the interface of event listeners.
 */

#ifndef SCIMCONSOLEEVENTLISTENER_H_
#define SCIMCONSOLEEVENTLISTENER_H_

#include "scim_console_common.h"

namespace scim_console
{


/**
 * The typedef for event types.
 */
typedef unsigned int event_type_t;

/**
 * The typedef for IDs of event listeners.
 */
typedef unsigned int event_listener_id_t;

/**
 * This value stands for no event at all.
 */
static const event_type_t EVENT_NONE = 0;


/**
 * This value stands for reading events.
 */
static const event_type_t EVENT_READ = 1 << 0;


/**
 * This value stands for writing events.
 */
static const event_type_t EVENT_WRITE = 1 << 1;


/**
 * This value stands for error events.
 */
static const event_type_t EVENT_ERROR = 1 << 2;


/**
 * This value stands for interruption events.
 */
static const event_type_t EVENT_INTERRUPT = 1 << 3;


/**
 * The interface of event listeners.
 */
class EventListener
{

    public:


        /**
         * Destructor.
         */
        virtual ~EventListener ();


        /**
         * Get the triggers of this client.
         * The client is invoked if one of the given events is occured.
         *
         * @return The triggers.
         */
        virtual event_type_t get_triggers () const = 0;


        /**
         * Get the file discriptor for this client.
         *
         * @return The file discriptor.
         */
        virtual int get_fd () const = 0;


        /**
         * Handle events.
         * 
         * @param The events which invoke this client.
         * @return false if this client should be removed, otherwise returns true.
         */
        virtual bool handle_event (event_type_t events) = 0;
        
        /**
         * Get the ID of this event listener.
         * 
         * @return The ID for this event listener.
         */
        event_listener_id_t get_id () const;


    protected:
    
        /**
         * The ID for this event listener.
         */
        event_listener_id_t id;


        /**
         * Constructor.
         */
        EventListener ();

};

};

#endif              /*SCIMCONSOLEEVENTLISTENER_H_*/
