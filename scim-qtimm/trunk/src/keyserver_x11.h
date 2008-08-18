struct TransKey {
        int keySymQt;
        uint keySymX;
};

static const TransKey g_rgQtToSymX[] =
{
        { Qt::Key_Escape,     scim::SCIM_KEY_Escape },
        { Qt::Key_Tab,        scim::SCIM_KEY_Tab },
        { Qt::Key_Backtab,    scim::SCIM_KEY_ISO_Left_Tab },
        { Qt::Key_Backspace,  scim::SCIM_KEY_BackSpace },
        { Qt::Key_Return,     scim::SCIM_KEY_Return },
        { Qt::Key_Enter,      scim::SCIM_KEY_KP_Enter },
        { Qt::Key_Insert,     scim::SCIM_KEY_Insert },
        { Qt::Key_Delete,     scim::SCIM_KEY_Delete },
        { Qt::Key_Pause,      scim::SCIM_KEY_Pause },
#ifdef sun
        { Qt::Key_Print,      scim::SCIM_KEY_F22 },
#else
        { Qt::Key_Print,      scim::SCIM_KEY_Print },
#endif
        { Qt::Key_SysReq,     scim::SCIM_KEY_Sys_Req },
        { Qt::Key_Home,       scim::SCIM_KEY_Home },
        { Qt::Key_End,        scim::SCIM_KEY_End },
        { Qt::Key_Left,       scim::SCIM_KEY_Left },
        { Qt::Key_Up,         scim::SCIM_KEY_Up },
        { Qt::Key_Right,      scim::SCIM_KEY_Right },
        { Qt::Key_Down,       scim::SCIM_KEY_Down },
        { Qt::Key_Prior,      scim::SCIM_KEY_Prior },
        { Qt::Key_Next,       scim::SCIM_KEY_Next },
        //{ Qt::Key_Shift,      0 },
        //{ Qt::Key_Control,    0 },
        //{ Qt::Key_Meta,       0 },
        //{ Qt::Key_Alt,        0 },
        { Qt::Key_CapsLock,   scim::SCIM_KEY_Caps_Lock },
        { Qt::Key_NumLock,    scim::SCIM_KEY_Num_Lock },
        { Qt::Key_ScrollLock, scim::SCIM_KEY_Scroll_Lock },
        { Qt::Key_F1,         scim::SCIM_KEY_F1 },
        { Qt::Key_F2,         scim::SCIM_KEY_F2 },
        { Qt::Key_F3,         scim::SCIM_KEY_F3 },
        { Qt::Key_F4,         scim::SCIM_KEY_F4 },
        { Qt::Key_F5,         scim::SCIM_KEY_F5 },
        { Qt::Key_F6,         scim::SCIM_KEY_F6 },
        { Qt::Key_F7,         scim::SCIM_KEY_F7 },
        { Qt::Key_F8,         scim::SCIM_KEY_F8 },
        { Qt::Key_F9,         scim::SCIM_KEY_F9 },
        { Qt::Key_F10,        scim::SCIM_KEY_F10 },
        { Qt::Key_F11,        scim::SCIM_KEY_F11 },
        { Qt::Key_F12,        scim::SCIM_KEY_F12 },
        { Qt::Key_F13,        scim::SCIM_KEY_F13 },
        { Qt::Key_F14,        scim::SCIM_KEY_F14 },
        { Qt::Key_F15,        scim::SCIM_KEY_F15 },
        { Qt::Key_F16,        scim::SCIM_KEY_F16 },
        { Qt::Key_F17,        scim::SCIM_KEY_F17 },
        { Qt::Key_F18,        scim::SCIM_KEY_F18 },
        { Qt::Key_F19,        scim::SCIM_KEY_F19 },
        { Qt::Key_F20,        scim::SCIM_KEY_F20 },
        { Qt::Key_F21,        scim::SCIM_KEY_F21 },
        { Qt::Key_F22,        scim::SCIM_KEY_F22 },
        { Qt::Key_F23,        scim::SCIM_KEY_F23 },
        { Qt::Key_F24,        scim::SCIM_KEY_F24 },
        { Qt::Key_F25,        scim::SCIM_KEY_F25 },
        { Qt::Key_F26,        scim::SCIM_KEY_F26 },
        { Qt::Key_F27,        scim::SCIM_KEY_F27 },
        { Qt::Key_F28,        scim::SCIM_KEY_F28 },
        { Qt::Key_F29,        scim::SCIM_KEY_F29 },
        { Qt::Key_F30,        scim::SCIM_KEY_F30 },
        { Qt::Key_F31,        scim::SCIM_KEY_F31 },
        { Qt::Key_F32,        scim::SCIM_KEY_F32 },
        { Qt::Key_F33,        scim::SCIM_KEY_F33 },
        { Qt::Key_F34,        scim::SCIM_KEY_F34 },
        { Qt::Key_F35,        scim::SCIM_KEY_F35 },
        { Qt::Key_Super_L,    scim::SCIM_KEY_Super_L },
        { Qt::Key_Super_R,    scim::SCIM_KEY_Super_R },
        { Qt::Key_Menu,       scim::SCIM_KEY_Menu },
        { Qt::Key_Hyper_L,    scim::SCIM_KEY_Hyper_L },
        { Qt::Key_Hyper_R,    scim::SCIM_KEY_Hyper_R },
        { Qt::Key_Help,       scim::SCIM_KEY_Help },
        //{ Qt::Key_Direction_L, scim::SCIM_KEY_Direction_L }, These keys don't exist in X11
        //{ Qt::Key_Direction_R, scim::SCIM_KEY_Direction_R },

        { '/',                scim::SCIM_KEY_KP_Divide },
        { '*',                scim::SCIM_KEY_KP_Multiply },
        { '-',                scim::SCIM_KEY_KP_Subtract },
        { '+',                scim::SCIM_KEY_KP_Add },
        { Qt::Key_Return,     scim::SCIM_KEY_KP_Enter }
#if 0 && QT_VERSION >= 0x030100

// the next lines are taken from XFree > 4.0 (X11/XF86keysyms.h), defining some special
// multimedia keys. They are included here as not every system has them.
#define XF86XK_Standby          0x1008FF10
#define XF86XK_AudioLowerVolume 0x1008FF11
#define XF86XK_AudioMute        0x1008FF12
#define XF86XK_AudioRaiseVolume 0x1008FF13
#define XF86XK_AudioPlay        0x1008FF14
#define XF86XK_AudioStop        0x1008FF15
#define XF86XK_AudioPrev        0x1008FF16
#define XF86XK_AudioNext        0x1008FF17
#define XF86XK_HomePage         0x1008FF18
#define XF86XK_Calculator       0x1008FF1D
#define XF86XK_Mail             0x1008FF19
#define XF86XK_Start            0x1008FF1A
#define XF86XK_Search           0x1008FF1B
#define XF86XK_AudioRecord      0x1008FF1C
#define XF86XK_Back             0x1008FF26
#define XF86XK_Forward          0x1008FF27
#define XF86XK_Stop             0x1008FF28
#define XF86XK_Refresh          0x1008FF29
#define XF86XK_Favorites        0x1008FF30
#define XF86XK_AudioPause       0x1008FF31
#define XF86XK_AudioMedia       0x1008FF32
#define XF86XK_MyComputer       0x1008FF33
#define XF86XK_OpenURL          0x1008FF38
#define XF86XK_Launch0          0x1008FF40
#define XF86XK_Launch1          0x1008FF41
#define XF86XK_Launch2          0x1008FF42
#define XF86XK_Launch3          0x1008FF43
#define XF86XK_Launch4          0x1008FF44
#define XF86XK_Launch5          0x1008FF45
#define XF86XK_Launch6          0x1008FF46
#define XF86XK_Launch7          0x1008FF47
#define XF86XK_Launch8          0x1008FF48
#define XF86XK_Launch9          0x1008FF49
#define XF86XK_LaunchA          0x1008FF4A
#define XF86XK_LaunchB          0x1008FF4B
#define XF86XK_LaunchC          0x1008FF4C
#define XF86XK_LaunchD          0x1008FF4D
#define XF86XK_LaunchE          0x1008FF4E
#define XF86XK_LaunchF          0x1008FF4F
// end of XF86keysyms.h
        ,
        { Qt::Key_Standby,    XF86XK_Standby },
        { Qt::Key_VolumeDown, XF86XK_AudioLowerVolume },
        { Qt::Key_VolumeMute, XF86XK_AudioMute },
        { Qt::Key_VolumeUp,   XF86XK_AudioRaiseVolume },
        { Qt::Key_MediaPlay,  XF86XK_AudioPlay },
        { Qt::Key_MediaStop,  XF86XK_AudioStop },
        { Qt::Key_MediaPrev,  XF86XK_AudioPrev },
        { Qt::Key_MediaNext,  XF86XK_AudioNext },
        { Qt::Key_HomePage,   XF86XK_HomePage },
        { Qt::Key_LaunchMail, XF86XK_Mail },
        { Qt::Key_Search,     XF86XK_Search },
        { Qt::Key_MediaRecord, XF86XK_AudioRecord },
        { Qt::Key_LaunchMedia, XF86XK_AudioMedia },
        { Qt::Key_Launch1,    XF86XK_Calculator },
        { Qt::Key_Back,       XF86XK_Back },
        { Qt::Key_Forward,    XF86XK_Forward },
        { Qt::Key_Stop,       XF86XK_Stop },
        { Qt::Key_Refresh,    XF86XK_Refresh },
        { Qt::Key_Favorites,  XF86XK_Favorites },
        { Qt::Key_Launch0,    XF86XK_MyComputer },
        { Qt::Key_OpenUrl,    XF86XK_OpenURL },
        { Qt::Key_Launch2,    XF86XK_Launch0 },
        { Qt::Key_Launch3,    XF86XK_Launch1 },
        { Qt::Key_Launch4,    XF86XK_Launch2 },
        { Qt::Key_Launch5,    XF86XK_Launch3 },
        { Qt::Key_Launch6,    XF86XK_Launch4 },
        { Qt::Key_Launch7,    XF86XK_Launch5 },
        { Qt::Key_Launch8,    XF86XK_Launch6 },
        { Qt::Key_Launch9,    XF86XK_Launch7 },
        { Qt::Key_LaunchA,    XF86XK_Launch8 },
        { Qt::Key_LaunchB,    XF86XK_Launch9 },
        { Qt::Key_LaunchC,    XF86XK_LaunchA },
        { Qt::Key_LaunchD,    XF86XK_LaunchB },
        { Qt::Key_LaunchE,    XF86XK_LaunchC },
        { Qt::Key_LaunchF,    XF86XK_LaunchD }
#endif
};

