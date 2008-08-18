/*
  Copyright (C) 2004-2005 LiuCougar <liuspider@users.sourceforge.net>
*/

#include "qsciminputcontext.h"

#include <iostream>

#if defined(Q_WS_X11)
#include <X11/Xlib.h>
#include <x11/scim_x11_utils.h>
#include <QX11Info>
const int XKeyPress = KeyPress;
const int XKeyRelease = KeyRelease;
#undef KeyPress
#undef KeyRelease
#endif

#include <QTextCharFormat>
#include <qevent.h>
#include <qapplication.h>
#include <qmutex.h>
#include <qsocketnotifier.h>
#include <qdatetime.h>
// #undef Q_WS_X11

#if !defined(Q_WS_X11)
#ifndef SCIM_QT_IMMODULE_USE_KDE
  #include "keyserver_x11.h"
#else
  #include <kkeynative.h>
#endif
#endif

namespace scim {

typedef std::map<int, QScimInputContext *> ScimInputContextRepository;

// Class to hold all global variables.
class QScimInputContextGlobal
{
public:
    FrontEndHotkeyMatcher    frontend_hotkey_matcher;
    IMEngineHotkeyMatcher    imengine_hotkey_matcher;
    int                      valid_key_mask;
    KeyboardLayout           keyboard_layout;

    ConfigModule            *config_module;
    ConfigPointer            config;
    BackEndPointer           backend;

    IMEngineInstancePointer  default_instance;

    PanelIOReceiver          panel_io_receiver;
    QSocketNotifier         *socket_read_notifier;

    QScimInputContext       *focused_ic;

    bool                     use_preedit;
    bool                     shared_input_method;

    int                      instance_count;
    int                      context_count;

    IMEngineFactoryPointer   fallback_factory;
    IMEngineInstancePointer  fallback_instance;

    PanelClient             *panel_client;

    bool                     initialized;
    bool                     panel_initialized;
    bool                     panel_exited;

    QMutex                   mutex;

    String                   language;

#if defined(Q_WS_X11)
    Display                 *display;
#endif

    ScimInputContextRepository input_contexts;

public:
    QScimInputContextGlobal ()
        : valid_key_mask ((int)SCIM_KEY_AllMasks),
          keyboard_layout (SCIM_KEYBOARD_Unknown),
          config_module (0),
          socket_read_notifier (0),
          focused_ic (0),
          use_preedit (true),
          shared_input_method (false),
          instance_count (0),
          context_count (0),
          panel_client (0),
          initialized (false),
          panel_initialized (false),
          panel_exited (false)
    {
        SCIM_DEBUG_FRONTEND (1) << "QScimInputContextGlobal::QScimInputContextGlobal ()\n";

#if defined(Q_WS_X11)
        display = QX11Info::display ();
#endif
    }

    ~QScimInputContextGlobal () {
        SCIM_DEBUG_FRONTEND (1) << "QScimInputContextGlobal::~QScimInputContextGlobal ()\n";

        if (initialized) finalize ();
    }

    void initialize ();
    void finalize ();

