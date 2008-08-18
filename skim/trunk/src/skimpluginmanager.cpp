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

#include <qwidget.h>
#include <qtimer.h>
#include <qsignal.h> 
#include <qdir.h>

#include <kdebug.h>
#include <kparts/componentfactory.h>
#include <kapplication.h>
#include <kconfigdialog.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <ksettings/dispatcher.h>

#include "skimglobalactions.h"
#include "skimplugin.h"
#include "utils/skimplugininfo.h"
#include "utils/scimactions.h"

#if defined(Q_WS_X11)
#include <X11/Xlib.h>
#endif

using namespace scim;

#include <qvaluestack.h>

#ifdef DEBUG_SESSIONMANAGEMENT
#include <qfile.h>
#endif

class SkimSessionManaged : KSessionManaged
{
public:
    SkimSessionManaged() : m_committed(false)
    {
#if DEBUG_SESSIONMANAGEMENT
        QFile f("/tmp/commitData");
        if ( f.open( IO_WriteOnly | IO_Append ) ) {
            QTextStream stream( &f );
            stream << "\nSkimSessionManaged started\n";
            f.close();
        }
#endif
    }
    bool commitData(QSessionManager&)
    {
        m_committed = true;
#if DEBUG_SESSIONMANAGEMENT
        QFile f("/tmp/commitData");
        if ( f.open( IO_WriteOnly | IO_Append ) ) {
            QTextStream stream( &f );
            stream << "commitData(QSessionManager&)\n";
            f.close();
        }
#endif
        ScimKdeSettings::writeConfig();
        return true;
    }

    bool dataCommitted()
    {
        return m_committed;
    }
private:
    bool m_committed;
};

class SkimPluginManagerPrivate
{
public:
  // All available plugins, regardless of category, and loaded or not
  SkimPluginInfo::List plugins;

  // Dict of all currently loaded plugins, mapping the SkimPluginInfo to
  // a plugin
  QMap<SkimPluginInfo *, SkimPlugin *> loadedPlugins;

  QMap<int, QPair<SkimPluginInfo *, int> > pluginActionInfoRep;
  QMap<int, KAction * > pluginActionRepository;

  // The plugin manager's mode. The mode is StartingUp until loadAllPlugins()
  // has finished loading the plugins, after which it is set to Running.
  // ShuttingDown and DoneShutdown are used during skim shutdown by the
  // async unloading of plugins.
  enum ShutdownMode { StartingUp, Running, ShuttingDown, DoneShutdown, Reloading };
  ShutdownMode shutdownMode;

  // Plugins pending for loading
  QValueStack<QString> pluginsToLoad;
  QValueList<int> pluginActionsToActivate;

  // These two lists are used to overwrite enabled plugins set in the config file
  // and if one plugin keyname are included in both of them, it will be enabled
  QStringList forceEnabledPlugins;
  QStringList forceDisabledPlugins;

  bool forcedRun;

  QValueList<QObject *> hasSpecialProperyObjects;
};

void SkimPluginManager::registerSpecialProperyObject(QObject * o)
{
    if(d->hasSpecialProperyObjects.contains(o))
        return;

    d->hasSpecialProperyObjects.append(o);
    connect(o, SIGNAL(destroyed ( QObject * )), SLOT(removeSpecialObject (QObject *)));
}

void SkimPluginManager::removeSpecialObject(QObject * o)
{
    if(d->hasSpecialProperyObjects.contains(o))
    {
        d->hasSpecialProperyObjects.remove(o);
    }
}

QValueList<QObject *> SkimPluginManager::specialProperyObjects(const char * name)
{
    if(!name)
        return d->hasSpecialProperyObjects;

    QValueList<QObject *> ret;
    for(size_t i=0; i<d->hasSpecialProperyObjects.size(); i++)
    {
        if(d->hasSpecialProperyObjects[i]->metaObject()->findProperty(name) != -1)
            ret.append(d->hasSpecialProperyObjects[i]);
    }

    return ret;
}

SkimGlobalActions* SkimPluginManager::getActionCollection()
{
  return defaultActionCollection;
}

SkimPluginManager* SkimPluginManager::m_self = 0L;
scim::ConfigModule* SkimPluginManager::m_cmodule = 0L;
scim::ConfigPointer SkimPluginManager::m_config;

