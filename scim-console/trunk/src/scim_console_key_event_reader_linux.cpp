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
 * @brief This file is the specific implementation of the key event readers for Linux.
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <linux/input.h>
#include <linux/kd.h>
#include <linux/vt.h>

#include <iostream>
#include <vector>
#include <sstream>

#define Uses_SCIM_EVENT
#include <scim.h>

#include "scim_console_debug.h"
#include "scim_console_display.h"
#include "scim_console_key_event_reader.h"

/**
 * The type for key modifiers.
 */
typedef scim::uint32 key_modifier_t;

/**
 * The type for key codes.
 */
typedef scim::uint32 key_code_t;

/**
 * The size of the table of linux scancodes.
 */
static const size_t LINUX_KEY_TABLE_SIZE = 256;

/**
 * The key mapping table from linux to scim.
 */
static key_code_t linux_to_scim_key_table[LINUX_KEY_TABLE_SIZE];

/**
 * The size of the upper case mappings.
 */
static const size_t SHIFTED_KEYCODE_TABLE_SIZE = 0x1000000;

/**
 * The mapping from the lower cases to the upper cases.
 */
static key_code_t shifted_keycode_table[SHIFTED_KEYCODE_TABLE_SIZE];

/**
 * The interval for searching a new keyboard.
 */
static const timeval KEYBOARD_SEARCH_INTERVAL = {1, 0};

namespace scim_console
{

/**
 * The implementation of key event reader for Linux.
 */
class KeyEventReaderLinux: public KeyEventReader
{

public:

    /**
     * Constructor.
     *
     * @param new_scim_console The scim console.
     */
    KeyEventReaderLinux (ScimConsole *new_scim_console);

    /**
     * Destructor.
     */
    ~KeyEventReaderLinux ();

    event_type_t get_triggers () const;

    int get_fd () const;

    bool handle_event (event_type_t events);

    /**
     * Initialize the event reader.
     *
     * @return false if the initialization is failed.
     */
    retval_t initialize ();

private:

    /**
     * The scim console.
     */
    ScimConsole *scim_console;

    /**
     * The fd for the event device.
     */
    int event_fd;

    /**
     * The fallback keyboard layout.
     */
    scim::KeyboardLayout fallback_keyboard_layout;

    /**
     * The input file descriptor for the socket.
     */
    int socket_in;

    /**
     * The output file descriptor for the socket.
     */
    int socket_out;

    /**
     * Find and open the keyboard device.
     *
     * @return RETVAL_SUCCEEDED if it succeeded.
     */
    retval_t open_device ();

    /**
     * Close the keyboard device.
     *
     * @return RETVAL_SUCCEEDED if it succeeded.
     */
    retval_t close_device ();

};

/**
 * The instance of KeyEventReader.
 */
KeyEventReaderLinux *key_event_reader = NULL;

}

using std::cerr;
using std::endl;
using std::vector;
using std::stringstream;

using namespace scim;
using namespace scim_console;

/**
 * Initialize statically.
 */