#include <qstring.h> 

static int keyQtToSym(int keyQt) {
        int symQt = keyQt & 0xffff;

        if( (keyQt & Qt::UNICODE_ACCEL) || symQt < 0x1000 ) {
                return QChar(symQt).lower().unicode();
        }

        for( uint i = 0; i < sizeof(g_rgQtToSymX)/sizeof(TransKey); i++ ) {
                if( g_rgQtToSymX[i].keySymQt == symQt ) {
                        return  g_rgQtToSymX[i].keySymX;
                }
        }

        return 0;
}

inline int map_sym_to_qt(uint keySym)
{
    if( keySym < 0x1000 ) {
        if( keySym >= 'a' && keySym <= 'z' )
            return QChar(keySym).upper();
        return keySym;
    }
#ifdef Q_WS_WIN
        if( keySym < 0x3000 )
                return keySym;
#else
        if( keySym < 0x3000 )
                return keySym | Qt::UNICODE_ACCEL;

        for( uint i = 0; i < sizeof(g_rgQtToSymX)/sizeof(TransKey); i++ )
            if( g_rgQtToSymX[i].keySymX == keySym )
                return g_rgQtToSymX[i].keySymQt;
#endif
        return Qt::Key_unknown;
}

static bool symToKeyQt( uint keySym, int& keyQt )
{
    keyQt = map_sym_to_qt(keySym);
    return (keyQt != Qt::Key_unknown);
}
