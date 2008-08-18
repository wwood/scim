#include <stdexcept>

#include "scim_console_menu.h"

using namespace std;
using namespace scim;
using namespace scim_console;

MenuItem::MenuItem (const String &new_key, const String &new_label): key (new_key), label (new_label), selected_index (-1)
{}

MenuItem::~MenuItem ()
{
    clear_all_children ();
}

size_t MenuItem::get_child_count () const
{
    return children.size ();
}

ssize_t MenuItem::get_selected_index () const
{
    return selected_index;
}

void MenuItem::set_selected_index (ssize_t new_index)
{
    if (new_index >= static_cast<ssize_t> (children.size ()))
        throw range_error ("Index out of bounds");

    if (selected_index >= 0 && children[selected_index]->get_selected_index () >= 0)
        children[selected_index]->set_selected_index (-1);
    selected_index = new_index;
}

const String &MenuItem::get_label () const
{
    return label;
}

void MenuItem::set_label (const String &new_label)
{
    label = new_label;
}

const String &MenuItem::get_key () const
{
    return key;
}

void MenuItem::set_key (const String &new_key)
{
    key = new_key;
}

void MenuItem::add_child (const String &new_key, const String &new_label)
{
    children.push_back (new MenuItem (new_key, new_label));
}

MenuItem &MenuItem::get_child (size_t index)
{
    if (index >= static_cast<ssize_t> (children.size ()))
        throw range_error ("Index out of bounds");
        
    return *children[index];
}

const MenuItem &MenuItem::get_child (size_t index) const
{
    if (index >= static_cast<ssize_t> (children.size ()))
        throw range_error ("Index out of bounds");
    
    return *children[index];
}

void MenuItem::clear_all_children ()
{
    for (vector<MenuItem*>::iterator i = children.begin (); i != children.end (); ++i) {
        delete *i;
    }
    children.clear ();
    
    set_selected_index (-1);
}


class RootMenuItem: public MenuItem
{

public:

    RootMenuItem (): MenuItem ("/", "")
    {}
    
    ~RootMenuItem ()
    {}

}
;

Menu::Menu ()
{
    root = new RootMenuItem ();
}

Menu::~Menu ()
{
    delete root;
}

const MenuItem &Menu::get_root () const
{
    return *root;
}

MenuItem &Menu::get_root ()
{
    return *root;
}
