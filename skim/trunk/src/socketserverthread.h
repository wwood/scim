/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef SOCKETSERVERTHREAD_H
#define SOCKETSERVERTHREAD_H

#define Uses_C_STDIO
#define Uses_SCIM_LOOKUP_TABLE
#define Uses_SCIM_SOCKET
#define Uses_SCIM_TRANSACTION
#define Uses_SCIM_TRANS_COMMANDS
#define Uses_SCIM_CONFIG
#define Uses_SCIM_CONFIG_MODULE
#define Uses_SCIM_DEBUG
#define Uses_SCIM_TRANSACTION
#define Uses_SCIM_HELPER
#define Uses_SCIM_HELPER_MODULE
#define Uses_SCIM_PANEL_AGENT
#define Uses_STL_MAP

#ifndef NO_CONFIG_H
#include "config.h"
#endif

#include <scim.h>

#include <qstringlist.h>
#include <qthread.h>
#include <qobject.h>

#ifdef USE_SCIM_KDE_SETTINGS
#include "utils/scimkdesettings.h"
#endif

namespace scim{
class SocketServerThread : public QObject, public QThread
{
    Q_OBJECT
private:
    PanelAgent *        m_panel_agent;
    bool                m_should_exit;
    bool                m_should_resident;
    ConfigPointer       m_scim_config;
    ConfigModule*       m_config_module;
    String              m_default_config_module;
    class KConfig *     m_config;
    std::vector<HelperInfo> m_helper_list;
    class QMutex * m_panel_agent_lock;
    QStringList m_overloadedScimModules;

signals:
    void x11FrontEndConnected();
    void lastConnectionFinished();
    void standaloneHelpersChanged();

    void turnOnPanelReq();
    void turnOffPanelReq();
    void updateAuxStringReq( const QString &, const scim::AttributeList & );
    void updateDisplayReq( QString );
    void updateScreenReq( uint32 );
    void updateFactoryInfoReq( const scim::PanelFactoryInfo & );
    void registerProperties( const scim::PropertyList &, int);
    void registerHelper(int /*id*/, const HelperInfo &/*helper*/);
    void removeHelper(int);
    void updateProperty(const scim::Property&, int);
    void updateSpotLocationReq( int, int );
    void updatePreeditStringReq(const QString &, const scim::AttributeList &);
    void updateLookupTableReq( const scim::LookupTable &, size_t &  );
    void showAuxStringReq();
    void showPreeditStringReq();
    void showStatusStringReq();
    void showFactoryMenu(const std::vector <scim::PanelFactoryInfo> &/*menu*/);
    void showLookupTableReq();
    void showHelp(const QString &);
    void hidePreeditStringReq();
    void hideStatusStringReq();
    void hideAuxStringReq();
    void hideLookupTableReq();
    void updatePreeditCaretReq( int );
    void disconnectCompleted();

    void transaction_start();
    void transaction_end();

public:
    SocketServerThread(QObject *parent, QStringList & args );
    ~SocketServerThread();
    void       start_auto_start_helpers             (void);

    void       slot_reload_config                   (void);
    void       slot_turn_on                         (void);
    void       slot_turn_off                        (void);
    void       slot_update_screen                   (int screen);
    void       slot_update_spot_location            (int x, int y);
    void       slot_update_factory_info             (const PanelFactoryInfo &info);
    void       slot_show_help                       (const String &help);
    void       slot_show_factory_menu               (const std::vector <scim::PanelFactoryInfo> &menu);

    void       slot_show_preedit_string             (void);
    void       slot_show_aux_string                 (void);
    void       slot_show_lookup_table               (void);
    void       slot_hide_preedit_string             (void);
    void       slot_hide_aux_string                 (void);
    void       slot_hide_lookup_table               (void);
    void       slot_update_preedit_string           (const String &str, const AttributeList &attrs);
    void       slot_update_preedit_caret            (int caret);
    void       slot_update_aux_string               (const String &str, const AttributeList &attrs);
    void       slot_update_lookup_table             (const LookupTable &table);
    void       slot_register_frontend_properties    (const PropertyList &props);
    void       slot_update_frontend_property        (const Property &prop);

    void       slot_register_helper_properties      (int id, const PropertyList &props);
    void       slot_update_helper_property          (int id, const Property &prop);
    void       slot_register_helper                 (int id, const HelperInfo &helper);
    void       slot_remove_helper                   (int id);
    void       slot_transaction_start               (void);
    void       slot_transaction_end                 (void);
    void       slot_lock                            (void);
    void       slot_unlock                          (void);

    virtual void run();
    bool initSocketServer(const QString &);
    void getStandaloneHelperList(std::vector<HelperInfo> &) const;
    void setOverloadedModules(QStringList & l);

    void start_helper(const String &);

public slots:

    void shutdown();
    void selectLookupTableItem(int);
    void lookupTablePageUp();
    void lookupTablePageDown();
    void activateProperty(int client, const scim::String & key);

    virtual void reloadScimConfig();
    virtual void getFactoryList();

    void getHelp();
    void changeFactory(const QString &);
protected slots:
    void exitCheckingSlot();
};
}

#endif
