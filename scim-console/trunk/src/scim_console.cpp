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
 * @brief This file describes about the implementation of scim console class.
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>

#include <iostream>
#include <list>
#include <string>
#include <vector>

#define Uses_SCIM_BACKEND
#define Uses_SCIM_CONFIG
#define Uses_SCIM_CONFIG_MODULE
#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_EVENT
#define Uses_SCIM_HOTKEY
#define Uses_SCIM_IMENGINE_MODULE
#define Uses_SCIM_SOCKET
#include <scim.h>

#include "scim_console.h"
#include "scim_console_client.h"
#include "scim_console_debug.h"
#include "scim_console_display.h"
#include "scim_console_event_listener.h"
#include "scim_console_helper.h"
#include "scim_console_helper_manager.h"
#include "scim_console_imcontext.h"
#include "scim_console_interruption_listener.h"
#include "scim_console_key_event_reader.h"
#include "scim_console_panel.h"
#include "scim_console_stdin_reader.h"

namespace scim_console
{

/**
 * The id for menu key in panel.
 */
static const int PANEL_HOTKEY_MENU = 0;

/**
 * The id for cancel key in panel.
 */
static const int PANEL_HOTKEY_CANCEL = 1;

/**
 * The id for OK key in panel.
 */
static const int PANEL_HOTKEY_OK = 2;

/**
 * The id for previous key in panel.
 */
static const int PANEL_HOTKEY_PREVIOUS = 3;

/**
 * The id for next key in panel.
 */
static const int PANEL_HOTKEY_NEXT = 4;

/**
 * The implementation of scim console.
 */
class ScimConsoleImpl: public ScimConsole
{

public:

    /**
     * Constructor.
     */
    ScimConsoleImpl ();

    /**
     * Destructor.
     */
    ~ScimConsoleImpl ();

    void quit (int new_retval);

    void abort ();
    
    void interrupt (const timeval &time);

    void interrupt ();

    /**
     * Initialize SCIM Console.
     * 
     * @return RETVAL_SUCCEEDED if it succeeded.
     */
    retval_t initialize ();
    
    /**
     * Run the main loop.
     */
    void run ();

    scim::ConfigPointer get_scim_config ();

    scim::BackEndPointer get_scim_backend ();

    const scim::String &get_scim_language () const;

    const scim::KeyboardLayout &get_scim_keyboard_layout () const;

    const Display &get_display () const;

    Panel &get_panel ();
    
    const Panel &get_panel () const;

    void start_helper (const scim::String &helper_uuid);

    void stop_helper (const scim::String &helper_uuid);

    void send_helper_event (const scim::String &helper_uuid, const scim::Transaction &trans);

    void register_properties (const scim::PropertyList &properties);

    void update_property (const scim::Property &property);

    void register_helper_properties (const scim::PropertyList &properties);

    void update_helper_property (const scim::Property &property);
        
    void accept_helper (Helper *helper);
    
    void beep ();
    
    void load_config ();

    void save_config ();

    int get_retval () const;

    /* Event Handlers */
    bool process_key_event (const scim::KeyEvent &key_event, bool hotkeys_enabled);

    void process_pty_output (const scim::String &string);

    void forward_key_event (const scim::KeyEvent &key_event);

    void forward_pty_input (const scim::String &string);

    void update_display ();

private:

    /**
     * The retval of the process.
     */
    int retval;

    /**
     * The set of event listeners.
     */
    std::list<EventListener*> event_listeners;

    /**
     * The IMContext.
     */
    IMContext *imcontext;

    /**
     * The terminal display.
     */
    Display *display;

    /**
     * The panel.
     */
    Panel *panel;

    /**
     * The manager of the helpers.
     */
    HelperManager *helper_manager;

    /**
     * The client.
     */
    Client *client;

    /**
     * The key event reader.
     */
    KeyEventReader *key_event_reader;

    /**
     * The listener for interruptions.
     */
    InterruptionListener *interruption_listener;
    
    /**
     * The stdin reader.
     */
    StdinReader *stdin_reader;

    /**
     * The language for SCIM.
     */
    scim::String scim_language;

    /**
     * The configuration module for SCIM.
     */
    scim::ConfigModule *scim_config_module;
    
