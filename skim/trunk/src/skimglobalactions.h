/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef SCIMGLOBALACTIONS_H
#define SCIMGLOBALACTIONS_H

#include "skimpluginmanager.h"

#include <kactioncollection.h>
#include <kaboutapplication.h>
#include <kaboutdata.h>

#include "scimhelpdialog.h"

class SkimPluginManager;
class ScimAction;
class ScimComboAction;

namespace scim_kde{
  union ClientObject{
  class ScimAction *action;
  class ScimComboAction *comboaction;
  class QPopupMenu *popup;
  };
  enum ClientObjectType {Unknown, Action, ComboAction, Popup};
}

struct ClientPropertyInfo {
    scim::Property   property;
    int globalUniqueID;    //the index in GlobalPropertyRepositoryIDDict
    scim_kde::ClientObject parentObject;
    scim_kde::ClientObjectType objectType;
};

typedef QMap <int, QPair<int, int > >                GlobalPropertyRepositoryIDDict;
typedef std::vector <ClientPropertyInfo>             PropertyRepository;
typedef QMap <int, PropertyRepository >              ClientPropertyRepository;

inline bool operator == (const ClientPropertyInfo &lhs, const scim::Property &rhs) {
    return lhs.property == rhs;
}

class SkimGlobalActions : public KActionCollection
{
Q_OBJECT
public:
    SkimGlobalActions(QObject *parent = 0, const char *name = 0);
    ~SkimGlobalActions();
    KActionCollection* externalActionCollection () {return m_externalActions;};
    KActionCollection* helperActionCollection () {return m_helperActions;};
    KActionPtrList guiPropertyActions() {return m_guiPropertyActions;};
    KActionPtrList frontendPropertyActions() {return m_frontendPropertyActions;};
signals:
    void propertiesRegistered(bool isFrontEndProperties);
    void preparePropertiesRemove(bool isFrontEndProperties);
    void propertiesRemoved(bool isFrontEndProperties);
    void propertyChanged(bool isFrontEndProperties);
    void standaloneHelperActionsChanged();
public slots:
    void registerProperties(const scim::PropertyList & properties, int client);
    void updateProperty(const scim::Property &property, int client);

protected slots:
    void initStantaloneHelperActions();
    void startSCIMConfigureHelper();

    void propertyActivated(int);
    void helperActivated(int);
    void deleteProperties(int client);
    void aboutApp();
    void showHelp( const QString & );

    void SetFactoryInfo( const scim::PanelFactoryInfo & );
    void changeFactory(const QString & path);
    void receiveFactoryMenu(const std::vector <scim::PanelFactoryInfo> &menu);

    void requestReloadAllModules();

    void endTransaction();

protected:
    void updateProperty(PropertyRepository::iterator &it, const scim::Property &property, int client);
    void create_properties(ClientPropertyRepository &repository,
                                          const scim::PropertyList &properties, 
                                          scim::PropertyList::const_iterator  begin,
                                          scim::PropertyList::const_iterator  end,
                                          int client);

    SkimPluginManager* m_mc;

    KActionCollection* m_externalActions;
    KActionCollection* m_helperActions;

    ScimComboAction* m_serverAction;

    KAboutApplication* m_AboutApp;
    ScimHelpDialog* m_helpDialog;

private:
    std::vector<scim::String> m_uuids;
    std::vector<scim::String> m_helper_uuids;

    scim::SocketServerThread* m_inputServer;
    class KHelpMenu* m_HelpMenu;

    ClientPropertyRepository m_property_repository;
    GlobalPropertyRepositoryIDDict m_global_property_dict;
    KActionPtrList m_guiPropertyActions;
    KActionPtrList m_frontendPropertyActions;

    bool updateFrontendPropertiesNeeded;
    bool updateGUIPropertiesNeeded;
};

#endif
