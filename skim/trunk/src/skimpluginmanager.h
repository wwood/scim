/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef SKIMPLUGINMANAGER_H
#define SKIMPLUGINMANAGER_H

#include "socketserverthread.h"
#include <kconfig.h> 
#include <vector>

#include <qstring.h>
#include <qwidget.h>

class KAboutData;
class SkimPluginInfo;

class SkimGlobalActions;
class SkimPlugin;

class SkimSessionManaged;
class SkimPluginManagerPrivate;

class SkimPluginManager : public QObject
{
Q_OBJECT
friend class ScimLauncher;
public:
    SkimPluginManager( QStringList includePlugins = QStringList(), 
        QStringList excludePlugins = QStringList(),
        QStringList otherArgs = QStringList(),
        QObject *parent = 0, const char *name = 0);
    ~SkimPluginManager();
    static SkimPluginManager* self();
    static scim::ConfigPointer scimConfigObject();

    QValueList<SkimPluginInfo *> availablePlugins( const QString &category = QString::null ) const;
    QMap<SkimPluginInfo *, SkimPlugin *> loadedPlugins( const QString &category = QString::null ) const;
    SkimPlugin *plugin( const QString &pluginName ) const;
    QString pluginId( const SkimPlugin *plugin ) const;
    QString pluginName( const SkimPlugin *plugin ) const;
    bool unloadPlugin( const QString &pluginName );
    QString pluginIcon( const SkimPlugin *plugin ) const;
    bool setPluginEnabled( const QString &name, bool enabled = true );

    static QValueList<SkimPluginInfo *> allAvailablePlugins();

    enum PluginLoadMode { LoadSync, LoadAsync };

    virtual scim::SocketServerThread* getInputServer();
    virtual const KAboutData * getAboutData();
    virtual SkimGlobalActions* getActionCollection();
    virtual void registerSpecialProperyObject(QObject *);
    virtual QValueList<QObject *> specialProperyObjects(const char * name = 0);

protected:
    bool loadModule(struct ModuleDefinition defaultMD, const QString & file, 
    const QValueList<QString> & uuidsToLoad, QWidget* preferedParent = 0 );
    bool loadModule( uint index, QWidget* preferedParent = 0 );
    bool unloadPlugin( SkimPluginInfo *info );

signals:
    void allPluginsLoaded();
    void pluginLoaded( SkimPlugin *plugin );
    void settingsChanged();
public slots:
    SkimPlugin * loadPlugin( const QString &pluginId, PluginLoadMode mode = LoadSync );
    void loadAllPlugins();
    void reloadAllPlugins();
    void shutdown();
    void removeSpecialObject (QObject *);

protected slots:
    void pluginActionActivated(int);
    void slotPluginReadyForUnload ();
    void slotShutdownDone ();
    void slotShutdownTimeout ();
    void slotPluginDestroyed ( QObject *plugin );
    void slotLoadNextPlugin ();

private:
    SkimPlugin * loadPluginInternal( const QString &pluginId );

    SkimPluginInfo * infoForPluginId( const QString &pluginId ) const;

    scim::SocketServerThread* InputServer;

    const KAboutData & aboutData;

    SkimGlobalActions* defaultActionCollection;

    static SkimPluginManager *m_self;
    static scim::ConfigPointer m_config;
    static scim::ConfigModule * m_cmodule;
    SkimPluginManagerPrivate * const d;
    SkimSessionManaged * const m_sm;
};

#endif