SkimPluginManager* SkimPluginManager::self()
{
    if ( !m_self )
      new SkimPluginManager();

    return m_self;
}

scim::ConfigPointer SkimPluginManager::scimConfigObject()
{
    if(m_cmodule && !m_config.null ())
        return m_config;

    m_cmodule = new scim::ConfigModule ("kconfig");
    if (m_cmodule && m_cmodule->valid ()) {
        m_config = m_cmodule->create_config ();
        if (m_config.null ()) {
            std::cerr << "  Failed to load Config Module 'kconfig'\n";
        }
    } else
        std::cerr << "  Failed to load Config Module 'kconfig'\n";

    if (m_config.null ()) m_config = new scim::DummyConfig ();

    return m_config;
}

SkimPluginManager::SkimPluginManager( QStringList includePlugins, QStringList excludePlugins,
   QStringList otherArgs, QObject *, const char *name)
    : QObject(0, name), aboutData(*(KGlobal::instance()->aboutData())),
      d(new SkimPluginManagerPrivate), m_sm(new SkimSessionManaged)
{
    if ( !m_self )
      m_self = this;

    d->forceEnabledPlugins = includePlugins;
    d->forceDisabledPlugins = excludePlugins;
    d->shutdownMode = SkimPluginManagerPrivate::StartingUp;
    d->forcedRun = false;

    //the correct display for this program should have been set properly before this point (set by qapplication)
    QString display;
    #if defined(Q_WS_X11)
    setenv ("DISPLAY", DisplayString(QPaintDevice::x11AppDisplay()), 1);
    display = DisplayString(QPaintDevice::x11AppDisplay());
    #else
    display = getenv ("DISPLAY");
    #endif
    InputServer = new scim::SocketServerThread(this, otherArgs);

    bool socketServerStarted = true;
    if(!InputServer->initSocketServer(display))
    {
      d->shutdownMode = SkimPluginManagerPrivate::DoneShutdown;
      socketServerStarted = false;
      if(!otherArgs.contains("force"))
        deleteLater();
      else
        d->forcedRun = true;
    }

    if(socketServerStarted)
    {
      InputServer->start();
      InputServer->start_auto_start_helpers();
    }
    if(socketServerStarted || d->forcedRun)
    {
      KSettings::Dispatcher::self()->registerInstance( KGlobal::instance(), this, SLOT( loadAllPlugins() ) );

      d->plugins = allAvailablePlugins();

      defaultActionCollection = new SkimGlobalActions(this, "Global Available Actions");

      loadAllPlugins();

      connect(kapp,SIGNAL(shutDown()),this, SLOT(shutdown()));

      connect(InputServer,SIGNAL(disconnectCompleted()),this, SLOT(shutdown()));
    }
}

SkimPluginInfo::List SkimPluginManager::allAvailablePlugins() {
    return SkimPluginInfo::fromServices( KTrader::self()->query( QString::fromLatin1( "Skim/Plugin" )));
}

void SkimPluginManager::pluginActionActivated(int id) {
    if(!d->loadedPlugins.contains(d->pluginActionInfoRep[id].first)) {
      d->pluginActionsToActivate.push_back(id);
      loadPluginInternal(d->pluginActionInfoRep[id].first->pluginName());
    } /*else {
      //if this plugin has been loaded, the action will get executed immediately because the target
      //slot has been connected to this action's activated() signal
    }*/
}

const KAboutData * SkimPluginManager::getAboutData()
{
    return &aboutData;
}

SkimPluginManager::~SkimPluginManager()
{
    if ( d->shutdownMode != SkimPluginManagerPrivate::DoneShutdown )
      kdWarning( 18018 ) << k_lineinfo << "Destructing plugin manager without going through the shutdown process!" << endl << kdBacktrace() << endl;

    // Quick cleanup of the remaining plugins, hope it helps
    QMap<SkimPluginInfo *, SkimPlugin *>::ConstIterator it;
    for ( it = d->loadedPlugins.begin(); it != d->loadedPlugins.end(); /* EMPTY */ )
    {
        // Remove causes the iterator to become invalid, so pre-increment first
        QMap<SkimPluginInfo *, SkimPlugin *>::ConstIterator nextIt( it );
        ++nextIt;
        kdWarning( 18018 ) << k_lineinfo << "Deleting stale plugin '" << it.data()->name() << "'" << endl;
        delete it.data();
        it = nextIt;
    }

    delete d;

    if(InputServer->running())
      if(!InputServer->wait(3000))
        std::cerr << "Input Socket Server exited abnormally!\n";

#if DEBUG_SESSIONMANAGEMENT
    QFile f("/tmp/commitData");
    if ( f.open( IO_WriteOnly | IO_Append ) ) {
        QTextStream stream( &f );
        stream << "SkimPluginManager::~SkimPluginManager() " << m_sm->dataCommitted() << "\n stoped \n";
        f.close();
    }
#endif
    //check whether the configure is already saved when logout the session
    if(!m_sm->dataCommitted())
        ScimKdeSettings::writeConfig();

    delete m_sm;
    delete ScimKdeSettings::self();
    kdDebug( 18018 ) << k_lineinfo << "SkimPluginManager exited!" << endl;
    kapp->quit();
}

