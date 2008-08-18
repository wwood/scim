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
 * @brief This file describes about the implementation of the peers for the SCIM helpers.
 */

#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

#define Uses_SCIM_EVENT
#define Uses_SCIM_PROPERTY
#define Uses_SCIM_UTILITY
#include <scim.h>

#include "scim_console_helper.h"
#include "scim_console_helper_manager.h"
#include "scim_console_imcontext.h"

namespace scim_console
{

/**
 * The type for the keys of helpers.
 */
typedef unsigned int helper_key_t;

/**
 * The implementation of Helper interface.
 */
class HelperImpl: public Helper
{

public:

    /**
     * Constructor.
     * 
     * @param socket_fd The socket connected with the helper process.
     * @param new_panel The pointer for the panel.
     * @param new_scim_console The pointer for the SCIM Console.
     */
    HelperImpl (int socket_fd, ScimConsole *new_scim_console);

    /**
     * Destructor.
     */
    ~HelperImpl ();
    
    void close ();
    
    const scim::String &get_uuid () const;
    
    retval_t send_helper_event (const scim::Transaction &transaction);
    
    event_type_t get_triggers () const;

    int get_fd () const;

    bool handle_event (event_type_t events);

private:

    /**
     * The timeout millisecs for the socket.
     */
    static const int socket_timeout = 500;
    
    /**
     * The pointer for SCIM Console.
     */
    ScimConsole *scim_console;

    /**
     * The socket.
     */
    scim::Socket socket;

    /**
     * The UUID for the helper.
     */
    scim::String uuid;

    /**
     * The name of the helper.
     */
    scim::String name;

    /**
     * The path for the icon file.
     */
    scim::String icon;

    /**
     * The description for the helper.
     */
    scim::String description;

    /**
     * The option for the helper.
     */
    scim::String option;

    /**
     * The key for the helper.
     */
    helper_key_t key;

    /**
     * If the socket is closed or not.
     */
    bool closed;

    /**
     * Establish a connection with the helper.
     * 
     * @param transaction The message transaction.
     * @return RETVAL_SUCCEEDED if it succeeded.
     */
    retval_t accept_connection (scim::Transaction &transaction);

    /**
     * Reload the configurations.
     * 
     * @param transaction The message transaction.
     * @return RETVAL_SUCCEEDED if it succeeded.
     */
    retval_t reload_config (scim::Transaction &transaction);

    /**
     * Forward a key event.
     * 
     * @param transaction The message transaction.
     * @return RETVAL_SUCCEEDED if it succeeded.
     */
    retval_t forward_key_event (scim::Transaction &transaction);

    /**
     * Process a key event.
     * 
     * @param transaction The message transaction.
     * @return RETVAL_SUCCEEDED if it succeeded.
     */
    retval_t process_key_event (scim::Transaction &transaction);
    
    /**
     * Send messages for the specific IMEngine.
     * 
     * @param transaction The message transaction.
     * @return RETVAL_SUCCEEDED if it succceeded.
     */
    retval_t send_imengine_event (scim::Transaction &transaction);

	/**
	 * Register properties.
	 * 
	 * @param transaction The message transaction.
	 * @return RETVAL_SUCCEEDED if it succeeded.
	 */
	retval_t register_properties (scim::Transaction &transaction);
    
    /**
     * Update property.
     * 
     * @param transaction The message transaction.
     * @return RETVAL_SUCCEEDED if it succeeded.
     */
    retval_t update_property (scim::Transaction &transaction);

    /**
     * Register this helper.
     *
     * @param transaction The message transaction.
     * @return RETVAL_SUCCEEDED if it succeeded.
     */
    retval_t register_helper (scim::Transaction &transaction);

    /**
     * Commit a string.
     *
     * @param transaction The message transaction.
     * @return RETVAL_SUCCEEDED if it succeeded.
     */
    retval_t commit_string (scim::Transaction &transaction);

};

/**
 * The manager of the helpers.
 */
static HelperManager *helper_manager = NULL;

};


using std::cerr;
using std::endl;

using scim::KeyEvent;
using scim::Property;
using scim::PropertyList;
using scim::String;
using scim::Transaction;
using scim::WideString;

