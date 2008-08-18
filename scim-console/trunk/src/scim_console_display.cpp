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
 * @brief This file describes about the implementation of the display class for SCIM-Console.
 */

#include <alloca.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>

#include <linux/kd.h>
#include <linux/vt.h>

extern "C" {
#include <curses.h>
#include <rote/rote.h>
}

#include <iostream>
#include <sstream>
#include <vector>

#define Uses_SCIM_ATTRIBUTE
#define Uses_SCIM_DEBUG
#define Uses_SCIM_IMEngine
#define Uses_SCIM_UTILITY
#include <scim.h>

#include "scim_console_debug.h"
#include "scim_console_display.h"
#include "scim_console_imcontext.h"
#include "scim_console_menu.h"
#include "scim_console_panel.h"

namespace scim_console
{

/**
 * The type for coloring attributes.
 */
typedef unsigned int color_t;

/**
 * The default foreground color for the preedit.
 */
static const color_t DEFAULT_PREEDIT_FOREGROUND = 0xFFFFFF;


/**
 * The default background color for the preedit.
 */
static const color_t DEFAULT_PREEDIT_BACKGROUND = 0x000000;


/**
 * The implementation of the terminal display.
 */
class DisplayImpl: public Display
{

public:


    /**
     * Constructor.
     *
     * @param new_scim_console The scim console.
     * @param imcontext The IMContext.
     */
    DisplayImpl (ScimConsole *new_scim_console);

    /**
     * Destructor.
     */
    ~DisplayImpl ();

    /**
     * Initialize the terminal.
     *
     * @return RETVAL_SUCCEEDED if no error has been occurred.
     */
    retval_t initialize ();

    /**
     * Handle extra escape sequences.
     *
     * @param sequence The escape sequence.
     * @return The retval.
     */
    int handle_escape_sequence (const char *sequence);

    retval_t get_cursor_location (Point &point) const;

    retval_t update ();

    retval_t forward_pty_output (const scim::String &output_string);

    retval_t beep ();

    size_t get_width () const;

    size_t get_height () const;

    size_t get_client_width () const;

    size_t get_client_height () const;

    const scim::String &get_name () const;

    int get_tty_number () const;

    bool is_active () const;

    int get_fd () const;

    event_type_t get_triggers () const;

    bool handle_event (event_type_t events);

private:

    /**
     * The height of the panel.
     */
    static const int PANEL_HEIGHT = 1;

    /**
     * The scim console.
     */
    ScimConsole *scim_console;

    /**
     * The rote terminal.
     */
    RoteTerm *rote_terminal;

    /**
     * If this terminal is initialized or not.
     */
    bool initialized;

    /**
     * The fd for the console.
     */
    int console_fd;

    /**
     * The number of the tty.
     */
    int tty_number;

    /**
     * The name of thie display.
     */
    scim::String display_name;

    /**
     * The width of the console.
     */
    int width;

    /**
     * The height of the console.
     */
    int height;

    /**
     * The max refresh interval in micro seconds.
     */
    unsigned int refresh_interval;
    
    /**
     * The main window for SCIM Console.
     */
    WINDOW *main_window;
    
    /**
     * The window for the client.
     */
    WINDOW *client_window;

    /**
     * The event socket for reading.
     */
    int socket_in;

    /**
     * The event socket for writing.
     */
    int socket_out;

    /**
     * The location of the cursor.
     */
    Point cursor_location;

    /**
     * The visibility of the cursor.
     */
    bool cursor_visible;

    /**
     * Update (refresh) the panel on the display.
     *
     * @return RETVAL_SUCCEEDED if it succeeded.
     */
    retval_t update_panel ();

    /**
     * Update (refresh) the preedit string on the display.
     *
     * @return RETVAL_SUCCEEDED if it succeeded.
     */
    retval_t update_preedit ();

    /**
     * Update (refresh) the client console on the display.
     *
     * @return RETVAL_SUCCEEDED if it succeeded.
     */
    retval_t update_client ();

