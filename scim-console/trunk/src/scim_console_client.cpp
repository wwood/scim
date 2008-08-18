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
 * @brief This file describes about the implementation of the abstruct client processes.
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <iostream>

#include <pty.h>
#include <utmp.h>

#include <curses.h>
#include <term.h>

#undef cursor_up
#undef cursor_down

#include "scim_console.h"
#include "scim_console_client.h"
#include "scim_console_debug.h"
#include "scim_console_display.h"

namespace scim_console
{

/**
 * The implementation of clients.
 */
class ClientImpl: public Client
{

public:


    /**
     * Constructor.
     * 
     * @param new_scim_console The interface of SCIM Console.
     */
    ClientImpl (ScimConsole *new_scim_console);

    /**
     * Destructor.
     */
    ~ClientImpl ();

    /**
     * Initialize the client.
     * 
     * @return RETVAL_SUCCEEDED if it succeeded.
     */
    retval_t initialize ();

    void forward_key_event (const scim::KeyEvent &key_event);
    
    virtual void forward_string (const scim::String &str);

    event_type_t get_triggers () const;
    
    int get_fd () const;
    
    bool handle_event (event_type_t events);

private:

    /**
     * The command for the client process.
     */
    static const char *DEFAULT_CLIENT_COMMAND;

    /**
     * The size of the mapping table from key codes to strings.
     */
    static const size_t KEY_TO_CMD_TABLE_SIZE = 0xFFFF;

    /**
     * The mapping table from key codes to strings.
     */
    char* key_to_cmd_table[KEY_TO_CMD_TABLE_SIZE];

    /**
     * The scim console.
     */
    ScimConsole *scim_console;

    /**
     * The PID for the client.
     */
    pid_t client_pid;

    /**
     * The file discriptor for the pty.
     */
    int pty_fd;

    /**
     * The buffer for the input sequences.
     */
    char *writing_buffer;

    /**
     * The seek index in the writing buffer.
     */
    size_t writing_buffer_index;


    /**
     * The size of the writing buffer.
     */
    size_t writing_buffer_capacity;


    /**
     * The size of the contents in the buffer.
     */
    size_t writing_buffer_length;


};

/**
 * The default client process.
 */
const char *ClientImpl::DEFAULT_CLIENT_COMMAND = "/bin/bash";

/**
 * The instance of the client.
 */
static ClientImpl *client_instance = NULL;

};

using std::cerr;
using std::endl;
using std::string;
using std::vector;

using namespace scim;
using namespace scim_console;


/* Implementations */
Client *Client::alloc (ScimConsole *new_scim_console)
{
    if (client_instance == NULL) {
        ClientImpl *instance = new ClientImpl (new_scim_console);
        if (instance->initialize ()) {
            delete instance;
            return NULL;
        } else {
            client_instance = instance;
            return client_instance;
        }
    } else {
        cerr << "There is another client" << endl;
        return NULL;
    }
}


ClientImpl::ClientImpl (ScimConsole *new_scim_console): scim_console (new_scim_console), client_pid (-1), pty_fd (-1), writing_buffer (NULL), writing_buffer_index (0), writing_buffer_capacity (0), writing_buffer_length (0)
{
    writing_buffer_capacity = 100;
    writing_buffer = new char[writing_buffer_capacity];
}

ClientImpl::~ClientImpl ()
{
    if (pty_fd < 0)
        close (pty_fd);

    client_instance = NULL;
}

retval_t ClientImpl::initialize ()
{
    const Display &display = scim_console->get_display ();

    winsize window_size;
    window_size.ws_xpixel = 0;
	window_size.ws_ypixel = 0;
    window_size.ws_col = display.get_client_width ();
    window_size.ws_row = display.get_client_height ();
    pid_t retval = forkpty (&pty_fd, NULL, NULL, &window_size);

    if (retval < 0) {
        cerr << "Failed to ptyfork ()" << endl;
        return RETVAL_FAILED;
    } else if (retval == 0) {
        const char *client_command = DEFAULT_CLIENT_COMMAND;
        const char* client_args[] = {client_command, NULL};
        if (execvp (client_command, const_cast<char *const *> (client_args))) {
            cerr << "Failed to exec the client command" << endl;
            return RETVAL_FAILED;
        }
    } else {
        dout (8) << "The child process has been launched (pty = " << pty_fd << ")" << endl;
        client_pid = retval;
    }
    
    for (size_t i = 0; i < KEY_TO_CMD_TABLE_SIZE; ++i) {
        key_to_cmd_table[i] = NULL;
    }
    
    for (size_t i = 0; i < KEY_TO_CMD_TABLE_SIZE; ++i) {
        key_to_cmd_table[i] = NULL;
    }

    key_to_cmd_table[SCIM_KEY_Left] = tigetstr ("kcub1");
    if (key_to_cmd_table[SCIM_KEY_Left] == reinterpret_cast<char*> (-1))
        key_to_cmd_table[SCIM_KEY_Left] = NULL;

    key_to_cmd_table[SCIM_KEY_Down] = tigetstr ("kcud1");
    if (key_to_cmd_table[SCIM_KEY_Down] == reinterpret_cast<char*> (-1))
        key_to_cmd_table[SCIM_KEY_Down] = NULL;

    key_to_cmd_table[SCIM_KEY_Up] = tigetstr ("kcuu1");
    if (key_to_cmd_table[SCIM_KEY_Up] == reinterpret_cast<char*> (-1))
        key_to_cmd_table[SCIM_KEY_Up] = NULL;

    key_to_cmd_table[SCIM_KEY_Right] = tigetstr ("kcuf1");
    if (key_to_cmd_table[SCIM_KEY_Right] == reinterpret_cast<char*> (-1))
        key_to_cmd_table[SCIM_KEY_Right] = NULL;

    key_to_cmd_table[SCIM_KEY_Home] = tigetstr ("khome");
    if (key_to_cmd_table[SCIM_KEY_Home] == reinterpret_cast<char*> (-1))
        key_to_cmd_table[SCIM_KEY_Home] = NULL;

    key_to_cmd_table[SCIM_KEY_End] = tigetstr ("kend");
    if (key_to_cmd_table[SCIM_KEY_End] == reinterpret_cast<char*> (-1))
        key_to_cmd_table[SCIM_KEY_End] = NULL;

    key_to_cmd_table[SCIM_KEY_Return] = "\n";
    if (key_to_cmd_table[SCIM_KEY_Return] == reinterpret_cast<char*> (-1))
        key_to_cmd_table[SCIM_KEY_Return] = NULL;

    return RETVAL_SUCCEEDED;
}

