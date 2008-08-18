/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "skimpluginmanager.h"
#include "skimglobalactions.h"
#include "socketserverthread.h"
#include "skimplugin.h"

#include <kaction.h>
#include <ksettings/dispatcher.h>

SkimPlugin::SkimPlugin( KInstance *instance, QObject *_parent, const char *name )
: QObject( _parent, name ), KXMLGUIClient()
{
    setInstance( instance );
    KSettings::Dispatcher::self()->registerInstance( instance, this, SIGNAL( settingsChanged() ) );
    inputServer =  static_cast<scim::SocketServerThread*>
           ( parent()->child( 0, "scim::SocketServerThread" ) );
    globalActionCollection = static_cast<SkimGlobalActions *>
           ( parent()->child( 0, "SkimGlobalActions" ) );
}

SkimPlugin::~SkimPlugin(){}

QString SkimPlugin::pluginId() const
{
  return QString::fromLatin1( className() );
}

SkimPlugin::CustomActionsList SkimPlugin::customActions()
{
    return CustomActionsList();
}

QString SkimPlugin::displayName() const
{
  return SkimPluginManager::self()->pluginName( this );
}

QString SkimPlugin::pluginIcon() const
{
  return SkimPluginManager::self()->pluginIcon( this );
}

void SkimPlugin::aboutToUnload()
{
    // Just make the unload synchronous by default
    emit readyForUnload();
}

#include "skimplugin.moc"