    bool panel_initialize ();
    void clean_socket_notifier ();

private:
    void fallback_commit_string_cb (IMEngineInstanceBase *si, const WideString &str);
    void panel_slot_reload_config  (int context);
    void reload_config_callback    (const ConfigPointer &config);
    bool check_socket_frontend     ();
};

static QScimInputContextGlobal global;

//------------------helper functions-----------------
#if !defined(Q_WS_X11)
static KeyEvent
keyevent_qt_to_scim (const QKeyEvent *e)
{
    static int pressedModifier = 0;
    static int curModKeySym = 0;
    static int lastModKeySym = 0;

    int qtKeyCode = e->key();
    int curusedpressedModifier = 0;

    switch (qtKeyCode) {
        case Qt::Key_Control:
            curModKeySym = SCIM_KEY_Control_L;
            break;
        case Qt::Key_Alt:
            curModKeySym = SCIM_KEY_Alt_L;
            break;
        case Qt::Key_Shift:
            curModKeySym = SCIM_KEY_Shift_L;
            break;
        default:
            curModKeySym = 0;
    }

    if(lastModKeySym) {
        switch (lastModKeySym) {
            case SCIM_KEY_Control_L:
            case SCIM_KEY_Control_R:
                pressedModifier |= SCIM_KEY_ControlMask;
                break;
            case SCIM_KEY_Alt_L:
            case SCIM_KEY_Alt_R:
                pressedModifier |= SCIM_KEY_AltMask;
                break;
            case SCIM_KEY_Shift_L:
            case SCIM_KEY_Shift_R:
                pressedModifier |= SCIM_KEY_ShiftMask;
                break;
        }
    }

    int sym;
    #ifndef SCIM_QT_IMMODULE_USE_KDE
    sym = keyQtToSym(qtKeyCode);
    #else
    KKeyNative nk = KKey(qtKeyCode);
    sym = nk.sym();
    #endif

    if(!sym) sym = curModKeySym;

    curusedpressedModifier = pressedModifier;
    if(e->type() == QEvent::KeyRelease) {
        if(curModKeySym) {
            switch (curModKeySym) {
                case SCIM_KEY_Control_L:
                case SCIM_KEY_Control_R:
                    pressedModifier &= ~SCIM_KEY_ControlMask;
                    break;
                case SCIM_KEY_Alt_L:
                case SCIM_KEY_Alt_R:
                    pressedModifier &= ~SCIM_KEY_AltMask;
                    break;
                case SCIM_KEY_Shift_L:
                case SCIM_KEY_Shift_R:
                    pressedModifier &= ~SCIM_KEY_ShiftMask;
                    break;
            }

            if(pressedModifier) curusedpressedModifier = pressedModifier;
            curModKeySym = lastModKeySym = 0;
        }
    } else {
        lastModKeySym = curModKeySym;
    }

    return KeyEvent (sym, (e->type() == QEvent::KeyPress)?
                    (curusedpressedModifier & ~ SCIM_KEY_ReleaseMask):
                    (curusedpressedModifier | SCIM_KEY_ReleaseMask));
}

static QKeyEvent
keyevent_scim_to_qt (const scim::KeyEvent & scimkey)
{
    QEvent::Type type = scimkey.is_key_press() ? QEvent::KeyPress : QEvent::KeyRelease;
    int keyQt;
    symToKeyQt(scimkey.code, keyQt);
    SCIM_DEBUG_FRONTEND(0) << "keyevent_scim_to_qt.. " << scimkey.code << " " << keyQt << "\n";
    int state = 0;
    if(scimkey.is_alt_down()) state |= Qt::AltButton;
    if(scimkey.is_shift_down()) state |= Qt::ShiftButton;
    if(scimkey.is_control_down()) state |= Qt::ControlButton;
    if(scimkey.is_meta_down()) state |= Qt::MetaButton;

    return QKeyEvent (type, keyQt, (int)scimkey.get_ascii_code(), state);
}
#endif

// Implementation of QScimInputContextGlobal;
void
QScimInputContextGlobal::initialize()
{
    SCIM_DEBUG_FRONTEND(1) << "QScimInputContextGlobal::initialize ()\n";

    mutex.lock();

    if(initialized) {
        SCIM_DEBUG_FRONTEND(2) << "QScimInputContextGlobal::initialize (): already initialized!\n";
        mutex.unlock ();
        return;
    }

    if (panel_exited) {
        SCIM_DEBUG_FRONTEND(2) << "QScimInputContextGlobal::initialize (): Panel has been exited, impossible to initialize!\n";
        mutex.unlock ();
        return;
    }

    initialized = true;

    String config_module_name ="socket";

    IMEngineFactoryPointer  factory;

    std::vector<String>     config_list;
    std::vector<String>     engine_list;
    std::vector<String>     load_engine_list;

    std::vector<String>     debug_mask_list;

    size_t                  i;

    bool                    manual = false;

    bool                    socket = true;

    // Get debug info
    {
        char *str;
        int debug_verbose = 0;

        str = getenv ("QT_IM_SCIM_DEBUG_VERBOSE");

        if (str != NULL)
            debug_verbose = atoi (str);

        DebugOutput::set_verbose_level (debug_verbose);

        str = getenv ("QT_IM_SCIM_DEBUG_MASK");

        if (str != NULL) {
            scim_split_string_list (debug_mask_list, String (str), ',');
            if (debug_mask_list.size ()) {
                DebugOutput::disable_debug (SCIM_DEBUG_AllMask);
                for (i=0; i<debug_mask_list.size (); i++)
                    DebugOutput::enable_debug_by_name (debug_mask_list [i]);
            }
        }
    }

    // Get system language.
    language = scim_get_locale_language (scim_get_current_locale ());

    //get modules list
    scim_get_imengine_module_list (engine_list);
    scim_get_config_module_list (config_list);

    if (std::find (engine_list.begin (), engine_list.end (), "socket") == engine_list.end () ||
        std::find (config_list.begin (), config_list.end (), "socket") == config_list.end ())
        socket = false;

    //Use kconfig config module as default if available.
    if (config_list.size ()) {
        config_module_name = scim_global_config_read (SCIM_GLOBAL_CONFIG_DEFAULT_CONFIG_MODULE, String ("kconfig"));
        if (std::find (config_list.begin (), config_list.end (), config_module_name) == config_list.end ())
            config_module_name = config_list [0];
    } else {
        config_module_name = "dummy";
    }

    if (config_module_name != "kconfig") {
        std::cerr << "WARNING: please edit ~/.scim/global and change /DefaultConfigModule to kconfig\n";
    }

    const char *engine_list_str = getenv ("QT_IM_SCIM_IMENGINE_MODULES");
    if (engine_list_str != NULL) {
        std::vector <String> spec_engine_list;
        scim_split_string_list (spec_engine_list, engine_list_str, ',');

        load_engine_list.clear ();

        for (size_t i = 0; i < spec_engine_list.size (); ++i)
            if (std::find (engine_list.begin (), engine_list.end (), spec_engine_list [i]) != engine_list.end ())
                load_engine_list.push_back (spec_engine_list [i]);

        manual = true;
    }

    // If you try to use the socket feature manually,
    // then let you do it by yourself.
    if (config_module_name == "socket" ||
        std::find (load_engine_list.begin (), load_engine_list.end (), "socket") != load_engine_list.end ())
        socket = false;

    // Try to start a SCIM SocketFrontEnd process if necessary.
    if (scim_get_default_socket_frontend_address () != scim_get_default_socket_imengine_address () &&
        scim_get_default_socket_frontend_address () != scim_get_default_socket_config_address ())
        socket = false;

    if (socket) {
        // If no Socket FrontEnd is running, then launch one.
        // And set manual to false.
        if (!check_socket_frontend ()) {
            char *new_argv [] = { const_cast<char*>("--no-stay"), 0 };
            scim_launch (true,
                         config_module_name,
                         (load_engine_list.size () ? scim_combine_string_list (load_engine_list, ',') : "all"),
                         "socket",
                         new_argv);
            manual = false;
        }

        // If there is one Socket FrontEnd running and it's not manual mode,
        // then just use this Socket Frontend.
        if (!manual) {
            for (int i = 0; i < 100; ++i) {
                if (check_socket_frontend ()) {
                    config_module_name = "socket";
                    load_engine_list.clear ();
                    load_engine_list.push_back ("socket");
                    break;
                }
                scim_usleep (100000);
            }
        }
    }

    //load config module
    SCIM_DEBUG_FRONTEND(1) << "Loading Config module: " << config_module_name << "...\n";
    config_module = new ConfigModule (config_module_name);

    //create config instance
    if (config_module != NULL && config_module->valid ()) {
        config = config_module->create_config ();
    } else {
        SCIM_DEBUG_FRONTEND(1) << "Config module cannot be loaded, using dummy Config.\n";

        if (config_module) delete config_module;
        config_module = NULL;

        config = new DummyConfig ();
        config_module_name = "dummy";
    }

    // create backend
    backend = new CommonBackEnd (config, load_engine_list.size () ? load_engine_list : engine_list);

    if (backend.null ()) {
        std::cerr << "Qt IM Module SCIM: Cannot create BackEnd Object!\n";
        exit (-1);
    }

    if (!config.null () && config->valid ()) {
        reload_config_callback(config);
        config->signal_connect_reload (slot (this, &QScimInputContextGlobal::reload_config_callback));
    } else {
        config.reset ();
    }

    fallback_factory = backend->get_factory (SCIM_COMPOSE_KEY_FACTORY_UUID);
    if (fallback_factory.null ()) fallback_factory = new DummyIMEngineFactory ();

    fallback_instance = fallback_factory->create_instance ("UTF-8", -1);
    fallback_instance->signal_connect_commit_string (slot (this, &QScimInputContextGlobal::fallback_commit_string_cb));

    // Initialize Panel client and Attach signals.
    if(!panel_client)
      panel_client = new PanelClient;

    panel_client->signal_connect_reload_config
        (slot (this, &QScimInputContextGlobal::panel_slot_reload_config));
    panel_client->signal_connect_exit
        (slot (&QScimInputContext::panel_slot_exit));
    panel_client->signal_connect_update_lookup_table_page_size
        (slot (&QScimInputContext::panel_slot_update_lookup_table_page_size));
    panel_client->signal_connect_lookup_table_page_up
        (slot (&QScimInputContext::panel_slot_lookup_table_page_up));
    panel_client->signal_connect_lookup_table_page_down
        (slot (&QScimInputContext::panel_slot_lookup_table_page_down));
    panel_client->signal_connect_trigger_property
        (slot (&QScimInputContext::panel_slot_trigger_property));
    panel_client->signal_connect_process_helper_event
        (slot (&QScimInputContext::panel_slot_process_helper_event));
    panel_client->signal_connect_move_preedit_caret
        (slot (&QScimInputContext::panel_slot_move_preedit_caret));
    panel_client->signal_connect_select_candidate
        (slot (&QScimInputContext::panel_slot_select_candidate));
    panel_client->signal_connect_process_key_event
        (slot (&QScimInputContext::panel_slot_process_key_event));
    panel_client->signal_connect_commit_string
        (slot (&QScimInputContext::panel_slot_commit_string));
    panel_client->signal_connect_forward_key_event
        (slot (&QScimInputContext::panel_slot_forward_key_event));
    panel_client->signal_connect_request_help
        (slot (&QScimInputContext::panel_slot_request_help));
    panel_client->signal_connect_request_factory_menu
        (slot (&QScimInputContext::panel_slot_request_factory_menu));
    panel_client->signal_connect_change_factory
        (slot (&QScimInputContext::panel_slot_change_factory));

    mutex.unlock();
}

void
QScimInputContextGlobal::finalize() 
{
    SCIM_DEBUG_FRONTEND(1) << "QScimInputContextGlobal::finalize ()\n";
    mutex.lock();
    if(initialized) {
        SCIM_DEBUG_FRONTEND(1) << "Finalizing QT SCIM IMModule...\n";

        default_instance.reset ();

        ScimInputContextRepository::iterator it;

        for ( it = input_contexts.begin(); it != input_contexts.end(); ++it ) {
            if (it->second && !it->second->m_instance.null ()) {
                it->second->m_instance->set_frontend_data (static_cast <void*> (it->second));
                it->second->finalize ();
            }
        }

        fallback_instance.reset ();
        fallback_factory.reset ();
        default_instance.reset ();

        backend.reset ();
        config.reset ();

        if (config_module) {
            SCIM_DEBUG_FRONTEND(2) << " Deleting _config_module...\n";
            delete config_module;
            config_module = 0;
        }

        clean_socket_notifier();
        panel_client->close_connection ();

        delete panel_client;

        panel_client = 0;

        initialized = false;
    }
    mutex.unlock();
}

bool
QScimInputContextGlobal::panel_initialize ()
{
    SCIM_DEBUG_FRONTEND(1) << "QScimInputContextGlobal::panel_initialize ()\n";
    mutex.lock();

    if(panel_initialized) {
        SCIM_DEBUG_FRONTEND(2) << "QScimInputContextGlobal::panel_initialize (), already initialized.\n";
        mutex.unlock ();
        return true;
    }

    if(panel_exited) {
        SCIM_DEBUG_FRONTEND(2) << "QScimInputContextGlobal::panel_initialize (), Panel has been exited, impossible to initialize!\n";
        mutex.unlock ();
        return false;
    }

    String display_name (getenv ("DISPLAY"));

#if defined(Q_WS_X11)
    display_name = String (XDisplayString(display));
#endif

    if (panel_client->open_connection (config->get_name (), display_name) >= 0) {
        int fd = panel_client->get_connection_number ();

        clean_socket_notifier ();

        socket_read_notifier = new QSocketNotifier( fd, QSocketNotifier::Read );
        panel_io_receiver.connect(socket_read_notifier, SIGNAL(activated ( int )), &panel_io_receiver, SLOT(panel_iochannel_handler()));

        panel_initialized = true;
    }

    mutex.unlock();

    return panel_initialized;
}

void
QScimInputContextGlobal::reload_config_callback (const ConfigPointer & config) 
{
    SCIM_DEBUG_FRONTEND(1) << "QScimInputContextGlobal::reload_config_callback ()\n";

    if (!config.null () && config->valid ()) {
        frontend_hotkey_matcher.load_hotkeys (config);
        imengine_hotkey_matcher.load_hotkeys (config);

        KeyEvent key;
        scim_string_to_key (key,
            config->read (String (SCIM_CONFIG_HOTKEYS_FRONTEND_VALID_KEY_MASK),
                          String ("Shift+Control+Alt+Meta")));

        valid_key_mask = (key.mask > 0) ? key.mask : (int) SCIM_KEY_AllMasks;
        valid_key_mask |= SCIM_KEY_ReleaseMask;

        use_preedit = config->read(SCIM_CONFIG_FRONTEND_ON_THE_SPOT, use_preedit);

        shared_input_method = config->read (String (SCIM_CONFIG_FRONTEND_SHARED_INPUT_METHOD), shared_input_method);

        // Get keyboard layout setting
        // Flush the global config first, in order to load the new configs from disk.
        scim_global_config_flush ();
        keyboard_layout = scim_get_default_keyboard_layout ();
    }
}

bool
QScimInputContextGlobal::check_socket_frontend ()
{
    SocketAddress address;
    SocketClient client;

    uint32 magic;

    address.set_address (scim_get_default_socket_frontend_address ());

    if (!client.connect (address))
        return false;

    if (!scim_socket_open_connection (magic,
                                      String ("ConnectionTester"),
                                      String ("SocketFrontEnd"),
                                      client,
                                      1000)) {
        return false;
    }

    return true;
}

void
QScimInputContextGlobal::clean_socket_notifier ()
{
    if(socket_read_notifier) {
        socket_read_notifier->setEnabled(false);
        socket_read_notifier->deleteLater();
        socket_read_notifier = 0;
    }
}

void
QScimInputContextGlobal::panel_slot_reload_config (int /*context*/)
{
    SCIM_DEBUG_FRONTEND(1) << "QScimInputContextGlobal::panel_slot_reload_config ()\n";
    config->reload ();
}

void
QScimInputContextGlobal::fallback_commit_string_cb (IMEngineInstanceBase  * /*si*/,
                                                    const WideString      &str)
{
    if (focused_ic)
        focused_ic->commit_string (QString::fromUtf8(utf8_wcstombs (str).c_str ()));
}

void
PanelIOReceiver::panel_iochannel_handler ()
{
    SCIM_DEBUG_FRONTEND(1) << "PanelIOReceiver::panel_iochannel_handler ()\n";
    if(!global.panel_client->filter_event ()) {
        global.panel_client->close_connection ();
        global.panel_initialized = false;
        global.panel_initialize ();
    }
}

// Implementation of QScimInputContext;
QScimInputContext::QScimInputContext()
    : QInputContext(),
      m_id (global.context_count ++),
      m_preedit_caret (0),
      m_preedit_sellen (0),
      m_is_on (false),
      m_shared_instance (false)
{
    SCIM_DEBUG_FRONTEND(1) << "QScimInputContext::QScimInputContext ()\n";

    global.input_contexts [m_id] = this;

    if (global.panel_exited) return;

    if (!global.initialized) global.initialize();

    if (global.backend.null ()) return;

    if (global.shared_input_method && !global.default_instance.null ()) {
        m_instance= global.default_instance;
        SCIM_DEBUG_FRONTEND(2) << "use default instance: " << m_instance->get_id () << " " << m_instance->get_factory_uuid () << "\n";
    }

    // Not in "shared input method" mode, or no default instance, create an instance.
    if (m_instance.null ()) {
        IMEngineFactoryPointer factory = global.backend->get_default_factory (global.language, "UTF-8");
        if (factory.null ()) return;
        m_instance = factory->create_instance ("UTF-8", global.instance_count++);
        if (m_instance.null ()) return;
        attach_instance (m_instance);
        SCIM_DEBUG_FRONTEND(2) << "create new instance: " << m_instance->get_id () << " " << m_instance->get_factory_uuid () << "\n";
    }

    // If "shared input method" mode is enabled, and there is no default instance,
    // then store this instance as default one.
    if (global.shared_input_method && global.default_instance.null ()) {
        SCIM_DEBUG_FRONTEND(2) << "update default instance.\n";
        global.default_instance = m_instance;
    }

    m_shared_instance = global.shared_input_method;

    if (global.shared_input_method)
        m_is_on = global.config->read (String (SCIM_CONFIG_FRONTEND_IM_OPENED_BY_DEFAULT), m_is_on);

    global.panel_client->prepare (m_id);
    global.panel_client->register_input_context (m_id, m_instance->get_factory_uuid ());
    set_ic_capabilities ();
    global.panel_client->send ();

    SCIM_DEBUG_FRONTEND(2) << "input context created: id = " << m_id << "\n";
}

QScimInputContext::~QScimInputContext()
{
    SCIM_DEBUG_FRONTEND(1) << "QScimInputContext::~QScimInputContext ()\n";

    finalize();

    if (global.input_contexts.count (m_id))
        global.input_contexts.erase (m_id);
    else
        std::cerr << "SOMETHING IS TERRIBLY WRONG! Input Context ID=" << m_id << " was not registered!\n";
}

QString
QScimInputContext::identifierName()
{
    return "scim";
}

QString
QScimInputContext::language()
{
    if (!m_instance.null () && !global.backend.null ()) {
        IMEngineFactoryPointer factory = global.backend->get_factory (m_instance->get_factory_uuid ());
        String lang = factory->get_language();
        return QString (lang.c_str());
    }
    return "C";
}

#if defined(Q_WS_X11)
bool
QScimInputContext::x11FilterEvent( QWidget * /*keywidget*/, XEvent *event )
{
    if(m_instance.null () || event->type != XKeyPress && event->type != XKeyRelease)
      return false;

    XKeyEvent & xkey = event->xkey;

    //this event is sent out by QScimInputContext
    if(xkey.send_event) {
        xkey.send_event = false;
        return false;
    }

    KeyEvent scimkey;

    scimkey = scim_x11_keyevent_x11_to_scim(global.display, xkey);

    scimkey.mask  &= global.valid_key_mask;
    scimkey.layout = global.keyboard_layout;

    return filterScimEvent(scimkey);
}
#endif

#if defined(Q_WS_X11)
bool
QScimInputContext::filterEvent( const QEvent * )
{
    return false;
}
#else
bool
QScimInputContext::filterEvent( const QEvent *event )
{
    if (m_instance.null () || (event->type() != QEvent::KeyPress && event->type() != QEvent::KeyRelease))
        return false;

    KeyEvent key = keyevent_qt_to_scim ((const QKeyEvent *)event);

    key.mask &= global.valid_key_mask;
    key.layout = global.keyboard_layout;

    return filterScimEvent(key);
}
#endif

void
QScimInputContext::reset()
{
    SCIM_DEBUG_FRONTEND(1) << "QScimInputContext::reset ()\n";

    if(m_is_on && !m_instance.null ()) {
        global.panel_client->prepare (m_id);
        m_instance->reset ();
        global.panel_client->send ();
    }

    m_preedit_caret = 0;
    m_preedit_sellen = 0;
    m_preedit_string = "";

//     QInputContext::reset();
}
bool 
QScimInputContext::isComposing() const
{
    return m_is_on;
}

void QScimInputContext::setFocusWidget( QWidget *w )
{
    QInputContext::setFocusWidget( w );

    if(w)
    {
        setFocus();
        update();   //update cursor location
    }
    else
        unsetFocus();
}

void
QScimInputContext::setFocus()
{
    SCIM_DEBUG_FRONTEND(1) << "QScimInputContext::setFocus (), this=" << this << " old focuse=" << global.focused_ic << "\n";

    if(!global.initialized || global.panel_exited || (!global.panel_initialized && !global.panel_initialize()))
        return;

    if(global.focused_ic)
        global.focused_ic->unsetFocus();

    bool need_cap = false;
    bool need_reset = false;
    bool need_reg = false;

    if(!m_instance.null ()) {
        global.panel_client->prepare (m_id);
        global.focused_ic = this;

        // Handle the "Shared Input Method" mode.
        if (global.shared_input_method) {
            SCIM_DEBUG_FRONTEND(2) << "shared input method.\n";
            IMEngineFactoryPointer factory = global.backend->get_default_factory (global.language, "UTF-8");
            if (!factory.null ()) {
                if (global.default_instance.null () || global.default_instance->get_factory_uuid () != factory->get_uuid ()) {
                    global.default_instance = factory->create_instance ("UTF-8", global.default_instance.null () ? global.instance_count++ : global.default_instance->get_id ());
                    attach_instance (global.default_instance);
                    SCIM_DEBUG_FRONTEND(2) << "create new default instance: " << global.default_instance->get_id () << " " << global.default_instance->get_factory_uuid () << "\n";
                }

                m_shared_instance = true;
                m_instance = global.default_instance;

                m_is_on= global.config->read (String (SCIM_CONFIG_FRONTEND_IM_OPENED_BY_DEFAULT), m_is_on);

                m_preedit_caret = 0;
                m_preedit_sellen = 0;
                m_preedit_string = "";

                need_cap = true;
                need_reset = true;
                need_reg = true;
            }
        } else if (m_shared_instance) {
            SCIM_DEBUG_FRONTEND(2) << "exit shared input method.\n";
            IMEngineFactoryPointer factory = global.backend->get_default_factory (global.language, "UTF-8");
            if (!factory.null ()) {
                m_instance = factory->create_instance ("UTF-8", global.instance_count++);
                m_preedit_caret = 0;
                m_preedit_sellen = 0;
                m_preedit_string = "";
                attach_instance (m_instance);
                need_cap = true;
                need_reg = true;
                m_shared_instance = false;
                SCIM_DEBUG_FRONTEND(2) << "create new instance: " << m_instance->get_id () << " " << m_instance->get_factory_uuid () << "\n";
            }
        }

        m_instance->set_frontend_data (static_cast <void*> (this));

        if (need_reg) global.panel_client->register_input_context (m_id, m_instance->get_factory_uuid ());
        if (need_cap) set_ic_capabilities ();
        if (need_reset) m_instance->reset ();

        panel_req_focus_in ();
        panel_req_update_screen ();
        panel_req_update_spot_location ();
        panel_req_update_factory_info ();

        if (m_is_on) {
            global.panel_client->turn_on (m_id);
            global.panel_client->hide_preedit_string (m_id);
            global.panel_client->hide_aux_string (m_id);
            global.panel_client->hide_lookup_table (m_id);
            m_instance->focus_in ();
        } else {
            global.panel_client->turn_off (m_id);
        }

        global.panel_client->send ();
    }
}

void
QScimInputContext::unsetFocus()
{
    SCIM_DEBUG_FRONTEND(1) << "QScimInputContext::unsetFocus (), this=" << this << " focused=" << global.focused_ic << "\n";

    if(!m_instance.null () && this == global.focused_ic) {
        global.panel_client->prepare (m_id);
        m_instance->focus_out ();
        if (m_shared_instance) m_instance->reset ();
        global.panel_client->turn_off (m_id);
        global.panel_client->focus_out (m_id);
        global.panel_client->send ();
        global.focused_ic = 0;
    }
}

void
QScimInputContext::update()
{
    SCIM_DEBUG_FRONTEND(1) << "QScimInputContext::update\n";
    if(QWidget * w = focusWidget())
    {
        QRect micro = w->inputMethodQuery(Qt::ImMicroFocus).toRect();
        QPoint new_cursor = w->mapToGlobal(micro.bottomLeft());
    //     int x = micro.x(), new_cursor_y = micro.y() + micro.height();
    //     SCIM_DEBUG_FRONTEND(1) << "QScimInputContext::update (x=" << x << ", y=" << micro.y() << ", w=" << micro.width() << ", h=" << micro.height() << ")\n";
    
        if (!m_instance.null () && this == global.focused_ic && m_cursor != new_cursor) {
            m_cursor = new_cursor;
    
            global.panel_client->prepare (m_id);
            panel_req_update_spot_location ();
            global.panel_client->send ();
        }
    }
}

void
QScimInputContext::mouseHandler ( int x, QMouseEvent * event )
{
    //SCIM_DEBUG_FRONTEND(3) << "QScimInputContext::mouseHandler (x=" << x << ", Type=" << type << ", Button=" << button << ", State=" << state << ")\n";
}

void
QScimInputContext::finalize ()
{
    SCIM_DEBUG_FRONTEND(1) << "QScimInputContext::finalize (" << m_id << ")\n";

    if (!m_instance.null()) {
        // Only do full finalize, if the Panel doesn't exit.
        if (!global.panel_exited) {
            global.panel_client->prepare (m_id);
 
            if (global.focused_ic == this)
                m_instance->focus_out ();
 
            // FIXME:
            // In case the instance send out some helper event,
            // and this context has been focused out,
            // we need set the focused_ic to this context temporary.
            QScimInputContext *old_focused = global.focused_ic;
            global.focused_ic = this;
            // XXX: Free the smart pointer, not reset the instance.
            m_instance.reset ();
            global.focused_ic = old_focused;
 
            if (global.focused_ic == this) {
                global.panel_client->turn_off (m_id);
                global.panel_client->focus_out (m_id);
            }
 
            global.panel_client->remove_input_context (m_id);
            global.panel_client->send ();
        } else {
            m_instance.reset ();
        }
    }

    if (global.focused_ic == this)
        global.focused_ic = 0;
}

bool
QScimInputContext::filterScimEvent (const KeyEvent &key)
{
    bool filtered = false;

    global.panel_client->prepare (m_id);
    if (!filter_hotkeys (key)) {
        if (!m_is_on || !m_instance->process_key_event (key)) {
            filtered = global.fallback_instance->process_key_event (key);
        } else {
            filtered = true;
        }
    } else {
        filtered = true;
    }

    global.panel_client->send ();
    return filtered;
}

void
QScimInputContext::open_specific_factory (const String &uuid)
{
    SCIM_DEBUG_FRONTEND(2) << "QScimInputContext::open_specific_factory (" << uuid << ")\n";

    // The same input method is selected, just turn on the IC.
    if (m_instance->get_factory_uuid () == uuid) {
        turn_on_ic ();
        return;
    }

    IMEngineFactoryPointer factory = global.backend->get_factory (uuid);
    if (uuid.length () && !factory.null ()) {
        turn_off_ic ();
        m_instance = factory->create_instance ("UTF-8", m_instance->get_id ());
        m_instance->set_frontend_data (static_cast <void*> (this));
        m_preedit_string = "";
        m_preedit_caret = 0;
        m_preedit_sellen = 0;
        attach_instance (m_instance);
        global.backend->set_default_factory (global.language, factory->get_uuid ());
        global.panel_client->register_input_context (m_id, factory->get_uuid ());
        set_ic_capabilities();
        turn_on_ic ();
        if (global.shared_input_method) {
            global.default_instance = m_instance;
            m_shared_instance = true;
        }
    } else {
        turn_off_ic ();
    }
}

void
QScimInputContext::open_next_factory ()
{
    SCIM_DEBUG_FRONTEND(2) << "QScimInputContext::open_next_factory ()\n";

    IMEngineFactoryPointer factory = global.backend->get_next_factory ("", "UTF-8", m_instance->get_factory_uuid ());
    if (!factory.null ()) {
        turn_off_ic();
        m_instance = factory->create_instance ("UTF-8", m_instance->get_id ());
        m_instance->set_frontend_data (static_cast <void*> (this));
        m_preedit_string = "";
        m_preedit_caret = 0;
        m_preedit_sellen = 0;
        attach_instance (m_instance);
        global.backend->set_default_factory (global.language, factory->get_uuid ());
        global.panel_client->register_input_context (m_id, factory->get_uuid ());
        set_ic_capabilities();
        turn_on_ic ();
        if (global.shared_input_method) {
            global.default_instance = m_instance;
            m_shared_instance = true;
        }
    }
}

void
QScimInputContext::open_previous_factory ()
{
    SCIM_DEBUG_FRONTEND(2) << "QScimInputContext::open_previous_factory ()\n";

    IMEngineFactoryPointer factory = global.backend->get_previous_factory ("", "UTF-8", m_instance->get_factory_uuid ());

    if (!factory.null ()) {
        turn_off_ic();
        m_instance = factory->create_instance ("UTF-8", m_instance->get_id ());
        m_instance->set_frontend_data (static_cast <void*> (this));
        m_preedit_string = "";
        m_preedit_caret = 0;
        m_preedit_sellen = 0;
        attach_instance (m_instance);
        global.backend->set_default_factory (global.language, factory->get_uuid ());
        global.panel_client->register_input_context (m_id, factory->get_uuid ());
        set_ic_capabilities();
        turn_on_ic ();
        if (global.shared_input_method) {
            global.default_instance = m_instance;
            m_shared_instance = true;
        }
    }
}

bool
QScimInputContext::filter_hotkeys (const KeyEvent &key)
{
    SCIM_DEBUG_FRONTEND(3) << "QScimInputContext::filter_hotkeys (" << key.get_key_string () << ")\n";

    bool ret = false;

    global.frontend_hotkey_matcher.push_key_event (key);
    global.imengine_hotkey_matcher.push_key_event (key);

    FrontEndHotkeyAction hotkey_action = global.frontend_hotkey_matcher.get_match_result ();

    if (hotkey_action == SCIM_FRONTEND_HOTKEY_TRIGGER) {
        SCIM_DEBUG_FRONTEND(1) << "QScimInputContext::filter_hotkeys SCIM_FRONTEND_HOTKEY_TRIGGER\n";
        if (!m_is_on)
            turn_on_ic ();
        else
            turn_off_ic ();
        ret = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_ON) {
        if (!m_is_on)
            turn_on_ic ();
        ret = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_OFF) {
        if (m_is_on)
            turn_off_ic ();
        ret = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_NEXT_FACTORY) {
        open_next_factory ();
        ret = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_PREVIOUS_FACTORY) {
        open_previous_factory ();
        ret = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_SHOW_FACTORY_MENU) {
        panel_req_show_factory_menu ();
        ret = true;
    } else if (global.imengine_hotkey_matcher.is_matched ()) {
        String sfid = global.imengine_hotkey_matcher.get_match_result ();
        open_specific_factory (sfid);
        ret = true;
    }
    return ret;
}

void
QScimInputContext::turn_on_ic ()
{
    SCIM_DEBUG_FRONTEND(1) << "QScimInputContext::turn_on_ic ()\n";
    if (!m_instance.null () && !m_is_on) {
        m_is_on = true;

        if (this == global.focused_ic) {
            panel_req_focus_in ();
            panel_req_update_screen ();
            panel_req_update_spot_location ();
            panel_req_update_factory_info ();
            global.panel_client->turn_on (m_id);
            global.panel_client->hide_preedit_string (m_id);
            global.panel_client->hide_aux_string (m_id);
            global.panel_client->hide_lookup_table (m_id);
            m_instance->focus_in ();
        }

        //Record the IC on/off status
        if (global.shared_input_method)
            global.config->write (String (SCIM_CONFIG_FRONTEND_IM_OPENED_BY_DEFAULT), true);
    }
}

void
QScimInputContext::turn_off_ic ()
{
    if (!m_instance.null () && m_is_on) {
        m_is_on = false;

        if (this == global.focused_ic) {
            m_instance->focus_out ();

            panel_req_update_factory_info ();
            global.panel_client->turn_off (m_id);
        }

        //Record the IC on/off status
        if (global.shared_input_method)
            global.config->write (String (SCIM_CONFIG_FRONTEND_IM_OPENED_BY_DEFAULT), false);

        //Hide preedit string
        QInputMethodEvent emptystring;
        sendEvent (emptystring);
    }
}

void
QScimInputContext::panel_req_update_screen()
{
#if defined(Q_WS_X11)
    int screen = QX11Info::appScreen ();
    if (screen >= 0) {
        global.panel_client->update_screen (m_id, screen);
    }
#endif
}

void
QScimInputContext::panel_req_show_help()
{
    String help = String (_("Smart Common Input Method platform ")) +
                  String (SCIM_VERSION) +
                  String (_("\n(C) 2002-2005 James Su <suzhe@tsinghua.org.cn>\n"
                      "     2003-2005 LiuCougar <liuspider@users.sourceforge.net>\n\n"));

    if (!m_instance.null()) {
        IMEngineFactoryPointer factory = global.backend->get_factory (m_instance->get_factory_uuid ());
        help += utf8_wcstombs (factory->get_name ());
        help += String (_(":\n\n"));

        help += utf8_wcstombs (factory->get_authors ());
        help += String ("\n\n");

        help += utf8_wcstombs (factory->get_help ());
        help += String ("\n\n");

        help += utf8_wcstombs (factory->get_credits ());
    }

    global.panel_client->show_help (m_id, help);
}

void
QScimInputContext::panel_req_show_factory_menu()
{
    std::vector <IMEngineFactoryPointer> factories;
    std::vector <PanelFactoryInfo> menu;

    global.backend->get_factories_for_encoding (factories, "UTF-8");

    for (size_t i = 0; i < factories.size (); ++ i) {
        menu.push_back (PanelFactoryInfo (factories [i]->get_uuid (),
                                          utf8_wcstombs (factories [i]->get_name ()),
                                          factories [i]->get_language (),
                                          factories [i]->get_icon_file ()));
    }

    if (menu.size ())
        global.panel_client->show_factory_menu (m_id, menu);
}

void
QScimInputContext::panel_req_focus_in()
{
    global.panel_client->focus_in (m_id, m_instance->get_factory_uuid ());

}

#ifndef SCIM_KEYBOARD_ICON_FILE
  #define SCIM_KEYBOARD_ICON_FILE            (SCIM_ICONDIR "/keyboard.png")
#endif

void
QScimInputContext::panel_req_update_factory_info ()
{
    if (this == global.focused_ic) {
        PanelFactoryInfo info;
        if (m_is_on) {
            IMEngineFactoryPointer sf = global.backend->get_factory (m_instance->get_factory_uuid ());
            info = PanelFactoryInfo (sf->get_uuid (), utf8_wcstombs (sf->get_name ()), sf->get_language (), sf->get_icon_file ());
        } else {
            info = PanelFactoryInfo (String (""), String (_("English/Keyboard")), String ("C"), String (SCIM_KEYBOARD_ICON_FILE));
        }
        global.panel_client->update_factory_info (m_id, info);
    }
}

void
QScimInputContext::panel_req_update_spot_location()
{
    global.panel_client->update_spot_location (m_id, (uint32) m_cursor.x(), (uint32) m_cursor.y());
}

bool
QScimInputContext::commit_string (QString s)
{
//     if (!s.length())
//         return false;

    QInputMethodEvent e;
    e.setCommitString(s);
    sendEvent (e);

    return true;
}

void
QScimInputContext::set_ic_capabilities ()
{
    if (!m_instance.null ()) {
        unsigned int cap = SCIM_CLIENT_CAP_ALL_CAPABILITIES;

        if (!global.use_preedit)
            cap -= SCIM_CLIENT_CAP_ONTHESPOT_PREEDIT;

        m_instance->update_client_capabilities (cap);
    }
}

void
QScimInputContext::attach_instance (const IMEngineInstancePointer &instance)
{
    instance->signal_connect_show_preedit_string (
        slot (&QScimInputContext::slot_show_preedit_string));
    instance->signal_connect_show_aux_string (
        slot (&QScimInputContext::slot_show_aux_string));
    instance->signal_connect_show_lookup_table (
        slot (&QScimInputContext::slot_show_lookup_table));

    instance->signal_connect_hide_preedit_string (
        slot (&QScimInputContext::slot_hide_preedit_string));
    instance->signal_connect_hide_aux_string (
        slot (&QScimInputContext::slot_hide_aux_string));
    instance->signal_connect_hide_lookup_table (
        slot (&QScimInputContext::slot_hide_lookup_table));

    instance->signal_connect_update_preedit_caret (
        slot (&QScimInputContext::slot_update_preedit_caret));
    instance->signal_connect_update_preedit_string (
        slot (&QScimInputContext::slot_update_preedit_string));
    instance->signal_connect_update_aux_string (
        slot (&QScimInputContext::slot_update_aux_string));
    instance->signal_connect_update_lookup_table (
        slot (&QScimInputContext::slot_update_lookup_table));

    instance->signal_connect_commit_string (
        slot (&QScimInputContext::slot_commit_string));

    instance->signal_connect_forward_key_event (
        slot (&QScimInputContext::slot_forward_key_event));

    instance->signal_connect_register_properties (
        slot (&QScimInputContext::slot_register_properties));

    instance->signal_connect_update_property (
        slot (&QScimInputContext::slot_update_property));

    instance->signal_connect_beep (
        slot (&QScimInputContext::slot_beep));

    instance->signal_connect_start_helper (
        slot (&QScimInputContext::slot_start_helper));

    instance->signal_connect_stop_helper (
        slot (&QScimInputContext::slot_stop_helper));

    instance->signal_connect_send_helper_event (
        slot (&QScimInputContext::slot_send_helper_event));

    instance->signal_connect_get_surrounding_text (
        slot (&QScimInputContext::slot_get_surrounding_text));

    instance->signal_connect_delete_surrounding_text (
        slot (&QScimInputContext::slot_delete_surrounding_text));
}

QScimInputContext *
QScimInputContext::find_ic(int context)
{
    if (global.input_contexts.count(context)) {
        return global.input_contexts [context];
    } else {
        SCIM_DEBUG_FRONTEND(0) << "ERROR: Can NOT find input context = " << context << "\n";
        return 0;
    }
}

//---------------scim slots----------------
void
QScimInputContext::slot_show_preedit_string (IMEngineInstanceBase *si)
{
    SCIM_DEBUG_FRONTEND(1) << "slot_show_preedit_string...\n";

    QScimInputContext *ic;

    if (si == 0 || (ic = static_cast<QScimInputContext *> (si->get_frontend_data ())) == 0)
        return;

//     if (!ic->isComposing())
//         ic->sendIMEvent (QEvent::IMStart);

    if (!global.use_preedit)
        global.panel_client->show_preedit_string (ic->m_id);
    else {
        QInputMethodEvent imevent(ic->m_preedit_string, ic->m_attribs);
        ic->sendEvent (imevent);
//         ic->sendIMEvent (QEvent::IMCompose, ic->m_preedit_string, ic->m_preedit_caret, ic->m_preedit_sellen);
    }
}

void
QScimInputContext::slot_show_aux_string (IMEngineInstanceBase *si)
{
    SCIM_DEBUG_FRONTEND(1) << "slot_show_aux_string...\n";

    QScimInputContext *ic;

    if (si == 0 || (ic = static_cast<QScimInputContext *> (si->get_frontend_data ())) == 0)
        return;

    global.panel_client->show_aux_string (ic->m_id);
}

void
QScimInputContext::slot_show_lookup_table (IMEngineInstanceBase *si)
{
    SCIM_DEBUG_FRONTEND(1) << "slot_show_lookup_table...\n";

    QScimInputContext *ic;

    if (si == 0 || (ic = static_cast<QScimInputContext *> (si->get_frontend_data ())) == 0)
        return;

    global.panel_client->show_lookup_table (ic->m_id);
}

void
QScimInputContext::slot_hide_preedit_string (IMEngineInstanceBase *si)
{
    SCIM_DEBUG_FRONTEND(1) << "slot_hide_preedit_string...\n";

    QScimInputContext *ic;

    if (si == 0 || (ic = static_cast<QScimInputContext *> (si->get_frontend_data ())) == 0)
        return;

    //Hide preedit string
    if (ic->isComposing ()) {
        QInputMethodEvent emptystring;
        ic->sendEvent (emptystring);
    }

    global.panel_client->hide_preedit_string (ic->m_id);
}

void
QScimInputContext::slot_hide_aux_string (IMEngineInstanceBase *si) 
{
    SCIM_DEBUG_FRONTEND(1) << "slot_hide_aux_string...\n";

    QScimInputContext *ic;

    if (si == 0 || (ic = static_cast<QScimInputContext *> (si->get_frontend_data ())) == 0)
        return;

    global.panel_client->hide_aux_string (ic->m_id);
}

void
QScimInputContext::slot_hide_lookup_table (IMEngineInstanceBase *si) 
{
    SCIM_DEBUG_FRONTEND(1) << "slot_hide_lookup_table...\n";

    QScimInputContext *ic;

    if (si == 0 || (ic = static_cast<QScimInputContext *> (si->get_frontend_data ())) == 0)
        return;

    global.panel_client->hide_lookup_table (ic->m_id);
}

void QScimInputContext::slot_update_preedit_caret (IMEngineInstanceBase *si, int caret) 
{
    SCIM_DEBUG_FRONTEND(1) << "slot_update_preedit_caret...\n";

    QScimInputContext *ic;

    if (si == 0 || (ic = static_cast<QScimInputContext *> (si->get_frontend_data ())) == 0)
        return;

    if (ic->m_preedit_caret != caret) {
        //FIXME: as of Qt 3, it is impossible to highlight a part of string, unless that part
        //is begin in the cursor, so let's just disable highlight in other cases
        ic->m_preedit_caret = caret;
        ic->m_preedit_sellen = 0;
    }

    if (global.use_preedit) {
        //FIXME: although Qt4 introduced QInputMethodEvent::Cursor, but it does not implemented in as in 4.0.1
        //TODO: after Qt4 implements it, uncomment the following 4 lines

        //once inserted into m_attribs, it is impossible to retrieve whether a QInputMethodEvent::Cursor exists or not,
        //so we have to copy it first
//         QList<QInputMethodEvent::Attribute> attribs = ic->m_attribs;
//         attribs << QInputMethodEvent::Attribute(QInputMethodEvent::Cursor, caret, 0, Qt::red );
//         QInputMethodEvent imevent(ic->m_preedit_string, attribs);
//         ic->sendEvent (imevent);
    } else {
        global.panel_client->update_preedit_caret (ic->m_id, caret);
    }
}

void
QScimInputContext::slot_update_preedit_string (IMEngineInstanceBase *si, const WideString &str, const AttributeList &attrs) 
{
    SCIM_DEBUG_FRONTEND(1) << "slot_update_preedit_string...\n";

    QScimInputContext *ic;

    if (si == 0 || (ic = static_cast<QScimInputContext *> (si->get_frontend_data ())) == 0)
        return;

    ic->m_preedit_string = QString::fromUtf8(utf8_wcstombs (str).c_str());

    if (global.use_preedit) {
        //start -> attribute
        typedef QMap<unsigned, QTextCharFormat > StartPosSegMap;
        StartPosSegMap segments;
        QTextFormat hightlightFormat = ic->standardFormat(QInputContext::SelectionFormat);
        QBrush highlight, highlightedText, normal, normalText;
        if(QWidget *w = ic->focusWidget()) {
            highlight = w->palette().highlight ();
            highlightedText = w->palette().highlightedText ();
            normal = w->palette().background ();
            normalText = w->palette().text();
        }
        ic->m_attribs.clear();
        ic->m_preedit_sellen = 0;

        AttributeList::const_iterator it = attrs.begin();
        segments[0] = ic->standardFormat(QInputContext::PreeditFormat).toCharFormat();
//         ic->m_attribs << QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat,
//                 0, ic->m_preedit_string.length(), ic->standardFormat(QInputContext::PreeditFormat));
        for(;it != attrs.end(); it++) {
            const unsigned int value = it->get_value(), end_pos = it->get_start() + it->get_length();

            if( !segments.count(it->get_start()) ) {
                StartPosSegMap::iterator qmit = segments.lowerBound(it->get_start());
                --qmit;
                segments[it->get_start()] = qmit.value();
            }

            if( !segments.count(end_pos) ) {
                StartPosSegMap::iterator qmit = segments.lowerBound(end_pos);
                --qmit;
                segments[end_pos] = qmit.value();
            }
            StartPosSegMap::iterator begin = segments.find(it->get_start()), end = segments.find(end_pos);
            for(StartPosSegMap::iterator i = begin; i != end; ++i)
            {
                switch(it->get_type())
                {
                    case SCIM_ATTR_DECORATE:
                        switch(value)
                        {
                            case SCIM_ATTR_DECORATE_HIGHLIGHT:
                                i->setBackground(highlight);
                                i->setForeground(highlightedText);
                                break;
                            case SCIM_ATTR_DECORATE_REVERSE:
                                i->setForeground(normal);
                                i->setBackground(normalText);
                                break;
                            case SCIM_ATTR_DECORATE_UNDERLINE:
                                i->setFontUnderline(true);
                            default:
                                break;
                        }
                        break;
                    case SCIM_ATTR_FOREGROUND:
                        i->setForeground(QColor(SCIM_RGB_COLOR_RED(value) * 256, SCIM_RGB_COLOR_GREEN(value) * 256, SCIM_RGB_COLOR_BLUE(value) * 256));
                        break;
                    case SCIM_ATTR_BACKGROUND:
                        i->setBackground(QColor(SCIM_RGB_COLOR_RED(value) * 256, SCIM_RGB_COLOR_GREEN(value) * 256, SCIM_RGB_COLOR_BLUE(value) * 256));
                    default:
                        break;
                }
            }

            segments[ic->m_preedit_string.length()] = QTextCharFormat();
            for(StartPosSegMap::iterator i = segments.begin(), ni = i + 1; ni != segments.end(); ++i, ++ni)
            {
                ic->m_attribs << QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat,
                        i.key(), ni.key() - i.key(), *i);
            }
        }

        QInputMethodEvent imevent(ic->m_preedit_string, ic->m_attribs);
        ic->sendEvent (imevent);
//         if(ic->isComposing())
//             ic->sendIMEvent (QEvent::IMCompose, ic->m_preedit_string, ic->m_preedit_caret, ic->m_preedit_sellen);
    } else {
        global.panel_client->update_preedit_string (ic->m_id, str, attrs);
    }
}

