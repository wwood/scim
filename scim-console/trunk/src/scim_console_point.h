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
 * @brief This header describes about the point class.
 */

#ifndef SCIMCONSOLEPOINT_H_
#define SCIMCONSOLEPOINT_H_

#include "scim_console_common.h"

namespace scim_console
{

/**
 * The class of the points.
 */
class Point
{


    public:


        /**
         * Constructor.
         */
        Point ();


        /**
         * Constructor.
         *
         * @param new_x The X location of the point.
         * @param new_y The Y location of the point. 
         */
        Point (int new_x, int new_y);


        /**
         * Destructor.
         */
        ~Point ();


        /**
         * Get the X location of the point.
         *
         * @return The X location of the point.
         */
        int get_x () const;


        /**
         * Set the X location of the point.
         *
         * @param new_x The X location of the point.
         */
        void set_x (int new_x);


        /**
         * Get the Y location of the point.
         *
         * @return The Y location of the point.
         */
        int get_y () const;


        /**
         * Set the Y location of the point.
         *
         * @param new_y The Y location of the point.
         */
        void set_y (int new_y);
    

    private:


        /**
         * The X location of the point.
         */
        int x;


        /**
         * The Y location of the point.
         */
        int y;

};

};


#endif              /*SCIMCONSOLEPOINT_H_*/
