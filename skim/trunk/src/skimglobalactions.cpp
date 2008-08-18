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
#include "skimpluginmanager.h"
#include "skimglobalactions.h"
#include "utils/scimactions.h"

#include <qpixmap.h>
#include <qpopupmenu.h>
#include <kiconloader.h>
#include <khelpmenu.h>
#include <kapplication.h>

SkimGlobalActions::SkimGlobalActions(QObject *parent, const char *name)
: KActionCollection(0, parent, name), m_AboutApp(0), m_helpDialog(0),
  updateFrontendPropertiesNeeded(false), updateGUIPropertiesNeeded(false){
    m_mc = static_cast<SkimPluginManager *>(parent);
    m_inputServer = m_mc->getInputServer();

    m_helperActions = new KActionCollection(0, parent, "helper actions");

    m_HelpMenu = new KHelpMenu( 0 );
    connect(m_HelpMenu,SIGNAL(showAboutApplication()), this, SLOT(aboutApp()));

    new ScimAction( KGuiItem( i18n( "SCIM Help" ), "scim_help" ),
                 "", m_inputServer, SLOT(getHelp()), this, "scim_help");
    new ScimAction( KGuiItem( i18n( "SCIM Setup" ), "scim_setup" ),
                 "", this, SLOT(startSCIMConfigureHelper()), this, "scim_setup");

    KStdAction::help(m_HelpMenu, SLOT(appHelpActivated()), this);
    KStdAction::aboutKDE(m_HelpMenu,SLOT(aboutKDE ()),this,"aboutkde");
    KStdAction::aboutApp(this,SLOT(aboutApp()),this,"aboutapp");

#if ENABLE_DEBUG
    new ScimAction( KGuiItem( i18n( "Reload all modules" ), "reload" ),
                 "", this, SLOT(requestReloadAllModules()), this, "reload");

    KStdAction::quit(m_mc, SLOT(shutdown()), this);
#endif

    connect( m_inputServer, SIGNAL(showHelp(const QString & ) ) , this, SLOT(showHelp(const QString & )));

    m_serverAction = new ScimComboAction(i18n("Input Method"), KGlobal::iconLoader()->loadIcon("scim_keyboard", KIcon::Small),
      "", m_inputServer, SLOT(getFactoryList()), this,"change_server");
    connect(m_serverAction, SIGNAL(itemActivated(const QString &)),
      SLOT(changeFactory(const QString &)));

    connect( m_inputServer, SIGNAL( updateFactoryInfoReq( const scim::PanelFactoryInfo &) ),
      SLOT( SetFactoryInfo( const scim::PanelFactoryInfo &) ) );

    connect( m_inputServer,
             SIGNAL(showFactoryMenu(const std::vector <scim::PanelFactoryInfo> &)) ,
             SLOT(receiveFactoryMenu(const std::vector <scim::PanelFactoryInfo> &)));

//     connect(mc, SIGNAL(settingsChanged()), SLOT(changeSetting()));

    m_externalActions = new KActionCollection(0, parent, "external actions");

    connect(m_inputServer, SIGNAL(registerProperties(const scim::PropertyList&, int)),
      SLOT(registerProperties(const scim::PropertyList&, int)));
    connect(m_inputServer, SIGNAL(removeHelper(int)), SLOT(deleteProperties(int)));
    connect(m_inputServer,SIGNAL(updateProperty(const scim::Property&, int )),
      SLOT(updateProperty(const scim::Property&, int )));

    //only after loadAllPlugins, overload standalone module list can be established
    connect(m_inputServer,SIGNAL(standaloneHelpersChanged()), this, SLOT(initStantaloneHelperActions()));

    connect(m_inputServer, SIGNAL( transaction_end() ), SLOT( endTransaction() ));
}

void SkimGlobalActions::startSCIMConfigureHelper()
{
    m_inputServer->start_helper("8034d025-bdfc-4a10-86a4-82b9461b32b0");
}