void
QScimInputContext::slot_update_aux_string (IMEngineInstanceBase *si, const WideString &str, const AttributeList &attrs) 
{
    SCIM_DEBUG_FRONTEND(1) << "slot_update_aux_string...\n";

    QScimInputContext *ic;

    if (si == 0 || (ic = static_cast<QScimInputContext *> (si->get_frontend_data ())) == 0)
        return;

    global.panel_client->update_aux_string (ic->m_id, str, attrs);
}

void
QScimInputContext::slot_commit_string (IMEngineInstanceBase *si, const WideString &str)
{
    SCIM_DEBUG_FRONTEND(1) << "slot_commit_string...\n";

    QScimInputContext *ic;

    if (si == 0 || (ic = static_cast<QScimInputContext *> (si->get_frontend_data ())) == 0)
        return;

    ic->commit_string (QString::fromUtf8(utf8_wcstombs (str).c_str()));
}

void
QScimInputContext::slot_forward_key_event (IMEngineInstanceBase *si, const KeyEvent &key) 
{
    SCIM_DEBUG_FRONTEND(1) << "slot_forward_key_event...\n";

    QScimInputContext *ic;

    if (si == 0 || (ic = static_cast<QScimInputContext *> (si->get_frontend_data ())) == 0)
        return;

    if (!global.fallback_instance->process_key_event (key)) {
        if(qApp->focusWidget ()) {
#ifdef Q_WS_X11
            XEvent xkp;

            //create event
            xkp.xkey = scim_x11_keyevent_scim_to_x11 (global.display, key);
            xkp.xkey.send_event = TRUE;  // prevent the infinite loop issue
            //this two have to be set, otherwise qt can not dispatch them to
            //the correct widget
            xkp.xkey.window = xkp.xkey.subwindow = qApp->focusWidget()->winId();

            if( qApp->x11ProcessEvent(&xkp) == -1 )
                std::cerr << "Key '" << key.get_key_string() << "' can not be dispatched to a qwidget.\n";
#else
            QKeyEvent qtkey = keyevent_scim_to_qt(key);
            QApplication::sendEvent(qApp->focusWidget (), &qtkey);
#endif
        }
    }
}