    /**
     * Update the popup dialog.
     * 
     * @return RETVAL_SUCCEEDED if it succeeded.
     */
    retval_t update_popup ();

    /**
     * Move the cursor into the correct position.
     */
    retval_t update_cursor ();

    /**
     * Fetch and store the display size.
     * 
     * @return RETVAL_SUCCEEDED if it succeeded.
     */
    retval_t fetch_display_size ();

};

/**
 * The instance of the Display.
 */
static DisplayImpl *display_instance = NULL;

};

using namespace std;
using namespace scim;
using namespace scim_console;


/* Helper Functions */

/**
 * Get the number of the columns consumed by the given wide character.
 * 
 * @return The number of the columns, or return -1 if error has been occured.
 */
static ssize_t get_wchar_width (const wchar_t wc)
{
    return wcwidth (wc);
}

/**
 * Get the number of the columns consumed by the given wide string.
 * 
 * @param wstring The wide string to print out.
 * @return The number of the columns, or return -1 if error has been occured.
 */
static ssize_t get_wstring_width (const WideString &wstring, ssize_t n = -1)
{
    if (n < 0 || n > wstring.size ()) {
        return wcswidth (wstring.c_str (), wstring.size ());
    } else {
        return wcswidth (wstring.c_str (), n);
    }
}

/**
 * Get the number of the columns consumed by the given string.
 * 
 * @param string The string to print out.
 * @param n The maximum number of the wide characters to count. If -1 is given here, the whole string is counted.
 * @return The number of the columns, or return -1 if error has been occured.
 */
static ssize_t get_string_width (const String &string, ssize_t n = -1)
{
    return get_wstring_width (utf8_mbstowcs (string), n);
}

/**
 * Clear the contents in a rectangular resion of the specific curses window.
 * 
 * @param window The window.
 * @param y The y location of the resion.
 * @param x The x location of the resion.
 * @param height The height of the resion.
 * @param width The width of the resion.
 * @return RETVAL_SUCCEEDED if it succeeded.
 */
static retval_t weraserect (WINDOW *window, size_t y, size_t x, size_t height, size_t width)
{
    for (size_t i = 0; i < height; ++i) {
        for (size_t j = 0; j < width; ++j) {
            if (mvwprintw (window, y + i, x + j, " "))
                return RETVAL_FAILED;
        }
    }

    return RETVAL_SUCCEEDED;
}

/**
 * Draw a box into the specific curses window.
 * 
 * @param window The window.
 * @param y The y location of the box.
 * @param x The x location of the box.
 * @param height The height of the box.
 * @param width The width of the box.
 * @return RETVAL_SUCCEEDED if it succeeded.
 */
static retval_t mvwbox (WINDOW *window, size_t y, size_t x, size_t height, size_t width)
{
    for (size_t i = 0; i < height; ++i) {
        if (mvwprintw (window, y + i, x, "#") || mvwprintw (window, y + i, x + width - 1, "#"))
            return RETVAL_FAILED;
    }

    for (size_t i = 0; i < width; ++i) {
        if (mvwprintw (window, y, x + i, "#") || mvwprintw (window, y + height - 1, x + i, "#"))
            return RETVAL_FAILED;
    }

    return RETVAL_SUCCEEDED;
}

/**
 * Check if the given fd is connected to the console or not.
 *
 * @param fd The fd to check.
 * @return true if the given fd is connected to console.
 */
static int is_a_console (int fd) {
	char arg;
	return (ioctl (fd, KDGKBTYPE, &arg) == 0 && ((arg == KB_101) || (arg == KB_84)));
}

/**
 * Open the given device as a console device.
 *
 * @return The fd for the console.
 */
static int open_console (const char *path) {
	int fd = open (path, O_RDWR);
	if (fd < 0 && errno == EACCES)
		fd = open (path, O_WRONLY);
	if (fd < 0 && errno == EACCES)
		fd = open (path, O_RDONLY);
	if (fd < 0)
		return -1;

	if (!is_a_console (fd)) {
		close (fd);
		return -1;
	} else {
	    return fd;
    }
}