static void static_initialize ()
{
    for (int i = 0; i < LINUX_KEY_TABLE_SIZE; ++i) {
        linux_to_scim_key_table[i] = SCIM_KEY_VoidSymbol;
    }

    linux_to_scim_key_table[KEY_RESERVED] = SCIM_KEY_NullKey;
    linux_to_scim_key_table[KEY_ESC] = SCIM_KEY_Escape;
    linux_to_scim_key_table[KEY_1] = SCIM_KEY_1;
    linux_to_scim_key_table[KEY_2] = SCIM_KEY_2;
    linux_to_scim_key_table[KEY_3] = SCIM_KEY_3;
    linux_to_scim_key_table[KEY_4] = SCIM_KEY_4;
    linux_to_scim_key_table[KEY_5] = SCIM_KEY_5;
    linux_to_scim_key_table[KEY_6] = SCIM_KEY_6;
    linux_to_scim_key_table[KEY_7] = SCIM_KEY_7;
    linux_to_scim_key_table[KEY_8] = SCIM_KEY_8;
    linux_to_scim_key_table[KEY_9] = SCIM_KEY_9;
    linux_to_scim_key_table[KEY_0] = SCIM_KEY_0;
    linux_to_scim_key_table[KEY_MINUS] = SCIM_KEY_minus;
    linux_to_scim_key_table[KEY_EQUAL] = SCIM_KEY_equal;
    linux_to_scim_key_table[KEY_BACKSPACE] = SCIM_KEY_BackSpace;
    linux_to_scim_key_table[KEY_TAB] = SCIM_KEY_Tab;
    linux_to_scim_key_table[KEY_Q] = SCIM_KEY_q;
    linux_to_scim_key_table[KEY_W] = SCIM_KEY_w;
    linux_to_scim_key_table[KEY_E] = SCIM_KEY_e;
    linux_to_scim_key_table[KEY_R] = SCIM_KEY_r;
    linux_to_scim_key_table[KEY_T] = SCIM_KEY_t;
    linux_to_scim_key_table[KEY_Y] = SCIM_KEY_y;
    linux_to_scim_key_table[KEY_U] = SCIM_KEY_u;
    linux_to_scim_key_table[KEY_I] = SCIM_KEY_i;
    linux_to_scim_key_table[KEY_O] = SCIM_KEY_o;
    linux_to_scim_key_table[KEY_P] = SCIM_KEY_p;
    linux_to_scim_key_table[KEY_LEFTBRACE] = SCIM_KEY_bracketleft;
    linux_to_scim_key_table[KEY_RIGHTBRACE] = SCIM_KEY_bracketright;
    linux_to_scim_key_table[KEY_ENTER] = SCIM_KEY_Return;
    linux_to_scim_key_table[KEY_LEFTCTRL] = SCIM_KEY_Control_L;
    linux_to_scim_key_table[KEY_A] = SCIM_KEY_a;
    linux_to_scim_key_table[KEY_S] = SCIM_KEY_s;
    linux_to_scim_key_table[KEY_D] = SCIM_KEY_d;
    linux_to_scim_key_table[KEY_F] = SCIM_KEY_f;
    linux_to_scim_key_table[KEY_G] = SCIM_KEY_g;
    linux_to_scim_key_table[KEY_H] = SCIM_KEY_h;
    linux_to_scim_key_table[KEY_J] = SCIM_KEY_j;
    linux_to_scim_key_table[KEY_K] = SCIM_KEY_k;
    linux_to_scim_key_table[KEY_L] = SCIM_KEY_l;
    linux_to_scim_key_table[KEY_SEMICOLON] = SCIM_KEY_semicolon;
    linux_to_scim_key_table[KEY_APOSTROPHE] = SCIM_KEY_apostrophe;
    linux_to_scim_key_table[KEY_GRAVE] = SCIM_KEY_grave;
    linux_to_scim_key_table[KEY_LEFTSHIFT] = SCIM_KEY_Shift_L;
    linux_to_scim_key_table[KEY_BACKSLASH] = SCIM_KEY_backslash;
    linux_to_scim_key_table[KEY_Z] = SCIM_KEY_z;
    linux_to_scim_key_table[KEY_X] = SCIM_KEY_x;
    linux_to_scim_key_table[KEY_C] = SCIM_KEY_c;
    linux_to_scim_key_table[KEY_V] = SCIM_KEY_v;
    linux_to_scim_key_table[KEY_B] = SCIM_KEY_b;
    linux_to_scim_key_table[KEY_N] = SCIM_KEY_n;
    linux_to_scim_key_table[KEY_M] = SCIM_KEY_m;
    linux_to_scim_key_table[KEY_COMMA] = SCIM_KEY_comma;
    linux_to_scim_key_table[KEY_DOT] = SCIM_KEY_period;
    linux_to_scim_key_table[KEY_SLASH] = SCIM_KEY_slash;
    linux_to_scim_key_table[KEY_RIGHTSHIFT] = SCIM_KEY_Shift_R;
    linux_to_scim_key_table[KEY_KPASTERISK] = SCIM_KEY_KP_Multiply;
    linux_to_scim_key_table[KEY_LEFTALT] = SCIM_KEY_Alt_L;
    linux_to_scim_key_table[KEY_SPACE] = SCIM_KEY_space;
    linux_to_scim_key_table[KEY_CAPSLOCK] = SCIM_KEY_Caps_Lock;
    linux_to_scim_key_table[KEY_F1] = SCIM_KEY_F1;
    linux_to_scim_key_table[KEY_F2] = SCIM_KEY_F2;
    linux_to_scim_key_table[KEY_F3] = SCIM_KEY_F3;
    linux_to_scim_key_table[KEY_F4] = SCIM_KEY_F4;
    linux_to_scim_key_table[KEY_F5] = SCIM_KEY_F5;
    linux_to_scim_key_table[KEY_F6] = SCIM_KEY_F6;
    linux_to_scim_key_table[KEY_F7] = SCIM_KEY_F7;
    linux_to_scim_key_table[KEY_F8] = SCIM_KEY_F8;
    linux_to_scim_key_table[KEY_F9] = SCIM_KEY_F9;
    linux_to_scim_key_table[KEY_F10] = SCIM_KEY_F10;
    linux_to_scim_key_table[KEY_NUMLOCK] = SCIM_KEY_Num_Lock;
    linux_to_scim_key_table[KEY_SCROLLLOCK] = SCIM_KEY_Scroll_Lock;
    linux_to_scim_key_table[KEY_KP7] = SCIM_KEY_KP_7;
    linux_to_scim_key_table[KEY_KP8] = SCIM_KEY_KP_8;
    linux_to_scim_key_table[KEY_KP9] = SCIM_KEY_KP_9;
    linux_to_scim_key_table[KEY_KPMINUS] = SCIM_KEY_KP_Subtract;
    linux_to_scim_key_table[KEY_KP4] = SCIM_KEY_KP_4;
    linux_to_scim_key_table[KEY_KP5] = SCIM_KEY_KP_5;
    linux_to_scim_key_table[KEY_KP6] = SCIM_KEY_KP_6;
    linux_to_scim_key_table[KEY_KPPLUS] = SCIM_KEY_KP_Add;
    linux_to_scim_key_table[KEY_KP1] = SCIM_KEY_KP_1;
    linux_to_scim_key_table[KEY_KP2] = SCIM_KEY_KP_2;
    linux_to_scim_key_table[KEY_KP3] = SCIM_KEY_KP_3;
    linux_to_scim_key_table[KEY_KP0] = SCIM_KEY_KP_0;
    linux_to_scim_key_table[KEY_KPDOT] = SCIM_KEY_KP_Decimal;
    linux_to_scim_key_table[KEY_ZENKAKUHANKAKU] = SCIM_KEY_Zenkaku_Hankaku;
    linux_to_scim_key_table[KEY_F11] = SCIM_KEY_F11;
    linux_to_scim_key_table[KEY_F12] = SCIM_KEY_F12;
    linux_to_scim_key_table[KEY_RO] = SCIM_KEY_kana_RO;;
    linux_to_scim_key_table[KEY_102ND] = SCIM_KEY_kana_RO;
    linux_to_scim_key_table[KEY_KATAKANA] = SCIM_KEY_Katakana;
    linux_to_scim_key_table[KEY_HIRAGANA] = SCIM_KEY_Hiragana;
    linux_to_scim_key_table[KEY_HENKAN] = SCIM_KEY_Henkan;
    linux_to_scim_key_table[KEY_KATAKANAHIRAGANA] = SCIM_KEY_Hiragana_Katakana;
    linux_to_scim_key_table[KEY_MUHENKAN] = SCIM_KEY_Muhenkan;
    linux_to_scim_key_table[KEY_KPENTER] = SCIM_KEY_KP_Enter;
    linux_to_scim_key_table[KEY_RIGHTCTRL] = SCIM_KEY_Control_R;
    linux_to_scim_key_table[KEY_KPSLASH] = SCIM_KEY_KP_Divide;
    linux_to_scim_key_table[KEY_SYSRQ] = SCIM_KEY_Sys_Req;
    linux_to_scim_key_table[KEY_RIGHTALT] = SCIM_KEY_Alt_R;
    linux_to_scim_key_table[KEY_LINEFEED] = SCIM_KEY_Linefeed;
    linux_to_scim_key_table[KEY_HOME] = SCIM_KEY_Home;
    linux_to_scim_key_table[KEY_UP] = SCIM_KEY_Up;
    linux_to_scim_key_table[KEY_PAGEUP] = SCIM_KEY_Page_Up;
    linux_to_scim_key_table[KEY_LEFT] = SCIM_KEY_Left;
    linux_to_scim_key_table[KEY_RIGHT] = SCIM_KEY_Right;
    linux_to_scim_key_table[KEY_END] = SCIM_KEY_End;
    linux_to_scim_key_table[KEY_DOWN] = SCIM_KEY_Down;
    linux_to_scim_key_table[KEY_PAGEDOWN] = SCIM_KEY_Page_Down;
    linux_to_scim_key_table[KEY_INSERT] = SCIM_KEY_Insert;
    linux_to_scim_key_table[KEY_DELETE] = SCIM_KEY_Delete;
    linux_to_scim_key_table[KEY_KPEQUAL] = SCIM_KEY_KP_Equal;
    linux_to_scim_key_table[KEY_KPPLUSMINUS] = SCIM_KEY_plusminus;
    linux_to_scim_key_table[KEY_PAUSE] = SCIM_KEY_Pause;
    linux_to_scim_key_table[KEY_KPCOMMA] = SCIM_KEY_KP_Separator;
    linux_to_scim_key_table[KEY_HANGEUL] = SCIM_KEY_Hangul;
    linux_to_scim_key_table[KEY_HANJA] = SCIM_KEY_Hangul_Hanja;
    linux_to_scim_key_table[KEY_YEN] = SCIM_KEY_yen;
    linux_to_scim_key_table[KEY_LEFTMETA] = SCIM_KEY_Meta_L;
    linux_to_scim_key_table[KEY_RIGHTMETA] = SCIM_KEY_Meta_R;
    linux_to_scim_key_table[KEY_COMPOSE] = SCIM_KEY_Multi_key;
    linux_to_scim_key_table[KEY_F13] = SCIM_KEY_F13;
    linux_to_scim_key_table[KEY_F14] = SCIM_KEY_F14;
    linux_to_scim_key_table[KEY_F15] = SCIM_KEY_F15;
    linux_to_scim_key_table[KEY_F16] = SCIM_KEY_F16;
    linux_to_scim_key_table[KEY_F17] = SCIM_KEY_F17;
    linux_to_scim_key_table[KEY_F18] = SCIM_KEY_F18;
    linux_to_scim_key_table[KEY_F19] = SCIM_KEY_F19;
    linux_to_scim_key_table[KEY_F20] = SCIM_KEY_F20;
    linux_to_scim_key_table[KEY_F21] = SCIM_KEY_F21;
    linux_to_scim_key_table[KEY_F22] = SCIM_KEY_F22;
    linux_to_scim_key_table[KEY_F23] = SCIM_KEY_F23;
    linux_to_scim_key_table[KEY_F24] = SCIM_KEY_F24;


    for (int i = 0; i < SHIFTED_KEYCODE_TABLE_SIZE; ++i) {
        shifted_keycode_table[i] = SCIM_KEY_VoidSymbol;
    }
    shifted_keycode_table[SCIM_KEY_a] = SCIM_KEY_A;
    shifted_keycode_table[SCIM_KEY_b] = SCIM_KEY_B;
    shifted_keycode_table[SCIM_KEY_c] = SCIM_KEY_C;
    shifted_keycode_table[SCIM_KEY_d] = SCIM_KEY_D;
    shifted_keycode_table[SCIM_KEY_e] = SCIM_KEY_E;
    shifted_keycode_table[SCIM_KEY_f] = SCIM_KEY_F;
    shifted_keycode_table[SCIM_KEY_g] = SCIM_KEY_G;
    shifted_keycode_table[SCIM_KEY_h] = SCIM_KEY_H;
    shifted_keycode_table[SCIM_KEY_i] = SCIM_KEY_I;
    shifted_keycode_table[SCIM_KEY_j] = SCIM_KEY_J;
    shifted_keycode_table[SCIM_KEY_k] = SCIM_KEY_K;
    shifted_keycode_table[SCIM_KEY_l] = SCIM_KEY_L;
    shifted_keycode_table[SCIM_KEY_m] = SCIM_KEY_M;
    shifted_keycode_table[SCIM_KEY_n] = SCIM_KEY_N;
    shifted_keycode_table[SCIM_KEY_o] = SCIM_KEY_O;
    shifted_keycode_table[SCIM_KEY_p] = SCIM_KEY_P;
    shifted_keycode_table[SCIM_KEY_q] = SCIM_KEY_Q;
    shifted_keycode_table[SCIM_KEY_r] = SCIM_KEY_R;
    shifted_keycode_table[SCIM_KEY_s] = SCIM_KEY_S;
    shifted_keycode_table[SCIM_KEY_t] = SCIM_KEY_T;
    shifted_keycode_table[SCIM_KEY_u] = SCIM_KEY_U;
    shifted_keycode_table[SCIM_KEY_v] = SCIM_KEY_V;
    shifted_keycode_table[SCIM_KEY_w] = SCIM_KEY_W;
    shifted_keycode_table[SCIM_KEY_x] = SCIM_KEY_X;
    shifted_keycode_table[SCIM_KEY_y] = SCIM_KEY_Y;
    shifted_keycode_table[SCIM_KEY_z] = SCIM_KEY_Z;
    shifted_keycode_table[SCIM_KEY_0] = SCIM_KEY_parenright ;
    shifted_keycode_table[SCIM_KEY_1] = SCIM_KEY_exclam;
    shifted_keycode_table[SCIM_KEY_2] = SCIM_KEY_at;
    shifted_keycode_table[SCIM_KEY_3] = SCIM_KEY_numbersign;
    shifted_keycode_table[SCIM_KEY_4] = SCIM_KEY_dollar;
    shifted_keycode_table[SCIM_KEY_5] = SCIM_KEY_percent;
    shifted_keycode_table[SCIM_KEY_6] = SCIM_KEY_asciicircum;
    shifted_keycode_table[SCIM_KEY_7] = SCIM_KEY_ampersand;
    shifted_keycode_table[SCIM_KEY_8] = SCIM_KEY_asterisk;
    shifted_keycode_table[SCIM_KEY_9] = SCIM_KEY_parenleft;
    shifted_keycode_table[SCIM_KEY_minus] = SCIM_KEY_underscore;
    shifted_keycode_table[SCIM_KEY_equal] = SCIM_KEY_equal;
    shifted_keycode_table[SCIM_KEY_backslash] = SCIM_KEY_bar;
    shifted_keycode_table[SCIM_KEY_at] = SCIM_KEY_grave;
    shifted_keycode_table[SCIM_KEY_bracketleft] = SCIM_KEY_braceleft;
    shifted_keycode_table[SCIM_KEY_bracketright] = SCIM_KEY_braceright;
    shifted_keycode_table[SCIM_KEY_semicolon] = SCIM_KEY_colon;
    shifted_keycode_table[SCIM_KEY_apostrophe] = SCIM_KEY_quotedbl;
    shifted_keycode_table[SCIM_KEY_comma] = SCIM_KEY_less;
    shifted_keycode_table[SCIM_KEY_period] = SCIM_KEY_greater;
    shifted_keycode_table[SCIM_KEY_slash] = SCIM_KEY_question;
}

