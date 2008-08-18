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
 * @brief This file describes about the implementation of the panel.
 */

#include <cassert>
#include <sstream>
#include <vector>

#define Uses_SCIM_PANEL
#define Uses_SCIM_PROPERTY
#define Uses_SCIM_UTILITY
#include <scim.h>

#include "scim_console.h"
#include "scim_console_debug.h"
#include "scim_console_imcontext.h"
#include "scim_console_menu.h"
#include "scim_console_panel.h"

namespace scim_console
{

/**
 * The type for modes of the panel.
 */
enum mode_t
{
    MODE_IDLE,
    MODE_SHOWING_POPUP,
    MODE_SELECTING_MENU
};

/**
 * The panel implementation.
 */
class PanelImpl: public Panel
{

public:

    /**
     * Constructor.
     *
     * @param new_scim_console The SCIM Console.
     */
    PanelImpl (ScimConsole *new_scim_console);

    /**
     * Destructor.
     */
    ~PanelImpl ();

    bool is_popup_shown () const;
    
    const scim::String &get_popup_string () const;
    
    const Menu &get_menu () const;
    
    retval_t register_properties (const scim::PropertyList &properties);
    
    retval_t deregister_properties (const scim::PropertyList &properties);

    retval_t update_property (const scim::Property &property);
    
    retval_t update ();
    
    bool has_update () const;

    void clear_update ();

    retval_t request_help ();

    retval_t request_factory_menu ();
    
    bool process_hotkey_menu ();

    bool process_hotkey_ok ();
    
    bool process_hotkey_cancel ();
    
    bool process_hotkey_previous ();
    
    bool process_hotkey_next ();

private:

    /**
     * The pointer for the scim console.
     */
    ScimConsole *scim_console;

    /**
     * The current popup string.
     */
    scim::String popup_string;

    /**
     * The mode of the panel.
     */
    mode_t mode;

    /**
     * If the panel has update.
     */
    bool updated;
    
    /**
     * The menu of the panel.
     */
    Menu menu;

    /**
     * The properties.
     */
    scim::PropertyList properties;

};

/**
 * The instance of the panel.
 */
static PanelImpl *panel = NULL;

};

using std::cerr;
using std::endl;
using std::stringstream;
using std::vector;

using namespace scim;
using namespace scim_console;

/* Implementations */
Panel *Panel::alloc (ScimConsole *new_scim_console)
{
    if (panel != NULL) {
        cerr << "There is already another panel" << endl;
        return NULL;
    } else {
        panel = new PanelImpl (new_scim_console);
        return panel;
    }
}

PanelImpl::PanelImpl (ScimConsole *new_console): scim_console (new_console), mode (MODE_IDLE), updated (false)
{
    update ();
}

PanelImpl::~PanelImpl ()
{
    panel = NULL;
}

bool PanelImpl::is_popup_shown () const
{
    return mode == MODE_SHOWING_POPUP;
}

const String &PanelImpl::get_popup_string () const
{
    return popup_string;
}

const Menu &PanelImpl::get_menu () const
{
    return menu;
}
    
retval_t PanelImpl::register_properties (const PropertyList &new_properties)
{
    for (PropertyList::const_iterator i = new_properties.begin (); i != new_properties.end (); ++i) {
        bool found = false;
        for (PropertyList::iterator j = properties.begin (); j != properties.end (); ++j) {
            if ((*i).get_key () == (*j).get_key ()) {
                found = true;
                break;
            }
        }
        if (!found) {
            const Property &property = *i;
            properties.push_back (property);
        }
    }
}

retval_t PanelImpl::deregister_properties (const PropertyList &old_properties)
{
    for (PropertyList::const_iterator i = old_properties.begin (); i != old_properties.end (); ++i) {
        for (PropertyList::iterator j = properties.begin (); j != properties.end ();) {
            if ((*i).get_key () == (*j).get_key ()) {
                j = properties.erase (j);
            } else {
                ++j;
            }
        }
    }
}