using scim::utf8_wcstombs;

using scim::SCIM_TRANS_CMD_COMMIT_STRING;
using scim::SCIM_TRANS_CMD_EXIT;
using scim::SCIM_TRANS_CMD_FAIL;
using scim::SCIM_TRANS_CMD_FORWARD_KEY_EVENT;
using scim::SCIM_TRANS_CMD_HELPER_ATTACH_INPUT_CONTEXT;
using scim::SCIM_TRANS_CMD_HELPER_PROCESS_IMENGINE_EVENT;
using scim::SCIM_TRANS_CMD_OK;
using scim::SCIM_TRANS_CMD_OPEN_CONNECTION;
using scim::SCIM_TRANS_CMD_PANEL_SEND_IMENGINE_EVENT;
using scim::SCIM_TRANS_CMD_PANEL_SEND_KEY_EVENT;
using scim::SCIM_TRANS_CMD_PROCESS_KEY_EVENT;
using scim::SCIM_TRANS_CMD_PANEL_REGISTER_HELPER;
using scim::SCIM_TRANS_CMD_RELOAD_CONFIG;
using scim::SCIM_TRANS_CMD_REGISTER_PROPERTIES;
using scim::SCIM_TRANS_CMD_REPLY;
using scim::SCIM_TRANS_CMD_REQUEST;
using scim::SCIM_TRANS_CMD_UPDATE_PROPERTY;

using namespace scim_console;

void Helper::static_initialize (HelperManager *new_helper_manager)
{
    assert (helper_manager == NULL);
    helper_manager = new_helper_manager;
}

void Helper::static_finalize ()
{
    helper_manager = NULL;
}

Helper *Helper::alloc (int socket_fd, ScimConsole *scim_console)
{
    HelperImpl *helper = new HelperImpl (socket_fd, scim_console);
    return helper;
}

HelperImpl::HelperImpl (int socket_fd, ScimConsole *new_scim_console): scim_console (new_scim_console), socket (socket_fd), key (0), closed (false)
{
}

HelperImpl::~HelperImpl ()
{
    close ();
    if (!uuid.empty ())
        helper_manager->deregister_helper (this);
}

void HelperImpl::close ()
{
    if (!closed)
        ::close (socket.get_id ());
    closed = true;
}

int HelperImpl::get_fd () const
{
    if (!closed) {
        return socket.get_id ();
    } else {
        return -1;
    }
}
    
event_type_t HelperImpl::get_triggers () const
{
    return EVENT_READ | EVENT_ERROR;
}

const String &HelperImpl::get_uuid () const
{
    return uuid;
}

retval_t HelperImpl::send_helper_event (const Transaction &transaction)
{
    if (!uuid.empty ()) {
    	Transaction sending_transaction;
        sending_transaction.put_command (SCIM_TRANS_CMD_REPLY);
 
 		IMContext *imcontext = IMContext::get_focused_imcontext ();
 	   	sending_transaction.put_data (imcontext->get_id ());
    	sending_transaction.put_data (imcontext->get_imengine_uuid ());
    	sending_transaction.put_command (SCIM_TRANS_CMD_HELPER_PROCESS_IMENGINE_EVENT);
    	sending_transaction.put_data (transaction);
    	sending_transaction.write_to_socket (socket);
    	return RETVAL_SUCCEEDED;
    } else {
    	cerr << "The connection has not been established yet" << endl;
    	return RETVAL_FAILED;
    }
}

