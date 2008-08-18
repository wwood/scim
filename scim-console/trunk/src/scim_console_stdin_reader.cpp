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
 * @brief This file describes about the implementation of the standard input readers.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <iostream>

#include "scim_console.h"
#include "scim_console_stdin_reader.h"

namespace scim_console
{

/**
 * This is a implementation for the stdin reader interface.
 */
class StdinReaderImpl: public StdinReader
{

public:

    /**
     * Constructor.
     * 
     * @param new_scim_console The pointer for the SCIM Console.
     */
    StdinReaderImpl (ScimConsole *new_scim_console);

    /**
     * Destructor.
     */
    ~StdinReaderImpl ();

    event_type_t get_triggers () const;

    int get_fd () const;

    bool handle_event (event_type_t events);

private:


    /**
     * The pointer for the Scim Console.
     */
    ScimConsole *scim_console;

};

/**
 * The instance for stdin reader.
 */
StdinReaderImpl *instance;

};

using std::cerr;
using std::endl;

using namespace scim_console;


StdinReader *StdinReader::alloc (ScimConsole *new_scim_console)
{
    if (instance == NULL) {
        instance = new StdinReaderImpl (new_scim_console);
        return instance;
    } else {
        return NULL;
    }
}


StdinReaderImpl::StdinReaderImpl (ScimConsole *new_scim_console): scim_console (new_scim_console)
{}


StdinReaderImpl::~StdinReaderImpl ()
{
    instance = NULL;
}


event_type_t StdinReaderImpl::get_triggers () const
{
    return EVENT_READ | EVENT_ERROR;
}

int StdinReaderImpl::get_fd () const
{
    return STDIN_FILENO;
}

bool StdinReaderImpl::handle_event (event_type_t events)
{
    if (events & EVENT_ERROR) {
        cerr << "The standard input stream has been closed" << endl;
        scim_console->abort ();

        return false;
    } else {
        const size_t buffer_capacity = 100;
        char buffer[buffer_capacity];
        const ssize_t read_size = read (STDIN_FILENO, buffer, sizeof (char) * buffer_capacity);
        if (read_size < 0) {
            if (errno != EINTR) {
                cerr << "Cannot read from the stdin: " << strerror (errno) << endl;
                scim_console->abort ();
                return false;
            }
        } else if (read_size == 0) {
            cerr << "The stdin was closed" << endl;
            scim_console->abort ();
            return false;
        } else {
            // Do nothing.
        }
        
        return true;
    }
}