retval_t PanelImpl::update_property (const Property &property)
{
    for (PropertyList::iterator i = properties.begin (); i != properties.end (); ++i) {
        Property &target_property = *i;
        if (target_property.get_key () == property.get_key ()) {
            target_property = property;
        }
    }
}

retval_t PanelImpl::update ()
{
    IMContext *focused_imcontext = IMContext::get_focused_imcontext ();
    const PanelFactoryInfo &factory_info = focused_imcontext->get_imengine_info ();

    MenuItem &root_menu_item = menu.get_root ();
    root_menu_item.clear_all_children ();
    if (mode == MODE_SELECTING_MENU) {
        root_menu_item.set_selected_index (-1);
        mode = MODE_IDLE;
    }
    root_menu_item.add_child ("/IMEngine", factory_info.name);
    MenuItem &imengine_menu_item = root_menu_item.get_child (0);

    stringstream strbuf;
    BackEndPointer scim_backend = scim_console->get_scim_backend ();

    vector<IMEngineFactoryPointer> factories;
    scim_backend->get_factories_for_encoding (factories, "UTF-8");

    for (size_t i = 0; i < factories.size (); ++i) {
        const String utf8_factory_name = utf8_wcstombs (factories[i]->get_name ());
        strbuf << "/IMEngine/" << utf8_factory_name;
        const String key = strbuf.str ();
        strbuf.str ("");
        const String label = utf8_factory_name;
        imengine_menu_item.add_child (key, label);
    }

    strbuf << "/IMEngine/" << factory_info.name << "/";
    const String key_prefix = strbuf.str ();
    for (PropertyList::iterator i = properties.begin (); i != properties.end (); ++i) {
        Property &property = *i;
        const String property_key = property.get_key ();
        
        if (property_key.size () >= key_prefix.size () && property_key.substr (0, key_prefix.size ()) == key_prefix) {
            String menu_key;
            String submenu_key;

            const size_t separator_index = property_key.find ("/", key_prefix.size ());
            if (separator_index == String::npos) {
                menu_key = property_key.substr (key_prefix.size ());
            } else {
                menu_key = property_key.substr (key_prefix.size (), separator_index - key_prefix.size ());
                submenu_key = property_key.substr (separator_index + 1);
            }
            dout (1) << menu_key << ":" << submenu_key << endl;

            const String menu_full_key = key_prefix + menu_key;
            
            bool menu_created = false;
            for (size_t i = 1; i < root_menu_item.get_child_count (); ++i) {
                if (root_menu_item.get_child (i).get_key () == menu_full_key) {
                    menu_created = true;
                    if (root_menu_item.get_child (i).get_label ().empty ())
                        root_menu_item.get_child (i).set_label (property.get_label ());

                    if (!submenu_key.empty ()) {
                        root_menu_item.get_child (i).add_child (property_key, property.get_label ());
                        break;
                    }
                }
            }
            if (!menu_created) {
                if (submenu_key.empty ()) {
                    root_menu_item.add_child (property_key, property.get_label ());
                } else {
                    root_menu_item.add_child (property_key, "");
                    root_menu_item.get_child (root_menu_item.get_child_count () - 1).add_child (property_key, property.get_label ());
                }
            }
        }
    }

    updated = true;
}

bool PanelImpl::has_update () const
{
    return updated;
}

void PanelImpl::clear_update ()
{
    updated = false;
}

bool PanelImpl::process_hotkey_menu ()
{
    if (mode == MODE_IDLE) {
        menu.get_root ().set_selected_index (0);
        mode = MODE_SELECTING_MENU;
        updated = true;
        return true;
    }
    
    return false;
}