SkimPluginInfo::List SkimPluginManager::availablePlugins( const QString &category ) const
{
    if ( category.isEmpty() )
      return d->plugins;

    SkimPluginInfo::List result;
    SkimPluginInfo::List::ConstIterator it;
    for ( it = d->plugins.begin(); it != d->plugins.end(); ++it )
    {
      if ( ( *it )->category() == category )
        result.append( *it );
    }

    return result;
}

QMap<SkimPluginInfo *, SkimPlugin *> SkimPluginManager::loadedPlugins( const QString &category ) const
{
    QMap<SkimPluginInfo *, SkimPlugin *> result;
    QMap<SkimPluginInfo *, SkimPlugin *>::ConstIterator it;
    for ( it = d->loadedPlugins.begin(); it != d->loadedPlugins.end(); ++it )
    {
      if ( category.isEmpty() || it.key()->category() == category )
        result.insert( it.key(), it.data() );
    }

    return result;
}

void SkimPluginManager::shutdown()
{
    //shutdown can be called when all connections are closed. As this signal is also needed for unload plugin
    //procedure, so we disconnect this signal from this slot
    disconnect( InputServer, SIGNAL( disconnectCompleted() ), this, SLOT(shutdown()));

    if(!d->forcedRun)
        InputServer->shutdown();

    d->shutdownMode = SkimPluginManagerPrivate::ShuttingDown;

    // Remove any pending plugins to load, we're shutting down now :)
    d->pluginsToLoad.clear();

    // Ask all plugins to unload
    QMap<SkimPluginInfo *, SkimPlugin *>::ConstIterator it;
    for ( it = d->loadedPlugins.begin(); it != d->loadedPlugins.end(); /* EMPTY */ )
    {
      // Remove causes the iterator to become invalid, so pre-increment first
      QMap<SkimPluginInfo *, SkimPlugin *>::ConstIterator nextIt( it );
      ++nextIt;
      it.data()->aboutToUnload();
      it = nextIt;
    }

    QTimer::singleShot( 3000, this, SLOT( slotShutdownTimeout() ) );
}

void SkimPluginManager::slotPluginReadyForUnload()
{
    // Using QObject::sender() is on purpose here, because otherwise all
    // plugins would have to pass 'this' as parameter, which makes the API
    // less clean for plugin authors
    SkimPlugin *plugin = dynamic_cast<SkimPlugin *>( const_cast<QObject *>( sender() ) );
    if ( !plugin )
    {
      kdWarning( 18018 ) << k_lineinfo << "Calling object is not a plugin!" << endl;
      return;
    }

    plugin->deleteLater();
}

void SkimPluginManager::slotShutdownTimeout()
{
    // When we were already done the timer might still fire.
    // Do nothing in that case.
    if ( d->shutdownMode == SkimPluginManagerPrivate::DoneShutdown )
      return;

    QStringList remaining;
    for ( QMap<SkimPluginInfo *, SkimPlugin *>::ConstIterator it = d->loadedPlugins.begin(); it != d->loadedPlugins.end(); ++it )
      remaining.append( it.data()->pluginId() );

    kdWarning( 18018 ) << k_lineinfo << "Some plugins didn't shutdown in time!" << endl
      << "Remaining plugins: " << remaining.join( QString::fromLatin1( ", " ) ) << endl
      << "Forcing skim shutdown now." << endl;

    slotShutdownDone();
}