    /**
     * The configuration for SCIM.
     */
    scim::ConfigPointer scim_config;

    /**
     * The backend for SCIM.
     */
    scim::BackEndPointer scim_backend;
    
    /**
     * The keyboard layout for SCIM.
     */
    scim::KeyboardLayout scim_keyboard_layout;

    /**
     * The hotkey matcher for SCIM.
     */
    scim::FrontEndHotkeyMatcher scim_frontend_hotkey_matcher;
    
    /**
     * The hotkey matcher for SCIM IMEngines.
     */
    scim::IMEngineHotkeyMatcher scim_imengine_hotkey_matcher;

    /**
     * The hotkey matcher for the panel.
     */
    scim::HotkeyMatcher panel_hotkey_matcher;

    /**
     * If it is active or not.
     */
    bool active;

    /**
     * The previous time of refreshing display.
     */
    timeval previous_refresh_time;

    /**
     * The interval of refreshing display in millisecs.
     */
    unsigned int refresh_interval;

    /**
     * Filter a key event and invoke a hotkey action.
     * 
     * @param key_event The key event. 
     * @return true if the key event is consumed.
     */
    bool process_hotkey (const scim::KeyEvent &key_event);

    /**
     * Finalize SCIM Console.
     */
    retval_t finalize ();

    /**
     * Initialize SCIM.
     * 
     * @return RETVAL_SUCCEEDED if it succeeded.
     */
    retval_t initialize_scim ();

    /**
     * Finalize SCIM.
     * 
     * @return RETVAL_SUCCEEDED if it succeeded.
     */
    retval_t finalize_scim ();

    /**
     * The configuration for SCIM is reloaded.
     *
     * @param config The configuration.
     */
    void slot_reload_config (const scim::ConfigPointer &config);

};

};

/* Helper functions */
using std::cerr;
using std::cout;
using std::endl;
using std::list;
using std::vector;

using namespace scim;
using namespace scim_console;

/**
 * Check the socket fronend for SCIM.
 *
 * @return true if the socket frontend is ready.
 */
static bool is_socket_frontend_ready ()
{
    SocketAddress address;
    address.set_address (scim_get_default_socket_frontend_address ());

    SocketClient socket_client;
    if (!socket_client.connect (address)) {
        cerr << "There is no socket frontend at all." << endl;        
        return false;
    } else {
        uint32 magic;
        if (!scim_socket_open_connection (magic, "ConnectionTester", "SocketFrontEnd", socket_client, 1000)) {
            cerr << "Failed to connect the socket fronent." << endl;
            return false;
        } else {
            return true;
        }
    }
}


/* Implementations */

ScimConsoleImpl::ScimConsoleImpl (): retval (0), active (true), refresh_interval (200)
{
    gettimeofday (&previous_refresh_time, NULL);
}

ScimConsoleImpl::~ScimConsoleImpl ()
{
    finalize ();
}

