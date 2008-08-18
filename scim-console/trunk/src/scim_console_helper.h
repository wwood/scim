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
 * @brief This header describes about the interface of helper processeces of SCIM.
 */

#ifndef SCIM_CONSOLE_HELPER_H_
#define SCIM_CONSOLE_HELPER_H_

#define Uses_SCIM_TYPES
#define Uses_SCIM_TRANSACTION
#include <scim.h>

#include "scim_console_common.h"
#include "scim_console_event_listener.h"

namespace scim_console
{

class Display;
class Helper;
class HelperManager;
class ScimConsole;

/**
 * The interface of helpers.
 */
class Helper: public EventListener
{

public:

    /**
     * Initialize the class itself.
     * 
     * @param helper_manager The helper manager.
     */
    static void static_initialize (HelperManager *helper_manager);

    /**
     * Finalize the class itself.
     */
    static void static_finalize ();

    /**
     * Allocate a helper.
     * 
     * @param socket_fd The fd for the socket connected with the helper process.
     * @param scim_console The pointer for the SCIM Console.
     * @return The new heler or NULL if failed.
     */
    static Helper *alloc (int socket_fd, ScimConsole *scim_console);
    
    /**
     * Destructor.
     */
    virtual ~Helper () {}
    
    /**
     * Close the connection with the helper process.
     */
    virtual void close () = 0;
    
    /**
     * Get the UUID for the process.
     * 
     * @return The UUID for the helper.
     */
    virtual const scim::String &get_uuid () const = 0;

    /**
     * Send an event for the helper program.
     * 
     * @param transaction The event transaction.
     * @return RETVAL_SUCCEEDED if it succeeded, otherwise it returns RETVAL_FAILED.
     */
    virtual retval_t send_helper_event (const scim::Transaction &transaction) = 0;

protected:

    /**
     * The hidden constructor.
     */
    Helper () {}

};

};

#endif /*SCIM_CONSOLE_HELPER_H_*/