/**
 * Translate the given key event into the upper case (= shifted form).
 *
 * @param key_event The key event.
 */
static void shift_key_event (KeyEvent &key_event)
{
    if (0 <= key_event.code && key_event.code < SHIFTED_KEYCODE_TABLE_SIZE) {
        const key_code_t new_key_code = shifted_keycode_table[key_event.code];
        if (new_key_code != SCIM_KEY_VoidSymbol)
            key_event.code = new_key_code;
    }
}

/* Implements */
KeyEventReader *KeyEventReader::alloc (ScimConsole *scim_console)
{
    static bool static_initialized = false;
    if (!static_initialized) {
        static_initialize ();
        static_initialized = true;
    }

    if (key_event_reader == NULL) {
        KeyEventReaderLinux *instance = new KeyEventReaderLinux (scim_console);
        if (instance->initialize ()) {
            cerr << "Failed to initialize a key event reader!" << endl;
            delete instance;
            return NULL;
        } else {
            key_event_reader = instance;
            return instance;
        }
    } else {
        cerr << "There is another key event reader!" << endl;
        return NULL;
    }
}

KeyEventReaderLinux::KeyEventReaderLinux (ScimConsole *new_scim_console): 
scim_console (new_scim_console), event_fd (-1), fallback_keyboard_layout (SCIM_KEYBOARD_Unknown), 
socket_in (-1), socket_out (-1)
{}