retval_t ScimConsoleImpl::initialize ()
{
    if (initialize_scim ()) {
        cerr << "Failed to initialize SCIM" << endl;
        return RETVAL_FAILED;
    }

    dout (7) << "Initializing the interruption listener..." << endl;
    interruption_listener = InterruptionListener::alloc ();
    if (interruption_listener == NULL) {
        cerr << "Failed to allocate an interruption listener" << endl;
        return RETVAL_FAILED;
    }

    dout (7) << "Initializing the imcontext..." << endl;
    imcontext = IMContext::alloc (this);
    if (imcontext == NULL) {
        cerr << "Failed to allocate a imcontext" << endl;
        return RETVAL_FAILED;
    }

    dout (7) << "Initializing the display..." << endl;
    display = Display::alloc (this);
    if (display == NULL) {
        cerr << "Failed to allocate a display" << endl;
        return RETVAL_FAILED;
    }

    dout (7) << "Initializing the panel..." << endl;
    panel = Panel::alloc (this);
    if (panel == NULL) {
        cerr << "Failed to allocate a panel" << endl;
        return RETVAL_FAILED;
    }

    dout (7) << "Initializing the key event reader..." << endl;
    key_event_reader = KeyEventReader::alloc (this);
    if (key_event_reader == NULL) {
        cerr << "Failed to allocate a key event reader" << endl;
        return RETVAL_FAILED;
    }

    dout (7) << "Initializing the client process..." << endl;
    client = Client::alloc (this);
    if (imcontext == NULL) {
        cerr << "Failed to allocate a client" << endl;
        return RETVAL_FAILED;
    }

    dout (7) << "Initializing the helper manager..." << endl;
    helper_manager = HelperManager::alloc (this);
    if (helper_manager == NULL) {
        cerr << "Failed to allocate a helper manager" << endl;
        return RETVAL_FAILED;
    }
    
    dout (7) << "Initializing the stdin reader..." << endl;
    stdin_reader = StdinReader::alloc (this);
    if (stdin_reader == NULL) {
        cerr << "Failed to allocate a stdin reader" << endl;
        return RETVAL_FAILED;
    }

    event_listeners.push_back (interruption_listener);
    event_listeners.push_back (key_event_reader);
    event_listeners.push_back (client);
    event_listeners.push_back (helper_manager);
    event_listeners.push_back (stdin_reader);
    event_listeners.push_back (display);

    if (imcontext != NULL) {
        bool enabled_by_default = false;
        if (scim_config->read (String (SCIM_CONFIG_FRONTEND_SHARED_INPUT_METHOD), false)) {
            enabled_by_default = scim_config->read (String (SCIM_CONFIG_FRONTEND_IM_OPENED_BY_DEFAULT), false);
        }
        imcontext->set_enabled (enabled_by_default);
    }

    return RETVAL_SUCCEEDED;
}

retval_t ScimConsoleImpl::finalize ()
{
    for (list<EventListener*>::iterator i = event_listeners.begin (); i != event_listeners.end (); ++i) {
        EventListener *event_listener = *i;
        delete event_listener;
    }
    event_listeners.clear ();

    key_event_reader = NULL;
    interruption_listener = NULL;
    client = NULL;
    helper_manager = NULL;
    stdin_reader = NULL;
    display = NULL;

    if (imcontext != NULL) {
        delete imcontext;
        imcontext = NULL;
    }

    if (panel != NULL) {
        delete panel;
        panel = NULL;
    }

    finalize_scim ();

    dout (9) << "Finalize, done" << endl;

    return RETVAL_SUCCEEDED;
}

retval_t ScimConsoleImpl::initialize_scim ()
{
    dout (7) << "Initializing SCIM..." << endl;

    // Get system language.
    scim_language = scim_get_locale_language (scim_get_current_locale ());

    // Get imengine modules
    vector<String> imengine_module_names;
    scim_get_imengine_module_list (imengine_module_names);
    if (find (imengine_module_names.begin (), imengine_module_names.end (), "socket") == imengine_module_names.end ()) {
        cerr << "There is no socket frontend of IMEngines for SCIM..." << endl;
        return RETVAL_FAILED;
    }

    // Get config modules
    vector<String> config_module_names;
    scim_get_config_module_list (config_module_names);

    if (find (config_module_names.begin (), config_module_names.end (), "socket") == config_module_names.end ()) {
        cerr << "There is no socket frontend of config module for SCIM...";
        return RETVAL_FAILED;
    }

    // If there is no socket frontend running, launch one.
    if (!is_socket_frontend_ready ()) {
        const String server_config_module_name = scim_global_config_read (SCIM_GLOBAL_CONFIG_DEFAULT_CONFIG_MODULE, String ("simple"));
        char *new_argv [] = {"--no-stay","-d", NULL};
        scim_launch (true, server_config_module_name.c_str (), "all", "socket", new_argv);

        // Wait until the connection is established.
        for (int i = 0; i < 100; ++i) {
            if (is_socket_frontend_ready ()) {
                break;
            } else if (i < 99) {
                usleep (100000);
            } else {
                cerr << "Cannot establish the socket connection..." << endl;
                return RETVAL_FAILED;
            }
        }
    }

    // load config module
    dout (7) << "Loading Config module...: socket" << endl;
    scim_config_module = new ConfigModule ("socket");

    // Create config instance
    if (scim_config_module != NULL && scim_config_module->valid ()) {
        scim_config = scim_config_module->create_config ();
    } else {
        cerr << "Cannot load the socket config module..." << endl;
        return RETVAL_FAILED;
    }

    // create backend
    scim_backend = new CommonBackEnd (scim_config, imengine_module_names);

    slot_reload_config (scim_config);
    scim_config->signal_connect_reload (slot (this, &ScimConsoleImpl::slot_reload_config));

    dout (7) << "Initialize SCIM, done" << endl;
    return RETVAL_SUCCEEDED;
}

