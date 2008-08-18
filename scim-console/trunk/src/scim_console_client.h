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
 * @brief This header describes about the interface of clients.
 */

#ifndef SCIMCONSOLECLIENT_H_
#define SCIMCONSOLECLIENT_H_

#define Uses_SCIM_EVENT
#define Uses_SCIM_TYPES
#include <scim.h>

#include "scim_console_common.h"
#include "scim_console_event_listener.h"

namespace scim_console
{

class ScimConsole;

/**
 * The interface of clients.
 */
class Client: public EventListener
{

    public:


        /**
         * Allocate an client.
         *
         * @param scim_console The interface of scim_console.
         * @return A new client.
         */
        static Client *alloc (ScimConsole *scim_console);


        /**
         * Destructor.
         */
        virtual ~Client () {}


        /**
         * Forward a key event.
         *
         * @param key_event The key event.
         */
        virtual void forward_key_event (const scim::KeyEvent &key_event) = 0;
        
        
        /**
         * Forward an input string.
         * 
         * @param str The new string.
         * */
        virtual void forward_string (const scim::String &str) = 0;


    protected:


        /**
         * Constructor.
         */
        Client () {}

};

};

#endif              /*SCIMCONSOLECLIENT_H_*/