void
QScimInputContext::slot_update_lookup_table (IMEngineInstanceBase *si, const LookupTable &table) 
{
    SCIM_DEBUG_FRONTEND(1) << "slot_update_lookup_table...\n";

    QScimInputContext *ic;

    if (si == 0 || (ic = static_cast<QScimInputContext *> (si->get_frontend_data ())) == 0)
        return;

    global.panel_client->update_lookup_table (ic->m_id, table);
}

void
QScimInputContext::slot_register_properties (IMEngineInstanceBase *si, const PropertyList &properties) 
{
    SCIM_DEBUG_FRONTEND(1) << "slot_register_properties...\n";

    QScimInputContext *ic;

    if (si == 0 || (ic = static_cast<QScimInputContext *> (si->get_frontend_data ())) == 0)
        return;

    global.panel_client->register_properties (ic->m_id, properties);
}

void
QScimInputContext::slot_update_property (IMEngineInstanceBase *si, const Property &property) 
{
    SCIM_DEBUG_FRONTEND(1) << "slot_update_property ...\n";

    QScimInputContext *ic;

    if (si == 0 || (ic = static_cast<QScimInputContext *> (si->get_frontend_data ())) == 0)
        return;

    global.panel_client->update_property (ic->m_id, property);
}

void
QScimInputContext::slot_beep (IMEngineInstanceBase * /*si*/)
{
    SCIM_DEBUG_FRONTEND(1) << "slot_beep ...\n";
    QApplication::beep();
}