retval_t ScimConsoleImpl::finalize_scim ()
{
    dout (7) << "Finalizing SCIM..." << endl;

    if (scim_backend != NULL)
        scim_backend.reset ();
    if (scim_config != NULL)
        scim_config.reset ();

    if (scim_config_module != NULL) {
        delete scim_config_module;
        scim_config_module = NULL;
    }

    dout (7) << "Finalize SCIM, done" << endl;
    return RETVAL_SUCCEEDED;
}

void ScimConsoleImpl::quit (int new_retval)
{
    active = false;
    retval = new_retval;
}

void ScimConsoleImpl::abort ()
{
    finalize ();
    exit (-1);
}

int ScimConsoleImpl::get_retval () const
{
    return retval;
}

void ScimConsoleImpl::interrupt (const timeval &time)
{
    interruption_listener->interrupt (time);
}

void ScimConsoleImpl::interrupt ()
{
    timeval time;
    time.tv_sec = 0;
    time.tv_usec = 0;
    interruption_listener->interrupt (time);
}

void ScimConsoleImpl::run ()
{
    dout (7) << "Launching the main loop..." << endl;

    while (active) {
        dout (1) << "The event loop" << endl;
        fd_set read_set;
        fd_set write_set;
        fd_set error_set;

        FD_ZERO (&read_set);
        FD_ZERO (&write_set);
        FD_ZERO (&error_set);

        dout (1) << "Preparing for select ()..." << endl;

        int max_fd = -1;
        for (list<EventListener*>::iterator i = event_listeners.begin (); i != event_listeners.end ();) {
            EventListener *event_listener = *i;
            const event_type_t triggers = event_listener->get_triggers ();
            const int fd = event_listener->get_fd ();

            if (fd < 0) {
                if ((triggers & EVENT_ERROR) && !event_listener->handle_event (EVENT_ERROR)) {
                    dout (5) << "A client has been taken off by the invalid fd" << endl;
                    delete event_listener;
                    i = event_listeners.erase (i);
                    continue;
                }
            } else {
                dout (1) << "Registering fd (" << fd << ") as ";
                if (fd > max_fd) max_fd = fd;
                if (triggers & EVENT_READ) {
                    dout (1) << " EVENT_READ";
                    FD_SET (fd, &read_set);
                }
                if (triggers & EVENT_WRITE) {
                    dout (1) << " EVENT_WRITE";
                    FD_SET (fd, &write_set);
                }
                if (triggers & EVENT_ERROR) {
                    dout (1) << " EVENT_ERROR";
                    FD_SET (fd, &error_set);
                }
                if (triggers & !EVENT_INTERRUPT) {
                    dout (1) << " EVENT_NONE" << endl;
                } else {
                    dout (1) << endl;
                }
            }
            ++i;
        }
        dout (1) << "select ()" << endl;

        if (max_fd < 0)
            continue;

        int select_retval;
        timeval remaining_time;
        remaining_time.tv_sec = 0;
        remaining_time.tv_usec = 0;
        if (interruption_listener->has_interruption ())
            remaining_time = interruption_listener->get_remaining_time ();
        
        if (remaining_time.tv_sec == 0 && remaining_time.tv_usec == 0) {
            select_retval = select (max_fd + 1, &read_set, &write_set, &error_set, NULL);
        } else {
            select_retval = select (max_fd + 1, &read_set, &write_set, &error_set, &remaining_time);
        }
        dout (1) << "Returned from select ()" << endl;

        if (select_retval < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                cerr << "An error occurred in selecting the sockets: " << strerror (errno) << endl;
                cerr << "remaining time: " << remaining_time.tv_sec << ":" << remaining_time.tv_usec << endl;
                abort ();
            }
        } else if (select_retval == 0) {
            continue;
        } else {
            const bool is_interrupted = interruption_listener->has_interruption ();
            interruption_listener->clear_interruption ();

            for (list<EventListener*>::iterator i = event_listeners.begin (); i != event_listeners.end ();) {
                EventListener *event_listener = *i;
                const int fd = event_listener->get_fd ();

                const event_type_t triggers = event_listener->get_triggers ();
                event_type_t events = EVENT_NONE;

                dout (1) << "Invoke (" << fd << ")";
                if (FD_ISSET (fd, &read_set) && (triggers & EVENT_READ)) {
                    dout (1) << " EVENT_READ";
                    events |= EVENT_READ;
                }
                if (FD_ISSET (fd, &write_set) && (triggers & EVENT_WRITE)) {
                    dout (1) << " EVENT_WRITE";
                    events |= EVENT_WRITE;
                }
                if (FD_ISSET (fd, &error_set) && (triggers & EVENT_ERROR)) {
                    dout (1) << " EVENT_ERROR";
                    events |= EVENT_ERROR;
                }
                if (is_interrupted && (triggers & EVENT_INTERRUPT)) {
                    dout (1) << " EVENT_INTERRUPT";
                    events |= EVENT_INTERRUPT;
                }
                if (events == EVENT_NONE) {
                    dout (1) << " EVENT_NONE" << endl;
                } else {
                    dout (1) << endl;
                }

                if (events != EVENT_NONE && !event_listener->handle_event (events)) {
                    dout (5) << "A client has been taken off" << endl;
                    delete event_listener;
                    i = event_listeners.erase (i);
                } else {
                    ++i;
                }
            }
        }
    }
}

