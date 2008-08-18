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
 * @brief This header describes about the debug interface of scim console.
 */

#ifndef SCIMCONSOLEDEBUG_H_
#define SCIMCONSOLEDEBUG_H_

#include <ostream>

#include "scim_console_common.h"

namespace scim_console
{

/**
 * The type of the type of debug messages.
 */
typedef unsigned int debug_mask_t;

/**
 * The type of the verbosity of debug messages.
 */
typedef unsigned int debug_level_t;

/**
 * The debug mask for all types.
 */
static const debug_mask_t DEBUG_ALL = 0xF;

/**
 * The debug mask for all types.
 */
static const debug_mask_t DEBUG_NONE = 0x0;

/**
 * Get the debug mask.
 *
 * @return The debug mask.
 */
debug_mask_t get_debug_mask ();

/**
 * Get the debug level.
 *
 * @return The debug level.
 */
debug_level_t get_debug_level ();

/**
 * Returns the debug output stream.
 *
 * @param level The verbosity of the debug message.
 * @return The debug output stream.
 */
std::ostream &dout (debug_level_t level);

/**
 * Returns the debug output stream.
 *
 * @param mask The mask of the debug message.
 * @param level The verbosity of the debug message.
 * @return The debug output stream.
 */
std::ostream &dout (debug_mask_t mask, debug_level_t level);

};

#endif                                            /*SCIMCONSOLEDEBUG_H_*/