void SkimPluginManager::slotShutdownDone()
{
    d->shutdownMode = SkimPluginManagerPrivate::DoneShutdown;

    if(d->forcedRun)
        deleteLater();
    else
        connect( InputServer, SIGNAL( disconnectCompleted() ), this, SLOT( deleteLater() ));
}

void SkimPluginManager::reloadAllPlugins() {

    d->shutdownMode = SkimPluginManagerPrivate::Reloading;

    // Ask all plugins to unload
    QMap<SkimPluginInfo *, SkimPlugin *>::ConstIterator it;
    for ( it = d->loadedPlugins.begin(); it != d->loadedPlugins.end(); /* EMPTY */ )
    {
      // Remove causes the iterator to become invalid, so pre-increment first
      QMap<SkimPluginInfo *, SkimPlugin *>::ConstIterator nextIt( it );
      ++nextIt;
      unloadPlugin(it.key());
      it = nextIt;
    }
}

void SkimPluginManager::loadAllPlugins()  // FIXME: We need session management here
{
    //we need to set this in order to emit the allPluginsLoaded signal
    //whenever plugins are reloaded
    if(d->shutdownMode == SkimPluginManagerPrivate::Running)
        d->shutdownMode = SkimPluginManagerPrivate::StartingUp;

    KIconLoader * kil = KGlobal::iconLoader();
    KConfigGroup *config = new KConfigGroup(KGlobal::config(), "Plugins");

    QStringList overloadedModules;
    for (SkimPluginInfo::List::ConstIterator it = d->plugins.begin(); it != d->plugins.end(); ++it )
    {
        QString key = ( *it )->pluginName();
        ( *it )->load(config);

        if ( d->forceDisabledPlugins.contains(key) && ( *it )->isNoDisplay())
        {
            //hidden plugins can not be disabled
            d->forceDisabledPlugins.remove(key);
            ( *it )->setPluginEnabled(true);
        }

        if ( (( *it )->isPluginEnabled() && !d->forceDisabledPlugins.contains(key))
                || d->forceEnabledPlugins.contains(key))
        {
            overloadedModules += ( *it )->overloadedModules();
            if ( !plugin( key ) ) {
                if(( *it )->isHasActions()) {
                    QValueList<SkimPluginActionInfo> & actions = ( *it )->actions();

                    for(uint ita = 0; ita < actions.size(); ita++) {
                        SkimPluginActionInfo & actinfo = actions[ita];

                        QIconSet icon(kil->loadIcon(actinfo.icon, KIcon::Toolbar));
                        if(( *it )->isOnDemand()) {
                            ScimAction * action = new ScimAction(actinfo.name, icon,
                                    defaultActionCollection, actinfo.id, actinfo.internalName.latin1());
                            action->setOption(scim_kde::ToolBarIconOnly);
                            action->setIcon(actinfo.icon);
                            d->pluginActionRepository[actinfo.id] = action;
                            connect(action, SIGNAL(activated(int )), SLOT(pluginActionActivated(int)));
                        } else {
                            if(actinfo.type == "Toggle") {
                                KToggleAction * action = new KToggleAction(actinfo.name, icon, "", 0, 0,
                                        defaultActionCollection, actinfo.internalName.latin1());
                                d->pluginActionRepository[actinfo.id] = action;
                            } else {
                                KAction * action = new KAction(actinfo.name, icon, "", 0, 0,
                                        defaultActionCollection, actinfo.internalName.latin1());
                                d->pluginActionRepository[actinfo.id] = action;
                            }
                        }
                        d->pluginActionInfoRep[actinfo.id] = QPair<SkimPluginInfo*, int> (*it, ita);
                    }
//                     d->pluginActionInfo2ExtraInfo[*it] = einfo;
                }
                if(!( *it )->isHasActions() || !( *it )->isOnDemand())
                    d->pluginsToLoad.push( key );
            }
        } else {
            if ( plugin( key ) ) {
                unloadPlugin( *it );
            }
        }
    }

    InputServer->setOverloadedModules(overloadedModules);
    delete config;
    // Schedule the plugins to load
    QTimer::singleShot( 0, this, SLOT( slotLoadNextPlugin() ) );
}