bool HelperImpl::handle_event (event_type_t events)
{
    if (!socket.valid () || closed)
        return false;

    Transaction transaction;
    if (transaction.read_from_socket (socket, socket_timeout)) {

        if (uuid.empty ()) {
            if (accept_connection (transaction)) {
                cerr << "Failed to establish the connection. Exitting..." << endl;
                close ();
                return false;
            }
        } else {
            int command;
            helper_key_t helper_key;
            while (transaction.get_command (command) && transaction.get_data (helper_key)) {
                if (command != SCIM_TRANS_CMD_REQUEST) {
                    cerr << "A protocol error has been occured" << endl;
                    close ();
                    return false;
                } else if (helper_key != key) {
                    cerr << "The key for the helper is mismatched" << endl;
                    close ();
                    return false;
                }

                switch (command) {
                case SCIM_TRANS_CMD_RELOAD_CONFIG:
                    if (reload_config (transaction))
                        return false;
                    break;
                case SCIM_TRANS_CMD_EXIT:
                    // Do nothing.
                    break;
                case SCIM_TRANS_CMD_PANEL_REGISTER_HELPER:
                    if (register_helper (transaction))
                        return false;
                    break;
                case SCIM_TRANS_CMD_COMMIT_STRING:
                    if (commit_string (transaction))
                        return false;
                    break;
                case SCIM_TRANS_CMD_PROCESS_KEY_EVENT:
                case SCIM_TRANS_CMD_PANEL_SEND_KEY_EVENT:
                    if (process_key_event (transaction))
                        return false;
                    break;
                case SCIM_TRANS_CMD_FORWARD_KEY_EVENT:
                    if (forward_key_event (transaction))
                        return false;
                    break;
                case SCIM_TRANS_CMD_PANEL_SEND_IMENGINE_EVENT:
                    if (send_imengine_event (transaction))
                        return false;
                    break;
                case SCIM_TRANS_CMD_REGISTER_PROPERTIES:
                    if (register_properties (transaction))
                        return false;
                    break;
                case SCIM_TRANS_CMD_UPDATE_PROPERTY:
                    if (update_property (transaction))
                        return false;
                    break;
                default:
                    cerr << "Unknown message has been received" << endl;
                    break;
                }
            }
        }

        return true;
    } else {
        const int error_number = socket.get_error_number ();
        cerr << "Failed to fetch a message from a helper: " << strerror (error_number) << endl;
        return false;
    }
}

retval_t HelperImpl::accept_connection (Transaction &transaction)
{
    static helper_key_t last_key = 0;

    int command;
    String version;
    String client_type;
    if (transaction.get_command (command)  && command == SCIM_TRANS_CMD_REQUEST &&
            transaction.get_command (command)  && command == SCIM_TRANS_CMD_OPEN_CONNECTION &&
            transaction.get_data (version) && version == String (SCIM_BINARY_VERSION) &&
            transaction.get_data (client_type) && (client_type == "Helper" || client_type == "ConnectionTester")) {

        const String server_types = "Console";
        key = last_key;
        transaction.clear ();
        transaction.put_command (SCIM_TRANS_CMD_REPLY);
        transaction.put_data (server_types);
        transaction.put_data (key);
        ++last_key;

        if (transaction.write_to_socket (socket) &&
                transaction.read_from_socket (socket, socket_timeout) &&
                transaction.get_command (command) && command == SCIM_TRANS_CMD_REPLY &&
                transaction.get_command (command) && command == SCIM_TRANS_CMD_OK) {

            // Client is ok, return the client type.
            return RETVAL_SUCCEEDED;
        }
    }

    return RETVAL_FAILED;
}

retval_t HelperImpl::register_helper (Transaction &transaction)
{
    if (uuid.size () > 0) {
        Transaction sending_transaction;
        sending_transaction.put_command (SCIM_TRANS_CMD_REPLY);
        sending_transaction.put_command (SCIM_TRANS_CMD_FAIL);
        sending_transaction.write_to_socket (socket);
        close ();
        return RETVAL_FAILED;
    }

    String version, client_type;
    if (transaction.get_data (uuid) && transaction.get_data (name) && transaction.get_data (icon)
            && transaction.get_data (description) && transaction.get_data (option)) {

        Transaction sending_transaction;
        sending_transaction.put_command (SCIM_TRANS_CMD_REPLY);

        if (uuid.empty () || helper_manager->register_helper (this)) {
            sending_transaction.put_command (SCIM_TRANS_CMD_FAIL);
            sending_transaction.write_to_socket (socket);
            close ();
            return RETVAL_FAILED;
        } else {
            sending_transaction.put_command (SCIM_TRANS_CMD_OK);

            sending_transaction.put_command (SCIM_TRANS_CMD_HELPER_ATTACH_INPUT_CONTEXT);
            sending_transaction.put_data (IMContext::get_focused_imcontext_id ());

            sending_transaction.write_to_socket (socket);
            return RETVAL_SUCCEEDED;
        }
    } else {
        cerr << "A protocol error has been occured" << endl;

        Transaction sending_transaction;
        sending_transaction.put_command (SCIM_TRANS_CMD_REPLY);
        sending_transaction.put_command (SCIM_TRANS_CMD_FAIL);
        sending_transaction.write_to_socket (socket);

        close ();
        return RETVAL_FAILED;
    }
}

