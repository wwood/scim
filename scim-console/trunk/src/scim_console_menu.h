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
 * @brief This file describes about the interface of menus.
 */

#ifndef SCIM_CONSOLE_MENU_H_
#define SCIM_CONSOLE_MENU_H_

#include <vector>

#include <scim.h>

namespace scim_console
{

class MenuItem
{

public:

    /**
     * Destructor.
     */
    virtual ~MenuItem ();

    /**
     * Get the num of children of this node.
     * 
     * @return The num of children.
     */
    size_t get_child_count () const;

    /**
     * Get the specific child item.
     * 
     * @param index The index of the child item.
     * @return The child item.
     */
    const MenuItem &get_child (size_t index) const;

    /**
     * Get the specific child item.
     * 
     * @param index The index of the child item.
     * @return The child item.
     */
    MenuItem &get_child (size_t index);

    /**
     * Add a child item.
     * 
     * @param new_key The key of the child.
     * @param new_label The label of the child.
     */
    void add_child (const scim::String &new_key, const scim::String &new_label);

    /**
     * Clear all the children.
     */
    void clear_all_children ();

    /**
     * Get the index of the selected child item.
     * It returns -1 if nothing is selected.
     * 
     * @return The index of the selected child item.
     */
    ssize_t get_selected_index () const;
    
    /**
     * Select or deselect a child item.
     * Give -1 to deselect all.
     * 
     * @param new_index The index of the new item to select.
     */
    void set_selected_index (ssize_t new_index);
    
    /**
     * Get the label of the menu item.
     * 
     * @return The label.
     */
    const scim::String &get_label () const;
    
    /**
     * Set the label of the menu item.
     * 
     * @param new_label The label of the menu item.
     */
    void set_label (const scim::String &new_label);
    
    /**
     * Get the key of the menu item.
     * 
     * @return The key.
     */
    const scim::String &get_key () const;
    
    /**
     * Set the key of the menu item.
     * 
     * @param new_key The new key.
     */
    void set_key (const scim::String &new_key);
    
protected:

    /**
     * Constructor.
     */
    MenuItem (const scim::String &key = "", const scim::String &label = "");

private:

    /**
     * The label of the menu item.
     */
    scim::String label;

    /**
     * The key for the menu item.
     */
    scim::String key;
    
    /**
     * The index of the selected child.
     */
    ssize_t selected_index;
    
    /**
     * The set of children.
     */
    std::vector<MenuItem*> children;

};

class Menu
{

public:

    /**
     * Constructor.
     */
    Menu ();

    /**
     * Destructor.
     */
    ~Menu ();
    
    /**
     * Get the root of the menu items.
     * 
     * @return The root of the menu items.
     */
    MenuItem &get_root ();
    
    /**
     * Get the root of the menu items.
     * 
     * @return The root of the menu items.
     */
    const MenuItem &get_root () const;

private:

    /**
     * The root of the menu items.
     */
    MenuItem *root;

};

};

#endif /*SCIM_CONSOLE_MENU_H_*/