void SkimPluginManager::slotLoadNextPlugin()
{
    if ( d->pluginsToLoad.isEmpty() )
    {
      if ( d->shutdownMode == SkimPluginManagerPrivate::StartingUp ||
          d->shutdownMode == SkimPluginManagerPrivate::Reloading )
      {
        d->shutdownMode = SkimPluginManagerPrivate::Running;
        emit allPluginsLoaded();
      }
      return;
    }

    QString key = d->pluginsToLoad.pop();
    loadPluginInternal( key );

    // Schedule the next run unconditionally to avoid code duplication on the
    // allPluginsLoaded() signal's handling. This has the added benefit that
    // the signal is delayed one event loop, so the accounts are more likely
    // to be instantiated.
    QTimer::singleShot( 0, this, SLOT( slotLoadNextPlugin() ) );
}

SkimPlugin * SkimPluginManager::loadPlugin( const QString &_pluginId, PluginLoadMode mode /* = LoadSync */ )
{
    QString pluginId = _pluginId;

    if ( mode == LoadSync )
    {
      return loadPluginInternal( pluginId );
    }
    else
    {
      d->pluginsToLoad.push( pluginId );
      QTimer::singleShot( 0, this, SLOT( slotLoadNextPlugin() ) );
      return 0L;
    }
}

SkimPlugin *SkimPluginManager::loadPluginInternal( const QString &pluginId )
{
    SkimPluginInfo *info = infoForPluginId( pluginId );
    if ( !info )
    {
      kdWarning( 18018 ) << k_funcinfo << "Unable to find a plugin named '" << pluginId << "'!" << endl;
      return 0L;
    }

    if ( d->loadedPlugins.contains( info ) )
      return d->loadedPlugins[ info ];

    int error = 0;
    SkimPlugin *plugin = KParts::ComponentFactory::createInstanceFromQuery<SkimPlugin>( 
      QString::fromLatin1( "Skim/Plugin" ),
      QString::fromLatin1( "[X-KDE-PluginInfo-Name]=='%1'" ).arg( pluginId ), 
      this, 0, QStringList(), &error );

    if ( plugin )
    {
      d->loadedPlugins.insert( info, plugin );
//       info->setPluginEnabled( true );

      connect( plugin, SIGNAL( destroyed( QObject * ) ), this, SLOT( slotPluginDestroyed( QObject * ) ) );
      connect( plugin, SIGNAL( readyForUnload() ), this, SLOT( slotPluginReadyForUnload() ) );

      kdDebug( 18018 ) << k_lineinfo << "Successfully loaded plugin '" << pluginId << "'" << endl;

      for(uint i = 0; i < info->actions().size(); i++) {
          SkimPluginActionInfo & ainfo = info->actions()[i];
          connect(d->pluginActionRepository[ainfo.id], SIGNAL(activated()), plugin, ainfo.slot.latin1());
      }

      for(uint i = 0; i< d->pluginActionsToActivate.size(); ++i) {
        int id = d->pluginActionsToActivate[i];
        if(info == d->pluginActionInfoRep[id].first) {
          pluginActionActivated(id);
          d->pluginActionsToActivate.remove(id);
        }
      }
      emit pluginLoaded( plugin );
    }
    else
    {
      switch( error )
      {
      case KParts::ComponentFactory::ErrNoServiceFound:
        kdDebug( 18018 ) << k_lineinfo << "No service implementing the given mimetype "
          << "and fullfilling the given constraint expression can be found." << endl;
        break;

      case KParts::ComponentFactory::ErrServiceProvidesNoLibrary:
        kdDebug( 18018 ) << "the specified service provides no shared library." << endl;
        break;

      case KParts::ComponentFactory::ErrNoLibrary:
        kdDebug( 18018 ) << "the specified library could not be loaded." << endl;
        break;

      case KParts::ComponentFactory::ErrNoFactory:
        kdDebug( 18018 ) << "the library does not export a factory for creating components." << endl;
        break;

      case KParts::ComponentFactory::ErrNoComponent:
        kdDebug( 18018 ) << "the factory does not support creating components of the specified type." << endl;
        break;
      }

      kdDebug( 18018 ) << k_lineinfo << "Loading plugin '" << pluginId << "' failed, KLibLoader reported error: '" << endl <<
        KLibLoader::self()->lastErrorMessage() << "'" << endl;
    }

    return plugin;
}

