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
#define Uses_SCIM_CONFIG_PATH
#include "src/skimpluginmanager.h"
#include "scimsetupwindow.h"

#include "src/skimglobalactions.h"

#include <sys/time.h>

#include <qlayout.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kparts/componentfactory.h>
#include "skimplugininfo.h"
#include <kcmoduleinfo.h> 

#include <ksettings/dispatcher.h>
#include <kgenericfactory.h>
#include <kapplication.h>

#include <map>

class ScimSetupWindowPrivate
{
public:
    scim::SocketServerThread *inputServer;
    bool scim_backend_need_update;
    std::vector<KCModuleInfo> mods;
    typedef std::multimap<int, int> WeightsMap; //map weight to index in mods
    struct SetupDirInfo
    {
        QStringList path;
        QString iconfile;
        WeightsMap sortedMods;
    };
    typedef std::multimap<int, SetupDirInfo> SetupDirRespType;//map weight to SetupDirInfo
    SetupDirRespType dirReposition; 
};

#define SetupWindowGroup "SetupWindow"
#define SetupWindowSize "SetupWindowSize"

ScimSetupWindow::ScimSetupWindow(scim::SocketServerThread *parent, const char */*name*/, KConfigSkeleton */*config*/)
 : KCMultiDialog(KDialogBase::TreeList, i18n("Configure skim"), 0), d(new ScimSetupWindowPrivate)
{
    m_mc = SkimPluginManager::self();
    setIcon(KGlobal::iconLoader()->loadIcon("configure", KIcon::NoGroup));
    d->inputServer = parent;
    d->scim_backend_need_update = false;
    setShowIconsInTreeList( true );
    connect( this, SIGNAL( configCommitted( const QCString & ) ),
      KSettings::Dispatcher::self(), SLOT( reparseConfiguration( const QCString & ) ) );
    connect( this, SIGNAL( configCommitted( const QCString & ) ), 
      SLOT(slotConfigurationChangedFor( const QCString & )));
    connect( m_mc, SIGNAL(allPluginsLoaded()), this, SLOT(load()));

    //we cache all the available kcms and their hierarchy, so that we do not
    //need to reload them everytime load() is called
    QValueList<SkimPluginInfo *> skimplugins = SkimPluginInfo::fromServices( KTrader::self()->query( "Skim/SetupDir", "[X-KDE-PluginInfo-Category] == 'Root'" ));

    QValueList<SkimPluginInfo *>::ConstIterator pit;

    ScimSetupWindowPrivate::SetupDirInfo curDir;
    for ( pit = skimplugins.begin(); pit != skimplugins.end(); ++pit )
    {
        curDir.sortedMods.clear();
        curDir.path.clear();
        QValueList< KService::Ptr > services =
                KTrader::self()->query( "Skim/KCModule",
        "[X-KDE-PluginInfo-DisplayParent] == '" + (*pit)->pluginName() + "'" );

        std::multimap<int, int> sortedMods; //map weight to index in mods
        /*if( services.size() > 1)*/
        {
            curDir.path << (*pit)->name();
            curDir.iconfile = (*pit)->icon();

            //sort the kcm according to their weight. 0 will be the first
            for( QValueList<KService::Ptr>::ConstIterator it =
                 services.begin();
                 it != services.end(); ++it )
            {
                KCModuleInfo mod(*it);
                d->mods.push_back(mod);
                curDir.sortedMods.insert(std::pair<int, int> (mod.weight(), d->mods.size()-1));
            }
            QVariant v = ( *pit )->weight();
            int weight = v.isValid () ? v.toInt() : 1000;
            //sort the dir according to their weight. 0 will be the first
            d->dirReposition.insert(std::pair<int, ScimSetupWindowPrivate::SetupDirInfo> (weight, curDir));
        }
    }

    load();

    if(ScimKdeSettings::self()->config()->hasGroup(SetupWindowGroup))
    {
        ScimKdeSettings::self()->config()->setGroup(SetupWindowGroup);
        if( ScimKdeSettings::self()->config()->hasKey(SetupWindowSize))
            resize(ScimKdeSettings::self()->config()->readSizeEntry(SetupWindowSize));
    }
}

