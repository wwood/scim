/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#define USE_SCIM_KDE_SETTINGS
#include "socketserverthread.h"

#include "skimpluginmanager.h"
#include "skimglobalactions.h"

#include "utils/scimactions.h"

#include <qstringlist.h>
#include <qtimer.h>

#include <kapplication.h>

#define SCIM_DEFAULT_SETTINGS_GROUP "SCIM"

using namespace scim;
SocketServerThread::SocketServerThread(QObject *parent, QStringList & args)
: QObject(parent), QThread(), m_panel_agent(0) {
    SCIM_DEBUG_MAIN (1) << "Initialize Socket Server...\n";

    m_should_exit = false;

    m_default_config_module = "socket";
    for(uint argi=0; argi<args.size(); argi++)
      if(args[argi] == "c") {
        m_default_config_module = String(args[argi+1].latin1());
      }

    m_panel_agent_lock = new QMutex();

    m_config_module = new ConfigModule (m_default_config_module);
    if (m_config_module != NULL && m_config_module->valid ())
        m_scim_config = m_config_module->create_config ();
    else {
        std::cerr << "Config module \"" << m_default_config_module << "\"cannot be loaded, using dummy Config.\n Some settings may not be able to get reloaded without restarting.";

        if (m_config_module) delete m_config_module;
        m_config_module = NULL;

        m_scim_config = new DummyConfig ();
    }

    if(args.contains("no-stay"))
      m_should_resident = false;
    else
      m_should_resident = true;
    m_config = ScimKdeSettings::self()->config();
    
    QTimer * exitChecking = new QTimer(this);
    connect(exitChecking, SIGNAL(timeout()), SLOT(exitCheckingSlot()));
    exitChecking->start(500);
}

void SocketServerThread::exitCheckingSlot() {
    kapp->lock();
    if(m_should_exit)
        emit disconnectCompleted();
    kapp->unlock();
}
#include <qerrormessage.h>
bool SocketServerThread::initSocketServer(const QString & display) {
    m_panel_agent = new PanelAgent ();

    if (!m_panel_agent->initialize (m_default_config_module, String(display.utf8()), m_should_resident)) {
        delete m_panel_agent;
        m_panel_agent = 0;
        return false;
    }

    m_panel_agent->signal_connect_reload_config              (slot (this, &SocketServerThread::reloadScimConfig));
    m_panel_agent->signal_connect_turn_on                    (slot (this, &SocketServerThread::slot_turn_on));
    m_panel_agent->signal_connect_turn_off                   (slot (this, &SocketServerThread::slot_turn_off));
    m_panel_agent->signal_connect_update_screen              (slot (this, &SocketServerThread::slot_update_screen));
    m_panel_agent->signal_connect_update_spot_location       (slot (this, &SocketServerThread::slot_update_spot_location));
    m_panel_agent->signal_connect_update_factory_info        (slot (this, &SocketServerThread::slot_update_factory_info));
    m_panel_agent->signal_connect_show_help                  (slot (this, &SocketServerThread::slot_show_help));
    m_panel_agent->signal_connect_show_factory_menu          (slot (this, &SocketServerThread::slot_show_factory_menu));
    m_panel_agent->signal_connect_show_preedit_string        (slot (this, &SocketServerThread::slot_show_preedit_string));
    m_panel_agent->signal_connect_show_aux_string            (slot (this, &SocketServerThread::slot_show_aux_string));
    m_panel_agent->signal_connect_show_lookup_table          (slot (this, &SocketServerThread::slot_show_lookup_table));
    m_panel_agent->signal_connect_hide_preedit_string        (slot (this, &SocketServerThread::slot_hide_preedit_string));
    m_panel_agent->signal_connect_hide_aux_string            (slot (this, &SocketServerThread::slot_hide_aux_string));
    m_panel_agent->signal_connect_hide_lookup_table          (slot (this, &SocketServerThread::slot_hide_lookup_table));
    m_panel_agent->signal_connect_update_preedit_string      (slot (this, &SocketServerThread::slot_update_preedit_string));
    m_panel_agent->signal_connect_update_preedit_caret       (slot (this, &SocketServerThread::slot_update_preedit_caret));
    m_panel_agent->signal_connect_update_aux_string          (slot (this, &SocketServerThread::slot_update_aux_string));
    m_panel_agent->signal_connect_update_lookup_table        (slot (this, &SocketServerThread::slot_update_lookup_table));
    m_panel_agent->signal_connect_register_properties        (slot (this,
                                                                &SocketServerThread::slot_register_frontend_properties));
    m_panel_agent->signal_connect_update_property            (slot (this, &SocketServerThread::slot_update_frontend_property));
    m_panel_agent->signal_connect_register_helper_properties (slot (this,
                                                                    &SocketServerThread::slot_register_helper_properties));
    m_panel_agent->signal_connect_update_helper_property     (slot (this, &SocketServerThread::slot_update_helper_property));
    m_panel_agent->signal_connect_register_helper            (slot (this, &SocketServerThread::slot_register_helper));
    m_panel_agent->signal_connect_remove_helper              (slot (this, &SocketServerThread::slot_remove_helper));
    m_panel_agent->signal_connect_transaction_start          (slot (this, &SocketServerThread::slot_transaction_start));
    m_panel_agent->signal_connect_transaction_end            (slot (this, &SocketServerThread::slot_transaction_end));
    m_panel_agent->signal_connect_lock                       (slot (this, &SocketServerThread::slot_lock));
    m_panel_agent->signal_connect_unlock                     (slot (this, &SocketServerThread::slot_unlock));

    m_panel_agent->get_helper_list (m_helper_list);

#if ENABLE_DEBUG
    if(!m_helper_list.size()) {
        QErrorMessage * err = new QErrorMessage(0);
        err->message("size of _m_helper_list is zero!");
    }
#endif
    return true;
}