KeyEventReaderLinux::~KeyEventReaderLinux ()
{
    if (socket_in >= 0) {
        close (socket_in);
        socket_in = -1;
    }
    if (socket_out >= 0) {
        close (socket_out);
        socket_out = -1;
    }

    close_device ();
}

retval_t KeyEventReaderLinux::initialize ()
{
    int socket_pair[2];

    if (socketpair (PF_UNIX, SOCK_STREAM, 0, socket_pair)) {
        cerr << "Cannot make a pipe for an dummy keyboard: " << strerror (errno) << endl;
        return RETVAL_FAILED;
    }

    socket_out = socket_pair[0];
    socket_in = socket_pair[1];

    event_fd = -1;

    if (open_device ()) {
        cerr << "No keyboard has been found" << endl;
        return RETVAL_FAILED;
    }

    return RETVAL_SUCCEEDED;
}

int KeyEventReaderLinux::get_fd () const
{
    if (event_fd >= 0) {
        return event_fd;
    } else {
        return socket_in;
    }
}

event_type_t KeyEventReaderLinux::get_triggers () const
{
    return EVENT_READ | EVENT_ERROR | EVENT_INTERRUPT;
}

bool KeyEventReaderLinux::handle_event (event_type_t events)
{
    dout (6) << "KeyEventReaderLinux::handle_event ()" << endl;

    if (events & EVENT_INTERRUPT) {
        if (event_fd < 0) {
            if (open_device ()) {
                scim_console->interrupt (KEYBOARD_SEARCH_INTERVAL);
                return true;
            }
        }
    }
    if (events & EVENT_ERROR) {
        close_device ();
        return true;
    }

    static bool shift_l_down = false;
    static bool shift_r_down = false;
    static bool control_l_down = false;
    static bool control_r_down = false;
    static bool alt_l_down = false;
    static bool alt_r_down = false;
    static bool meta_l_down = false;
    static bool meta_r_down = false;
    static bool caps_lock_down = false;
    static bool num_lock_down = false;

    struct input_event event;
    const size_t read_size = read (event_fd, &event, sizeof (event));
    if (read_size <= 0) {
        if (errno == EAGAIN || errno == EINTR) {
            dout (1) << "No key event has been received?" << endl;
            return true;
        } else {
            cerr << "An IO exception occurred at KeyEventReaderEvdev::handle_event (): " << strerror (errno) << endl;
            scim_console-> quit (-1);
            return false;
        }
    } else if (event.type == EV_KEY && scim_console->get_display ().is_active ()) {
        const bool key_pressed = event.value;
        const unsigned int key_code = event.code;

        key_code_t scim_key_code = SCIM_KEY_NullKey;
        if (0 <= key_code && key_code < LINUX_KEY_TABLE_SIZE)
            scim_key_code = linux_to_scim_key_table[key_code];
        switch (scim_key_code) {
            case SCIM_KEY_Shift_L:
                shift_l_down = key_pressed;
                break;
            case SCIM_KEY_Shift_R:
                shift_r_down = key_pressed;
                break;
            case SCIM_KEY_Control_L:
                control_l_down = key_pressed;
                break;
            case SCIM_KEY_Control_R:
                control_r_down = key_pressed;
                break;
            case SCIM_KEY_Alt_L:
                alt_l_down = key_pressed;
                break;
            case SCIM_KEY_Alt_R:
                alt_r_down = key_pressed;
                break;
            case SCIM_KEY_Meta_L:
                meta_l_down = key_pressed;
                break;
            case SCIM_KEY_Meta_R:
                meta_r_down = key_pressed;
                break;
            case SCIM_KEY_Caps_Lock:
                caps_lock_down = key_pressed;
                break;
            case SCIM_KEY_Num_Lock:
                num_lock_down = key_pressed;
                break;
            default:
                break;
        }

        key_modifier_t scim_key_modifiers = SCIM_KEY_NullMask;
        if (!key_pressed)
            scim_key_modifiers |= SCIM_KEY_ReleaseMask;
        if (shift_l_down || shift_r_down)
            scim_key_modifiers |= SCIM_KEY_ShiftMask;
        if (control_l_down || control_r_down)
            scim_key_modifiers |= SCIM_KEY_ControlMask;
        if (alt_l_down || alt_r_down)
            scim_key_modifiers |= SCIM_KEY_AltMask;
        if (meta_l_down || meta_r_down)
            scim_key_modifiers |= SCIM_KEY_MetaMask;
        if (caps_lock_down)
            scim_key_modifiers |= SCIM_KEY_CapsLockMask;
        if (num_lock_down)
            scim_key_modifiers |= SCIM_KEY_NumLockMask;

        KeyEvent scim_key_event = KeyEvent (scim_key_code, scim_key_modifiers, SCIM_KEYBOARD_US);
        if ((shift_l_down | shift_r_down) ^ caps_lock_down)
            shift_key_event (scim_key_event);

        KeyboardLayout keyboard_layout = scim_console->get_scim_keyboard_layout ();
        if (keyboard_layout == SCIM_KEYBOARD_Unknown)
            keyboard_layout = fallback_keyboard_layout;
        KeyEvent localized_scim_key_event = scim_key_event.map_to_layout (keyboard_layout);
        if (keyboard_layout == SCIM_KEYBOARD_Japanese) {
            // Due to bug of evdev some japanese keys are mapped on hangul ones.
            switch (scim_key_event.code) {
            case SCIM_KEY_Hangul_Hanja:
                localized_scim_key_event.code = SCIM_KEY_Muhenkan;
                break;
            case SCIM_KEY_Hangul:
                localized_scim_key_event.code = SCIM_KEY_Henkan_Mode;
                break;
            case SCIM_KEY_kana_RO:
                if (!scim_key_event.is_shift_down ()) {
                    localized_scim_key_event.code = SCIM_KEY_backslash;
                } else {
                    localized_scim_key_event.code = SCIM_KEY_underscore;
                }
                break;
            case SCIM_KEY_yen:
                if (!scim_key_event.is_shift_down ()) {
                    localized_scim_key_event.code = SCIM_KEY_backslash;
                } else {
                    localized_scim_key_event.code = SCIM_KEY_bar;
                }
                break;
            }
        }
        dout (5) << "Key event (code = " << localized_scim_key_event.code << ", pressed = " << key_pressed << ")" << endl;

        if (!scim_console->process_key_event (localized_scim_key_event))
            scim_console->forward_key_event (localized_scim_key_event);
        scim_console->update_display ();
    } else {
        dout (1) << "Unknown event" << endl;
    }

    return true;
}

