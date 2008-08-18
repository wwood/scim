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
 * @brief This file describes about the implementation of the interruption listeners.
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/times.h>
#include <sys/types.h>

#define Uses_SCIM_DEBUG
#include <scim.h>

#include "scim_console_interruption_listener.h"

namespace scim_console
{

/**
 * The class definition of interruption listeners.
 */
class InterruptionListenerImpl: public InterruptionListener
{

public:

    /**
     * The constructor.
     */
    InterruptionListenerImpl ();

    /**
     * The destructor.
     */
    ~InterruptionListenerImpl ();

    retval_t initialize ();

    event_type_t get_triggers () const;

    int get_fd () const;

    bool handle_event (event_type_t events);

    bool has_interruption () const;

    void clear_interruption ();

    timeval get_remaining_time () const;

    void interrupt (const timeval &time);

private:

    /**
     * The input file descriptor for the socket.
     */
    int socket_in;

    /**
     * The output file descriptor for the socket.
     */
    int socket_out;

    /**
     * The time of the next interruption.
     */
    timeval interruption_time;

    /**
     * If there is a pending interruption.
     */
    bool interrupted;

};

/**
 * The instance of the interruption listener.
 */
static InterruptionListenerImpl *instance = NULL;

};

/* Implementations */
using std::cerr;
using std::endl;

using namespace scim;
using namespace scim_console;

InterruptionListener *InterruptionListener::alloc ()
{
    if (instance != NULL) {
        return NULL;
    } else {
        InterruptionListenerImpl *new_instance = new InterruptionListenerImpl ();
        if (new_instance->initialize ()) {
            delete new_instance;
            return NULL;
        } else {
            instance = new_instance;
            return instance;
        }
    }
}

InterruptionListenerImpl::InterruptionListenerImpl (): socket_in (-1), socket_out (-1), interrupted (false)
{
    gettimeofday (&interruption_time, NULL);
}

InterruptionListenerImpl::~InterruptionListenerImpl ()
{
    if (socket_in >= 0) {
        close (socket_in);
        socket_in = -1;
    }
    if (socket_out >= 0) {
        close (socket_out);
        socket_out = -1;
    }
}

retval_t InterruptionListenerImpl::initialize ()
{
    int socket_pair[2];

    if (socketpair (PF_UNIX, SOCK_STREAM, 0, socket_pair)) {
        cerr << "Cannot make a pipe for a interruption heandler: " << strerror (errno) << endl;
        return RETVAL_FAILED;
    }

    socket_out = socket_pair[0];
    socket_in = socket_pair[1];

    return RETVAL_SUCCEEDED;
}

int InterruptionListenerImpl::get_fd () const
{
    return socket_in;
}

event_type_t InterruptionListenerImpl::get_triggers () const
{
    return EVENT_READ;
}

bool InterruptionListenerImpl::has_interruption () const
{
    return interrupted;
}

timeval InterruptionListenerImpl::get_remaining_time () const
{
    long dt = 0;

    if (interrupted) {
        timeval current_time;
        gettimeofday (&current_time, NULL);

        const long dt = (interruption_time.tv_sec - current_time.tv_sec) * 1000000 + (interruption_time.tv_usec - current_time.tv_usec);
    }
    if (dt < 0)
        dt = 0;
    
    timeval remaining_time;
    remaining_time.tv_sec = dt / 1000000;
    remaining_time.tv_sec = dt % 1000000;

    return remaining_time;
}

void InterruptionListenerImpl::interrupt (const timeval &time)
{
    if (interrupted) {
        timeval current_time;
        gettimeofday (&current_time, NULL);
        
        const long dt = (interruption_time.tv_sec - current_time.tv_sec) * 1000000 + (interruption_time.tv_usec - current_time.tv_usec);

        if (dt < time.tv_sec * 1000000 + time.tv_usec)
            return;
    }

    gettimeofday (&interruption_time, NULL);
    interruption_time.tv_sec += time.tv_sec;
    interruption_time.tv_usec += time.tv_usec;

    interrupted = true;
    if (send (socket_out, " ", sizeof (char), MSG_NOSIGNAL) < 0) {
        cerr << "Failed to make an interruption: " << strerror (errno) << endl;
    }
}


void InterruptionListenerImpl::clear_interruption ()
{
    char c;
    recv (socket_in, &c, sizeof (char), MSG_DONTWAIT);
    interrupted = false;
}


bool InterruptionListenerImpl::handle_event (event_type_t events)
{
    return true;
}
