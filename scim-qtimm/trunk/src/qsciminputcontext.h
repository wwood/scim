/*
  Copyright (C) 2004-2005 LiuCougar <liuspider@users.sourceforge.net>
*/
#ifndef _QSCIMINPUT_CONTEXT_H_
#define _QSCIMINPUT_CONTEXT_H_

//#include "config.h"

#define Uses_SCIM_DEBUG
#define Uses_SCIM_BACKEND
#define Uses_SCIM_IMENGINE
#define Uses_SCIM_IMENGINE_MODULE
#define Uses_SCIM_CONFIG
#define Uses_SCIM_CONFIG_MODULE
#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_TRANSACTION
#define Uses_SCIM_TRANS_COMMANDS
#define Uses_SCIM_HOTKEY
#define Uses_SCIM_PANEL_CLIENT
#include <scim.h>

#include <QtGui/qinputcontext.h>
#include <qobject.h>
#include <QEvent>

#include <libintl.h>

#define _(String) dgettext(PACKAGE,String)

// #undef Q_WS_X11

namespace scim {

class QScimInputContextGlobal;

class QScimInputContext : public QInputContext
{
    friend class QScimInputContextGlobal;

Q_OBJECT
private:
    int                      m_id;                // Context ID to identify this input context.

    IMEngineInstancePointer  m_instance;
    QString                  m_preedit_string;
    int                      m_preedit_caret;
    int                      m_preedit_sellen;
    QPoint                   m_cursor;

    bool                     m_is_on;
    bool                     m_shared_instance;

    QList<QInputMethodEvent::Attribute> m_attribs;

public:
    QScimInputContext ();
    virtual ~QScimInputContext ();

    virtual QString identifierName ();
    virtual QString language ();

#if defined(Q_WS_X11)
    virtual bool x11FilterEvent (QWidget *keywidget, XEvent *event);
#endif

    virtual bool filterEvent (const QEvent *event);

    virtual void update ();
    virtual void reset ();
    virtual void setFocus ();
    virtual void unsetFocus ();
//     virtual void setMicroFocus (int x, int y, int w, int h, QFont *f = 0);
    virtual void mouseHandler ( int x, QMouseEvent * event );
    virtual bool isComposing() const;
    virtual void setFocusWidget( QWidget *w );

public:
    void finalize ();

    bool filterScimEvent (const KeyEvent &key);

private:
    void open_specific_factory (const String &uuid);
    void open_next_factory ();
    void open_previous_factory ();

    bool filter_hotkeys (const KeyEvent &key);

    void turn_on_ic ();
    void turn_off_ic ();

    void panel_req_update_display ();
    void panel_req_update_screen ();
    void panel_req_show_help ();
    void panel_req_show_factory_menu ();
    void panel_req_focus_in ();

    void panel_req_update_factory_info ();
    void panel_req_update_spot_location ();

    // Utility Functions
    bool commit_string (QString s);
    void set_ic_capabilities ();

    // Static utility functions
    static void attach_instance (const IMEngineInstancePointer &instance);
    static QScimInputContext * find_ic (int context);

public: //scim slots
    static void slot_show_preedit_string   (IMEngineInstanceBase   *si);
    static void slot_show_aux_string       (IMEngineInstanceBase   *si);
    static void slot_show_lookup_table     (IMEngineInstanceBase   *si);
    
    static void slot_hide_preedit_string   (IMEngineInstanceBase   *si);
    static void slot_hide_aux_string       (IMEngineInstanceBase   *si);
    static void slot_hide_lookup_table     (IMEngineInstanceBase   *si);
    
    static void slot_update_preedit_caret  (IMEngineInstanceBase   *si,
                                            int                     caret);
    static void slot_update_preedit_string (IMEngineInstanceBase   *si,
                                            const WideString       &str,
                                            const AttributeList    &attrs);
    static void slot_update_aux_string     (IMEngineInstanceBase   *si,
                                            const WideString       &str,
                                            const AttributeList    &attrs);
    static void slot_commit_string         (IMEngineInstanceBase   *si,
                                            const WideString       &str);
    static void slot_forward_key_event     (IMEngineInstanceBase   *si,
                                            const KeyEvent         &key);
    static void slot_update_lookup_table   (IMEngineInstanceBase   *si,
                                            const LookupTable      &table);
    
    static void slot_register_properties   (IMEngineInstanceBase   *si,
                                            const PropertyList     &properties);
    static void slot_update_property       (IMEngineInstanceBase   *si,
                                            const Property         &property);
    static void slot_beep                  (IMEngineInstanceBase   *si);
    static void slot_start_helper          (IMEngineInstanceBase   *si,
                                            const String           &helper_uuid);
    static void slot_stop_helper           (IMEngineInstanceBase   *si,
                                            const String           &helper_uuid);
    static void slot_send_helper_event     (IMEngineInstanceBase   *si,
                                            const String           &helper_uuid,
                                            const Transaction      &trans);
    static bool slot_get_surrounding_text   (IMEngineInstanceBase   *si,
                                            WideString             &text,
                                            int                    &cursor,
                                            int                     maxlen_before,
                                            int                     maxlen_after);
    static bool slot_delete_surrounding_text(IMEngineInstanceBase   *si,
                                            int                     offset,
                                            int                     len);

    //Panel Client slots
    static void panel_slot_exit                         (int                     context);
    static void panel_slot_update_lookup_table_page_size(int                     context,
                                                         int                     page_size);
    static void panel_slot_lookup_table_page_up         (int                     context);
    static void panel_slot_lookup_table_page_down       (int                     context);
    static void panel_slot_trigger_property             (int                     context,
                                                         const String           &property);
    static void panel_slot_process_helper_event         (int                     context,
                                                         const String           &target_uuid,
                                                         const String           &helper_uuid,
                                                         const Transaction      &trans);
    static void panel_slot_move_preedit_caret           (int                     context,
                                                         int                     caret_pos);
    static void panel_slot_select_candidate             (int                     context,
                                                         int                     cand_index);
    static void panel_slot_process_key_event            (int                     context,
                                                         const KeyEvent         &key);
    static void panel_slot_commit_string                (int                     context,
                                                         const WideString       &wstr);
    static void panel_slot_forward_key_event            (int                     context,
                                                         const KeyEvent         &key);
    static void panel_slot_request_help                 (int                     context);
    static void panel_slot_request_factory_menu         (int                     context);
    static void panel_slot_change_factory               (int                     context,
                                                         const String           &uuid);
};

class PanelIOReceiver : public QObject
{
Q_OBJECT
public slots:
    void panel_iochannel_handler();
};

};
#endif /* Not def: _QSCIMINPUT_CONTEXT_H_ */

/*
vi:ts=4:nowrap:ai:expandtab
*/