bool SkimPluginManager::unloadPlugin( const QString &spec )
{
    kdDebug() << k_lineinfo << spec << endl;

    QMap<SkimPluginInfo *, SkimPlugin *>::ConstIterator it;
    for ( it = d->loadedPlugins.begin(); it != d->loadedPlugins.end(); ++it )
    {
      if ( it.key()->pluginName() == spec )
      {
        unloadPlugin(it.key());
        return true;
      }
    }

    return false;
}

bool SkimPluginManager::unloadPlugin( SkimPluginInfo *info )
{
    //first delete all actions registered by this plugin
    for(uint curaction = 0; curaction < info->actions().size(); ++curaction) {
        int id = info->actions()[curaction].id;
        d->pluginActionRepository[id]->unplugAll();
        d->pluginActionRepository[id]->deleteLater();
        d->pluginActionRepository.remove(id);
    }

    d->loadedPlugins[info]->aboutToUnload();
    return true;
}

void SkimPluginManager::slotPluginDestroyed( QObject *plugin )
{
    QMap<SkimPluginInfo *, SkimPlugin *>::Iterator it;
    for ( it = d->loadedPlugins.begin(); it != d->loadedPlugins.end(); ++it )
    {
      if ( it.data() == plugin )
      {
        kdDebug( 18018 ) << k_lineinfo << "Plugin '" << it.key()->pluginName() 
          << "' is successfully unloaded." << endl;
        d->loadedPlugins.erase( it );
        break;
      }
    }

    if (d->loadedPlugins.isEmpty())
    {
      if ( d->shutdownMode == SkimPluginManagerPrivate::ShuttingDown )
      {
        // Use a timer to make sure any pending deleteLater() calls have
        // been handled first
        QTimer::singleShot( 0, this, SLOT( slotShutdownDone() ) );
      }
      if ( d->shutdownMode == SkimPluginManagerPrivate::Reloading )
        QTimer::singleShot( 0, this, SLOT( loadAllPlugins() ) );
    }
}

SkimPlugin* SkimPluginManager::plugin( const QString &_pluginId ) const
{
    SkimPluginInfo *info = infoForPluginId( _pluginId );
    if ( !info )
      return 0L;

    if ( d->loadedPlugins.contains( info ) )
      return d->loadedPlugins[ info ];
    else
      return 0L;
}
QString SkimPluginManager::pluginName( const SkimPlugin *plugin ) const
{
    QMap<SkimPluginInfo *, SkimPlugin *>::ConstIterator it;
    for ( it = d->loadedPlugins.begin(); it != d->loadedPlugins.end(); ++it )
    {
      if ( it.data() == plugin )
        return it.key()->name();
    }

    return QString::fromLatin1( "Unknown" );
}

QString SkimPluginManager::pluginId( const SkimPlugin *plugin ) const
{
    QMap<SkimPluginInfo *, SkimPlugin *>::ConstIterator it;
    for ( it = d->loadedPlugins.begin(); it != d->loadedPlugins.end(); ++it )
    {
      if ( it.data() == plugin )
        return it.key()->pluginName();
    }

    return QString::fromLatin1( "unknown" );
}

QString SkimPluginManager::pluginIcon( const SkimPlugin *plugin ) const
{
    QMap<SkimPluginInfo *, SkimPlugin *>::ConstIterator it;
    for ( it = d->loadedPlugins.begin(); it != d->loadedPlugins.end(); ++it )
    {
      if ( it.data() == plugin )
        return it.key()->icon();
    }

    return QString::fromLatin1( "Unknown" );
}

SkimPluginInfo * SkimPluginManager::infoForPluginId( const QString &pluginId ) const
{
    SkimPluginInfo::List::ConstIterator it;
    for ( it = d->plugins.begin(); it != d->plugins.end(); ++it )
    {
      if ( ( *it )->pluginName() == pluginId )
        return *it;
    }

    return 0L;
}

bool SkimPluginManager::setPluginEnabled( const QString &_pluginId, bool enabled /* = true */ )
{
    QString pluginId = _pluginId;

    KConfig *config = KGlobal::config();
    config->setGroup( "Plugins" );

    if ( !infoForPluginId( pluginId ) )
      return false;

    config->writeEntry( pluginId/* + QString::fromLatin1( "Enabled" )*/, enabled );
    config->sync();

    return true;
}

scim::SocketServerThread* SkimPluginManager::getInputServer()
{
    return InputServer;
}

#include "skimpluginmanager.moc"