void SocketServerThread::run() {
    SCIM_DEBUG_MAIN(1) << " In Socket Server Thread...\n";
    if (!m_panel_agent || !m_panel_agent->run ())
        std::cerr << "Failed to run Panel.\n";
    kapp->lock();
    m_should_exit = true;
    kapp->unlock();
}

void SocketServerThread::start_auto_start_helpers (void)
{
    SCIM_DEBUG_MAIN(1) << "start_auto_start_helpers ()\n";

    // Add Helper object items.
    for (size_t i = 0; i < m_helper_list.size (); ++i) {
        if ((m_helper_list [i].option & SCIM_HELPER_AUTO_START) != 0) {
            m_panel_agent->start_helper (m_helper_list [i].uuid);
        }
    }
}

void SocketServerThread::start_helper(const String & uuid)
{
    m_panel_agent->start_helper (uuid);
}

void SocketServerThread::slot_update_preedit_string(const String &str, const AttributeList &attrs) {
    SCIM_DEBUG_MAIN (1) << "  socket_update_preedit_string...\n";
    emit( updatePreeditStringReq( QString::fromUtf8( str.c_str() ), attrs ) );
}

void SocketServerThread::slot_update_spot_location(int x, int y) {
    emit( updateSpotLocationReq( x, y ) );
}

void SocketServerThread::slot_update_frontend_property(const Property &prop) {
    slot_update_helper_property(-1, prop);
}

void SocketServerThread::slot_update_helper_property(int id, const Property &prop) {
    SCIM_DEBUG_MAIN (2) << "  socket_update_property on client\n";
    emit updateProperty(prop, id);
}

void SocketServerThread::slot_register_frontend_properties(const PropertyList &props) {
    slot_register_helper_properties(-1, props);
}

void SocketServerThread::slot_register_helper_properties (int id, const PropertyList &props) {
    SCIM_DEBUG_MAIN (1) << "  Register properties...\n";
    emit registerProperties(props, id);
}

void SocketServerThread::slot_register_helper (int id, const HelperInfo &helper)
{
    emit registerHelper(id, helper);
}

void SocketServerThread::slot_transaction_start (void)
{
//     SCIM_DEBUG_MAIN (0) << "  SocketServerThread::slot_bunch_transaction_start ++++++++...\n";
    kapp->lock();
    emit transaction_start();
}

void SocketServerThread::slot_transaction_end (void)
{
//     SCIM_DEBUG_MAIN (0) << "  SocketServerThread::slot_bunch_transaction_end ---------...\n";
    emit transaction_end();
    kapp->unlock();
}

void SocketServerThread::slot_lock (void)
{
    m_panel_agent_lock->lock();
}

void SocketServerThread::slot_unlock (void)
{
    m_panel_agent_lock->unlock();
}

void SocketServerThread::slot_remove_helper (int id)
{
    emit removeHelper(id);
}

void SocketServerThread::slot_show_preedit_string () {
    SCIM_DEBUG_MAIN (1) << "  Show preedit string...\n";
    emit( showPreeditStringReq() );
}

void SocketServerThread::slot_show_aux_string () {
    SCIM_DEBUG_MAIN (1) << "  Show aux string...\n";
    emit( showAuxStringReq() );
}

void SocketServerThread::slot_show_lookup_table () {
    SCIM_DEBUG_MAIN (1) << "  Show lookup table...\n";
    emit( showLookupTableReq() );
}

void SocketServerThread::slot_turn_on () {
    SCIM_DEBUG_MAIN (1) << "  Turn on panel...\n";
    emit( turnOnPanelReq() );
}

void SocketServerThread::slot_hide_preedit_string () {
    SCIM_DEBUG_MAIN (1) << "  Hide preedit string...\n";
    emit( hidePreeditStringReq() );
}