/**
 * Handle extra escape sequences.
 *
 * @param rt The virtual terminal.
 * @param es The escape sequence.
 * @return the retval.
 */
static int handle_extra_escape_sequence (RoteTerm *rt, const char *sequence) {
    return display_instance->handle_escape_sequence (sequence);
}
/* Implementations */

Display *Display::alloc (ScimConsole *new_scim_console)
{
    if (display_instance == NULL) {
        display_instance = new DisplayImpl (new_scim_console);
        if (display_instance->initialize ()) {
            delete display_instance;
            display_instance = NULL;
            return NULL;
        } else {
            return display_instance;
        }
    } else {
        return NULL;
    }
}


DisplayImpl::DisplayImpl (ScimConsole *new_scim_console): scim_console (new_scim_console), rote_terminal (NULL), initialized (false), console_fd (-1), tty_number (-1), refresh_interval (100000), 
main_window (NULL), client_window (NULL), socket_out (-1), socket_in (-1), cursor_visible (true)
{}


DisplayImpl::~DisplayImpl ()
{
    if (console_fd >= 0) {
        close (console_fd);
        console_fd = -1;
    }

    if (initialized) {
        if (delwin (client_window) && delwin (main_window)) 
            cerr << "Failed to free the sub windows" << endl;
        
        if (endwin () == ERR)
            cerr << "Failed to finalize the console" << endl;
    }
    
    if (rote_terminal != NULL) {
        rote_vt_destroy (rote_terminal);
        rote_terminal = NULL;
    }
    
    display_instance = NULL;
}


retval_t DisplayImpl::initialize ()
{
    setlocale(LC_ALL, "");
    setenv("TERM", "linux", 1);

    int socket_pair[2];

    if (socketpair (PF_UNIX, SOCK_STREAM, 0, socket_pair)) {
        cerr << "Cannot make a pipe for a interruption heandler: " << strerror (errno) << endl;
        return RETVAL_FAILED;
    }

    socket_out = socket_pair[0];
    socket_in = socket_pair[1];
    
    int fd = -1;

    fd = open_console ("/dev/tty");
	if (fd < 0)
        fd = open_console ("/dev/tty0");
	if (fd < 0)
        fd = open_console ("/dev/vc/0");
	if (fd < 0)
        fd = open_console ("/dev/console");
	if (fd < 0) {
    	for (int i = 0; i < 3; ++i) {
    		if (is_a_console (i)) {
                fd = i;
                break;
            }
        }
    }

    if (fd < 0) {
        cerr << "No console device has been found" << endl;
        return RETVAL_FAILED;
    }
    console_fd = fd;

    struct vt_stat vtstat;
    if (ioctl (console_fd, VT_GETSTATE, &vtstat)) {
        cerr << "Failed to grab the tty device: " << strerror (errno) << endl;
        return RETVAL_FAILED;
    }
    tty_number = vtstat.v_active;
    
    dout (8) << "Grabbing the display configuration..." << endl;
    
    ostringstream oss;
    oss << ":-1." << tty_number;
    display_name = oss.str ();
    
    dout (8) << "Initializing the curses libraries..." << endl;
    if (initscr () == NULL || move (0, 0)) {
        cerr << "Failed to initialize the terminal: " << strerror (errno) << endl;
        return RETVAL_FAILED;
    }
    
    if (fetch_display_size ()) {
        cerr << "Failed to grab the display size" << endl;
        return RETVAL_FAILED;
    }
    dout (6) << "Display size: " << width << " x " << height << endl;

    //start_color ();
    
    main_window = newwin (height, width, 0, 0);
    client_window = subwin (main_window, height - PANEL_HEIGHT, width, 0, 0);
    dout (8) << "curses, initialized" << endl;

    for (int i = 0; i < height - 1; ++i) {
        for (int j = 0; j < width; ++j) {
            addch(' ');
        }
    }
    refresh ();

    rote_terminal = rote_vt_create (height - 1, width);
    rote_vt_install_handler (rote_terminal, handle_extra_escape_sequence);
    
    raw ();

    initialized = true;

    dout (8) << "Initilize, done" << endl;
    return RETVAL_SUCCEEDED;
}