retval_t KeyEventReaderLinux::close_device ()
{
    if (event_fd >= 0) {
        close (event_fd);
        event_fd = -1;
        fallback_keyboard_layout = SCIM_KEYBOARD_Unknown;
    }
}

retval_t KeyEventReaderLinux::open_device ()
{
    static const size_t BITS_PER_LONG = sizeof (long) * 8;

    close_device ();

    int fd = -1;
    stringstream ss;
    for (int i = 0; i < 32; ++i) {
        ss.str ("");
        ss << "/dev/input/event" << i;
        const String device_path = ss.str ();
        dout (1) << "Scanning a input device '" << device_path << "'..." << endl;

        fd = open (device_path.c_str (), O_RDONLY);
        if (fd < 0) {
            dout (6) << "Failed to open a device file, '" << device_path << "': " << strerror (errno) << endl;
            continue;
        }

        unsigned long key_bits[((KEY_MAX + 1) / BITS_PER_LONG) + 1];
        if (ioctl (fd, EVIOCGBIT (EV_KEY, sizeof (key_bits)), &key_bits) < 0) {
            dout (6) << "Failed to grab key bits: " << strerror (errno) << endl;
            continue;
        }

        dout (1) << "key_bits = ";
        for (int i = 0; i < (KEY_MAX + 1) / BITS_PER_LONG + 1; ++i) {
            dout (1) << " " << key_bits[i];
        }
        dout (1) << endl;

        int key_count = 0;
        for (int key_code = 0; key_code <= KEY_UNKNOWN; ++key_code) {
            const bool key_enabled = (key_bits[key_code / BITS_PER_LONG] >> (key_code % BITS_PER_LONG)) & 0x1;
            if (key_enabled) {
                ++key_count;
                if (scim_console->get_scim_keyboard_layout () == SCIM_KEYBOARD_Unknown && fallback_keyboard_layout == SCIM_KEYBOARD_Unknown) {
                    if (key_code == KEY_YEN || key_code == KEY_102ND) {
                        fallback_keyboard_layout = SCIM_KEYBOARD_Japanese;
                    }
                }
            }

            if (event_fd < 0 && key_count > 24) {
                dout (8) << "Found a keyboard (path = " << device_path << ", fd = " << fd << ")" << endl;
                event_fd = fd;
            }
        }
        if (event_fd < 0) {
            dout (1) << "This is not a keyboard" << endl;
        } else {            
                return RETVAL_SUCCEEDED;
        }
    }

    return RETVAL_FAILED;
}