void SkimGlobalActions::initStantaloneHelperActions() {
    for(uint i=0; i<m_helperActions->count();++i)
    {
        m_helperActions->action(i)->unplugAll();
    }

    m_helperActions->clear();

    //add standalone helper into an actioncollection
    std::vector<scim::HelperInfo>  _helper_list;
    m_inputServer->getStandaloneHelperList(_helper_list);

    m_helper_uuids.clear();
    for (size_t i = 0; i < _helper_list.size (); ++i) {
        m_helper_uuids.push_back(_helper_list [i].uuid);
            QString text = QString::fromUtf8(_helper_list [i].name.c_str ());
            ScimAction * node = new ScimAction(text, m_helperActions,
            i, _helper_list [i].uuid.c_str ());
//             node->setOption(scim_kde::ToolBarPreferIcon);
            node->setVisible(true);
            node->setEnabled(true);
            node->setDisplayedText(text);
            QString filename = QString::fromUtf8(_helper_list [i].icon.c_str());
            node->setIcon(filename);
            node->setWhatsThis(QString::fromUtf8(_helper_list [i].description.c_str ()));
            connect(node, SIGNAL(activated(int)), SLOT(helperActivated(int)));
    }

    emit standaloneHelperActionsChanged();
}

SkimGlobalActions::~SkimGlobalActions() {
    m_HelpMenu->deleteLater();
    if( m_helpDialog )
        m_helpDialog->deleteLater();
    if( m_AboutApp )
        m_AboutApp->deleteLater();
}

void SkimGlobalActions::helperActivated(int i) {
    if ( static_cast<size_t>(i) < m_helper_uuids.size() )
         m_inputServer->start_helper(m_helper_uuids[i]);
}

void SkimGlobalActions::requestReloadAllModules() {
    m_mc->reloadAllPlugins();
}

void SkimGlobalActions::receiveFactoryMenu(const std::vector <scim::PanelFactoryInfo> &menu) {
    if(menu.size() != m_uuids.size()) {
        m_uuids.clear();

        std::map <scim::String, std::vector <size_t> > groups;
        for (size_t i = 0; i < menu.size (); ++i)
            groups [menu [i].lang].push_back (i);

        m_serverAction->clear();

        if(groups.size() == 1)
            for(uint i = 0; i < menu.size(); ++i) {
                m_serverAction->insertItem("/" + QString::fromLatin1(menu[i].uuid.c_str()),
                                           KGlobal::iconLoader()->loadIcon(
                                                   QString::fromLocal8Bit(menu[i].icon.c_str()), KIcon::Small),
                                           QString::fromUtf8(menu[i].name.c_str()), false);
                m_uuids.push_back(menu[i].uuid.c_str());
            }
        else
            for (std::map <scim::String, std::vector <size_t> >::iterator it = groups.begin ();
                    it != groups.end (); ++ it) {
                scim::String s = scim::scim_get_language_name((it->first).c_str());
                QString parentPath = "/" + QString::number(std::distance(groups.begin (), it));
                m_serverAction->insertItem(parentPath, 0, QString::fromUtf8(s.c_str()), true);
                for (unsigned i = 0; i < it->second.size (); ++i) {
                    m_serverAction->insertItem(parentPath + "/" +
                        QString::fromLatin1(menu[it->second [i]].uuid.c_str()),
                        KGlobal::iconLoader()->loadIcon(
                                QString::fromLocal8Bit(menu[it->second [i]].icon.c_str()), KIcon::Small),
                        QString::fromUtf8(menu[it->second[i]].name.c_str()), false);
                    m_uuids.push_back(menu[it->second [i]].uuid.c_str());
                }
            }

        //append an entry for direct keyboard
        m_serverAction->insertItem("/", KGlobal::iconLoader()->loadIcon("scim_keyboard", KIcon::Small), i18n("Keyboard"));

        //since the content of this popupmenu has been changed, repopup it to settle it
        //in the proper place
        if(m_serverAction->popup()->isVisible()) {
            m_serverAction->popup()->hide();
        }
    }
    //if popup is not visible, we should popup it (when user press the shortcut to trigger this menu)
    if(!m_serverAction->popup()->isVisible())
      m_serverAction->slotPopup();

    return;
}

void SkimGlobalActions::propertyActivated(int id) {
    if(m_global_property_dict.contains(id)) {
      kapp->lock();
      m_inputServer->activateProperty(m_global_property_dict[id].first,
        m_property_repository[m_global_property_dict[id].first][m_global_property_dict[id].second].property.get_key());
      kapp->unlock();
    }
}