int DisplayImpl::get_tty_number () const
{
    return tty_number;
}

bool DisplayImpl::is_active () const
{
    struct vt_stat vtstat;
    if (ioctl (console_fd, VT_GETSTATE, &vtstat)) {
        cerr << "Failed to grab the tty device: " << strerror (errno) << endl;
        scim_console-> quit (-1);
        return false;
    }
    
    return vtstat.v_active == tty_number;
}

int DisplayImpl::handle_escape_sequence (const char *sequence)
{
    if (sequence[0] != '{') {
        dout (9) << "Unknown escape sequence" << endl;
        return ROTE_HANDLERESULT_NOWAY;
    }

    if (sequence[strlen (sequence) - 1] != '}') {
        dout (2) << "The escape sequence continues..." << endl;
        return ROTE_HANDLERESULT_NOTYET;
    }

    const int color = atoi (sequence + 1);
    if (color < 0 || color > 7) {
        dout (9) << "Unknown coloring sequence" << endl;
        return false;
    }

    attrset (COLOR_PAIR (color * 8));
    move (0, 0);
    for (int i = 0; i < height - PANEL_HEIGHT; i++) {
        for (int j = 0; j < width; j++) {
            addch(' ');
        }
    }
    touchwin (client_window);
   
    return ROTE_HANDLERESULT_OK;
}

retval_t DisplayImpl::get_cursor_location (Point &point) const
{
    int x = rote_terminal->ccol;
    int y = rote_terminal->crow;

    if (y >= 0 && x >= 0) {
        point.set_x (x);
        point.set_y (y);
        return RETVAL_SUCCEEDED;
    } else {
        return RETVAL_FAILED;
    }
}

retval_t DisplayImpl::fetch_display_size ()
{
    getmaxyx (stdscr, height, width);
    return RETVAL_SUCCEEDED;
}

size_t DisplayImpl::get_width () const
{
    return width;
}

size_t DisplayImpl::get_height () const
{
    return height;
}

size_t DisplayImpl::get_client_width () const
{
    return width;
}

size_t DisplayImpl::get_client_height () const
{
    return height - PANEL_HEIGHT;
}

retval_t DisplayImpl::update ()
{
    dout (6) << "DisplayImpl::update ()" << endl;

    static timeval previous_time;
    static bool first_run = true;

    if (first_run)
        gettimeofday (&previous_time, NULL);
    
    timeval current_time;
    gettimeofday (&current_time, NULL);

    const long dt = (current_time.tv_sec - previous_time.tv_sec) * 1000000 + previous_time.tv_usec;
    if (first_run || dt > refresh_interval) {

        if (!touchwin (main_window) && !update_client () && !update_preedit () && !update_panel () && !update_popup () && !update_cursor () && !wrefresh (main_window)) {
            dout (1) << "update, done" << endl;
            return RETVAL_SUCCEEDED;
        } else {
            dout (1) << "update, failed" << endl;
            return RETVAL_FAILED;
        }

        previous_time = current_time;
    } else {
        timeval remaining_time;
        remaining_time.tv_sec = dt / 1000000;
        remaining_time.tv_usec = dt % 1000000;
        scim_console->interrupt (remaining_time);
    }

    first_run = false;
}

retval_t DisplayImpl::update_client ()
{
    rote_vt_draw (rote_terminal, client_window, 0, 0, NULL);
    return RETVAL_SUCCEEDED;
}

retval_t DisplayImpl::update_popup ()
{
    return RETVAL_SUCCEEDED;
}

