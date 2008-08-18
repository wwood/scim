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
 * @brief This file is the implementation of Point class.
 */

#include "scim_console_point.h"


using namespace scim_console;


Point::Point (): x (0), y (0)
{
}


Point::Point (int new_x, int new_y): x (new_x), y (new_y)
{
}


Point::~Point ()
{
}


int Point::get_x () const
{
    return x;
}


void Point::set_x (int new_x)
{
    x = new_x;
}


int Point::get_y () const
{
    return y;
}


void Point::set_y (int new_y)
{
    y = new_y;
}