void SkimGlobalActions::deleteProperties(int client) {
    bool changed = false;
    emit preparePropertiesRemove(client < 0);

    if(m_property_repository.contains(client)) {
        ScimAction * action;
        for(PropertyRepository::iterator it = m_property_repository[client].begin ();
        it != m_property_repository[client].end (); ++it) {
            switch (it->objectType) {
                case scim_kde::Action:
                case scim_kde::ComboAction:
                    action = it->parentObject.action;
                    if(client < 0)
                        m_frontendPropertyActions.remove(action);
                    else
                        m_guiPropertyActions.remove(action);
                    changed = true;

                    action->unplugAll();
                    action->deleteLater();
                break;
                case scim_kde::Popup:
                break;
                default:
                std::cerr<< "Unknow object type associated with a property in deleteProperties\n";
            }
        }
        m_property_repository.erase(client);
    }
    if(changed) emit propertiesRemoved(client < 0);
}

void SkimGlobalActions::registerProperties(const scim::PropertyList & properties, int client) {
    create_properties (m_property_repository,
                          properties, properties.begin(),
                          properties.end(), client);

    if(client < 0) { //registering a new set of frontend properties,
        scim::PropertyList::const_iterator pit = properties.begin ();
        PropertyRepository::iterator prit;

        m_frontendPropertyActions.clear();
        pit = properties.begin ();

        if(m_property_repository.contains(client)) {
            //add all registered properties to m_frontendPropertyActions
            for (; pit != properties.end (); ++pit) {
                prit = std::find(m_property_repository[client].begin(), m_property_repository[client].end(),*pit);
                if( prit != m_property_repository[client].end())
                {
                    switch (prit->objectType) {
                        case scim_kde::Action:
                        case scim_kde::ComboAction:
                            m_frontendPropertyActions.push_back(prit->parentObject.action);
                            //fall through
                        case scim_kde::Popup:
                            break;
                        default:
                            std::cerr<< "Unknow object type associated with a property\n";
                    }
                }
            }
        }
    }

    emit propertiesRegistered(client < 0);
}

