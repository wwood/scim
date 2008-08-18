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
 * @brief This header describes about the manager class of helper processeces.
 */

#ifndef SCIM_CONSOLE_HELPER_MANAGER_H_
#define SCIM_CONSOLE_HELPER_MANAGER_H_

#define Uses_SCIM_TYPES
#include <scim.h>

#include "scim_console_common.h"
#include "scim_console_event_listener.h"

namespace scim_console
{

class ScimConsole;
class Helper;

/**
 * The manager class of helpers.
 * This also works as a servers for them.
 */
class HelperManager: public EventListener
{
    
    public:
    
    	/**
    	 * Allocate a helper manager.
    	 * 
         * @param new_scim_console The scim console.
    	 * @return A new instance of the helper manager class, otherwise it returns NULL.
    	 */
    	static HelperManager *alloc (ScimConsole *new_scim_console);
    
        /**
         * Destructor.
         */
        virtual ~HelperManager () {}
        
        /**
         * Register a helper.
         * 
         * @param helper The helper.
         * @return RETVAL_FAILS if this helper has been already registered.
         */
        virtual retval_t register_helper (Helper *helper) = 0;
        
        /**
         * Deregister a helper.
         * 
         * @param helper The helper.
         * @return RETVAL_FAILS if this helper has not yet been registered.
         */
        virtual retval_t deregister_helper (Helper *helper) = 0;
        
        /**
         * Find a helper by the uuid.
         * 
         * @param uuid The UUID for the helper.
         * @return The helper for the given UUID, or NULL if not registered.
         */ 
        virtual Helper *find_helper (const scim::String &uuid) = 0;
	
	    /**
	     * Launch a helper program.
	     * 
	     * @param helper_uuid The UUID for the helper.
	     * @return RETVAL_SUCCEEDED if it succeeded, otherwise it returns RETVAL_FAILED.
	     */
	    virtual retval_t launch_helper (const scim::String &helper_uuid) = 0;
    
    protected:
    
        /**
         * Constructor.
         */
        HelperManager () {}
    
};

};

#endif /*SCIM_CONSOLE_HELPER_MANAGER_H_*/
