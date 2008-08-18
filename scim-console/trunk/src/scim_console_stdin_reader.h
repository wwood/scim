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
 * @brief This file describes about the public interface of the standard input readers.
 */
 
#ifndef SCIM_CONSOLE_STDIN_READER_H_
#define SCIM_CONSOLE_STDIN_READER_H_

#include "scim_console.h"
#include "scim_console_common.h"
#include "scim_console_event_listener.h"

namespace scim_console
{

/**
 * The public interface for stdin readers.
 */
class StdinReader: public EventListener
{

public:

    /**
     * Allocate a new stdin reader.
     * 
     * @param scim_console The pointer for the SCIM Console.
     * @return The new stdin reader.
     */
    static StdinReader *alloc (ScimConsole *scim_console);

    /**
     * Destructor.
     */
    virtual ~StdinReader ()
    {}

protected:

    /**
     * Constructor.
     */
    StdinReader ()
    {}

}
;

};

#endif /*SCIM_CONSOLE_STDIN_READER_H_*/