retval_t DisplayImpl::update_cursor ()
{
    curs_set (cursor_visible);
    if (wmove (main_window, cursor_location.get_y (), cursor_location.get_x ())) {
        cerr << "Failed to move the cursor" << endl;
        return RETVAL_FAILED;
    } else {
        return RETVAL_SUCCEEDED;
    }
}

retval_t DisplayImpl::update_preedit ()
{
    if (wattrset (main_window, A_NORMAL)) {
        cerr << "Failed to clear the preedit string" << endl;
        return RETVAL_FAILED;
    }
    IMContext *imcontext = IMContext::get_focused_imcontext ();
    
    // Update the preeedit.
    bool tailing_cursor = true;
    get_cursor_location (cursor_location);
    if (imcontext->is_preedit_shown ()) {

        const size_t preedit_cursor_position = imcontext->get_preedit_cursor_position ();
        const WideString &preedit_string = imcontext->get_preedit_string ();
        const AttributeList &preedit_attributes = imcontext->get_preedit_attributes ();

        color_t preedit_foreground;
        color_t preedit_background;
        bool preedit_reversed;
        bool preedit_hilighted;
        bool preedit_underlined;

        cursor_visible = true;
        size_t cursor_x = cursor_location.get_x ();
        size_t cursor_y = cursor_location.get_y ();
        for (size_t i = 0; i < preedit_string.size (); ++i) {

            preedit_foreground = DEFAULT_PREEDIT_FOREGROUND;
            preedit_background = DEFAULT_PREEDIT_BACKGROUND;
            preedit_reversed = false;
            preedit_hilighted = false;
            preedit_underlined = false;

            for (size_t j = 0; j < preedit_attributes.size (); ++j) {
                const Attribute &attribute = preedit_attributes [j];
                if (attribute.get_start () <= i && i < attribute.get_end ()) {
                    switch (attribute.get_type ()) {
                    case SCIM_ATTR_FOREGROUND:
                        preedit_foreground = 0xFFFFFF & attribute.get_value ();
                        break;
                    case SCIM_ATTR_BACKGROUND:
                        preedit_background = 0xFFFFFF & attribute.get_value ();
                        break;
                    case SCIM_ATTR_DECORATE:
                        switch (attribute.get_value ()) {
                        case SCIM_ATTR_DECORATE_UNDERLINE:
                            preedit_underlined = true;
                            break;
                        case SCIM_ATTR_DECORATE_HIGHLIGHT:
                            preedit_hilighted = true;
                            break;
                        case SCIM_ATTR_DECORATE_REVERSE:
                            preedit_reversed = true;
                            break;
                        default:
                            break;
                        }
                        break;
                    default:
                        break;
                    }
                }
            }

            // FIXME: Coloring attributes are not yet supported.
            if (preedit_hilighted) {
                if (wattron (main_window, A_STANDOUT))
                    cerr << "Failed to apply an attribute on the preedit string" << endl;
            } else {
                if (wattroff (main_window, A_STANDOUT))
                    cerr << "Failed to apply an attribute on the preedit string" << endl;
            }
            if (preedit_reversed) {
                if (wattron (main_window, A_REVERSE))
                    cerr << "Failed to apply an attribute on the preedit string" << endl;
            } else {
                if (wattroff (main_window, A_REVERSE))
                    cerr << "Failed to apply an attribute on the preedit string" << endl;
            }
            if (preedit_underlined) {
                if (wattron (main_window, A_UNDERLINE))
                    cerr << "Failed to apply an attribute on the preedit string" << endl;
            } else {
                if (wattroff (main_window, A_UNDERLINE))
                    cerr << "Failed to apply an attribute on the preedit string" << endl;
            }

            if (i == preedit_cursor_position) {
                cursor_location.set_x (cursor_x);
                cursor_location.set_y (cursor_y);
                tailing_cursor = false;
            }
    
            unsigned char utf8_str[5];
            bzero (utf8_str, sizeof (unsigned char) * 5);
            utf8_wctomb (utf8_str, preedit_string[i], 5);
            const size_t wchar_width = get_wchar_width (preedit_string[i]);
            if (cursor_x + wchar_width > width) {
                cursor_x = 0;
                ++cursor_y;
            }
        
            if (mvwprintw (main_window, cursor_y, cursor_x, "%s", utf8_str)) {
                cerr << "Failed to print the preedit string" << endl;
                return RETVAL_FAILED;
            }
            cursor_x += wchar_width;
        }

        if (tailing_cursor) {
            cursor_location.set_x (cursor_x);
            cursor_location.set_y (cursor_y);
        }
    } else {
        cursor_visible = true;
    }
    
    // Update the lookup table.
    if (imcontext->is_lookup_table_shown ()) {
        if (wattrset (main_window, A_NORMAL)) {
            cerr << "Failed to prepare drawing the lookup table" << endl;
            return RETVAL_FAILED;
        }
            
        const LookupTable &lookup_table = imcontext->get_lookup_table ();
        const size_t table_height = lookup_table.get_current_page_size () + 4;

        size_t max_label_width = 0;
        for (size_t i = 0; i < lookup_table.get_current_page_size (); ++i) {
            const size_t label_width = get_wstring_width (lookup_table.get_candidate (i));
            if (label_width > max_label_width)
                max_label_width = label_width;
        }
        const size_t table_width = max_label_width + 4 + 2;

        Point cursor_location;
        get_cursor_location (cursor_location);
        size_t table_x = cursor_location.get_x ();
        size_t table_y = cursor_location.get_y () + 1;
        
        const size_t preedit_cursor_position = imcontext->get_preedit_cursor_position ();
        const WideString &preedit_string = imcontext->get_preedit_string ();
        for (int i = 0; i < preedit_cursor_position ; ++i) {
            const ssize_t progress = get_wchar_width (preedit_string[i]);
            if (table_x + progress > width) {
                table_x = 0;
                ++table_y;
            }
            table_x += progress;
        }
        
        if (table_x + table_width >= width)
            table_x = width - table_width - 1;
        if (table_y + table_height >= height - 1)
            table_y = height - table_height - 2;

        const size_t page_index = lookup_table.get_current_page_start () / lookup_table.get_page_size ();
        const size_t page_count = lookup_table.number_of_candidates () / lookup_table.get_page_size () + 1;
        if (weraserect (main_window, table_y, table_x, table_height, table_width) || 
            mvwbox (main_window, table_y, table_x, 3, table_width) ||
            mvwbox (main_window, table_y, table_x, table_height, table_width) ||
            mvwprintw (main_window, table_y + 1, table_x + 1, " %d/%d", page_index + 1, page_count)) {
            cerr << "Failed to draw the lookup table" << endl;
            return RETVAL_FAILED;
        }

        for (int i = 0; i < lookup_table.get_current_page_size (); ++i) {
            const char *label_cstr = utf8_wcstombs (lookup_table.get_candidate (i)).c_str ();
            
            if (lookup_table.get_cursor_pos_in_current_page () == i) {
                if (wattron (main_window, A_REVERSE)) {
                    cerr << "Failed to draw the lookup candidates" << endl;
                    return RETVAL_FAILED;
                }
            }
            if (mvwprintw (main_window, table_y + i + 3, table_x + 1, " %1d:%s", i, label_cstr)) {
                cerr << "Failed to draw the lookup candidates" << endl;
                return RETVAL_FAILED;
            }
            if (wattroff (main_window, A_REVERSE)) {
                cerr << "Failed to draw the lookup candidates" << endl;
                return RETVAL_FAILED;
            }
        }
    }

    return RETVAL_SUCCEEDED;
}


