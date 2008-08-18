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
 * @brief This header describes about the common typedefs and constants for scim console.
 */

#ifndef SCIMCONSOLECOMMON_H_
#define SCIMCONSOLECOMMON_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define Uses_SCIM_TYPES
#include <scim.h>

#define _(string) (string)

/**
 * The type for IMContext IDs.
 */
typedef scim::uint32 imcontext_id_t;

/**
 * The type for the retvals from the funcitons.
 */
typedef int retval_t;

/**
 * The type for the transaction commands.
 */
typedef int transaction_command_t;

/**
 * The retval of failtures.
 */
static const retval_t RETVAL_FAILED = -1;

/**
 * The retval of success.
 */
static const retval_t RETVAL_SUCCEEDED = 0;

#ifndef SCIM_LIBEXEC_DIR
/**
 * The default libexecdir for scim.
 */
#define SCIM_LIBEXEC_DIR "/usr/lib/scim-1.0"
#endif

#ifndef SCIM_HELPER_MANAGER_PROGRAM
/**
 * The default helper manager.
 */
#define SCIM_HELPER_MANAGER_PROGRAM SCIM_LIBEXEC_DIR "/scim-helper-manager"
#endif

#ifndef SCIM_DATA_DIR
/**
 * The default data dir for scim.
 */
#define SCIM_DATA_DIR "/usr/share/scim"
#endif

#ifndef SCIM_KEYBOARD_ICON_FILE
/**
 * The default keyboard icon file.
 */
#define SCIM_KEYBOARD_ICON_FILE SCIM_DATA_DIR "/icons/keyboard.png"
#endif
#endif /*SCIMCONSOLECOMMON*/