ConfigPointer ScimConsoleImpl::get_scim_config ()
{
    return scim_config;
}

BackEndPointer ScimConsoleImpl::get_scim_backend ()
{
    return scim_backend;
}

const String &ScimConsoleImpl::get_scim_language () const
{
    return scim_language;
}

const KeyboardLayout &ScimConsoleImpl::get_scim_keyboard_layout () const
{
    return scim_keyboard_layout;
}

const Display &ScimConsoleImpl::get_display () const
{
    return *display;
}

Panel &ScimConsoleImpl::get_panel ()
{
    return *panel;
}

const Panel &ScimConsoleImpl::get_panel () const
{
    return *panel;
}

void ScimConsoleImpl::save_config ()
{
    IMContext *imcontext = IMContext::get_focused_imcontext ();
    scim_config->write (String (SCIM_CONFIG_FRONTEND_IM_OPENED_BY_DEFAULT), imcontext->is_enabled ());
}

void ScimConsoleImpl::load_config ()
{
    scim_config->reload ();
}

bool ScimConsoleImpl::process_hotkey (const KeyEvent &key_event)
{
    IMContext *imcontext = IMContext::get_focused_imcontext ();

    panel_hotkey_matcher.push_key_event (key_event);
    const int panel_hotkey_action = panel_hotkey_matcher.get_match_result ();
    if (panel_hotkey_action == PANEL_HOTKEY_MENU) {
        if (panel->process_hotkey_menu ())
            return true;
    } else if (panel_hotkey_action == PANEL_HOTKEY_CANCEL) {
        if (panel->process_hotkey_cancel ())
            return true;
    } else if (panel_hotkey_action == PANEL_HOTKEY_OK) {
        if (panel->process_hotkey_ok ())
            return true;
    } else if (panel_hotkey_action == PANEL_HOTKEY_PREVIOUS) {
        if (panel->process_hotkey_previous ())
            return true;
    } else if (panel_hotkey_action == PANEL_HOTKEY_NEXT) {
        if (panel->process_hotkey_next ())
            return true;
    } else if (panel_hotkey_action == PANEL_HOTKEY_MENU) {
        if (panel->process_hotkey_menu ())
            return true;
    }

    scim_frontend_hotkey_matcher.push_key_event (key_event);
    const FrontEndHotkeyAction hotkey_action = scim_frontend_hotkey_matcher.get_match_result ();

    if (hotkey_action == SCIM_FRONTEND_HOTKEY_TRIGGER) {
        const bool new_status = !imcontext->is_enabled ();
        imcontext->set_enabled (new_status);
        save_config ();
        panel->update ();
        return true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_ON) {
        const bool new_status = true;
        imcontext->set_enabled (new_status);
        save_config ();
        panel->update ();
        return true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_OFF) {
        const bool new_status = false;
        imcontext->set_enabled (new_status);
        save_config ();
        panel->update ();
        return true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_NEXT_FACTORY) {
        imcontext->rotate_imengines (true);
        panel->update ();
        return true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_PREVIOUS_FACTORY) {
        imcontext->rotate_imengines (false);
        panel->update ();
        return true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_SHOW_FACTORY_MENU) {
        //request_factory_menu ();
        panel->update ();
        return true;
    } else {
        scim_imengine_hotkey_matcher.push_key_event (key_event);
        if (scim_imengine_hotkey_matcher.is_matched ()) {
            const String factory_uuid = scim_imengine_hotkey_matcher.get_match_result ();
            imcontext->change_imengine_by_uuid (factory_uuid);
            panel->update ();
            return true;
        }
    }

    return false;
}