bool PanelImpl::process_hotkey_ok ()
{
    if (mode == MODE_SHOWING_POPUP) {
        mode = MODE_IDLE;
        updated = true;
        return true;
    } else if (mode == MODE_SELECTING_MENU) {
        MenuItem *parent_menu_item = NULL;
        MenuItem *menu_item = &menu.get_root ();
        while (menu_item->get_selected_index () >= 0) {
            parent_menu_item = menu_item;
            menu_item = &parent_menu_item->get_child (parent_menu_item->get_selected_index ());
        }

        if (menu_item->get_child_count () > 0) {
            menu_item->set_selected_index (0);
            updated = true;
        } else {
            IMContext *imcontext = IMContext::get_focused_imcontext ();
            const String property_key = menu_item->get_key ();

            const size_t imengine_prefix_length = strlen ("/IMEngine/");
            if (property_key.size () > imengine_prefix_length && strncmp ("/IMEngine/", property_key.c_str (), imengine_prefix_length) == 0 && property_key.find ('/', imengine_prefix_length + 1) == String::npos) {
                vector<IMEngineFactoryPointer> factories;
                BackEndPointer scim_backend = scim_console->get_scim_backend ();
                scim_backend->get_factories_for_encoding (factories, "UTF-8");
                MenuItem &imengine_menu_item = menu.get_root ().get_child (0);
                const ssize_t imengine_index = imengine_menu_item.get_selected_index ();
                if (imengine_index >= 0) {
                    imcontext->change_imengine (factories[imengine_index]);
                    update ();
                } else {
                    dout (9) << "No IMEngine is selected" << endl;
                }
            } else {
                imcontext->trigger_property (property_key);
                update ();
            }
        }
        
        return true;
    }

    return false;
}

bool PanelImpl::process_hotkey_cancel ()
{
    if (mode == MODE_SHOWING_POPUP) {
        mode = MODE_IDLE;
        updated = true;
    } else if (mode == MODE_SELECTING_MENU) {
        MenuItem *parent_menu_item = NULL;
        MenuItem *menu_item = &menu.get_root ();
        while (menu_item->get_selected_index () >= 0) {
            parent_menu_item = menu_item;
            menu_item = &parent_menu_item->get_child (parent_menu_item->get_selected_index ());
        }

        parent_menu_item->set_selected_index (-1);
        if (parent_menu_item == &menu.get_root ()) {
            mode = MODE_IDLE;
        }
        updated = true;
        return true;
    }

    return false;
}

bool PanelImpl::process_hotkey_previous ()
{
    if (mode == MODE_SELECTING_MENU) {
        MenuItem *parent_menu_item = NULL;
        MenuItem *menu_item = &menu.get_root ();
        while (menu_item->get_selected_index () >= 0) {
            parent_menu_item = menu_item;
            menu_item = &parent_menu_item->get_child (parent_menu_item->get_selected_index ());
        }

        if (parent_menu_item != NULL && parent_menu_item->get_child_count () > 0) {
            ssize_t new_index = parent_menu_item->get_selected_index ();
            new_index = (new_index - 1) % static_cast<ssize_t> (parent_menu_item->get_child_count ());
            if (new_index < 0)
                new_index += parent_menu_item->get_child_count ();
            parent_menu_item->set_selected_index (new_index);
            updated = true;
        }
        return true;
    }

    return false;
}

bool PanelImpl::process_hotkey_next ()
{
    if (mode == MODE_SELECTING_MENU) {
        MenuItem *parent_menu_item = NULL;
        MenuItem *menu_item = &menu.get_root ();
        while (menu_item->get_selected_index () >= 0) {
            parent_menu_item = menu_item;
            menu_item = &parent_menu_item->get_child (parent_menu_item->get_selected_index ());
        }

        if (parent_menu_item != NULL && parent_menu_item->get_child_count () > 0) {
            ssize_t new_index = parent_menu_item->get_selected_index ();
            new_index = (new_index + 1) % static_cast<ssize_t> (parent_menu_item->get_child_count ());
            parent_menu_item->set_selected_index (new_index);
            updated = true;
        }
        return true;
    }

    return false;
}

retval_t PanelImpl::request_help ()
{
    IMContext *imcontext = IMContext::get_focused_imcontext ();
    if (imcontext != NULL) {
        popup_string = imcontext->get_help_string ();
        mode = MODE_SHOWING_POPUP;
        scim_console->update_display ();
        return RETVAL_SUCCEEDED;
    } else {
        return RETVAL_FAILED;
    }
}

retval_t PanelImpl::request_factory_menu ()
{
    cerr << "PanelImpl::request_factory_menu: not implemented yet" << endl;

    return RETVAL_FAILED;
}