void SkimGlobalActions::create_properties(ClientPropertyRepository &repository,
                                          const scim::PropertyList &/*properties*/, 
                                          scim::PropertyList::const_iterator  begin,
                                          scim::PropertyList::const_iterator  end,
                                          int client) {
    static int root_property_id = 0;
    scim::PropertyList::const_iterator it;
    scim::PropertyList::const_iterator next;

    if (begin == end) return;

    it = begin;
    next = begin + 1;

    while (it != end) {
        if (next == end || !next->is_a_leaf_of (*it)) {
            QString text = QString::fromUtf8(it->get_label ().c_str()),
                    name = QString::fromUtf8(it->get_key ().c_str());
                QPair<int, int> dictcand(client, 0);
                if(next != it + 1) {  //this property has child(ren), create comboaction
                  ScimComboAction * node = 0;
                  PropertyRepository::const_iterator eNodeIt; 
                  ClientPropertyInfo info = {*it, client, 0, scim_kde::ComboAction};
                  if(!repository.contains(client))
                    repository[client] = PropertyRepository();
                  if((eNodeIt = 
                        std::find(repository[client].begin(),  repository[client].end(),*it)) == repository[client].end()) {
                    info.globalUniqueID = root_property_id++;

                    node = new ScimComboAction(text, "", 0, 0, m_externalActions, it->get_key ().c_str());
                    if(client >= 0)
                        m_guiPropertyActions.push_back(node);
                    info.parentObject.comboaction = node;
                    node->setDisplayedText(text);
                    if(it->get_icon ().length())
                        node->setCurrentIconSet(
                            KGlobal::iconLoader()->loadIconSet(QString::fromLocal8Bit(it->get_icon ().c_str()), KIcon::Small ) );
                    node->setOption(scim_kde::ToolBarPreferIcon);
//                     _propertyActions.push_back(node);
                    dictcand.second = repository[client].size();
                    m_global_property_dict[info.globalUniqueID] = dictcand;
                    repository[client].push_back(info);
                    connect(node, SIGNAL(itemActivated(int)), SLOT(propertyActivated(int)));
                  } else {
                    if (eNodeIt->objectType == scim_kde::ComboAction) {
                      node = eNodeIt->parentObject.comboaction;
                      info = *eNodeIt;
                    } else {
                      std::cerr << "Oops, the root property " << it->get_key ().c_str() 
                        << " has been registered, but it was not a comboaction!\nI would skip all children properties of this one.";
                      it = next;
                      next++;
                      continue;
                    }
                  }
                  //add all children of this property as a (multi-depth) menu
                  info.objectType = scim_kde::Popup;
                  it++;
                  scim::PropertyList::const_iterator nodeEnd = next, localNextIt;
                  localNextIt = it + 1;
                  for(; it != nodeEnd; it++, localNextIt++) {
                    eNodeIt = std::find(repository[client].begin(),  repository[client].end(),*it);
                    if(eNodeIt == repository[client].end()) {  //this property hasn't been registered
                      text = QString::fromUtf8(it->get_label ().c_str()),
                      name = QString::fromUtf8(it->get_key ().c_str());
                      info.property = *it;
                      info.globalUniqueID = root_property_id++;
                      if(localNextIt == nodeEnd || !localNextIt->is_a_leaf_of (*it)) { //a normal menu entry
                          info.globalUniqueID = node->insertItem(name,
                                  KGlobal::iconLoader()->loadIcon(
                                          QString::fromLocal8Bit(it->get_icon ().c_str()), KIcon::Small, 0,
                                          KIcon::DefaultState, 0, true),
                                  text, false, info.globalUniqueID);
                      } else {  //a popup menu
                          info.globalUniqueID = node->insertItem(name,
                                  KGlobal::iconLoader()->loadIcon(
                                          QString::fromLocal8Bit(it->get_icon ().c_str()), KIcon::Small, 0,
                                          KIcon::DefaultState, 0, true),
                                  text, true, info.globalUniqueID);
                      }
                    dictcand.second = repository[client].size();
                    m_global_property_dict[info.globalUniqueID] = dictcand;
                    repository[client].push_back(info);
                    } else {  //existing property has been updated in the registerProperties function, so nothing to do here
//                         std::cout << "existing property has been updated in the registerProperties function, so nothing to do here\n";
//                         updateProperty (eNodeIt, *it, client);
                    }
                  }
                } else {  //no child, create a std action
                  PropertyRepository::const_iterator eNodeIt = 
                      std::find(repository[client].begin(),  repository[client].end(),*it);
                  if(eNodeIt == repository[client].end()) {
                      int cid = root_property_id++;
                      ScimAction * node = new ScimAction(text, m_externalActions,
                        cid, it->get_key ().c_str());
                      if(client >= 0)
                          m_guiPropertyActions.push_back(node);
                      node->setOption(scim_kde::ToolBarPreferIcon);
                      node->setVisible(it->visible());
                      node->setEnabled(it->active());
                      node->setDisplayedText(text);
                      if(it->get_icon ().length())
                        node->setCurrentIconSet(
                            KGlobal::iconLoader()->loadIconSet(QString::fromLocal8Bit(it->get_icon ().c_str()), KIcon::Small ));
//                       _propertyActions.push_back(node);
                      ClientPropertyInfo info = {*it, cid, node, scim_kde::Action};
                    dictcand.second = repository[client].size();
                    m_global_property_dict[info.globalUniqueID] = dictcand;
                    repository[client].push_back(info);
                      connect(node, SIGNAL(activated(int)), SLOT(propertyActivated(int)));
                  } else {  //existing property has been updated in the registerProperties function, so nothing to do here
//                       updateProperty (eNodeIt, *it, client);
                  }
                }
            it = next;
        }
        ++ next;
    }
}

void SkimGlobalActions::endTransaction()
{
    if(updateFrontendPropertiesNeeded)
    {
        updateFrontendPropertiesNeeded = false;
        emit propertyChanged(true);
    }

    if(updateGUIPropertiesNeeded)
    {
        updateGUIPropertiesNeeded = false;
        emit propertyChanged(false);
    }
}