retval_t HelperImpl::reload_config (Transaction &transaction)
{
    scim_console->load_config ();
    return RETVAL_SUCCEEDED;
}

retval_t HelperImpl::commit_string (Transaction &transaction)
{
    imcontext_id_t imcontext_id;
    String target_uuid;
    WideString commit_wstr;

    if (transaction.get_data (imcontext_id) && transaction.get_data (target_uuid) && transaction.get_data (commit_wstr)) {
        if (IMContext::get_focused_imcontext_id () == imcontext_id && !commit_wstr.empty ()) {
            String commit_str = utf8_wcstombs (commit_wstr);
            scim_console->forward_pty_input (commit_str);
            return RETVAL_SUCCEEDED;
        } else {
            cerr << "A protocol error has been occured" << endl;

            return RETVAL_FAILED;
        }
    } else {
        cerr << "A protocol error has been occured" << endl;

        close ();
        return RETVAL_FAILED;
    }
}

retval_t HelperImpl::process_key_event (Transaction &transaction)
{
    imcontext_id_t imcontext_id;
    String target_uuid;
    KeyEvent key_event;

    if (transaction.get_data (imcontext_id) && transaction.get_data (target_uuid) && transaction.get_data (key_event)) {
        if (IMContext::get_focused_imcontext_id () != imcontext_id) {
            return RETVAL_SUCCEEDED;
        } else {
            scim_console->process_key_event (key_event);
            return RETVAL_SUCCEEDED;
        }
    } else {
        cerr << "A protocol error has been occured" << endl;

        close ();
        return RETVAL_FAILED;
    }
}

retval_t HelperImpl::forward_key_event (Transaction &transaction)
{
    imcontext_id_t imcontext_id;
    String target_uuid;
    KeyEvent key_event;

    if (transaction.get_data (imcontext_id) && transaction.get_data (target_uuid) && transaction.get_data (key_event)) {
        if (IMContext::get_focused_imcontext_id () != imcontext_id) {
            return RETVAL_SUCCEEDED;
        } else {
            scim_console->forward_key_event (key_event);
            return RETVAL_SUCCEEDED;
        }
    } else {
        cerr << "A protocol error has been occured" << endl;

        close ();
        return RETVAL_FAILED;
    }
}

retval_t HelperImpl::send_imengine_event (Transaction &transaction)
{
    imcontext_id_t imcontext_id;
    String target_uuid;
    Transaction nest_transaction;
    
    if (transaction.get_data (imcontext_id) && transaction.get_data (target_uuid) &&
            transaction.get_data (nest_transaction) && nest_transaction.valid ()) {
		
		IMContext *focused_imcontext = IMContext::get_focused_imcontext ();
		if (focused_imcontext->get_id ()) focused_imcontext->process_helper_event (target_uuid, uuid, nest_transaction);
        
        return RETVAL_SUCCEEDED;
    }
    
    return RETVAL_FAILED;
}

retval_t HelperImpl::register_properties (Transaction &transaction)
{
    PropertyList properties;
    if (transaction.get_data (properties)) {
        scim_console->register_helper_properties (properties);
        return RETVAL_SUCCEEDED;
    } else {
        return RETVAL_FAILED;
    }
}

retval_t HelperImpl::update_property (Transaction &transaction)
{
    cerr << "HelperImpl::update_property ()" << endl;
    
    Property property;
    if (transaction.get_data (property)) {
        scim_console->update_helper_property (property);
        return RETVAL_SUCCEEDED;
    } else {
        return RETVAL_FAILED;
    }
}

