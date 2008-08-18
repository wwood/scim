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
 * @brief This header describes about the implementation of helper managers.
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>

#include <map>

#define Uses_SCIM_GLOBAL_CONFIG
#define Uses_SCIM_HELPER
#define Uses_SCIM_HELPER_MANAGER
#define Uses_SCIM_SOCKET
#include <scim.h>

#include "scim_console.h"
#include "scim_console_debug.h"
#include "scim_console_display.h"
#include "scim_console_helper.h"
#include "scim_console_helper_manager.h"

namespace scim_console
{

class HelperManagerImpl: public HelperManager
{

    public:
    
        /**
         * Constructor.
         *
         * @param new_scim_console The scim console.
         */
        HelperManagerImpl (ScimConsole *new_scim_console);
        
        /**
         * Destructor.
         */
        ~HelperManagerImpl ();
        
        /**
         * Initialize.
         * 
         * @return RETVAL_SUCCEEDED if it succeeded.
         */
        retval_t initialize ();
        
        retval_t register_helper (Helper *helper);
        
        retval_t deregister_helper (Helper *helper);
        
        Helper *find_helper (const scim::String &uuid);
        
        event_type_t get_triggers () const;
        
        int get_fd () const;
        
        bool handle_event (event_type_t events);
    
		retval_t launch_helper (const scim::String &helper_uuid);
        
    private:

        /**
         * The scim console.
         */
        ScimConsole *scim_console;
    
        /**
         * The map of registered helpers.
         */
        std::map<scim::String, Helper*> helpers;
        
        /**
         * The server socket.
         */
        int socket_fd;
        
        /**
         * The scim frontend for the helper manager.
         */
        scim::HelperManager scim_helper_manager;

};

static HelperManagerImpl *helper_manager_instance = NULL;

};

using std::cerr;
using std::cout;
using std::endl;
using std::map;
using std::pair;

using scim::Socket;
using scim::SocketAddress;
using scim::String;
using scim::Transaction;

using scim::SCIM_SOCKET_LOCAL;

using scim::scim_get_default_panel_socket_address;

using namespace scim_console;

HelperManagerImpl::HelperManagerImpl (ScimConsole *new_scim_console): scim_console (new_scim_console), socket_fd (-1)
{
}

HelperManagerImpl::~HelperManagerImpl ()
{
    if (helper_manager_instance != NULL) {
        Helper::static_finalize ();
        helper_manager_instance = NULL;
    }
    
    if (socket_fd >= 0) close (socket_fd);
}

HelperManager *HelperManager::alloc (ScimConsole *new_scim_console)
{
    if (helper_manager_instance == NULL) {
        HelperManagerImpl *instance = new HelperManagerImpl (new_scim_console);
        if (instance->initialize ()) {
            delete instance;
            return NULL;
        } else {
            helper_manager_instance = instance;
            Helper::static_initialize (helper_manager_instance);
            return helper_manager_instance;
        }
    } else {
        return NULL;
    }
}

retval_t HelperManagerImpl::initialize ()
{
    SocketAddress socket_address = SocketAddress (scim_get_default_panel_socket_address (scim_console->get_display ().get_name ()));
    if (socket_address.get_family () != SCIM_SOCKET_LOCAL) {
        cerr << "The socket family must be 'LOCAL'!" << endl;
        return RETVAL_FAILED;
    }

    const struct sockaddr_un *unix_socket_address = reinterpret_cast<const struct sockaddr_un*> (socket_address.get_data ());
    const socklen_t unix_socket_address_length = socket_address.get_data_length ();
    const char *socket_path = unix_socket_address->sun_path;

    socket_fd = socket (PF_UNIX, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        cerr << "Cannot create an unix domain socket: " << strerror (errno) << endl;
        return RETVAL_FAILED;
    }
    
    unlink (socket_path);
    dout (5) << "Creating a socket on '" << socket_path << "'..." << endl;
    if (bind (socket_fd, reinterpret_cast <const struct sockaddr*> (unix_socket_address), unix_socket_address_length)) {
        cerr << "Cannot bind the socket: " << strerror (errno) << endl;
        close (socket_fd);
        socket_fd = -1;
        return RETVAL_FAILED;
    }
    
    if (listen (socket_fd, 64)) {
        cerr << "Cannot start listening the socket: " << strerror (errno) << endl;
        close (socket_fd);
        socket_fd = -1;
        return RETVAL_FAILED;
    }
    return RETVAL_SUCCEEDED;
}

retval_t HelperManagerImpl::register_helper (Helper *helper)
{
    if (helper->get_uuid ().empty ()) {
        return RETVAL_FAILED;
    } else {
        const map<String, Helper*>::iterator i = helpers.find (helper->get_uuid ());
        if (i != helpers.end ()) {
            return RETVAL_FAILED;
        } else {
            helpers.insert(pair<String, Helper*> (helper->get_uuid (), helper));
        }
    }
}

retval_t HelperManagerImpl::deregister_helper (Helper *helper)
{
    if (helper->get_uuid ().empty ()) {
        return RETVAL_FAILED;
    } else if (helpers.erase (helper->get_uuid ()) == 0) {
        return RETVAL_FAILED;
    } else {
        return RETVAL_SUCCEEDED;
    }
}

Helper *HelperManagerImpl::find_helper (const String &uuid)
{
    map<String, Helper*>::iterator i = helpers.find (uuid);
    if (i == helpers.end ()) {
        return NULL;
    } else {
        return i->second;
    }
}

retval_t HelperManagerImpl::launch_helper (const String &helper_uuid)
{
	Helper *helper = find_helper (helper_uuid);
	if (helper != NULL) {
		cerr << "The helper already exists: " << helper_uuid << endl;
		return RETVAL_FAILED;
	} else {
        scim_helper_manager.run_helper (helper_uuid, scim_console->get_scim_config ()->get_name (), scim_console->get_display ().get_name ());
    }
}

event_type_t HelperManagerImpl::get_triggers () const
{
    return EVENT_ERROR | EVENT_READ;
}

int HelperManagerImpl::get_fd () const
{
    return socket_fd;
}

bool HelperManagerImpl::handle_event (event_type_t triggers)
{
	if (triggers & EVENT_ERROR) {
		cerr << "A socket exception at HelperManagerImpl::handle_event ()" << endl;
        scim_console->quit (-1);
		return false;
	} else {
        assert (triggers & EVENT_READ);
        struct sockaddr_un empty_socket_addr;
        socklen_t emtpy_socket_size = sizeof (empty_socket_addr);
        
		const int fd = accept (socket_fd, (sockaddr*) (&emtpy_socket_size), &emtpy_socket_size);
		if (fd < 0) {
			cerr << "Failed to accept a helper" << endl;
			return true;
		} else {
			Helper *helper = Helper::alloc (fd, scim_console);
			if (helper != NULL) {
				scim_console->accept_helper (helper);
			}
		}
	}
}