void
QScimInputContext::slot_start_helper (IMEngineInstanceBase *si, const String &helper_uuid)
{
    SCIM_DEBUG_FRONTEND(1) << "slot_start_helper ...\n";

    QScimInputContext *ic;

    if (si == 0 || (ic = static_cast<QScimInputContext *> (si->get_frontend_data ())) == 0)
        return;

    global.panel_client->start_helper (ic->m_id, helper_uuid);
}

void
QScimInputContext::slot_stop_helper (IMEngineInstanceBase  *si, const String &helper_uuid)
{
    SCIM_DEBUG_FRONTEND(1) << "slot_stop_helper ...\n";

    QScimInputContext *ic;

    if (si == 0 || (ic = static_cast<QScimInputContext *> (si->get_frontend_data ())) == 0)
        return;

    global.panel_client->stop_helper (ic->m_id, helper_uuid);
}

void
QScimInputContext::slot_send_helper_event (IMEngineInstanceBase *si, const String &helper_uuid, const Transaction &trans)
{
    SCIM_DEBUG_FRONTEND(1) << "slot_send_helper_event ...\n";

    QScimInputContext *ic;

    if (si == 0 || (ic = static_cast<QScimInputContext *> (si->get_frontend_data ())) == 0)
        return;

    global.panel_client->send_helper_event (ic->m_id, helper_uuid, trans);
}