void ClientImpl::forward_string (const String &str)
{
    dout (3) << "ClientImpl::forward_string ()" << endl;
    dout (3) << "The forwarding string: " << str << endl;
    if (str.empty ())
        return;

    if (writing_buffer_length + str.length () + 10 >= writing_buffer_capacity) {
        const size_t new_buffer_capacity = writing_buffer_capacity + 50;
        char *new_buffer = new char[new_buffer_capacity];

        const size_t former_length = writing_buffer_capacity - writing_buffer_index;
        const size_t latter_length = writing_buffer_length - former_length;
        memcpy (new_buffer, writing_buffer + sizeof (char) * writing_buffer_index, sizeof (char) * former_length);
        memcpy (new_buffer + sizeof (char) * former_length, writing_buffer, sizeof (char) * latter_length);
        writing_buffer_index = 0;

        buffer_capacity = new_buffer_capacity;
        delete[] writing_buffer;
        writing_buffer = new_buffer;
        writing_buffer_capacity = new_buffer_capacity;
    }

    for (size_t i = 0; i < str.length (); ++i) {
        writing_buffer[(writing_buffer_index + writing_buffer_length) % writing_buffer_capacity] = str[i];
        ++writing_buffer_length;
    }
    
    scim_console->interrupt ();
}


void ClientImpl::forward_key_event (const KeyEvent &key_event)
{
    dout (3) << "ClientImpl::forward_key_event ()" << endl;
    if (key_event.is_key_release ())
        return;
    
    char *cmd = NULL;
    if (key_event.is_control_down () && !key_event.is_shift_down () && !key_event.is_alt_down ()) {
        const unsigned char c = tolower (key_event.get_ascii_code ());
        if ('a' <= c && c <= 'z') {
            cmd = static_cast<char*> (alloca (sizeof (char) * 2));
            cmd[0] = static_cast<char> (c - 'a' + 1);
            cmd[1] = '\0';
        }
    } else if (key_event.code >= 0 && key_event.code < KEY_TO_CMD_TABLE_SIZE) {
        cmd = key_to_cmd_table[key_event.code];
    }

    if (cmd != NULL) {
        dout (1) << "Special key" << endl;
        const String str (cmd);
        forward_string (str);
    } else {
        dout (1) << "Normal key" << endl;
        char cstr[2];
        cstr[0] = key_event.get_ascii_code ();
        cstr[1] = '\0';
        forward_string (cstr);
    }
}

event_type_t ClientImpl::get_triggers () const
{
    if (writing_buffer_length > 0) {
        return EVENT_READ | EVENT_WRITE | EVENT_ERROR;
    } else {
        return EVENT_READ | EVENT_ERROR;
    }
}

int ClientImpl::get_fd () const
{
    return pty_fd;
}

bool ClientImpl::handle_event (event_type_t events)
{
    dout (6) << "ClientImpl::handle_event ()" << endl;

    if (events & EVENT_ERROR) {
        cerr << "An exception occurred at ClientImpl::handle_event ()" << endl;
        scim_console->quit (-1);
        return false;
    } else {
        if (events & EVENT_WRITE) {
            dout (6) << "Writing into the client stdin" << endl;
            size_t send_length = writing_buffer_length;
            if (writing_buffer_index + writing_buffer_length > writing_buffer_capacity)
                send_length = writing_buffer_capacity - writing_buffer_index;
            const ssize_t retval = write (pty_fd, writing_buffer + writing_buffer_index, sizeof (char) * send_length);
            if (retval < 0) {
                if (errno != EINTR && errno != EAGAIN) {
                    cerr << "Cannot write to the client stdin: " << strerror (errno) << endl;
                    scim_console->quit (-1);
                    return false;
                }
            } else {
                writing_buffer_index = (writing_buffer_index + retval / sizeof (char)) % writing_buffer_capacity;
                writing_buffer_length -= retval / sizeof (char);
            }
        }

        if (events & EVENT_READ) {
            dout (6) << "Reading from the client stdout" << endl;
            const size_t receiving_buffer_capacity = 1000;
            char receiving_buffer[receiving_buffer_capacity];
            const ssize_t retval = read (pty_fd, receiving_buffer, receiving_buffer_capacity);
            if (retval < 0) {
                if (errno != EINTR && errno != EAGAIN) {
                    cerr << "Cannot read from the client stdout: " << strerror (errno) << endl;
                    scim_console->quit (-1);
                    return false;
                }
            } else if (retval == 0) {
                cerr << "It seems like the client process has been shutdowned..." << endl;
                scim_console->quit (-1);
                return false;
            } else {
                const String str = String (receiving_buffer, retval / sizeof (char)); 
                scim_console->process_pty_output (str);
                scim_console->update_display ();
            }
        }

        return true;
    }
}