void ScimConsoleImpl::slot_reload_config (const ConfigPointer &config)
{
    scim_frontend_hotkey_matcher.load_hotkeys (scim_config);
    scim_imengine_hotkey_matcher.load_hotkeys (scim_config);

    panel_hotkey_matcher.clear ();
    vector<String> strings;
    KeyEventList hotkeys;

    hotkeys.clear ();
    if (config->read ("/FrontEnd/Console/Hotkeys/Menu", &strings)) {
        for (vector<String>::iterator i = strings.begin (); i != strings.end (); ++i) {
            const String &string = *i;
            KeyEventList keys;
            if (scim_string_to_key_list (keys, string)) {
                for (KeyEventList::iterator j = keys.begin (); j != keys.end (); ++j) {
                    KeyEvent &key = *j;
                    hotkeys.push_back (key);
                }
            }
        }
    }
    if (hotkeys.empty ()) {
        hotkeys.push_back (KeyEvent (SCIM_KEY_F2));
        hotkeys.push_back (KeyEvent (SCIM_KEY_Menu));
    }
    panel_hotkey_matcher.add_hotkeys (hotkeys, PANEL_HOTKEY_MENU);

    hotkeys.clear ();
    if (config->read ("/FrontEnd/Console/Hotkeys/Cancel", &strings)) {
        for (vector<String>::iterator i = strings.begin (); i != strings.end (); ++i) {
            const String &string = *i;
            KeyEventList keys;
            if (scim_string_to_key_list (keys, string)) {
                for (KeyEventList::iterator j = keys.begin (); j != keys.end (); ++j) {
                    KeyEvent &key = *j;
                    hotkeys.push_back (key);
                }
            }
        }
    }
    if (hotkeys.empty ()) {
        hotkeys.push_back (KeyEvent (SCIM_KEY_Escape));
        hotkeys.push_back (KeyEvent (SCIM_KEY_BackSpace));
        hotkeys.push_back (KeyEvent (SCIM_KEY_Delete));
    }
    panel_hotkey_matcher.add_hotkeys (hotkeys, PANEL_HOTKEY_CANCEL);

    hotkeys.clear ();
    if (config->read ("/FrontEnd/Console/Hotkeys/OK", &strings)) {
        for (vector<String>::iterator i = strings.begin (); i != strings.end (); ++i) {
            const String &string = *i;
            KeyEventList keys;
            if (scim_string_to_key_list (keys, string)) {
                for (KeyEventList::iterator j = keys.begin (); j != keys.end (); ++j) {
                    KeyEvent &key = *j;
                    hotkeys.push_back (key);
                }
            }
        }
    }
    if (hotkeys.empty ()) {
        hotkeys.push_back (KeyEvent (SCIM_KEY_Return));
        hotkeys.push_back (KeyEvent (SCIM_KEY_space));
    }
    panel_hotkey_matcher.add_hotkeys (hotkeys, PANEL_HOTKEY_OK);

    hotkeys.clear ();
    if (config->read ("/FrontEnd/Console/Hotkeys/Previous", &strings)) {
        for (vector<String>::iterator i = strings.begin (); i != strings.end (); ++i) {
            const String &string = *i;
            KeyEventList keys;
            if (scim_string_to_key_list (keys, string)) {
                for (KeyEventList::iterator j = keys.begin (); j != keys.end (); ++j) {
                    KeyEvent &key = *j;
                    hotkeys.push_back (key);
                }
            }
        }
    }
    if (hotkeys.empty ()) {
        hotkeys.push_back (KeyEvent (SCIM_KEY_Left));
        hotkeys.push_back (KeyEvent (SCIM_KEY_Up));
    }
    panel_hotkey_matcher.add_hotkeys (hotkeys, PANEL_HOTKEY_PREVIOUS);
    
    hotkeys.clear ();
    if (config->read ("/FrontEnd/Console/Hotkeys/Next", &strings)) {
        for (vector<String>::iterator i = strings.begin (); i != strings.end (); ++i) {
            const String &string = *i;
            KeyEventList keys;
            if (scim_string_to_key_list (keys, string)) {
                for (KeyEventList::iterator j = keys.begin (); j != keys.end (); ++j) {
                    KeyEvent &key = *j;
                    hotkeys.push_back (key);
                }
            }
        }
    }
    if (hotkeys.empty ()) {
        hotkeys.push_back (KeyEvent (SCIM_KEY_Right));
        hotkeys.push_back (KeyEvent (SCIM_KEY_Down));
    }
    panel_hotkey_matcher.add_hotkeys (hotkeys, PANEL_HOTKEY_NEXT);

    // Get keyboard layout setting
    // Flush the global config first, in order to load the new configs from disk.
    scim_global_config_flush ();

    scim_keyboard_layout = scim_get_default_keyboard_layout ();
}