bool
QScimInputContext::slot_get_surrounding_text (IMEngineInstanceBase *si,
                                              WideString            &text,
                                              int                   &cursor,
                                              int                    maxlen_before,
                                              int                    maxlen_after)
{
    SCIM_DEBUG_FRONTEND(1) << "slot_get_surrounding_text ...\n";

    QScimInputContext* ic;

    if (si == 0 || (ic = static_cast<QScimInputContext *> (si->get_frontend_data ())) == 0)
        return false;

    if (ic && !ic->m_instance.null () && ic == global.focused_ic) {
        if (QWidget * w = ic->focusWidget()) {
            cursor = w->inputMethodQuery(Qt::ImCursorPosition).toInt();
            text = utf8_mbstowcs(w->inputMethodQuery(Qt::ImSurroundingText).toString().toUtf8().data());
            return true;
        }
    }
    return false;
}

bool
QScimInputContext::slot_delete_surrounding_text (IMEngineInstanceBase *si,
                                                 int                    offset,
                                                 int                    len)
{
    SCIM_DEBUG_FRONTEND(1) << "slot_delete_surrounding_text ...\n";

    QScimInputContext* ic;

    if (si == 0 || (ic = static_cast<QScimInputContext *> (si->get_frontend_data ())) == 0)
        return false;

    if (ic && !ic->m_instance.null () && ic == global.focused_ic) {
            QInputMethodEvent replacestring;
            replacestring.setCommitString("", offset, len);
            ic->sendEvent(replacestring);
            return true;
    }

    return false;
}