void SocketServerThread::slot_hide_aux_string () {
    SCIM_DEBUG_MAIN (1) << "  Hide aux string...\n";
    emit( hideAuxStringReq() );
}

void SocketServerThread::slot_hide_lookup_table () {
    SCIM_DEBUG_MAIN (1) << "  Hide lookup table...\n";
    emit( hideLookupTableReq() );
}

void SocketServerThread::slot_turn_off() {
    SCIM_DEBUG_MAIN (1) << "  socket_turn_off...\n";
    emit( turnOffPanelReq() );
}


void SocketServerThread::slot_update_screen (int screen) {
    SCIM_DEBUG_MAIN (1) << "  socket_update_screen...\n";
    emit( updateScreenReq( screen ) );
}
void SocketServerThread::slot_update_factory_info(const PanelFactoryInfo &info) {
    SCIM_DEBUG_MAIN (1) << "  socket_update_factory_info...\n";
    emit( updateFactoryInfoReq( info ) );
}

void SocketServerThread::slot_show_help(const String &help) {
    emit( showHelp( QString::fromUtf8( help.c_str() ) ) );
}

void SocketServerThread::slot_show_factory_menu (const std::vector <scim::PanelFactoryInfo> &menu) {
    SCIM_DEBUG_MAIN (1) << "  socket_show_factory_menu...\n";
    emit( showFactoryMenu( menu ) );
}

void SocketServerThread::slot_update_preedit_caret (int caret) {
//     SCIM_DEBUG_MAIN (1) << "  Update preedit caret...\n";
    emit( updatePreeditCaretReq( caret ) );
}

void SocketServerThread::slot_update_aux_string(const String &str, const AttributeList &attrs) {
    SCIM_DEBUG_MAIN (1) << "  Update aux string...\n";
    emit( updateAuxStringReq( QString::fromUtf8(str.c_str()), attrs ) );
}

void SocketServerThread::slot_update_lookup_table(const LookupTable &table) {
    SCIM_DEBUG_MAIN (1) << "  Update lookup table...\n";

    size_t item_num;

    emit( updateLookupTableReq( table, item_num ) );

    if ( item_num < (uint)table.get_current_page_size ())
        m_panel_agent->update_lookup_table_page_size (item_num);
}

void SocketServerThread::reloadScimConfig () {
    if(!m_panel_agent)
        return;

    m_panel_agent->reload_config ();

    if (!m_scim_config.null ()) m_scim_config->reload ();
}

SocketServerThread::~SocketServerThread () {
    m_scim_config.reset ();

    if (m_config_module) {
        SCIM_DEBUG_FRONTEND(2) << " Deleting _config_module...\n";
        delete m_config_module;
        m_config_module = 0;
    }

    delete m_panel_agent_lock;
}

void SocketServerThread::shutdown () {
    m_panel_agent->exit();
}

void SocketServerThread::selectLookupTableItem(int item) {
    m_panel_agent->select_candidate((uint32)item);
}

void SocketServerThread::lookupTablePageDown() {
    m_panel_agent->lookup_table_page_down ();
}
void SocketServerThread::lookupTablePageUp() {
    m_panel_agent->lookup_table_page_up ();
}
void SocketServerThread::activateProperty(int client, const scim::String & property_key)
{
    if(client < 0) {  //this is a frontend property
        m_panel_agent->trigger_property (property_key);
    } else {
        m_panel_agent->trigger_helper_property (client, property_key);
    }
}
void SocketServerThread::getFactoryList() {
    m_panel_agent->request_factory_menu ();
}

void SocketServerThread::changeFactory(const QString &uuid) {
    m_panel_agent->change_factory (String(uuid.latin1()));
}

void SocketServerThread::getHelp() {
    if (!m_panel_agent->request_help ()) {
        QString help 
             =  i18n("SKIM: KDE frontend for SCIM %1\n").arg(VERSION) +
                i18n("(C) 2003-2006 LiuCougar <liuspider@users.sourceforge.net>");
        emit showHelp(help);
    }
}
void SocketServerThread::setOverloadedModules(QStringList & l)
{
    if(m_overloadedScimModules != l) {
        m_overloadedScimModules = l;
        emit standaloneHelpersChanged();
    }
}

void SocketServerThread::getStandaloneHelperList(std::vector<HelperInfo> & stdal) const
{
    stdal.clear();
    for (size_t i = 0; i < m_helper_list.size (); ++i) {
        if ((m_helper_list [i].option & scim::SCIM_HELPER_STAND_ALONE) != 0 &&
            (m_helper_list [i].option & scim::SCIM_HELPER_AUTO_START) == 0 &&
            //if this scim module is not overloaded by a skim one
            !m_overloadedScimModules.contains(QString(m_helper_list [i].uuid.c_str())) ) {
            stdal.push_back(m_helper_list [i]);
        }
    }
}

#include "socketserverthread.moc"