/* Event Handlers */
bool ScimConsoleImpl::process_key_event (const KeyEvent &key_event, bool hotkeys_enabled)
{
    dout (6) << "ScimConsoleImpl::process_key_event ()" << endl;
    if (hotkeys_enabled && process_hotkey (key_event)) {
        return true;
    } else if (imcontext->process_key_event (key_event)) {
        return true;
    } else {
        return false;
    }
}

void ScimConsoleImpl::forward_key_event (const KeyEvent &key_event)
{
    client->forward_key_event (key_event);
}


void ScimConsoleImpl::forward_pty_input (const String &input_string)
{
    client->forward_string (input_string);
}

void ScimConsoleImpl::process_pty_output (const String &output_string)
{
    dout (6) << "ScimConsoleImpl::process_pty_output ()" << endl;
    display->forward_pty_output (output_string);
}

void ScimConsoleImpl::update_display ()
{
    display->update ();
}

void ScimConsoleImpl::beep ()
{
    display->beep ();
}

void ScimConsoleImpl::register_properties (const PropertyList &properties)
{
    panel->register_properties (properties);
    panel->update ();
}

void ScimConsoleImpl::update_property (const Property &property)
{
    panel->update_property (property);
    panel->update ();
}

void ScimConsoleImpl::register_helper_properties (const PropertyList &properties)
{
    panel->register_properties (properties);
    panel->update ();
}

void ScimConsoleImpl::update_helper_property (const Property &property)
{
    panel->update_property (property);
    panel->update ();
}

void ScimConsoleImpl::accept_helper (Helper *helper)
{
    event_listeners.push_back (helper);
}

void ScimConsoleImpl::start_helper (const String &helper_uuid)
{
    helper_manager->launch_helper (helper_uuid);
}

void ScimConsoleImpl::stop_helper (const String &helper_uuid)
{
    Helper *helper = helper_manager->find_helper (helper_uuid);
    if (helper != NULL)
        helper->close ();
}

void ScimConsoleImpl::send_helper_event (const String &helper_uuid, const Transaction &trans)
{
    Helper *helper = helper_manager->find_helper (helper_uuid);
    if (helper != NULL)
        helper->send_helper_event (trans);
}

/* Main */
using namespace scim_console;

int main (int argc, const char **argv)
{
    ScimConsoleImpl *scim_console = new ScimConsoleImpl ();
    if (scim_console->initialize ()) {
        cerr << "Failed to initialize SCIM Console" << endl;
        delete scim_console;
        return -1;
    } else {
        scim_console->run ();
        const int retval = scim_console->get_retval ();
        delete scim_console;

        return retval;
    }
}