//------------ Panel slot functions --------------------
void
QScimInputContext::panel_slot_exit(int /*context*/)
{
    SCIM_DEBUG_FRONTEND(1) << "QScimInputContext::panel_slot_exit ()\n";

    global.panel_exited = true;
    global.finalize ();
}

void
QScimInputContext::panel_slot_update_lookup_table_page_size (int context, int page_size)
{
    SCIM_DEBUG_FRONTEND(1) << "QScimInputContext::panel_slot_update_lookup_table_page_size (" << context << "," << page_size << ")\n";

    QScimInputContext* ic = find_ic(context);

    if(ic && !ic->m_instance.null ()) {
        global.panel_client->prepare (ic->m_id);
        ic->m_instance->update_lookup_table_page_size (page_size);
        global.panel_client->send ();
    }
}

void
QScimInputContext::panel_slot_lookup_table_page_up (int context)
{
    SCIM_DEBUG_FRONTEND(1) << "QScimInputContext::panel_slot_lookup_table_page_up (" << context << ")\n";

    QScimInputContext* ic = find_ic(context);

    if(ic && !ic->m_instance.null ()) {
        global.panel_client->prepare (ic->m_id);
        ic->m_instance->lookup_table_page_up ();
        global.panel_client->send ();
    }
}

void
QScimInputContext::panel_slot_lookup_table_page_down (int context)
{
    SCIM_DEBUG_FRONTEND(1) << "QScimInputContext::panel_slot_lookup_table_page_down (" << context << ")\n";

    QScimInputContext* ic = find_ic(context);

    if(ic && !ic->m_instance.null ()) {
        global.panel_client->prepare (ic->m_id);
        ic->m_instance->lookup_table_page_down ();
        global.panel_client->send ();
    }
}