void SkimGlobalActions::updateProperty(PropertyRepository::iterator &it,
  const scim::Property &property, int client) {
    QString text = QString::fromUtf8(property.get_label ().c_str());
    const char * pix = property.get_icon ().c_str();

    ScimAction *action;
    ScimComboAction *comboaction;
    switch (it->objectType) {
      case scim_kde::Action:
          action = it->parentObject.action;
          action->setDisplayedText(text);
          if(property.get_icon ().length())
            action->setCurrentIconSet(
                    KGlobal::iconLoader()->loadIconSet(QString::fromLocal8Bit(pix), KIcon::Small ));

          action->setEnabled(property.active ());
          if(action->visible() != property.visible() || action->currentShown() == false) {
            action->setCurrentShown(true);
            action->setVisible(property.visible());
            if(client < 0)
                updateFrontendPropertiesNeeded = true;
            else
                updateGUIPropertiesNeeded = true;
          }
        break;
      case scim_kde::ComboAction:
          comboaction = it->parentObject.comboaction;
          comboaction->setDisplayedText(text);
          if(property.get_icon ().length())
            comboaction->setCurrentIconSet(
                    KGlobal::iconLoader()->loadIconSet(QString::fromLocal8Bit(pix), KIcon::Small ));

          comboaction->setEnabled(property.active ());
          if(comboaction->visible() != property.visible() || comboaction->currentShown() == false) {
            comboaction->setCurrentShown(true);
            comboaction->setVisible(property.visible());
            if(client < 0)
                updateFrontendPropertiesNeeded = true;
            else
                updateGUIPropertiesNeeded = true;
          }
        break;
      case scim_kde::Popup:
          comboaction = it->parentObject.comboaction;
          if(strlen(pix))
              comboaction->changeItem(
                      KGlobal::iconLoader()->loadIcon(QString::fromLocal8Bit(pix), KIcon::Small),
                      text, it->globalUniqueID);
          comboaction->setCurrentShown(true);
          comboaction->setItemVisible(it->globalUniqueID, property.visible());
          comboaction->setItemEnabled(it->globalUniqueID, property.active ());
        break;
      default:
        std::cerr<< "Unknow object type associated with a property\n";
    }

    it->property = property;
}
void SkimGlobalActions::updateProperty(const scim::Property &property, int client) {
    PropertyRepository::iterator it = 
        std::find(m_property_repository[client].begin(), m_property_repository[client].end(), property);
    if(it != m_property_repository[client].end())
        updateProperty(it, property, client);
}
void SkimGlobalActions::changeFactory(const QString & path) {
    QString uuid = path.section('/',-1);
    kapp->lock();
    if(!uuid.isNull())
      m_inputServer->changeFactory(uuid);
    else
      m_inputServer->changeFactory("");
    kapp->unlock();
}

void SkimGlobalActions::SetFactoryInfo( const scim::PanelFactoryInfo & info ) {
    static scim::String old_uuid = "";
    if(old_uuid != info.uuid) {
        QString newservername = QString::fromUtf8( info.name.c_str() );
        m_serverAction->setDisplayedText(newservername);
        if(info.icon.length())
            m_serverAction->setCurrentIconSet(
                    KGlobal::iconLoader()->loadIconSet(QString::fromLocal8Bit(info.icon.c_str()), KIcon::Small ));
        old_uuid = info.uuid;

        //all displayed frontend properties have to be hidden, for the new IMEngine may not have any properties
        m_frontendPropertyActions.clear();
        emit propertiesRegistered(true); //update toolbar etc
    }
}

void SkimGlobalActions::aboutApp() {
    if( m_AboutApp == 0 ) {
        m_AboutApp = new KAboutApplication( m_mc->getAboutData(), 0, "about", false );
    }
    m_AboutApp->show();
}
void SkimGlobalActions::showHelp( const QString & txt) {
    if( m_helpDialog == 0 )
        m_helpDialog = new ScimHelpDialog(0);

    if( !m_helpDialog->isVisible() ) {
        m_helpDialog->setGeneralInfo( txt );
        m_helpDialog->show();
    }
    if( m_helpDialog->isMinimized())
        m_helpDialog->showNormal();
    else
        m_helpDialog->raise();
}

#include "skimglobalactions.moc"
