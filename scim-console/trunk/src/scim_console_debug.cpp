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
 * @brief This source describes about implementations of the debug interface.
 */

#include <stdlib.h>

#include <sstream>

#include "scim_console_debug.h"

namespace scim_console {

/**
 * The dummy output stream.
 */
static std::stringstream ss;

/**
 * The debug mask.
 */
static debug_mask_t debug_mask = DEBUG_ALL;

/**
 * The debug level.
 */
static debug_level_t debug_level = 0;

/**
 * Initialized or not.
 */
static bool initialized = false;

/* Implementations */
using namespace std;

/**
 * Initialize the debug miscs.
 */
static void initialize ()
{
    const char *debug_level_str = getenv ("SCIM_CONSOLE_DEBUG_LEVEL");
    if (debug_level_str != NULL) {
        const int new_value = atoi (debug_level_str);
        if (new_value >= 0 && new_value <= 10)
            debug_level = new_value;
    }

    initialized = true;
}

debug_mask_t get_debug_mask ()
{
    if (!initialized)
        initialize ();
    return debug_mask;
}

debug_level_t get_debug_level ()
{
    if (!initialized)
        initialize ();
    return debug_level;
}

ostream &dout (debug_level_t level) {
    return dout (DEBUG_ALL, level);
}

ostream &dout (debug_mask_t mask, debug_level_t level)
{
    ss.str ("");
    if (level + get_debug_level () >= 10 && (mask & get_debug_mask ())) {
        return cerr;
    } else {
        return ss;
    }
}

};