void
QScimInputContext::panel_slot_trigger_property (int context, const String &property)
{
    SCIM_DEBUG_FRONTEND(1) << "QScimInputContext::panel_slot_trigger_property (" << context << "," << property << ")\n";

    QScimInputContext* ic = find_ic(context);

    if(ic && !ic->m_instance.null ()) {
        global.panel_client->prepare (ic->m_id);
        ic->m_instance->trigger_property (property);
        global.panel_client->send ();
    }
}

void
QScimInputContext::panel_slot_process_helper_event (int context, const String &target_uuid, const String &helper_uuid, const Transaction &trans)
{
    SCIM_DEBUG_FRONTEND(3) << "QScimInputContext::panel_slot_process_helper_event (" << context << "," << target_uuid << "," << helper_uuid << ",...)\n";

    QScimInputContext* ic = find_ic(context);

    if(ic && !ic->m_instance.null () && ic->m_instance->get_factory_uuid () == target_uuid) {
        global.panel_client->prepare (ic->m_id);
        ic->m_instance->process_helper_event (helper_uuid, trans);
        global.panel_client->send ();
    }
}

void
QScimInputContext::panel_slot_move_preedit_caret (int context, int caret_pos)
{
    SCIM_DEBUG_FRONTEND(1) << "QScimInputContext::panel_slot_move_preedit_caret (" << context << "," << caret_pos << ")\n";

    QScimInputContext* ic = find_ic(context);

    if(ic && !ic->m_instance.null ()) {
        global.panel_client->prepare (ic->m_id);
        ic->m_instance->move_preedit_caret (caret_pos);
        global.panel_client->send ();
    }
}

void
QScimInputContext::panel_slot_select_candidate (int context, int cand_index)
{
    SCIM_DEBUG_FRONTEND(1) << "QScimInputContext::panel_slot_select_candidate (" << context << "," << cand_index << ")\n";

    QScimInputContext* ic = find_ic(context);

    if(ic && !ic->m_instance.null ()) {
        global.panel_client->prepare (ic->m_id);
        ic->m_instance->select_candidate (cand_index);
        global.panel_client->send ();
    }
}

void
QScimInputContext::panel_slot_process_key_event (int context, const KeyEvent &key)
{
    SCIM_DEBUG_FRONTEND(1) << "QScimInputContext::panel_slot_process_key_event (" << context << "," << key.get_key_string () << ")\n";

    QScimInputContext* ic = find_ic(context);

    if(ic && !ic->m_instance.null ()) {
        global.panel_client->prepare (ic->m_id);
        if (!ic->filter_hotkeys (key)) {
            if (!ic->m_is_on || !ic->m_instance->process_key_event (key))
                slot_forward_key_event(ic->m_instance, key);
        }
        global.panel_client->send ();
    }
}

void
QScimInputContext::panel_slot_commit_string (int context, const WideString &wstr)
{
    SCIM_DEBUG_FRONTEND(1) << "QScimInputContext::panel_slot_commit_string (" << context << "," << utf8_wcstombs (wstr) << ")\n";

    QScimInputContext* ic = find_ic(context);

    if(ic && !ic->m_instance.null ())
        ic->commit_string (QString::fromUtf8(utf8_wcstombs (wstr).c_str()));
}

void
QScimInputContext::panel_slot_forward_key_event (int context, const KeyEvent &key)
{
    SCIM_DEBUG_FRONTEND(1) << "QScimInputContext::panel_slot_forward_key_event (" << context << "," << key.get_key_string () << ")\n";

    QScimInputContext* ic = find_ic(context);

    if(ic && !ic->m_instance.null ())
        slot_forward_key_event(ic->m_instance, key);
}

void
QScimInputContext::panel_slot_request_help (int context)
{
    SCIM_DEBUG_FRONTEND(1) << "QScimInputContext::panel_slot_request_help (" << context << ")\n";

    QScimInputContext* ic = find_ic(context);

    if(ic && !ic->m_instance.null ()) {
        global.panel_client->prepare (ic->m_id);
        ic->panel_req_show_help ();
        global.panel_client->send ();
    }
}

void
QScimInputContext::panel_slot_request_factory_menu (int context)
{
    SCIM_DEBUG_FRONTEND(1) << "QScimInputContext::panel_slot_request_factory_menu (" << context << ")\n";

    QScimInputContext* ic = find_ic(context);

    if(ic && !ic->m_instance.null ()) {
        global.panel_client->prepare (ic->m_id);
        ic->panel_req_show_factory_menu ();
        global.panel_client->send ();
    }
}

void
QScimInputContext::panel_slot_change_factory (int context, const String &uuid)
{
    SCIM_DEBUG_FRONTEND(1) << "QScimInputContext::panel_slot_change_factory (" << context << "," << uuid << ")\n";

    QScimInputContext* ic = find_ic(context);

    if(ic && !ic->m_instance.null ()) {
        global.panel_client->prepare (ic->m_id);
        ic->open_specific_factory (uuid);
        global.panel_client->send ();
    }
}

};

#include "qsciminputcontext.moc"
/*
vi:ts=4:nowrap:ai:expandtab
*/