void ScimSetupWindow::load()
{
    removeAllModules();

    //first we load all plugins and retrieve a list of kcmodules which belong to
    //disabled plugins
    QStringList disabledModules;
    QValueList<SkimPluginInfo *> plugins = m_mc->availablePlugins();
    for (QValueList<SkimPluginInfo *>::ConstIterator it = plugins.begin(); it != plugins.end(); ++it )
    {
        if( (*it)->pluginName() == "skimplugin_scim")  //all kcmodules for this plugin should always be shown
            continue;

        if( !( *it )->isPluginEnabled() ) {
            const QValueList< KService::Ptr > & kcms = ( *it )->kcmServices();
            for(uint i=0; i<kcms.size(); i++)
                disabledModules << kcms[i]->library(), kdDebug() << kcms[i]->library() << " is disabled\n";
        }
    }

    ScimSetupWindowPrivate::SetupDirRespType::const_iterator pit;

    for ( pit = d->dirReposition.begin(); pit != d->dirReposition.end(); ++pit )
    {
        setFolderIcon((*pit).second.path, SmallIcon( (*pit).second.iconfile, IconSize( KIcon::Small ) ));

        for( ScimSetupWindowPrivate::WeightsMap::const_iterator it =
             (*pit).second.sortedMods.begin(); it != (*pit).second.sortedMods.end(); ++it )
        {
            if( !disabledModules.contains(d->mods[(*it).second].library()) )
                addModule(d->mods[(*it).second], (*pit).second.path);
        }
    }

    unfoldTreeList();
}

void ScimSetupWindow::hide()
{
    KCMultiDialog::hide();
    emit readyForUnload();
}

KActionCollection* ScimSetupWindow::actionCollection()
{
    return m_mc->getActionCollection();
}

ScimSetupWindow::~ScimSetupWindow()
{
    ScimKdeSettings::self()->config()->setGroup(SetupWindowGroup);
    ScimKdeSettings::self()->config()->writeEntry(SetupWindowSize, size());
}

void ScimSetupWindow::slotOk()
{
    KCMultiDialog::slotOk();
    slotApply();
}

void ScimSetupWindow::slotApply()
{
    kdDebug(18010) << "ScimSetupWindow::updateSettings()\n";

    KCMultiDialog::slotApply();

    {//update SCIM_CONFIG_UPDATE_TIMESTAMP to make config module emit "reload" signal
    timeval m_update_timestamp;
    gettimeofday (&m_update_timestamp, 0);

    char buf [128];
    snprintf (buf, 128, "%lu:%lu", m_update_timestamp.tv_sec, m_update_timestamp.tv_usec);
    
    ScimKdeSettings::self()->config()->setGroup("SCIM");
    ScimKdeSettings::self()->config()->writeEntry(QString::fromLatin1(SCIM_CONFIG_UPDATE_TIMESTAMP),
        QString::fromLatin1(buf));
    ScimKdeSettings::writeConfig();
    }

    if(d->scim_backend_need_update) {
      kapp->lock();
      d->inputServer->reloadScimConfig();
      kapp->unlock();
    }
    d->scim_backend_need_update = false;
}

void ScimSetupWindow::slotConfigurationChangedFor( const QCString & component)
{//TODO: the correctness of this slot behavior relys on the immediate invoke of 
 // slots in Qt3. Maybe need modification in Qt4
    if( component == "skimplugin_scim" )
        d->scim_backend_need_update = true;
}


typedef KGenericFactory<SkimConfigPlugin> SkimConfigPluginFactory;

K_EXPORT_COMPONENT_FACTORY( skimplugin_setupwindow,
  SkimConfigPluginFactory( "skimplugin_setupwindow" ) )

SkimConfigPlugin::SkimConfigPlugin( QObject *parent, const char *name, const QStringList & /*args*/ )
 : SkimPlugin(SkimConfigPluginFactory::instance(), parent, name)
{
     m_configdialog = new ScimSetupWindow(inputServer,
       "SetupWindow", ScimKdeSettings::self());
     connect(m_configdialog, SIGNAL(readyForUnload()), this, SIGNAL(readyForUnload()));
}

void SkimConfigPlugin::SlotConfigure()
{
    if(m_configdialog->isVisible())
        m_configdialog->raise();
    else
        m_configdialog->show();
}

SkimConfigPlugin::~SkimConfigPlugin()
{
    delete m_configdialog;
}

#include "scimsetupwindow.moc"