retval_t DisplayImpl::update_panel ()
{
    Panel &panel = scim_console->get_panel ();
    const Menu &menu = panel.get_menu ();
    if (!panel.has_update () && menu.get_root ().get_selected_index () < 0)
        return RETVAL_SUCCEEDED;

    if (wattrset (main_window, A_NORMAL)) {
        cerr << "Failed to reset the attributes on the main window" << endl;
        return RETVAL_FAILED;
    }
    if (wmove (main_window, height - PANEL_HEIGHT, 0) || wdeleteln (main_window)) {
        cerr << "Failed to clear the panel area on the main window" << endl;
        return RETVAL_FAILED;
    }

    size_t cursor_x = 0;
    if (mvwprintw (main_window, height - PANEL_HEIGHT, 0, "[SCIM]")) {
        cerr << "Failed to draw the panel" << endl;
        return RETVAL_FAILED;
    }
    cursor_x = get_string_width ("[SCIM]");

    for (int i = 0; i < menu.get_root ().get_child_count (); ++i) {
        const MenuItem &menu_item = menu.get_root ().get_child (i);
        
        if (menu.get_root ().get_selected_index () != i) {
            if (mvwprintw (main_window, height - PANEL_HEIGHT, cursor_x, "[%s]", menu_item.get_label ().c_str ())) {
                cerr << "Failed to draw a menu item" << endl;
                return RETVAL_FAILED;
            }
        } else {
            if (wattron (main_window, A_REVERSE) || mvwprintw (main_window, height - PANEL_HEIGHT, cursor_x, "[%s]", menu_item.get_label ().c_str ()) || wattroff (main_window, A_REVERSE)) {
                cerr << "Failed to draw the menu" << endl;
                return RETVAL_FAILED;
            }
            
            if (menu_item.get_child_count () > 0 && menu_item.get_selected_index () >= 0) {
                size_t max_label_width = 0;
                for (int j = 0; j < menu_item.get_child_count (); ++j) {
                    const size_t label_width = get_string_width (menu_item.get_child (j).get_label ());
                    if (label_width > max_label_width)
                        max_label_width = label_width;
                }
                const size_t menu_width = max_label_width + 2;
                const size_t menu_height = menu_item.get_child_count () + 2;
                const size_t menu_x = cursor_x;
                const size_t menu_y = height - menu_height - PANEL_HEIGHT;
                
                if (mvwbox (main_window, menu_y, menu_x, menu_height, menu_width)) {
                    cerr << "Failed to draw a menu" << endl;
                    return RETVAL_FAILED;
                }
                for (int j = 0; j < menu_item.get_child_count (); ++j) {
                    if (menu_item.get_selected_index () == j) {
                        if (wattron (main_window, A_REVERSE)) {
                            cerr << "Failed to draw a menu item" << endl;
                            return RETVAL_FAILED;
                        }
                    }
                    if (mvwprintw (main_window, menu_y + j + 1, menu_x + 1, "%s", menu_item.get_child (j).get_label ().c_str ()) ||
                        wattroff (main_window, A_REVERSE)) {
                        cerr << "Failed to draw a menu item" << endl;
                        return RETVAL_FAILED;
                    }
                }
            }
        }
        
        cursor_x += get_string_width (menu_item.get_label ()) + 2;
    }

    panel.clear_update ();
    return RETVAL_SUCCEEDED;
}


retval_t DisplayImpl::forward_pty_output (const String &output_string)
{
    static int i = 0;
    dout (6) << "DisplayImpl::forward_pty_output ()" << endl;
    dout (3) << "string: " << output_string << endl;
    rote_vt_inject (rote_terminal, output_string.c_str (), output_string.size ());

    return RETVAL_SUCCEEDED;
}

retval_t DisplayImpl::beep ()
{
    if (beep ()) {
        cerr << "Failed to make a beep sound" << endl;
        return RETVAL_FAILED;
    } else {
        return RETVAL_SUCCEEDED;
    }
}

const String &DisplayImpl::get_name () const
{
    return display_name;
}

int DisplayImpl::get_fd () const
{
    return socket_in;
}

event_type_t DisplayImpl::get_triggers () const
{
    return EVENT_INTERRUPT;
}

bool DisplayImpl::handle_event (event_type_t events)
{
    return true;
}
